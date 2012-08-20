/* str.c -- es string operations ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"
#include "print.h"

/* grow -- buffer grow function for str() */
static void str_grow(Format *f, size_t more) {
	Buffer *buf = expandbuffer(f->u.p, more);
	f->u.p		= buf;
	f->buf		= buf->str + (f->buf - f->bufbegin);
	f->bufbegin	= buf->str;
	f->bufend	= buf->str + buf->len;
}

/* strv -- print a formatted string into gc space */
extern char *strv(const char *fmt, va_list args) {
	Buffer *buf;
	Format format;

	gcdisable();
	buf = openbuffer(0);
	format.u.p	= buf;
#if NO_VA_LIST_ASSIGN
	memcpy(format.args, args, sizeof(va_list));
#else
	format.args	= args;
#endif
	format.buf	= buf->str;
	format.bufbegin	= buf->str;
	format.bufend	= buf->str + buf->len;
	format.grow	= str_grow;
	format.flushed	= 0;

	printfmt(&format, fmt);
	fmtputc(&format, '\0');
	gcenable();

	return sealbuffer(format.u.p);
}

/* str -- create a string (in garbage collection space) by printing to it */
extern char *str VARARGS1(const char *, fmt) {
	char *s;
	va_list args;
	VA_START(args, fmt);
	s = strv(fmt, args);
	va_end(args);
	return s;
}


#define	PRINT_ALLOCSIZE	64

/* mprint_grow -- buffer grow function for mprint() */
static void mprint_grow(Format *format, size_t more) {
	char *buf;
	size_t len = format->bufend - format->bufbegin + 1;
	len = (len >= more)
		? len * 2
		: ((len + more) + PRINT_ALLOCSIZE) &~ (PRINT_ALLOCSIZE - 1);
	buf = erealloc(format->bufbegin, len);
	format->buf	 = buf + (format->buf - format->bufbegin);
	format->bufbegin = buf;
	format->bufend	 = buf + len - 1;
}

/* mprint -- create a string in ealloc space by printing to it */
extern char *mprint VARARGS1(const char *, fmt) {
	Format format;
	format.u.n = 1;
	VA_START(format.args, fmt);

	format.buf	= ealloc(PRINT_ALLOCSIZE);
	format.bufbegin	= format.buf;
	format.bufend	= format.buf + PRINT_ALLOCSIZE - 1;
	format.grow	= mprint_grow;
	format.flushed	= 0;

	printfmt(&format, fmt);
	*format.buf = '\0';
	va_end(format.args);
	return format.bufbegin;
}


/*
 * StrList -- lists of strings
 *	to even include these is probably a premature optimization
 */

DefineTag(StrList, static);

extern StrList *mkstrlist(char *str, StrList *next) {
	gcdisable();
	assert(str != NULL);
	Ref(StrList *, list, gcnew(StrList));
	list->str = str;
	list->next = next;
	gcenable();
	RefReturn(list);
}

static void *StrListCopy(void *op) {
	void *np = gcnew(StrList);
	memcpy(np, op, sizeof (StrList));
	return np;
}

static size_t StrListScan(void *p) {
	StrList *list = p;
	list->str = forward(list->str);
	list->next = forward(list->next);
	return sizeof (StrList);
}

