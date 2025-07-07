/* prim.h -- definitions for es primitives ($Revision: 1.1.1.1 $) */

typedef struct { List *(*prim)(List *, Binding *, int); } Prim;

#define	PRIM(name)	static List *CONCAT(prim_,name)( \
				List UNUSED *list, Binding UNUSED *binding, int UNUSED evalflags \
			)
#define	X(name)		STMT( \
			static Prim CONCAT(prim_struct_,name) = { CONCAT(prim_,name) }; \
			primdict = dictput( \
				primdict, \
				STRING(name), \
				(void *) &CONCAT(prim_struct_,name) \
			))

extern Dict *initprims_controlflow(Dict *primdict);	/* prim-ctl.c */
extern Dict *initprims_io(Dict *primdict);		/* prim-io.c */
extern Dict *initprims_etc(Dict *primdict);		/* prim-etc.c */
extern Dict *initprims_sys(Dict *primdict);		/* prim-sys.c */
extern Dict *initprims_proc(Dict *primdict);		/* proc.c */
extern Dict *initprims_access(Dict *primdict);		/* access.c */
