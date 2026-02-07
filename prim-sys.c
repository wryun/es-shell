/* prim-sys.c -- system call primitives ($Revision: 1.2 $) */

#define	REQUIRE_IOCTL	1

#include "es.h"
#include "prim.h"

#ifdef HAVE_SETRLIMIT
# define BSD_LIMITS 1
#else
# define BSD_LIMITS 0
#endif

#if BSD_LIMITS || BUILTIN_TIME
#include <sys/time.h>
#include <sys/resource.h>
#if !HAVE_GETRUSAGE
#include <sys/times.h>
#include <limits.h>
#endif
#endif

#include <sys/stat.h>

PRIM(newpgrp) {
	int e;
	pid_t pgid;
	if (list != NULL)
		fail("$&newpgrp", "usage: newpgrp");
	pgid = spgrp(getpid());
	if ((e = tctakepgrp()) != 0) {
		spgrp(pgid);
		fail("$&newpgrp", "newpgrp: %s", esstrerror(e));
	}
	return ltrue;
}

PRIM(background) {
	int pid = efork(TRUE, TRUE);
	if (pid == 0) {
#if JOB_PROTECT
		/* job control safe version: put it in a new pgroup, if interactive. */
		if (isinteractive())
			setpgid(0, 0);
#endif
		mvfd(eopen("/dev/null", oOpen), 0);
		esexit(exitstatus(eval(list, NULL, evalflags | eval_inchild)));
	}
	return mklist(mkstr(str("%d", pid)), NULL);
}

PRIM(fork) {
	int pid, status;
	pid = efork(TRUE, FALSE);
	if (pid == 0)
		esexit(exitstatus(eval(list, NULL, evalflags | eval_inchild)));
	status = ewaitfor(pid);
	SIGCHK();
	printstatus(0, status);
	return mklist(mkstr(mkstatus(status)), NULL);
}

PRIM(run) {
	char *file;
	if (list == NULL)
		fail("$&run", "usage: %%run file argv0 argv1 ...");
	Ref(List *, lp, list);
	file = getstr(lp->term);
	lp = forkexec(file, lp->next, (evalflags & eval_inchild) != 0);
	RefReturn(lp);
}

PRIM(umask) {
	if (list == NULL) {
		int mask = umask(0);
		umask(mask);
		print("%04o\n", mask);
		return ltrue;
	}
	if (list->next == NULL) {
		int mask;
		char *s, *t;
		s = getstr(list->term);
		mask = strtol(s, &t, 8);
		if ((t != NULL && *t != '\0') || ((unsigned) mask) > 07777)
			fail("$&umask", "bad umask: %s", s);
		umask(mask);
		return ltrue;
	}
	fail("$&umask", "usage: umask [mask]");
	NOTREACHED;
}

PRIM(cd) {
	char *dir;
	if (list == NULL || list->next != NULL)
		fail("$&cd", "usage: $&cd directory");
	dir = getstr(list->term);
	if (chdir(dir) == -1)
		fail("$&cd", "chdir %s: %s", dir, esstrerror(errno));
	return ltrue;
}

PRIM(setsignals) {
	int i;
	Sigeffect effects[NSIG];
	for (i = 0; i < NSIG; i++)
		effects[i] = sig_default;
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		int sig;
		const char *s = getstr(lp->term);
		Sigeffect effect = sig_catch;
		switch (*s) {
		case '-':	effect = sig_ignore;	s++; break;
		case '/':	effect = sig_noop;	s++; break;
		case '.':	effect = sig_special;	s++; break;
		}
		sig = signumber(s);
		if (sig < 0)
			fail("$&setsignals", "unknown signal: %s", s);
		effects[sig] = effect;
	}
	RefEnd(lp);
	blocksignals();
	setsigeffects(effects);
	unblocksignals();
	return mksiglist();
}

/*
 * limit builtin -- this is too much code for what it gives you
 */

#if BSD_LIMITS
typedef struct Suffix Suffix;
struct Suffix {
	const char *name;
	long amount;
	const Suffix *next;
};

static const Suffix sizesuf[] = {
	{ "g",	1024*1024*1024,	sizesuf + 1 },
	{ "m",	1024*1024,	sizesuf + 2 },
	{ "k",	1024,		NULL },
};

static const Suffix timesuf[] = {
	{ "h",	60 * 60,	timesuf + 1 },
	{ "m",	60,		timesuf + 2 },
	{ "s",	1,		NULL },
};

typedef struct {
	char *name;
	int flag;
	const Suffix *suffix;
} Limit;

static const Limit limits[] = {

	{ "cputime",		RLIMIT_CPU,	timesuf },
	{ "filesize",		RLIMIT_FSIZE,	sizesuf },
	{ "datasize",		RLIMIT_DATA,	sizesuf },
	{ "stacksize",		RLIMIT_STACK,	sizesuf },
	{ "coredumpsize",	RLIMIT_CORE,	sizesuf },

#ifdef RLIMIT_RSS	/* SysVr4 does not have this */
	{ "memoryuse",		RLIMIT_RSS,	sizesuf },
#endif
#ifdef RLIMIT_VMEM	/* instead, they have this! */
	{ "memorysize",		RLIMIT_VMEM,	sizesuf },
#endif

#ifdef RLIMIT_MEMLOCK	/* 4.4bsd adds an unimplemented limit on non-pageable memory */
	{ "lockedmemory",	RLIMIT_CORE,	sizesuf },
#endif

#ifdef RLIMIT_NOFILE	/* SunOS 4.1 adds a limit on file descriptors */
	{ "descriptors",	RLIMIT_NOFILE,	NULL },
#elif defined(RLIMIT_OFILE) /* but 4.4bsd uses this name for it */
	{ "descriptors",	RLIMIT_OFILE,	NULL },
#endif

#ifdef RLIMIT_NPROC	/* 4.4bsd adds a limit on child processes */
	{ "processes",		RLIMIT_NPROC,	NULL },
#endif

	{ NULL, 0, NULL }
};

static void printlimit(const Limit *limit, Boolean hard) {
	struct rlimit rlim;
	LIMIT_T lim;
	getrlimit(limit->flag, &rlim);
	if (hard)
		lim = rlim.rlim_max;
	else
		lim = rlim.rlim_cur;
	if (lim == (LIMIT_T) RLIM_INFINITY)
		print("%-8s\tunlimited\n", limit->name);
	else {
		const Suffix *suf;

		for (suf = limit->suffix; suf != NULL; suf = suf->next)
			if (lim % suf->amount == 0 && (lim != 0 || suf->amount > 1)) {
				lim /= suf->amount;
				break;
			}
		print("%-8s\t%d%s\n", limit->name, (int)lim, (suf == NULL || lim == 0) ? "" : suf->name);
	}
}

static long parselimit(const Limit *limit, char *s) {
	long lim;
	char *t;
	const Suffix *suf = limit->suffix;
	if (streq(s, "unlimited"))
		return RLIM_INFINITY;
	if (!isdigit(*s))
		fail("$&limit", "%s: bad limit value", s);
	if (suf == timesuf && (t = strchr(s, ':')) != NULL) {
		char *u;
		lim = strtol(s, &u, 0) * 60;
		if (u != t)
			fail("$&limit", "%s %s: bad limit value", limit->name, s);
		lim += strtol(u + 1, &t, 0);
		if (t != NULL && *t == ':')
			lim = lim * 60 + strtol(t + 1, &t, 0);
		if (t != NULL && *t != '\0')
			fail("$&limit", "%s %s: bad limit value", limit->name, s);
	} else {
		lim = strtol(s, &t, 0);
		if (t != NULL && *t != '\0')
			for (;; suf = suf->next) {
				if (suf == NULL)
					fail("$&limit", "%s %s: bad limit value", limit->name, s);
				if (streq(suf->name, t)) {
					lim *= suf->amount;
					break;
				}
			}
	}
	return lim;
}

PRIM(limit) {
	const Limit *lim = limits;
	Boolean hard = FALSE;
	Ref(List *, lp, list);

	if (lp != NULL && streq(getstr(lp->term), "-h")) {
		hard = TRUE;
		lp = lp->next;
	}

	if (lp == NULL)
		for (; lim->name != NULL; lim++)
			printlimit(lim, hard);
	else {
		char *name = getstr(lp->term);
		for (;; lim++) {
			if (lim->name == NULL)
				fail("$&limit", "%s: no such limit", name);
			if (streq(name, lim->name))
				break;
		}
		lp = lp->next;
		if (lp == NULL)
			printlimit(lim, hard);
		else {
			long n;
			struct rlimit rlim;
			getrlimit(lim->flag, &rlim);
			if ((n = parselimit(lim, getstr(lp->term))) < 0)
				fail("$&limit", "%s: bad limit value", getstr(lp->term));
			if (hard)
				rlim.rlim_max = (LIMIT_T)n;
			else
				rlim.rlim_cur = (LIMIT_T)n;
			if (setrlimit(lim->flag, &rlim) == -1)
				fail("$&limit", "setrlimit: %s", esstrerror(errno));
		}
	}
	RefEnd(lp);
	return ltrue;
}
#endif	/* BSD_LIMITS */

/*
 * time builtin -- this is nearly as bad as limit
 */

#if BUILTIN_TIME
struct times {
	intmax_t real_usec;
	intmax_t user_usec;
	intmax_t sys_usec;
};

static void tmerrchk(int result, char *str, Boolean throw) {
	if (result != -1)
		return;
	if (throw)
		fail("$&time", "%s: %s", str, esstrerror(errno));
	eprint("%s: %s\n", str, esstrerror(errno));
	eprint("Calls to `$&time` or `time` in this shell may produce bad values.\n");
}

static void getrealtime(struct times *ret, Boolean throw) {
#if HAVE_GETTIMEOFDAY
#define HAVE_PRECISE_REALTIME	1
	struct timeval tv;
	tmerrchk(gettimeofday(&tv, NULL), "getrealtime()", throw);
	ret->real_usec = (tv.tv_sec * INTMAX_C(1000000)) + tv.tv_usec;
#else	/* use time(3p) */
#define HAVE_PRECISE_REALTIME	0
	time_t t = time(NULL);
	tmerrchk(t, "getrealtime()", throw);
	ret->real_usec = t * 1000000;
#endif
}

static void getusagetimes(struct times *ret, Boolean throw) {
#if HAVE_GETRUSAGE
	struct rusage ru_self, ru_child;
	tmerrchk(getrusage(RUSAGE_SELF, &ru_self), "getrusage(RUSAGE_SELF)", throw);
	tmerrchk(getrusage(RUSAGE_CHILDREN, &ru_child), "getrusage(RUSAGE_CHILDREN)", throw);
	ret->user_usec = (ru_self.ru_utime.tv_sec * 1000000)
		+ ru_self.ru_utime.tv_usec
		+ (ru_child.ru_utime.tv_sec * 1000000)
		+ ru_child.ru_utime.tv_usec;
	ret->sys_usec  = (ru_self.ru_stime.tv_sec * 1000000)
		+ ru_self.ru_stime.tv_usec
		+ (ru_child.ru_stime.tv_sec * 1000000)
		+ ru_child.ru_stime.tv_usec;
#else
	struct tms tms;
	static long mul = -1;
	if (mul == -1)
		mul = 1000000 / sysconf(_SC_CLK_TCK);
	tmerrchk(times(&tms), "getusagetimes()", throw);
	ret->user_usec = ((intmax_t)tms.tms_utime + tms.tms_cutime) * mul;
	ret->sys_usec  = ((intmax_t)tms.tms_stime + tms.tms_cstime) * mul;
#endif
}

static void gettimes(struct times *ret, Boolean throw) {
	getrealtime(ret, throw);
	getusagetimes(ret, throw);
}

static void parsetimes(List *list, struct times *ret) {
	char *suffix;
	if (length(list) != 3)
		fail("$&time", "usage: $&time [r u s]");

	ret->real_usec = strtoimax(getstr(list->term), &suffix, 10);
	if (*suffix != '\0')
		fail("$&time", "real-time argument not an int", list->term);
	ret->user_usec = strtoimax(getstr(list->next->term), &suffix, 10);
	if (*suffix != '\0')
		fail("$&time", "user-time argument not an int", list->next->term);
	ret->sys_usec = strtoimax(getstr(list->next->next->term), &suffix, 10);
	if (*suffix != '\0')
		fail("$&time", "sys-time argument not an int", list->next->next->term);
}

static void subtimes(struct times a, struct times b, struct times *ret) {
	ret->real_usec = a.real_usec - b.real_usec;
	ret->user_usec = a.user_usec - b.user_usec;
	ret->sys_usec = a.sys_usec - b.sys_usec;
}

static char *strtimes(struct times time) {
	return str(
#if HAVE_PRECISE_REALTIME
		"%6.3jd"
#else
		"%6jd"
#endif
		"r %7.3jdu %7.3jds",
#if HAVE_PRECISE_REALTIME
		time.real_usec / 1000,
#else
		time.real_usec / 1000000,
#endif
		time.user_usec / 1000,
		time.sys_usec / 1000
	);
}

static struct times base;
extern void setbasetime(void) {
	gettimes(&base, FALSE);
}

PRIM(time) {
	struct times prev, time;

	gettimes(&time, TRUE);
	subtimes(time, base, &time);

	if (list != NULL) {
		parsetimes(list, &prev);
		subtimes(time, prev, &time);
	}

	gcdisable();
	list = mklist(mkstr(strtimes(time)),
		mklist(mkstr(str("%jd", time.real_usec)),
		mklist(mkstr(str("%jd", time.user_usec)),
		mklist(mkstr(str("%jd", time.sys_usec)), NULL))));
	gcenable();
	return list;
}
#endif	/* BUILTIN_TIME */

#if !KERNEL_POUNDBANG
PRIM(execfailure) {
	int fd, len, argc;
	char header[1024], *args[10], *s, *end, *file;

	gcdisable();
	if (list == NULL)
		fail("$&execfailure", "usage: %%exec-failure name argv");

	file = getstr(list->term);
	fd = eopen(file, oOpen);
	if (fd < 0) {
		gcenable();
		return NULL;
	}
	len = read(fd, header, sizeof header);
	close(fd);
	if (len <= 2 || header[0] != '#' || header[1] != '!') {
		gcenable();
		return NULL;
	}

	s = &header[2];
	end = &header[len];
	argc = 0;
	while (argc < arraysize(args) - 1) {
		int c;
		while ((c = *s) == ' ' || c == '\t')
			if (++s >= end) {
				gcenable();
				return NULL;
			}
		if (c == '\n' || c == '\r')
			break;
		args[argc++] = s;
		do
			if (++s >= end) {
				gcenable();
				return NULL;
			}
		while (s < end && (c = *s) != ' ' && c != '\t' && c != '\n' && c != '\r');
		*s++ = '\0';
		if (c == '\n' || c == '\r')
			break;
	}
	if (argc == 0) {
		gcenable();
		return NULL;
	}

	list = list->next;
	if (list != NULL)
		list = list->next;
	list = mklist(mkstr(file), list);
	while (argc != 0)
		list = mklist(mkstr(args[--argc]), list);

	Ref(List *, lp, list);
	gcenable();
	lp = eval(lp, NULL, eval_inchild);
	RefReturn(lp);
}
#endif /* !KERNEL_POUNDBANG */

extern Dict *initprims_sys(Dict *primdict) {
	X(newpgrp);
	X(background);
	X(umask);
	X(cd);
	X(fork);
	X(run);
	X(setsignals);
#if BSD_LIMITS
	X(limit);
#endif
#if BUILTIN_TIME
	X(time);
#endif
#if !KERNEL_POUNDBANG
	X(execfailure);
#endif /* !KERNEL_POUNDBANG */
	return primdict;
}
