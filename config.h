/* config.h -- es(1) configuration parameters */

/*
 * Compile time options
 *
 *	ASSERTIONS
 *		if this is on, asserts will be checked, raising errors on
 *		actual assertion failure.
 *
 *	GCDEBUG
 *		when this is on, the garbage collector is run in such a way
 *		that just about any coding error will lead to an almost
 *		immediate crash.  requires os/mmu support for enabling and
 *		disabling access to pages.
 *
 *	PROTECT_ENV
 *		if on, makes all variable names in the environment ``safe'':
 *		that is, makes sure no characters other than c identifier
 *		characters appear in them.
 *
 *	USE_DIRENT
 *		if on, <dirent.h> is used; if off, <sys/direct.h>.
 *
 *	READLINE
 *		true if es is being linked with editline or gnu readline.
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
 *	USE_SIG_ATOMIC_T
 *		define this on a system which has its own typedef for
 *		sig_atomic_t.
 */


/*
 * platform specific options
 *	please send new configurations to haahr@adobe.com or byron@netapp.com
 */


/* NeXT defaults -- paul haahr */

#if NeXT
#define	DEFAULT_USE_DIRENT	0
#define DEFAULT_USE_SIG_ATOMIC_T 1
#endif


/* Irix -- derived from rc 1.4 */

#ifdef sgi
#define	DEFAULT_SYSV_SIGNALS	1
#define	DEFAULT_SPECIAL_SIGCLD	1
#define DEFAULT_INITIAL_PATH	"/usr/bsd", "/usr/sbin", "/usr/bin", "/bin", ""
#endif


/* SunOS -- derived from rc 1.4 */

#ifdef sun
#define DEFAULT_INITIAL_PATH	"/usr/ucb", "/usr/bin", ""
#endif


/*
 * default defaults -- edit this section, but carefully
 */

#ifndef	DEFAULT_ASSERTIONS
#define	DEFAULT_ASSERTIONS	1
#endif

#ifndef	DEFAULT_GCDEBUG
#define	DEFAULT_GCDEBUG		0
#endif

#ifndef	DEFAULT_PROTECT_ENV
#define	DEFAULT_PROTECT_ENV	1
#endif

#ifndef	DEFAULT_USE_DIRENT
#define	DEFAULT_USE_DIRENT	1
#endif

#ifndef	DEFAULT_READLINE
#define	DEFAULT_READLINE	0
#endif

#ifndef	DEFAULT_SYSV_SIGNALS
#define	DEFAULT_SYSV_SIGNALS	0
#endif

#ifndef	DEFAULT_SPECIAL_SIGCLD
#define	DEFAULT_SPECIAL_SIGCLD	0
#endif

#ifndef	DEFAULT_INITIAL_PATH
#define	DEFAULT_INITIAL_PATH	"/usr/ucb", "/usr/bin", "/bin", ""
#endif

#ifndef	DEFAULT_JOB_PROTECT
#define	DEFAULT_JOB_PROTECT	1
#endif

#ifndef DEFAULT_USE_SIG_ATOMIC_T
#define DEFAULT_USE_SIG_ATOMIC_T 0
#endif

/*
 * actual configuration flags -- do not edit this section
 */

#ifndef	ASSERTIONS
#define	ASSERTIONS	DEFAULT_ASSERTIONS
#endif

#ifndef	GCDEBUG
#define	GCDEBUG		DEFAULT_GCDEBUG
#endif

#ifndef	PROTECT_ENV
#define	PROTECT_ENV	DEFAULT_PROTECT_ENV
#endif

#ifndef	USE_DIRENT
#define	USE_DIRENT	DEFAULT_USE_DIRENT
#endif

#ifndef	READLINE
#define	READLINE	DEFAULT_READLINE
#endif

#ifndef	SYSV_SIGNALS
#define	SYSV_SIGNALS	DEFAULT_SYSV_SIGNALS
#endif

#ifndef	SPECIAL_SIGCLD
#define	SPECIAL_SIGCLD	DEFAULT_SPECIAL_SIGCLD
#endif

#ifndef	INITIAL_PATH
#define	INITIAL_PATH	DEFAULT_INITIAL_PATH
#endif

#ifndef	JOB_PROTECT
#define	JOB_PROTECT	DEFAULT_JOB_PROTECT
#endif

#ifndef USE_SIG_ATOMIC_T
#define USE_SIG_ATOMIC_T DEFAULT_USE_SIG_ATOMIC_T
#endif
