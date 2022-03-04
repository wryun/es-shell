/* dump.c -- dump es's internal state as a c program ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "var.h"
#include "term.h"

#define	MAXVARNAME 20

/*
 * the $&dumpstate prints the appropriate C data structures for
 * representing the parts of es's memory that can be stored in
 * the text (read-only) segment of the program.  (some liberties
 * are taken with regard to what the initial.es routines can do
 * regarding changing lexically bound values in order that more
 * things can be here.)
 *
 * since these things are read-only they cannot point to structures
 * that need to be garbage collected.  (think of this like a very
 * old generation in a generational collector.)
 *
 * to simplify matters, all values are stored in C variables with
 * idiosyncratic names:
 *	S_string	"string"
 *	X_address	string at address, when name wouldn't fit
 *	L_address	List at address
 *	E_address	Term at address
 *	T_address	Tree at address
 *	B_address	Binding at address
 *	C_address	Closure at address
 *
 * in order that addresses are internally consistent, garbage collection
 * is disabled during the dumping process.
 */

static Dict *cvars, *strings;

static Boolean allprintable(const char *s) {
	int c;
	for (; (c = *(unsigned char *) s) != '\0'; s++)
		if (!isprint(c) || c == '"' || c == '\\')
			return FALSE;
	return TRUE;
}

static char *dumpstring(char *string) {
	char *name;
	if (string == NULL)
		return "NULL";
	name = dictget(strings, string);
	if (name == NULL) {
		name = str("S_%F", string);
		if (strlen(name) > MAXVARNAME)
			name = str("X_%ulx", string);
		print("static const char %s[] = ", name);
		if (allprintable(string))
			print("\"%s\";\n", string);
		else {
			int c;
			char *s;
			print("{ ");
			for (s = string; (c = *(unsigned char *) s) != '\0'; s++) {
				switch (c) {
				case '\a':	print("'\\a'");		break;
				case '\b':	print("'\\b'");		break;
				case '\f':	print("'\\f'");		break;
				case '\n':	print("'\\n'");		break;
				case '\r':	print("'\\r'");		break;
				case '\t':	print("'\\t'");		break;
				case '\'':	print("'\\''");		break;
				case '\\':	print("'\\\\'");	break;
				default:	print(isprint(c) ? "'%c'" :"%d", c); break;
				}
				print(", ");
			}
			print("'\\0', };\n");
		}
		strings = dictput(strings, string, name);
	}
	return name;
}

static char *dumplist(List *list);

static const char *nodename(NodeKind k) {
	switch(k) {
	default:	panic("nodename: bad node kind %d", k);
	case nAssign:	return "Assign";
	case nCall:	return "Call";
	case nClosure:	return "Closure";
	case nConcat:	return "Concat";
	case nFor:	return "For";
	case nLambda:	return "Lambda";
	case nLet:	return "Let";
	case nList:	return "List";
	case nLocal:	return "Local";
	case nMatch:	return "Match";
	case nExtract:	return "Extract";
	case nPrim:	return "Prim";
	case nQword:	return "Qword";
	case nThunk:	return "Thunk";
	case nVar:	return "Var";
	case nVarsub:	return "Varsub";
	case nWord:	return "Word";
	}
}

static char *dumptree(Tree *tree) {
	char *name;
	if (tree == NULL)
		return "NULL";
	name = str("&T_%ulx", tree);
	if (dictget(cvars, name) == NULL) {
		switch (tree->kind) {
		    default:
			panic("dumptree: bad node kind %d", tree->kind);
		    case nWord: case nQword: case nPrim:
			print("static const Tree_s %s = { n%s, { { (char *) %s } } };\n",
			      name + 1, nodename(tree->kind), dumpstring(tree->u[0].s));
			break;
		    case nCall: case nThunk: case nVar:
			print("static const Tree_p %s = { n%s, { { (Tree *) %s } } };\n",
			      name + 1, nodename(tree->kind), dumptree(tree->u[0].p));
			break;
		    case nAssign:  case nConcat: case nClosure: case nFor:
		    case nLambda: case nLet: case nList:  case nLocal:
		    case nVarsub: case nMatch: case nExtract:
			print("static const Tree_pp %s = { n%s, { { (Tree *) %s }, { (Tree *) %s } } };\n",
			      name + 1, nodename(tree->kind), dumptree(tree->u[0].p), dumptree(tree->u[1].p));
		}
		cvars = dictput(cvars, name, tree);
	}
	return name;
}

static char *dumpbinding(Binding *binding) {
	char *name;
	if (binding == NULL)
		return "NULL";
	name = str("&B_%ulx", binding);
	if (dictget(cvars, name) == NULL) {
		print(
			"static const Binding %s = { (char *) %s, (List *) %s, (Binding *) %s };\n",
			name + 1,
			dumpstring(binding->name),
			dumplist(binding->defn),
			dumpbinding(binding->next)
		);
		cvars = dictput(cvars, name, binding);
	}
	return name;
}

static char *dumpclosure(Closure *closure) {
	char *name;
	if (closure == NULL)
		return "NULL";
	name = str("&C_%ulx", closure);
	if (dictget(cvars, name) == NULL) {
		print(
			"static const Closure %s = { (Binding *) %s, (Tree *) %s };\n",
			name + 1,
			dumpbinding(closure->binding),
			dumptree(closure->tree)
		);
		cvars = dictput(cvars, name, closure);
	}
	return name;
}

static char *dumpterm(Term *term) {
	char *name;
	if (term == NULL)
		return "NULL";
	name = str("&E_%ulx", term);
	if (dictget(cvars, name) == NULL) {
		print(
			"static const Term %s = { (char *) %s, (Closure *) %s };\n",
			name + 1,
			dumpstring(term->str),
			dumpclosure(term->closure)
		);
		cvars = dictput(cvars, name, term);
	}
	return name;
}

static char *dumplist(List *list) {
	char *name;
	if (list == NULL)
		return "NULL";
	name = str("&L_%ulx", list);
	if (dictget(cvars, name) == NULL) {
		print(
			"static const List %s = { (Term *) %s, (List *) %s };\n",
			name + 1,
			dumpterm(list->term),
			dumplist(list->next)
		);
		cvars = dictput(cvars, name, list);
	}
	return name;
}

static void dumpvar(void *ignore, char *key, void *value) {
	Var *var = value;
	dumpstring(key);
	dumplist(var->defn);
}

static void dumpdef(char *name, Var *var) {
	print("\t{ %s, (const List *) %s },\n", dumpstring(name), dumplist(var->defn));
}

static void dumpfunctions(void *ignore, char *key, void *value) {
	if (hasprefix(key, "fn-"))
		dumpdef(key, value);
}

static void dumpsettors(void *ignore, char *key, void *value) {
	if (hasprefix(key, "set-"))
		dumpdef(key, value);
}

static void dumpvariables(void *ignore, char *key, void *value) {
	if (!hasprefix(key, "fn-") && !hasprefix(key, "set-"))
		dumpdef(key, value);
}

#define TreeTypes \
	typedef struct { NodeKind k; struct { char *s; } u[1]; } Tree_s; \
	typedef struct { NodeKind k; struct { Tree *p; } u[1]; } Tree_p; \
	typedef struct { NodeKind k; struct { Tree *p; } u[2]; } Tree_pp;
TreeTypes
#define	PPSTRING(s)	STRING(s)

static void printheader(List *title) {
	if (
		   offsetof(Tree, u[0].s) != offsetof(Tree_s,  u[0].s)
		|| offsetof(Tree, u[0].p) != offsetof(Tree_p,  u[0].p)
		|| offsetof(Tree, u[0].p) != offsetof(Tree_pp, u[0].p)
		|| offsetof(Tree, u[1].p) != offsetof(Tree_pp, u[1].p)
	)
		panic("dumpstate: Tree union sizes do not match struct sizes");

	print("/* %L */\n\n#include \"es.h\"\n#include \"term.h\"\n\n", title, " ");
	print("%s\n\n", PPSTRING(TreeTypes));
}

extern void runinitial(void) {
	List *title = runfd(0, "initial.es", 0);
	
	gcdisable();

	cvars = mkdict();
	strings = mkdict();

	printheader(title);
	dictforall(vars, dumpvar, NULL);

	/* these must be assigned in this order, or things just won't work */
	print("\nstatic const struct { const char *name; const List *value; } defs[] = {\n");
	dictforall(vars, dumpfunctions, NULL);
	dictforall(vars, dumpsettors, NULL);
	dictforall(vars, dumpvariables, NULL);
	print("\t{ NULL, NULL }\n");
	print("};\n\n");

	print("\nextern void runinitial(void) {\n");
	print("\tint i;\n");
	print("\tfor (i = 0; defs[i].name != NULL; i++)\n");
	print("\t\tvardef((char *) defs[i].name, NULL, (List *) defs[i].value);\n");
	print("}\n");

	exit(0);
}
