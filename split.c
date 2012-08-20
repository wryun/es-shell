/* split.c -- split strings based on separators */

#include "es.h"
#include "gc.h"

#define	BUFSIZE	4096

#if ASSERTIONS
static Boolean splitting = FALSE;
#endif
static Boolean coalesce;
static Boolean inword;
static Buffer buffer;
static List *value;
static char *bufp, *bufend;

static Boolean ifsvalid = FALSE;
static char ifs[10], isifs[256];

extern void startsplit(const char *sep, Boolean coalescef) {
	static Boolean initialized = FALSE;
	if (!initialized) {
		initialized = TRUE;
		globalroot(&value);
	}

	assert(!splitting);
#if ASSERTIONS
	splitting = TRUE;
#endif
	assert(value == NULL);

	coalesce = coalescef;
	inword = FALSE;
	bufend = bufp = NULL;

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

extern void splitstring(char *in, size_t len, Boolean endword) {
	unsigned char *s, *inend;

	assert(splitting);
	s = (unsigned char *) in;
	inend = s + len;

	if (!coalesce && !inword) {
		inword = TRUE;
		buffer = openbuffer(BUFSIZE);
		bufp = buffer.str;
		bufend = bufp + buffer.len;
		assert(bufp + 1 < bufend);
	}

	while (s < inend) {
		int c = *s++;
		if (inword) {
			if (isifs[c]) {
				Term *term;
				*bufp++ = '\0';
				term = mkterm(sealbuffer(bufp), NULL);
				value = mklist(term, value);
				bufend = bufp = NULL;
				if (coalesce)
					inword = FALSE;
				else {
					buffer = openbuffer(BUFSIZE);
					bufp = buffer.str;
					bufend = bufp + buffer.len;
					assert(bufp + 1 < bufend);
				}
			} else {
				*bufp++ = c;
				if (bufp >= bufend) {
					buffer = expandbuffer(BUFSIZE);
					bufp = buffer.str;
					bufend = bufp + buffer.len;
					assert(bufp + 1 < bufend);
				}
			}
		} else if (!isifs[c]) {
			inword = TRUE;
			buffer = openbuffer(BUFSIZE);
			bufp = buffer.str;
			bufend = bufp + buffer.len;
			*bufp++ = c;
			assert(bufp + 1 < bufend);
		}
	}

	if (endword && inword) {
		Term *term;
		*bufp++ = '\0';
		term = mkterm(sealbuffer(bufp), NULL);
		value = mklist(term, value);
		inword = FALSE;
		bufend = bufp = NULL;
	}
}

extern List *endsplit(void) {
	List *result;

	assert(splitting);
#if ASSERTIONS
	splitting = FALSE;
#endif

	if (inword) {
		Term *term;
		*bufp++ = '\0';
		term = mkterm(sealbuffer(bufp), NULL);
		value = mklist(term, value);
		inword = FALSE;
		bufend = bufp = NULL;
	}
	result = reverse(value);
	value = NULL;
	return result;
}
