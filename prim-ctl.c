/* prim-ctl.c -- control flow primitives ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "prim.h"

PRIM(seq) {
	Ref(List *, result, true);
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next)
		result = eval1(lp->term, evalflags &~ (lp->next == NULL ? 0 : eval_inchild));
	RefEnd(lp);
	RefReturn(result);
}

PRIM(if) {
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		List *cond = eval1(lp->term, evalflags & (lp->next == NULL ? eval_inchild : 0));
		lp = lp->next;
		if (lp == NULL) {
			RefPop(lp);
			return cond;
		}
		if (istrue(cond)) {
			List *result = eval1(lp->term, evalflags);
			RefPop(lp);
			return result;
		}
	}
	RefEnd(lp);
	return true;
}

PRIM(forever) {
	Ref(List *, body, list);
	for (;;)
		list = eval(body, NULL, evalflags & eval_exitonfalse);
	RefEnd(body);
	return list;
}

PRIM(throw) {
	if (list == NULL)
		fail("$&throw", "usage: throw exception [args ...]");
	throw(list);
	NOTREACHED;
}

PRIM(catch) {
	Atomic retry;

	if (list == NULL)
		fail("$&catch", "usage: catch catcher body");

	Ref(List *, result, NULL);
	Ref(List *, lp, list);

	do {
		retry = FALSE;

		ExceptionHandler

			result = eval(lp->next, NULL, evalflags);

		CatchException (frombody)

			blocksignals();
			ExceptionHandler
				result
				  = eval(mklist(mkstr("$&noreturn"),
					        mklist(lp->term, frombody)),
					 NULL,
					 evalflags);
				unblocksignals();
			CatchException (fromcatcher)

				if (termeq(fromcatcher->term, "retry")) {
					retry = TRUE;
					unblocksignals();
				} else
					throw(fromcatcher);
			EndExceptionHandler

		EndExceptionHandler
	} while (retry);
	RefEnd(lp);
	RefReturn(result);
}

extern Dict *initprims_controlflow(Dict *primdict) {
	X(seq);
	X(if);
	X(throw);
	X(forever);
	X(catch);
	return primdict;
}
