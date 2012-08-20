/* parse.y -- grammar for es ($Revision: 1.2 $) */

%{
/* Some yaccs insist on including stdlib.h */
#define _STDLIB_H
#include "es.h"
#include "input.h"
#include "syntax.h"
%}

%token	WORD QWORD
%token	LOCAL LET FOR CLOSURE FN
%token	ANDAND BACKBACK EXTRACT CALL COUNT DUP FLAT OROR PRIM REDIR SUB
%token	NL ENDFILE ERROR

%left	LOCAL LET FOR CLOSURE ')'
%left	ANDAND OROR NL
%left	'!'
%left	PIPE
%right	'$' 
%left	SUB

%union {
	Tree *tree;
	char *str;
	NodeKind kind;
}

%type <str>	WORD QWORD keyword
%type <tree>	REDIR PIPE DUP
		body cmd cmdsa cmdsan comword first fn line word param assign
		binding bindings params nlwords words simple redir sword
%type <kind>	binder

%start es

%%

es	: line end		{ parsetree = $1; YYACCEPT; }
	| error end		{ yyerrok; parsetree = NULL; YYABORT; }

end	: NL			{ if (!readheredocs(FALSE)) YYABORT; }
	| ENDFILE		{ if (!readheredocs(TRUE)) YYABORT; }

line	: cmd			{ $$ = $1; }
	| cmdsa line		{ $$ = mkseq("%seq", $1, $2); }

body	: cmd			{ $$ = $1; }
	| cmdsan body		{ $$ = mkseq("%seq", $1, $2); }

cmdsa	: cmd ';'		{ $$ = $1; }
	| cmd '&'		{ $$ = prefix("%background", mk(nList, thunkify($1), NULL)); }

cmdsan	: cmdsa			{ $$ = $1; }
	| cmd NL		{ $$ = $1; if (!readheredocs(FALSE)) YYABORT; }

cmd	:		%prec LET		{ $$ = NULL; }
	| simple				{ $$ = redirect($1); if ($$ == &errornode) YYABORT; }
	| redir cmd	%prec '!'		{ $$ = redirect(mk(nRedir, $1, $2)); if ($$ == &errornode) YYABORT; }
	| first assign				{ $$ = mk(nAssign, $1, $2); }
	| fn					{ $$ = $1; }
	| binder nl '(' bindings ')' nl cmd	{ $$ = mk($1, $4, $7); }
	| cmd ANDAND nl cmd			{ $$ = mkseq("%and", $1, $4); }
	| cmd OROR nl cmd			{ $$ = mkseq("%or", $1, $4); }
 	| cmd PIPE nl cmd			{ $$ = mkpipe($1, $2->u[0].i, $2->u[1].i, $4); }
	| '!' caret cmd				{ $$ = prefix("%not", mk(nList, thunkify($3), NULL)); }
	| '~' word words			{ $$ = mk(nMatch, $2, $3); }
	| EXTRACT word words			{ $$ = mk(nExtract, $2, $3); }

simple	: first				{ $$ = treecons2($1, NULL); }
	| simple word			{ $$ = treeconsend2($1, $2); }
	| simple redir			{ $$ = redirappend($1, $2); }

redir	: DUP				{ $$ = $1; }
	| REDIR word			{ $$ = mkredir($1, $2); }

bindings: binding			{ $$ = treecons2($1, NULL); }
	| bindings ';' binding		{ $$ = treeconsend2($1, $3); }
	| bindings NL binding		{ $$ = treeconsend2($1, $3); }

binding	:				{ $$ = NULL; }
	| fn				{ $$ = $1; }
	| word assign			{ $$ = mk(nAssign, $1, $2); }

assign	: caret '=' caret words		{ $$ = $4; }

fn	: FN word params '{' body '}'	{ $$ = fnassign($2, mklambda($3, $5)); }
	| FN word			{ $$ = fnassign($2, NULL); }

first	: comword			{ $$ = $1; }
	| first '^' sword		{ $$ = mk(nConcat, $1, $3); }

sword	: comword			{ $$ = $1; }
	| keyword			{ $$ = mk(nWord, $1); }

word	: sword				{ $$ = $1; }
	| word '^' sword		{ $$ = mk(nConcat, $1, $3); }

comword	: param				{ $$ = $1; }
	| '(' nlwords ')'		{ $$ = $2; }
	| '{' body '}'			{ $$ = thunkify($2); }
	| '@' params '{' body '}'	{ $$ = mklambda($2, $4); }
	| '$' sword			{ $$ = mk(nVar, $2); }
	| '$' sword SUB words ')'	{ $$ = mk(nVarsub, $2, $4); }
	| CALL sword			{ $$ = mk(nCall, $2); }
	| COUNT sword			{ $$ = mk(nCall, prefix("%count", treecons(mk(nVar, $2), NULL))); }
	| FLAT sword			{ $$ = flatten(mk(nVar, $2), " "); }
	| PRIM WORD			{ $$ = mk(nPrim, $2); }
	| '`' sword			{ $$ = backquote(mk(nVar, mk(nWord, "ifs")), $2); }
	| BACKBACK word	sword		{ $$ = backquote($2, $3); }

param	: WORD				{ $$ = mk(nWord, $1); }
	| QWORD				{ $$ = mk(nQword, $1); }

params	:				{ $$ = NULL; }
	| params param			{ $$ = treeconsend($1, $2); }

words	:				{ $$ = NULL; }
	| words word			{ $$ = treeconsend($1, $2); }

nlwords :				{ $$ = NULL; }
	| nlwords word			{ $$ = treeconsend($1, $2); }
	| nlwords NL			{ $$ = $1; }

nl	:
	| nl NL

caret 	:
	| '^'

binder	: LOCAL	        { $$ = nLocal; }
	| LET		{ $$ = nLet; }
	| FOR		{ $$ = nFor; }
	| CLOSURE	{ $$ = nClosure; }

keyword	: '!'		{ $$ = "!"; }
	| '~'		{ $$ = "~"; }
	| EXTRACT	{ $$ = "~~"; }
        | LOCAL 	{ $$ = "local"; }
	| LET		{ $$ = "let"; }
	| FOR		{ $$ = "for"; }
	| FN		{ $$ = "fn"; }
	| CLOSURE	{ $$ = "%closure"; }

