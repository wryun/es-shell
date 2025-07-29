#ifndef _yy_defines_h_
#define _yy_defines_h_

#define WORD 257
#define QWORD 258
#define LOCAL 259
#define LET 260
#define FOR 261
#define CLOSURE 262
#define FN 263
#define REDIR 264
#define DUP 265
#define ANDAND 266
#define BACKBACK 267
#define BBFLAT 268
#define BFLAT 269
#define EXTRACT 270
#define CALL 271
#define COUNT 272
#define FLAT 273
#define OROR 274
#define PRIM 275
#define SUB 276
#define NL 277
#define ENDFILE 278
#define ERROR 279
#define MATCH 280
#define PIPE 281
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union YYSTYPE {
	Tree *tree;
	char *str;
	NodeKind kind;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */

#endif /* _yy_defines_h_ */
