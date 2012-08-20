/* proc.c -- process control system calls */

#include "es.h"

Boolean hasforked = FALSE;

extern int fork(void);
extern int wait(int *statusp);


typedef struct Proc Proc;
struct Proc {
	int pid;
	int status;
	Boolean alive, background;
	Proc *next, *prev;
};

static Proc *proclist = NULL;


/* efork -- fork (if necessary) and clean up as appropriate */
extern int efork(Boolean parent, Boolean continuing, Boolean background) {
	if (parent) {
		int pid = fork();
		switch (pid) {
		default: {	/* parent */
			Proc *proc = ealloc(sizeof (Proc));
			proc->pid = pid;
			proc->alive = TRUE;
			proc->next = proclist;
			proc->prev = NULL;
			if (proclist != NULL)
				proclist->prev = proc;
			proclist = proc;
			return pid;
		}
		case 0:
			proclist = NULL;
			hasforked = TRUE;
			break;
		case -1:
			fail("fork: %s", strerror(errno));
		}
	}
	if (!continuing) {
		setsigdefaults(FALSE);
		newchildcatcher();
	}
	return 0;
}

/* ewait -- a wait wrapper that interfaces with signals */
static int ewait(int *statusp) {
	int n;
	interrupted = FALSE;
	if (!setjmp(slowlabel)) {
		slow = TRUE;
		n = interrupted ? -2 : wait(statusp);
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
			return;
		}
}

/* ewaitfor -- wait for a specific process to die */
extern int ewaitfor(int pid) {
	Proc *proc;
	for (proc = proclist; proc != NULL; proc = proc->next)
		if (proc->pid == pid) {
			int status;
			if (proc->alive) {
				int deadpid;
				while ((deadpid = ewait(&proc->status)) != pid) {
					if (deadpid == -1 && errno != EINTR)
						fail("wait: %s", strerror(errno));
					reap(deadpid, proc->status);
				}
			}
			if (proc->next != NULL)
				proc->next->prev = proc->prev;
			if (proc->prev != NULL)
				proc->prev->next = proc->next;
			else
				proclist = proc->next;
			status = proc->status;
			efree(proc);
			return status;
		}
	fail("wait: %d is not a child of this shell", pid);
}
