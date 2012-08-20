/* config.h -- es(1) configuration parameters ($Revision: 1.4 $) */

/*
 * Compile time options
 *
 *	These options are best set on the command line in the Makefile.
 *	If the machine you use requires a different set of defaults than
 *	is provided, please send mail to
 *		haahr@adobe.com
 *		byron@netapp.com
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
 *	KERNEL_POUNDBANG
 *		this value should be true when the builtin version of execve(2)
 *		understands #! headers.  if false, es provides a builtin for
 *		running #! files.  the default is true; are there any real
 *		systems still out there that don't support it?
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
 *	please send new configurations to haahr@adobe.com and byron@netapp.com
 */


/* NeXT defaults -- paul haahr */

#if NeXT
#ifndef	USE_DIRENT
#define	USE_DIRENT	0
#endif
#ifndef	USE_SIG_ATOMIC_T
#define	USE_SIG_ATOMIC_T 1
#endif
#ifndef	USE_UNISTD
#define	USE_UNISTD	0
#endif
#endif	/* NeXT */


/* Irix -- derived from rc 1.4 */

#if sgi
#ifndef	SYSV_SIGNALS
#define	SYSV_SIGNALS	1
#endif
#ifndef	SPECIAL_SIGCLD
#define	SPECIAL_SIGCLD	1
#endif
#ifndef	INITIAL_PATH
#define	INITIAL_PATH	"/usr/bsd", "/usr/sbin", "/usr/bin", "/bin", ""
#endif
#endif	/* sgi */


/* SunOS -- derived from rc 1.4 */

#if sun
#ifndef	INITIAL_PATH
#define	INITIAL_PATH	"/usr/ucb", "/usr/bin", ""
#endif
#endif	/* sun */


/*
 * default defaults -- don't change this section
 */

#ifndef	ASSERTIONS
#define	ASSERTIONS	1
#endif

#ifndef	BSD_LIMITS
#define	BSD_LIMITS	1
#endif

#ifndef	DEVFD
#define	DEVFD		0
#endif

#ifndef	DEVFD_PATH
#define	DEVFD_PATH	"/dev/fd/%d"
#endif

#ifndef	GCALWAYS
#define	GCALWAYS	0
#endif

#ifndef	GCDEBUG
#define	GCDEBUG		0
#endif

#ifndef	GCINFO
#define	GCINFO		0
#endif

#ifndef	GCPROTECT
#define	GCPROTECT	0
#endif

#ifndef	GCVERBOSE
#define	GCVERBOSE	0
#endif

#ifndef	INITIAL_PATH
#define	INITIAL_PATH	"/usr/ucb", "/usr/bin", "/bin", ""
#endif

#ifndef	KERNEL_POUNDBANG
#define	KERNEL_POUNDBANG 1
#endif

#ifndef	JOB_PROTECT
#define	JOB_PROTECT	1
#endif

#ifndef	PROTECT_ENV
#define	PROTECT_ENV	1
#endif

#ifndef	READLINE
#define	READLINE	0
#endif

#ifndef	REISER_CPP
#define	REISER_CPP	0
#endif

#ifndef	SYSV_SIGNALS
#define	SYSV_SIGNALS	0
#endif

#ifndef	SPECIAL_SIGCLD
#define	SPECIAL_SIGCLD	0
#endif

#ifndef	USE_CONST
#define	USE_CONST	1
#endif

#ifndef	USE_DIRENT
#define	USE_DIRENT	1
#endif

#ifndef	USE_SIG_ATOMIC_T
#define	USE_SIG_ATOMIC_T 0
#endif

#ifndef	USE_STDARG
#define	USE_STDARG	1
#endif

#ifndef	USE_UNISTD
#define	USE_UNISTD	1
#endif

#ifndef	USE_VOLATILE
#define	USE_VOLATILE	1
#endif

#ifndef	VOID_SIGNALS
#define	VOID_SIGNALS	1
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
