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

static char *doconcat(Tree *word) {
	NodeKind k = word->kind;
	assert(gcisblocked());
	assert(k == nWord || k == nQword || k == nConcat);
	if (k != nConcat)
		return word->u[0].s;
	return str("%s%s", doconcat(word->u[0].p), doconcat(word->u[1].p));
}

static char *getrefid(Tree *defn) {
	assert(defn == NULL || defn->kind == nList);
	if (defn == NULL)
		return NULL;
	if (defn->u[0].p->kind != nPrim)
		return NULL;
	if (!streq(defn->u[0].p->u[0].s, "ref"))
		return NULL;
	defn = defn->u[1].p->u[0].p;
	if (defn->kind != nWord && defn->kind != nQword)
		return NULL;
	return defn->u[0].s;
}

static Closure *extractbindingsinrefscope(Tree *tree, Dict **refdictp);

static Binding *extract(Tree *tree, Binding *bindings, Dict **refdictp) {
	assert(gcisblocked());

	for (; tree != NULL; tree = tree->u[1].p) {
		Tree *defn = tree->u[0].p;
		assert(tree->kind == nList);
		if (defn != NULL) {
			List *list = NULL;
			Tree *name = defn->u[0].p;
			char *refid;

			assert(name->kind == nWord || name->kind == nQword);
			bindings = mkbinding(name->u[0].s, NULL, bindings);

			if ((refid = getrefid(defn->u[1].p)) != NULL) {
				Binding *ref;
				if ((ref = dictget(*refdictp, refid)) != NULL) {
					bindings = ref;
					continue;
				} else
					*refdictp = dictput(*refdictp, refid, bindings);
				defn = defn->u[1].p->u[1].p;
			}

			defn = revtree(defn->u[1].p);
			for (; defn != NULL; defn = defn->u[1].p) {
				Term *term;
				Tree *word = defn->u[0].p;
				assert(defn->kind == nList);
				switch (word->kind) {
				case nConcat:
					term = mkstr(doconcat(word));
					break;
				case nQword:
					/* eagerly parse sub-closures so they
					 * have access to the enclosing $&ref scope.
					 * boy, do I dislike %closure syntax! */
					if (hasprefix(word->u[0].s, "%closure")) {
						Tree *np = parsestring(word->u[0].s);
						Closure *c = extractbindingsinrefscope(np, refdictp);
						term = mkterm(NULL, c);
						break;
					}
					FALLTHROUGH;
				case nWord:
					term = mkstr(word->u[0].s);
					break;
				case nPrim: case nLambda: case nThunk:
					term = mkterm(NULL, mkclosure(word, NULL));
					break;
				case nCall: case nVar: case nVarsub:
					fail("$&parse", "bad definition in %%closure: %T\n", defn);
				default:
					NOTREACHED;
				}
				list = mklist(term, list);
			}
			bindings->defn = list;
		}
	}

	return bindings;
}

static Closure *extractbindingsinrefscope(Tree *tree, Dict **refdictp) {
	Binding *bindings = NULL;
	if (tree->kind == nList && tree->u[1].p == NULL)
		tree = tree->u[0].p;

	while (tree->kind == nClosure) {
		bindings = extract(tree->u[0].p, bindings, refdictp);
		tree = tree->u[1].p;
		if (tree == NULL)
			fail("$&parse", "null body in %%closure");
		if (tree->kind == nList && tree->u[1].p == NULL)
			tree = tree->u[0].p;
	}
	return mkclosure(tree, bindings);
}

extern Closure *extractbindings(Tree *tree) {
	Dict *refdict;
	gcdisable();
	refdict = mkdict();
	Ref(Closure *, result, extractbindingsinrefscope(tree, &refdict));
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
