/* open.c -- to insulate <fcntl.h> from the rest of rc */

#include <fcntl.h>
#include "es.h"
#include "token.h"

/* prototype for open() follows. comment out if necessary */
#if __MACH__
extern int open(char *, int, ...);
#endif

/*
 * Opens a file with the necessary flags.  Assumes the following order:
 *	typedef enum {
 *		rOpen, rCreate, rAppend, rDup, rClose, rPipe, rHeredoc, rHerestring
 *	} RedirKind;
 */

static int mode_masks[] = {
	O_RDONLY,			/* rOpen */
	O_TRUNC  | O_CREAT | O_WRONLY,	/* rCreate */
	O_APPEND | O_CREAT | O_WRONLY	/* rAppend */
};

extern int eopen(char *name, RedirKind k) {
	assert((unsigned) k < arraysize(mode_masks));
	return open(name, mode_masks[k], 0666);
}
