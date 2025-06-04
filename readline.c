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
 * history
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

/* dequote -- teach es how to dequote a word */
static char *dequote(char *text, int quote_char) {
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

typedef enum {
	NORMAL,
	SYNTAX_ERROR,
	FDBRACES
} CompletionType;

/* hmm. */
extern const char nw[];

/* Scan line back to its start. */
/* This is a lot of code, and a poor reimplementation of the parser. :( */
CompletionType boundcmd(char **start) {
	char *line = rl_line_buffer;
	char syntax[128] = { 0 };
	int lp, sp = 0;
	Boolean quote = FALSE, first_word = TRUE;

	for (lp = rl_point; lp > 0; lp--) {
		if (quote)
			continue;

		switch (line[lp]) {
		/* quotes. pretty easy */
		case '\'':
			quote = !quote;
			continue;

		/* "stackable" syntax.  remember, we're moving backwards */
		case '}':
			syntax[sp++] = '{';
			break;
		case '{':
			if (sp == 0) {
				*start = rl_line_buffer + lp + 1;
				return NORMAL;
			}
			if (syntax[--sp] != '{') {
				*start = rl_line_buffer;
				return SYNTAX_ERROR;
			}
			break;
		case ')':
			syntax[sp++] = '(';
			break;
		case '(':
			if (sp > 0) {
				if (syntax[--sp] != '(') {
					*start = rl_line_buffer;
					return SYNTAX_ERROR;
				}
			} else {
				/* TODO: make `<=(a b` work */
				first_word = TRUE;
			}
			break;

		/* command separator chars */
		case ';':
			if (sp == 0) {
				*start = rl_line_buffer + lp + 1;
				return NORMAL;
			}
			break;
		case '&':
			if (sp == 0) {
				*start = rl_line_buffer + lp + 1;
				return NORMAL;
			}
			break;
		case '|':
			if (sp == 0) {
				int pp = lp+1;
				Boolean inbraces = FALSE;
				if (pp < rl_point && line[pp] == '[') {
					inbraces = TRUE;
					while (pp < rl_point) {
						if (line[pp++] == ']') {
							inbraces = FALSE;
							break;
						}
					}
				}
				*start = rl_line_buffer + pp;
				return inbraces ? FDBRACES : NORMAL;
			}
			break;
		case '`':
			if (first_word) {
				*start = rl_line_buffer + lp + 1;
				return NORMAL;
			}
			break;
		case '<':
			if (first_word && lp < rl_point - 1 && line[lp+1] == '=') {
				*start = rl_line_buffer + lp + 2;
				return NORMAL;
			}
			break;
		}
		if (nw[(unsigned char)line[lp]])
			first_word = FALSE;
	}
	/* TODO: fetch previous lines if sp > 0 */
	*start = rl_line_buffer;
	return NORMAL;
}


/* calls `%complete prefix word` to get a list of candidates for how to complete
 * `word`.
 *
 * TODO: improve argv for %complete
 *  - special dispatch for special syntax
 *  - split up args in a syntax-aware way
 *  - dequote args before and requote after (already done, just do it better)
 *  - skip/handle "command-irrelevant" syntax
 *      ! redirections binders
 *  - MAYBE: provide raw command/point?
 *
 * all the new behaviors above should ideally be done "manually", so that %complete
 * can be used the same way without worrying about the line editing library.
 *
 * Handle the following properly, though maybe not in this function
 *   `let (a =`
 *   `let (a = b)`
 *   `a =`
 *   `a > `
 *   `!`
 *   `$(f`
 */

static List *callcomplete(char *word) {
	int len;
	char *start;
	CompletionType type;

	Ref(List *, result, NULL);
	Ref(List *, fn, NULL);
	if ((fn = varlookup("fn-%complete", NULL)) == NULL) {
		RefPop(fn);
		return NULL;
	}
	type = boundcmd(&start);

	if (type == FDBRACES) {
		/* TODO: fd completion */
		RefPop2(result, fn);
		return NULL;
	}

	len = rl_point - (start - rl_line_buffer) - strlen(word);
	if (len < 0) {	/* TODO: fix `word` for `|[2]` and delete this hack */
		len = 0;
		word = "";
	}
	Ref(char *, line, gcndup(start, len));
	gcdisable();
	fn = append(fn, mklist(mkstr(line),
				mklist(mkstr(str("%s", word)), NULL)));
	gcenable();
	result = eval(fn, NULL, 0);
	RefEnd2(line, fn);
	RefReturn(result);
}

/* calls 'callcomplete' to produce candidates, and then returns them in a way
 * readline likes. */
static char *completion_matches(const char *text, int state) {
	static char **matches = NULL;
	static int matches_idx, matches_len;
	int rlen;
	char *result;

	if (!state) {
		Vector *vm = vectorize(callcomplete((char *)text));
		matches = vm->vector;
		matches_len = vm->count;
		matches_idx = 0;
	}

	if (!matches || matches_idx >= matches_len)
		return NULL;

	rlen = strlen(matches[matches_idx]);
	result = ealloc(rlen + 1);
	strcpy(result, matches[matches_idx]);
	result[rlen] = '\0';

	matches_idx++;
	return result;
}

/* calls out to get candidates, and manages the tools to present those candidates
 * correctly.
 * TODO:
 *  - Hook function so completers can not only say "are these candidates files?"
 *    but also "how to get to the file from these candidates?" (for e.g.,
 *    pathsearch-y commands)
 */
char **es_completion(UNUSED const char *text, UNUSED int start, UNUSED int end) {
	char **matches;
	Push caf;
	varpush(&caf, "completions-are-filenames", NULL);

	matches = rl_completion_matches(text, completion_matches);

	/* mechanisms to control how the results are presented */
	rl_filename_completion_desired = istrue(varlookup("completions-are-filenames", NULL));
	rl_attempted_completion_over = 1;	/* suppress "default" completions */

	varpop(&caf);
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
	rl_filename_dequoting_function = dequote;
}

/* set up readline for the next call */
extern void rlsetup(void) {
	static Boolean initialized = FALSE;
	if (!initialized) {
		initreadline();
		initialized = TRUE;
	}

	rl_attempted_completion_function = es_completion;

	if (reloadhistory)
		reload_history();
	if (resetterminal) {
		rl_reset_terminal(NULL);
		resetterminal = FALSE;
	}
	if (RL_ISSTATE(RL_STATE_INITIALIZED))
		rl_reset_screen_size();
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

static char *emptyprompt = "";

PRIM(readline) {
	char *line;
	char *pr0 = list == NULL ? "" : getstr(list->term);
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
