/* input.h -- definitions for es lexical analyzer ($Revision: 1.1.1.1 $) */

#define	MAXUNGET	2		/* maximum 2 character pushback */

typedef enum { NW, RW, KW } WordState;

typedef struct Input Input;
struct Input {
	/* reading state */
	int (*get)(Input *self);
	int (*fill)(Input *self), (*rfill)(Input *self);
	void (*cleanup)(Input *self);
	Input *prev;
	const char *name;
	unsigned char *buf, *bufend, *bufbegin, *rbuf;
	size_t buflen;
	int unget[MAXUNGET];
	int ungot;
	int lineno;
	int fd;
	int runflags;
	Boolean ignoreeof;

	/* parsing state */
	void *pspace;
	Boolean parsing;
	Tree *parsetree;
	const char *error;

	/* lexing state */
	WordState ws;
	Boolean goterror, dollar;
	size_t bufsize;
	char *tokenbuf;
};


#define	GETC()		(*input->get)(input)
#define	UNGETC(c)	unget(input, c)


/* input.c */

extern Input *input;
extern void unget(Input *in, int c);
extern Boolean ignoreeof;
extern void yyerror(const char *s);


/* token.c */

/* this is very awkward.  how to otherwise get YYSTYPE? */
#include "token.h"

extern const char dnw[];
extern int yylex(YYSTYPE *y);
extern void inityy(Input *in);
extern void increment_line(void);


/* parse.y */

extern int yyparse(void);


/* heredoc.c */

extern void emptyherequeue(void);
