/* readline.c -- readline primitives */

#include "es.h"
#include "prim.h"

#if HAVE_READLINE

#include <readline/readline.h>
#include <readline/history.h>


/*
 * globals
 */

static Boolean reloadhistory = FALSE;
static Boolean resetterminal = FALSE;
static char *history = NULL;

#if 0
/* These split history file entries by timestamp, which allows readline to pick up
 * multi-line commands correctly across process boundaries.  Disabled by default,
 * because it leaves the history file itself kind of ugly. */
static int history_write_timestamps = 1;
static char history_comment_char = '#';
#endif


/*
 * history functions
 */

static int sethistorylength = -1; /* unlimited */

extern void setmaxhistorylength(int len) {
	sethistorylength = len;
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

static int count_history(void) {
	int i, n, count = 0, fd = eopen(history, oOpen);
	char buf[4096];
	if (fd < 0)
		return -1;
	while ((n = read(fd, &buf, 4096)) != 0) {
		if (n < 0) {
			if (errno == EINTR) {
				SIGCHK();
				continue;
			} else {
				close(fd);
				return -1;
			}
		}
		for (i = 0; i < n; i++)
			if (buf[i] == '\n')
				count++;
	}
	close(fd);
	return count;
}

static void reload_history(void) {
	/* Attempt to populate readline history with new history file. */
	if (history != NULL) {
		int n = count_history() - sethistorylength;
		if (sethistorylength < 0 || n < 0) n = 0;
		read_history_range(history, n, -1);
	}
	using_history();

	reloadhistory = FALSE;
}

static void inithistory(void) {
	static Boolean initialized = FALSE;
	if (initialized)
		return;
	globalroot(&history);
	initialized = TRUE;
}

extern void sethistory(char *file) {
	inithistory();
	if (reloadhistory)
		reload_history();
	reloadhistory = TRUE;
	history = file;
}

extern void checkhistory(void) {
	static int effectivelength = -1;
	if (reloadhistory)
		reload_history();
	if (sethistorylength != effectivelength) {
		switch (sethistorylength) {
		case -1:
			unstifle_history();
			break;
		case 0:
			clear_history();
			FALLTHROUGH;
		default:
			stifle_history(sethistorylength);
		}
		effectivelength = sethistorylength;
	}
}


/*
 * readline functions
 */

/* quote -- teach readline how to quote a word during completion.
 * prefix is prepended _before_ the quotes, such as: $'foo bar' */
static char *quote(char *text, Boolean open, char *prefix, char *qp) {
	char *quoted;
	if (*qp != '\0' || strpbrk(text, rl_filename_quote_characters)) {
		quoted = mprint("%s%#S", prefix, text);
		if (open)
			quoted[strlen(quoted)-1] = '\0';
	} else {
		quoted = mprint("%s%s", prefix, text);
	}
	efree(text);
	return quoted;
}

/* unquote -- remove quotes from text and point *qp at the relevant quote char */
static char *unquote(const char *text, char **qp) {
	char *p, *r;
	Boolean quoted = FALSE;

	p = r = ealloc(strlen(text) + 1);
	while ((*p = *text++)) {
		if (*p == '\'') {
			if (quoted && *text == '\'') {
				p++;
				text++;
			} else {
				quoted = !quoted;
				if (quoted && qp != NULL)
					*qp = p;
			}
		} else if (!quoted && *p == '\\') {
			/* anything else won't be handled correctly by the completer */
			if (*text == ' ' || *text == '\'')
				*p++ = *text++;
		} else
			p++;
	}
	*p = '\0';
	if (!quoted && qp != NULL)
		*qp = p;
	return r;
}

/* Unquote files to allow readline to detect which are directories. */
static int unquote_for_stat(char **name) {
	char *unquoted;
	if (!strpbrk(*name, rl_filename_quote_characters))
		return 0;

	unquoted = unquote(*name, NULL);
	efree(*name);
	*name = unquoted;
	return 1;
}

/* Find the start of the word to complete.  This uses the trick where we set rl_point
 * to the start of the word to indicate the start of the word.  For this to work,
 * rl_basic_quote_characters must be the empty string or else this function's result
 * is overwritten, and doing that means we have to reimplement basically all quoting
 * behavior manually. */
static char *completion_start(void) {
	int i, start = 0;
	Boolean quoted = FALSE, backslash = FALSE;
	for (i = 0; i < rl_point; i++) {
		char c = rl_line_buffer[i];
		if (backslash) {
			backslash = FALSE;
			continue;
		}
		if (c == '\'')
			quoted = !quoted;
		else if (!quoted && c == '\\')
			backslash = TRUE;
		else if (!quoted && strchr(rl_basic_word_break_characters, c))
			start = i; /* keep possible '$' char in term */
	}
	rl_point = start;
	return NULL;
}

/* Basic function to use an es List created by gen() to generate readline matches. */
static char *list_completion(const char *text, int state, List *(*gen)(const char *)) {
	static char **matches = NULL;
	static int i, len;

	if (!state) {
		Vector *vm = vectorize(gen(text));
		matches = vm->vector;
		len = vm->count;
		i = 0;
	}

	if (!matches || i >= len)
		return NULL;

	return mprint("%s", matches[i++]);
}

static char *var_completion(const char *text, int state) {
	return list_completion(text, state, varswithprefix);
}

static char *prim_completion(const char *text, int state) {
	return list_completion(text, state, primswithprefix);
}

static int matchcmp(const void *a, const void *b) {
	return strcoll(*(const char **)a, *(const char **)b);
}

/* Pick out a completion to perform based on the string's prefix */
rl_compentry_func_t *select_completion(const char *text, char **prefix) {
	if (*text == '$') {
		switch (text[1]) {
		case '&':
			*prefix = "$&";
			return prim_completion;
		case '^': *prefix = "$^"; break;
		case '#': *prefix = "$#"; break;
		default:  *prefix = "$";
		}
		return var_completion;
	} else if (*text == '~' && !strchr(text, '/')) {
		/* ~foo => username.  ~foo/bar gets completed as a filename. */
		return rl_username_completion_function;
	}
	return rl_filename_completion_function;
}

static rl_compentry_func_t *completion_func = NULL;

/* Top-level completion function.  If completion_func is set, performs that completion.
 * Otherwise, performs a completion based on the prefix of the text. */
char **builtin_completion(const char *text, int UNUSED start, int UNUSED end) {
	char **matches = NULL, *qp = NULL, *prefix = "";
	/* Manually unquote the text, since we told readline not to. */
	char *t = unquote(text, &qp);
	rl_compentry_func_t *completion;

	if (completion_func != NULL) {
		completion = completion_func;
		completion_func = NULL;
	} else
		completion = select_completion(text, &prefix);

	matches = rl_completion_matches(t+strlen(prefix), completion);

	/* Manually sort and then re-quote the matches. */
	if (matches != NULL) {
		size_t i, n;
		for (n = 1; matches[n]; n++)
			;
		qsort(&matches[1], n - 1, sizeof(matches[0]), matchcmp);
		matches[0] = quote(matches[0], n > 1, prefix, qp);
		for (i = 1; i < n; i++)
			matches[i] = quote(matches[i], FALSE, prefix, qp);
	}

	efree(t);

	/* Since we had to sort and quote results ourselves, we disable the automatic
	 * filename completion and sorting. */
	rl_attempted_completion_over = 1;
	rl_sort_completion_matches = 0;
	return matches;
}

/* Unquote matches when displaying in a menu.  This wouldn't be necessary, if not for
 * menu-complete. */
static void display_matches(char **matches, int num, int max) {
	int i;
	char **unquoted;

	if (rl_completion_query_items > 0 && num >= rl_completion_query_items) {
		int c;
		rl_crlf();
		fprintf(rl_outstream, "Display all %d possibilities? (y or n)", num);
		fflush(rl_outstream);
		c = rl_read_key();
		if (c != 'y' && c != 'Y' && c != ' ') {
			rl_crlf();
			rl_forced_update_display();
			return;
		}
	}

	unquoted = ealloc(sizeof(char *) * (num + 2));
	for (i = 0; matches[i]; i++)
		unquoted[i] = unquote(matches[i], NULL);
	unquoted[i] = NULL;

	rl_display_match_list(unquoted, num, max);
	rl_forced_update_display();

	for (i = 0; unquoted[i]; i++)
		efree(unquoted[i]);
	efree(unquoted);
}

static int es_complete_filename(int UNUSED count, int UNUSED key) {
	completion_func = rl_filename_completion_function;
	return rl_complete_internal(rl_completion_mode(es_complete_filename));
}

static int es_complete_variable(int UNUSED count, int UNUSED key) {
	completion_func = var_completion;
	return rl_complete_internal(rl_completion_mode(es_complete_variable));
}

static int es_complete_primitive(int UNUSED count, int UNUSED key) {
	completion_func = prim_completion;
	return rl_complete_internal(rl_completion_mode(es_complete_primitive));
}

static void initreadline(void) {
	rl_readline_name = "es";

	/* this word_break_characters excludes '&' due to primitive completion */
	rl_basic_word_break_characters = " \t\n`$><=;|{()}";
	rl_filename_quote_characters = " \t\n\\`'$><=;|&{()}";
	rl_basic_quote_characters = "";
	rl_special_prefixes = "$";

	rl_completion_word_break_hook = completion_start;
	rl_filename_stat_hook = unquote_for_stat;
	rl_attempted_completion_function = builtin_completion;
	rl_completion_display_matches_hook = display_matches;

	rl_add_funmap_entry("es-complete-filename", es_complete_filename);
	rl_add_funmap_entry("es-complete-variable", es_complete_variable);
	rl_add_funmap_entry("es-complete-primitive", es_complete_primitive);
	rl_bind_keyseq("\033/", es_complete_filename);
	rl_bind_keyseq("\033$", es_complete_variable);
}

static void prepreadline(void) {
	static Boolean initialized = FALSE;
	if (!initialized) {
		initreadline();
		initialized = TRUE;
	}
	checkhistory();
	if (resetterminal) {
		rl_reset_terminal(NULL);
		resetterminal = FALSE;
	}
	if (RL_ISSTATE(RL_STATE_INITIALIZED))
		rl_reset_screen_size();
}

/* callreadline -- readline wrapper */
static char *callreadline(char *prompt0) {
	char *r;
	Ref(char *volatile, prompt, prompt0);
	prepreadline();
	if (prompt == NULL)
		prompt = ""; /* bug fix for readline 2.0 */
	if (!sigsetjmp(slowlabel, 1)) {
		slow = TRUE;
		r = readline(prompt);
	} else {
		r = NULL;
		errno = EINTR;
	}
	slow = FALSE;
	SIGCHK();
	RefEnd(prompt);
	return r;
}

static FILE *fdmapopen(int fd, const char *mode) {
	FILE *f;
	if ((fd = dup(fdmap(fd))) == -1)
		fail("$&readline", "dup: %s", esstrerror(errno));
	if ((f = fdopen(fd, mode)) == NULL) {
		int err = errno;
		close(fd);
		fail("$&readline", "fdopen: %s", esstrerror(err));
	}
	return f;
}


/*
 * primitive interface
 */

PRIM(readline) {
	char *line;
	char *prompt = (list == NULL ? "" : getstr(list->term));
	if (list != NULL && list->next != NULL)
		fail("$&readline", "usage: %read-line [prompt]");

	if (!isatty(fdmap(0))) {
		list = prim("read", NULL, 0);
		if (length(list) <= 1)
			return list;
		return mklist(mkstr(str("%L", list, "")), NULL);
	}

	rl_instream = fdmapopen(0, "r");
	ExceptionHandler
		rl_outstream = fdmapopen(2, "w");
	CatchException (e)
		fclose(rl_instream);
		throw(e);
	EndExceptionHandler

	ExceptionHandler

		do {
			line = callreadline(prompt);
		} while (line == NULL && errno == EINTR);

	CatchException (e)

		fclose(rl_instream);
		fclose(rl_outstream);
		throw(e);

	EndExceptionHandler

	fclose(rl_instream);
	fclose(rl_outstream);

	if (line == NULL)
		return NULL;
	list = mklist(mkstr(str("%s", line)), NULL);
	efree(line);
	return list;
}

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

extern Dict *initprims_readline(Dict *primdict) {
	X(readline);
	X(sethistory);
	X(writehistory);
	X(resetterminal);
	X(setmaxhistorylength);
	return primdict;
}
#endif
