/* history.c -- control the history file ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"
#include "input.h"


/*
 * constants
 */

#define	BUFSIZE		((size_t) 4096)		/* buffer size to fill reads into */


/*
 * globals
 */

static Buffer *histbuffer = NULL;


/*
 * histbuffer -- build the history line during input and dump it as a gc-string
 */

extern void newhistbuffer() {
	assert(histbuffer == NULL);
	histbuffer = openbuffer(BUFSIZE);
}

extern void addhistbuffer(char c) {
	if (histbuffer == NULL)
		return;
	histbuffer = bufputc(histbuffer, c);
}

extern char *dumphistbuffer() {
	char *s;
        size_t len;
	assert(histbuffer != NULL);

	s = sealcountedbuffer(histbuffer);
	histbuffer = NULL;

	len = strlen(s);
	if (len > 0 && s[len - 1] == '\n')
		s[len - 1] = '\0';
	return s;
}
