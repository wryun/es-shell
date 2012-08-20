/* input.h -- definitions for es lexical analyzer ($Revision: 1.1.1.1 $) */

#define	MAXUNGET	2		/* maximum 2 character pushback */

typedef struct Input Input;
struct Input {
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
};


#define	GETC()		(*input->get)(input)
#define	UNGETC(c)	unget(input, c)


/* input.c */

extern Input *input;
extern void unget(Input *in, int c);
extern Boolean disablehistory;
extern void yyerror(char *s);


/* token.c */

extern const char dnw[];
extern int yylex(void);
extern void inityy(void);
extern void print_prompt2(void);


/* parse.y */

extern Tree *parsetree;
extern int yyparse(void);
extern void initparse(void);


/* heredoc.c */

extern void emptyherequeue(void);
