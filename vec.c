/* vec.c -- argv[] and envp[] vectors */

#include "es.h"
#include "gc.h"

static Tag VectorTag;

extern Vector *mkvector(int n) {
	int i;
	Vector *v = gcalloc(offsetof(Vector, vector[n + 1]), &VectorTag);
	v->len = n;
	for (i = 0; i <= n; i++)
		v->vector[i] = NULL;
	return v;
}

static void *VectorCopy(void *ov) {
	size_t n = offsetof(Vector, vector[((Vector *) ov)->len + 1]);
	void *nv = gcalloc(n, &VectorTag);
	memcpy(nv, ov, sizeof (Vector));
	return nv;
}

static size_t VectorScan(void *p) {
	Vector *v = p;
	int i, n = v->len;
	for (i = 0; i <= n; i++)
		v->vector[i] = forward(v->vector[i]);
	return offsetof(Vector, vector[n + 1]);
}

static DefineTag(Vector);


extern Vector *vectorize(List *list) {
	int i, n = length(list);

	Ref(Vector *, v, NULL);
	Ref(List *, lp, list);
	v = mkvector(n);

	for (i = 0; lp != NULL; lp = lp->next, i++) {
		char *s = getstr(lp->term); /* must evaluate before v->vector[i] */
		v->vector[i] = s;
	}

	RefEnd(lp);
	RefReturn(v);
}
