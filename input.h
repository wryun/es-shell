/* input.h -- definitions for es lexical analyzer */

/* token.c */

extern const char dnw[];
extern int lineno;
extern int yylex(void);
extern void inityy(void);
extern char *locate(char *s);
extern void scanerror(char *s);
extern void print_prompt2(void);


/* input.c */

extern Boolean disablehistory;
extern int last;
extern int gchar(void);
extern void ugchar(int);
extern void flushu(void);
extern void yyerror(char *s);


/* parse.y */

extern Tree *parsetree;
extern int yyparse(void);
extern void initparse(void);


/* heredoc.c */

extern void emptyherequeue(void);
