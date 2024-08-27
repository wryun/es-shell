/* split.c -- split strings based on separators ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

static Boolean coalesce;
static Boolean splitchars;
static Buffer *buffer;
static List *value;

static Boolean ifsvalid = FALSE;
static char ifs[10], isifs[256];

extern void startsplit(const char *sep, Boolean coalescef) {
	static Boolean initialized = FALSE;
	if (!initialized) {
		initialized = TRUE;
		globalroot(&value);
	}

	value = NULL;
	buffer = NULL;
	coalesce = coalescef;
	splitchars = !coalesce && *sep == '\0';

	if (!ifsvalid || !streq(sep, ifs)) {
		int c;
		if (strlen(sep) + 1 < sizeof ifs) {
			strcpy(ifs, sep);
			ifsvalid = TRUE;
		} else
			ifsvalid = FALSE;
		memzero(isifs, sizeof isifs);
		for (isifs['\0'] = TRUE; (c = (*(unsigned const char *)sep)) != '\0'; sep++)
			isifs[c] = TRUE;
	}
}

extern char *stepsplit(char *in, size_t len, Boolean endword) {
	Buffer *buf = buffer;
	unsigned char *s = (unsigned char *) in, *inend = s + len;

	if (splitchars) {
		Boolean end;
		Term *term;

		if (*s == '\0') return NULL;
		assert(buf == NULL);

		end = *(s + 1) == '\0';

		term = mkstr(gcndup((char *) s, 1));
		value = mklist(term, value);

		if (end) return NULL;
		return (char *) ++s;
	}

	if (!coalesce && buf == NULL)
		buf = openbuffer(0);

	while (s < inend) {
		int c = *s++;
		if (buf != NULL)
			if (isifs[c]) {
				Term *term = mkstr(sealcountedbuffer(buf));
				value = mklist(term, value);
				buffer = buf = coalesce ? NULL : openbuffer(0);
				return (char *) s;
			} else
				buf = bufputc(buf, c);
		else if (!isifs[c])
			buf = bufputc(openbuffer(0), c);
	}

	if (endword && buf != NULL) {
		Term *term = mkstr(sealcountedbuffer(buf));
		value = mklist(term, value);
		buf = NULL;
	}
	buffer = buf;
	return NULL;
}

extern void splitstring(char *in, size_t len, Boolean endword) {
	size_t remainder;
	char *s = in;
	do {
		remainder = len - (s - in);
		s = stepsplit(s, remainder, endword);
	} while (s != NULL);
}

extern List *endsplit(void) {
	List *result;

	if (buffer != NULL) {
		Term *term = mkstr(sealcountedbuffer(buffer));
		value = mklist(term, value);
		buffer = NULL;
	}
	result = reverse(value);
	value = NULL;
	return result;
}

extern List *fsplit(const char *sep, List *list, Boolean coalesce) {
	Ref(List *, lp, list);
	startsplit(sep, coalesce);
	for (; lp != NULL; lp = lp->next) {
		char *bs = getstr(lp->term), *s = bs;
		do {
			char *ns = getstr(lp->term);
			s = ns + (s - bs);
			bs = ns;
			s = stepsplit(s, strlen(s), TRUE);
		} while (s != NULL);
	}
	RefEnd(lp);
	return endsplit();
}
