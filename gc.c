/* gc.c -- copying garbage collector for es */

#define	GARBAGE_COLLECTOR	1	/* for es.h */

#include "es.h"
#include "gc.h"

#define	ALIGN(n)	(((n) + sizeof (void *) - 1) &~ (sizeof (void *) - 1))

typedef struct Space Space;
struct Space {
	char *current, *bot, *top;
	Space *prev;
};

#define	SPACESIZE(sp)	(((sp)->top - (sp)->bot))
#define	SPACEFREE(sp)	(((sp)->top - (sp)->current))
#define	SPACEUSED(sp)	(((sp)->current - (sp)->bot))
#define	INSPACE(p, sp)	((sp)->bot <= (char *) (p) && (char *) (p) < (sp)->top)

#if GCDEBUG
#define	NSPACES		10
#endif

/* globals */
Root *rootlist;
int gcblocked = 0;
Tag StringTag;

/* own variables */
static Space *new, *old;
#if GCDEBUG
static Space *spaces;
#endif
static Root *globalrootlist;
static size_t minspace = 10000;		/* minimum number of bytes in a new space */
static Buffer *buffer;


/*
 * debugging
 */

#if GCDEBUG
#define	DEBUG(p)	if (!gcdebug) ; else eprint p ;
#else
#define	DEBUG(p)	do {} while (0)
#endif


#if GCDEBUG

/*
 * GCDEBUG
 *	to use the GCDEBUG option, you must provide the following functions
 *		take
 *		release
 *		invalidate
 *		revalidate
 *		initmmu
 *	for your operating system
 */

#if __MACH__

/* mach specific version, for mmu assertion checking */

#include <mach.h>
#include <mach_error.h>

#define	PAGEROUND(n)	((n) + vm_page_size - 1) &~ (vm_page_size - 1)

/* take -- allocate memory for a space */
static void *take(size_t n) {
	vm_address_t addr;
	kern_return_t error = vm_allocate(task_self(), &addr, n, TRUE);
	if (error != KERN_SUCCESS) {
		mach_error("vm_allocate", error);
		exit(1);
	}
	memset((void *) addr, 0xC9, n);
	return (void *) addr;
}

/* release -- deallocate a range of memory */
static void release(void *p, size_t n) {
	kern_return_t error = vm_deallocate(task_self(), (vm_address_t) p, n);
	if (error != KERN_SUCCESS) {
		mach_error("vm_deallocate", error);
		exit(1);
	}
}

/* invalidate -- disable access to a range of memory */
static void invalidate(void *p, size_t n) {
	kern_return_t error = vm_protect(task_self(), (vm_address_t) p, n, FALSE, 0);
	if (error != KERN_SUCCESS) {
		mach_error("vm_protect 0", error);
		exit(1);
	}
}

/* revalidate -- enable access to a range of memory */
static void revalidate(void *p, size_t n) {
	kern_return_t error =
		vm_protect(task_self(), (vm_address_t) p, n, FALSE, VM_PROT_READ|VM_PROT_WRITE);
	if (error != KERN_SUCCESS) {
		mach_error("vm_protect VM_PROT_READ|VM_PROT_WRITE", error);
		exit(1);
	}
	memset(p, 0x4F, n);
}

/* initmmu -- initialization for memory management calls */
static void initmmu(void) {
}

#elif sun			/* TODO: sun mmu tweaking */
#include <sys/mman.h>

extern void mprotect(void *p, size_t nbytes, int flags);

static int pagesize;
#define	PAGEROUND(n)	((n) + pagesize - 1) &~ (pagesize - 1)

/* take -- allocate memory for a space */
static void *take(size_t n) {
	/* TODO: sun take */
}

/* revalidate -- enable access to a range of memory */
static void release(void *p, size_t n) {
	/* TODO: sun release */
}

/* invalidate -- disable access to a range of memory */
static void invalidate(void *p, size_t n) {
	mprotect(p, n, PROT_NONE);
}

/* revalidate -- enable access to a range of memory */
static void revalidate(void *p, size_t n) {
	mprotect(p, n, PROT_READ|PROT_WRITE);
}

/* initmmu -- initialization for memory management calls */
static void initmmu(void) {
	pagesize = getpagesize();
}

#endif	/* sun */
#endif	/* GCDEBUG */


/*
 * ``half'' space management
 */

#if GCDEBUG

/* mkspace -- create a new ``half'' space in debugging mode */
static Space *mkspace(Space *space, Space *prev) {
	assert(space == NULL || (&spaces[0] <= space && space < &spaces[NSPACES]));

	if (space != NULL) {
		Space *sp;
		if (space->bot == NULL)
			sp = NULL;
		else if (SPACESIZE(space) < minspace)
			sp = space;
		else {
			sp = space->prev;
			revalidate(space->bot, SPACESIZE(space));
		}
		while (sp != NULL) {
			Space *tail = sp->prev;
			release(sp->bot, SPACESIZE(sp));
			if (&spaces[0] <= space && space < &spaces[NSPACES])
				sp->bot = NULL;
			else
				efree(sp);
			sp = tail;
		}
	}

	if (space == NULL) {
		space = ealloc(sizeof (Space));
		memzero(space, sizeof (Space));
	}
	if (space->bot == NULL) {
		size_t n = PAGEROUND(minspace);
		space->bot = take(n);
		space->top = space->bot + n / (sizeof (*space->bot));
	}

	space->prev = prev;
	space->current = space->bot;

	return space;
}
#define	newspace(prev)		mkspace(NULL, prev)

#else	/* !GCDEBUG */

/* newspace -- create a new ``half'' space */
static Space *newspace(Space *prev) {
	size_t n = ALIGN(minspace);
	Space *space = ealloc(sizeof (Space) + n);
	space->bot = (void *) &space[1];
	space->top = (void *) (((char *) space->bot) + n);
	space->current = space->bot;
	space->prev = prev;
	return space;
}

#endif	/* !GCDEBUG */

/* deprecate -- take a space and invalidate it */
static void deprecate(Space *space) {
#if GCDEBUG
	Space *base;
	assert(space != NULL);
	for (base = space; base->prev != NULL; base = base->prev)
		;
	assert(&spaces[0] <= base && base < &spaces[NSPACES]);
	for (;;) {
		invalidate(space->bot, SPACESIZE(space));
		if (space == base)
			break;
		else {
			Space *prev = space->prev;
			space->prev = base->prev;
			base->prev = space;
			space = prev;
		}
	}
#else
	while (space != NULL) {
		Space *old = space;
		space = space->prev;
		efree(old);
	}

#endif
}

/* isinspace -- does an object lie inside a given Space? */
extern Boolean isinspace(Space *space, void *p) {
	for (; space != NULL; space = space->prev)
		if (INSPACE(p, space)) {
		 	assert((char *) p < space->current);
		 	return TRUE;
		}
	return FALSE;
}


/*
 * root list building and scanning
 */

/* globalroot -- add an external to the list of global roots */
extern void globalroot(void *addr) {
	Root *root;
#if ASSERTIONS
	for (root = globalrootlist; root != NULL; root = root->next)
		assert(root->p != addr);
#endif
	root = ealloc(sizeof (Root));
	root->p = addr;
	root->next = globalrootlist;
	globalrootlist = root;
}

/* not portable to word addressed machines */
#define	TAG(p)		(((Tag **) p)[-1])
#define	FORWARDED(tagp)	(((int) tagp) & 1)
#define	FOLLOWTO(p)	((Tag *) (((char *) p) + 1))
#define	FOLLOW(tagp)	((void *) (((char *) tagp) - 1))

/* forward -- forward an individual pointer from old space */
extern void *forward(void *p) {
	Tag *tag;
	void *np;

	if (!isinspace(old, p))
		return p;

	DEBUG(("GC %8ux : ", p));

	tag = TAG(p);
	assert(tag != NULL);
	if (FORWARDED(tag)) {
		np = FOLLOW(tag);
		assert(TAG(np)->magic == TAGMAGIC);
		DEBUG(("%s	-> %8ux (followed)\n", TAG(np)->typename, np));
	} else {
		assert(tag->magic == TAGMAGIC);
		np = (*tag->copy)(p);
		DEBUG(("%s	-> %8ux (forwarded)\n", tag->typename, np));
		TAG(p) = FOLLOWTO(np);
	}
	return np;
}

/* scanroots -- scan a rootlist */
static void scanroots(Root *rootlist) {
	Root *root;
	for (root = rootlist; root != NULL; root = root->next)
		*root->p = forward(*root->p);
}

/* scanspace -- scan new space until it is up to date */
static void scanspace(void) {
	Space *sp, *nsp;
	for (sp = new;; sp = nsp) {
		char *scan = sp->bot;
		while (scan < sp->current) {
			Tag *tag = *(Tag **) scan;
			assert(tag->magic == TAGMAGIC);
			scan += sizeof (Tag *);
			DEBUG(("GC %8ux : %s	scan\n", scan, tag->typename));
			scan += ALIGN((*tag->scan)(scan));
		}
		if (sp == new)
			break;
		for (nsp = new; nsp->prev != sp; nsp = nsp->prev)
			assert(nsp->prev != NULL);
	}
}


/*
 * the garbage collector public interface
 */

/* gcenable -- enable collections */
extern void gcenable(void) {
	assert(gcblocked > 0);
	--gcblocked;
	if (!gcblocked && new->prev != NULL)
		gc();
}

/* gcdisable -- disable collections, collect first if space needed */
extern void gcdisable(size_t minfree) {
	assert(gcblocked >= 0);
	if (!gcblocked && SPACEFREE(new) < minfree) {
		if (minspace < minfree)
			minspace = minfree;
		gc();
	}
	++gcblocked;
}

/* gc -- actually do a garbage collection */
extern void gc(void) {
	do {
		size_t livedata;
		Space *space;

		assert(gcblocked >= 0);
		if (gcblocked > 0)
			return;
		++gcblocked;

		assert(new != NULL);
		assert(old == NULL);
		old = new;
#if GCDEBUG
		for (; new->prev != NULL; new = new->prev)
			;
		if (++new >= &spaces[NSPACES])
			new = &spaces[0];
		new = mkspace(new, NULL);
#else
		new = newspace(NULL);
#endif
		DEBUG(("\nGC collection starting\n"));
		DEBUG(("GC old space = %ux ... %ux\n", old->bot, old->current));
		DEBUG(("GC new space = %ux ... %ux\n", new->bot, new->current));
		DEBUG(("GC scanning root list\n"));
		scanroots(rootlist);
		DEBUG(("GC scanning global root list\n"));
		scanroots(globalrootlist);
		DEBUG(("GC scanning new space\n"));
		scanspace();
		DEBUG(("GC collection done\n\n"));

		deprecate(old);
		old = NULL;

		for (livedata = 0, space = new; space != NULL; space = space->prev)
			livedata += SPACEUSED(space);
		if (minspace < livedata * 5)
			minspace = livedata * 4;
		else if (minspace > livedata * 16)
			minspace /= 2;

		--gcblocked;
	} while (new->prev != NULL);
}

/* initgc -- initialize the garbage collector */
extern void initgc(void) {
#if GCDEBUG
	initmmu();
	spaces = ealloc(NSPACES * sizeof (Space));
	memzero(spaces, NSPACES * sizeof (Space));
	new = mkspace(&spaces[0], NULL);
#else
	new = newspace(NULL);
#endif
	old = NULL;
}


/*
 * allocation
 */

/* gcalloc -- allocate an object in new space */
extern void *gcalloc(size_t nbytes, Tag *tag) {
	size_t n = ALIGN(nbytes + sizeof (Tag *));
#if GCDEBUG || GCALWAYS
	gc();
#endif
	assert(tag == NULL || tag->magic == TAGMAGIC);
	for (;;) {
		Tag **p = (void *) new->current;
		char *q = ((char *) p) + n;
		if (q <= new->top) {
			new->current = q;
			*p++ = tag;
			return p;
		}
		if (minspace < nbytes)
			minspace = nbytes + sizeof (Tag *);
		if (gcblocked)
			new = newspace(new);
		else
			gc();
	}
}


/*
 * strings
 */

extern char *gcndup(const char *s, size_t n) {
	char *ns;

	gcdisable(0);
	ns = gcalloc((n + 1) * sizeof (char), &StringTag);
	memcpy(ns, s, n);
	ns[n] = '\0';
	assert(strlen(ns) == n);

	Ref(char *, result, ns);
	gcenable();
	RefReturn(result);
}

extern char *gcdup(const char *s) {
	return gcndup(s, strlen(s));
}

static void *StringCopy(void *op) {
	size_t n = strlen(op) + 1;
	char *np = gcalloc(n, &StringTag);
	memcpy(np, op, n);
	return np;
}

static size_t StringScan(void *p) {
	return strlen(p) + 1;
}

DefineTag(String);


/*
 * allocation of large, contiguous buffers for large object creation
 *	see the use of this in str().  note that this region may
 *	not contain pointers until after sealbuffer() has been called.
 */

extern Buffer openbuffer(size_t minsize) {
	Buffer *b;
	if (minsize < 512)
		minsize = 512;
	b = ealloc(sizeof (Buffer) + minsize);
	b->str = (void *) &b[1];
	b->len = minsize;
	b->prev = buffer;
	buffer = b;
	return *buffer;
}

extern Buffer expandbuffer(size_t minsize) {
	buffer->len += (minsize < 1024) ? 1024 : minsize;
	buffer = erealloc(buffer, sizeof (Buffer) + buffer->len);
	return *buffer;
}

extern char *sealbuffer(char *p) {
	Buffer *b = buffer;
	char *result = gcdup(b->str);
	buffer = b->prev;
	efree(b);
	return result;
}


#if GCDEBUG
/*
 * memdump -- print out all of gc space, as best as possible
 */

static char *tree1name(NodeKind k) {
	switch(k) {
	default:	panic("tree1name: bad node kind %d", k);
	case nPrim:	return "Prim";
	case nQword:	return "Qword";
	case nCall:	return "Call";
	case nThunk:	return "Thunk";
	case nVar:	return "Var";
	case nWord:	return "Word";
	}
}

static char *tree2name(NodeKind k) {
	switch(k) {
	default:	panic("tree2name: bad node kind %d", k);
	case nAssign:	return "Assign";
	case nConcat:	return "Concat";
	case nFor:	return "For";
	case nLambda:	return "Lambda";
	case nLet:	return "Let";
	case nList:	return "List";
	case nLocal:	return "Local";
	case nMatch:	return "Match";
	case nRec:	return "Rec";
	case nVarsub:	return "Varsub";
	}
}

static size_t dump(Tag *t, void *p) {
	char *s = t->typename;
	debug("%8ux %s\t", p, s);

	if (streq(s, "String")) {
		debug("%s\n", p);
		return strlen(p) + 1;
	}

	if (streq(s, "Term")) {
		Term *t = p;
		debug("str = %ux  closure = %ux\n", t->str, t->closure);
		return sizeof (Term);
	}

	if (streq(s, "List")) {
		List *l = p;
		debug("term = %ux  next = %ux\n", l->term, l->next);
		return sizeof (List);
	}

	if (streq(s, "StrList")) {
		StrList *l = p;
		debug("str = %ux  next = %ux\n", l->str, l->next);
		return sizeof (StrList);
	}

	if (streq(s, "Closure")) {
		Closure *c = p;
		debug("tree = %ux  binding = %ux\n", c->tree, c->binding);
		return sizeof (Closure);
	}

	if (streq(s, "Tree1")) {
		Tree *t = p;
		debug("%s	%ux\n", tree1name(t->kind), t->u[0].p);
		return offsetof(Tree, u[1]);
	}

	if (streq(s, "Tree2")) {
		Tree *t = p;
		debug("%s	%ux  %ux\n", tree2name(t->kind), t->u[0].p, t->u[1].p);
		return offsetof(Tree, u[2]);
	}

	debug("<<unknown>>\n");
	return 0;
}

extern void memdump(void) {
	Space *sp, *nsp;
	for (sp = new;; sp = nsp) {
		char *scan = sp->bot;
		while (scan < sp->current) {
			Tag *tag = *(Tag **) scan;
			assert(tag->magic == TAGMAGIC);
			scan += sizeof (Tag *);
			scan += ALIGN(dump(tag, scan));
		}
		if (sp == new)
			break;
		for (nsp = new; nsp->prev != sp; nsp = nsp->prev)
			assert(nsp->prev != NULL);
	}
}
#endif
