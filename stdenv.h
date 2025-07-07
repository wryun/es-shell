/* stdenv.h -- set up an environment we can use ($Revision: 1.3 $) */

#include "esconfig.h"
#ifdef HAVE_SYS_CDEFS_H
# include <sys/cdefs.h>
#endif

/*
 * protect the rest of es source from the dance of the includes
 */

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if REQUIRE_PARAM
#include <sys/param.h>
#endif

#include <string.h>
#include <stddef.h>

#if HAVE_MEMORY_H
#include <memory.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <ctype.h>

/* #if REQUIRE_STAT || REQUIRE_IOCTL */
/* We need sys/types.h for the prototype of gid_t on Linux */
#include <sys/types.h>
/* #endif */

#if REQUIRE_IOCTL
#include <sys/ioctl.h>
#endif

#if REQUIRE_STAT
#include <sys/stat.h>
#endif

#if REQUIRE_DIRENT
#if HAVE_DIRENT_H
#include <dirent.h>
typedef struct dirent Dirent;
#else
#include <sys/dir.h>
typedef struct direct Dirent;
#endif
/* prototypes for XXXdir functions. comment out if necessary */
#if !HPUX
extern DIR *opendir(const char *);
extern Dirent *readdir(DIR *);
/*extern int closedir(DIR *);*/
#endif
#endif

#if REQUIRE_PWD
#include <pwd.h>
#endif

#if REQUIRE_FCNTL
#include <fcntl.h>
#endif

/* stdlib */
#ifndef Noreturn
#if __GNUC__
#define Noreturn __attribute__((__noreturn__)) void
#else
#define Noreturn void
#endif
#endif

#ifndef UNUSED
#if __GNUC__
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED
#endif
#endif

#ifndef FALLTHROUGH
#if __GNUC__
#define FALLTHROUGH __attribute__((__fallthrough__))
#else
#define FALLTHROUGH (void)0
#endif
#endif

#if STDC_HEADERS
# include <stdlib.h>
#else
extern Noreturn exit(int);
extern Noreturn abort(void);
extern long strtol(const char *num, char **end, int base);
extern void *qsort(
	void *base, size_t nmemb, size_t size,
	int (*compar)(const void *, const void *)
);
#endif /* !STDC_HEADERS */

#if HAVE_READLINE
# include <stdio.h>
#endif

#include <sys/wait.h>
#include <time.h>

/*
 * things that should be defined by header files but might not have been
 */

#ifndef	offsetof
#define	offsetof(t, m)	((size_t) (((char *) &((t *) 0)->m) - (char *)0))
#endif

#ifndef	EOF
#define	EOF	(-1)
#endif

/* setjmp */

#if defined sigsetjmp || HAVE_SIGSETJMP
/* under linux, sigsetjmp and setjmp are both macros 
 * -- need to undef setjmp to avoid problems
 */
# ifdef setjmp
#  undef setjmp
# endif
# define setjmp(buf) sigsetjmp(buf,1)
# define longjmp(x,y)     siglongjmp(x,y)
# define jmp_buf     sigjmp_buf
#endif


/*
 * macros
 */

#define	streq(s, t)		(strcmp(s, t) == 0)
#define	strneq(s, t, n)		(strncmp(s, t, n) == 0)
#define	hasprefix(s, p)		strneq(s, p, (sizeof p) - 1)
#define	arraysize(a)		((int) (sizeof (a) / sizeof (*a)))
#define	memzero(dest, count)	memset(dest, 0, count)
#define	atoi(s)			strtol(s, NULL, 0)

#define	STMT(stmt)		do { stmt; } while (0)
#define	NOP			do {} while (0)

#if REISER_CPP
#define CONCAT(a,b)	a/**/b
#define STRING(s)	"s"
#else
#define CONCAT(a,b)	a ## b
#define STRING(s)	#s
#endif

/* this is not a principled fallback, but it should work on most systems */
#ifndef NSIG
#define NSIG 64
#endif


/*
 * types we use throughout es
 */

#undef FALSE
#undef TRUE
typedef enum { FALSE, TRUE } Boolean;


#if USE_SIG_ATOMIC_T
typedef volatile sig_atomic_t Atomic;
#else
typedef volatile int Atomic;
#endif

typedef GETGROUPS_T gidset_t;


/*
 * variable argument lists
 */

#if HAVE_STDARG_H

#define	VARARGS				, ...
#define	VARARGS1(t1, v1)		(t1 v1, ...)
#define	VARARGS2(t1, v1, t2, v2)	(t1 v1, t2 v2, ...)
#define	VA_START(ap, v)			va_start(ap, v)

#else	/* !HAVE_STDARG_H */

#define	VARARGS
#define	VARARGS1(t1, v1)		(v1, va_alist) t1 v1; va_dcl
#define	VARARGS2(t1, v1, t2, v2)	(v1, v2, va_alist) t1 v1; t2 v2; va_dcl
#define	VA_START(ap, var)		va_start(ap)

/* __va_* are defined by the compiler */
#define va_start(ap)		__va_start(ap)
#define va_copy(dest, src)	__va_copy(dest, src)
#define va_end(ap)		__va_end(ap)

#endif


/*
 * assertion checking
 */

#if ASSERTIONS
#define	assert(expr) \
	STMT( \
		if (!(expr)) { \
			eprint("%s:%d: assertion failed (%s)\n", \
				__FILE__, __LINE__, STRING(expr)); \
			abort(); \
		} \
	)
#else
#define	assert(ignore)	NOP
#endif

enum { UNREACHABLE = 0 };


#define	NOTREACHED	STMT(assert(UNREACHABLE))

/*
 * system calls -- can we get these from some standard header uniformly?
 */

#if !HAVE_UNISTD_H
extern int chdir(const char *dirname);
extern int close(int fd);
extern int dup(int fd);
extern int dup2(int srcfd, int dstfd);
extern int execve(char *name, char **argv, char **envp);
extern int fork(void);
extern int getegid(void);
extern int geteuid(void);
extern int getpagesize(void);
extern int getpid(void);
extern int pipe(int p[2]);
extern int read(int fd, void *buf, size_t n);
extern int umask(int mask);
extern int write(int fd, const void *buf, size_t n);

#if REQUIRE_IOCTL
extern int ioctl(int fd, int cmd, void *arg);
#endif

#if REQUIRE_STAT
extern int stat(const char *, struct stat *);
#endif

#ifdef NGROUPS
extern int getgroups(int, int *);
#endif
#endif	/* !HAVE_UNISTD_H */


/*
 * hacks to present a standard system call interface
 */

#if !HAVE_LSTAT
#define	lstat	stat
#endif



/*
 * macros for picking apart statuses
 *	in general systems should have these macros defined, so this
 *	should all be a bunch of no-ops.  the only interesting case is
 *	WCOREDUMP, which was only very recently standardized and is
 *	still spelled by some systems as WIFCORED.
 */

#ifndef 	WIFSIGNALED
# define	WIFSIGNALED(status)	(((status) & 0xff) != 0)
#endif

#ifndef 	WTERMSIG
# define	WTERMSIG(status)	((status) & 0x7f)
#endif

#ifndef 	WCOREDUMP
# ifdef 	WIFCORED
#  define	WCOREDUMP(status)	(WIFCORED(status))
# else
#  define	WCOREDUMP(status)	((status) & 0x80)
# endif
#endif

#ifndef 	WIFEXITED
# define	WIFEXITED(status)	(!WIFSIGNALED(status))
#endif

#ifndef 	WEXITSTATUS
# define	WEXITSTATUS(status)	(((status) >> 8) & 0xff)
#endif
