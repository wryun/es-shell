/* eval.c -- evaluation of lists and trees */

#include "es.h"

extern int execve(char *name, char **argv, char **envp);

/* forkexec -- fork (if necessary) and exec */
extern List *forkexec(char *file, List *list, Boolean parent) {
	int pid, status;
	Vector *env;
	gcdisable(0);
	env = mkenv();
	pid = efork(parent, FALSE, FALSE);
	if (pid == 0) {
		execve(file, vectorize(list)->vector, env->vector);
		uerror(file);
		exit(1);
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
static Binding *bindargs(Tree *params, List *args, Binding *binding) {
	if (params == NULL)
		return bind("*", args, binding);

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
		binding = bind(param->u[0].s, value, binding);
	}

	Ref(Binding *, result, binding);
	gcenable();
	RefReturn(result);
}

/* eval -- evaluate a list, producing a list */
extern List *eval(List *list, Binding *binding0, Boolean parent) {
	Closure *cp;
	List *fn;
	char *file;

	Ref(char *, funcname, NULL);

restart:
	debug("<< eval : %L >>\n", list, " ");
	if (list == NULL) {
		RefPop(funcname);
		return true;
	}

	Ref(List *, lp, list);
	Ref(Binding *, binding, binding0);
	assert(lp->term != NULL);

	if ((cp = getclosure(lp->term)) != NULL) {
		switch (cp->tree->kind) {
		case nPrim:
			assert(cp->binding == NULL);
			lp = prim(cp->tree->u[0].s, lp->next, parent);
			break;
		case nThunk:
			lp = walk(cp->tree->u[0].p, cp->binding, parent);
			break;
		case nLambda: {
			Handler h;
			List *e;
			if ((e = pushhandler(&h)) != NULL) {
				if (funcname != NULL)
					varpop("0");
				if (e->term->str != NULL && streq(e->term->str, "return")) {
					lp = e->next;
					goto done;
				}
				throw(e);
			}
			Ref(Tree *, tree, cp->tree);
			Ref(Binding *, context, bindargs(tree->u[0].p, lp->next, cp->binding));
			if (funcname != NULL)
				varpush("0", mklist(mkterm(funcname, NULL), NULL));
			lp = walk(tree->u[1].p, context, parent);
			RefEnd2(context, tree);
			pophandler(&h);
			break;
		}
		case nList: {
			list = append(glom(cp->tree, cp->binding, TRUE), lp->next);
			RefPop2(binding, lp)
			goto restart;
		}
		default:
			panic("eval: bad closure node kind %d", cp->tree->kind);
		}
		goto done;
	}

	fn = varlookup2("fn-", lp->term->str);
	if (fn != NULL) {
		funcname = lp->term->str;
		list = append(fn, lp->next);
		RefPop2(binding, lp);
		goto restart;
	}

	file = which(lp->term->str, TRUE);
	lp = (file == NULL) ? false : forkexec(file, lp, parent);

done:
	list = lp;
	RefEnd3(binding, lp, funcname);
	return list;
}

/* eval1 -- evaluate a term, producing a list */
extern List *eval1(Term *term, Boolean parent) {
	return eval(mklist(term, NULL), NULL, parent);
}

/* walk -- walk through a tree, evaluating nodes */
extern List *walk(Tree *tree, Binding *binding, Boolean parent) {

	SIGCHK();

	debug("<< walk : %T >>\n", tree);

top:
	if (tree == NULL) {
		if (!parent)
			exit(0);
		return true;
	}

	switch (tree->kind) {

	case nConcat: case nList: case nQword: case nVar: case nVarsub: case nWord:
	case nThunk: case nLambda: case nCall: case nPrim: case nRec: {
		List *list;
		Ref(Binding *, bp, binding);
		list = glom(tree, binding, TRUE);
		binding = bp;
		RefEnd(bp);
		return eval(list, binding, parent);
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
		return result ? true : false;
	}

	case nAssign: {
		Ref(Binding *, bp, binding);
		Ref(Tree *, tp, tree);
		Ref(char *, var, varname(glom(tp->u[0].p, bp, FALSE)));
		vardef(var, bp, glom(tp->u[1].p, bp, TRUE));
		RefEnd3(var, tp, bp);
		return true;
	}

	case nLocal: {
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
			bp = bind(var, lp, bp);
			RefEnd3(lp, var, assign);
		}
		RefEnd(defn);
		tree = body;
		binding = bp;
		RefEnd2(body, bp);
		goto top;
	}

	case nLet: {
		Handler h;
		List *e;

		Ref(List *, status, NULL);
		Ref(Tree *, tp, tree);
		Ref(Binding *, bp, binding);
		Ref(StrList *, vars, NULL);
		if ((e = pushhandler(&h)) != NULL) {
			for (; vars != NULL; vars = vars->next)
				varpop(vars->str);
			throw(e);
		}
		Ref(Tree *, defn, tp->u[0].p);
		for (; defn != NULL; defn = defn->u[1].p) {
			assert(defn->kind == nList);
			if (defn->u[0].p == NULL)
				continue;
			Ref(Tree *, assign, defn->u[0].p);
			assert(assign->kind == nAssign);
			Ref(char *, var, varname(glom(assign->u[0].p, bp, FALSE)));
			varpush(var, glom(assign->u[1].p, bp, TRUE));
			vars = mkstrlist(var, vars);
			RefEnd2(var, assign);
		}
		RefEnd(defn);
		status = walk(tp->u[1].p, bp, parent);
		for (; vars != NULL; vars = vars->next)
			varpop(vars->str);
		pophandler(&h);
		RefEnd3(vars, bp, tp);
		RefReturn(status);
	}

	case nFor: {
		Handler h;
		List *e;

		if ((e = pushhandler(&h)) != NULL) {
			if (e->term->str != NULL && streq(e->term->str, "return"))
				return e->next;
			throw(e);
		}

		Ref(List *, result, true);
		Ref(Tree *, body, tree->u[1].p);
		Ref(Binding *, context, binding);
		Ref(Binding *, looping, NULL);
		Ref(Tree *, defn, tree->u[0].p);
		Ref(Binding *, oldbinding, binding);
		
		for (; defn != NULL; defn = defn->u[1].p) {
			Term placeholder;
			assert(defn->kind == nList);
			if (defn->u[0].p == NULL)
				continue;
			Ref(Tree *, assign, defn->u[0].p);
			assert(assign->kind == nAssign);
			Ref(char *, var, varname(glom(assign->u[0].p, oldbinding, FALSE)));
			Ref(List *, lp, glom(assign->u[1].p, oldbinding, TRUE));
			looping = bind(var, lp, looping);
			context = bind(var, mklist(&placeholder, NULL), context);
			RefEnd3(lp, var, assign);
		}
		RefEnd2(oldbinding, defn);

		for (;;) {
			Boolean allnull = TRUE;
			Ref(Binding *, lp, looping);
			Ref(Binding *, bp, context);
			for (; lp != NULL; lp = lp->next, bp = bp->next)
				if (lp->defn == NULL)
					bp->defn = NULL;
				else {
					bp->defn->term = lp->defn->term;
					lp->defn = lp->defn->next;
					allnull = FALSE;
				}
			RefEnd2(bp, lp);
			if (allnull)
				break;
			result = walk(body, context, TRUE);
		}

		e = result;
		RefEnd4(looping, context, body, result);
		pophandler(&h);
		return e;
	}

	default:
		panic("walk: bad node kind %d", tree->kind);
	}
}
