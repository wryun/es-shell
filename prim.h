/* prim.h -- definitions for es primitives */


#define	PRIM(name)	static List *CONCAT(prim_,name)(List *list, Boolean parent, Boolean exitonfalse)
#define	X(name)		(primdict = dictput(primdict, STRING(name), CONCAT(prim_,name)))

extern Dict *initprims_controlflow(Dict *primdict);	/* prim-ctl.c */
extern Dict *initprims_io(Dict *primdict);		/* prim-io.c */
extern Dict *initprims_etc(Dict *primdict);		/* prim-etc.c */
extern Dict *initprims_sys(Dict *primdict);		/* prim-sys.c */
