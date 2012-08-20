/* prim.h -- definitions for es primitives ($Revision: 1.6 $) */

#define	PRIM(name)	static List *CONCAT(prim_,name)(List *list, int evalflags)
#define	X(name)		(primdict = dictput(primdict, STRING(name), CONCAT(prim_,name)))

extern Dict *initprims_controlflow(Dict *primdict);	/* prim-ctl.c */
extern Dict *initprims_io(Dict *primdict);		/* prim-io.c */
extern Dict *initprims_etc(Dict *primdict);		/* prim-etc.c */
extern Dict *initprims_sys(Dict *primdict);		/* prim-sys.c */
extern Dict *initprims_proc(Dict *primdict);		/* proc.c */
extern Dict *initprims_access(Dict *primdict);		/* access.c */
