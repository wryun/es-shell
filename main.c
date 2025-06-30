/* main.c -- initialization for es ($Revision: 1.3 $) */

#include "es.h"

/* main -- initialize, parse command arguments, and start running */
int main(int argc, char **argv) {
	volatile int status = 1;
	initgc();
	initconv();
	initgc();

	initinput();
#if HAVE_READLINE
	inithistory();
#endif
	initprims();
	initvars();

	runinitial();

	initsignals();
	hidevariables();
	importenv(FALSE);

	Ref(List *, args, listify(argc, argv));

	Ref(List *, esmain, varlookup("fn-%main", NULL));
	if (esmain == NULL) {
		eprint("%main not set\n");
		return 1;
	}

	ExceptionHandler

		esmain = append(esmain, args);
		return exitstatus(eval(esmain, NULL, 0));

	CatchException (e)

		/* This is the sub-root handler for the shell.
		 * The real root handler is in runtime.es. */
		if (termeq(e->term, "exit"))
			status = exitstatus(e->next);
		else
			exitonsignal(e);

	EndExceptionHandler
	RefEnd2(esmain, args);

#if JOB_PROTECT
	tcreturnpgrp();
#endif
	return status;
}
