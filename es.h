/* es.h -- definitions for higher order shell */

#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>

#include "config.h"

/*
 * augmentations to standard c environment
 */

#undef FALSE
#undef TRUE
typedef enum { FALSE, TRUE } Boolean;

#ifndef	offsetof
#define	offsetof(t, m)	((size_t) (((char *) &((t *) 0)->m) - (char *)0))
#endif

#define	streq(s, t)		(strcmp(s, t) == 0)
#define	strneq(s, t, n)		(strncmp(s, t, n) == 0)
#define arraysize(a)		((int)(sizeof(a)/sizeof(*a)))
#define	memzero(dest, count)	memset(dest, 0, count)

#if __GNUC__
typedef volatile void noreturn;
#else
typedef void noreturn;
#endif

#if GCDEBUG && NeXT
#define	ABORT()	asm("trap #15")	/* (*(char *) 0 = 'c'); */
#else
#define	ABORT()	abort()
#endif

#if ASSERTIONS
#define assert(expr) \
	do { \
		if (!(expr)) { \
			eprint("%s:%d: assertion failed (%s)\n", \
					__FILE__, __LINE__, #expr); \
			ABORT(); \
		} \
	} while (0)
#else
#define assert(ignore) do ; while (0)
#endif

#ifndef	EOF
#define	EOF	(-1)
#endif

#ifndef	NULL
#define	NULL	((void *) 0)
#endif

extern noreturn abort(void);


/*
 * system protos used; do something about this
 */

/* stdlib */
extern noreturn exit(int);
extern long strtol(const char *num, char **end, int base);
#define	atoi(s)	strtol(s, NULL, 0)

/* string */
extern void *memcpy(void *dst, const void *src, size_t n);
extern void *memset(void *dst, int c, size_t n);

/* syscalls */
extern int read(int fd, void *buf, size_t n);
extern int write(int fd, const void *buf, size_t n);
extern int dup(int fd);
extern int dup2(int srcfd, int dstfd);
extern void close(int fd);
extern int getpagesize(void);
extern int pipe(int[2]);
extern int getpid(void);


/*
 * redirection types:  these would be in token.h, but they are also used for
 * eopen() from open.c
 */

typedef enum {
	rOpen, rCreate, rAppend, rDup, rClose, rPipe, rHeredoc, rHerestring
} RedirKind;


/*
 * the fundamental es data structures.  
 */

typedef struct Tree Tree;
typedef struct Term Term;
typedef struct List List;
typedef struct Binding Binding;
typedef struct Closure Closure;

struct Term {
	char *str;
	Closure *closure;
};

struct List {
	Term *term;
	List *next;
};

struct Binding {
	char *name;
	List *defn;
	Binding *next;
};

struct Closure {
	Binding	*binding;
	Tree *tree;
};


/*
 * parse trees
 */

typedef enum {
	nAssign, nCall, nConcat, nFor, nLambda, nLet, nList, nLocal,
	nMatch, nPrim, nQword, nRec, nThunk, nVar, nVarsub, nWord
} NodeKind;

struct Tree {
	NodeKind kind;
	union {
		int i;
		char *s;
		Tree *p;
	} u[2];
};


/*
 * miscellaneous data structures
 */

typedef struct StrList StrList;
struct StrList {
	char *str;
	StrList *next;
};

typedef struct {
	int len;
	char *vector[1];
} Vector;			/* environment or arguments */


/*
 * our programming environment
 */

/* main.c */

extern Boolean interactive;		/* -i or isatty(input) */
extern Boolean loginshell;		/* -l or $0[0] == '-' */
extern Boolean noexecute;		/* -n */
extern Boolean verbose;			/* -v */
extern Boolean printcmds;		/* -x */

extern Boolean gcdebug;			/* -G */
extern Boolean bugme;			/* -B */


/* term.c */

extern Term *mkterm(char *str, Closure *closure);
extern char *getstr(Term *term);
extern Closure *getclosure(Term *term);
extern Term *termcat(Term *t1, Term *t2);


/* list.c */

extern List *mklist(Term *term, List *next);
extern List *reverse(List *list);
extern List *append(List *head, List *tail);
extern List *listcopy(List *list);
extern int length(List *list);
extern List *listify(int argc, char **argv);
extern Term *nth(List *list, int n);


/* tree.c */

extern Tree *mk(NodeKind, ...);
extern Tree *revtree(Tree *tree);


/* closure.c */

extern Closure *mkclosure(Tree *tree, Binding *binding);
extern Closure *extractbindings(Tree *tree);
extern Binding *bind(char *name, List *defn, Binding *next);


/* eval.c */

extern List *walk(Tree *tree, Binding *binding, Boolean parent);
extern List *eval(List *list, Binding *binding, Boolean parent);
extern List *eval1(Term *term, Boolean parent);


/* glom.c */

extern List *glom(Tree *tree, Binding *binding, Boolean globit);
extern List *glom2(Tree *tree, Binding *binding, StrList **quotep);


/* glob.c */

extern char QUOTED[], UNQUOTED[];
extern List *glob(List *list, StrList *quote);


/* match.c */
extern Boolean match(const char *subject, const char *pattern, const char *quote);
extern Boolean listmatch(List *subject, List *pattern, StrList *quote);


/* var.c */

extern void initvars(char **envp, const char *initial, Boolean protected);
extern char *varname(List *);
extern List *varlookup(char *, Binding *);
extern List *varlookup2(char *name1, char *name2);
extern void vardef(char *, Binding *, List *);
extern void varpush(char *, List *);
extern void varpop(char *);
extern Vector *mkenv(void);
extern void noexport(char *name);


/* status.c */

extern List *true, *false;
extern Boolean istrue(List *status);
extern int exitstatus(List *status);
extern char *mkstatus(int status);
extern void printstatus(int pid, int status);


/* proc.c */

extern Boolean hasforked;
extern int efork(Boolean parent, Boolean continuing, Boolean background);
extern int ewaitfor(int pid);


/* which.c */

extern char *which(char *name, Boolean verbose);


/* dict.c */

typedef struct Dict Dict;
extern Dict *mkdict(void);
extern void dictforall(Dict *dict, void (*proc)(void *, char *, void *), void *arg);
extern void *dictget(Dict *dict, const char *name);
extern Dict *dictput(Dict *dict, char *name, void *value);
extern void *dictget2(Dict *dict, const char *name1, const char *name2);


/* conv.c */

extern void initconv(void);
extern int defaultfd(RedirKind op);


/* print.c -- see print.h for more */

extern int print(const char *fmt,...);
extern int eprint(const char *fmt,...);
extern int fprint(int fd, const char *fmt,...);
extern noreturn panic(const char *fmt, ...);
extern void debug(const char *fmt, ...);


/* str.c */

extern char *str(const char *fmt, ...);		/* create a gc space string by printing */
extern char *mprint(const char *fmt, ...);	/* create an ealloc space string by printing */
extern StrList *mkstrlist(char *, StrList *);


/* vec.c */

extern Vector *mkvector(int n);
extern Vector *vectorize(List *list);


/* util.c */

extern char *strerror(int err);
extern void uerror(char *msg);
extern void *ealloc(size_t n);
extern void *erealloc(void *p, size_t n);
extern void efree(void *p);
extern void ewrite(int fd, char *s, size_t n);
extern long eread(int fd, char *buf, size_t n);
extern int mvfd(int old, int new);
extern int cpfd(int old, int new);
extern Boolean isabsolute(char *path);


/* input.c */

extern char *prompt, *prompt2;
extern List *runstring(const char *str);
extern Tree *parsestring(const char *str);
extern List *runfd(int fd);
extern void sethistory(char *file);

extern int gchar(void);
extern void ugchar(int c);
extern void flushu(void);
extern void closefds(void);
extern void initinput(void);


/* prim.c */

extern List *prim(char *s, List *list, Boolean parent);
extern void initprims(void);


/* split.c */

extern void startsplit(const char *sep, Boolean coalesce);
extern void splitstring(char *in, size_t len, Boolean endword);
extern List *endsplit(void);


/* signal.c */

#if USE_SIG_ATOMIC_T
#define SIG_ATOMIC_T sig_atomic_t
#else
#define SIG_ATOMIC_T int
#endif

#define SIGCHK() sigchk()
extern volatile SIG_ATOMIC_T slow, interrupted;
extern jmp_buf slowlabel;
extern Boolean sigint_newline;
extern void catcher(int sig);
extern void sigchk(void);
extern void (*esignal(int sig, void (*h)(int)))(int);
extern void initsignals(Boolean allowdumps);
extern Boolean issilentsignal(List *e);
extern void setsigdefaults(Boolean background);


/* open.c */

extern int eopen(char *name, RedirKind k);


/* version.c */

extern const char * const version;


/* gc.c -- see gc.h for more */

typedef struct Tag Tag;
#define	gcnew(type)	((type *) gcalloc(sizeof (type), &type ## Tag))

extern void *gcalloc(size_t n, Tag *t);		/* allocate n with collection tag t */
extern char *gcdup(const char *s);		/* copy a 0-terminated string into gc space */
extern char *gcndup(const char *s, size_t n);	/* copy a counted string into gc space */

extern void initgc(void);			/* must be called at the dawn of time */
extern void gc(void);				/* provoke a collection, if enabled */
extern void gcenable(void);			/* enable collections */
extern void gcdisable(size_t);			/* disable collections, collect first if space needed */

#ifndef GARBAGE_COLLECTOR
extern int gcblocked;
#endif


/*
 * garbage collector tags
 */

typedef struct Root Root;
struct Root {
	void **p;
	Root *next;
};

extern Root *rootlist;

#define	Ref(t, v, init) \
	do { \
		t v = init; \
		Root (__root__ ## v); \
		(__root__ ## v).p = (void **) &v; \
		(__root__ ## v).next = rootlist; \
		rootlist = &(__root__ ## v)
#define	RefPop(v) \
		assert(rootlist == &(__root__ ## v)); \
		assert(rootlist->p == (void **) &v); \
		rootlist = rootlist->next;
#define RefEnd(v) \
		assert(rootlist == &(__root__ ## v)); \
		assert(rootlist->p == (void **) &v); \
		RefPop(v); \
	} while (0)
#define RefReturn(v)	/* { */	\
		assert(rootlist == &(__root__ ## v)); \
		assert(rootlist->p == (void **) &v); \
		RefPop(v); \
		return v; \
	} while (0)

#define	RefEnd2(v1, v2)		RefEnd(v1); RefEnd(v2)
#define	RefEnd3(v1, v2, v3)	RefEnd(v1); RefEnd2(v2, v3)
#define	RefEnd4(v1, v2, v3, v4)	RefEnd(v1); RefEnd3(v2, v3, v4)

#define	RefPop2(v1, v2)		RefPop(v1); RefPop(v2)
#define	RefPop3(v1, v2, v3)	RefPop(v1); RefPop2(v2, v3)
#define	RefPop4(v1, v2, v3, v4)	RefPop(v1); RefPop3(v2, v3, v4)

extern void globalroot(void *addr);


/*
 * exceptions
 *	typical use is
 *		Handler h;
 *		List *e;
 *		while ((e = pushhandler(&h)) != NULL) {
 *			if (we want to pass e back)
 *				throw(e);
 *			if (we can return)
 *				return ...;
 *			// otherwise just retry
 *		}
 *		... // protected code
 *		pophandler(&h);
 */

typedef struct Handler Handler;

struct Handler {
	Handler *up;
	Root *rootlist;
	jmp_buf label;
};

extern Handler childhandler, *bottomhandler;
extern List *exception;
extern List *pushhandler(Handler *handler);	/* must be a macro */
extern void pophandler(Handler *handler);
extern noreturn throw(List *exc);
extern noreturn fail(const char *name, ...);
extern void newchildcatcher(void);

#define	pushhandler(hp)	( \
		((hp)->rootlist = rootlist), \
		((hp)->up = bottomhandler), \
		(bottomhandler = (hp)), \
		(setjmp((hp)->label) ? exception : NULL) \
	)

