/* main.c -- initialization for es ($Revision: 1.3 $) */

#include "es.h"

/* initpid -- set $pid for this shell */
static void initpid(void) {
	vardef("pid", NULL, mklist(mkstr(str("%d", getpid())), NULL));
}

/* main -- initialize, parse command arguments, and start running */
int main(int argc, char **argv) {
	initgc();
	initconv();

	initinput();
	initprims();
	initvars();

	runinitial();

	initpid();
	initsignals();
	hidevariables();
	importenv(FALSE);

	Ref(List *, args, listify(argc, argv));

	Ref(List *, esmain, varlookup("fn-%main", NULL));
	if (esmain == NULL) {
		eprint("es:main not set\n");
		return 1;
	}

	ExceptionHandler

		esmain = append(esmain, args);
		return exitstatus(eval(esmain, NULL, 0));

	CatchException (e)

		/* This is the sub-root handler for the shell.
		 * The real root handler is in runtime.es. */
		if (termeq(e->term, "exit"))
			return exitstatus(e->next);
		return 1;

	EndExceptionHandler

	RefEnd2(esmain, args);
}
