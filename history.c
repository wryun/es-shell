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

static char *history;
static int historyfd = -1;
static Buffer *histbuffer = NULL;

#if READLINE
extern void add_history(char *);
extern int read_history(char *);
extern void stifle_history(int);
#endif


/*
 * histbuffer -- build the history line during input and dump it as a gc-string
 */

extern Boolean pendinghistory() {
	return histbuffer != NULL;
}

extern void addhistbuf(char *line, size_t len) {
	if (line == NULL || len == 0)
		return;
	if (histbuffer == NULL)
		histbuffer = openbuffer(BUFSIZE);
	histbuffer = bufncat(histbuffer, line, len);
}

extern char *dumphistbuf() {
	char *s, *p;
        size_t len;
	if (histbuffer == NULL)
		return NULL;

	s = sealcountedbuffer(histbuffer);
	histbuffer = NULL;

	len = strlen(s);
	if (len > 0 && s[len - 1] == '\n')
		s[len - 1] = '\0';

	for (p = s; *p != '\0'; p++)
		switch(*p) {
		case '#': case '\n':	goto retnull;
		case ' ': case '\t':	break;
		default:		goto retreal;
		}
retnull:
	return NULL;
retreal:
	return s;
}

extern void discardhistbuf() {
	if (histbuffer == NULL)
		return;
	freebuffer(histbuffer);
	histbuffer = NULL;
}


/*
 * history file
 */

/* loghistory -- write the last command out to a file */
extern void loghistory(char *cmd) {
	size_t len;
	if (cmd == NULL)
		return;
#if READLINE
	add_history(cmd);
#endif
	if (history == NULL)
		return;
	if (historyfd == -1) {
		historyfd = eopen(history, oAppend);
		if (historyfd == -1) {
			eprint("history(%s): %s\n", history, esstrerror(errno));
			vardef("history", NULL, NULL);
			return;
		}
	}
	len = strlen(cmd);
	ewrite(historyfd, cmd, len);
	ewrite(historyfd, "\n", 1);
}

/* sethistory -- change the file for the history log */
extern void sethistory(char *file) {
	if (historyfd != -1) {
		close(historyfd);
		historyfd = -1;
	}
#if READLINE
	/* Attempt to populate readline history with new history file. */
	stifle_history(50000); /* Keep memory usage within sane-ish bounds. */
	read_history(file);
#endif
	history = file;
}


/*
 * initialization
 */

/* inithistory -- called at dawn of time from main() */
extern void inithistory(void) {
	/* declare the global roots */
	globalroot(&history);		/* history file */

	/* mark the historyfd as a file descriptor to hold back from forked children */
	registerfd(&historyfd, TRUE);
}
