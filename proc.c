/* proc.c -- process control system calls ($Revision: 1.2 $) */

#include "es.h"
#include "term.h"
#include "prim.h"
#include <termios.h> //fixme:make conditionally

/* TODO: the rusage code for the time builtin really needs to be cleaned up */

#if HAVE_WAIT3
#include <sys/time.h>
#include <sys/resource.h>
#endif

extern void catcher(int); // fixme: put into es.h


Boolean hasforked = FALSE;

typedef struct Proc Proc;
struct Proc {
	Proc *next, *prev;
	int pid, pgid, status;
	Boolean alive, background, stopped, haschanged;
	struct termios tmodes;
	char *cmd;
};
static Proc *proclist = NULL;



/* mkproc -- create a Proc structure */
static Proc *mkproc(int pid, Boolean background, List *cmd) {
	Proc *proc;
	char *s;

	for (proc = proclist; proc != NULL; proc = proc->next)
		if (proc->pid == pid) {		/* are we recycling pids? */
			assert(!proc->alive);	/* if false, violates unix semantics */
			break;
		}
	if (proc == NULL) {
		proc = ealloc(sizeof (Proc));
		proc->next = proclist;
	}
	proc->prev = NULL;
	proc->pid = pid;
	proc->pgid = shell_pgid;
	proc->alive = TRUE;
	proc->background = background;
	proc->stopped = FALSE;
	proc->haschanged = FALSE;
	proc->tmodes = shell_tmodes;
	
	s = (cmd==NULL)?"":str("%L",cmd," ");
	proc->cmd = ealloc(strlen(s)+1);
	strcpy(proc->cmd, s);

	return proc;
}

/* Unlink a process from proclist and free up its memory.
 * A copy of (struct Proc) is kept for further reference and
 * *p instructed to point thereon. This copy remains valid until
 * the next call to unlinkproc. */
static void unlinkproc(Proc **p) {
	static Proc unlinkedproc;
	Proc *proc;
	proc = *p;
	
	if (proc==NULL) {
		return;
	}

	unlinkedproc = *proc;
	if (proc->next != NULL) {
		proc->next->prev = proc->prev;
	}
	if (proc->prev != NULL) {
		proc->prev->next = proc->next;
	} else {
		proclist = proc->next;
	}
	efree(proc->cmd);
	efree(proc);
	*p=&unlinkedproc;
}

/* POSIX stupidity work-around */
/* fixme: newpgrp(pid) does almost the same */
extern int assign_tty(int tty, int pgid) {
	int r;

	block(SIGTTOU);
	r=tcsetpgrp(tty, pgid);
	unblock(SIGTTOU);
	return r;
}

/* pgid=0: pgid:=childpid;     pgid=-1: don't change pg */
extern int efork(Boolean parent, Boolean background, int pgid, List *cmd) {
	if (pgid==-1) {
		pgid=shell_pgid;
	}
	if (parent) {
		int pid = fork();
		switch (pid) {
		default: {	/* parent */
			Proc *proc;
			proc = mkproc(pid, background, cmd);
			if (has_job_control && isinteractive()) {
				if (pgid==0) {
					pgid=pid;
				}
				
				setpgid(pid, pgid);
				proc->pgid=pgid;
				if (!background) {
					assign_tty(shell_tty, pgid);
				}
			}
			if (proclist != NULL)
				proclist->prev = proc;
			proclist = proc;
			return pid;
		}
		case 0:		/* child */
			hasforked = TRUE;
			if (has_job_control && isinteractive()) {
				if (pgid==0) {
					pgid=getpid();
				}
				setpgid(pid, pgid);
				if (!background && isatty(shell_tty)) {
					while (assign_tty(shell_tty, pgid)!=0);
				}
			}
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
	  


/* fixme: delete?
 * We don't need a wrapper for sigint and sigtstp anymore, since those signals
 * are directly received by our childs. But what about other signals?
/* dowait -- a wait wrapper that interfaces with signals */
static int waitpidwrapper(int pid, int *statusp, int options) {
	int n;
	interrupted = FALSE;
	if (!setjmp(slowlabel)) {
		slow = TRUE;
		if (interrupted) {
			n = -2;
		} else {
			n = waitpid(pid, statusp, options);
		}
	} else {
		n = -2;
	}
	slow = FALSE;
	if (n == -2) {
		errno = EINTR;
		n = -1;
	}
	return n;
}

/* Return the first proc which matches the parameters, or otherwise return NULL.
 * If onlyfg is TRUE then *only* forground can processes match. The first process
 * of the process group -pid is returned if pid is negative. A zero pid matches 
 * any process. */
static Proc *getproc(int pid, Boolean onlyfg) {
	Proc *proc;
	for (proc = proclist; proc != NULL; proc = proc->next) {
		if ((pid == proc->pid || -pid == proc->pgid || pid == 0) &&
		    proc->alive && 
		    (!proc->background || !onlyfg) ) {
		  return proc;
		}
	}
	return NULL;
}

/* Give the tty to the shell if appropriate */
extern void ask_for_tty(void) {
	if (isinteractive() && getproc(0, TRUE) == NULL) {
		assign_tty(shell_tty, shell_pgid);
		tcsetattr(shell_tty, TCSADRAIN, &shell_tmodes);
	}
}

/* Remove all dead processes from the proclist. */
static void cleanproclist() {
	Proc *proc;
	for (proc = proclist; proc != NULL; proc = proc->next) {
		if (!proc->alive) {
			unlinkproc(&proc);
		}
	}
}

/* Wait for background processes without actually waiting for them :-) */
static int donotwait(void) {
	Proc *proc;
	int wpid, wstatus, r = FALSE;
	
	for (proc = proclist; proc != NULL; proc = proc->next) {
		if ((proc->alive && proc->background)) {
			wpid = waitpidwrapper(proc->pid, &wstatus, WNOHANG);
			if (wpid == proc->pid) {
				proc->haschanged = TRUE;
				proc->alive = FALSE;
				proc ->status = wstatus;
				r = TRUE;
			}
		}                       
	}
	return r;
}


/* fixme: delete */
static void eprintprocs(void) {
        Proc *proc;
	for (proc = proclist; proc != NULL; proc = proc->next) {
                eprint("pid=%5d pgid=%5d al=%d bg=%d tstp=%d chg=%d %s\n",
                       proc->pid, proc->pgid,
                       proc->alive, proc->background, proc->stopped,
                       proc->haschanged, 
                       proc->cmd);
        }
}

/* Wait for a specific process (pid>0), for any process (pid==0) or
 * for a process group (pid<0). */
extern int ewait(int pid, Boolean interruptible, void *rusage) {
	Proc *proc;
	int wpid, wstatus;

	/* First, kick zombies to hell :-) */
	donotwait();
	
	while (getproc(pid, FALSE) != NULL) {
		wpid = waitpidwrapper(pid, &wstatus, WUNTRACED); /* fixme: dont use the wrapper */
		if (wpid == -1) {
                        /* this is more save than ask_for_tty(); */
                        assign_tty(shell_tty, shell_pgid);
                        fail("es:ewait", "wait: %d %s", pid, esstrerror(errno));
		} else {
                        proc = getproc(wpid, FALSE);
			proc->haschanged = TRUE;
			if (SIFSTOPPED(wstatus)) {
                                catcher(WSTOPSIG(wstatus));
				proc->stopped = TRUE;
				proc->background = TRUE;
				tcgetattr(shell_tty, &proc->tmodes);
				break;
			} else {
                                if (SIFSIGNALED(wstatus)) {
                                        catcher(WTERMSIG(wstatus));
                                }
				proc->alive = FALSE;
				proc->status = wstatus;				
                        }
		}
        }
                
	/* Dead processes are kept in the proclist of an interactive session until
	 * explicit deletion. This is done from the REPL before prompting. Large
	 * interactive jobs must do this themselves to prevent the proclist from 
	 * increasing to infinity. For non interactive sessions there is no need 
	 * to keep dead processes for further reference in the proclist. They are
	 * deleted now: */
	if (!isinteractive()) {
		cleanproclist();
	}
        
	ask_for_tty();
        assign_tty(shell_tty, shell_pgid); //fixme:delete
	
	/* we return with the status of the *last* command of a job */
	return wstatus;
}		
		
	
	

/* Reports all alive process pids, or the pid of the first alive process of
 * each job (option -j), or the pids of all jobs in the process list 
 * (option -a). */
PRIM(apids) {
	Proc *p;
	Boolean jobflag, allflag;
	int pgid = -1;

	Ref(List *, lp, NULL);
		jobflag = (list != NULL && termeq(list->term, "-j"));
		allflag = (list != NULL && termeq(list->term, "-a"));
	
		for (p = proclist; p != NULL; p = p->next) {
			if (allflag || (p->alive && p->background)) {
				if (!jobflag || p->pgid != pgid) {
					Term *t = mkstr(str("%d", p->pid));
					lp = mklist(t, lp);
					pgid=p->pgid;
				}
			}
		}
	/* TODO: sort the return value, but by number? */
	/*       ..., better by time of creation ;-) */
	RefReturn(lp);
}
	  
PRIM(wait) {
	int pid;
	if (list == NULL)
		pid = 0;
	else if (list->next == NULL) {
		pid = atoi(getstr(list->term));
	} else {
		fail("$&wait", "usage: wait [pid]");
		NOTREACHED;
	}
	return mklist(mkstr(mkstatus(ewait(pid, TRUE, NULL))), NULL);
}

PRIM(fg) {
	int pid, pgid, status;
	Proc *proc;

	if (list == NULL || list->next != NULL) {
		fail("$&fg", "usage: $&fg pid");
		NOTREACHED;
	}
	pid = atoi(getstr(list->term));
	
	for (proc = proclist; proc != NULL && proc->pid != pid; proc = proc->next);
	if (proc == NULL || proc->alive == FALSE) {
		fail("$&fg", "$&fg: %d: invalid pid", pid);
		NOTREACHED;
	}
	pgid = proc->pgid;

	assign_tty(shell_tty, proc->pgid);
	tcsetattr(shell_tty, TCSADRAIN, &proc->tmodes);
	for (proc = proclist; proc != NULL; proc = proc->next) {
		if (proc->alive && proc->pgid == pgid) {
			kill(proc->pid, SIGCONT);
			proc->background = FALSE;
			proc->stopped = FALSE;
		}
	}
	for (proc = proclist; proc != NULL; proc = proc->next) {
		if (proc->alive && proc->pgid == pgid) {
                        status = ewait(proc->pid, TRUE, NULL);
		}
	}
        return mklist(mkstr(mkstatus(status)), NULL);
}

PRIM(bg) {
	int pid, pgid;
	Proc *proc;
	Proc *last=NULL;
	
	if (list == NULL || list->next != NULL) {
		fail("$&bg", "usage: $&bg pid");
		NOTREACHED;
	}
	pid = atoi(getstr(list->term));
	
	for (proc = proclist; proc != NULL && proc->pid != pid; proc=proc->next);
	if (proc == NULL || proc->alive == FALSE) {
		fail("$&bg", "$&bg: %d: invalid pid", pid);
		NOTREACHED;
	}
	pgid = proc->pgid;

	for (proc = proclist; proc != NULL; proc = proc->next) {
		if (proc->alive && proc->pgid == pgid) {
			kill(proc->pid, SIGCONT);
			proc->background = TRUE;
			proc->stopped = FALSE;
			last = proc;
		}
	}
	Ref(List *, l, NULL);
		l = mklist(mkstr(last == NULL ? "1" : "0"), NULL);
	RefReturn(l);;
}

PRIM(procinfo) {
	Proc *proc;
	int pid;
	static char *t = "true", *f = "false";

	if (list == NULL || list->next != NULL) {
		fail("$&procinfo", "usage: $&procinfo [pid | -pgid]");
		NOTREACHED;
	}
	pid = atoi(getstr(list->term));
	
	Ref(List *, l, NULL);
		for (proc = proclist;
		     proc != NULL && proc->pid != pid && proc->pgid != -pid;
		     proc = proc->next);
		for (; 
		     pid < 0 && proc->next != NULL && proc->next->pgid == -pid; 
		     proc = proc->next);
		if (proc != NULL) {
			l = mklist(mkstr(str("%d", proc->status)),l); // fixme: cmd last
			l = mklist(mkstr(str("%s", proc->cmd)),l);
			l = mklist(mkstr(str("%s", proc->haschanged ? t : f)),l);
			l = mklist(mkstr(str("%s", proc->stopped ? t : f)),l);
			l = mklist(mkstr(str("%s", proc->background ? t : f)),l);
			l = mklist(mkstr(str("%s", proc->alive ? t : f)),l);
			l = mklist(mkstr(str("%d", proc->pgid)),l);
			l = mklist(mkstr(str("%d", proc->pid)),l);
		}
	RefReturn(l);
}


PRIM(procchange) {
	Proc *proc;
	int pid;

	if (list == NULL || list->next != NULL) {
		fail("$&procchange", "usage: $&procchange pid");
		NOTREACHED;
	}
	pid = atoi(getstr(list->term));
	
	for (proc = proclist; proc != NULL && proc->pid != pid; proc = proc->next);
	if (proc != NULL) {
		proc->haschanged = !proc->haschanged;
	}
	return proc->haschanged ? true : false;
}

/* Free a process from the proclist. You can shoot in your own foot if you
 * remove processes which are still alive -- be aware of zombies! */
PRIM(procfree) {
	Proc *proc;
	int pid;
	
	if (list == NULL || list->next != NULL) {
		fail("$&procfree", "usage: $&procfree pid");
		NOTREACHED;
	}
	pid = atoi(getstr(list->term));
	
	for (proc = proclist; proc != NULL && proc->pid != pid; proc = proc->next);
	if (proc == NULL /* || proc->alive == FALSE */) { 
		fail("$&procfree", "$&procfree: %d: invalid pid", pid);
		NOTREACHED;
	}
	
	/* Don't leave zombies around. */
	waitpid(pid, &proc->status, WNOHANG);
	
	unlinkproc(&proc);
	return NULL;
}

PRIM(donotwait) {
	return donotwait() ? true : false;
}
	
extern Dict *initprims_proc(Dict *primdict) {
	X(apids);
	X(wait);
	X(fg);
	X(bg);
	X(procinfo);
	X(procfree);
	X(procchange);
	X(donotwait);
	return primdict;
}
