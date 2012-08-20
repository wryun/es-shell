/* prim.h -- definitions for es primitives */


#define	PRIM(name)	static List *prim_##name(List *list, Boolean parent)
#define	X(name)		(primdict = dictput(primdict, #name, prim_##name))

extern Dict *initprims_controlflow(Dict *primdict);	/* prim-ctl.c */
extern Dict *initprims_io(Dict *primdict);		/* prim-io.c */
extern Dict *initprims_etc(Dict *primdict);		/* prim-etc.c */
