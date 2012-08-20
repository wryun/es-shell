/* main.c -- initialization for es ($Revision: 1.10 $) */

#include "es.h"

#if GCVERBOSE
Boolean gcverbose	= FALSE;	/* -G */
#endif
#if GCINFO
Boolean gcinfo		= FALSE;	/* -I */
#endif

extern int getopt (int argc, char **argv, const char *optstring);
extern int optind;
extern char *optarg;

extern int isatty(int fd);
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
	for (i = arraysize(path); i-- > 0;)
		list = mklist(mkterm((char *) path[i], NULL), list);
	vardef("path", NULL, list);
	RefEnd(list);
}

/* initpid -- set $pid for this shell */
static void initpid(void) {
	vardef("pid", NULL, mklist(mkterm(str("%d", getpid()), NULL), NULL));
}

/* usage -- print usage message and die */
static noreturn usage(void) {
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
	exit(1);
}

/* main -- initialize, parse command arguments, and start running */
int main(int argc, char **argv) {
	int c;
	List *e;
	Handler h;
	volatile int ac;
	char **volatile av;

	volatile int runflags = 0;		/* -[einvxL] */
	volatile Boolean protected = FALSE;	/* -p */
	volatile Boolean allowquit = FALSE;	/* -d */
	volatile Boolean stdin = FALSE;		/* -s */
	volatile Boolean loginshell = FALSE;	/* -l or $0[0] == '-' */
	Boolean keepclosed = FALSE;		/* -o */
	const char *volatile cmd = NULL;	/* -c */

	initgc();
	initconv();

	if (argc == 0) {
		argc = 1;
		argv = ealloc(2 * sizeof (char *));
		argv[0] = "es";
		argv[1] = NULL;
	}
	if (*argv[0] == '-')
		loginshell = TRUE;

	while ((c = getopt(argc, argv, "eilxvnpodsc:?GIL")) != EOF)
		switch (c) {
		case 'c':	cmd = optarg;			break;
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
		case 's':	stdin = TRUE;			goto getopt_done;
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
	if (stdin && cmd != NULL) {
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
	     && (optind == argc || stdin)
	     && (runflags & run_interactive) == 0
	     && isatty(0)
	)
		runflags |= run_interactive;

	ac = argc;
	av = argv;

	roothandler = &h;
	if ((e = pushhandler(&h)) != NULL) {
		if (streq(getstr(e->term), "error"))
			eprint("%L\n", e->next == NULL ? NULL : e->next->next, " ");
		else if (!issilentsignal(e))
			eprint("uncaught exception: %L\n", e, " ");
		return 1;
	}

	initinput();
	initprims();
	initvars();

	runinitial();

	initpath();
	initpid();
	initsignals(runflags & run_interactive, allowquit);
	hidevariables();
	initenv(environ, protected);

	if (loginshell) {
		char *esrc = str("%L/.esrc", varlookup("home", NULL), "\001");
		int fd = eopen(esrc, oOpen);
		if (fd != -1)
			runfd(fd, esrc, 0);
	}

	if (cmd == NULL && !stdin && optind < ac) {
		int fd;
		Ref(char *, file, av[optind++]);
		if ((fd = eopen(file, oOpen)) == -1) {
			eprint("%s: %s\n", file, strerror(errno));
			return 1;
		}
		vardef("*", NULL, listify(ac - optind, av + optind));
		vardef("0", NULL, mklist(mkterm(file, NULL), NULL));
		return exitstatus(runfd(fd, file, runflags));
		RefEnd(file);
	}

	vardef("*", NULL, listify(ac - optind, av + optind));
	vardef("0", NULL, mklist(mkterm(av[0], NULL), NULL));
	if (cmd != NULL)
		return exitstatus(runstring(cmd, NULL, runflags));
	return exitstatus(runfd(0, "stdin", runflags));
}
