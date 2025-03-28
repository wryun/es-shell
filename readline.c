/* prim-readline.c -- readline primitives */

#include "es.h"
#include "prim.h"

#if HAVE_READLINE

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

static Boolean reloadhistory = FALSE;
static Boolean resetterminal = FALSE;
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

/*
 * readey liney
 */

/* quote -- teach readline how to quote a word in es during completion */
static char *quote(char *text, int type, char *qp) {
	char *p, *r;

	/* worst-case size: string is 100% quote characters which will all be
	 * doubled, plus initial and final quotes and \0 */
	p = r = ealloc(strlen(text) * 2 + 3);
	/* supply opening quote if not already present */
	if (*qp != '\'')
		*p++ = '\'';
	while (*text) {
		/* double any quotes for es quote-escaping rules */
		if (*text == '\'')
			*p++ = '\'';
		*p++ = *text++;
	}
	if (type == SINGLE_MATCH)
		*p++ = '\'';
	*p = '\0';
	return r;
}

/* unquote -- teach es how to unquote a word */
static char *unquote(char *text, int quote_char) {
	char *p, *r;

	p = r = ealloc(strlen(text) + 1);
	while (*text) {
		*p++ = *text++;
		if (quote_char && *(text - 1) == '\'' && *text == '\'')
			++text;
	}
	*p = '\0';
	return r;
}

static char *complprefix;
static List *(*wordslistgen)(char *);

static char *list_completion_function(const char *text, int state) {
	static char **matches = NULL;
	static int matches_idx, matches_len;
	int i, rlen;
	char *result;

	const int pfx_len = strlen(complprefix);

	if (!state) {
		const char *name = &text[pfx_len];

		Vector *vm = vectorize(wordslistgen((char *)name));
		matches = vm->vector;
		matches_len = vm->count;
		matches_idx = 0;
	}

	if (!matches || matches_idx >= matches_len)
		return NULL;

	rlen = strlen(matches[matches_idx]);
	result = ealloc(rlen + pfx_len + 1);
	for (i = 0; i < pfx_len; i++)
		result[i] = complprefix[i];
	strcpy(&result[pfx_len], matches[matches_idx]);
	result[rlen + pfx_len] = '\0';

	matches_idx++;
	return result;
}

char **builtin_completion(const char *text, int UNUSED start, int UNUSED end) {
	char **matches = NULL;

	if (*text == '$') {
		wordslistgen = varswithprefix;
		complprefix = "$";
		switch (text[1]) {
		case '&':
			wordslistgen = primswithprefix;
			complprefix = "$&";
			break;
		case '^': complprefix = "$^"; break;
		case '#': complprefix = "$#"; break;
		}
		matches = rl_completion_matches(text, list_completion_function);
	}

	/* ~foo => username.  ~foo/bar already gets completed as filename. */
	if (!matches && *text == '~' && !strchr(text, '/'))
		matches = rl_completion_matches(text, rl_username_completion_function);

	return matches;
}

static List *cmdcomplete(char *prefix) {
	List *fn = varlookup("fn-%complete", NULL);
	if (fn == NULL)
		return NULL;
	gcdisable();
	fn = append(fn, mklist(mkstr(str("%s", prefix)), NULL));
	gcenable();
	return eval(fn, NULL, 0);
}

char **es_completion(UNUSED const char *text, UNUSED int start, UNUSED int end) {
	char **matches;

	complprefix = "";
	wordslistgen = cmdcomplete;

	matches = rl_completion_matches(text, list_completion_function);

	rl_attempted_completion_over = 1;	/* suppress "default" completions */
	return matches;
}

static void initreadline(void) {
	rl_readline_name = "es";

	/* these two word_break_characters exclude '&' due to primitive completion */
	rl_completer_word_break_characters = " \t\n\\'`$><=;|{()}";
	rl_basic_word_break_characters = " \t\n\\'`$><=;|{()}";
	rl_completer_quote_characters = "'";
	rl_special_prefixes = "$";

	rl_filename_quote_characters = " \t\n\\`'$><=;|&{()}";
	rl_filename_quoting_function = quote;
	rl_filename_dequoting_function = unquote;
}

/* set up readline for the next call */
extern void rlsetup(void) {
	static Boolean initialized = FALSE;
	if (!initialized) {
		initreadline();
		initialized = TRUE;
	}

	rl_attempted_completion_function = builtin_completion;
	/* rl_attempted_completion_function = es_completion; */

	if (reloadhistory)
		reload_history();
	if (resetterminal) {
		rl_reset_terminal(NULL);
		resetterminal = FALSE;
	}
	if (RL_ISSTATE(RL_STATE_INITIALIZED))
		rl_reset_screen_size();
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
	return ltrue;
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

static char *callreadline(char *prompt) {
	char *r, *volatile line = NULL;
	/* should this be called after each interruption, or? */
	rlsetup();
	interrupted = FALSE;
	if (!setjmp(slowlabel)) {
		slow = TRUE;
		r = interrupted ? NULL : readline(prompt);
		if (interrupted)
			errno = EINTR;
	} else {
		r = NULL;
		errno = EINTR;
	}
	slow = FALSE;
	if (r != NULL) {
		line = str("%s", r);
		efree(r);
	}
	SIGCHK();
	return line;
}

static char *emptyprompt = "";

PRIM(readline) {
	char *line;
	char *pr0 = getstr(list->term);
	char *prompt = emptyprompt;
	int old = dup(0), in = fdmap(0);

	if (list != NULL) {
		size_t psize = strlen(pr0) * sizeof(char) + 1;
		prompt = ealloc(psize);
		memcpy(prompt, pr0, psize);
	}
	if (dup2(in, 0) == -1) {
		if (prompt != emptyprompt)
			efree(prompt);
		fail("$&readline", "dup2: %s", esstrerror(errno));
	}

	ExceptionHandler

		do {
			line = callreadline(prompt);
		} while (line == NULL && errno == EINTR);

	CatchException (e)

		if (prompt != emptyprompt)
			efree(prompt);
		mvfd(old, 0);
		throw(e);

	EndExceptionHandler

	if (prompt != emptyprompt)
		efree(prompt);
	mvfd(old, 0);

	if (line == NULL)
		return NULL;
	list = mklist(mkstr(line), NULL);
	return list;
}

/*
 * initialization
 */

extern Dict *initprims_readline(Dict *primdict) {
	X(sethistory);
	X(writehistory);
	X(resetterminal);
	X(setmaxhistorylength);
	X(readline);
	return primdict;
}

/* inithistory -- called at dawn of time from main() */
extern void inithistory(void) {
	/* declare the global roots */
	globalroot(&history);		/* history file */
}

#endif
