/* input.c -- read input from files or strings */

#include "es.h"
#include "token.h"

/*
 * NOTE: character unget is supported for up to two characters, but NOT
 * in the case of EOF. Since EOF does not fit in a char, it is easiest
 * to support only one unget of EOF.
 */

typedef struct Input {
	enum { iFd, iString } t;
	char *ibuf;
	int fd, index, read, lineno, last;
	Boolean saved, eofread;
} Input;

#define BUFSIZE ((size_t) 256)

#if READLINE
extern char *readline(char *);
extern void add_history(char *);
static char *rlinebuf;
#endif

char *prompt, *prompt2;
Boolean esrc;

static int dead(void);
static int fdgchar(void);
static int stringgchar(void);
static void ugdead(int);
static void pushcommon(void);

static char *inbuf;
static size_t istacksize, chars_out, chars_in;
static Boolean eofread = FALSE, save_lineno = TRUE;
static Input *istack, *itop;

static char *history;
static int historyfd = -1;

static int (*realgchar)(void);
static void (*realugchar)(int);

int last;

extern int gchar() {
	if (eofread) {
		eofread = FALSE;
		return last = EOF;
	}
	return (*realgchar)();
}

extern void ugchar(int c) {
	(*realugchar)(c);
}

static int dead() {
	return last = EOF;
}

static void ugdead(int c) {
	return;
}

static void ugalive(int c) {
	if (c == EOF)
		eofread = TRUE;
	else
		inbuf[--chars_out] = c;
}

/* stringgchar -- get the next character from a string. */
static int stringgchar() {
	return last = (inbuf[chars_out] == '\0' ? EOF : inbuf[chars_out++]);
}

/* loghistory -- write last command out to a file */
static void loghistory(void) {
	size_t a;
	if (!interactive || history == NULL)
		return;
	if (historyfd == -1) {
		historyfd = eopen(history, rAppend);
		if (historyfd == -1) {
			eprint("history(%s): %s\n", history, strerror(errno));
			vardef("history", NULL, NULL);
			return;
		}
	}
	/*
	 * Small unix hack: since read() reads only up to a newline
	 * from a terminal, then presumably this write() will write at
	 * most only one input line at a time.
	 */
	for (a = 2; a < chars_in + 2; a++) { /* skip empty lines and comments in history. */
		if (inbuf[a] == '#' || inbuf[a] == '\n')
			return;
		if (inbuf[a] != ' ' && inbuf[a] != '\t')
			break;
	}
	ewrite(historyfd, inbuf + 2, chars_in);
}

/* sethistory -- change the file for the history log */
extern void sethistory(char *file) {
	if (historyfd != -1) {
		close(historyfd);
		historyfd = -1;
	}
	history = file;
}

#if READLINE && !SYSV_SIGNALS
/* doreadline -- signal-safe readline wrapper */
static char *doreadline(char *prompt) {
	char *r;
	interrupt_happened = FALSE;
	if (!setjmp(slowbuf.j)) {
		slow = TRUE;
		if (!interrupt_happened)
			r = readline(prompt);
		else
			r = NULL;
	} else
		r = NULL;
	slow = FALSE;
	if (r == NULL)
		errno = EINTR;
	SIGCHK();
	return r;
}
#else
#define doreadline readline
#endif	/* READLINE && !SVSIGS */

/*
   read a character from a file-descriptor. If GNU readline is defined, add a newline and doctor
   the buffer to look like a regular fdgchar buffer.
*/

static int fdgchar() {
	if (chars_out >= chars_in + 2) { /* has the buffer been exhausted? if so, replenish it */
		while (1) {
#if READLINE
			if (interactive && istack->fd == 0) {
				rlinebuf = readline(prompt);
				if (rlinebuf == NULL) {
					chars_in = 0;
				} else {
					if (*rlinebuf != '\0')
						add_history(rlinebuf);
					chars_in = strlen(rlinebuf) + 1;
					efree(inbuf);
					inbuf = ealloc(chars_in + 3);
					strcpy(inbuf+2, rlinebuf);
					strcat(inbuf+2, "\n");
				}
			} else
#endif
				{
				long r = eread(istack->fd, inbuf + 2, BUFSIZE);
				if (r < 0) {
					if (errno == EINTR)
						continue; /* Suppose it was interrupted by a signal */
					uerror("read");
					exit(1);
				}
				chars_in = (size_t) r;
			}
			break;
		}
		if (chars_in == 0)
			return last = EOF;
		chars_out = 2;
		if (verbose)
			ewrite(2, inbuf + 2, chars_in);
		loghistory();
	}
	return last = inbuf[chars_out++];
}

/* push an input source onto the stack. set up a new input buffer, and set gchar() */

static void pushcommon() {
	size_t idiff;
	istack->index = chars_out;
	istack->read = chars_in;
	istack->ibuf = inbuf;
	istack->lineno = lineno;
	istack->saved = save_lineno;
	istack->last = last;
	istack->eofread = eofread;
	istack++;
	idiff = istack - itop;
	if (idiff >= istacksize / sizeof (Input)) {
		itop = erealloc(itop, istacksize *= 2);
		istack = itop + idiff;
	}
	realugchar = ugalive;
	chars_out = 2;
	chars_in = 0;
}

static void pushfd(int fd) {
	pushcommon();
	istack->t = iFd;
	save_lineno = TRUE;
	istack->fd = fd;
	realgchar = fdgchar;
	inbuf = ealloc(BUFSIZE + 2);
	lineno = 1;
}

#if 0
static void pushstring(char **a, Boolean save) {
	pushcommon();
	istack->t = iString;
	save_lineno = lineno;
	inbuf = mprint("..%A", a);
	realgchar = stringgchar;
	if (save_lineno)
		lineno = 1;
	else
		--lineno;
}
#endif

/* popinput -- remove an input source from the stack. restore the right kind of getchar (string,fd) etc. */
static void popinput(void) {
	if (istack->t == iFd)
		close(istack->fd);
	efree(inbuf);
	--istack;
	realgchar = (istack->t == iString ? stringgchar : fdgchar);
	if (istack->fd == -1) { /* top of input stack */
		realgchar = dead;
		realugchar = ugdead;
	}
	last = istack->last;
	eofread = istack->eofread;
	inbuf = istack->ibuf;
	chars_out = istack->index;
	chars_in = istack->read;
	if (save_lineno)
		lineno = istack->lineno;
	else
		lineno++;
	save_lineno = istack->saved;
}

/* flushu -- flush input characters upto newline. Used by scanerror() */
extern void flushu(void) {
	int c;
	if (last == '\n' || last == EOF)
		return;
	while ((c = gchar()) != '\n' && c != EOF)
		; /* skip to newline */
	if (c == EOF)
		ugchar(c);
}


/*
 * parsing and scanning errors
 */

static char *error = NULL;

/* yyerror -- yacc error entry point */
extern void yyerror(char *s) {
	if (error == NULL)	/* first error is generally the most informative */
		error = locate(s);
}

/* callparse -- call yyparse(), but disable garbage collection and catch errors */
static void callparse(void) {
	int result;
	assert(error == NULL);

	gcdisable(200 * sizeof (Tree));		/* TODO: find a good size */
	result = yyparse();
	gcenable();

	if (result) {
		char *e = error;
		assert(error != NULL);
		error = NULL;
		fail(e);
	}
	assert(error == NULL);
}


/* doit -- the wrapper loop in rc: prompt for commands until EOF, calling yyparse and walk() */
static List *doit(Boolean execit) {
	Boolean eof;
	Handler h;
	List *e;
	Ref(List *, result, true);

	if (noexecute)
		execit = FALSE;

	while ((e = pushhandler(&h)) != NULL) {
		if (!interactive) {
			popinput();
			throw(e);
		}
		if (streq(getstr(e->term), "error"))
			eprint("%L\n", e->next, " ");
		else if (!issilentsignal(e))
			eprint("uncaught exception: %L\n", e, " ");
	}

	for (eof = FALSE; !eof;) {
		SIGCHK();
		if (interactive) {
			List *lp;
			if (!noexecute && (lp = varlookup("fn-prompt", NULL)) != NULL)
				eval(lp, NULL, TRUE);
#if !READLINE
			if (prompt != NULL)
				eprint("%s", prompt);
#endif
		}
		inityy();
		callparse();
		eof = (last == EOF); /* "last" can be clobbered during a walk() */
		if (parsetree != NULL)
			if (execit)
				result = walk(parsetree, NULL, TRUE);
			else if (printcmds && noexecute)
				eprint("%T\n", parsetree);
	}
	popinput();
	pophandler(&h);
	if (!execit) {
		RefPop(result);
		return (List *) parsetree;
	}
	RefReturn(result);
}

/* parsestring -- turn a string into a parse tree */
extern Tree *parsestring(const char *str) {
	Handler h;
	List *e;
	Tree *result;
	Boolean save_interactive;

	pushcommon();
	istack->t = iString;
	save_lineno = lineno;
	realgchar = stringgchar;
	if (save_lineno)
		lineno = 1;
	else
		--lineno;
	inbuf = ealloc(strlen(str) + 3);
	strcpy(inbuf + 2, str);
	save_interactive = interactive;
	interactive = FALSE;
	if ((e = pushhandler(&h)) != NULL) {
		interactive = save_interactive;
		throw(e);
	}
	result = (Tree *) doit(FALSE);
	interactive = save_interactive;
	pophandler(&h);
	interactive = save_interactive;
	return result;
}

/* runstring -- run commands from a string */
extern List *runstring(const char *str) {
	Handler h;
	List *result, *e;
	Boolean save_interactive;

	pushcommon();
	istack->t = iString;
	save_lineno = lineno;
	realgchar = stringgchar;
	if (save_lineno)
		lineno = 1;
	else
		--lineno;
	inbuf = ealloc(strlen(str) + 3);
	strcpy(inbuf + 2, str);
	save_interactive = interactive;
	interactive = FALSE;

	if ((e = pushhandler(&h)) != NULL) {
		interactive = save_interactive;
		throw(e);
	}
	result = doit(TRUE);
	interactive = save_interactive;
	pophandler(&h);
	return result;
}

/* runfd -- run commands from a file descriptor */
extern List *runfd(int fd) {
	pushfd(fd);
	return doit(TRUE);
}

/* closefds -- close file descriptors after a fork() */
extern void closefds(void) {
	Input *i;
	if (historyfd != -1) {			/* Close an open history file */
		close(historyfd);
		historyfd = -1;
	}
	for (i = istack; i != itop; --i)	/* close open input sources */
		if (i->t == iFd && i->fd > 2) {
			close(i->fd);
			i->fd = -1;
		}
}


/*
 * initialization
 */

/* initinput -- called at dawn of time from main() */
extern void initinput(void) {
	/*
	 * set up the input stack, and put a "dead" input at the bottom,
	 * so that yyparse will always read eof
	 */
	istack = itop = ealloc(istacksize = 256 * sizeof (Input));
	istack->t = iFd;
	istack->fd = -1;
	realugchar = ugalive;

	/* declare the global roots */
	globalroot(&history);		/* history file */
	globalroot(&error);		/* parse errors */
	globalroot(&prompt);		/* main prompt */
	globalroot(&prompt2);		/* secondary prompt */

	/* call the parser's initialization */
	initparse();
}
