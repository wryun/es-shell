/* list.c -- operations on lists */

#include "es.h"
#include "gc.h"

/*
 * allocation and garbage collector support
 */

static Tag ListTag;

extern List *mklist(Term *term, List *next) {
	gcdisable(0);
	assert(term != NULL);
	Ref(List *, list, gcnew(List));
	list->term = term;
	list->next = next;
	gcenable();
	RefReturn(list);
}

static void *ListCopy(void *op) {
	void *np = gcnew(List);
	memcpy(np, op, sizeof (List));
	return np;
}

static size_t ListScan(void *p) {
	List *list = p;
	list->term = forward(list->term);
	list->next = forward(list->next);
	return sizeof (List);
}

static DefineTag(List);


/*
 * basic list manipulations
 */

#if 0
/* safereverse -- reverse a list, non-destructively */
extern List *safereverse(List *list) {
	List *lp;

	if (list == NULL)
		return NULL;
	if (list->next == NULL)
		return list;

	gcdisable(0);

	for (lp = NULL; list != NULL; list = list->next)
		lp = mklist(list->term, lp);

	Ref(List*, result, lp);
	gcenable();
	RefReturn(result);
}
#endif

/* reverse -- destructively reverse a list */
extern List *reverse(List *list) {
	List *prev, *next;
	if (list == NULL)
		return NULL;
	prev = NULL;
	do {
		next = list->next;
		list->next = prev;
		prev = list;
	} while ((list = next) != NULL);
	return prev;
}

/* append -- merge two lists, non-destructively */
extern List *append(List *head0, List *tail0) {
	List **prevp;
#if 0	/* if you want this optimization, rewrite listcopy */
	if (tail0 == NULL)
		return head0;
#endif
	Ref(List *, lp, NULL);
	Ref(List *, head, head0);
	Ref(List *, tail, tail0);
	for (prevp = &lp; head != NULL; head = head->next) {
		List *np = mklist(head->term, NULL);
		*prevp = np;
		prevp = &np->next;
	}
	*prevp = tail;
	RefEnd2(tail, head);
	RefReturn(lp);
}

/* listcopy */
extern List *listcopy(List *list) {
	return append(list, NULL);
}

extern int length(List *list) {
	int len = 0;
	for (; list != NULL; list = list->next)
		++len;
	return len;
}

/* listify -- turn an argc/argv vector into a list */
extern List *listify(int argc, char **argv) {
	Ref(List *, list, NULL);
	while (argc > 0) {
		Term *term = mkterm(argv[--argc], NULL);
		list = mklist(term, list);
	}
	RefReturn(list);
}

/* nth -- return nth element of a list, indexed from 1 */
extern Term *nth(List *list, int n) {
	assert(n > 0);
	for (; list != NULL; list = list->next) {
		assert(list->term != NULL);
		if (--n == 0)
			return list->term;
	}
	return NULL;
}
