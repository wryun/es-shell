/* prim-etc.c -- miscellaneous primitives */

#define	REQUIRE_PWD	1

#include "es.h"
#include "prim.h"

PRIM(true) {
	return true;
}

PRIM(false) {
	return false;
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

PRIM(noexport) {
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next)
		noexport(getstr(lp->term));
	RefEnd(lp);
	return true;
}

PRIM(isnoexport) {
	if (list == NULL || list->next != NULL)
		fail("usage: $&isnoexport variable");
	return isnoexport(getstr(list->term)) ? true : false;
}

PRIM(version) {
	return mklist(mkterm((char *) version, NULL), NULL);
}

PRIM(exec) {
	return eval(list, NULL, FALSE, exitonfalse);
}

PRIM(eval) {
	return runstring(str("%L", list, " "), "<eval>", 0);
}

PRIM(dot) {
	int fd;
	List *e;
	Handler h;
	volatile int runflags = 0;

	/* TODO: allow -[xvnL] */

	Ref(List *, result, NULL);
	Ref(List *, lp, list);
	Ref(char *, file, NULL);
	for (;;) {
		if (lp == NULL)
			fail("usage: . [-i] file");
		file = getstr(lp->term);
		lp = lp->next;
		if ((runflags & run_interactive) || !streq(file, "-i"))
			break;
		runflags |= run_interactive;
	}

	fd = eopen(file, oOpen);
	if (fd == -1)
		fail("%s: %s", file, strerror(errno));

	if ((e = pushhandler(&h)) != NULL) {
		varpop("*");
		varpop("0");
		throw(e);
	}

	varpush("*", lp);
	varpush("0", mklist(mkterm(file, NULL), NULL));

	result = runfd(fd, file, runflags);

	pophandler(&h);
	varpop("*");
	varpop("0");
	RefEnd2(file, lp);
	RefReturn(result);
}

PRIM(flatten) {
	char *sep;
	if (list == NULL)
		fail("usage: $&flatten separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = mklist(mkterm(str("%L", lp->next, sep), NULL), NULL);
	RefReturn(lp);
}

PRIM(whatis) {
	if (list == NULL || list->next != NULL)
		fail("usage: $&whatis program");
	Ref(Term *, term, list->term);
	if (term->closure == NULL) {
		Ref(char *, prog, term->str);
		assert(prog != NULL);
		if (isabsolute(prog) || (list = varlookup2("fn-", prog)) == NULL)
			list = mklist(mkterm(which(prog), NULL), NULL);
		RefEnd(prog);
	}
	RefEnd(term);
	return list;
}

PRIM(pathsearch) {
	if (list == NULL || list->next != NULL)
		fail("usage: $&pathsearch program");
	if (list->term->closure != NULL)
		fail("$&pathsearch must be applied to a string not a tree");
	assert(list->term->str != NULL);
	return mklist(mkterm(which(list->term->str), NULL), NULL);
}

PRIM(split) {
	if (list == NULL)
		fail("usage: $&split separator [args ...]");
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
		fail("usage: $&split separator [args ...]");
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
		fail("var: %s is undefined", name);
	rest = prim_var(rest, parent, exitonfalse);
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
					? prim("parse", NULL, TRUE, exitonfalse)
					: eval(parser, NULL, TRUE, exitonfalse);
			SIGCHK();
			if (arg != NULL)
				cmd = append(arg, cmd);
			if (cmd != NULL) {
				result = eval(cmd, NULL, TRUE, exitonfalse);
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
		fail("usage: %home [user]");
	pw = getpwnam(getstr(list->term));
	return (pw == NULL) ? NULL : mklist(mkterm(gcdup(pw->pw_dir), NULL), NULL);
}


/*
 * initialization
 */

extern Dict *initprims_etc(Dict *primdict) {
	X(true);
	X(false);
	X(echo);
	X(count);
	X(noexport);
	X(version);
	X(exec);
	X(eval);
	X(dot);
	X(flatten);
	X(whatis);
	X(pathsearch);
	X(sethistory);
	X(split);
	X(fsplit);
	X(isnoexport);
	X(var);
	X(parse);
	X(batchloop);
	X(collect);
	X(home);
	return primdict;
}
