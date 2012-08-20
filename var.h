/* var.h -- es variables ($Revision: 1.1.1.1 $) */

typedef struct Var Var;
struct Var {
	List *defn;
	char *env;
	int flags;
};

#define	var_hasbindings		1
#define	var_isinternal		2

extern Dict *vars;
