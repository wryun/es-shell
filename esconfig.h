/* config.h -- es(1) configuration parameters ($Revision: 1.1.1.1 $) */

/*
 * Compile time options
 *
 *	These options are best set on the command line in the Makefile.
 *	If the machine you use requires a different set of defaults than
 *	is provided, please send mail to
 *
 *		Paul Haahr <haahr@netcom.com>
 *		Byron Rakitzis <byron@netapp.com>
 *
 *	If you decide to add things to this file, add them before the
 *	defaults and make sure that they can be overriden by command
 *	line definitions.  (That is, remember to do the #ifndef dance.)
 *
 *
 *	ASSERTIONS
 *		if this is on, asserts will be checked, raising errors on
 *		actual assertion failure.
 *
 *	BSD_LIMITS
 *		if this is on, the limit builtin (ala csh) is included.
 *
 *	BUILTIN_TIME
 *		if this is on, the time builtin is included.  by default, it is
 *		on, but feel free to turn it off.  see also USE_WAIT3.
 *
 *	HAVE_DEV_FD
 *		turn this on if your system supports /dev/fd for >{} and <{}
 *
 *	DEVFD_PATH
 *		(used only if DEVFD is on.)  a format string for print() to
 *		a file path for opening file descriptor n.
 *
 *	GCALWAYS
 *		if this is on, the a collection is done after every allocation.
 *		this stress-tests the garbage collector.  any missed Ref()
 *		declarations should cause a crash or assertion failure very
 *		quickly in this mode.
 *
 *	GCDEBUG
 *		when this is on, the garbage collector is run in such a way
 *		that just about any coding error will lead to an almost
 *		immediate crash.  it is equivalent to all 3 of GCALWAYS,
 *		GCPROTECT, and GCVERBOSE
 *
 *	GCINFO
 *		a terse version of GCVERBOSE, which prints a short message
 *		for every collection.
 *
 *	GCPROTECT
 *		makes the garbage collector disable access to pages
 *		that are in old space, making unforwarded references
 *		crasht the interpreter.  requires os/mmu support for
 *		enabling and disabling access to pages.
 *
 *	GCVERBOSE
 *		if this is on, it is possible to run the garbage collector
 *		in a mode where it explains what it is doing at all times.
 *		implied by GCDEBUG.
 *
 *	GETGROUPS_USES_GID_T
 *		define this as true if getgroups() takes a gid_t* as its
 *		second argument.  while POSIX.1 says it does, on many
 *		systems, gid_t is a short while getgroups() takes an int*.
 *
 *	HAS_LSTAT
 *		define this as true if your system has lstat(2).  the default
 *		is on.  (it's been reported that SCO does not have lstat, but
 *		is this true even for recent versions?)
 *
 *	INITIAL_PATH
 *		this is the default value for $path (and $PATH) when the shell
 *		starts up.  it is replaced by one from the environment if it
 *		exists.
 *
 *	KERNEL_POUNDBANG
 *		this value should be true when the builtin version of execve(2)
 *		understands #! headers.  if false, es provides a builtin for
 *		running #! files.  the default is true; are there any real
 *		systems still out there that don't support it?
 *
 *	JOB_PROTECT
 *		set this to true if you want es to perform
 *		backgrounding as if it were a job controlling shell;
 *		that is, if you want background jobs to be put in new
 *		process groups.  this flag is ignored if the system
 *		does not support the job control signals.  since there
 *		are many broken programs that do not behave correctly
 *		when backgrounded in a v7 non-job-control fashion, the
 *		default for this option is on, even though it is ugly.
 *
 *	PROTECT_ENV
 *		if on, makes all variable names in the environment ``safe'':
 *		that is, makes sure no characters other than c identifier
 *		characters appear in them.
 *
 *	READLINE
 *		true if es is being linked with editline or gnu readline.
 *
 *	REF_ASSERTIONS
 *		if this is on, assertions about the use of the Ref() macro
 *		will be checked at run-time.  this is only useful if you're
 *		modifying es source, and makes the binary much larger.
 *
 *	REISER_CPP
 *		true if es is being compiled with a reiser-style preprocessor.
 *		if you have an ansi preprocessor, use it and turn this off.
 *
 *	SHOW_DOT_FILES
 *		if this option is off (the default), wildcard patterns do not
 *		match files that being with a ``.'' character;  this behavior
 *		is the same as in most unix shells.  if it is on, the only
 *		files not matched are ``.'' and ``..'';  this behavior follows
 *		convention on bell labs research unix systems (since the eighth
 *		edition) and plan 9.  in either case, an explicit ``.'' at
 *		the beginning of a pattern will match the hidden files.
 *		(Contributed by Steve Kilbane.)
 *
 *	SPECIAL_SIGCLD
 *		true if SIGCLD has System V semantics.  this is true at least
 *		for silicon graphics machines running Irix.  (according to
 *		Byron, ``if you trap SIGCLD on System V machines, weird things
 *		happen.'')
 *
 *	SYSV_SIGNALS
 *		True if signal handling follows System V behavior;
 *		otherwise, berkeley signals are assumed.  If you set
 *		USE_SIGACTION, this value is ignore.  By System V
 *		behavior, we mean, signal must be called to reinstall
 *		the signal handler after it is invoked.  This behavior
 *		is also known as ``unreliable signals.''
 *
 *	USE_CONST
 *		allow const declarations.  if your compiler supports 'em,
 *		use 'em.
 *
 *	USE_DIRENT
 *		if on, <dirent.h> is used; if off, <sys/direct.h>.
 *
 *	USE_MEMORY
 *		if on, <memory.h> is used; if off, it's assumed that
 *		<string.h> does the job.
 *
 *	USE_SIGACTION
 *		turn this on if your system understands the POSIX.1
 *		sigaction(2) call.  it's probably better to use this
 *		version if you have it.  if sigaction() is used, es
 *		assumes that signals have POSIX semantics, so the
 *		SPECIAL_SIGCLD and SYSV_SIGNALS options are turned
 *		off.
 *
 *	USE_SIG_ATOMIC_T
 *		define this on a system which has its own typedef for
 *		sig_atomic_t.
 *
 *	USE_STDARG
 *		define this if you have an ansi compiler and the <stdarg.h>
 *		header file.  if not, es will try to use <varargs.h>, but
 *		you may need to hack a bit to get that working.
 *
 *	USE_VOLATILE
 *		allow volatile declarations.  if your compiler
 *		supports 'em, use 'em.
 *
 *	USE_UNISTD
 *		define this if you have the include file <unistd.h>
 *
 *	USE_WAIT3
 *		this option should be on if your system supports the
 *		BSD-style wait3(2) system call.  by default, it is on.
 *		if this option is false and the BUILTIN_TIME is true,
 *		the times(2) call must exist.
 *
 *	VOID_SIGNALS
 *		define this as true if signal handlers are declared with void
 *		return type; otherwise es uses int for signal returns. */


/*
 * platform specific options
 *	please send new configurations to haahr@adobe.com and byron@netapp.com
 */

#include "config.h"

#if HAVE_SIGRELSE && HAVE_SIGHOLD
# define SYSV_SIGNALS 1
#endif

#if HAVE_LIBREADLINE || HAVE_LIBEDITLINE
# define READLINE 1
#endif

/* NeXT defaults */

#if NeXT
#ifndef	USE_SIG_ATOMIC_T
#define	USE_SIG_ATOMIC_T	1
#endif
#endif	/* NeXT */


/* Irix defaults */

#if sgi
#ifndef	INITIAL_PATH
#define	INITIAL_PATH		"/usr/bsd", "/usr/sbin", "/usr/bin", "/bin", ""
#endif
#endif	/* sgi */


/* SunOS 4.x defaults */

#if sun && !SOLARIS
#ifndef	INITIAL_PATH
#define	INITIAL_PATH		"/usr/ucb", "/usr/bin", ""
#endif
#endif	/* sun */


/* HP/UX 9.0.1 -- from rsalz@osf.org (Rich $alz) and haahr*/

#if HPUX
#define _INCLUDE_POSIX_SOURCE	1
#define _INCLUDE_XOPEN_SOURCE	1
#define _INCLUDE_HPUX_SOURCE	1
#endif


/* SCO Xenix -- from steveo@world.std.com (Steven W Orr) for SCO-ODT-1.1 */

#if sco
#ifndef	USE_SIG_ATOMIC_T
#define USE_SIG_ATOMIC_T	1
#endif
#endif	/* sco */


/* OSF/1 -- this is taken from the DEC Alpha */

#if OSF1
#ifndef	INITIAL_PATH
#define	INITIAL_PATH		"/usr/bin", ""
#endif
#endif	/* OSF1 */

/* OSF/1 on HP snakes -- from John Robert LoVerso <loverso@osf.org> */

#ifdef __hp_osf
#define __NO_FP_VARARGS		/* avoid bug compiling print.c */
#endif


/* DEC Ultrix 4.2 -- from render@massive.uccs.edu (Hal Render) */

#if ultrix
#ifndef USE_SIG_ATOMIC_T
#define USE_SIG_ATOMIC_T	1
#endif
#endif /* ultrix */


/* 386BSD -- from dbarker@mulga.awadi.com.AU (Dave Barker) */

#if __386BSD__
#ifndef	INITIAL_PATH
#define	INITIAL_PATH		"/usr/sbin", "/sbin", "/usr/bin", "/bin", ""
#endif
#define SIG_ERR			BADSIG
#ifndef	REQUIRE_STAT
#define REQUIRE_STAT		1
#endif
#endif


/*
 * default defaults -- don't change this section
 */

#ifndef	ASSERTIONS
#define	ASSERTIONS		1
#endif

#ifndef	BUILTIN_TIME
#define	BUILTIN_TIME		1
#endif

#ifndef	DEVFD_PATH
#define	DEVFD_PATH		"/dev/fd/%d"
#endif

#ifndef	GCALWAYS
#define	GCALWAYS		0
#endif

#ifndef	GCDEBUG
#define	GCDEBUG			0
#endif

#ifndef	GCINFO
#define	GCINFO			0
#endif

#ifndef	GCPROTECT
#define	GCPROTECT		0
#endif

#ifndef	GCVERBOSE
#define	GCVERBOSE		0
#endif

#ifndef	INITIAL_PATH
#define	INITIAL_PATH		"/usr/ucb", "/usr/bin", "/bin", ""
#endif

#ifndef	JOB_PROTECT
#define	JOB_PROTECT		1
#endif

#ifndef	PROTECT_ENV
#define	PROTECT_ENV		1
#endif

#ifndef	READLINE
#define	READLINE		0
#endif

#ifndef	REF_ASSERTIONS
#define	REF_ASSERTIONS		0
#endif

#ifndef	REISER_CPP
#define	REISER_CPP		0
#endif

#ifndef	SHOW_DOT_FILES
#define	SHOW_DOT_FILES		0
#endif

#ifndef	SYSV_SIGNALS
#define	SYSV_SIGNALS		0
#endif

#ifndef	HAVE_MEMORY
#define	HAVE_MEMORY		0
#endif

#ifndef	USE_SIG_ATOMIC_T
#define	USE_SIG_ATOMIC_T	0
#endif

/*
 * enforcing choices that must be made
 */

#if GCDEBUG
#undef	GCALWAYS
#undef	GCINFO
#undef	GCPROTECT
#undef	GCVERBOSE
#define	GCALWAYS		1
#define	GCINFO			1
#define	GCPROTECT		1
#define	GCVERBOSE		1
#endif

#if HAVE_SIGACTION
#undef	SYSV_SIGNALS
#define	SYSV_SIGNALS		0
#endif
