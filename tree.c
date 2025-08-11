/* tree.c -- functions for manipulating parse-trees. (create, copy, scan) ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

DefineTag(Tree1, static);
DefineTag(Tree2, static);

/* gmk -- make a new node; used to generate the parse tree */
static Tree *gmk(void *(*alloc)(size_t, Tag *), NodeKind t, va_list ap) {
	Tree *n;

	gcdisable();
	switch (t) {
	    default:
		panic("mk: bad node kind %d", t);
	    case nWord: case nQword: case nPrim:
		n = alloc(offsetof(Tree, u[1]), &Tree1Tag);
		n->u[0].s = va_arg(ap, char *);
		break;
	    case nCall: case nThunk: case nVar:
		n = alloc(offsetof(Tree, u[1]), &Tree1Tag);
		n->u[0].p = va_arg(ap, Tree *);
		break;
	    case nAssign:  case nConcat: case nClosure: case nFor:
	    case nLambda: case nLet: case nList:  case nLocal:
	    case nVarsub: case nMatch: case nExtract:
		n = alloc(offsetof(Tree, u[2]), &Tree2Tag);
		n->u[0].p = va_arg(ap, Tree *);
		n->u[1].p = va_arg(ap, Tree *);
		break;
	    case nRedir:
		n = alloc(offsetof(Tree, u[2]), NULL);
		n->u[0].p = va_arg(ap, Tree *);
		n->u[1].p = va_arg(ap, Tree *);
		break;
	    case nPipe:
		n = alloc(offsetof(Tree, u[2]), NULL);
		n->u[0].i = va_arg(ap, int);
		n->u[1].i = va_arg(ap, int);
		break;
 	}
	n->kind = t;

	Ref(Tree *, tree, n);
	gcenable();
	RefReturn(tree);
}

extern Tree *mk VARARGS1(NodeKind, t) {
	va_list ap;
	Tree *tree = NULL;
	VA_START(ap, t);
	tree = gmk(palloc, t, ap);
	va_end(ap);
	return tree;
}

extern Tree *gcmk VARARGS1(NodeKind, t) {
	va_list ap;
	Ref(Tree *, tree, NULL);
	VA_START(ap, t);
	tree = gmk(gcalloc, t, ap);
	va_end(ap);
	RefReturn(tree);
}


/*
 * garbage collection functions
 *	these are segregated by size so copy doesn't have to check
 *	the type to figure out size.
 */

static void *Tree1Copy(void *op) {
	void *np = gcalloc(offsetof(Tree, u[1]), &Tree1Tag);
	memcpy(np, op, offsetof(Tree, u[1]));
	return np;
}

static void *Tree2Copy(void *op) {
	void *np = gcalloc(offsetof(Tree, u[2]), &Tree2Tag);
	memcpy(np, op, offsetof(Tree, u[2]));
	return np;
}

static size_t Tree1Scan(void *p) {
	Tree *n = p;
	switch (n->kind) {
	    default:
		panic("Tree1Scan: bad node kind %d", n->kind);
	    case nPrim: case nWord: case nQword:
		n->u[0].s = forward(n->u[0].s);
		break;
	    case nCall: case nThunk: case nVar:
		n->u[0].p = forward(n->u[0].p);
		break;
	}
	return offsetof(Tree, u[1]);
}

static size_t Tree2Scan(void *p) {
	Tree *n = p;
	switch (n->kind) {
	    case nAssign:  case nConcat: case nClosure: case nFor:
	    case nLambda: case nLet: case nList:  case nLocal:
	    case nVarsub: case nMatch: case nExtract: case nRedir:
		n->u[0].p = forward(n->u[0].p);
		n->u[1].p = forward(n->u[1].p);
		break;
	    default:
		panic("Tree2Scan: bad node kind %d", n->kind);
	}
	return offsetof(Tree, u[2]);
}
