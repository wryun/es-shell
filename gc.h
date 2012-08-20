/* gc.h -- garbage collector interface for es */


/*
 * garbage collector controls
 */

extern void initgc(void);		/* must be called at the dawn of time */
extern void gc(void);			/* provoke a collection, if enabled */
extern void gcenable(void);		/* enable collections */
extern void gcdisable(size_t);		/* disable collections, collect first if space needed */

#ifndef GARBAGE_COLLECTOR
extern const int gcblocked;
#endif


/*
 * tags
 */

struct Tag {
	void *(*copy)(void *);
	size_t (*scan)(void *);
	long magic;
	char *typename;
};

enum {TAGMAGIC = 0xDefaced};

extern Tag StringTag;

#define	DefineTag(t)	Tag t ## Tag = { t ## Copy, t ## Scan, TAGMAGIC, #t }

/*
 * allocation
 */

extern void *gcalloc(size_t, Tag *);

typedef struct Buffer Buffer;
struct Buffer {
	char *str;
	size_t len;
	Buffer *prev;
};

extern Buffer openbuffer(size_t minsize);
extern Buffer expandbuffer(size_t minsize);
extern char *sealbuffer(char *endp);

extern void *forward(void *p);
