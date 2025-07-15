/* prim-dump.c -- primitives for the esdump step ($Revision: 1.0 $) */

#include "es.h"
#include "prim.h"

PRIM(batchloop) {
	Ref(List *, result, ltrue);

	ExceptionHandler

		for (;;) {
			List *cmd = prim("parse", NULL, NULL, 0);
			if (cmd != NULL)
				result = eval(cmd, NULL, evalflags);
			SIGCHK();
		}

	CatchException (e)

		if (!termeq(e->term, "eof"))
			throw(e);

	EndExceptionHandler

	RefReturn(result);
}

PRIM(defaultpath) {
	int i;
	static const char * const path[] = { INITIAL_PATH };

	Ref(List *, list, NULL);
	for (i = arraysize(path); i-- > 0;) {
		Term *t = mkstr((char *) path[i]);
		list = mklist(t, list);
	}
	RefReturn(list);
}

PRIM(usage) {
	static char *usage =
		"usage: es [-c command] [-silevxnpo] [file [args ...]]\n"
		"	-c cmd	execute argument\n"
		"	-s	read commands from standard input; stop option parsing\n"
		"	-i	interactive shell\n"
		"	-l	login shell\n"
		"	-e	exit if any command exits with false status\n"
		"	-v	print input to standard error\n"
		"	-x	print commands to standard error before executing\n"
		"	-n	just parse; don't execute\n"
		"	-p	don't load functions from the environment\n"
		"	-o	don't open stdin, stdout, and stderr if they were closed\n"
		"	-d	don't ignore SIGQUIT or SIGTERM"
	;
	static char *optusage = "\n"
#if GCINFO
		"	-I	print garbage collector information"
#endif
#if GCVERBOSE
		"\n	-G	print verbose garbage collector information"
#endif
#if LISPTREES
		"\n	-L	print parser results in LISP format"
#endif
	;
	return mklist(mkstr(str("%s%s", usage, optusage)), NULL);
}

PRIM(conditionalflags) {
	Ref(List *, list, NULL);
	Ref(Term *, t, NULL);
#if GCINFO
	t = mkstr("gcinfo");	list = mklist(t, list);
	t = mkstr("I");		list = mklist(t, list);
#endif
#if GCVERBOSE
	t = mkstr("gcverbose");	list = mklist(t, list);
	t = mkstr("G");		list = mklist(t, list);
#endif
#if LISPTREES
	t = mkstr("lisptrees");	list = mklist(t, list);
	t = mkstr("L");		list = mklist(t, list);
#endif
	RefEnd(t);
	RefReturn(list);
}


/*
 * initialization
 */

extern Dict *initprims_dump(Dict *primdict) {
	X(batchloop);
	X(defaultpath);
	X(usage);
	X(conditionalflags);
	return primdict;
}
