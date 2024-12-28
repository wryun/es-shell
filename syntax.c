/* syntax.c -- abstract syntax tree re-writing rules ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "input.h"
#include "syntax.h"
#include "token.h"

Tree errornode;
Tree *parsetree;

/* initparse -- called at the dawn of time */
extern void initparse(void) {
	globalroot(&parsetree);
}

/* treecons -- create new tree list cell */
extern Tree *treecons(Tree *car, Tree *cdr) {
	assert(cdr == NULL || cdr->kind == nList);
	return (car == NULL) ? cdr
		: (cdr == NULL && car->kind == nList) ? car
		: mk(nList, car, cdr);
}

/* treeappend -- destructive append for tree lists */
extern Tree *treeappend(Tree *head, Tree *tail) {
	Tree *p, **prevp;
	for (p = head, prevp = &head; p != NULL; p = *(prevp = &p->CDR))
		assert(p->kind == nList || p->kind == nRedir);
	*prevp = tail;
	return head;
}

/* treeconsend -- destructive add node at end for tree lists */
extern Tree *treeconsend(Tree *head, Tree *tail) {
	if (tail == NULL) {
		assert(head == NULL || head->kind == nList || head->kind == nRedir);
		return head;
	}
	return treeappend(head, treecons(tail, NULL));
}

/* thunkify -- wrap a tree in thunk braces if it isn't already a thunk */
extern Tree *thunkify(Tree *tree) {
	Tree *t;
	for (t = tree; t != NULL && t->kind == nList && t->CDR == NULL; t = t->CAR)
		;
	return (t != NULL && t->kind == nThunk) ? tree : mk(nThunk, tree);
}

/* firstis -- check if the first word of a literal command matches a known string */
static Boolean firstis(Tree *t, const char *s) {
	if (t == NULL || t->kind != nList)
		return FALSE;
	t = t->CAR;
	if (t == NULL || t->kind != nWord)
		return FALSE;
	assert(t->u[0].s != NULL);
	return streq(t->u[0].s, s);
}

/* prefix -- prefix a tree with a given word */
extern Tree *prefix(char *s, Tree *t) {
	return treecons(mk(nWord, s), t);
}

/* flatten -- flatten the output of the glommer so we can pass the result as a single element */
extern Tree *flatten(Tree *t, char *sep) {
	return mk(nCall, prefix("%flatten", treecons(mk(nQword, sep), treecons(t, NULL))));
}

/* backquote -- create a backquote command */
extern Tree *backquote(Tree *ifs, Tree *body) {
	return mk(nCall,
		  prefix("%backquote",
		  	 treecons(flatten(ifs, ""),
				  treecons(body, NULL))));
}

/* fnassign -- translate a function definition into an assignment */
extern Tree *fnassign(Tree *name, Tree *defn) {
	return mk(nAssign, mk(nConcat, mk(nWord, "fn-"), name), defn);
}

/* mklambda -- create a lambda */
extern Tree *mklambda(Tree *params, Tree *body) {
	return mk(nLambda, params, body);
}

/* mkseq -- destructively add to a sequence of nList/nThink operations */
extern Tree *mkseq(char *op, Tree *t1, Tree *t2) {
	Tree *tail;
	Boolean sametail;

	if (streq(op, "%seq")) {
		if (t1 == NULL)
			return t2;
		if (t2 == NULL)
			return t1;
	}

	sametail = firstis(t2, op);
	tail = sametail ? t2->CDR : treecons(thunkify(t2), NULL);
	if (firstis(t1, op))
		return treeappend(t1, tail);
	t1 = thunkify(t1);
	if (sametail) {
		  t2->CDR = treecons(t1, tail);
		  return t2;
	}
	return prefix(op, treecons(t1, tail));
}

/* mkpipe -- assemble a pipe from the commands that make it up (destructive) */
extern Tree *mkpipe(Tree *t1, int outfd, int infd, Tree *t2) {
	Tree *tail;
	Boolean pipetail;

	pipetail = firstis(t2, "%pipe");
	tail = prefix(str("%d", outfd),
		      prefix(str("%d", infd),
			     pipetail ? t2->CDR : treecons(thunkify(t2), NULL)));
	if (firstis(t1, "%pipe"))
		return treeappend(t1, tail);
	t1 = thunkify(t1);
	if (pipetail) {
		  t2->CDR = treecons(t1, tail);
		  return t2;
	}
	return prefix("%pipe", treecons(t1, tail));
}

/*
 * redirections -- these involve queueing up redirection in the prefix of a
 *	tree and then rewriting the tree to include the appropriate commands
 */

static Tree placeholder = { nRedir, {{NULL}} };

extern Tree *redirect(Tree *t) {
	Tree *r, *p;
	if (t == NULL)
		return NULL;
	if (t->kind != nRedir)
		return t;
	r = t->CAR;
	t = t->CDR;
	for (; r->kind == nRedir; r = r->CDR)
		t = treeappend(t, r->CAR);
	for (p = r; p->CAR != &placeholder; p = p->CDR) {
		assert(p != NULL);
		assert(p->kind == nList);
	}
	if (firstis(r, "%heredoc"))
		if (!queueheredoc(r))
			return &errornode;
	p->CAR = thunkify(redirect(t));
	return r;
}

extern Tree *mkredircmd(char *cmd, int fd) {
	return prefix(cmd, prefix(str("%d", fd), NULL));
}

extern Tree *mkredir(Tree *cmd, Tree *file) {
	Tree *word = NULL;
	if (file != NULL && file->kind == nThunk) {	/* /dev/fd operations */
		char *op;
		Tree *var;
		static int id = 0;
		if (firstis(cmd, "%open"))
			op = "%readfrom";
		else if (firstis(cmd, "%create"))
			op = "%writeto";
		else {
			yyerror("bad /dev/fd redirection");
			op = "";
		}
		var = mk(nWord, str("_devfd%d", id++));
		cmd = treecons(
			mk(nWord, op),
			treecons(var, NULL)
		);
		word = treecons(mk(nVar, var), NULL);
	} else if (!firstis(cmd, "%heredoc") && !firstis(cmd, "%here"))
		file = mk(nCall, prefix("%one", treecons(file, NULL)));
	cmd = treeappend(
		cmd,
		treecons(
			file,
			treecons(&placeholder, NULL)
		)
	);
	if (word != NULL)
		cmd = mk(nRedir, word, cmd);
	return cmd;
}

/* mkclose -- make a %close node with a placeholder */
extern Tree *mkclose(int fd) {
	return prefix("%close", prefix(str("%d", fd), treecons(&placeholder, NULL)));
}

/* mkdup -- make a %dup node with a placeholder */
extern Tree *mkdup(int fd0, int fd1) {
	return prefix("%dup",
		      prefix(str("%d", fd0),
			     prefix(str("%d", fd1),
				    treecons(&placeholder, NULL))));
}

/* redirappend -- destructively add to the list of redirections, before any other nodes */
extern Tree *redirappend(Tree *tree, Tree *r) {
	Tree *t, **tp;
	for (; r->kind == nRedir; r = r->CDR)
		tree = treeappend(tree, r->CAR);
	assert(r->kind == nList);
	for (t = tree, tp = &tree; t != NULL && t->kind == nRedir; t = *(tp = &t->CDR))
		;
	assert(t == NULL || t->kind == nList);
	*tp = mk(nRedir, r, t);
	return tree;
}

/* mkmatch -- rewrite match as appropriate if with ~ commands */
extern Tree *mkmatch(Tree *subj, Tree *cases) {
	const char *varname = "matchexpr";
	Tree *sass, *svar, *matches;
	/*
	 * Empty match -- with no patterns to match the subject,
	 * it's like saying {if}, which simply returns true.
	 * This avoids an unnecessary call.
	 */
	if (cases == NULL)
		return thunkify(NULL);
	/*
	 * Prevent backquote substitution in the subject from executing
	 * repeatedly by assigning it to a temporary variable and using that
	 * variable as the first argument to '~' .
	 */
	sass = treecons(mk(nAssign, mk(nWord, varname), subj), NULL);
	svar = mk(nVar, mk(nWord, varname));
	matches = NULL;
	for (; cases != NULL; cases = cases->CDR) {
		Tree *match;
		Tree *pattlist = cases->CAR->CAR;
		Tree *cmd = cases->CAR->CDR;
		if (pattlist != NULL && pattlist->kind != nList)
			pattlist = treecons(pattlist, NULL);
		match = treecons(
			thunkify(mk(nMatch, svar, pattlist)),
			treecons(cmd, NULL)
		);
		matches = treeappend(matches, match);
	}
	matches = thunkify(prefix("if", matches));
	return mk(nLocal, sass, matches);
}

/* firstprepend -- insert a command node before its arg nodes after all redirections */
extern Tree *firstprepend(Tree *first, Tree *args) {
	Tree *t, **tp;
	if (first == NULL)
		return args;
	for (t = args, tp = &args; t != NULL && t->kind == nRedir; t = *(tp = &t->CDR))
		;
	assert(t == NULL || t->kind == nList);
	*tp = treecons(first, t);
	if (args->kind == nRedir)
		return args;
	return *tp;
}
