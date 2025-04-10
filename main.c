/* main.c -- initialization for es ($Revision: 1.3 $) */

#include "es.h"

#if GCVERBOSE
Boolean gcverbose	= FALSE;	/* -G */
#endif
#if GCINFO
Boolean gcinfo		= FALSE;	/* -I */
#endif

/* #if 0 && !HPUX && !defined(linux) && !defined(sgi) */
/* extern int getopt (int argc, char **argv, const char *optstring); */
/* #endif */

/* extern int isatty(int fd); */
extern char **environ;


/* checkfd -- open /dev/null on an fd if it is closed */
static void checkfd(int fd, OpenKind r) {
	int new;
	new = dup(fd);
	if (new != -1)
		close(new);
	else if (errno == EBADF && (new = eopen("/dev/null", r)) != -1)
		mvfd(new, fd);
}

/* initpath -- set $path based on the configuration default */
static void initpath(void) {
	int i;
	static const char * const path[] = { INITIAL_PATH };

	Ref(List *, list, NULL);
	for (i = arraysize(path); i-- > 0;) {
		Term *t = mkstr((char *) path[i]);
		list = mklist(t, list);
	}
	vardef("path", NULL, list);
	RefEnd(list);
}

/* initpid -- set $pid for this shell */
static void initpid(void) {
	vardef("pid", NULL, mklist(mkstr(str("%d", getpid())), NULL));
}

/* runesrc -- run the user's profile, if it exists */
static void runesrc(void) {
	char *esrc = str("%L/.esrc", varlookup("home", NULL), "\001");
	int fd = eopen(esrc, oOpen);
	if (fd != -1) {
		ExceptionHandler
			runfd(fd, esrc, 0);
		CatchException (e)
			if (termeq(e->term, "exit"))
				exit(exitstatus(e->next));
			else if (termeq(e->term, "error")) {
				eprint("%L\n",
				       e->next == NULL ? NULL : e->next->next,
				       " ");
				return;
			}
			if (!issilentsignal(e))
				eprint("uncaught exception: %L\n", e, " ");
			return;
		EndExceptionHandler
	}
}

/* usage -- print usage message and die */
static Noreturn usage(void) {
	eprint(
		"usage: es [-c command] [-silevxnpo] [file [args ...]]\n"
		"	-c cmd	execute argument\n"
		"	-s	read commands from standard input; stop option parsing\n"
		"	-i	interactive shell\n"
		"	-l	login shell\n"
		"	-e	exit if any command exits with false status\n"
		"	-v	print input to standard error\n"
		"	-x	print commands to standard error before executing\n"
		"	-n	just parse; don't execute\n"
		"	-p	don't load functions from the environment\n"
		"	-o	don't open stdin, stdout, and stderr if they were closed\n"
		"	-d	don't ignore SIGQUIT or SIGTERM\n"
	);
	eprint(""
#if GCINFO
		"	-I	print garbage collector information\n"
#endif
#if GCVERBOSE
		"	-G	print verbose garbage collector information\n"
#endif
#if LISPTREES
		"	-L	print parser results in LISP format\n"
#endif
	);
	exit(1);
}


/* main -- initialize, parse command arguments, and start running */
int main(int argc, char **argv0) {
	int c, status = 0;
	char **volatile argv = argv0;

	volatile int runflags = 0;		/* -[einvxL] */
	volatile Boolean protected = FALSE;	/* -p */
	volatile Boolean allowquit = FALSE;	/* -d */
	volatile Boolean cmd_stdin = FALSE;		/* -s */
	volatile Boolean loginshell = FALSE;	/* -l or $0[0] == '-' */
	Boolean keepclosed = FALSE;		/* -o */
	Ref(const char *volatile, cmd, NULL);	/* -c */

	initconv();
	initgc();

	if (argc == 0) {
		argc = 1;
		argv = ealloc(2 * sizeof (char *));
		argv[0] = "es";
		argv[1] = NULL;
	}
	if (*argv[0] == '-')
		loginshell = TRUE;

	Ref(List *, args, listify(argc, argv));
	esoptbegin(args->next, NULL, NULL, FALSE);
	while ((c = esopt("eilxvnpodsc:?GIL")) != EOF)
		switch (c) {
		case 'c':	cmd = getstr(esoptarg());	break;
		case 'e':	runflags |= eval_exitonfalse;	break;
		case 'i':	runflags |= run_interactive;	break;
		case 'n':	runflags |= run_noexec;		break;
		case 'v':	runflags |= run_echoinput;	break;
		case 'x':	runflags |= run_printcmds;	break;
#if LISPTREES
		case 'L':	runflags |= run_lisptrees;	break;
#endif
		case 'l':	loginshell = TRUE;		break;
		case 'p':	protected = TRUE;		break;
		case 'o':	keepclosed = TRUE;		break;
		case 'd':	allowquit = TRUE;		break;
		case 's':	cmd_stdin = TRUE;		goto getopt_done;
#if GCVERBOSE
		case 'G':	gcverbose = TRUE;		break;
#endif
#if GCINFO
		case 'I':	gcinfo = TRUE;			break;
#endif
		default:
			usage();
		}

getopt_done:
	Ref(List *, argp, esoptend());

	if (cmd_stdin && cmd != NULL) {
		eprint("es: -s and -c are incompatible\n");
		exit(1);
	}

	if (!keepclosed) {
		checkfd(0, oOpen);
		checkfd(1, oCreate);
		checkfd(2, oCreate);
	}

	if (
		cmd == NULL
	     && (argp == NULL || cmd_stdin)
	     && (runflags & run_interactive) == 0
	     && isatty(0)
	)
		runflags |= run_interactive;

	ExceptionHandler
		roothandler = &_localhandler;	/* unhygeinic */

		initinput();
		initprims();
		initvars();

		runinitial();

		initpath();
		initpid();
		initsignals(runflags & run_interactive, allowquit);
		initpgrp();
		hidevariables();
		initenv(environ, protected);

		if (loginshell)
			runesrc();

		if (cmd == NULL && !cmd_stdin && argp != NULL) {
			int fd;
			char *file = getstr(argp->term);
			argp = argp->next;
			if ((fd = eopen(file, oOpen)) == -1) {
				eprint("%s: %s\n", file, esstrerror(errno));
				status = 1;
				goto return_main;
			}
			vardef("*", NULL, argp);
			vardef("0", NULL, mklist(mkstr(file), NULL));
			status = exitstatus(runfd(fd, file, runflags));
			goto return_main;
		}

		vardef("*", NULL, argp);
		vardef("0", NULL, mklist(mkstr(argv[0]), NULL));
		if (cmd != NULL)
			status = exitstatus(runstring(cmd, NULL, runflags));
		else
			status = exitstatus(runfd(0, "stdin", runflags));

	CatchException (e)

		if (termeq(e->term, "exit")) {
			status = exitstatus(e->next);
			goto return_main;
		} else if (termeq(e->term, "error")) {
			eprint("%L\n",
			       e->next == NULL ? NULL : e->next->next,
			       " ");
			status = 1;
			goto return_main;
		} else {
#if JOB_PROTECT
			tcreturnpgrp();
#endif
			exitonsignal(e);
		}
		if (!issilentsignal(e))
			eprint("uncaught exception: %L\n", e, " ");
		status = 1;

	EndExceptionHandler
	RefEnd3(argp, args, cmd);
return_main:
#if JOB_PROTECT
	tcreturnpgrp();
#endif
	return status;
}
