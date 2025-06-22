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
	return ltrue;
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

/* If not -DGCVERBOSE or -DGCINFO, these variables are harmlessly useless */
Boolean gcverbose	= FALSE;
Boolean gcinfo		= FALSE;

PRIM(setrunflags) {
	Ref(List *, lp, list);
	for (lp = list; lp != NULL; lp = lp->next) {
		if (termeq(lp->term, "gcverbose"))
			gcverbose = TRUE;
		else if (termeq(lp->term, "gcinfo"))
			gcinfo = TRUE;
	}
	RefEnd(lp);

	setrunflags(runflags_to_int(list));
	return list;
}

PRIM(runfile) {
	int fd = 0;
	if (list == NULL || (list->next != NULL && list->next->next != NULL))
		fail("$&runfile", "usage: $&runfile command [file]");
	Ref(List *, result, NULL);
	Ref(List *, cmd, mklist(list->term, NULL));
	Ref(char *, file, "stdin");

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
	Ref(char *, string, getstr(list->next->term));

	result = runstring(string, cmd);

	RefEnd2(string, cmd);
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

static void loginput(char *input) {
	char *c;
	List *fn = varlookup("fn-%write-history", NULL);
	/* TODO: fix this for isinteractive() or !isfromfd() */
	if (fn == NULL)
		return;
	for (c = input;; c++)
		switch (*c) {
		case '#': case '\n': return;
		case ' ': case '\t': break;
		default: goto writeit;
		}
writeit:
	gcdisable();
	Ref(List *, list, append(fn, mklist(mkstr(input), NULL)));
	gcenable();
	eval(list, NULL, 0);
	RefEnd(list);
}

PRIM(parse) {
	List *result;
	Ref(char *, prompt1, NULL);
	Ref(char *, prompt2, NULL);
	Ref(List *, lp, list);
	if (lp != NULL) {
		prompt1 = getstr(lp->term);
		if ((lp = lp->next) != NULL)
			prompt2 = getstr(lp->term);
	}
	RefEnd(lp);
	newhistbuffer();

	Ref(Tree *, tree, NULL);
	ExceptionHandler
		tree = parse(prompt1, prompt2);
	CatchException (ex)
		Ref(List *, e, ex);
		loginput(dumphistbuffer());
		throw(e);
		RefEnd(e);
	EndExceptionHandler

	loginput(dumphistbuffer());
	result = (tree == NULL)
		   ? NULL
		   : mklist(mkterm(NULL, mkclosure(gcmk(nThunk, tree), NULL)),
			    NULL);
	RefEnd3(tree, prompt2, prompt1);
	return result;
}

PRIM(exitonfalse) {
	return eval(list, NULL, evalflags | eval_exitonfalse);
}

PRIM(collect) {
	gc();
	return ltrue;
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

#ifdef noreturn
#undef noreturn
#endif
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

static Boolean didonce = FALSE;
PRIM(importenvfuncs) {
	if (didonce)
		return lfalse;

	importenv(TRUE);
	didonce = TRUE;

	return ltrue;
}

PRIM(getpid) {
	return mklist(mkstr(str("%d", getpid())), NULL);
}

#if HAVE_READLINE
PRIM(sethistory) {
	if (list == NULL) {
		sethistory(NULL);
		return NULL;
	}
	Ref(List *, lp, list);
	sethistory(getstr(lp->term));
	RefReturn(lp);
}

PRIM(writehistory) {
	if (list == NULL || list->next != NULL)
		fail("$&writehistory", "usage: $&writehistory command");
	loghistory(getstr(list->term));
	return NULL;
}

PRIM(setmaxhistorylength) {
	char *s;
	int n;
	if (list == NULL) {
		setmaxhistorylength(-1); /* unlimited */
		return NULL;
	}
	if (list->next != NULL)
		fail("$&setmaxhistorylength", "usage: $&setmaxhistorylength [limit]");
	Ref(List *, lp, list);
	n = (int)strtol(getstr(lp->term), &s, 0);
	if (n < 0 || (s != NULL && *s != '\0'))
		fail("$&setmaxhistorylength", "max-history-length must be set to a positive integer");
	setmaxhistorylength(n);
	RefReturn(lp);
}

PRIM(resetterminal) {
	resetterminal = TRUE;
	return ltrue;
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
	X(getpid);
#if HAVE_READLINE
	X(sethistory);
	X(writehistory);
	X(resetterminal);
	X(setmaxhistorylength);
#endif
	return primdict;
}
