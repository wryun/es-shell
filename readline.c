/* prim-readline.c -- readline primitives */

#include "es.h"
#include "prim.h"

#if HAVE_READLINE

#include <readline/history.h>

Boolean reloadhistory = FALSE;
static char *history;

#if 0
/* These split history file entries by timestamp, which allows readline to pick up
 * multi-line commands correctly across process boundaries.  Disabled by default,
 * because it leaves the history file itself kind of ugly. */
static int history_write_timestamps = 1;
static char history_comment_char = '#';
#endif

/*
 * history management
 */

extern void setmaxhistorylength(int len) {
	static int currenthistlen = -1; /* unlimited */
	if (len != currenthistlen) {
		switch (len) {
		case -1:
			unstifle_history();
			break;
		case 0:
			clear_history();
			FALLTHROUGH;
		default:
			stifle_history(len);
		}
		currenthistlen = len;
	}
}

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

static void reload_history(void) {
	/* Attempt to populate readline history with new history file. */
	if (history != NULL)
		read_history(history);
	using_history();

	reloadhistory = FALSE;
}

extern void sethistory(char *file) {
	if (reloadhistory)
		reload_history();
	reloadhistory = TRUE;
	history = file;
}

extern void checkreloadhistory(void) {
	if (reloadhistory)
		reload_history();
}

/*
 * primitives
 */

PRIM(sethistory) {
	if (list == NULL) {
		sethistory(NULL);
		return NULL;
	}
	Ref(List *, lp, list);
	sethistory(getstr(lp->term));
	RefReturn(lp);
}

PRIM(writehistory) {
	if (list == NULL || list->next != NULL)
		fail("$&writehistory", "usage: $&writehistory command");
	loghistory(getstr(list->term));
	return NULL;
}

PRIM(setmaxhistorylength) {
	char *s;
	int n;
	if (list == NULL) {
		setmaxhistorylength(-1); /* unlimited */
		return NULL;
	}
	if (list->next != NULL)
		fail("$&setmaxhistorylength", "usage: $&setmaxhistorylength [limit]");
	Ref(List *, lp, list);
	n = (int)strtol(getstr(lp->term), &s, 0);
	if (n < 0 || (s != NULL && *s != '\0'))
		fail("$&setmaxhistorylength", "max-history-length must be set to a positive integer");
	setmaxhistorylength(n);
	RefReturn(lp);
}

PRIM(resetterminal) {
	resetterminal = TRUE;
	return ltrue;
}

/*
 * initialization
 */

extern Dict *initprims_readline(Dict *primdict) {
	X(sethistory);
	X(writehistory);
	X(resetterminal);
	X(setmaxhistorylength);
	return primdict;
}

/* inithistory -- called at dawn of time from main() */
extern void inithistory(void) {
	/* declare the global roots */
	globalroot(&history);		/* history file */
}

#endif
