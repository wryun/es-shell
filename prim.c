/* prim.c -- primitives and primitive dispatching */

#include "es.h"
#include "prim.h"

static Dict *prims;

extern List *prim(char *s, List *list, Boolean parent, Boolean exitonfalse) {
	List *(*p)(List *list, Boolean parent, Boolean exitonfalse);
	p = dictget(prims, s);
	if (p == NULL)
		fail("unknown primitive: %s", s);
	return (*p)(list, parent, exitonfalse);
}

static void addtolist(void *arg, char *key, void *value) {
	List **listp = arg;
	Term *term = mkterm(key, NULL);
	*listp = mklist(term, *listp);
}

PRIM(primitives) {
	static List *primlist = NULL;
	if (list != NULL)
		fail("usage: &$primitives");
	if (primlist == NULL) {
		globalroot(&primlist);
		dictforall(prims, addtolist, &primlist);
		primlist = sortlist(primlist);
	}
	return primlist;
}

PRIM(vars) {
	extern Dict *vars;
	if (list != NULL)
		fail("usage: &$vars");
	Ref(List *, varlist, NULL);
	dictforall(vars, addtolist, &varlist);
	varlist = sortlist(varlist);
	RefReturn(varlist);
}

extern void initprims(void) {
	prims = mkdict();
	globalroot(&prims);

	prims = initprims_controlflow(prims);
	prims = initprims_io(prims);
	prims = initprims_etc(prims);

#define	primdict prims
	X(primitives);
	X(vars);
}
