/* proc.c -- process control system calls ($Revision: 1.2 $) */

#include "es.h"

/* TODO: the rusage code for the time builtin really needs to be cleaned up */

#if HAVE_WAIT3
#include <sys/time.h>
#include <sys/resource.h>
#endif

Boolean hasforked = FALSE;

typedef struct Proc Proc;
struct Proc {
	int pid;
	int status;
	Boolean alive, background;
	Proc *next, *prev;
#if HAVE_WAIT3
	struct rusage rusage;
#endif
};

static Proc *proclist = NULL;

/* mkproc -- create a Proc structure */
extern Proc *mkproc(int pid, Boolean background) {
	Proc *proc;
	for (proc = proclist; proc != NULL; proc = proc->next)
		if (proc->pid == pid) {		/* are we recycling pids? */
			assert(!proc->alive);	/* if false, violates unix semantics */
			break;
		}
	if (proc == NULL) {
		proc = ealloc(sizeof (Proc));
		proc->next = proclist;
	}
	proc->pid = pid;
	proc->alive = TRUE;
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

#if HAVE_WAIT3
static struct rusage wait_rusage;
#endif

/* dowait -- a wait wrapper that interfaces with signals */
static int dowait(int *statusp) {
	int n;
	interrupted = FALSE;
	if (!setjmp(slowlabel)) {
		slow = TRUE;
		n = interrupted ? -2 :
#if HAVE_WAIT3
			wait3((void *) statusp, 0, &wait_rusage);
#else
			wait((void *) statusp);
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

/* reap -- mark a process as dead and attach its exit status */
static void reap(int pid, int status) {
	Proc *proc;
	for (proc = proclist; proc != NULL; proc = proc->next)
		if (proc->pid == pid) {
			assert(proc->alive);
			proc->alive = FALSE;
			proc->status = status;
#if HAVE_WAIT3
			proc->rusage = wait_rusage;
#endif
			return;
		}
}

/* ewait -- wait for a specific process to die, or any process if pid == 0 */
extern int ewait(int pid, Boolean interruptible, void *rusage) {
	Proc *proc;
top:
	for (proc = proclist; proc != NULL; proc = proc->next)
		if (proc->pid == pid || (pid == 0 && !proc->alive)) {
			int status;
			if (proc->alive) {
				int deadpid;
				while ((deadpid = dowait(&proc->status)) != pid)
					if (deadpid != -1)
						reap(deadpid, proc->status);
					else if (errno != EINTR)
						fail("es:ewait", "wait: %s", esstrerror(errno));
					else if (interruptible)
						SIGCHK();
				proc->alive = FALSE;
#if HAVE_WAIT3
				proc->rusage = wait_rusage;
#endif
			}
			if (proc->next != NULL)
				proc->next->prev = proc->prev;
			if (proc->prev != NULL)
				proc->prev->next = proc->next;
			else
				proclist = proc->next;
			status = proc->status;
			if (proc->background)
				printstatus(proc->pid, status);
			efree(proc);
#if HAVE_WAIT3
			if (rusage != NULL)
				memcpy(rusage, &proc->rusage, sizeof (struct rusage));
#else
			assert(rusage == NULL);
#endif
			return status;
		}
	if (pid == 0) {
		int status;
		while ((pid = dowait(&status)) == -1) {
			if (errno != EINTR)
				fail("es:ewait", "wait: %s", esstrerror(errno));
			if (interruptible)
				SIGCHK();
		}
		reap(pid, status);
		goto top;
	}
	fail("es:ewait", "wait: %d is not a child of this shell", pid);
	NOTREACHED;
}

#include "prim.h"

PRIM(apids) {
	Proc *p;
	Ref(List *, lp, NULL);
	for (p = proclist; p != NULL; p = p->next)
		if (p->background && p->alive) {
			Term *t = mkstr(str("%d", p->pid));
			lp = mklist(t, lp);
		}
	/* TODO: sort the return value, but by number? */
	RefReturn(lp);
}

PRIM(wait) {
	int pid;
	if (list == NULL)
		pid = 0;
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
