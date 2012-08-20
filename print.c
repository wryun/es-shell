/* print.c -- formatted printing routines ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "print.h"

#define	MAXCONV 256

/*
 * conversion functions
 *	true return -> flag changes only, not a conversion
 */

#define	Flag(name, flag) \
static Boolean name(Format *format) { \
	format->flags |= flag; \
	return TRUE; \
}

Flag(uconv,	FMT_unsigned)
Flag(hconv,	FMT_short)
Flag(longconv,	FMT_long)
Flag(altconv,	FMT_altform)
Flag(leftconv,	FMT_leftside)
Flag(dotconv,	FMT_f2set)

static Boolean digitconv(Format *format) {
	int c = format->invoker;
	if (format->flags & FMT_f2set)
		format->f2 = 10 * format->f2 + c - '0';
	else {
		format->flags |= FMT_f1set;
		format->f1 = 10 * format->f1 + c - '0';
	}
	return TRUE;
}

static Boolean zeroconv(Format *format) {
	if (format->flags & (FMT_f1set | FMT_f2set))
		return digitconv(format);
	format->flags |= FMT_zeropad;
	return TRUE;
}

static void pad(Format *format, long len, int c) {
	while (len-- > 0)
		fmtputc(format, c);
}

static Boolean sconv(Format *format) {
	char *s = va_arg(format->args, char *);
	if ((format->flags & FMT_f1set) == 0)
		fmtcat(format, s);
	else {
		size_t len = strlen(s), width = format->f1 - len;
		if (format->flags & FMT_leftside) {
			fmtappend(format, s, len);
			pad(format, width, ' ');
		} else {
			pad(format, width, ' ');
			fmtappend(format, s, len);
		}
	}
	return FALSE;
}

static char *utoa(unsigned long u, char *t, unsigned int radix, char *digit) {
	if (u >= radix) {
		t = utoa(u / radix, t, radix, digit);
		u %= radix;
	}
	*t++ = digit[u];
	return t;
}

static void intconv(Format *format, unsigned int radix, int upper, char *altform) {
	static char * table[] = {
		"0123456789abcdefghijklmnopqrstuvwxyz",
		"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ",
	};
	char padchar;
	size_t len, pre, zeroes, padding, width;
	long n, flags;
	unsigned long u;
	char number[64], prefix[20];

	if (radix > 36)
		return;

	flags = format->flags;
	if (flags & FMT_long)
		n = va_arg(format->args, long);
	else if (flags & FMT_short)
		n = va_arg(format->args, short);
	else
		n = va_arg(format->args, int);

	pre = 0;
	if ((flags & FMT_unsigned) || n >= 0)
		u = n;
	else {
		prefix[pre++] = '-';
		u = -n;
	}

	if (flags & FMT_altform)
		while (*altform != '\0')
			prefix[pre++] = *altform++;

	len = utoa(u, number, radix, table[upper]) - number;
	if ((flags & FMT_f2set) && (size_t) format->f2 > len)
		zeroes = format->f2 - len;
	else
		zeroes = 0;

	width = pre + zeroes + len;
	if ((flags & FMT_f1set) && (size_t) format->f1 > width) {
		padding = format->f1 - width;
	} else
		padding = 0;

	padchar = ' ';
	if (padding > 0 && flags & FMT_zeropad) {
		padchar = '0';
		if ((flags & FMT_leftside) == 0) {
			zeroes += padding;
			padding = 0;
		}
	}

	if ((flags & FMT_leftside) == 0)
		pad(format, padding, padchar);
	fmtappend(format, prefix, pre);
	pad(format, zeroes, '0');
	fmtappend(format, number, len);
	if (flags & FMT_leftside)
		pad(format, padding, padchar);
}

static Boolean cconv(Format *format) {
	fmtputc(format, va_arg(format->args, int));
	return FALSE;
}

static Boolean dconv(Format *format) {
	intconv(format, 10, 0, "");
	return FALSE;
}

static Boolean oconv(Format *format) {
	intconv(format, 8, 0, "0");
	return FALSE;
}

static Boolean xconv(Format *format) {
	intconv(format, 16, 0, "0x");
	return FALSE;
}

static Boolean pctconv(Format *format) {
	fmtputc(format, '%');
	return FALSE;
}

static Boolean badconv(Format *format) {
	panic("bad conversion character in printfmt: %%%c", format->invoker);
	return FALSE; /* hush up gcc -Wall */
}


/*
 * conversion table management
 */

static Conv *fmttab;

static void inittab(void) {
	int i;

	fmttab = ealloc(MAXCONV * sizeof (Conv));
	for (i = 0; i < MAXCONV; i++)
		fmttab[i] = badconv;

	fmttab['s'] = sconv;
	fmttab['c'] = cconv;
	fmttab['d'] = dconv;
	fmttab['o'] = oconv;
	fmttab['x'] = xconv;
	fmttab['%'] = pctconv;

	fmttab['u'] = uconv;
	fmttab['h'] = hconv;
	fmttab['l'] = longconv;
	fmttab['#'] = altconv;
	fmttab['-'] = leftconv;
	fmttab['.'] = dotconv;

	fmttab['0'] = zeroconv;
	for (i = '1'; i <= '9'; i++)
		fmttab[i] = digitconv;
}

Conv fmtinstall(int c, Conv f) {
	Conv oldf;
	if (fmttab == NULL)
		inittab();
	c &= MAXCONV - 1;
	oldf = fmttab[c];
	if (f != NULL)
		fmttab[c] = f;
	return oldf;
}


/*
 * functions for inserting strings in the format buffer
 */

extern void fmtappend(Format *format, const char *s, size_t len) {
	while (format->buf + len > format->bufend) {
		size_t split = format->bufend - format->buf;
		memcpy(format->buf, s, split);
		format->buf += split;
		s += split;
		len -= split;
		(*format->grow)(format, len);
	}
	memcpy(format->buf, s, len);
	format->buf += len;
}

extern void fmtcat(Format *format, const char *s) {
	fmtappend(format, s, strlen(s));
}

/*
 * printfmt -- the driver routine
 */

extern int printfmt(Format *format, const char *fmt) {
	unsigned char *s = (unsigned char *) fmt;

	if (fmttab[0] == NULL)
		inittab();

	for (;;) {
		int c = *s++;
		switch (c) {
		case '%':
			format->flags = format->f1 = format->f2 = 0;
			do
				format->invoker = c = *s++;
			while ((*fmttab[c])(format));
			break;
		case '\0':
			return format->buf - format->bufbegin + format->flushed;
		default:
			fmtputc(format, c);
			break;
		}
	}
}


/*
 * the public entry points
 */

extern int fmtprint VARARGS2(Format *, format, const char *, fmt) {
	int n = -format->flushed;
#if NO_VA_LIST_ASSIGN
	va_list saveargs;

	memcpy(saveargs, format->args, sizeof(va_list));
#else
	va_list saveargs = format->args;
#endif


	VA_START(format->args, fmt);
	n += printfmt(format, fmt);
	va_end(format->args);
	format->args = saveargs;

	return n + format->flushed;
}

static void fprint_flush(Format *format, size_t more) {
	size_t n = format->buf - format->bufbegin;
	char *buf = format->bufbegin;

	format->flushed += n;
	format->buf = format->bufbegin;
	while (n != 0) {
		int written = write(format->u.n, buf, n);
		if (written == -1) {
			if (format->u.n != 2)
				uerror("write");
			exit(1);
		}
		n -= written;
	}
}

static void fdprint(Format *format, int fd, const char *fmt) {
	char buf[FPRINT_BUFSIZ];

	format->buf	= buf;
	format->bufbegin = buf;
	format->bufend	= buf + sizeof buf;
	format->grow	= fprint_flush;
	format->flushed	= 0;
	format->u.n	= fdmap(fd);

	gcdisable();
	printfmt(format, fmt);
	fprint_flush(format, 0);
	gcenable();
}

extern int fprint VARARGS2(int, fd, const char *, fmt) {
	Format format;
	VA_START(format.args, fmt);
	fdprint(&format, fd, fmt);
	va_end(format.args);
	return format.flushed;
}

extern int print VARARGS1(const char *, fmt) {
	Format format;
	VA_START(format.args, fmt);
	fdprint(&format, 1, fmt);
	va_end(format.args);
	return format.flushed;
}

extern int eprint VARARGS1(const char *, fmt) {
	Format format;
	VA_START(format.args, fmt);
	fdprint(&format, 2, fmt);
	va_end(format.args);
	return format.flushed;
}

extern noreturn panic VARARGS1(const char *, fmt) {
	Format format;
	gcdisable();
	VA_START(format.args, fmt);
	eprint("es panic: ");
	fdprint(&format, 2, fmt);
	va_end(format.args);
	eprint("\n");
	exit(1);
}
