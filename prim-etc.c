/* prim-etc.c -- miscellaneous primitives ($Revision: 1.2 $) */

#define	REQUIRE_PWD	1

#include "es.h"
#include "prim.h"

PRIM(result) {
	return list;
}

PRIM(echo) {
	const char *eol = "\n";
	if (list != NULL) {
		if (termeq(list->term, "-n")) {
			eol = "";
			list = list->next;
		} else if (termeq(list->term, "--"))
			list = list->next;
	}
	print("%L%s", list, " ", eol);
	return true;
}

PRIM(count) {
	return mklist(mkstr(str("%d", length(list))), NULL);
}

PRIM(setnoexport) {
	Ref(List *, lp, list);
	setnoexport(lp);
	RefReturn(lp);
}

PRIM(version) {
	return mklist(mkstr((char *) version), NULL);
}

PRIM(exec) {
	return eval(list, NULL, evalflags | eval_inchild);
}

#if GCVERBOSE
Boolean gcverbose	= FALSE;
#endif
#if GCINFO
Boolean gcinfo		= FALSE;
#endif

PRIM(setrunflags) {
#if GCVERBOSE || GCINFO
	Boolean gcv = FALSE;
	Boolean gci = FALSE;
	Ref(List *, lp, list);
		for (lp = list; lp != NULL; lp = lp->next) {
		if (termeq(lp->term, "gcverbose"))
			gcv = TRUE;
		else if (termeq(lp->term, "gcinfo"))
			gci = TRUE;
	}
	RefEnd(lp);

#if GCVERBOSE
	gcverbose = gcv;
#endif
#if GCINFO
	gcinfo = gci;
#endif
#endif
	setrunflags(runflags_to_int(list));
	return list;
}

PRIM(runfile) {
	if (list == NULL || (list->next != NULL && list->next->next != NULL))
		fail("$&runinput", "usage: $&runinput command [file]");
	Ref(List *, result, NULL);
	Ref(List *, cmd, mklist(list->term, NULL));
	Ref(char *, file, "stdin");

	int fd = 0;

	if (list->next != NULL) {
		file = getstr(list->next->term);
		fd = eopen(file, oOpen);
		if (fd == -1)
			fail("$&runfile", "%s: %s", file, esstrerror(errno));
	}

	result = runfd(fd, file, cmd);

	RefEnd2(file, cmd);
	RefReturn(result);
}

PRIM(runstring) {
	if (list == NULL || list->next == NULL || list->next->next != NULL)
		fail("$&runstring", "usage: $&runstring command string");
	Ref(List *, result, NULL);
	Ref(List *, cmd, mklist(list->term, NULL));
	char *string = mprint(getstr(list->next->term));

	result = runstring(string, cmd);

	efree(string);
	RefEnd(cmd);
	RefReturn(result);
}

PRIM(flatten) {
	char *sep;
	if (list == NULL)
		fail("$&flatten", "usage: %%flatten separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = mklist(mkstr(str("%L", lp->next, sep)), NULL);
	RefReturn(lp);
}

PRIM(whatis) {
	/* the logic in here is duplicated in eval() */
	if (list == NULL || list->next != NULL)
		fail("$&whatis", "usage: $&whatis program");
	Ref(Term *, term, list->term);
	if (getclosure(term) == NULL) {
		List *fn;
		Ref(char *, prog, getstr(term));
		assert(prog != NULL);
		fn = varlookup2("fn-", prog, binding);
		if (fn != NULL)
			list = fn;
		else {
			if (isabsolute(prog)) {
				char *error = checkexecutable(prog);
				if (error != NULL)
					fail("$&whatis", "%s: %s", prog, error);
			} else
				list = pathsearch(term);
		}
		RefEnd(prog);
	}
	RefEnd(term);
	return list;
}

PRIM(split) {
	char *sep;
	if (list == NULL)
		fail("$&split", "usage: %%split separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = fsplit(sep, lp->next, TRUE);
	RefReturn(lp);
}

PRIM(fsplit) {
	char *sep;
	if (list == NULL)
		fail("$&fsplit", "usage: %%fsplit separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = fsplit(sep, lp->next, FALSE);
	RefReturn(lp);
}

PRIM(var) {
	Term *term;
	if (list == NULL)
		return NULL;
	Ref(List *, rest, list->next);
	Ref(char *, name, getstr(list->term));
	Ref(List *, defn, varlookup(name, NULL));
	rest = prim_var(rest, NULL, evalflags);
	term = mkstr(str("%S = %#L", name, defn, " "));
	list = mklist(term, rest);
	RefEnd3(defn, name, rest);
	return list;
}

PRIM(sethistory) {
	if (list == NULL) {
		sethistory(NULL);
		return NULL;
	}
	Ref(List *, lp, list);
	sethistory(getstr(lp->term));
	RefReturn(lp);
}

PRIM(parse) {
	List *result;
	Tree *tree;
	Ref(char *, prompt1, NULL);
	Ref(char *, prompt2, NULL);
	Ref(List *, lp, list);
	if (lp != NULL) {
		prompt1 = getstr(lp->term);
		if ((lp = lp->next) != NULL)
			prompt2 = getstr(lp->term);
	}
	RefEnd(lp);
	tree = parse(prompt1, prompt2);
	result = (tree == NULL)
		   ? NULL
		   : mklist(mkterm(NULL, mkclosure(mk(nThunk, tree), NULL)),
			    NULL);
	RefEnd2(prompt2, prompt1);
	return result;
}

PRIM(exitonfalse) {
	return eval(list, NULL, evalflags | eval_exitonfalse);
}

PRIM(collect) {
	gc();
	return true;
}

PRIM(home) {
	struct passwd *pw;
	if (list == NULL)
		return varlookup("home", NULL);
	if (list->next != NULL)
		fail("$&home", "usage: %%home [user]");
	pw = getpwnam(getstr(list->term));
	return (pw == NULL) ? NULL : mklist(mkstr(gcdup(pw->pw_dir)), NULL);
}

PRIM(vars) {
	return listvars(FALSE);
}

PRIM(internals) {
	return listvars(TRUE);
}

PRIM(noreturn) {
	if (list == NULL)
		fail("$&noreturn", "usage: $&noreturn lambda args ...");
	Ref(List *, lp, list);
	Ref(Closure *, closure, getclosure(lp->term));
	if (closure == NULL || closure->tree->kind != nLambda)
		fail("$&noreturn", "$&noreturn: %E is not a lambda", lp->term);
	Ref(Tree *, tree, closure->tree);
	Ref(Binding *, context, bindargs(tree->u[0].p, lp->next, closure->binding));
	lp = walk(tree->u[1].p, context, evalflags);
	RefEnd3(context, tree, closure);
	RefReturn(lp);
}

PRIM(setmaxevaldepth) {
	char *s;
	long n;
	if (list == NULL) {
		maxevaldepth = MAXmaxevaldepth;
		return NULL;
	}
	if (list->next != NULL)
		fail("$&setmaxevaldepth", "usage: $&setmaxevaldepth [limit]");
	Ref(List *, lp, list);
	n = strtol(getstr(lp->term), &s, 0);
	if (n < 0 || (s != NULL && *s != '\0'))
		fail("$&setmaxevaldepth", "max-eval-depth must be set to a positive integer");
	if (n < MINmaxevaldepth)
		n = (n == 0) ? MAXmaxevaldepth : MINmaxevaldepth;
	maxevaldepth = n;
	RefReturn(lp);
}

PRIM(importenvfuncs) {
	importenv(TRUE);
	return true;
}

#if READLINE
PRIM(resetterminal) {
	resetterminal = TRUE;
	return true;
}
#endif


/*
 * initialization
 */

extern Dict *initprims_etc(Dict *primdict) {
	X(echo);
	X(count);
	X(version);
	X(exec);
	X(setrunflags);
	X(runfile);
	X(runstring);
	X(flatten);
	X(whatis);
	X(sethistory);
	X(split);
	X(fsplit);
	X(var);
	X(parse);
	X(collect);
	X(home);
	X(setnoexport);
	X(vars);
	X(internals);
	X(result);
	X(exitonfalse);
	X(noreturn);
	X(setmaxevaldepth);
	X(importenvfuncs);
#if READLINE
	X(resetterminal);
#endif
	return primdict;
}
