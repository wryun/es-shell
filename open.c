/* open.c -- to insulate <fcntl.h> from the rest of es ($Revision: 1.1.1.1 $) */

#define	REQUIRE_FCNTL	1

#include "es.h"

#if NeXT
extern int open(const char *, int, ...);
#endif

#define	MIN_ttyfd	3


/*
 * Opens a file with the necessary flags.  Assumes the following order:
 *	typedef enum {
 *		oOpen, oCreate, oAppend, oReadCreate, oReadTrunc oReadAppend
 *	} OpenKind;
 */

static int mode_masks[] = {
	O_RDONLY,			/* rOpen */
	O_WRONLY | O_CREAT | O_TRUNC,	/* rCreate */
	O_WRONLY | O_CREAT | O_APPEND,	/* rAppend */
	O_RDWR   | O_CREAT,		/* oReadWrite */
	O_RDWR   | O_CREAT | O_TRUNC,	/* oReadCreate */
	O_RDWR   | O_CREAT | O_APPEND,	/* oReadAppend */
};

extern int eopen(char *name, OpenKind k) {
	assert((unsigned) k < arraysize(mode_masks));
	return open(name, mode_masks[k], 0666);
}

extern int opentty(void) {
	int e = 0, old, fd = 2;
	if (isatty(fd))
		return fcntl(fd, F_DUPFD, MIN_ttyfd);
	old = open("/dev/tty", O_RDWR|O_NONBLOCK);
	fd = fcntl(old, F_DUPFD, MIN_ttyfd);
	if (fd == -1)
		e = errno;
	close(old);
	if (e != 0)
		errno = e;
	assert(fd < 0 || fd >= MIN_ttyfd);
	return fd;
}
