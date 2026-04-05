/* input.c -- read input from files or strings ($Revision: 1.2 $) */

#define	REQUIRE_STAT	1

#include "es.h"
#include "input.h"

/*
 * constants
 */

#define	BUFSIZE		((size_t) 4096)		/* buffer size to fill reads into */


/*
 * globals
 */

static Input *input = NULL;


/*
 * errors and warnings
 */

/* locate -- identify where an error came from */
static const char *locate(Input *in, const char *s) {
	return (in->runflags & run_interactive)
		? pstr("%s", s)
		: pstr("%s:%d: %s", in->name, in->lineno, s);
}

/* yyerror -- yacc error entry point */
extern void yyerror(Parser *p, const char *s) {
	if (p->error == NULL)	/* first error is generally the most informative */
		p->error = locate(p->input, s);
}


/*
 * getting and ungetting characters
 */

/* fill -- fill input buffer by running a command */
static int fill(Parser *p) {
	List *result;
	char *read;
	size_t nread;
	Input *in = p->input;

	assert(p->buf == p->bufend);

	if (p->reader != NULL) {
		result = eval(p->reader, NULL, 0);
		read = str("%L\n", result, " ");
	} else {
		result = prim("read", NULL, 0);
		RefAdd(result);
		if (length(result) > 1)
			eprint("%s\n", locate(in, "null character ignored"));
		read = str("%L\n", result, "");
		RefRemove(result);
	}
	if (result == NULL) {	/* eof */
		in->eof = TRUE;
		return EOF;
	}
	if ((nread = strlen(read)) > p->buflen) {
		p->bufbegin = erealloc(p->bufbegin, nread);
		p->buflen = nread;
	}
	memcpy(p->bufbegin, read, nread);

	p->buf = p->bufbegin;
	p->bufend = &p->buf[nread];

	return *p->buf++;
}

/* get -- get a character, filter out nulls */
extern int get(Parser *p) {
	int c;
	if (p->ungot > 0)
		return p->unget[--p->ungot];
	c = p->buf < p->bufend ? *p->buf++ : fill(p);
	if (c != EOF && p->input->runflags & run_echoinput) {
		char buf = (char)c;
		ewrite(2, &buf, 1);
	}
	return c;
}

/* unget -- push back one character */
extern void unget(Parser *p, int c) {
	assert(p->ungot < MAXUNGET);
	p->unget[p->ungot++] = c;
}

static void initbuf(Parser *p) {
	const char *initial = p->input->str;
	p->buflen = initial == NULL ? BUFSIZE : strlen(initial);
	p->bufbegin = p->buf = ealloc(p->buflen);
	if (initial != NULL) {
		memcpy(p->buf, initial, p->buflen);
		p->bufend = p->bufbegin + p->buflen;
	} else
		p->bufend = p->bufbegin;
}

/*
 * parse -- wrapper around yyparse()
 */
extern Tree *parse(List *reader) {
	volatile int result;
	int fd, ticket = UNREGISTERED;
	Parser p;
	void *oldpspace;
	List *volatile readexception = NULL;

	if (input->eof) {
		input->eof = FALSE;
		throw(mklist(mkstr("eof"), NULL));
	}

	memzero(&p, sizeof (Parser));
	p.input = input;
	p.reader = reader;
	RefAdd(p.reader);
	p.space = createpspace();
	oldpspace = setpspace(p.space);

	inityy(&p);
	initbuf(&p);
	p.tokenbuf = ealloc(p.bufsize);

	fd = (input->fd == -1)
		? eopen("/dev/null", oOpen)
		: dup(input->fd);
	ticket = defer_mvfd(TRUE, fd, 0);

	ExceptionHandler

		result = yyparse(&p);

	CatchException (e)

		readexception = e;

	EndExceptionHandler

	undefer(ticket);
	RefRemove(p.reader);
	assert(p.ungot == 0);
	if (p.bufbegin != NULL)
		efree(p.bufbegin);
	if (p.tokenbuf != NULL)
		efree(p.tokenbuf);

	if (result || p.error != NULL || readexception != NULL) {
		assert(p.error != NULL || readexception != NULL);
		Ref(const char *, e, p.error != NULL ? str("%s", p.error) : NULL);
		pseal(NULL);
		setpspace(oldpspace);
		if (e != NULL)
			fail("$&parse", "%s", e);
		else
			throw(readexception);
		RefEnd(e);
	}

	Ref(Tree *, tree, pseal(p.tree));
	setpspace(oldpspace);
#if LISPTREES
	if (input->runflags & run_lisptrees)
		eprint("%B\n", tree);
#endif
	RefReturn(tree);
}


/*
 * the input loop
 */

/* cleanup -- clean up after an input source */
static void cleanup(Input *in) {
	if (in->fd != -1) {
		unregisterfd(&in->fd);
		close(in->fd);
	}
}

/* runinput -- run from an input source */
extern List *runinput(Input *in, int runflags) {
	volatile int flags = runflags;
	List * volatile result = NULL;
	List *repl, *dispatch;
	Push push;
	Input *prev = input;
	const char *dispatcher[] = {
		"fn-%eval-noprint",
		"fn-%eval-print",
		"fn-%noeval-noprint",
		"fn-%noeval-print",
	};

	flags &= ~eval_inchild;
	in->runflags = flags;
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
				? prim("batchloop", NULL, flags)
				: eval(repl, NULL, flags);

		varpop(&push);

	CatchException (e)

		cleanup(input);
		input = prev;
		throw(e);

	EndExceptionHandler

	cleanup(input);
	input = prev;
	return result;
}


/*
 * pushing new input sources
 */

/* runfd -- run commands from a file descriptor */
extern List *runfd(int fd, const char *name, int flags) {
	Input in;
	List *result;

	memzero(&in, sizeof (Input));
	in.lineno = 1;
	in.fd = fd;
	registerfd(&in.fd, TRUE);
	in.name = (name == NULL) ? str("fd %d", fd) : name;

	RefAdd(in.name);
	result = runinput(&in, flags);
	RefRemove(in.name);

	return result;
}

/* runstring -- run commands from a string */
extern List *runstring(const char *str, int flags) {
	Input in;
	List *result;

	assert(str != NULL);

	memzero(&in, sizeof (Input));
	in.fd = -1;
	in.lineno = 1;
	in.name = str;
	in.str = str;

	RefAdd(in.name);
	RefAdd(in.str);
	result = runinput(&in, flags);
	RefRemove(in.str);
	RefRemove(in.name);
	return result;
}

/* parseinput -- turn an input source into a tree */
extern Tree *parseinput(Input *in) {
	Tree * volatile result = NULL;
	Input *prev = input;

	in->runflags = 0;
	input = in;

	ExceptionHandler
		result = parse(NULL);
		if (!in->eof)
			fail("$&parse", "more than one value in term");
	CatchException (e)
		cleanup(input);
		input = prev;
		throw(e);
	EndExceptionHandler

	cleanup(input);
	input = prev;
	return result;
}

/* parsestring -- turn a string into a tree; must be exactly one tree */
extern Tree *parsestring(const char *str) {
	Input in;
	Tree *result;

	assert(str != NULL);

	memzero(&in, sizeof (Input));
	in.fd = -1;
	in.lineno = 1;
	in.name = str;
	in.str = str;

	RefAdd(in.name);
	RefAdd(in.str);
	result = parseinput(&in);
	RefRemove(in.str);
	RefRemove(in.name);
	return result;
}

/* isinteractive -- is the innermost input source interactive? */
extern Boolean isinteractive(void) {
	return input == NULL ? FALSE : ((input->runflags & run_interactive) != 0);
}

/* isfromfd -- is the innermost input source reading from a file descriptor? */
extern Boolean isfromfd(void) {
	return input == NULL ? FALSE : (input->fd >= 0);
}
