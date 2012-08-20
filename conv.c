/*
 * conv.c -- a collection of functions to convert internal representations of
 * variables and functions to external representations, and vice versa
 */

#include <ctype.h>
#include "es.h"
#include "print.h"


/* %L -- print a list */
static Boolean Lconv(Format *f) {
	List *lp, *next;
	char *sep;
	const char *fmt = (f->flags & FMT_altform) ? "%S%s" : "%s%s";
	
	lp = va_arg(f->args, List *);
	sep = va_arg(f->args, char *);
	for (; lp != NULL; lp = next) {
		next = lp->next;
		fmtprint(f, fmt, getstr(lp->term), next == NULL ? "" : sep);
	}
	return FALSE;
}

/* %D -- convert a redirection to a printable form */
static Boolean Dconv(Format *f) {
	char *name = "?";
	int n = va_arg(f->args, int);
	switch (n) {
	case rOpen:		name = "<";	break;
	case rCreate:		name = ">";	break;
	case rAppend:		name = ">>";	break;
	case rHeredoc:		name = "<<";	break;
	case rHerestring:	name = "<<<";	break;
	default:		panic("Dconv: bad redirection type %d\n", n);
	}
	fmtcat(f, name);
	return FALSE;
}

/* defaultfd -- return the default fd for a given redirection operation */
extern int defaultfd(RedirKind op) {
	return (op == rCreate || op == rAppend) ? 1 : 0;
}

/* treecount -- count the number of nodes in a flattened tree list */
static int treecount(Tree *tree) {
	return tree == NULL
		 ? 0
		 : tree->kind == nList
		    ? treecount(tree->u[0].p) + treecount(tree->u[1].p)
		    : 1;
}

/* binding -- print a binding statement */
static void binding(Format *f, char *keyword, Tree *tree) {
	Tree *np;
	char *sep = "";
	fmtprint(f, "%s(", keyword);
	for (np = tree->u[0].p; np != NULL; np = np->u[1].p) {
		Tree *binding;
		assert(np->kind == nList);
		binding = np->u[0].p;
		assert(binding != NULL);
		assert(binding->kind == nAssign);
		fmtprint(f, "%s%#T=%T", sep, binding->u[0].p, binding->u[1].p);
		sep = ";";
	}
	fmtprint(f, ")%T", tree->u[1].p);
}

static Boolean fmttree(Format *f, Tree *n) {
	Boolean group = (f->flags & FMT_altform) != 0;
	Boolean suppressat = (f->flags & FMT_leftside) != 0;

top:
	if (n == NULL) {
		if (group)
			fmtcat(f, "()");
		return FALSE;
	}

	switch (n->kind) {

	case nAssign:	fmtprint(f, "%#T=%T", n->u[0].p, n->u[1].p);		break;
	case nCall:	fmtprint(f, "<>{%T}", n->u[0].p);			break;
	case nConcat:	fmtprint(f, "%#T^%#T", n->u[0].p, n->u[1].p);		break;
	case nMatch:	fmtprint(f, "~ %#T %T", n->u[0].p, n->u[1].p);		break;
	case nPrim:	fmtprint(f, "$&%s", n->u[0].s);				break;
	case nThunk:	fmtprint(f, "{%T}", n->u[0].p);				break;
	case nVar:	fmtprint(f, "$%#T", n->u[0].p);				break;
	case nVarsub:	fmtprint(f, "$%#T(%T)", n->u[0].p, n->u[1].p);		break;
	case nWord:	fmtprint(f, "%s", n->u[0].s);				break;

	case nLocal:	binding(f, "local", n);	break;
	case nLet:	binding(f, "let", n);	break;
	case nFor:	binding(f, "for", n);	break;

	case nQword: {
		char *s = n->u[0].s;
		int c = *s;
		if (!isprint(c) && s[1] == '\0')
			switch (c) {
			case '\033':	fmtprint(f, "\\e"); break;
			case '\a':	fmtprint(f, "\\a"); break;
			case '\b':	fmtprint(f, "\\b"); break;
			case '\f':	fmtprint(f, "\\f"); break;
			case '\n':	fmtprint(f, "\\n"); break;
			case '\r':	fmtprint(f, "\\r"); break;
			case '\t':	fmtprint(f, "\\t"); break;
			default:	fmtprint(f, "\\%o", c); break;
			}
		else
			fmtprint(f, "'%s'", s);
		break;
	}

	case nLambda:
		if (!suppressat)
			fmtprint(f, "@ ");
		if (n->u[0].p == NULL)
			fmtprint(f, "* ");
		else
			fmtprint(f, "%T", n->u[0].p);
		fmtprint(f, "{%T}", n->u[1].p);
		break;

	case nList:
		if (!group) {
			for (; n->u[1].p != NULL; n = n->u[1].p)
				fmtprint(f, "%T ", n->u[0].p);
			n = n->u[0].p;
			goto top;
		}
		switch (treecount(n)) {
		case 0:
			fmtcat(f, "()");
			break;
		case 1:
			fmtprint(f, "%T%T", n->u[0].p, n->u[1].p);
			break;
		default:
			fmtprint(f, "(%T", n->u[0].p);
			while ((n = n->u[1].p) != NULL) {
				assert(n->kind == nList);
				fmtprint(f, " %T", n->u[0].p);
			}
			fmtputc(f, ')');
		}
		break;

	case nRec: {
		int c = n->u[0].i;
		assert(c > 0);
		fmtprint(f, "%#T", n->u[1].p);
		while (--c >= 0)
			fmtputc(f, '&');
		break;
	}
	default:
		panic("Tconv: bad node kind %d", n->kind);
 	}
	return FALSE;
}

/* %T -- print a tree */
static Boolean Tconv(Format *f) {
	return fmttree(f, va_arg(f->args, Tree *));
}

/* %C -- print a closure */
static Boolean Cconv(Format *f) {
	Closure *closure = va_arg(f->args, Closure *);
	Tree *tree = closure->tree;
	Binding *binding = closure->binding;

	assert(tree->kind == nThunk || tree->kind == nLambda || tree->kind == nPrim);

	if (binding == NULL) {
		f->flags = 0;
		return fmttree(f, tree);
	}

	/* TODO: deal with recursion */
	fmtprint(f, "@ ");
	for (; binding != NULL; binding = binding->next)
		fmtprint(f, "%S=(%#L) ", binding->name, binding->defn, " ");

	fmtprint(f, "%-T", tree);

	return FALSE;
}

/* %S -- print a string with conservative quoting rules */
static Boolean Sconv(Format *f) {
	int c;
	const char *s, *t;
	extern const char nw[];
	
	s = va_arg(f->args, const char *);
	for (t = s; (c = *t) != '\0'; t++)
		if (nw[c])
			goto quoteit;
	fmtprint(f, "%s", s);
	return FALSE;

quoteit:
	fmtputc(f, '\'');
	for (t = s; (c = *t) != '\0'; t++) {
		if (c == '\'')
			fmtputc(f, '\'');
		fmtputc(f, c);
	}
	fmtputc(f, '\'');

	return FALSE;
}

/* %Z -- print a StrList */
static Boolean Zconv(Format *f) {
	StrList *lp, *next;
	char *sep;
	
	lp = va_arg(f->args, StrList *);
	sep = va_arg(f->args, char *);
	for (; lp != NULL; lp = next) {
		next = lp->next;
		fmtprint(f, "%s%s", lp->str, next == NULL ? "" : sep);
	}
	return FALSE;
}

/* %F -- protect an exported name from brain-dead shells */
static Boolean Fconv(Format *f) {
#if PROTECT_ENV
	int c;
	unsigned char *name, *s;
	
	name = va_arg(f->args, unsigned char *);

	for (s = name; (c = *s) != '\0'; s++)
		if ((s == name ? isalpha(c) : isalnum(c)) || (c == '_' && s[1] != '_'))
			fmtputc(f, c);
		else
			fmtprint(f, "__%02x", c);
	return FALSE;
#else
	static Conv sconv = NULL;
	if (sconv == NULL)
		sconv = fmtinstall('s', NULL);
	return (*sconv)(f);
#endif
}

/* %N -- undo %F */
static Boolean Nconv(Format *f) {
#if PROTECT_ENV
	int c;
	unsigned char *s = va_arg(f->args, unsigned char *);

	while ((c = *s++) != '\0') {
		if (c == '_' && *s == '_') {
			static const char hexchar[] = "0123456789abcdef";
			const char *h1 = strchr(hexchar, s[1]);
			const char *h2 = strchr(hexchar, s[2]);
			if (h1 != NULL && h2 != NULL) {
				c = ((h1 - hexchar) << 4) | (h2 - hexchar);
				s += 3;
			}
		}
		fmtputc(f, c);
	}
	return FALSE;
#else
	static Conv sconv = NULL;
	if (sconv == NULL)
		sconv = fmtinstall('s', NULL);
	return (*sconv)(f);
#endif
}

/* install the conversion routines */
void initconv(void) {
	fmtinstall('C', Cconv);
	fmtinstall('D', Dconv);
	fmtinstall('F', Fconv);
	fmtinstall('L', Lconv);
	fmtinstall('N', Nconv);
	fmtinstall('S', Sconv);
	fmtinstall('T', Tconv);
	fmtinstall('Z', Zconv);
}
