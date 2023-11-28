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
static char *history;

#if READLINE
extern void add_history(char *);
extern int read_history(char *);
extern void stifle_history(int);
extern int append_history(int, const char *);
extern void using_history(void);

#if 0
/* These split history file entries by timestamp, which allows readline to pick up
 * multi-line commands correctly across process boundaries.  Disabled by default,
 * because it leaves the history file itself kind of ugly. */
static int history_write_timestamps = 1;
static char history_comment_char = '#';
#endif
#else
static int historyfd = -1;
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

#if READLINE
extern void loghistory(char *cmd) {
	int err;
	if (cmd == NULL)
		return;
	add_history(cmd);
	if (history == NULL)
		return;

	if ((err = append_history(1, history))) {
		eprint("history(%s): %s\n", history, esstrerror(err));
		vardef("history", NULL, NULL);
	}
}

extern void sethistory(char *file) {
	/* Attempt to populate readline history with new history file. */
	stifle_history(50000); /* Keep memory usage within sane-ish bounds. */
	read_history(file);
	history = file;
}
#else /* !READLINE */
extern void loghistory(char *cmd) {
	size_t len;
	if (cmd == NULL || history == NULL)
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

extern void sethistory(char *file) {
	if (historyfd != -1) {
		close(historyfd);
		historyfd = -1;
	}
	history = file;
}
#endif


/*
 * initialization
 */

/* inithistory -- called at dawn of time from main() */
extern void inithistory(void) {
	/* declare the global roots */
	globalroot(&history);		/* history file */

#if READLINE
	using_history();
#else
	/* mark the historyfd as a file descriptor to hold back from forked children */
	registerfd(&historyfd, TRUE);
#endif
}
