/* term.c -- operations on terms */

#include "es.h"
#include "gc.h"

static Tag TermTag;

extern Term *mkterm(char *str, Closure *closure) {
	gcdisable(0);
	Ref(Term *, term, gcnew(Term));
	term->str = str;
	term->closure = closure;
	gcenable();
	RefReturn(term);
}

extern char *getstr(Term *term) {
	assert(term != NULL);
	Ref(Term *, tp, term);
	if (tp->str == NULL) {
		assert(tp->closure != NULL);
		tp->str = str("%C", tp->closure);
	}
	term = tp;
	RefEnd(tp);
	return term->str;
}

extern Closure *getclosure(Term *term) {
	if (term->closure == NULL) {
		char *s = term->str;
		assert(s != NULL);
		if (
			((*s == '{' || *s == '@') && s[strlen(s) - 1] == '}')
			|| (*s == '$' && s[1] == '&')
		) {
			Ref(Term *, tp, term);
			Ref(Tree *, np, parsestring(s));
			if (np == NULL) {
				RefPop2(np, tp);
				return NULL;
			}
			tp->closure = extractbindings(np);
			term = tp;
			RefEnd2(np, tp);
		}
	}
	return term->closure;
}

extern Term *termcat(Term *t1, Term *t2) {
	if (t1 == NULL)
		return t2;
	if (t2 == NULL)
		return t1;

	Ref(Term *, term, mkterm(NULL, NULL));
	Ref(char *, str1, getstr(t1));
	Ref(char *, str2, getstr(t2));
	term->str = str("%s%s", str1, str2);
	RefEnd2(str2, str1);
	RefReturn(term);
}


static void *TermCopy(void *op) {
	void *np = gcnew(Term);
	memcpy(np, op, sizeof (Term));
	return np;
}

static size_t TermScan(void *p) {
	Term *term = p;
	term->str = forward(term->str);
	term->closure = forward(term->closure);
	return sizeof (Term);
}

static DefineTag(Term);
