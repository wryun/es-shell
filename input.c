/* input.c -- read input from files or strings ($Revision: 1.2 $) */

#define	REQUIRE_STAT	1

#include "es.h"
#include "input.h"

/*
 * constants
 */

#define	BUFSIZE		((size_t) 4096)		/* buffer size to fill reads into */


/*
 * macros
 */

#define	ISEOF(in)	((in)->fill == eoffill)


/*
 * globals
 */

Input *input;
char *prompt, *prompt2;

Boolean ignoreeof = FALSE;
Boolean resetterminal = FALSE;

#if HAVE_READLINE
#include <readline/readline.h>
#endif


/*
 * errors and warnings
 */

/* locate -- identify where an error came from */
static const char *locate(Input *in, const char *s) {
	return (in->runflags & run_interactive)
		? s
		: str("%s:%d: %s", in->name, in->lineno, s);
}

static const char *error = NULL;

/* yyerror -- yacc error entry point */
extern void yyerror(const char *s) {
#if sgi
	/* this is so that trip.es works */
	if (streq(s, "Syntax error"))
		s = "syntax error";
#endif
	if (error == NULL)	/* first error is generally the most informative */
		error = locate(input, s);
}

/* warn -- print a warning */
static void warn(char *s) {
	eprint("warning: %s\n", locate(input, s));
}


/*
 * unget -- character pushback
 */

/* ungetfill -- input->fill routine for ungotten characters */
static int ungetfill(Input *in) {
	int c;
	assert(in->ungot > 0);
	c = in->unget[--in->ungot];
	if (in->ungot == 0) {
		assert(in->rfill != NULL);
		in->fill = in->rfill;
		in->rfill = NULL;
		assert(in->rbuf != NULL);
		in->buf = in->rbuf;
		in->rbuf = NULL;
	}
	return c;
}

/* unget -- push back one character */
extern void unget(Input *in, int c) {
	if (in->ungot > 0) {
		assert(in->ungot < MAXUNGET);
		in->unget[in->ungot++] = c;
	} else {
		assert(in->rfill == NULL);
		in->rfill = in->fill;
		in->fill = ungetfill;
		assert(in->rbuf == NULL);
		in->rbuf = in->buf;
		in->buf = in->bufend;
		assert(in->ungot == 0);
		in->ungot = 1;
		in->unget[0] = c;
	}
}


/*
 * getting characters
 */

/* get -- get a character, filter out nulls */
static int get(Input *in) {
	int c;
	Boolean uf = (in->fill == ungetfill);
	while ((c = (in->buf < in->bufend ? *in->buf++ : (*in->fill)(in))) == '\0')
		warn("null character ignored");
	if (!uf && c != EOF)
		addhistbuffer((char)c);
	return c;
}

/* getverbose -- get a character, print it to standard error */
static int getverbose(Input *in) {
	if (in->fill == ungetfill)
		return get(in);
	else {
		int c = get(in);
		if (c != EOF) {
			char buf = c;
			ewrite(2, &buf, 1);
		}
		return c;
	}
}

/* eoffill -- report eof when called to fill input buffer */
static int eoffill(Input UNUSED *in) {
	assert(in->fd == -1);
	return EOF;
}

#if HAVE_READLINE
/* callreadline -- readline wrapper */
static char *callreadline(char *prompt0) {
	char *r;
	Ref(char *volatile, prompt, prompt0);
	if (prompt == NULL)
		prompt = ""; /* bug fix for readline 2.0 */
	checkreloadhistory();
	if (resetterminal) {
		rl_reset_terminal(NULL);
		resetterminal = FALSE;
	}
	if (RL_ISSTATE(RL_STATE_INITIALIZED))
		rl_reset_screen_size();
	if (!setjmp(slowlabel)) {
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
#endif


/* fdfill -- fill input buffer by reading from a file descriptor */
static int fdfill(Input *in) {
	long nread;
	assert(in->buf == in->bufend);
	assert(in->fd >= 0);

#if HAVE_READLINE
	if (in->runflags & run_interactive && in->fd == 0) {
		char *rlinebuf = NULL;
		do {
			rlinebuf = callreadline(prompt);
		} while (rlinebuf == NULL && errno == EINTR);
		if (rlinebuf == NULL)
			nread = 0;
		else {
			nread = strlen(rlinebuf) + 1;
			if (in->buflen < (unsigned int)nread) {
				while (in->buflen < (unsigned int)nread)
					in->buflen *= 2;
				in->bufbegin = erealloc(in->bufbegin, in->buflen);
			}
			memcpy(in->bufbegin, rlinebuf, nread - 1);
			in->bufbegin[nread - 1] = '\n';
			efree(rlinebuf);
		}
	} else
#endif
	do {
		nread = read(in->fd, (char *) in->bufbegin, in->buflen);
		SIGCHK();
	} while (nread == -1 && errno == EINTR);

	if (nread <= 0) {
		if (!ignoreeof) {
			close(in->fd);
			in->fd = -1;
			in->fill = eoffill;
			in->runflags &= ~run_interactive;
		}
		if (nread == -1)
			fail("$&parse", "%s: %s", in->name == NULL ? "es" : in->name, esstrerror(errno));
		return EOF;
	}

	in->buf = in->bufbegin;
	in->bufend = &in->buf[nread];
	return *in->buf++;
}


/*
 * the input loop
 */

/* parse -- call yyparse(), but disable garbage collection and catch errors */
extern Tree *parse(char *pr1, char *pr2) {
	int result;
	assert(error == NULL);

	inityy();
	emptyherequeue();

	if (ISEOF(input))
		throw(mklist(mkstr("eof"), NULL));

#if HAVE_READLINE
	prompt = (pr1 == NULL) ? "" : pr1;
#else
	if (pr1 != NULL)
		eprint("%s", pr1);
#endif
	prompt2 = pr2;

	result = yyparse();

	if (result || error != NULL) {
		assert(error != NULL);
		Ref(const char *, e, error);
		error = NULL;
		pseal(NULL);
		fail("$&parse", "%s", e);
		RefEnd(e);
	}

#if LISPTREES
	Ref(Tree *, pt, pseal(parsetree));
	if (input->runflags & run_lisptrees)
		eprint("%B\n", pt);
	RefReturn(pt);
#else
	return pseal(parsetree);
#endif

}

/* resetparser -- clear parser errors in the signal handler */
extern void resetparser(void) {
	error = NULL;
}

/* runinput -- run from an input source */
extern List *runinput(Input *in, int runflags) {
	volatile int flags = runflags;
	List * volatile result = NULL;
	List *repl, *dispatch;
	Push push;
	const char *dispatcher[] = {
		"fn-%eval-noprint",
		"fn-%eval-print",
		"fn-%noeval-noprint",
		"fn-%noeval-print",
	};

	flags &= ~eval_inchild;
	in->runflags = flags;
	in->get = (flags & run_echoinput) ? getverbose : get;
	in->prev = input;
	input = in;

	ExceptionHandler

		dispatch
	          = varlookup(dispatcher[((flags & run_printcmds) ? 1 : 0)
					 + ((flags & run_noexec) ? 2 : 0)],
			      NULL);
		if (flags & eval_exitonfalse) {
			dispatch = mklist(mkstr("%exit-on-false"), dispatch);
			flags &= ~eval_exitonfalse;
		}
		varpush(&push, "fn-%dispatch", dispatch);

		repl = varlookup((flags & run_interactive)
				   ? "fn-%interactive-loop"
				   : "fn-%batch-loop",
				 NULL);
		result = (repl == NULL)
				? prim("batchloop", NULL, NULL, flags)
				: eval(repl, NULL, flags);

		varpop(&push);

	CatchException (e)

		(*input->cleanup)(input);
		input = input->prev;
		throw(e);

	EndExceptionHandler

	input = in->prev;
	(*in->cleanup)(in);
	return result;
}


/*
 * pushing new input sources
 */

/* fdcleanup -- cleanup after running from a file descriptor */
static void fdcleanup(Input *in) {
	unregisterfd(&in->fd);
	if (in->fd != -1)
		close(in->fd);
	efree(in->bufbegin);
}

/* runfd -- run commands from a file descriptor */
extern List *runfd(int fd, const char *name, int flags) {
	Input in;
	List *result;

	memzero(&in, sizeof (Input));
	in.lineno = 1;
	in.fill = fdfill;
	in.cleanup = fdcleanup;
	in.fd = fd;
	registerfd(&in.fd, TRUE);
	in.buflen = BUFSIZE;
	in.bufbegin = in.buf = ealloc(in.buflen);
	in.bufend = in.bufbegin;
	in.name = (name == NULL) ? str("fd %d", fd) : name;

	RefAdd(in.name);
	result = runinput(&in, flags);
	RefRemove(in.name);

	return result;
}

/* stringcleanup -- cleanup after running from a string */
static void stringcleanup(Input *in) {
	efree(in->bufbegin);
}

/* stringfill -- placeholder than turns into EOF right away */
static int stringfill(Input *in) {
	in->fill = eoffill;
	return EOF;
}

/* runstring -- run commands from a string */
extern List *runstring(const char *str, const char *name, int flags) {
	Input in;
	List *result;
	unsigned char *buf;

	assert(str != NULL);

	memzero(&in, sizeof (Input));
	in.fd = -1;
	in.lineno = 1;
	in.name = (name == NULL) ? str : name;
	in.fill = stringfill;
	in.buflen = strlen(str);
	buf = ealloc(in.buflen + 1);
	memcpy(buf, str, in.buflen);
	in.bufbegin = in.buf = buf;
	in.bufend = in.buf + in.buflen;
	in.cleanup = stringcleanup;

	RefAdd(in.name);
	result = runinput(&in, flags);
	RefRemove(in.name);
	return result;
}

/* parseinput -- turn an input source into a tree */
extern Tree *parseinput(Input *in) {
	Tree * volatile result = NULL;

	in->prev = input;
	in->runflags = 0;
	in->get = get;
	input = in;

	ExceptionHandler
		result = parse(NULL, NULL);
		if (get(in) != EOF)
			fail("$&parse", "more than one value in term");
	CatchException (e)
		(*input->cleanup)(input);
		input = input->prev;
		throw(e);
	EndExceptionHandler

	input = in->prev;
	(*in->cleanup)(in);
	return result;
}

/* parsestring -- turn a string into a tree; must be exactly one tree */
extern Tree *parsestring(const char *str) {
	Input in;
	Tree *result;
	unsigned char *buf;

	assert(str != NULL);

	/* TODO: abstract out common code with runstring */

	memzero(&in, sizeof (Input));
	in.fd = -1;
	in.lineno = 1;
	in.name = str;
	in.fill = stringfill;
	in.buflen = strlen(str);
	buf = ealloc(in.buflen + 1);
	memcpy(buf, str, in.buflen);
	in.bufbegin = in.buf = buf;
	in.bufend = in.buf + in.buflen;
	in.cleanup = stringcleanup;

	RefAdd(in.name);
	result = parseinput(&in);
	RefRemove(in.name);
	return result;
}

/* isinteractive -- is the innermost input source interactive? */
extern Boolean isinteractive(void) {
	return input == NULL ? FALSE : ((input->runflags & run_interactive) != 0);
}

/* isfromfd -- is the innermost input source reading from a file descriptor? */
extern Boolean isfromfd(void) {
	return input == NULL ? FALSE : (input->fill == fdfill);
}


/*
 * readline integration.
 */
#if HAVE_READLINE
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

static int unquote_for_stat(char **name) {
	if (!strpbrk(*name, rl_filename_quote_characters))
		return 0;

	char *unquoted = unquote(*name, NULL);
	efree(*name);
	*name = unquoted;
	return 1;
}

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

static int matchcmp(const void *a, const void *b) {
	return strcoll(*(const char **)a, *(const char **)b);
}

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

static rl_compentry_func_t *completion_func = NULL;

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

char **builtin_completion(const char *text, int UNUSED start, int UNUSED end) {
	char **matches = NULL, *qp = NULL, *prefix = "";
	char *t = unquote(text, &qp);
	rl_compentry_func_t *completion;

	if (completion_func != NULL) {
		completion = completion_func;
		completion_func = NULL;
	} else
		completion = select_completion(text, &prefix);

	matches = rl_completion_matches(t+strlen(prefix), completion);
	if (matches != NULL) {
		size_t i, n;
		for (n = 1; matches[n]; n++)
			;
		qsort(&matches[1], n - 1, sizeof(matches[0]), matchcmp);
		if (rl_completion_type != '?') {
			matches[0] = quote(matches[0], n > 1, prefix, qp);
			for (i = 1; i < n; i++)
				matches[i] = quote(matches[i], FALSE, prefix, qp);
		}
	}

	efree(t);
	rl_attempted_completion_over = 1;
	rl_sort_completion_matches = 0;
	return matches;
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
#endif /* HAVE_READLINE */


/*
 * initialization
 */

/* initinput -- called at dawn of time from main() */
extern void initinput(void) {
	input = NULL;

	/* declare the global roots */
	globalroot(&error);		/* parse errors */
	globalroot(&prompt);		/* main prompt */
	globalroot(&prompt2);		/* secondary prompt */

#if HAVE_READLINE
	rl_readline_name = "es";

	/* this word_break_characters excludes '&' due to primitive completion */
	rl_basic_word_break_characters = " \t\n`$><=;|{()}";
	rl_filename_quote_characters = " \t\n\\`'$><=;|&{()}";
	rl_basic_quote_characters = "";
	rl_special_prefixes = "$";

	rl_completion_word_break_hook = completion_start;
	rl_filename_stat_hook = unquote_for_stat;
	rl_attempted_completion_function = builtin_completion;

	rl_add_funmap_entry("es-complete-filename", es_complete_filename);
	rl_add_funmap_entry("es-complete-variable", es_complete_variable);
	rl_add_funmap_entry("es-complete-primitive", es_complete_primitive);
	rl_bind_keyseq("\e/", es_complete_filename);
	rl_bind_keyseq("\e$", es_complete_variable);
#endif
}
