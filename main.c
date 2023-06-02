/* main.c -- initialization for es ($Revision: 1.3 $) */

#include "es.h"

extern int optind;
extern char *optarg;

extern char **environ;


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

/* main -- initialize, parse command arguments, and start running */
int main(int argc, char **argv) {
	initgc();
	initconv();

	ExceptionHandler
		roothandler = &_localhandler;	/* unhygeinic */

		initinput();
		initprims();
		initvars();

		runinitial();

		initpath();
		initpid();
		initsignals();
		hidevariables();
		importenv(environ, FALSE);

		Ref(List *, args, listify(argc, argv));

		Ref(List *, esmain, varlookup("es:main", NULL));
		if (esmain == NULL) {
			eprint("es:main not set\n");
			return 1;
		}

		esmain = append(esmain, args);
		return exitstatus(eval(esmain, NULL, 0));

		RefEnd2(esmain, args);

	CatchException (e)

		if (termeq(e->term, "exit"))
			return exitstatus(e->next);
		else if (termeq(e->term, "error"))
			eprint("%L\n",
			       e->next == NULL ? NULL : e->next->next,
			       " ");
		else if (!issilentsignal(e))
			eprint("uncaught exception: %L\n", e, " ");
		return 1;

	EndExceptionHandler
}
