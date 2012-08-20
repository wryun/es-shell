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
#if ASSERTIONS || GCVERBOSE
	long magic;
	char *typename;
#endif
};

extern Tag StringTag;

#if ASSERTIONS || GCVERBOSE
enum {TAGMAGIC = 0xDefaced};
#define	DefineTag(t)	Tag CONCAT(t, Tag) = { CONCAT(t, Copy), CONCAT(t, Scan), TAGMAGIC, STRING(t) }
#else
#define	DefineTag(t)	Tag CONCAT(t, Tag) = { CONCAT(t, Copy), CONCAT(t, Scan) }
#endif

/*
 * allocation
 */

extern void *gcalloc(size_t, Tag *);

typedef struct Buffer Buffer;
struct Buffer {
	size_t len;
	size_t current;
	char str[1];
};

extern Buffer *openbuffer(size_t minsize);
extern Buffer *expandbuffer(Buffer *buf, size_t minsize);
extern Buffer *bufncat(Buffer *buf, const char *s, size_t len);
extern Buffer *bufcat(Buffer *buf, const char *s);
extern Buffer *bufputc(Buffer *buf, char c);
extern char *sealbuffer(Buffer *buf);
extern char *sealcountedbuffer(Buffer *buf);
extern void freebuffer(Buffer *buf);

extern void *forward(void *p);
