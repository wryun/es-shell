/* es.h -- definitions for higher order shell ($Revision: 1.2 $) */

#include "config.h"
#include "stdenv.h"

/*
 * meta-information for exported environment strings
 */

#define ENV_SEPARATOR	'\001'		/* control-A */
#define	ENV_ESCAPE	'\002'		/* control-B */


/*
 * the fundamental es data structures.
 */

typedef struct Tree Tree;
typedef struct Term Term;
typedef struct List List;
typedef struct Binding Binding;
typedef struct Closure Closure;

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
	nAssign, nCall, nClosure, nConcat, nFor, nLambda, nLet, nList, nLocal,
	nMatch, nExtract, nPrim, nQword, nThunk, nVar, nVarsub, nWord,
	nRedir, nPipe		/* only appear during construction */
} NodeKind;

struct Tree {
	NodeKind kind;
	union {
		Tree *p;
		char *s;
		int i;
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
	int alloclen, count;
	char *vector[1];
} Vector;			/* environment or arguments */


/*
 * our programming environment
 */


/* main.c */

#if GCVERBOSE
extern Boolean gcverbose;		/* -G */
#endif
#if GCINFO
extern Boolean gcinfo;			/* -I */
#endif


/* initial.c (for es) or dump.c (for esdump) */

extern void runinitial(void);


/* fd.c */

extern void mvfd(int old, int new);
extern int newfd(void);

#define	UNREGISTERED	(-999)
extern void registerfd(int *fdp, Boolean closeonfork);
extern void unregisterfd(int *fdp);
extern void releasefd(int fd);
extern void closefds(void);

extern int fdmap(int fd);
extern int defer_mvfd(Boolean parent, int old, int new);
extern int defer_close(Boolean parent, int fd);
extern void undefer(int ticket);


/* term.c */

extern Term *mkterm(char *str, Closure *closure);
extern Term *mkstr(char *str);
extern char *getstr(Term *term);
extern Closure *getclosure(Term *term);
extern Term *termcat(Term *t1, Term *t2);
extern Boolean termeq(Term *term, const char *s);
extern Boolean isclosure(Term *term);


/* list.c */

extern List *mklist(Term *term, List *next);
extern List *reverse(List *list);
extern List *append(List *head, List *tail);
extern List *listcopy(List *list);
extern int length(List *list);
extern List *listify(int argc, char **argv);
extern Term *nth(List *list, int n);
extern List *sortlist(List *list);


/* tree.c */

extern Tree *mk(NodeKind VARARGS);


/* closure.c */

extern Closure *mkclosure(Tree *tree, Binding *binding);
extern Closure *extractbindings(Tree *tree);
extern Binding *mkbinding(char *name, List *defn, Binding *next);
extern Binding *reversebindings(Binding *binding);


/* eval.c */

extern Binding *bindargs(Tree *params, List *args, Binding *binding);
extern List *forkexec(char *file, List *list, Boolean inchild);
extern List *walk(Tree *tree, Binding *binding, int flags);
extern List *eval(List *list, Binding *binding, int flags);
extern List *eval1(Term *term, int flags);
extern List *pathsearch(Term *term);

extern unsigned long evaldepth, maxevaldepth;
#define	MINmaxevaldepth		100
#define	MAXmaxevaldepth		0xffffffffU

#define	eval_inchild		1
#define	eval_exitonfalse	2
#define	eval_flags		(eval_inchild|eval_exitonfalse)


/* glom.c */

extern List *glom(Tree *tree, Binding *binding, Boolean globit);
extern List *glom2(Tree *tree, Binding *binding, StrList **quotep);


/* glob.c */

extern char QUOTED[], UNQUOTED[];
extern List *glob(List *list, StrList *quote);
extern Boolean haswild(const char *pattern, const char *quoting);


/* match.c */
extern Boolean match(const char *subject, const char *pattern, const char *quote);
extern Boolean listmatch(List *subject, List *pattern, StrList *quote);
extern List *extractmatches(List *subjects, List *patterns, StrList *quotes);


/* var.c */

extern void initvars(void);
extern void initenv(char **envp, Boolean protected);
extern void hidevariables(void);
extern void validatevar(const char *var);
extern List *varlookup(const char *name, Binding *binding);
extern List *varlookup2(char *name1, char *name2, Binding *binding);
extern void vardef(char *, Binding *, List *);
extern Vector *mkenv(void);
extern void setnoexport(List *list);
extern void addtolist(void *arg, char *key, void *value);
extern List *listvars(Boolean internal);

typedef struct Push Push;
extern Push *pushlist;
extern void varpush(Push *, char *, List *);
extern void varpop(Push *);


/* status.c */

extern List *true, *false;
extern Boolean istrue(List *status);
extern int exitstatus(List *status);
extern char *mkstatus(int status);
extern void printstatus(int pid, int status);


/* access.c */

extern char *checkexecutable(char *file);


/* proc.c */

extern Boolean hasforked;
extern int efork(Boolean parent, Boolean background);
extern int ewait(int pid, Boolean interruptible, void *rusage);
#define	ewaitfor(pid)	ewait(pid, FALSE, NULL)


/* dict.c */

typedef struct Dict Dict;
extern Dict *mkdict(void);
extern void dictforall(Dict *dict, void (*proc)(void *, char *, void *), void *arg);
extern void *dictget(Dict *dict, const char *name);
extern Dict *dictput(Dict *dict, char *name, void *value);
extern void *dictget2(Dict *dict, const char *name1, const char *name2);


/* conv.c */

extern void initconv(void);


/* print.c -- see print.h for more */

extern int print(const char *fmt VARARGS);
extern int eprint(const char *fmt VARARGS);
extern int fprint(int fd, const char *fmt VARARGS);
extern noreturn panic(const char *fmt VARARGS);


/* str.c */

extern char *str(const char *fmt VARARGS);	/* create a gc space string by printing */
extern char *mprint(const char *fmt VARARGS);	/* create an ealloc space string by printing */
extern StrList *mkstrlist(char *, StrList *);


/* vec.c */

extern Vector *mkvector(int n);
extern Vector *vectorize(List *list);
extern void sortvector(Vector *v);


/* util.c */

extern char *esstrerror(int err);
extern void uerror(char *msg);
extern void *ealloc(size_t n);
extern void *erealloc(void *p, size_t n);
extern void efree(void *p);
extern void ewrite(int fd, const char *s, size_t n);
extern long eread(int fd, char *buf, size_t n);
extern Boolean isabsolute(char *path);
extern Boolean streq2(const char *s, const char *t1, const char *t2);


/* input.c */

extern char *prompt, *prompt2;
extern Tree *parse(char *esprompt1, char *esprompt2);
extern Tree *parsestring(const char *str);
extern void sethistory(char *file);
extern Boolean isinteractive(void);
extern void initinput(void);
extern void resetparser(void);

extern List *runfd(int fd, const char *name, int flags);
extern List *runstring(const char *str, const char *name, int flags);

/* eval_* flags are also understood as runflags */
#define	run_interactive		 4	/* -i or $0[0] = '-' */
#define	run_noexec		 8	/* -n */
#define	run_echoinput		16	/* -v */
#define	run_printcmds		32	/* -x */
#define	run_lisptrees		64	/* -L and defined(LISPTREES) */

#if READLINE
extern Boolean resetterminal;
#endif


/* opt.c */

extern void esoptbegin(List *list, const char *caller, const char *usage);
extern int esopt(const char *options);
extern Term *esoptarg(void);
extern List *esoptend(void);


/* prim.c */

extern List *prim(char *s, List *list, Binding *binding, int evalflags);
extern void initprims(void);


/* split.c */

extern void startsplit(const char *sep, Boolean coalesce);
extern void splitstring(char *in, size_t len, Boolean endword);
extern List *endsplit(void);
extern List *fsplit(const char *sep, List *list, Boolean coalesce);


/* signal.c */

extern int signumber(const char *name);
extern char *signame(int sig);
extern char *sigmessage(int sig);

#define	SIGCHK() sigchk()
typedef enum {
	sig_nochange, sig_catch, sig_default, sig_ignore, sig_noop, sig_special
} Sigeffect;
extern Sigeffect esignal(int sig, Sigeffect effect);
extern void setsigeffects(const Sigeffect effects[]);
extern void getsigeffects(Sigeffect effects[]);
extern List *mksiglist(void);
extern void initsignals(Boolean interactive, Boolean allowdumps);
extern Atomic slow, interrupted;
extern jmp_buf slowlabel;
extern Boolean sigint_newline;
extern void sigchk(void);
extern Boolean issilentsignal(List *e);
extern void setsigdefaults(void);
extern void blocksignals(void);
extern void unblocksignals(void);


/* open.c */

typedef enum { oOpen, oCreate, oAppend, oReadWrite, oReadCreate, oReadAppend } OpenKind;
extern int eopen(char *name, OpenKind k);


/* version.c */

extern const char * const version;


/* gc.c -- see gc.h for more */

typedef struct Tag Tag;
#define	gcnew(type)	((type *) gcalloc(sizeof (type), &(CONCAT(type,Tag))))

extern void *gcalloc(size_t n, Tag *t);		/* allocate n with collection tag t */
extern char *gcdup(const char *s);		/* copy a 0-terminated string into gc space */
extern char *gcndup(const char *s, size_t n);	/* copy a counted string into gc space */

extern void initgc(void);			/* must be called at the dawn of time */
extern void gc(void);				/* provoke a collection, if enabled */
extern void gcreserve(size_t nbytes);		/* provoke a collection, if enabled and not enough space */
extern void gcenable(void);			/* enable collections */
extern void gcdisable(void);			/* disable collections */
extern Boolean gcisblocked();			/* is collection disabled? */


/*
 * garbage collector tags
 */

typedef struct Root Root;
struct Root {
	void **p;
	Root *next;
};

extern Root *rootlist;

#if REF_ASSERTIONS
#define	refassert(e)	assert(e)
#else
#define	refassert(e)	NOP
#endif

#define	Ref(t, v, init) \
	if (0) ; else { \
		t v = init; \
		Root (CONCAT(v,__root__)); \
		(CONCAT(v,__root__)).p = (void **) &v; \
		(CONCAT(v,__root__)).next = rootlist; \
		rootlist = &(CONCAT(v,__root__))
#define	RefPop(v) \
		refassert(rootlist == &(CONCAT(v,__root__))); \
		refassert(rootlist->p == (void **) &v); \
		rootlist = rootlist->next;
#define RefEnd(v) \
		RefPop(v); \
	}
#define RefReturn(v) \
		RefPop(v); \
		return v; \
	}
#define	RefAdd(e) \
	if (0) ; else { \
		Root __root__; \
		__root__.p = (void **) &e; \
		__root__.next = rootlist; \
		rootlist = &__root__
#define	RefRemove(e) \
		refassert(rootlist == &__root__); \
		refassert(rootlist->p == (void **) &e); \
		rootlist = rootlist->next; \
	}

#define	RefEnd2(v1, v2)		RefEnd(v1); RefEnd(v2)
#define	RefEnd3(v1, v2, v3)	RefEnd(v1); RefEnd2(v2, v3)
#define	RefEnd4(v1, v2, v3, v4)	RefEnd(v1); RefEnd3(v2, v3, v4)

#define	RefPop2(v1, v2)		RefPop(v1); RefPop(v2)
#define	RefPop3(v1, v2, v3)	RefPop(v1); RefPop2(v2, v3)
#define	RefPop4(v1, v2, v3, v4)	RefPop(v1); RefPop3(v2, v3, v4)

#define	RefAdd2(v1, v2)		RefAdd(v1); RefAdd(v2)
#define	RefAdd3(v1, v2, v3)	RefAdd(v1); RefAdd2(v2, v3)
#define	RefAdd4(v1, v2, v3, v4)	RefAdd(v1); RefAdd3(v2, v3, v4)

#define	RefRemove2(v1, v2)		RefRemove(v1); RefRemove(v2)
#define	RefRemove3(v1, v2, v3)		RefRemove(v1); RefRemove2(v2, v3)
#define	RefRemove4(v1, v2, v3, v4)	RefRemove(v1); RefRemove3(v2, v3, v4)

extern void globalroot(void *addr);

/* struct Push -- varpush() placeholder */

struct Push {
	Push *next;
	char *name;
	List *defn;
	int flags;
	Root nameroot, defnroot;
};


/*
 * exception handling
 *
 *	ExceptionHandler
 *		... body ...
 *	CatchException (e)
 *		... catching code using e ...
 *	EndExceptionHandler
 *
 */

typedef struct Handler Handler;
struct Handler {
	Handler *up;
	Root *rootlist;
	Push *pushlist;
	unsigned long evaldepth;
	jmp_buf label;
};

extern Handler *tophandler, *roothandler;
extern List *exception;
extern void pophandler(Handler *handler);
extern noreturn throw(List *exc);
extern noreturn fail(const char *from, const char *name VARARGS);
extern void newchildcatcher(void);

#if DEBUG_EXCEPTIONS
extern List *raised(List *e);
#else
#define	raised(e)	(e)
#endif

#define ExceptionHandler \
	{ \
		Handler _localhandler; \
		_localhandler.rootlist = rootlist; \
		_localhandler.pushlist = pushlist; \
		_localhandler.evaldepth = evaldepth; \
		_localhandler.up = tophandler; \
		tophandler = &_localhandler; \
		if (!setjmp(_localhandler.label)) {
	
#define CatchException(e) \
			pophandler(&_localhandler); \
		} else { \
			List *e = raised(exception); \

#define CatchExceptionIf(condition, e) \
			if (condition) \
				pophandler(&_localhandler); \
		} else { \
			List *e = raised(exception); \

#define EndExceptionHandler \
		} \
	}
