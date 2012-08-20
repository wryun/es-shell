/* term.c -- operations on terms ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"
#include "term.h"

DefineTag(Term, static);

extern Term *mkterm(char *str, Closure *closure) {
	gcdisable();
	Ref(Term *, term, gcnew(Term));
	term->str = str;
	term->closure = closure;
	gcenable();
	RefReturn(term);
}

extern Term *mkstr(char *str) {
	Term *term;
	Ref(char *, string, str);
	term = gcnew(Term);
        term->str = string;
	term->closure = NULL;
        RefEnd(string);
        return term;
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

extern char *getstr(Term *term) {
	char *s = term->str;
	Closure *closure = term->closure;
	assert((s == NULL) != (closure == NULL));
	if (s != NULL)
		return s;

#if 0	/* TODO: decide whether getstr() leaves term in closure or string form */
	Ref(Term *, tp, term);
	s = str("%C", closure);
	tp->str = s;
	tp->closure = NULL;
	RefEnd(tp);
	return s;
#else
	return str("%C", closure);
#endif
}

extern Term *termcat(Term *t1, Term *t2) {
	if (t1 == NULL)
		return t2;
	if (t2 == NULL)
		return t1;

	Ref(Term *, term, mkstr(NULL));
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

extern Boolean termeq(Term *term, const char *s) {
	assert(term != NULL);
	if (term->str == NULL)
		return FALSE;
	return streq(term->str, s);
}

extern Boolean isclosure(Term *term) {
	assert(term != NULL);
	return term->closure != NULL;
}
