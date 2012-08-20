/* prim-sys.c -- system call primitives */

#define	REQUIRE_IOCTL	1
#define REQUIRE_CTYPE   1

#include "es.h"
#include "prim.h"
#include "sigmsgs.h"

PRIM(newpgrp) {
	int pid;
	if (list != NULL)
		fail("usage: newpgrp");
	pid = getpid();
	setpgrp(pid, pid);
#ifdef TIOCSPGRP
	{
		sigresult (*sigtstp)(int) = esignal(SIGTSTP, SIG_IGN);
		sigresult (*sigttin)(int) = esignal(SIGTTIN, SIG_IGN);
		sigresult (*sigttou)(int) = esignal(SIGTTOU, SIG_IGN);
		ioctl(2, TIOCSPGRP, &pid);
		esignal(SIGTSTP, sigtstp);
		esignal(SIGTTIN, sigttin);
		esignal(SIGTTOU, sigttou);
	}
#endif
	return true;
}

PRIM(background) {
	int pid = efork(TRUE, TRUE);
	if (pid == 0) {
#if JOB_PROTECT && SIGTTOU && SIGTTIN && SIGTSTP
		/* job control safe version: put it in a new pgroup. */
		/* TODO:
		esignal(SIGTTOU, SIG_IGN);
		esignal(SIGTTIN, SIG_IGN);
		esignal(SIGTSTP, SIG_IGN); */
		setpgrp(0, getpid());
#endif
		mvfd(eopen("/dev/null", oOpen), 0);
		exit(exitstatus(eval(list, NULL, FALSE, exitonfalse)));
	}
	if (isinteractive())
		eprint("%d\n", pid);
	vardef("apid", NULL, mklist(mkterm(str("%d", pid), NULL), NULL));
	return true;
}

PRIM(exit) {
	exit(exitstatus(list));
	unreached(NULL);
}

PRIM(fork) {
	int pid, status;
	pid = efork(TRUE, FALSE);
	if (pid == 0)
		exit(exitstatus(eval(list, NULL, FALSE, exitonfalse)));
	status = ewaitfor(pid);
	SIGCHK();
	printstatus(0, status);
	return mklist(mkterm(mkstatus(status), NULL), NULL);
}

PRIM(umask) {
	if (list == NULL) {
		int mask = umask(0);
		umask(mask);
		print("%04o\n", mask);
		return true;
	}
	if (list->next == NULL) {
		int mask;
		char *s, *t;
		s = getstr(list->term);
		mask = strtol(s, &t, 8);
		if (*t != '\0' || ((unsigned) mask) > 0777)
			fail("bad umask: %s", s);
		if (umask(mask) == -1)
			fail("umask %04d: %s", mask, strerror(errno));
		return true;
	}
	fail("usage: umask [mask]");
	unreached(NULL);
}

PRIM(cd) {
	if (list == NULL) {
		char *dir;
		List *home = varlookup("home", NULL);
		if (home == NULL)
			fail("cd: no home directory");
		if (home->next != NULL)
			fail("cd: $home must be 1 word long");
		dir = getstr(home->term);
		if (chdir(dir) == -1)
			fail("chdir %s: %s", dir, strerror(errno));
		return true;
	}
	if (list->next == NULL) {
		char *name = getstr(list->term);
		if (isabsolute(name) || streq(name, ".") || streq(name, ".."))
			if (chdir(name) == -1)
				fail("chdir %s: %s", name, strerror(errno));
			else
				return true;
		else {
			int len = strlen(name);
			Ref(List *, path, varlookup("cdpath", NULL));
			for (; path != NULL; path = path->next) {
				static char *test;
				static size_t testlen = 0;
				
				const char *dir = getstr(path->term);
				size_t pathlen = strlen(dir) + len + 2;
				
				if (testlen < pathlen) {
					testlen = pathlen;
					test = erealloc(test, testlen);
				}
				if (*dir == '\0')
					strcpy(test, name);
				else {
					strcpy(test, dir);
					if (!streq(test, "/"))		/* "//" is special to POSIX */
						strcat(test, "/");
					strcat(test, name);
				}
				if (chdir(test) >= 0) {
					if (isinteractive() && *dir != '\0' && !streq(dir, "."))
						print("%s\n", test);
					RefPop(path);
					return true;
				}
			}
			RefEnd(path);
			fail("cd %s: directory not found", name);
		}
	}
	fail("usage: cd [directory]");
	unreached(NULL);
}

PRIM(setsignals) {
	int i;
	Boolean sigs[NUMOFSIGNALS];
	for (i = 0; i < NUMOFSIGNALS; i++)
		sigs[i] = FALSE;
	Ref(List *, result, list);
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		for (i = 1; !streq(getstr(lp->term), signals[i].name); )
			if (++i == NUMOFSIGNALS)
				fail("unknown signal: %s", getstr(lp->term));
		sigs[i] = TRUE;
	}
	RefEnd(lp);
	trapsignals(sigs);
	RefReturn(result);
}

/*
 * limit builtin -- this is too much code for what it gives you
 */

#if BSD_LIMITS

#include <sys/time.h>
#include <sys/resource.h>
extern int getrlimit(int, struct rlimit *);
extern int setrlimit(int, struct rlimit *);

typedef struct Suffix Suffix;
struct Suffix {
	const Suffix *next;
	long amount;
	const char *name;
};

static const Suffix
	kbsuf = { NULL, 1024, "k" },
	mbsuf = { &kbsuf, 1024*1024, "m" },
	gbsuf = { &mbsuf, 1024*1024*1024, "g" },
	stsuf = { NULL, 1, "s" },
	mtsuf = { &stsuf, 60, "m" },
	htsuf = { &mtsuf, 60*60, "h" };
#define	SIZESUF &gbsuf
#define	TIMESUF &htsuf
#define	NOSUF ((Suffix *) NULL)  /* for RLIMIT_NOFILE on SunOS 4.1 */

typedef struct {
	char *name;
	int flag;
	const Suffix *suffix;
} Limit;
static const Limit limits[] = {
	{ "cputime",		RLIMIT_CPU,	TIMESUF },
	{ "filesize",		RLIMIT_FSIZE,	SIZESUF },
	{ "datasize",		RLIMIT_DATA,	SIZESUF },
	{ "stacksize",		RLIMIT_STACK,	SIZESUF },
	{ "coredumpsize",	RLIMIT_CORE,	SIZESUF },
#ifdef RLIMIT_RSS	/* SysVr4 does not have this */
	{ "memoryuse",		RLIMIT_RSS,	SIZESUF },
#endif
#ifdef RLIMIT_VMEM	/* instead, they have this! */
	{ "vmemory",		RLIMIT_VMEM,	SIZESUF },
#endif
#ifdef RLIMIT_NOFILE	/* SunOS 4.1 adds a limit on file descriptors */
	{ "descriptors",	RLIMIT_NOFILE,	NOSUF },
#endif
	{ NULL, 0, NULL }
};

static void printlimit(const Limit *limit, Boolean hard) {
	struct rlimit rlim;
	long lim;
	getrlimit(limit->flag, &rlim);
	if (hard)
		lim = rlim.rlim_max;
	else
		lim = rlim.rlim_cur;
	if (lim == RLIM_INFINITY)
		fprint(1, "%s \tunlimited\n", limit->name);
	else {
		const Suffix *suf;
		for (suf = limit->suffix; suf != NULL; suf = suf->next)
			if (lim % suf->amount == 0 && (lim != 0 || suf->amount > 1)) {
				lim /= suf->amount;
				break;
			}
		eprint("%-8s\t%d%s\n", limit->name, lim, (suf == NULL || lim == 0) ? "" : suf->name);
	}
}

static long parselimit(const Limit *limit, const char *s) {
	long lim;
	int len = strlen(s);
	const char *t;
	const Suffix *suf = limit->suffix;
	if (streq(s, "unlimited"))
		lim = RLIM_INFINITY;
	if (!isdigit(*s))
		return -1;
	if (suf == TIMESUF && (t = strchr(s, ':')) != NULL)
		lim = 60 * atoi(s) + atoi(++t);
	else {
		for (lim = 1; suf != NULL; suf = suf->next)
			if (streq(suf->name, s + len - strlen(suf->name))) {
				lim = suf->amount;
				break;
			}
		lim *= atoi(s);
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
		const char *name = getstr(lp->term);
		for (;; lim++) {
			if (lim->name == NULL)
				fail("%s: no such limit", name);
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
				fail("%s: bad limit value", getstr(lp->term));
			if (hard)
				rlim.rlim_max = n;
			else
				rlim.rlim_cur = n;
			if (setrlimit(lim->flag, &rlim) == -1)
				fail("setrlimit: %s", strerror(errno));
		}
	}
	RefEnd(lp);
	return true;
}

PRIM(time) {
	int pid, status;
	time_t t0, t1;
	struct rusage r;

	Ref(List *, lp, list);

	gc();	/* do a garbage collection first to ensure reproducible results */
	t0 = time(NULL);
	pid = efork(TRUE, FALSE);
	if (pid == 0)
		exit(exitstatus(eval(lp, NULL, FALSE, exitonfalse)));
	status = ewaitfor2(pid, &r);
	t1 = time(NULL);
	SIGCHK();
	printstatus(0, status);

	eprint(
		"%6ldr %5ld.%ldu %5ld.%lds\t%L\n",
		t1 - t0,
		r.ru_utime.tv_sec, (long) (r.ru_utime.tv_usec / 100000),
		r.ru_stime.tv_sec, (long) (r.ru_stime.tv_usec / 100000),
		lp, " "
	);

	RefEnd(lp);
	return mklist(mkterm(mkstatus(status), NULL), NULL);
}

#endif	/* BSD_LIMITS */

extern Dict *initprims_sys(Dict *primdict) {
	X(newpgrp);
	X(background);
	X(exit);
	X(umask);
	X(cd);
	X(fork);
	X(setsignals);
#if BSD_LIMITS
	X(limit);
	X(time);
#endif
	return primdict;
}
