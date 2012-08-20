/* eval.c -- evaluation of lists and trees ($Revision: 1.17 $) */

#include "es.h"

static noreturn failexec(char *file, List *args) {
	List *fn;
	assert(gcisblocked());
	fn = varlookup("fn-%exec-failure", NULL);
	if (fn != NULL) {
		int olderror = errno;
		Ref(List *, list, append(fn, mklist(mkterm(file, NULL), args)));
		RefAdd(file);
		gcenable();
		RefRemove(file);
		eval(list, NULL, 0);
		RefEnd(list);
		errno = olderror;
	}
	eprint("%s: %s\n", file, strerror(errno));
	exit(1);
}

/* forkexec -- fork (if necessary) and exec */
extern List *forkexec(char *file, List *list, Boolean inchild) {
	int pid, status;
	Vector *env;
	gcdisable(0);
	env = mkenv();
	pid = efork(!inchild, FALSE);
	if (pid == 0) {
		execve(file, vectorize(list)->vector, env->vector);
		failexec(file, list);
	}
	gcenable();
	status = ewaitfor(pid);
	if ((status & 0xff) == 0) {
		sigint_newline = FALSE;
		SIGCHK();
		sigint_newline = TRUE;
	} else
		SIGCHK();
	printstatus(0, status);
	return mklist(mkterm(mkstatus(status), NULL), NULL);
}

/* bindargs -- bind an argument list to the parameters of a lambda */
extern Binding *bindargs(Tree *params, List *args, Binding *binding) {
	if (params == NULL)
		return mkbinding("*", args, binding);

	gcdisable(0);

	for (; params != NULL; params = params->u[1].p) {
		Tree *param;
		List *value;
		assert(params->kind == nList);
		param = params->u[0].p;
		assert(param->kind == nWord || param->kind == nQword);
		if (args == NULL)
			value = NULL;
		else if (params->u[1].p == NULL || args->next == NULL) {
			value = args;
			args = NULL;
		} else {
			value = mklist(args->term, NULL);
			args = args->next;
		}
		binding = mkbinding(param->u[0].s, value, binding);
	}

	Ref(Binding *, result, binding);
	gcenable();
	RefReturn(result);
}

/* pathsearch -- evaluate fn %pathsearch + some argument */
extern List *pathsearch(Term *term) {
	List *search, *list;
	search = varlookup("fn-%pathsearch", NULL);
	if (search == NULL)
		fail("es:pathsearch", "%E: fn %%pathsearch undefined", term);
	list = mklist(term, NULL);
	list = eval(append(search, list), NULL, 0);
	return list;
}

/* eval -- evaluate a list, producing a list */
extern List *eval(List *list, Binding *binding0, int flags) {
	Closure *cp;
	List *fn;

	Ref(char *, funcname, NULL);

restart:
	if (list == NULL) {
		RefPop(funcname);
		return listcopy(true);
	}

	Ref(List *, lp, list);
	Ref(Binding *, binding, binding0);
	assert(lp->term != NULL);

	if ((cp = getclosure(lp->term)) != NULL) {
		switch (cp->tree->kind) {
		case nPrim:
			assert(cp->binding == NULL);
			lp = prim(cp->tree->u[0].s, lp->next, flags);
			break;
		case nThunk:
			lp = walk(cp->tree->u[0].p, cp->binding, flags);
			break;
		case nLambda: {
			Push p;
			List *e;
			Handler h;
			if ((e = pushhandler(&h)) != NULL) {
				if (e->term->str != NULL && streq(e->term->str, "return")) {
					lp = e->next;
					goto done;
				}
				throw(e);
			}
			Ref(Tree *, tree, cp->tree);
			Ref(Binding *, context, bindargs(tree->u[0].p, lp->next, cp->binding));
			if (funcname != NULL)
				varpush(&p, "0", mklist(mkterm(funcname, NULL), NULL));
			lp = walk(tree->u[1].p, context, flags);
			if (funcname != NULL)
				varpop(&p);
			RefEnd2(context, tree);
			pophandler(&h);
			break;
		}
		case nList: {
			list = glom(cp->tree, cp->binding, TRUE);
			list = append(list, lp->next);
			RefPop2(binding, lp)
			goto restart;
		}
		default:
			panic("eval: bad closure node kind %d", cp->tree->kind);
		}
		goto done;
	}

	/* the logic here is duplicated in $&whatis */

	if (isabsolute(lp->term->str)) {
		char *error = checkexecutable(lp->term->str);
		if (error != NULL)
			fail("$&whatis", "%s: %s", lp->term->str, error);
		lp = forkexec(lp->term->str, lp, flags & eval_inchild);
		goto done;
	}

	fn = varlookup2("fn-", lp->term->str);
	if (fn != NULL) {
		funcname = lp->term->str;
		list = append(fn, lp->next);
		RefPop2(binding, lp);
		goto restart;
	}

	fn = pathsearch(lp->term);
	if (fn != NULL && fn->next == NULL && fn->term->closure == NULL) {
		char *name = fn->term->str;
		lp = forkexec(name, lp, flags & eval_inchild);
		goto done;
	}

	list = append(fn, lp->next);
	RefPop2(binding, lp);
	goto restart;

done:
	if ((flags & eval_exitonfalse) && !istrue(lp))
		exit(exitstatus(lp));
	list = lp;
	RefEnd3(binding, lp, funcname);
	return list;
}

/* eval1 -- evaluate a term, producing a list */
extern List *eval1(Term *term, int flags) {
	return eval(mklist(term, NULL), NULL, flags);
}

/* local -- perform a single local assignment & recurse	or just dispatch a binding */
static List *local(Tree *defn, Tree *body, Binding *binding, int evalflags) {
	Push p;
	char *name;
	List *list;

	for (;; defn = defn->u[1].p) {
		if (defn == NULL)
			return walk(body, binding, evalflags);
		assert(defn->kind == nList);
		if (defn->u[0].p != NULL)
			break;
	}

	Ref(List *, result, NULL);
	RefAdd3(defn, body, binding);
	Ref(Tree *, assign, defn->u[0].p);
	assert(assign->kind == nAssign);
	Ref(char *, var, varname(glom(assign->u[0].p, binding, FALSE)));
	list = glom(assign->u[1].p, binding, TRUE);
	name = var;
	RefEnd2(var, assign);

	varpush(&p, name, list);
	result = local(defn->u[1].p, body, binding, evalflags);
	varpop(&p);

	RefRemove3(binding, body, defn);
	RefReturn(result);
}


/* walk -- walk through a tree, evaluating nodes */
extern List *walk(Tree *tree0, Binding *binding0, int flags) {
	Tree *volatile tree = tree0;
	Binding *volatile binding = binding0;

	SIGCHK();

top:
	if (tree == NULL)
		return listcopy(true);

	switch (tree->kind) {

	case nConcat: case nList: case nQword: case nVar: case nVarsub: case nWord:
	case nThunk: case nLambda: case nCall: case nPrim: {
		List *list;
		Ref(Binding *, bp, binding);
		list = glom(tree, binding, TRUE);
		binding = bp;
		RefEnd(bp);
		list = eval(list, binding, flags);
		return list;
	}

	case nMatch: {
		Boolean result;
		List *pattern;
		StrList *quote = NULL;
		Ref(Binding *, bp, binding);
		Ref(Tree *, tp, tree);
		Ref(List *, subject, glom(tp->u[0].p, bp, TRUE));
		pattern = glom2(tp->u[1].p, bp, &quote);
		result = listmatch(subject, pattern, quote);
		RefEnd3(subject, tp, bp);
		return listcopy(result ? true : false);
	}

	case nAssign: {
		Ref(Binding *, bp, binding);
		Ref(Tree *, tp, tree);
		Ref(char *, var, varname(glom(tp->u[0].p, bp, FALSE)));
		Ref(List *, value, glom(tp->u[1].p, bp, TRUE));
		vardef(var, bp, value);
		RefEnd4(value, var, tp, bp);
		return listcopy(true);
	}

	case nLet: case nClosure: {
		Ref(Binding *, bp, binding);
		Ref(Tree *, body, tree->u[1].p);
		Ref(Tree *, defn, tree->u[0].p);
		for (; defn != NULL; defn = defn->u[1].p) {
			assert(defn->kind == nList);
			if (defn->u[0].p == NULL)
				continue;
			Ref(Tree *, assign, defn->u[0].p);
			assert(assign->kind == nAssign);
			Ref(char *, var, varname(glom(assign->u[0].p, bp, FALSE)));
			Ref(List *, lp, glom(assign->u[1].p, bp, TRUE));
			bp = mkbinding(var, lp, bp);
			RefEnd3(lp, var, assign);
		}
		RefEnd(defn);
		tree = body;
		binding = bp;
		RefEnd2(body, bp);
		goto top;
	}

	case nLocal:
		return local(tree->u[0].p, tree->u[1].p, binding, flags);

	case nFor: {
		Handler h;
		List *e;

		if ((e = pushhandler(&h)) != NULL) {
			if (e->term->str != NULL && streq(e->term->str, "break"))
				return e->next;
			throw(e);
		}

		Ref(List *, result, true);
		Ref(Binding *, outer, binding);
		Ref(Tree *, body, tree->u[1].p);
		Ref(Binding *, looping, NULL);
		Ref(Tree *, defn, tree->u[0].p);

		for (; defn != NULL; defn = defn->u[1].p) {
			assert(defn->kind == nList);
			if (defn->u[0].p == NULL)
				continue;
			Ref(Tree *, assign, defn->u[0].p);
			assert(assign->kind == nAssign);
			Ref(char *, var, varname(glom(assign->u[0].p, outer, FALSE)));
			Ref(List *, lp, glom(assign->u[1].p, outer, TRUE));
			looping = mkbinding(var, lp, looping);
			RefEnd3(lp, var, assign);
			SIGCHK();
		}
		RefEnd(defn);

		if (looping != NULL) {
			Binding *prev, *next;
			prev = NULL;
			do {
				next = looping->next;
				looping->next = prev;
				prev = looping;
			} while ((looping = next) != NULL);
			looping = prev;
		}

		for (;;) {
			Boolean allnull = TRUE;
			Ref(Binding *, bp, outer);
			Ref(Binding *, lp, looping);
			for (; lp != NULL; lp = lp->next) {
				Ref(List *, defn, NULL);
				if (lp->defn != NULL) {
					defn = mklist(lp->defn->term, NULL);
					lp->defn = lp->defn->next;
					allnull = FALSE;
				}
				bp = mkbinding(lp->name, defn, bp);
				RefEnd(defn);
			}
			RefEnd(lp);
			if (allnull) {
				RefPop(bp);
				break;
			}
			result = walk(body, bp, flags & eval_exitonfalse);
			RefEnd(bp);
			SIGCHK();
		}

		e = (result == true) ? listcopy(true) : result;
		RefEnd4(looping, body, outer, result);
		pophandler(&h);
		return e;
	}
	
	default:
		panic("walk: bad node kind %d", tree->kind);

	}
	NOTREACHED;
}
