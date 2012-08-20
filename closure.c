/* closure.c -- operations on bindings, closures, lambdas, and thunks */

#include "es.h"
#include "gc.h"

/*
 * Closure garbage collection support
 */

static Tag ClosureTag;

extern Closure *mkclosure(Tree *tree, Binding *binding) {
	gcdisable(0);
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

static DefineTag(Closure);

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

static Binding *extract(Tree *tp, Binding *bp) {
	Ref(Binding *, bindings, bp);
	Ref(Tree *, tree, tp);

	for (; tree != NULL; tree = tree->u[1].p) {
		assert(tree->kind == nList);
		Ref(Tree *, defn, tree->u[0].p);
		if (defn != NULL) {
			Ref(Tree *, name, defn->u[0].p);
			assert(name->kind == nWord || name->kind == nQword);
			Ref(List *, list, NULL);
			defn = revtree(defn->u[1].p);
			for (; defn != NULL; defn = defn->u[1].p) {
				Tree *word = defn->u[0].p;
				assert(defn->kind == nList);
				assert(word->kind == nWord || word->kind == nQword);
				list = mklist(mkterm(word->u[0].s, NULL), list);
			}
			bindings = mkbinding(name->u[0].s, list, bindings);
			RefEnd2(list, name);
		}
		RefEnd(defn);
	}

	RefEnd(tree);
	RefReturn(bindings);
}

extern Closure *extractbindings(Tree *tp) {
	Closure *result;
	Ref(Tree *, tree, tp);
	Ref(Binding *, bindings, NULL);

	if (tree->kind == nList && tree->u[1].p == NULL)
		tree = tree->u[0].p; 

	while (tree->kind == nClosure) {
		bindings = extract(tree->u[0].p, bindings);
		tree = tree->u[1].p;
		if (tree->kind == nList && tree->u[1].p == NULL)
			tree = tree->u[0].p; 
	}
	result = mkclosure(tree, bindings);
	RefEnd2(bindings, tree);
	return result;
}


/*
 * Binding garbage collection support
 */

static Tag BindingTag;

extern Binding *mkbinding(char *name, List *defn, Binding *next) {
	assert(next == NULL || next->name != NULL);
	gcdisable(0);
	Ref(Binding *, binding, gcnew(Binding));
	binding->name = name;
	binding->defn = defn;
	binding->next = next;
	gcenable();
	RefReturn(binding);
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

static DefineTag(Binding);
