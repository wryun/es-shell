/* proc.c -- process control system calls ($Revision: 1.2 $) */

#include "es.h"

/* TODO: the rusage code for the time builtin really needs to be cleaned up */

#if HAVE_GETRUSAGE
#include <sys/time.h>
#include <sys/resource.h>
#endif

Boolean hasforked = FALSE;

typedef struct Proc Proc;
struct Proc {
	int pid;
	Boolean background;
	Proc *next, *prev;
};

static Proc *proclist = NULL;

static pid_t espgid;
#if JOB_PROTECT
static pid_t tcpgid0;
#endif

/* mkproc -- create a Proc structure */
extern Proc *mkproc(int pid, Boolean background) {
	Proc *proc = ealloc(sizeof (Proc));
	proc->next = proclist;
	proc->pid = pid;
	proc->background = background;
	proc->prev = NULL;
	return proc;
}

/* efork -- fork (if necessary) and clean up as appropriate */
extern int efork(Boolean parent, Boolean background) {
	if (parent) {
		int pid = fork();
		switch (pid) {
		default: {	/* parent */
			Proc *proc = mkproc(pid, background);
			if (proclist != NULL)
				proclist->prev = proc;
			proclist = proc;
			return pid;
		}
		case 0:		/* child */
			proclist = NULL;
			hasforked = TRUE;
#if JOB_PROTECT
			tcpgid0 = 0;
#endif
			break;
		case -1:
			fail("es:efork", "fork: %s", esstrerror(errno));
		}
	}
	closefds();
	setsigdefaults();
	newchildcatcher();
	return 0;
}

extern void newpgrp(void) {
	setpgid(0, 0);
	espgid = getpgrp();
}

static void tcspgrp(pid_t pgid) {
	Sigeffect tstp = esignal(SIGTSTP, sig_ignore);
	Sigeffect ttin = esignal(SIGTTIN, sig_ignore);
	Sigeffect ttou = esignal(SIGTTOU, sig_ignore);
	tcsetpgrp(2, pgid);
	esignal(SIGTSTP, tstp);
	esignal(SIGTTIN, ttin);
	esignal(SIGTTOU, ttou);
}

extern void tctakepgrp(void) {
	pid_t tcpgid;
	tcpgid = tcgetpgrp(2);
	if (tcpgid != espgid)
		tcspgrp(espgid);
}

extern void initpgrp(void) {
	espgid = getpgrp();
#if JOB_PROTECT
	tcpgid0 = tcgetpgrp(2);
#endif
}

#if JOB_PROTECT
extern void tcreturnpgrp(void) {
	if (tcpgid0 != 0 && tcpgid0 != tcgetpgrp(2))
		tcspgrp(tcpgid0);
}

extern Noreturn esexit(int code) {
	tcreturnpgrp();
	exit(code);
}
#endif

#if HAVE_GETRUSAGE
/* This function is provided as timersub(3) on some systems, but it's simple enough
 * to do ourselves. */
static void timesub(struct timeval *a, struct timeval *b, struct timeval *res) {
	res->tv_sec = a->tv_sec - b->tv_sec;
	res->tv_usec = a->tv_usec - b->tv_usec;
	if (res->tv_usec < 0) {
		res->tv_sec -= 1;
		res->tv_usec += 1000000;
	}
}
#endif

/* dowait -- a waitpid wrapper that gets rusage and interfaces with signals */
static int dowait(int pid, int *statusp, void UNUSED *rusagep) {
	int n;
#if HAVE_GETRUSAGE
	static struct rusage ru_saved;
	struct rusage ru_new;
#endif
	interrupted = FALSE;
	if (!setjmp(slowlabel)) {
		slow = TRUE;
		n = interrupted ? -2 :
			waitpid(pid, statusp, 0);
#if HAVE_GETRUSAGE
		if (rusagep != NULL) {
			struct rusage *rusage = (struct rusage *)rusagep;
			if (getrusage(RUSAGE_CHILDREN, &ru_new) == -1)
				fail("es:ewait", "getrusage: %s", esstrerror(errno));
			timesub(&ru_new.ru_utime, &ru_saved.ru_utime, &rusage->ru_utime);
			timesub(&ru_new.ru_stime, &ru_saved.ru_stime, &rusage->ru_stime);
			ru_saved = ru_new;
		}
#endif
	} else
		n = -2;
	slow = FALSE;
	if (n == -2) {
		errno = EINTR;
		n = -1;
	}
	return n;
}

/* reap -- mark a process as dead and return it */
static Proc *reap(int pid) {
	Proc *proc;
	for (proc = proclist; proc != NULL; proc = proc->next)
		if (proc->pid == pid)
			break;
	assert(proc != NULL);
	if (proc->next != NULL)
		proc->next->prev = proc->prev;
	if (proc->prev != NULL)
		proc->prev->next = proc->next;
	else
		proclist = proc->next;
	return proc;
}

/* ewait -- wait for a specific process to die, or any process if pid == -1 */
extern int ewait(int pidarg, Boolean interruptible, void *rusage) {
	int deadpid, status;
	Proc *proc;
	while ((deadpid = dowait(pidarg, &status, rusage)) == -1) {
		if (errno == ECHILD && pidarg > 0)
			fail("es:ewait", "wait: %d is not a child of this shell", pidarg);
		else if (errno != EINTR)
			fail("es:ewait", "wait: %s", esstrerror(errno));
		if (interruptible)
			SIGCHK();
	}
	proc = reap(deadpid);
#if JOB_PROTECT
	tctakepgrp();
#endif
	if (proc->background)
		printstatus(proc->pid, status);
	efree(proc);
	return status;
}

#include "prim.h"

PRIM(apids) {
	Proc *p;
	Ref(List *, lp, NULL);
	for (p = proclist; p != NULL; p = p->next)
		if (p->background) {
			Term *t = mkstr(str("%d", p->pid));
			lp = mklist(t, lp);
		}
	/* TODO: sort the return value, but by number? */
	RefReturn(lp);
}

PRIM(wait) {
	int pid;
	if (list == NULL)
		pid = -1;
	else if (list->next == NULL) {
		pid = atoi(getstr(list->term));
		if (pid <= 0) {
			fail("$&wait", "wait: %d: bad pid", pid);
			NOTREACHED;
		}
	} else {
		fail("$&wait", "usage: wait [pid]");
		NOTREACHED;
	}
	return mklist(mkstr(mkstatus(ewait(pid, TRUE, NULL))), NULL);
}

extern Dict *initprims_proc(Dict *primdict) {
	X(apids);
	X(wait);
	return primdict;
}
