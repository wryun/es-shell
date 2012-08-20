/* input.c -- read input from files or strings */

#include "es.h"
#include "input.h"


/*
 * constants
 */

#define	BUFSIZE		((size_t) 1024)		/* buffer size to fill reads into */


/*
 * macros
 */

#define	ISEOF(in)	((in)->fill == eoffill)


/*
 * globals
 */

Input *input;
char *prompt, *prompt2;

Boolean disablehistory;
static char *history;
static int historyfd = -1;

#if READLINE
extern int rl_meta_chars;
extern char *readline(char *);
extern void add_history(char *);
#endif


/*
 * errors and warnings
 */

/* locate -- identify where an error came from */
static char *locate(Input *in, char *s) {
	return in->interactive
		? s
		: str("%s:%d: %s", in->name, in->lineno, s);
}

static char *error = NULL;

/* yyerror -- yacc error entry point */
extern void yyerror(char *s) {
	if (error == NULL)	/* first error is generally the most informative */
		error = locate(input, s);
}

/* warn -- print a warning */
static void warn(char *s) {
	eprint("warning: %s\n", locate(input, s));
}


/*
 * history
 */

/* loghistory -- write the last command out to a file */
static void loghistory(const char *cmd, size_t len) {
	const char *s, *end;
	if (history == NULL || disablehistory)
		return;
	if (historyfd == -1) {
		historyfd = eopen(history, oAppend);
		if (historyfd == -1) {
			eprint("history(%s): %s\n", history, strerror(errno));
			vardef("history", NULL, NULL);
			return;
		}
	}
	/* skip empty lines and comments in history */
	for (s = cmd, end = s + len; s < end; s++)
		switch (*s) {
		case '#': case '\n':	return;
		case ' ': case '\t':	break;
		default:		goto writeit;
		}
	writeit:
		;
	/*
	 * Small unix hack: since read() reads only up to a newline
	 * from a terminal, then presumably this write() will write at
	 * most only one input line at a time.
	 */
	ewrite(historyfd, cmd, len);
}

/* sethistory -- change the file for the history log */
extern void sethistory(char *file) {
	if (historyfd != -1) {
		close(historyfd);
		historyfd = -1;
	}
	history = file;
}


/*
 * getting characters
 */

/* get -- get a character, filter out nulls */
extern int get(Input *in) {
	int c;
	while ((c = (in->buf < in->bufend ? *in->buf++ : (*in->fill)(in))) == '\0')
		warn("null character ignored");
	return c;
}

/* eoffill -- report eof when called to fill input buffer */
static int eoffill(Input *in) {
	assert(in->fd == -1);
	return EOF;
}

/* callreadline -- signal-safe readline wrapper */
#if READLINE && !SYSV_SIGNALS
static char *callreadline(char *prompt) {
	char *r;
	interrupted = FALSE;
	if (!setjmp(slowlabel)) {
		slow = TRUE;
		r = interrupted ? NULL : readline(prompt);
	} else
		r = NULL;
	slow = FALSE;
	if (r == NULL)
		errno = EINTR;
	SIGCHK();
	return r;
}
#else
#define	callreadline readline
#endif	/* READLINE && !SVSIGS */

/* fdfill -- fill input buffer by reading from a file descriptor */
static int fdfill(Input *in) {
	long nread;
	assert(in->buf == in->bufend);
	assert(in->fd >= 0);

#if READLINE
	if (in->interactive) {
		char *rlinebuf = callreadline(prompt);
		if (rlinebuf == NULL)
			nread = 0;
		else {
			if (*rlinebuf != '\0')
				add_history(rlinebuf);
			nread = strlen(rlinebuf) + 1;
			if (in->buflen < nread) {
				while (in->buflen < nread)
					in->buflen *= 2;
				efree(in->bufbegin);
				in->bufbegin = erealloc(in->bufbegin, in->buflen);
			}
			memcpy(in->bufbegin, rlinebuf, nread - 1);
			in->bufbegin[nread - 1] = '\n';
		}
	} else
#endif
	do {
		nread = eread(in->fd, (char *) in->bufbegin, in->buflen);
		SIGCHK();
	} while (nread == -1 && errno == EINTR);

	if (nread <= 0) {
		close(in->fd);
		in->fd = -1;
		in->fill = eoffill;
		in->interactive = FALSE;
		if (nread == -1)
			fail("%s: %s", in->name == NULL ? "es" : in->name, strerror(errno));
		return EOF;
	}

	if (in->echoinput)
		ewrite(2, (char *) in->bufbegin, nread);
	if (in->interactive)
		loghistory((char *) in->bufbegin, nread);

	in->buf = in->bufbegin;
	in->bufend = &in->buf[nread];
	return *in->buf++;
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
	} else if (in->bufbegin < in->buf && in->buf[-1] == c)
		--in->buf;
	else {
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
 * the input loop
 */

/* parse -- call yyparse(), but disable garbage collection and catch errors */
extern Tree *parse(char *pr1, char *pr2) {
	int result;
	assert(error == NULL);

	inityy();
	emptyherequeue();

	if (ISEOF(input))
		throw(mklist(mkterm("eof", NULL), NULL));

#if READLINE
	prompt = (pr1 == NULL) ? "" : pr1;
#else
	if (pr1 != NULL)
		eprint("%s", pr1);
#endif
	prompt2 = pr2;

	gcdisable(200 * sizeof (Tree));		/* TODO: find a good size */
	result = yyparse();
	gcenable();

	if (result || error != NULL) {
		char *e;
		assert(error != NULL);
		e = error;
		error = NULL;
		fail(e);
	}
#if LISPTREES
	if (input->lisptrees)
		eprint("%B\n", parsetree);
#endif
	return parsetree;
}

/* runinput -- run from an input source */
extern List *runinput(Input *in, int flags) {
	Handler h;
	List *e, *repl, *arg, *result;
	const char *dispatcher[] = {
		"fn-%eval-noprint",
		"fn-%eval-print",
		"fn-%noeval-noprint",
		"fn-%noeval-print",
	};

	in->prev = input;
	input = in;

	if (flags & run_echoinput) {
		in->echoinput = TRUE;
		if (in->buf < in->bufend)
			ewrite(2, (char *) in->buf, in->bufend - in->buf);
	}
	if (flags & run_interactive)
		in->interactive = TRUE;
#if LISPTREES
	if (flags & run_lisptrees)
		in->lisptrees = TRUE;
#endif

	if ((e = pushhandler(&h)) != NULL) {
		(*input->cleanup)(input);
		input = input->prev;
		throw(e);
	}

	arg = varlookup(dispatcher[
			  ((flags & run_printcmds) ? 1 : 0)
			+ ((flags & run_noexec)    ? 2 : 0)
		], NULL);
	repl = varlookup((flags & run_interactive) ? "fn-%interactive-loop" : "fn-%batch-loop", NULL);
	if (repl == NULL)
		result = prim("batchloop", arg, TRUE, exitonfalse);
	else {
		if (arg != NULL)
			repl = append(repl, arg);
		result = eval(repl, NULL, TRUE, exitonfalse);
	}

	pophandler(&h);
	input = in->prev;
	(*in->cleanup)(in);
	return result;
}


/*
 * pushing new input sources
 */

/* fdcleanup -- cleanup after running from a file descriptor */
static void fdcleanup(Input *in) {
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
	Handler h;
	List *e;
	Tree *result;

	in->prev = input;
	input = in;

	if ((e = pushhandler(&h)) != NULL) {
		(*input->cleanup)(input);
		input = input->prev;
		throw(e);
	}

	result = parse(NULL, NULL);
	if (get(in) != EOF)
		fail("more than one value in term");

	pophandler(&h);
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
	return input == NULL ? FALSE : input->interactive;
}


/*
 * initialization
 */

/* initinput -- called at dawn of time from main() */
extern void initinput(void) {
	input = NULL;

	/* declare the global roots */
	globalroot(&history);		/* history file */
	globalroot(&error);		/* parse errors */
	globalroot(&prompt);		/* main prompt */
	globalroot(&prompt2);		/* secondary prompt */

	/* mark the historyfd as a file descriptor to hold back from forked children */
	registerfd(&historyfd, TRUE);

	/* call the parser's initialization */
	initparse();

#if READLINE
	rl_meta_chars = 0;
#endif
}
