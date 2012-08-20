/* open.c -- to insulate <fcntl.h> from the rest of es ($Revision: 1.1.1.1 $) */

#define	REQUIRE_FCNTL	1

#include "es.h"

#if NeXT
extern int open(const char *, int, ...);
#endif

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
