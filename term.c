/* term.c -- operations on terms ($Revision: 1.2 $) */

#include "es.h"
#include "gc.h"

DefineTag(Term, static);

extern Term *mkterm(char *str, Closure *closure) {
	gcdisable(0);
	Ref(Term *, term, gcnew(Term));
	term->str = str;
	term->closure = closure;
	gcenable();
	RefReturn(term);
}

extern Closure *getclosure(Term *term) {
	if (term->closure == NULL) {
		char *s = term->str;
		assert(s != NULL);
		if (
			((*s == '{' || *s == '@') && s[strlen(s) - 1] == '}')
			|| (*s == '$' && s[1] == '&')
			|| hasprefix(s, "%closure")
		) {
			Ref(Term *, tp, term);
			Ref(Tree *, np, parsestring(s));
			if (np == NULL) {
				RefPop2(np, tp);
				return NULL;
			}
			tp->closure = extractbindings(np);
			tp->str = NULL;
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
	term->closure = forward(term->closure);
	term->str = forward(term->str);
	return sizeof (Term);
}

