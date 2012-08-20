/* open.c -- to insulate <fcntl.h> from the rest of es ($Revision: 1.3 $) */

#define	REQUIRE_FCNTL	1

#include "es.h"

/* prototype for open() follows. comment out if necessary */
#if __MACH__
extern int open(char *, int VARARGS);
#endif

/*
 * Opens a file with the necessary flags.  Assumes the following order:
 *	typedef enum { oOpen, oCreate, oAppend } OpenKind;
 */

static int mode_masks[] = {
	O_RDONLY,			/* rOpen */
	O_TRUNC  | O_CREAT | O_WRONLY,	/* rCreate */
	O_APPEND | O_CREAT | O_WRONLY	/* rAppend */
};

extern int eopen(char *name, OpenKind k) {
	assert((unsigned) k < arraysize(mode_masks));
	return open(name, mode_masks[k], 0666);
}
