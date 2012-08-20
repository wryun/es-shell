/* prim.c -- primitives and primitive dispatching */

#include "es.h"
#include "prim.h"

static Dict *prims;

extern List *prim(char *s, List *list, Boolean parent) {
	List *(*p)(List *list, Boolean parent);
	debug("<< prim : %s >>\n", s);
	p = dictget(prims, s);
	if (p == NULL)
		fail("unknown primitive: %s", s);
	return (*p)(list, parent);
}

static void printprim(void *ignore, char *key, void *value) {
	print("%s\n", key);
}

PRIM(primitives) {
	dictforall(prims, printprim, NULL);
	return true;
}

extern void initprims(void) {
	prims = mkdict();
	globalroot(&prims);

	prims = initprims_controlflow(prims);
	prims = initprims_io(prims);
	prims = initprims_etc(prims);

#define primdict prims
	X(primitives);
}
