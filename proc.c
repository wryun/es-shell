/* proc.c -- process control system calls ($Revision: 1.2 $) */

#include "es.h"

Boolean hasforked = FALSE;

typedef struct Proc Proc;
struct Proc {
	int pid;
	Boolean background;
	Proc *next, *prev;
};

static Proc *proclist = NULL;

static int ttyfd = -1;
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
			while (proclist != NULL) {
				Proc *p = proclist;
				proclist = proclist->next;
				efree(p);
			}
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

extern pid_t spgrp(pid_t pgid) {
	pid_t old = getpgrp();
	setpgid(0, pgid);
	espgid = pgid;
	return old;
}

static int tcspgrp(pid_t pgid) {
	int e = 0;
	Sigeffect tstp, ttin, ttou;
	if (ttyfd < 0)
		return ENOTTY;
	tstp = esignal(SIGTSTP, sig_ignore);
	ttin = esignal(SIGTTIN, sig_ignore);
	ttou = esignal(SIGTTOU, sig_ignore);
	if (tcsetpgrp(ttyfd, pgid) != 0)
		e = errno;
	esignal(SIGTSTP, tstp);
	esignal(SIGTTIN, ttin);
	esignal(SIGTTOU, ttou);
	return e;
}

extern int tctakepgrp(void) {
	pid_t tcpgid = 0;
	if (ttyfd < 0)
		return ENOTTY;
	tcpgid = tcgetpgrp(ttyfd);
	if (espgid == 0 || tcpgid == espgid)
		return 0;
	return tcspgrp(espgid);
}

extern void initpgrp(void) {
	espgid = getpgrp();
	ttyfd = opentty();
#if JOB_PROTECT
	if (ttyfd >= 0)
		tcpgid0 = tcgetpgrp(ttyfd);
#endif
}

#if JOB_PROTECT
extern void tcreturnpgrp(void) {
	if (tcpgid0 != 0 && ttyfd >= 0 && tcpgid0 != tcgetpgrp(ttyfd))
		tcspgrp(tcpgid0);
}

extern Noreturn esexit(int code) {
	tcreturnpgrp();
	exit(code);
}
#endif

/* dowait -- a waitpid wrapper that interfaces with signals */
static int dowait(int pid, int *statusp) {
	int n;
	interrupted = FALSE;
	if (!setjmp(slowlabel)) {
		slow = TRUE;
		n = interrupted ? -2 :
			waitpid(pid, statusp, 0);
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
extern int ewait(int pidarg, Boolean interruptible) {
	int deadpid, status;
	Proc *proc;
	while ((deadpid = dowait(pidarg, &status)) == -1) {
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
	return mklist(mkstr(mkstatus(ewait(pid, TRUE))), NULL);
}

extern Dict *initprims_proc(Dict *primdict) {
	X(apids);
	X(wait);
	return primdict;
}
