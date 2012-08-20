/* token.h -- definitions for es lexical analyzer */

typedef struct Redir Redir;

struct Redir {
	RedirKind kind;
	int fd[2];
	Redir *next;
	Tree *tree;
};

#ifndef YACCING
#include "y.tab.h"	/* TODO: use x.tab.h hack */
#endif


/* token.c */

extern int lineno;
extern int yylex(void);
extern void inityy(void);
extern char *locate(char *s);
extern void scanerror(char *s);


/* input.c */

extern int last;
extern int gchar(void);
extern void ugchar(int);
extern void flushu(void);
extern void yyerror(char *s);


/* parse.y */

extern Tree *parsetree;
extern int yyparse(void);
extern void initparse(void);
