/* prim-dump.c -- primitives for the esdump step ($Revision: 1.0 $) */

#include "es.h"
#include "prim.h"

PRIM(batchloop) {
	Ref(List *, result, true);

	ExceptionHandler

		for (;;) {
			List *cmd = prim("parse", NULL, NULL, 0);
			result = eval(cmd, NULL, evalflags);
			SIGCHK();
		}

	CatchException (e)

		if (!termeq(e->term, "eof"))
			throw(e);

	EndExceptionHandler

	RefReturn(result);
}


/*
 * initialization
 */

extern Dict *initprims_dump(Dict *primdict) {
	X(batchloop);
	return primdict;
}
