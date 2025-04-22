/* input.c -- read input from files or strings ($Revision: 1.2 $) */

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
Boolean ignoreeof = FALSE;


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

static int eoffill(Input UNUSED *in);

/* yyerror -- yacc error entry point */
extern void yyerror(const char *s) {
	/* TODO: more graceful handling for memory exhaustion?
	 * if we're here, then we're probably hopelessly lost */
	if (streq(s, "memory exhausted")) {
		input->fd = -1;
		input->fill = eoffill;
		input->runflags &= ~run_interactive;
		error = s;
	} else if (error == NULL)	/* first error is generally the most informative */
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
	while ((c = (in->buf < in->bufend ? *in->buf++ : (*in->fill)(in))) == '\0')
		warn("null character ignored");
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

/* fdfill -- fill input buffer by reading from a file descriptor */
static int fdfill(Input *in) {
	long nread;
	if (in->fd < 0)
		fail("$&parse", "cannot read from closed file descriptor");

	do {
		nread = eread(in->fd, (char *) in->bufbegin, in->buflen);
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

static List *fillcmd = NULL;

static int cmdfill(Input *in) {
	char *read;
	List *result;
	size_t nread;
	int oldf;

	assert(in->buf == in->bufend);

	if (fillcmd == NULL)
		return fdfill(in);

	oldf = dup(0);
	if (in->fd >= 0) {
		if (dup2(in->fd, 0) == -1)
			fail("$&parse", "dup2: %s", esstrerror(errno));
	} else
		close(0);

	ExceptionHandler

		result = eval(fillcmd, NULL, 0);

	CatchException (e)

		mvfd(oldf, 0);
		throw(e);

	EndExceptionHandler

	mvfd(oldf, 0);

	if (result == NULL) { /* eof */
		if (!ignoreeof) {
			close(in->fd);
			in->fd = -1;
			in->fill = eoffill;
			in->runflags &= ~run_interactive;
		}
		return EOF;
	}
	read = str("%L\n", result, " ");

	if ((nread = strlen(read)) > in->buflen) {
		in->bufbegin = erealloc(in->bufbegin, nread);
		in->buflen = nread;
	}
	memcpy(in->bufbegin, read, nread);

	in->buf = in->bufbegin;
	in->bufend = &in->buf[nread];

	return *in->buf++;
}

/*
 * the input loop
 */

static Boolean parsing = FALSE;

/* parse -- call yyparse() and catch errors */
extern Tree *parse(List *fc) {
	int result;
	assert(error == NULL);

	inityy();
	emptyherequeue();

	if (ISEOF(input))
		throw(mklist(mkstr("eof"), NULL));

	if (parsing)
		fail("$&parse", "cannot perform nested parsing");

	fillcmd = fc;
	parsing = TRUE;

	ExceptionHandler

		result = yyparse();

	CatchException (e)

		parsing = FALSE;
		fillcmd = NULL;
		pseal(NULL);
		throw(e);

	EndExceptionHandler

	parsing = FALSE;
	fillcmd = NULL;

	if (result || error != NULL) {
		assert(error != NULL);
		Ref(const char *, e, error);
		error = NULL;
		pseal(NULL);
		fail("$&parse", "%s", e);
		RefEnd(e);
	}

	Ref(Tree *, pt, pseal(parsetree));
#if LISPTREES
	if (input->runflags & run_lisptrees)
		eprint("%B\n", pt);
#endif
	RefReturn(pt);
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
	in.fill = cmdfill;
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
		result = parse(NULL);
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

extern Boolean isfromfd(void) {
	return input == NULL ? FALSE : (input->fill == fdfill || input->fill == cmdfill);
}


/*
 * initialization
 */

/* initinput -- called at dawn of time from main() */
extern void initinput(void) {
	input = NULL;

	/* declare the global roots */
	globalroot(&fillcmd);
	globalroot(&error);		/* parse errors */
}
