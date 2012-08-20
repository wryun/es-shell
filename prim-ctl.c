/* prim-ctl.c -- control flow primitives ($Revision: 1.9 $) */

#include "es.h"
#include "prim.h"

PRIM(seq) {
	Ref(List *, result, true);
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next)
		result = eval1(lp->term, evalflags &~ (lp->next == NULL ? 0 : eval_inchild));
	RefEnd(lp);
	if (result == true)
		result = listcopy(true);
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
	return listcopy(true);
}

PRIM(and) {
	Ref(List *, cond, true);
	Ref(List *, lp, list);
	for (; istrue(cond) && lp != NULL; lp = lp->next)
		cond = eval1(lp->term, evalflags & (lp->next == NULL ? eval_inchild : 0));
	RefEnd(lp);
	if (cond == true)
		cond = listcopy(true);
	RefReturn(cond);
}

PRIM(or) {
	Ref(List *, cond, false);
	Ref(List *, lp, list);
	for (; !istrue(cond) && lp != NULL; lp = lp->next)
		cond = eval1(lp->term, evalflags & (lp->next == NULL ? eval_inchild : 0));
	RefEnd(lp);
	if (cond == false)
		cond = listcopy(false);
	RefReturn(cond);
}

PRIM(not) {
	return istrue(eval(list, NULL, 0)) ? false : true;
}

PRIM(while) {
	Handler h;
	List *e;

	if (list == NULL)
		fail("$&while", "usage: while condition body");

	if ((e = pushhandler(&h)) != NULL) {
		if (e->term->str != NULL && streq(e->term->str, "break"))
			return e->next;
		throw(e);
	}

	Ref(List *, result, true);
	Ref(Term *, cond, list->term);
	Ref(List *, body, list->next);
	while (istrue(eval1(cond, 0)))
		result = eval(body, NULL, evalflags & eval_exitonfalse);
	e = (result == true) ? listcopy(true) : result;
	RefEnd3(body, cond, result);
	pophandler(&h);
	return e;
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
	Handler h;
	List *e, *e2;

	if (list == NULL)
		fail("$&catch", "usage: catch catcher body");

	Ref(List *, lp, list);
	while ((e = pushhandler(&h)) != NULL) {
		blocksignals();
		if ((e2 = pushhandler(&h)) == NULL) {
			list = eval(mklist(lp->term, e), NULL, evalflags);
			pophandler(&h);
			RefPop(lp);
			unblocksignals();
			return list;
		} else if (!streq(e2->term->str, "retry"))
			throw(e2);
		unblocksignals();
	}

	lp = eval(lp->next, NULL, evalflags);
	pophandler(&h);
	RefReturn(lp);
}

PRIM(unwindprotect) {
	Handler h;

	if (list == NULL || list->next == NULL || list->next->next != NULL)
		fail("$&unwindprotect", "usage: unwind-protect body cleanup");

	Ref(List *, result, NULL);
	Ref(List *, e, NULL);
	Ref(Term *, cleanup, list->next->term);

	if ((e = pushhandler(&h)) == NULL) {
		result = eval1(list->term, 0);
		pophandler(&h);
	}

	eval1(cleanup, evalflags);
	if (e != NULL)
		throw(e);
	RefEnd2(cleanup, e);
	RefReturn(result);
}

extern Dict *initprims_controlflow(Dict *primdict) {
	X(seq);
	X(if);
	X(and);
	X(or);
	X(not);
	X(throw);
	X(while);
	X(forever);
	X(catch);
	X(unwindprotect);
	return primdict;
}
