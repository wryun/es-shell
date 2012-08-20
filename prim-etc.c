/* prim-etc.c -- miscellaneous primitives ($Revision: 1.11 $) */

#define	REQUIRE_PWD	1

#include "es.h"
#include "prim.h"

PRIM(true) {
	return true;
}

PRIM(false) {
	return false;
}

PRIM(result) {
	return list;
}

PRIM(echo) {
	const char *eol = "\n";
	if (list != NULL) {
		const char *opt;
		assert(list->term != NULL);
		opt = list->term->str;
		if (opt != NULL && opt[0] == '-' && opt[2] == '\0') {
			switch (opt[1]) {
			case 'n':
				eol = "";
				list = list->next;
				break;
			case '-':
				list = list->next;
				break;
			default:
				break;
			}
		}
	}
	print("%L%s", list, " ", eol);
	return true;
}

PRIM(count) {
	return mklist(mkterm(str("%d", length(list)), NULL), NULL);
}

PRIM(setnoexport) {
	Ref(List *, lp, list);
	setnoexport(lp);
	RefReturn(lp);
}

PRIM(version) {
	return mklist(mkterm((char *) version, NULL), NULL);
}

PRIM(exec) {
	return eval(list, NULL, evalflags | eval_inchild);
}

PRIM(eval) {
	return runstring(str("%L", list, " "), "<eval>", evalflags);
}

PRIM(dot) {
	int c, fd;
	Push zero, star;
	volatile int runflags = (evalflags & eval_inchild);
	const char * const usage = ". [-invx] file [arg ...]";

	esoptbegin(list, "$&dot", usage);
	while ((c = esopt("einvx")) != EOF)
		switch (c) {
		case 'e':	runflags |= eval_exitonfalse;	break;
		case 'i':	runflags |= run_interactive;	break;
		case 'n':	runflags |= run_noexec;		break;
		case 'v':	runflags |= run_echoinput;	break;
		case 'x':	runflags |= run_printcmds;	break;
		}

	Ref(List *, result, NULL);
	Ref(List *, lp, esoptend());
	if (lp == NULL)
		fail("$&dot", "usage: %s", usage);

	Ref(char *, file, getstr(lp->term));
	lp = lp->next;
	fd = eopen(file, oOpen);
	if (fd == -1)
		fail("$&dot", "%s: %s", file, strerror(errno));

	varpush(&star, "*", lp);
	varpush(&zero, "0", mklist(mkterm(file, NULL), NULL));

	result = runfd(fd, file, runflags);

	varpop(&zero);
	varpop(&star);
	RefEnd2(file, lp);
	RefReturn(result);
}

PRIM(flatten) {
	char *sep;
	if (list == NULL)
		fail("$&flatten", "usage: %flatten separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = mklist(mkterm(str("%L", lp->next, sep), NULL), NULL);
	RefReturn(lp);
}

PRIM(whatis) {
	/* the logic in here is duplicated in eval() */
	if (list == NULL || list->next != NULL)
		fail("$&whatis", "usage: $&whatis program");
	Ref(Term *, term, list->term);
	if (term->closure == NULL) {
		Ref(char *, prog, term->str);
		assert(prog != NULL);
		if (!isabsolute(prog) && (list = varlookup2("fn-", prog)) == NULL)
			list = pathsearch(term);
		RefEnd(prog);
	}
	RefEnd(term);
	return list;
}

PRIM(split) {
	if (list == NULL)
		fail("$&split", "usage: %split separator [args ...]");
	Ref(List *, lp, list);
	startsplit(getstr(lp->term), TRUE);
	while ((lp = lp->next) != NULL) {
		char *s = getstr(lp->term);
		splitstring(s, strlen(s), TRUE);
	}
	RefEnd(lp);
	return endsplit();
}

PRIM(fsplit) {
	char *sep;
	if (list == NULL)
		fail("$&fsplit", "usage: %fsplit separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = fsplit(sep, lp->next);
	RefReturn(lp);
}

PRIM(var) {
	Term *term;
	if (list == NULL)
		return NULL;
	Ref(List *, rest, list->next);
	Ref(char *, name, getstr(list->term));
	Ref(List *, defn, varlookup(name, NULL));
	if (defn == NULL)
		fail("$&var", "%s is undefined", name);
	rest = prim_var(rest, evalflags);
	term = mkterm(str("%S = %#L", name, defn, " "), NULL);
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
			: mklist(mkterm(NULL, mkclosure(mk(nThunk, tree), NULL)), NULL);
	RefEnd2(prompt2, prompt1);
	return result;
}

PRIM(exitonfalse) {
	return eval(list, NULL, evalflags | eval_exitonfalse);
}

PRIM(batchloop) {
	Handler h;
	List *e;
	Ref(List *, result, true);
	Ref(List *, arg, list);

	SIGCHK();
	if ((e = pushhandler(&h)) == NULL)
		for (;;) {
			List *parser, *cmd;
			parser = varlookup("fn-%parse", NULL);
			cmd = (parser == NULL)
					? prim("parse", NULL, 0)
					: eval(parser, NULL, 0);
			SIGCHK();
			if (arg != NULL)
				cmd = append(arg, cmd);
			if (cmd != NULL) {
				result = eval(cmd, NULL, evalflags);
				SIGCHK();
			}
		}

	if (e->term == NULL || e->term->str == NULL || !streq(e->term->str, "eof"))
		throw(e);
	RefEnd(arg);
	RefReturn(result);
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
		fail("$&home", "usage: %home [user]");
	pw = getpwnam(getstr(list->term));
	return (pw == NULL) ? NULL : mklist(mkterm(gcdup(pw->pw_dir), NULL), NULL);
}

PRIM(vars) {
	return listvars(FALSE);
}

PRIM(internals) {
	return listvars(TRUE);
}

PRIM(isinteractive) {
	return isinteractive() ? true : false;
}


/*
 * initialization
 */

extern Dict *initprims_etc(Dict *primdict) {
	X(true);
	X(false);
	X(echo);
	X(count);
	X(version);
	X(exec);
	X(eval);
	X(dot);
	X(flatten);
	X(whatis);
	X(sethistory);
	X(split);
	X(fsplit);
	X(var);
	X(parse);
	X(batchloop);
	X(collect);
	X(home);
	X(setnoexport);
	X(vars);
	X(internals);
	X(result);
	X(isinteractive);
	X(exitonfalse);
	return primdict;
}
