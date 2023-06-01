/* prim-etc.c -- miscellaneous primitives ($Revision: 1.2 $) */

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
		if (result == true)
			result = true;
		RefReturn(result);

	EndExceptionHandler
}


/*
 * initialization
 */

extern Dict *initprims_dump(Dict *primdict) {
	X(batchloop);
	return primdict;
}
