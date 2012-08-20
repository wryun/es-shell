/* config.h -- es(1) configuration parameters */

/*
 * Compile time options
 *
 *	ASSERTIONS
 *		if this is on, asserts will be checked, raising errors on
 *		actual assertion failure.
 *
 *	BSD_LIMITS
 *		if this is on, the limit builtin (ala csh) is included.
 *		also triggers inclusion of the time builtin.
 *
 *	DEVFD
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
 *		makes the garbage collector disable access to pages that are
 *		in old space, making unforwarded references crasht the interpreter.
 *		requires os/mmu support for enabling and disabling access to pages.
 *
 *	GCVERBOSE
 *		if this is on, it is possible to run the garbage collector
 *		in a mode where it explains what it is doing at all times.
 *		implied by GCDEBUG.
 *
 *	INITIAL_PATH
 *		this is the default value for $path (and $PATH) when the shell
 *		starts up.  it is replaced by one from the environment if it
 *		exists.
 *
 *	JOB_PROTECT
 *		set this to true if you want es to perform backgrounding as if
 *		it were a job controlling shell;  that is, if you want background
 *		jobs to be put in new process groups.  this flag is ignored if the
 *		system does not support the job control signals.  since there are
 *		many broken programs that do not behave correctly when backgrounded
 *		in a v7 non-job-control fashion, the default for this option is on,
 *		even though it is ugly.
 *
 *	PROTECT_ENV
 *		if on, makes all variable names in the environment ``safe'':
 *		that is, makes sure no characters other than c identifier
 *		characters appear in them.
 *
 *	READLINE
 *		true if es is being linked with editline or gnu readline.
 *
 *	REISER_CPP
 *		true if es is being compiled with a reiser-style preprocessor.
 *		if you have an ansi preprocessor, use it and turn this off.
 *
 *	SYSV_SIGNALS
 *		true if signal handling follows System V behavior; otherwise,
 *		berkeley signals are assumed.
 *
 *	SPECIAL_SIGCLD
 *		true if SIGCLD has System V semantics.  this is true at least
 *		for silicon graphics machines running Irix.  (according to
 *		Byron, ``if you trap SIGCLD on System V machines, weird things
 *		happen.'')
 *
 *	USE_CONST
 *		allow const declarations.  if your compiler supports 'em, use 'em.
 *
 *	USE_DIRENT
 *		if on, <dirent.h> is used; if off, <sys/direct.h>.
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
 *		allow volatile declarations.  if your compiler supports 'em, use 'em.
 *
 *	USE_UNISTD
 *		define this if you have the include file <unistd.h>
 *
 *	VOID_SIGNALS
 *		define this as true if signal handlers are declared with void
 *		return type; otherwise es uses int for signal returns.
 */


/*
 * platform specific options
 *	please send new configurations to haahr@adobe.com or byron@netapp.com
 */


/* NeXT defaults -- paul haahr */

#if NeXT
#define	DEFAULT_USE_DIRENT	0
#define	DEFAULT_USE_SIG_ATOMIC_T 1
#define	DEFAULT_USE_UNISTD	0
#endif


/* Irix -- derived from rc 1.4 */

#if sgi
#define	DEFAULT_SYSV_SIGNALS	1
#define	DEFAULT_SPECIAL_SIGCLD	1
#define	DEFAULT_INITIAL_PATH	"/usr/bsd", "/usr/sbin", "/usr/bin", "/bin", ""
#endif


/* SunOS -- derived from rc 1.4 */

#if sun
#define	DEFAULT_INITIAL_PATH	"/usr/ucb", "/usr/bin", ""
#endif


/*
 * default defaults -- edit this section, but carefully
 */

#ifndef	DEFAULT_ASSERTIONS
#define	DEFAULT_ASSERTIONS	1
#endif

#ifndef	DEFAULT_BSD_LIMITS
#define	DEFAULT_BSD_LIMITS	1
#endif

#ifndef	DEFAULT_DEVFD
#define	DEFAULT_DEVFD		0
#endif

#ifndef	DEFAULT_DEVFD_PATH
#define	DEFAULT_DEVFD_PATH	"/dev/fd/%d"
#endif

#ifndef	DEFAULT_GCALWAYS
#define	DEFAULT_GCALWAYS	0
#endif

#ifndef	DEFAULT_GCDEBUG
#define	DEFAULT_GCDEBUG		0
#endif

#ifndef	DEFAULT_GCINFO
#define	DEFAULT_GCINFO		0
#endif

#ifndef	DEFAULT_GCPROTECT
#define	DEFAULT_GCPROTECT	0
#endif

#ifndef	DEFAULT_GCVERBOSE
#define	DEFAULT_GCVERBOSE	0
#endif

#ifndef	DEFAULT_INITIAL_PATH
#define	DEFAULT_INITIAL_PATH	"/usr/ucb", "/usr/bin", "/bin", ""
#endif

#ifndef	DEFAULT_JOB_PROTECT
#define	DEFAULT_JOB_PROTECT	1
#endif

#ifndef	DEFAULT_PROTECT_ENV
#define	DEFAULT_PROTECT_ENV	1
#endif

#ifndef	DEFAULT_READLINE
#define	DEFAULT_READLINE	0
#endif

#ifndef	DEFAULT_REISER_CPP
#define	DEFAULT_REISER_CPP	0
#endif

#ifndef	DEFAULT_SYSV_SIGNALS
#define	DEFAULT_SYSV_SIGNALS	0
#endif

#ifndef	DEFAULT_SPECIAL_SIGCLD
#define	DEFAULT_SPECIAL_SIGCLD	0
#endif

#ifndef	DEFAULT_USE_CONST
#define	DEFAULT_USE_CONST	1
#endif

#ifndef	DEFAULT_USE_DIRENT
#define	DEFAULT_USE_DIRENT	1
#endif

#ifndef	DEFAULT_USE_SIG_ATOMIC_T
#define	DEFAULT_USE_SIG_ATOMIC_T 0
#endif

#ifndef	DEFAULT_USE_STDARG
#define	DEFAULT_USE_STDARG	1
#endif

#ifndef	DEFAULT_USE_UNISTD
#define	DEFAULT_USE_UNISTD	1
#endif

#ifndef	DEFAULT_USE_VOLATILE
#define	DEFAULT_USE_VOLATILE	1
#endif

#ifndef	DEFAULT_VOID_SIGNALS
#define	DEFAULT_VOID_SIGNALS	1
#endif


/*
 * actual configuration flags -- do not edit this section
 */

#ifndef	ASSERTIONS
#define	ASSERTIONS	DEFAULT_ASSERTIONS
#endif

#ifndef	BSD_LIMITS
#define	BSD_LIMITS	DEFAULT_BSD_LIMITS
#endif

#ifndef	DEVFD
#define	DEVFD		DEFAULT_DEVFD
#endif

#ifndef	DEVFD_PATH
#define	DEVFD_PATH	DEFAULT_DEVFD_PATH
#endif

#ifndef	GCALWAYS
#define	GCALWAYS	DEFAULT_GCALWAYS
#endif

#ifndef	GCDEBUG
#define	GCDEBUG		DEFAULT_GCDEBUG
#endif

#ifndef	GCINFO
#define	GCINFO		DEFAULT_GCINFO
#endif

#ifndef	GCPROTECT
#define	GCPROTECT	DEFAULT_GCPROTECT
#endif

#ifndef	GCVERBOSE
#define	GCVERBOSE	DEFAULT_GCVERBOSE
#endif

#ifndef	INITIAL_PATH
#define	INITIAL_PATH	DEFAULT_INITIAL_PATH
#endif

#ifndef	JOB_PROTECT
#define	JOB_PROTECT	DEFAULT_JOB_PROTECT
#endif

#ifndef	PROTECT_ENV
#define	PROTECT_ENV	DEFAULT_PROTECT_ENV
#endif

#ifndef	READLINE
#define	READLINE	DEFAULT_READLINE
#endif

#ifndef	REISER_CPP
#define	REISER_CPP	DEFAULT_REISER_CPP
#endif

#ifndef	SYSV_SIGNALS
#define	SYSV_SIGNALS	DEFAULT_SYSV_SIGNALS
#endif

#ifndef	SPECIAL_SIGCLD
#define	SPECIAL_SIGCLD	DEFAULT_SPECIAL_SIGCLD
#endif

#ifndef	USE_CONST
#define	USE_CONST	DEFAULT_USE_CONST
#endif

#ifndef	USE_DIRENT
#define	USE_DIRENT	DEFAULT_USE_DIRENT
#endif

#ifndef	USE_SIG_ATOMIC_T
#define	USE_SIG_ATOMIC_T DEFAULT_USE_SIG_ATOMIC_T
#endif

#ifndef	USE_STDARG
#define	USE_STDARG	DEFAULT_USE_STDARG
#endif

#ifndef	USE_UNISTD
#define	USE_UNISTD	DEFAULT_USE_UNISTD
#endif

#ifndef	USE_VOLATILE
#define	USE_VOLATILE	DEFAULT_USE_VOLATILE
#endif

#ifndef	VOID_SIGNALS
#define	VOID_SIGNALS	DEFAULT_VOID_SIGNALS
#endif


/*
 * enforcing choices that must be made
 */

#if	GCDEBUG
#undef	GCALWAYS
#undef	GCINFO
#undef	GCPROTECT
#undef	GCVERBOSE
#define	GCALWAYS	1
#define	GCINFO		1
#define	GCPROTECT	1
#define	GCVERBOSE	1
#endif
