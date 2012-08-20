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

extern Closure *extractbindings(Tree *tp) {
	Closure *result;
	Ref(Tree *, tree, (tp->kind == nList && tp->u[1].p == NULL) ? tp->u[0].p : tp);
	Ref(Binding *, binding, NULL);
	if (tree->kind == nLambda) {
		Ref(Tree *, cx, tree->u[0].p);
		for (; cx != NULL; cx = cx->u[1].p) {
			assert(cx->kind == nList);
			if (cx->u[0].p->kind != nAssign)		/* TODO: nRec */
				break;
			Ref(Tree *, defn, cx->u[0].p);
			Ref(Tree *, name, defn->u[0].p);
			Ref(List *, list, NULL);
			defn = revtree(defn->u[1].p);
			for (; defn != NULL; defn = defn->u[1].p) {
				Tree *word = defn->u[0].p;
				assert(defn->kind == nList);
				assert(word->kind == nWord || word->kind == nQword);
				list = mklist(mkterm(word->u[0].s, NULL), list);
			}
			assert(name->kind == nWord || name->kind == nQword);
			binding = bind(name->u[0].s, list, binding);
			RefEnd3(list, name, defn);
		}
		if (cx != NULL)
			tree->u[0].p = cx;
		else
			tree = mk(nThunk, tree->u[1].p);
		RefEnd(cx);
	}
	result = mkclosure(tree, binding);
	RefEnd2(binding, tree);
	return result;
}


/*
 * Binding garbage collection support
 */

static Tag BindingTag;

extern Binding *bind(char *name, List *defn, Binding *next) {
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
