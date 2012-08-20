/* parse.y -- grammar for es */

/*
 * Adapted from rc grammar, v10 manuals, volume 2.
 */

%{
#define	YACCING	1	/* disable include of "y.tab.h" in "token.h" */

#include "es.h"
#include "token.h"
static Tree *star, *ifs;
Tree *parsetree;	/* not using yylval because bison declares it as an auto */


/* TODO */
extern Boolean heredoc(Boolean);
extern Boolean qdoc(Tree *, Tree *);
%}

%token	'@' '!' '~' '|'
%token	WORD QWORD
%token	LOCAL LET FOR FN SWITCH CASE
%token	ANDAND BACKBACK BOX COUNT DUP FLAT OROR PRIM REDIR SUB
%token	END ERROR

%left LOCAL LET FOR ')'
%left ANDAND OROR '\n'
%left '!'
%left '|'
%right '$' 
%left SUB

%union {
	char *str;
	Tree *tree;
	Tree *revtree;
	Redir *redir;
	struct {
		Tree *tree;
		Redir *redir;
	} simple;
	NodeKind kind;
	int count;
}

%type <redir>	REDIR DUP '|'
%type <str>	WORD QWORD keyword
%type <tree>	body cmd cmdsa cmdsan comword
		first fn line nlwords sword word words
		pword param params binding bindings
%type <simple>	simple redir
%type <kind>	binder
%type <count>	rec

%start es

%%

es	: line end		{ parsetree = $1; YYACCEPT; }
	| error end		{ yyerrok; parsetree = NULL; YYABORT; }

/* an es line may end in end-of-file as well as newline, e.g., es -c 'ls' */
end	: END	/* EOF */	{ if (!heredoc(1)) YYABORT; } /* flag error if there is a heredoc in the queue */
	| '\n'			{ if (!heredoc(0)) YYABORT; } /* get heredoc on \n */

/* a cmdsa is a command followed by ampersand or newline (used in "line" and "body") */
cmdsa	: cmd ';'		{ $$ = $1; }
	| cmd '&'		{ $$ = prefix("%background", mk(nList, thunkify($1), NULL)); }

/* a line is a single command, or a command terminated by ; or & followed by a line (recursive) */
line	: cmd			{ $$ = $1; }
	| cmdsa line		{ $$ = mkseq("%seq", $1, $2); }

/* a body is like a line, only commands may also be terminated by newline */
body	: cmd
	| cmdsan body		{ $$ = mkseq("%seq", $1, $2); }

cmdsan	: cmdsa
	| cmd '\n'		{ $$ = $1; if (!heredoc(0)) YYABORT; } /* get h.d. on \n */

/* a redirection is a dup (e.g., >[1=2]) or a file redirection. (e.g., > /dev/null) */
redir	: DUP			{ $$.redir = $1; $$.tree = NULL; }
	| REDIR word		{
				  if ($2->kind == nThunk) {
				  	if (($1->kind != rOpen && $1->kind != rCreate)
					    || $1->fd[0] != defaultfd($1->kind) || $1->fd[1] >= -1) {
						yyerror("named pipe syntax");
						YYABORT;
					}
					$$.redir = NULL;
					$$.tree = mk(nCall, prefix($1->kind == rOpen ? "%readfrom" : "%writeto",
								   $2->u[0].p));
				  } else {
					$$.tree = NULL;
				  	$$.redir = $1;
					$$.redir->tree = $2;
				  }
				}

cmd	:		%prec LET		{ $$ = NULL; }
	| first caret '=' caret words		{ $$ = mk(nAssign, $1, revtree($5)); }
	| fn					{ $$ = $1; }
	| simple				{ $$ = redirect(revtree($1.tree), $1.redir); }
	| redir cmd	%prec '!'		{ $$ = redirect(cons($1.tree, $2), $1.redir); }
	| binder nl '(' bindings ')' nl cmd	{ $$ = mk($1, revtree($4), $7); }
	| cmd ANDAND nl cmd			{ $$ = mkseq("%and", $1, $4); }
	| cmd OROR nl cmd			{ $$ = mkseq("%or", $1, $4); }
 	| cmd '|' nl cmd			{ $$ = mkpipe($1, $2, $4); }
	| '~' caret word words			{ $$ = mk(nMatch, $3, revtree($4)); }
	| '!' caret cmd				{ $$ = prefix("%not", mk(nList, thunkify($3), NULL)); }

fn	: FN word params '{' body '}'		{ $$ = mk(nAssign, mk(nConcat, mk(nWord, "fn-"), $2),
								   mk(nLambda, revtree($3), $5)); }
	| FN word				{ $$ = mk(nAssign, mk(nConcat, mk(nWord, "fn-"), $2), NULL); }

bindings: binding			{ $$ = cons($1, NULL); }
	| bindings ';' binding		{ $$ = cons($3, $1); }
	| bindings '\n' binding		{ $$ = cons($3, $1); }

binding	:				{ $$ = NULL; }
	| word caret '=' caret words	{ $$ = mk(nAssign, $1, revtree($5)); }
	| fn				{ $$ = $1; }

simple	: first				{ $$.tree = cons($1, NULL); $$.redir = NULL; }
	| simple word			{ $$.tree = cons($2, $1.tree); $$.redir = $1.redir; }
	| simple redir			{ $$.tree = cons($2.tree, $1.tree);
					  $$.redir = mergeredir($2.redir, $1.redir); }

first	: comword			{ $$ = $1; }
	| first '^' sword		{ $$ = mk(nConcat, $1, $3); }

sword	: comword			{ $$ = $1; }
	| keyword			{ $$ = mk(nWord, $1); }

word	: sword				{ $$ = $1; }
	| word '^' sword		{ $$ = mk(nConcat, $1, $3); }

comword	: pword				{ $$ = $1; }
	| '(' nlwords ')'		{ $$ = revtree($2); }
	| '{' body '}'			{ $$ = thunkify($2); }
	| '@' params '{' body '}'	{ $$ = mk(nLambda, revtree($2), $4); }
	| '$' sword			{ $$ = mk(nVar, $2); }
	| '$' sword SUB words ')'	{ $$ = mk(nVarsub, $2, revtree($4)); }
	| PRIM WORD			{ $$ = mk(nPrim, $2); }
	| BOX sword			{ $$ = mk(nCall, unthunkify($2)); }
	| COUNT sword			{ $$ = mk(nCall, prefix("%count", cons(mk(nVar, $2), NULL))); }
	| FLAT sword			{ $$ = flatten(cons(mk(nVar, $2), NULL), " "); }
	| '`' sword			{ $$ = backquote(flatten(cons(mk(nVar, mk(nWord, "ifs")), NULL), ""), $2); }
	| BACKBACK word	sword		{ $$ = backquote(flatten(cons($2, NULL), ""), $3); }

params	:				{ $$ = NULL; }
	| params param			{ $$ = cons($2, $1); }

param	: pword				{ $$ = $1; }
	| pword caret '=' caret word	{ $$ = mk(nAssign, $1, $5); }
	| pword rec			{ $$ = mk(nRec, $2, $1); }

pword	: WORD				{ $$ = mk(nWord, $1); }
	| QWORD				{ $$ = mk(nQword, $1); }

rec	: '&'				{ $$ = 1; }
	| ANDAND			{ $$ = 2; }
	| rec '&'			{ $$ = $1 + 1; }
	| rec ANDAND			{ $$ = $1 + 2; }

binder	: LOCAL		{ $$ = nLocal; }
	| LET		{ $$ = nLet; }
	| FOR		{ $$ = nFor; }

keyword	: '~'		{ $$ = "~"; }
	| '!'		{ $$ = "!"; }
	| LOCAL		{ $$ = "local"; }
	| LET		{ $$ = "let"; }
	| FOR		{ $$ = "for"; }
	| FN		{ $$ = "fn"; }
	| SWITCH	{ $$ = "switch"; }
	| CASE		{ $$ = "case"; }

words	:		{ $$ = NULL; }
	| words word	{ $$ = cons($2, $1); }

nlwords :		{ $$ = NULL; }
	| nlwords '\n'	{ $$ = cons($1, NULL); }
	| nlwords word	{ $$ = cons($2, $1); }

nl	:
	| nl '\n'

caret 	:
	| '^'

%%

#define	CAR(list)	((list)->u[0].p)
#define	CDR(list)	((list)->u[1].p)

extern Tree *cons(Tree *car, Tree *cdr) {
	assert(cdr == NULL || cdr->kind == nList);
	return mk(nList, car, cdr);
}

/* initparse -- called at the dawn of time */
extern void initparse(void) {
	globalroot(&parsetree);
	globalroot(&star);
	globalroot(&ifs);

	star = mk(nVar, mk(nWord, "*"));
	ifs = mk(nVar, mk(nWord, "ifs"));
}

/* mergeredir -- destructive merging of redirections */
static Redir *mergeredir(Redir *r1, Redir *r2) {
	if (r1 == NULL)
		return r2;
	if (r2 != NULL) {
		Redir *r;
		for (r = r1; r->next != NULL; r = r->next)
			assert(r != NULL);
		r->next = r2;
	}
	return r1;
}

/* prefix -- prefix a tree with a given word */
static Tree *prefix(char *s, Tree *t) {
	return cons(mk(nWord, s), t);
}

/* thunkify -- wrap a tree in thunk braces if it isn't already a thunk */
static Tree *thunkify(Tree *tree) {
	if (tree != NULL && tree->kind == nThunk)
		return tree;
	return mk(nThunk, tree);
}

/* unthunkify -- remove thunk braces from a tree if it is one */
static Tree *unthunkify(Tree *tree) {
	/* if (tree != NULL && tree->kind == nThunk)
		return tree->u[0].p; */
	return tree;
}

/* redirect -- build up a tree from a tree and some redirections */
static Tree *redirect(Tree *t, Redir *r) {
	for (; r != NULL; r = r->next) {
		char *op = NULL; /* uninit'd use ok'd */
		switch (r->kind) {
		case rOpen:	op = "%open"; break;
		case rCreate:	op = "%create"; break;
		case rAppend:	op = "%append"; break;
		case rClose:	op = "%close"; break;
		case rDup:	op = "%dup"; break;
		case rHerestring:
		case rHeredoc:	op = "%here"; break;
		default:	panic("redirect: bad redirection kind %d", r->kind);
		}
		t = cons(thunkify(t), NULL);
		if (r->kind != rHerestring && r->kind != rHeredoc && r->kind != rDup) {
			Tree *file = r->tree;
			if (file == NULL)
				yyerror("null filename in redirection");
			if (file->kind != nList)
				file = cons(file, NULL);
			t = cons(mk(nCall, prefix("%one", file)), t);
		}
		if (r->fd[1] >= 0)
			t = prefix(str("%d", r->fd[1]), t);
		if (r->fd[0] >= 0)
			t = prefix(str("%d", r->fd[0]), t);
		t = prefix(op, t);
	}
	return t;
#if 0
	if ($1->kind == rHeredoc && !qdoc($2, $$)) YYABORT; /* TODO: queue heredocs up */
#endif
}

/* first -- return the first word of a command, if it is a literal */
static char *first(Tree *t) {
	if (t == NULL || t->kind != nList)
		return "";
	t = CAR(t);
	if (t == NULL || t->kind != nWord)
		return "";
	assert(t->u[0].s);
	return t->u[0].s;
}

/* append -- destructive tree appending for nList/nThunk lists */
static Tree *appendtree(Tree *t1, Tree *t2) {
	Tree *t, *next;
	assert(t1 != NULL && t1->kind == nList);
	assert(t2 == NULL || t2->kind == nList);
	for (t = t1; (next = CDR(t)) != NULL; t = next)
		assert(t->kind == nList);
	assert(t->kind == nList);
	CDR(t) = t2;
	return t1;
}

/* mkseq -- destructively add to a sequence of nList/nThink operations */
static Tree *mkseq(char *op, Tree *t1, Tree *t2) {
	Tree *tail;
	Boolean sametail;

	if (streq(op, "%seq")) {
		if (t1 == NULL)
			return t2;
		if (t2 == NULL)
			return t1;
	}
	
	sametail = streq(first(t2), op);
	tail = sametail ? CDR(t2) : cons(thunkify(t2), NULL);
	if (streq(first(t1), op))
		return appendtree(t1, tail);
	t1 = thunkify(t1);
	if (sametail) {
		  CDR(t2) = cons(t1, tail);
		  return t2;
	}
	return prefix(op, cons(t1, tail));
}

/* mkpipe -- assemble a pipe from the commands that make it up (destructive) */
static Tree *mkpipe(Tree *t1, Redir *pipe, Tree *t2) {
	Tree *tail;
	Boolean pipetail = streq(first(t2), "%pipe");

	tail = prefix(str("%d", pipe->fd[0]),
		      prefix(str("%d", pipe->fd[1]),
			     pipetail ? CDR(t2) : cons(thunkify(t2), NULL)));
	if (streq(first(t1), "%pipe"))
		return appendtree(t1, tail);
	t1 = thunkify(t1);
	if (pipetail) {
		  CDR(t2) = cons(t1, tail);
		  return t2;
	}
	return prefix("%pipe", cons(t1, tail));
}

/* flatten -- flatten the output of the glommer so we can pass the result as a single element */
static Tree *flatten(Tree *t, char *sep) {
	return mk(nCall, prefix("%flatten", cons(mk(nQword, sep), t)));
}

/* backquote -- create a backquote command */
static Tree *backquote(Tree *ifs, Tree *body) {
	return mk(nCall, prefix("%backquote", cons(ifs, cons(unthunkify(body), NULL))));
}
