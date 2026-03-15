/* input.h -- definitions for es lexical analyzer ($Revision: 1.1.1.1 $) */

#define	MAXUNGET	2		/* maximum 2 character pushback */

#include "token.h"	/* for YYSTYPE */

/* Input contains state that lasts longer than a $&parse. */
struct Input {
	/* input buffer */
	size_t buflen;
	unsigned char *buf, *bufend, *bufbegin;

	/* input metadata and flags */
	const char *name;
	int lineno;
	int fd;
	int runflags;
	Boolean eof;

	/* TODO: these belong in Parser */
	char *prompt, *prompt2;	/* pspace-allocated */
};

typedef enum { NW, RW, KW } WordState;	/* nonword, realword, keyword */

/* Parser contains state that lasts for one call to $&parse or less. */
struct Parser {
	Input *input;
	void *space;	/* where the parse tree is kept in memory */

	/* these variables are all allocated in pspace */
	Tree *tree;		/* the final parse tree */
	Here *hereq;		/* pending here document queue */
	const char *error;	/* syntax error, if it exists */

	/* token pushback buffer */
	int unget[MAXUNGET];
	int ungot;

	/* lexer state */
	WordState ws;
	Boolean newline, goterror, dollar;
	size_t bufsize;
	char *tokenbuf;
};


/* input.c */

extern int get(Parser *p);
extern void unget(Parser *p, int c);
extern void yyerror(Parser *p, const char *s);


/* token.c */

extern const char dnw[];
extern int yylex(YYSTYPE *y, Parser *p);
extern void inityy(Parser *p);
extern void print_prompt2(Parser *p);


/* parse.y */

extern int yyparse(Parser *p);
