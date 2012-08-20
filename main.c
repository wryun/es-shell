/* main.c -- initialization for es */

#include "es.h"

Boolean interactive	= FALSE;	/* -i or isatty(input) */
Boolean loginshell	= FALSE;	/* -l or $0[0] == '-' */
Boolean noexecute	= FALSE;	/* -n */
Boolean verbose		= FALSE;	/* -v */
Boolean printcmds	= FALSE;	/* -x */

Boolean bugme		= FALSE;	/* -B */
Boolean gcdebug		= FALSE;	/* -G */

static const char initial[] =
#include "initial.h"
;

extern int getopt (int argc, char **argv, const char *optstring);
extern int optind;
extern char *optarg;

extern int isatty(int fd);


/* checkfd -- open /dev/null on an fd if it is closed */
static void checkfd(int fd, RedirKind r) {
	int new;
	new = dup(fd);
	if (new != -1)
		close(new);
	else if (errno == EBADF && (new = eopen("/dev/null", r)) != -1)
		mvfd(new, fd);
}


/* usage -- print usage message and die */
static noreturn usage(void) {
	fprint(2,
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
int main(int argc, char **argv, char **envp) {
	Handler h;
	List *e;
	int c;

	const char *volatile cmd = NULL;	/* -c */
	Boolean protected = FALSE;		/* -p */
	Boolean keepclosed = FALSE;		/* -o */
	Boolean allowquit = FALSE;		/* -d */
	volatile Boolean stdin = FALSE;		/* -s */

	initgc();
	initconv();

	if (*argv[0] == '-')
		loginshell = TRUE;

	while ((c = getopt(argc, argv, "eilxvnpodsc:?DG")) != EOF)
		switch (c) {
		case 'c':	cmd = optarg;		break;
		case 'i':	interactive = TRUE;	break;
		case 'l':	loginshell = TRUE;	break;
		case 'v':	verbose = TRUE;		break;
		case 'x':	printcmds = TRUE;	break;
		case 'n':	noexecute = TRUE;	break;
		case 'p':	protected = TRUE;	break;
		case 'o':	keepclosed = TRUE;	break;
		case 'd':	allowquit = TRUE;	break;

		case 's':
			if (cmd != NULL) {
				eprint("es: cannot use -s with -c\n");
				exit(1);
			}
			stdin = TRUE;
			goto getopt_done;

		case 'e':
		default:
			usage();
		case 'D':	bugme = TRUE;		break;
		case 'G':	gcdebug = TRUE;		break;
		}

getopt_done:
	if (!keepclosed) {
		checkfd(0, rOpen);
		checkfd(1, rCreate);
		checkfd(2, rCreate);
	}

	if (cmd == NULL && (optind == argc || stdin) && !interactive && isatty(0))
		interactive = TRUE;

	initinput();
	initprims();
	initvars(envp, initial, protected);
	initsignals(allowquit);

	if (
		(setjmp(childhandler.label) && (e = exception) != NULL)
	     || (e = pushhandler(&h)) != NULL
	) {
		if (streq(getstr(e->term), "error"))
			eprint("%L\n", e->next);
		else if (!issilentsignal(e))
			eprint("uncaught exception: %L\n", e, " ");
		return 1;
	}

	if (loginshell) {
		char *esrc = str("%L/.esrc", varlookup("home", NULL), "\001");
		int fd = eopen(esrc, rOpen);
		if (fd != -1) {
			Boolean save_interactive = interactive;
			interactive = FALSE;
			runfd(fd);
			interactive = save_interactive;
		}
	}

	if (cmd == NULL && !stdin && optind < argc) {
		char *file = argv[optind++];
		int fd = eopen(file, rOpen);
		if (fd == -1) {
			eprint("%s: %s\n", file, strerror(errno));
			return 1;
		}
		vardef("*", NULL, listify(argc - optind, argv + optind));
		vardef("0", NULL, mklist(mkterm(file, NULL), NULL));
		noexport("0");
		return exitstatus(runfd(fd));
	}

	vardef("*", NULL, listify(argc - optind, argv + optind));
	vardef("0", NULL, mklist(mkterm(argv[0], NULL), NULL));
	noexport("0");
	if (cmd != NULL)
		return exitstatus(runstring(cmd));
	return exitstatus(runfd(0));
}
