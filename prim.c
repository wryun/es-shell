/* prim.c -- primitives and primitive dispatching ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "prim.h"

static Dict *prims;

extern List *prim(char *s, List *list, Binding *binding, int evalflags) {
	Prim *p;
	p = (Prim *) dictget(prims, s);
	if (p == NULL)
		fail("es:prim", "unknown primitive: %s", s);
	return (p->prim)(list, binding, evalflags);
}

static const char *list_prefix;

static void listwithprefix(void *arg, char *key, void *value) {
	if (strneq(key, list_prefix, strlen(list_prefix)))
		addtolist(arg, key, value);
}

extern List *primswithprefix(const char *prefix) {
	Ref(List *, primlist, NULL);
	list_prefix = prefix;
	dictforall(prims, listwithprefix, &primlist);
	RefReturn(primlist);
}

PRIM(primitives) {
	static List *primlist = NULL;
	if (primlist == NULL) {
		globalroot(&primlist);
		dictforall(prims, addtolist, &primlist);
		primlist = sortlist(primlist);
	}
	return primlist;
}

extern void initprims(void) {
	prims = mkdict();
	globalroot(&prims);

	prims = initprims_controlflow(prims);
	prims = initprims_io(prims);
	prims = initprims_etc(prims);
	prims = initprims_sys(prims);
	prims = initprims_proc(prims);
	prims = initprims_access(prims);

#define	primdict prims
	X(primitives);
}
