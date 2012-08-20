/* match.c -- pattern matching routines ($Revision: 1.1.1.1 $) */

#include "es.h"

enum { RANGE_FAIL = -1, RANGE_ERROR = -2 };

/*
   From the ed(1) man pages (on ranges):

	The `-' is treated as an ordinary character if it occurs first
	(or first after an initial ^) or last in the string.

	The right square bracket does not terminate the enclosed string
	if it is the first character (after an initial `^', if any), in
	the bracketed string.

   rangematch() matches a single character against a class, and returns
   an integer offset to the end of the range on success, or -1 on
   failure.
*/

#define	ISQUOTED(q, n)	((q) == QUOTED || ((q) != UNQUOTED && (q)[n] == 'q'))
#define TAILQUOTE(q, n) ((q) == UNQUOTED ? UNQUOTED : ((q) + (n)))

/* rangematch -- match a character against a character class */
static int rangematch(const char *p, const char *q, char c) {
	const char *orig = p;
	Boolean neg;
	Boolean matched = FALSE;
	if (*p == '~' && !ISQUOTED(q, 0)) {
		p++, q++;
	    	neg = TRUE;
	} else
		neg = FALSE;
	if (*p == ']' && !ISQUOTED(q, 0)) {
		p++, q++;
		matched = (c == ']');
	}
	for (; *p != ']' || ISQUOTED(q, 0); p++, q++) {
		if (*p == '\0')
			return RANGE_ERROR;	/* bad syntax */
		if (p[1] == '-' && !ISQUOTED(q, 1) && ((p[2] != ']' && p[2] != '\0') || ISQUOTED(q, 2))) {
			/* check for [..-..] but ignore [..-] */
			if (c >= *p && c <= p[2])
				matched = TRUE;
			p += 2;
			q += 2;
		} else if (*p == c)
			matched = TRUE;
	}
	if (matched ^ neg)
		return p - orig + 1; /* skip the right-bracket */
	else
		return RANGE_FAIL;
}

/* match -- match a single pattern against a single string. */
extern Boolean match(const char *s, const char *p, const char *q) {
	int i;
	if (q == QUOTED)
		return streq(s, p);
	for (i = 0;;) {
		int c = p[i++];
		if (c == '\0')
			return *s == '\0';
		else if (q == UNQUOTED || q[i - 1] == 'r') {
			switch (c) {
			case '?':
				if (*s++ == '\0')
					return FALSE;
				break;
			case '*':
				while (p[i] == '*' && (q == UNQUOTED || q[i] == 'r'))	/* collapse multiple stars */
					i++;
				if (p[i] == '\0') 	/* star at end of pattern? */
					return TRUE;
				while (*s != '\0')
					if (match(s++, p + i, TAILQUOTE(q, i)))
						return TRUE;
				return FALSE;
			case '[': {
				int j;
				if (*s == '\0')
					return FALSE;
				switch (j = rangematch(p + i, TAILQUOTE(q, i), *s)) {
				default:
					i += j;
					break;
				case RANGE_FAIL:
					return FALSE;
				case RANGE_ERROR:
					if (*s != '[')
						return FALSE;
				}
				s++;
				break;
			}
			default:
				if (c != *s++)
					return FALSE;
			}
		} else if (c != *s++)
			return FALSE;
	}
}


/*
 * listmatch
 *
 * 	Matches a list of words s against a list of patterns p.
 *	Returns true iff a pattern in p matches a word in s.
 *	() matches (), but otherwise null patterns match nothing.
 */

extern Boolean listmatch(List *subject, List *pattern, StrList *quote) {
	if (subject == NULL) {
		if (pattern == NULL)
			return TRUE;
		Ref(List *, p, pattern);
		Ref(StrList *, q, quote);
		for (; p != NULL; p = p->next, q = q->next) {
			/* one or more stars match null */
			char *pw = getstr(p->term), *qw = q->str;
			if (*pw != '\0' && qw != QUOTED) {
				int i;
				Boolean matched = TRUE;
				for (i = 0; pw[i] != '\0'; i++)
					if (pw[i] != '*'
					    || (qw != UNQUOTED && qw[i] != 'r')) {
						matched = FALSE;
						break;
					}
				if (matched) {
					RefPop2(q, p);
					return TRUE;
				}
			}
		}
		RefEnd2(q, p);
		return FALSE;
	}

	Ref(List *, s, subject);
	Ref(List *, p, pattern);
	Ref(StrList *, q, quote);

	for (; p != NULL; p = p->next, q = q->next) {
		assert(q != NULL);
		assert(p->term != NULL);
		assert(q->str != NULL);
		Ref(char *, pw, getstr(p->term));
		Ref(char *, qw, q->str);
		Ref(List *, t, s);
		for (; t != NULL; t = t->next) {
			char *tw = getstr(t->term);
			if (match(tw, pw, qw)) {
				RefPop3(t, qw, pw);
				RefPop3(q, p, s);
				return TRUE;
			}
		}
		RefEnd3(t, qw, pw);
	}
	RefEnd3(q, p, s);
	return FALSE;
}

/*
 * extractsinglematch -- extract matching parts of a single subject and a
 *			 single pattern, returning it backwards
 */

static List *extractsinglematch(const char *subject, const char *pattern,
				const char *quoting, List *result) {
	int i;
	const char *s;

	if (!haswild(pattern, quoting) /* no wildcards, so no matches */
	    || !match(subject, pattern, quoting))
		return NULL;

	for (s = subject, i = 0; pattern[i] != '\0'; s++) {
		if (ISQUOTED(quoting, i))
			i++;
		else {
			int c = pattern[i++];
			switch (c) {
			    case '*': {
				const char *begin;
				if (pattern[i] == '\0')
					return mklist(mkstr(gcdup(s)), result);
				for (begin = s;; s++) {
					const char *q = TAILQUOTE(quoting, i);
					assert(*s != '\0');
					if (match(s, pattern + i, q)) {
						result = mklist(mkstr(gcndup(begin, s - begin)), result);
						return haswild(pattern + i, q)
							? extractsinglematch(s, pattern + i, q, result)
							: result;
					}
				}
			    }
			    case '[': {
				int j = rangematch(pattern + i, TAILQUOTE(quoting, i), *s);
				assert(j != RANGE_FAIL);
				if (j == RANGE_ERROR) {
					assert(*s == '[');
					break;
				}
				i += j;
			    }
			    /* FALLTHROUGH */
			    case '?':
				result = mklist(mkstr(str("%c", *s)), result);
				break;
			    default:
				break;
			}
		}
	}

	return result;
}

/*
 * extractmatches
 *
 *	Compare subject and patterns like listmatch().  For all subjects
 *	that match a pattern, return the wildcarded portions of the
 *	subjects as the result.
 */

extern List *extractmatches(List *subjects, List *patterns, StrList *quotes) {
	List **prevp;
	List *subject;
	Ref(List *, result, NULL);
	prevp = &result;

	gcdisable();

	for (subject = subjects; subject != NULL; subject = subject->next) {
		List *pattern;
		StrList *quote;
		for (pattern = patterns, quote = quotes;
		     pattern != NULL;
		     pattern = pattern->next, quote = quote->next) {
			List *match;
			char *pat = getstr(pattern->term);
			match = extractsinglematch(getstr(subject->term),
						   pat, quote->str, NULL);
			if (match != NULL) {
				/* match is returned backwards, so reverse it */
				match = reverse(match);
				for (*prevp = match; match != NULL; match = *prevp)
					prevp = &match->next;
				break;
			}
		}
	}

	gcenable();
	RefReturn(result);
}
