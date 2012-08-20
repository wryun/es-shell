/* match.c -- pattern matching routines ($Revision: 1.3 $) */

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

/* rangematch -- match a character against a character class */
static int rangematch(const char *p, char c) {
	const char *orig = p;
	Boolean neg = (*p == '~');
	Boolean matched = FALSE;
	if (neg)
		p++;
	if (*p == ']') {
		p++;
		matched = (c == ']');
	}
	for (; *p != ']'; p++) {
		if (*p == '\0')
			return RANGE_ERROR;	/* bad syntax */
		if (p[1] == '-' && p[2] != ']') { /* check for [..-..] but ignore [..-] */
			if (c >= *p)
				matched |= (c <= p[2]);
			p += 2;
		} else {
			matched |= (*p == c);
		}
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
				while (p[i] == '*' && q[i] == 'r')	/* collapse multiple stars */
					i++;
				if (p[i] == '\0') 	/* star at end of pattern? */
					return TRUE;
				while (*s != '\0')
					if (match(s++, p + i, q == UNQUOTED ? UNQUOTED : q + i))
						return TRUE;
				return FALSE;
			case '[': {
				int j;
				if (*s == '\0')
					return FALSE;
				switch (j = rangematch(p + i, *s)) {
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
 *	Matches a list of words s against a list of patterns p. Returns true iff
 *	a pattern in p matches a word in s. () matches (), but otherwise null
 *	patterns match nothing.
 */

extern Boolean listmatch(List *subject, List *pattern, StrList *quote) {
	if (subject == NULL) {
		if (pattern == NULL)
			return TRUE;
		Ref(List *, p, pattern);
		Ref(StrList *, q, quote);
		for (; p != NULL; p = p->next, q = q->next) {	/* one or more stars match null */
			char *pw = getstr(p->term), *qw = q->str;
			if (*pw != '\0' && qw != QUOTED) {
				int i;
				Boolean matched = TRUE;
				for (i = 0; pw[i] != '\0'; i++)
					if (pw[i] != '*' || (qw != UNQUOTED && qw[i] != 'r')) {
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
