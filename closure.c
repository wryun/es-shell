/* closure.c -- operations on bindings, closures, lambdas, and thunks ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

/*
 * Closure garbage collection support
 */

DefineTag(Closure, static);

extern Closure *mkclosure(Tree *tree, Binding *binding) {
	gcdisable();
	Ref(Closure *, closure, gcnew(Closure));
	closure->tree = tree;
	closure->binding = binding;
	gcenable();
	RefReturn(closure);
}

static void *ClosureCopy(void *op) {
	void *np = gcnew(Closure);
	memcpy(np, op, sizeof (Closure));
	return np;
}

static size_t ClosureScan(void *p) {
	Closure *closure = p;
	closure->tree = forward(closure->tree);
	closure->binding = forward(closure->binding);
	return sizeof (Closure);
}

/* revtree -- destructively reverse a list stored in a tree */
static Tree *revtree(Tree *tree) {
	Tree *prev, *next;
	if (tree == NULL)
		return NULL;
	prev = NULL;
	do {
		assert(tree->kind == nList);
		next = tree->u[1].p;
		tree->u[1].p = prev;
		prev = tree;
	} while ((tree = next) != NULL);
	return prev;
}

typedef struct Chain Chain;
struct Chain {
	Closure *closure;
	Chain *next;
};
static Chain *chain = NULL;

static Binding *extract(Tree *tree, Binding *bindings) {
	assert(gcisblocked());

	for (; tree != NULL; tree = tree->u[1].p) {
		Tree *defn = tree->u[0].p;
		assert(tree->kind == nList);
		if (defn != NULL) {
			List *list = NULL;
			Tree *name = defn->u[0].p;
			assert(name->kind == nWord || name->kind == nQword);
			defn = revtree(defn->u[1].p);
			for (; defn != NULL; defn = defn->u[1].p) {
				Term *term;
				Tree *word = defn->u[0].p;
				NodeKind k = word->kind;
				assert(defn->kind == nList);
				assert(k == nWord || k == nQword || k == nPrim);
				if (k == nPrim) {
					char *prim = word->u[0].s;
					if (streq(prim, "nestedbinding")) {
						int i, count;
						Chain *cp;
						if (
							(defn = defn->u[1].p) == NULL
						     || defn->u[0].p->kind != nWord
						     || (count = (atoi(defn->u[0].p->u[0].s))) < 0
						) {
							fail("$&parse", "improper use of $&nestedbinding");
							NOTREACHED;
						}
						for (cp = chain, i = 0;; cp = cp->next, i++) {
							if (cp == NULL) {
								fail("$&parse", "bad count in $&nestedbinding: %d", count);
								NOTREACHED;
							}
							if (i == count)
								break;
						}
						term = mkterm(NULL, cp->closure);
					} else {
						fail("$&parse", "bad unquoted primitive in %%closure: $&%s", prim);
						NOTREACHED;
					}
				} else
					term = mkstr(word->u[0].s);
				list = mklist(term, list);
			}
			bindings = mkbinding(name->u[0].s, list, bindings);
		}
	}

	return bindings;
}

extern Closure *extractbindings(Tree *tree0) {
	Chain me;
	Tree *volatile tree = tree0;
	Binding *volatile bindings = NULL;

	gcdisable();

	if (tree->kind == nList && tree->u[1].p == NULL)
		tree = tree->u[0].p; 

	me.closure = mkclosure(NULL, NULL);
	me.next = chain;
	chain = &me;

	ExceptionHandler

		while (tree->kind == nClosure) {
			bindings = extract(tree->u[0].p, bindings);
			tree = tree->u[1].p;
			if (tree->kind == nList && tree->u[1].p == NULL)
				tree = tree->u[0].p; 
		}

	CatchException (e)
	
		chain = chain->next;
		throw(e);

	EndExceptionHandler

	chain = chain->next;

	Ref(Closure *, result, me.closure);
	result->tree = tree;
	result->binding = bindings;
	gcenable();
	RefReturn(result);
}


/*
 * Binding garbage collection support
 */

DefineTag(Binding, static);

extern Binding *mkbinding(char *name, List *defn, Binding *next) {
	assert(next == NULL || next->name != NULL);
	validatevar(name);
	gcdisable();
	Ref(Binding *, binding, gcnew(Binding));
	binding->name = name;
	binding->defn = defn;
	binding->next = next;
	gcenable();
	RefReturn(binding);
}

extern Binding *reversebindings(Binding *binding) {
	if (binding == NULL)
		return NULL;
	else {
		Binding *prev, *next;
		prev = NULL;
		do {
			next = binding->next;
			binding->next = prev;
			prev = binding;
		} while ((binding = next) != NULL);
		return prev;
	}
}

static void *BindingCopy(void *op) {
	void *np = gcnew(Binding);
	memcpy(np, op, sizeof (Binding));
	return np;
}

static size_t BindingScan(void *p) {
	Binding *binding = p;
	binding->name = forward(binding->name);
	binding->defn = forward(binding->defn);
	binding->next = forward(binding->next);
	return sizeof (Binding);
}
