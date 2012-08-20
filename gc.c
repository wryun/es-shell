/* gc.c -- copying garbage collector for es ($Revision: 1.7 $) */

#define	GARBAGE_COLLECTOR	1	/* for es.h */

#include "es.h"
#include "gc.h"

#define	ALIGN(n)	(((n) + sizeof (void *) - 1) &~ (sizeof (void *) - 1))

typedef struct Space Space;
struct Space {
	char *current, *bot, *top;
	Space *next;
};

#define	SPACESIZE(sp)	(((sp)->top - (sp)->bot))
#define	SPACEFREE(sp)	(((sp)->top - (sp)->current))
#define	SPACEUSED(sp)	(((sp)->current - (sp)->bot))
#define	INSPACE(p, sp)	((sp)->bot <= (char *) (p) && (char *) (p) < (sp)->top)

#if GCPROTECT
#define	NSPACES		10
#endif

/* globals */
Root *rootlist;
int gcblocked = 0;
Tag StringTag;

/* own variables */
static Space *new, *old;
#if GCPROTECT
static Space *spaces;
#endif
static Root *globalrootlist;
static size_t minspace = 10000;		/* minimum number of bytes in a new space */


/*
 * debugging
 */

#if GCVERBOSE
#define	VERBOSE(p)	STMT(if (gcverbose) eprint p;)
#else
#define	VERBOSE(p)	STMT(;)
#endif


#if GCPROTECT

/*
 * GCPROTECT
 *	to use the GCPROTECT option, you must provide the following functions
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
#endif	/* GCPROTECT */


/*
 * ``half'' space management
 */

#if GCPROTECT

/* mkspace -- create a new ``half'' space in debugging mode */
static Space *mkspace(Space *space, Space *next) {
	assert(space == NULL || (&spaces[0] <= space && space < &spaces[NSPACES]));

	if (space != NULL) {
		Space *sp;
		if (space->bot == NULL)
			sp = NULL;
		else if (SPACESIZE(space) < minspace)
			sp = space;
		else {
			sp = space->next;
			revalidate(space->bot, SPACESIZE(space));
		}
		while (sp != NULL) {
			Space *tail = sp->next;
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

	space->next = next;
	space->current = space->bot;

	return space;
}
#define	newspace(next)		mkspace(NULL, next)

#else	/* !GCPROTECT */

/* newspace -- create a new ``half'' space */
static Space *newspace(Space *next) {
	size_t n = ALIGN(minspace);
	Space *space = ealloc(sizeof (Space) + n);
	space->bot = (void *) &space[1];
	space->top = (void *) (((char *) space->bot) + n);
	space->current = space->bot;
	space->next = next;
	return space;
}

#endif	/* !GCPROTECT */

/* deprecate -- take a space and invalidate it */
static void deprecate(Space *space) {
#if GCPROTECT
	Space *base;
	assert(space != NULL);
	for (base = space; base->next != NULL; base = base->next)
		;
	assert(&spaces[0] <= base && base < &spaces[NSPACES]);
	for (;;) {
		invalidate(space->bot, SPACESIZE(space));
		if (space == base)
			break;
		else {
			Space *next = space->next;
			space->next = base->next;
			base->next = space;
			space = next;
		}
	}
#else
	while (space != NULL) {
		Space *old = space;
		space = space->next;
		efree(old);
	}

#endif
}

/* isinspace -- does an object lie inside a given Space? */
extern Boolean isinspace(Space *space, void *p) {
	for (; space != NULL; space = space->next)
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

	if (!isinspace(old, p)) {
		VERBOSE(("GC %8ux : <<not in old space>>\n", p));
		return p;
	}

	VERBOSE(("GC %8ux : ", p));

	tag = TAG(p);
	assert(tag != NULL);
	if (FORWARDED(tag)) {
		np = FOLLOW(tag);
		assert(TAG(np)->magic == TAGMAGIC);
		VERBOSE(("%s	-> %8ux (followed)\n", TAG(np)->typename, np));
	} else {
		assert(tag->magic == TAGMAGIC);
		np = (*tag->copy)(p);
		VERBOSE(("%s	-> %8ux (forwarded)\n", tag->typename, np));
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
	Space *sp, *scanned;
	for (scanned = NULL;;) {
		Space *front = new;
		for (sp = new; sp != scanned; sp = sp->next) {
			char *scan;
			assert(sp != NULL);
			scan = sp->bot;
			while (scan < sp->current) {
				Tag *tag = *(Tag **) scan;
				assert(tag->magic == TAGMAGIC);
				scan += sizeof (Tag *);
				VERBOSE(("GC %8ux : %s	scan\n", scan, tag->typename));
				scan += ALIGN((*tag->scan)(scan));
			}
		}
		if (new == front)
			break;
		scanned = front;
	}
}


/*
 * the garbage collector public interface
 */

/* gcenable -- enable collections */
extern void gcenable(void) {
	assert(gcblocked > 0);
	--gcblocked;
	if (!gcblocked && new->next != NULL)
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

/* gcisblocked -- is collection disabled? */
extern Boolean gcisblocked(void) {
	assert(gcblocked >= 0);
	return gcblocked != 0;
}

/* gc -- actually do a garbage collection */
extern void gc(void) {
	do {
		size_t livedata;
		Space *space;

#if GCINFO
		size_t olddata = 0;
		if (gcinfo)
			for (space = new; space != NULL; space = space->next)
				olddata += SPACEUSED(space);
#endif

		assert(gcblocked >= 0);
		if (gcblocked > 0)
			return;
		++gcblocked;

		assert(new != NULL);
		assert(old == NULL);
		old = new;
#if GCPROTECT
		for (; new->next != NULL; new = new->next)
			;
		if (++new >= &spaces[NSPACES])
			new = &spaces[0];
		new = mkspace(new, NULL);
#else
		new = newspace(NULL);
#endif
		VERBOSE(("\nGC collection starting\n"));
#if GCVERBOSE
		for (space = old; space != NULL; space = space->next)
			VERBOSE(("GC old space = %ux ... %ux\n", space->bot, space->current));
#endif
		VERBOSE(("GC new space = %ux ... %ux\n", new->bot, new->top));
		VERBOSE(("GC scanning root list\n"));
		scanroots(rootlist);
		VERBOSE(("GC scanning global root list\n"));
		scanroots(globalrootlist);
		VERBOSE(("GC scanning new space\n"));
		scanspace();
		VERBOSE(("GC collection done\n\n"));

		deprecate(old);
		old = NULL;

		for (livedata = 0, space = new; space != NULL; space = space->next)
			livedata += SPACEUSED(space);

#if GCINFO
		if (gcinfo)
			eprint(
				"[GC: old %8d  live %8d  min %8d  (pid %5d)]\n",
				olddata, livedata, minspace, getpid()
			);
#endif

		if (minspace < livedata * 2)
			minspace = livedata * 4;
		else if (minspace > livedata * 12)
			minspace /= 2;

		--gcblocked;
	} while (new->next != NULL);
}

/* initgc -- initialize the garbage collector */
extern void initgc(void) {
#if GCPROTECT
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
#if GCALWAYS
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

#define notstatic
DefineTag(String, notstatic);

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


/*
 * allocation of large, contiguous buffers for large object creation
 *	see the use of this in str().  note that this region may not
 *	contain pointers or '\0' until after sealbuffer() has been called.
 */

extern Buffer *openbuffer(size_t minsize) {
	Buffer *buf;
	if (minsize < 500)
		minsize = 500;
	buf = ealloc(offsetof(Buffer, str[minsize]));
	buf->len = minsize;
	buf->current = 0;
	return buf;
}

extern Buffer *expandbuffer(Buffer *buf, size_t minsize) {
	buf->len += (minsize > buf->len) ? minsize : buf->len;
	buf = erealloc(buf, offsetof(Buffer, str[buf->len]));
	return buf;
}

extern char *sealbuffer(Buffer *buf) {
	char *s = gcdup(buf->str);
	efree(buf);
	return s;
}

extern char *sealcountedbuffer(Buffer *buf) {
	char *s = gcndup(buf->str, buf->current);
	efree(buf);
	return s;
}

extern Buffer *bufncat(Buffer *buf, const char *s, size_t len) {
	while (buf->current + len >= buf->len)
		buf = expandbuffer(buf, buf->current + len - buf->len);
	memcpy(buf->str + buf->current, s, len);
	buf->current += len;
	return buf;
}

extern Buffer *bufcat(Buffer *buf, const char *s) {
	return bufncat(buf, s, strlen(s));
}

extern Buffer *bufputc(Buffer *buf, char c) {
	return bufncat(buf, &c, 1);
}

extern void freebuffer(Buffer *buf) {
	efree(buf);
}


#if GCVERBOSE
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
	case nClosure:	return "Closure";
	case nFor:	return "For";
	case nLambda:	return "Lambda";
	case nLet:	return "Let";
	case nList:	return "List";
	case nLocal:	return "Local";
	case nMatch:	return "Match";
	case nVarsub:	return "Varsub";
	}
}

/* having these here violates every data hiding rule in the book */

	typedef struct {
		char *name;
		void *value;
	} Assoc;
	struct Dict {
		int size, remain;
		Assoc table[1];		/* variable length */
	};

#include "var.h"


static size_t dump(Tag *t, void *p) {
	char *s = t->typename;
	print("%8ux %s\t", p, s);

	if (streq(s, "String")) {
		print("%s\n", p);
		return strlen(p) + 1;
	}

	if (streq(s, "Term")) {
		Term *t = p;
		print("str = %ux  closure = %ux\n", t->str, t->closure);
		return sizeof (Term);
	}

	if (streq(s, "List")) {
		List *l = p;
		print("term = %ux  next = %ux\n", l->term, l->next);
		return sizeof (List);
	}

	if (streq(s, "StrList")) {
		StrList *l = p;
		print("str = %ux  next = %ux\n", l->str, l->next);
		return sizeof (StrList);
	}

	if (streq(s, "Closure")) {
		Closure *c = p;
		print("tree = %ux  binding = %ux\n", c->tree, c->binding);
		return sizeof (Closure);
	}

	if (streq(s, "Binding")) {
		Binding *b = p;
		print("name = %ux  defn = %ux  next = %ux\n", b->name, b->defn, b->next);
		return sizeof (Binding);
	}

	if (streq(s, "Var")) {
		Var *v = p;
		print("defn = %ux  env = %ux  flags = %d\n",
		      v->defn, v->env, v->flags);
		return sizeof (Var);
	}

	if (streq(s, "Tree1")) {
		Tree *t = p;
		print("%s	%ux\n", tree1name(t->kind), t->u[0].p);
		return offsetof(Tree, u[1]);
	}

	if (streq(s, "Tree2")) {
		Tree *t = p;
		print("%s	%ux  %ux\n", tree2name(t->kind), t->u[0].p, t->u[1].p);
		return offsetof(Tree, u[2]);
	}

	if (streq(s, "Vector")) {
		Vector *v = p;
		int i;
		print("alloclen = %d  count = %d [", v->alloclen, v->count);
		for (i = 0; i <= v->alloclen; i++)
			print("%s%ux", i == 0 ? "" : " ", v->vector[i]);
		print("]\n");
		return offsetof(Vector, vector[v->alloclen + 1]);
	}

	if (streq(s, "Dict")) {
		Dict *d = p;
		int i;
		print("size = %d  remain = %d\n", d->size, d->remain);
		for (i = 0; i < d->size; i++)
			print("\tname = %ux  value = %ux\n",
			      d->table[i].name, d->table[i].value);
		return offsetof(Dict, table[d->size]);
	}

	print("<<unknown>>\n");
	return 0;
}

extern void memdump(void) {
	Space *sp;
	for (sp = new; sp != NULL; sp = sp->next) {
		char *scan = sp->bot;
		while (scan < sp->current) {
			Tag *tag = *(Tag **) scan;
			assert(tag->magic == TAGMAGIC);
			scan += sizeof (Tag *);
			scan += ALIGN(dump(tag, scan));
		}
	}
}
#endif
