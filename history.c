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

#if READLINE
extern void add_history(char *);
extern int read_history(char *);
extern void stifle_history(int);
extern int append_history(int, const char *);
extern void using_history(void);

static char *history;

#if 0
/* These split history file entries by timestamp, which allows readline to pick up
 * multi-line commands correctly across process boundaries.  Disabled by default,
 * because it leaves the history file itself kind of ugly. */
static int history_write_timestamps = 1;
static char history_comment_char = '#';
#endif
#endif


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


/*
 * initialization
 */

/* inithistory -- called at dawn of time from main() */
extern void inithistory(void) {
	/* declare the global roots */
	globalroot(&history);		/* history file */
	using_history();
}
#endif
