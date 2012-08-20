/* prim-ctl.c -- control flow primitives */

#include "es.h"
#include "prim.h"

PRIM(seq) {
	Ref(List *, result, true);
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next)
		result = eval1(lp->term, lp->next == NULL ? parent : TRUE);
	RefEnd(lp);
	RefReturn(result);
}

PRIM(if) {
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		List *cond = eval1(lp->term, lp->next == NULL ? parent : TRUE);
		lp = lp->next;
		if (lp == NULL) {
			RefPop(lp);
			return cond;
		}
		if (istrue(cond)) {
			List *result = eval1(lp->term, parent);
			RefPop(lp);
			return result;
		}
	}
	RefEnd(lp);
	return true;
}

PRIM(and) {
	Ref(List *, cond, true);
	Ref(List *, lp, list);
	for (; istrue(cond) && lp != NULL; lp = lp->next)
		cond = eval1(lp->term, lp->next == NULL ? parent : TRUE);
	RefEnd(lp);
	RefReturn(cond);
}

PRIM(or) {
	Ref(List *, cond, false);
	Ref(List *, lp, list);
	for (; !istrue(cond) && lp != NULL; lp = lp->next)
		cond = eval1(lp->term, lp->next == NULL ? parent : TRUE);
	RefEnd(lp);
	RefReturn(cond);
}

PRIM(not) {
	return istrue(eval(list, NULL, parent)) ? false : true;
}

PRIM(while) {
	Handler h;
	List *e;

	if (list == NULL)
		fail("usage: while condition body");

	if ((e = pushhandler(&h)) != NULL) {
		if (e->term->str != NULL && streq(e->term->str, "break"))
			return e->next;
		throw(e);
	}

	Ref(List *, result, true);
	Ref(Term *, cond, list->term);
	Ref(List *, body, list->next);
	while (istrue(eval1(cond, TRUE)))
		result = eval(body, NULL, TRUE);
	e = result;
	RefEnd3(body, cond, result);
	pophandler(&h);
	return e;
}

PRIM(throw) {
	if (list == NULL)
		fail("usage: throw exception [args ...]");
	throw(list);
}

PRIM(catch) {
	Handler h;
	List *e, *e2;

	if (list == NULL)
		fail("usage: catch catcher body");

	Ref(List *, lp, list);
	while ((e = pushhandler(&h)) != NULL)
		if ((e2 = pushhandler(&h)) == NULL) {
			list = eval(mklist(lp->term, e), NULL, TRUE);
			pophandler(&h);
			RefPop(lp);
			return list;
		} else if (!streq(e2->term->str, "retry"))
			throw(e2);

	lp = eval(lp->next, NULL, TRUE);
	pophandler(&h);
	RefReturn(lp);
}

extern Dict *initprims_controlflow(Dict *primdict) {
	X(seq);
	X(if);
	X(and);
	X(or);
	X(not);
	X(throw);
	X(while);
	X(catch);
	return primdict;
}
