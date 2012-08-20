/* prim-io.c -- input/output and redirection primitives ($Revision: 1.8 $) */

#include "es.h"
#include "prim.h"

static const char *caller;

static int getnumber(const char *s) {
	char *end;
	int result = strtol(s, &end, 0);

	if (*end != '\0' || result < 0)
		fail(caller, "bad number: %s", s);
	return result;
}

static List *redir(List *(*rop)(int *fd, List *list), List *list, int evalflags) {
	List *e;
	Handler h;
	int destfd, srcfd;
	volatile int ticket = UNREGISTERED;

	assert(list != NULL);
	Ref(List *, lp, list);
	destfd = getnumber(getstr(lp->term));
	lp = (*rop)(&srcfd, lp->next);

	if ((e = pushhandler(&h)) != NULL) {
		undefer(ticket);
		throw(e);
	}

	ticket = (srcfd == -1)
			? defer_close((evalflags & eval_inchild) == 0, destfd)
			: defer_mvfd((evalflags & eval_inchild) == 0, srcfd, destfd);
	lp = eval(lp, NULL, evalflags);
	undefer(ticket);
	pophandler(&h);

	RefReturn(lp);
}

static List *simple(OpenKind o, int *srcfdp, List *list) {
	char *name;
	int fd;
	assert(length(list) == 2);
	Ref(List *, lp, list);
	name = getstr(lp->term);
	lp = lp->next;
	fd = eopen(name, o);
	if (fd == -1)
		fail(caller, "%s: %s", name, strerror(errno));
	*srcfdp = fd;
	RefReturn(lp);
}

#define	REDIR(name)	static List *CONCAT(redir_,name)(int *srcfdp, List *list)

static noreturn argcount(const char *s) {
	fail(caller, "argument count: usage: %s", s);
}

REDIR(open) {
	return simple(oOpen, srcfdp, list);
}

PRIM(open) {
	caller = "$&open";
	if (length(list) != 3)
		argcount("%open fd file cmd");
	return redir(redir_open, list, evalflags);
}

REDIR(create) {
	return simple(oCreate, srcfdp, list);
}

PRIM(create) {
	caller = "$&create";
	if (length(list) != 3)
		argcount("%create fd file cmd");
	return redir(redir_create, list, evalflags);
}

REDIR(append) {
	return simple(oAppend, srcfdp, list);
}

PRIM(append) {
	caller = "$&append";
	if (length(list) != 3)
		argcount("%append fd file cmd");
	return redir(redir_append, list, evalflags);
}

REDIR(dup) {
	int fd;
	assert(length(list) == 2);
	Ref(List *, lp, list);
	fd = dup(fdmap(getnumber(getstr(lp->term))));
	if (fd == -1)
		fail("$&dup", "dup: %s", strerror(errno));
	*srcfdp = fd;
	lp = lp->next;
	RefReturn(lp);
}

PRIM(dup) {
	caller = "$&dup";
	if (length(list) != 3)
		argcount("%dup newfd oldfd cmd");
	return redir(redir_dup, list, evalflags);
}

REDIR(close) {
	*srcfdp = -1;
	return list;
}

PRIM(close) {
	caller = "$&close";
	if (length(list) != 2)
		argcount("%close fd cmd");
	return redir(redir_close, list, evalflags);
}

/* pipefork -- create a pipe and fork */
static int pipefork(int p[2], int *extra) {
	int pid;
	List *e;
	Handler h;

	if (pipe(p) == -1)
		fail(caller, "pipe: %s", strerror(errno));

	registerfd(&p[0], FALSE);
	registerfd(&p[1], FALSE);
	if (extra != NULL)
		registerfd(extra, FALSE);

	if ((e = pushhandler(&h)) != NULL) {
		unregisterfd(&p[0]);
		unregisterfd(&p[1]);
		if (extra != NULL)
			unregisterfd(extra);
		throw(e);
	}

	pid = efork(TRUE, FALSE);
	if (pid != 0)
		pophandler(&h);

	unregisterfd(&p[0]);
	unregisterfd(&p[1]);
	if (extra != NULL)
		unregisterfd(extra);
	return pid;
}

REDIR(here) {
	int pid, p[2];
	List *doc, *tail, **tailp;

	assert(list != NULL);
	for (tailp = &list; (tail = *tailp)->next != NULL; tailp = &tail->next)
		;
	doc = (list == tail) ? NULL : list;
	*tailp = NULL;

	if ((pid = pipefork(p, NULL)) == 0) {		/* child that writes to pipe */
		close(p[0]);
		fprint(p[1], "%L", doc, "");
		exit(0);
	}

	close(p[1]);
	*srcfdp = p[0];
	return tail;
}

PRIM(here) {
	caller = "$&here";
	if (length(list) < 2)
		argcount("%here fd [word ...] cmd");
	return redir(redir_here, list, evalflags);
}

PRIM(pipe) {
	int n, infd, inpipe;
	static int *pids = NULL, pidmax = 0;

	caller = "$&pipe";
	n = length(list);
	if ((n % 3) != 1)
		fail("$&pipe", "usage: pipe cmd [ outfd infd cmd ] ...");
	n = (n + 2) / 3;
	if (n > pidmax) {
		pids = erealloc(pids, n * sizeof *pids);
		pidmax = n;
	}
	n = 0;

	infd = inpipe = -1;

	for (;; list = list->next) {
		int p[2], pid;
		
		pid = (list->next == NULL) ? efork(TRUE, FALSE) : pipefork(p, &inpipe);

		if (pid == 0) {		/* child */
			if (inpipe != -1) {
				assert(infd != -1);
				releasefd(infd);
				mvfd(inpipe, infd);
			}
			if (list->next != NULL) {
				int fd = getnumber(getstr(list->next->term));
				releasefd(fd);
				mvfd(p[1], fd);
				close(p[0]);
			}
			exit(exitstatus(eval1(list->term, evalflags | eval_inchild)));
		}
		pids[n++] = pid;
		close(inpipe);
		if (list->next == NULL)
			break;
		list = list->next->next;
		infd = getnumber(getstr(list->term));
		inpipe = p[0];
		close(p[1]);
	}

	Ref(List *, result, NULL);
	do {
		Term *t;
		int status = ewaitfor(pids[--n]);
		printstatus(0, status);
		t = mkterm(mkstatus(status), NULL);
		result = mklist(t, result);
	} while (0 < n);
	if (evalflags & eval_inchild)
		exit(exitstatus(result));
	RefReturn(result);
}

#if DEVFD
PRIM(readfrom) {
	int pid, p[2], status;
	List *e;
	Handler h;
	Push push;

	caller = "$&readfrom";
	if (length(list) != 3)
		argcount("%readfrom var input cmd");
	Ref(List *, lp, list);
	Ref(char *, var, getstr(lp->term));
	lp = lp->next;
	Ref(Term *, input, lp->term);
	lp = lp->next;
	Ref(Term *, cmd, lp->term);

	if ((pid = pipefork(p, NULL)) == 0) {
		close(p[0]);
		mvfd(p[1], 1);
		exit(exitstatus(eval1(input, evalflags &~ eval_inchild)));
	}

	close(p[1]);
	lp = mklist(mkterm(str(DEVFD_PATH, p[0]), NULL), NULL);
	varpush(&push, var, lp);

	if ((e = pushhandler(&h)) != NULL) {
		close(p[0]);
		ewaitfor(pid);
		throw(e);
	}

	lp = eval1(cmd, evalflags);

	pophandler(&h);
	close(p[0]);
	status = ewaitfor(pid);
	printstatus(0, status);
	varpop(&push);
	RefEnd3(cmd, input, var);
	RefReturn(lp);
}

PRIM(writeto) {
	int pid, p[2], status;
	List *e;
	Handler h;
	Push push;

	caller = "$&writeto";
	if (length(list) != 3)
		argcount("%writeto var output cmd");
	Ref(List *, lp, list);
	Ref(char *, var, getstr(lp->term));
	lp = lp->next;
	Ref(Term *, output, lp->term);
	lp = lp->next;
	Ref(Term *, cmd, lp->term);

	if ((pid = pipefork(p, NULL)) == 0) {
		close(p[1]);
		mvfd(p[0], 0);
		exit(exitstatus(eval1(output, evalflags &~ eval_inchild)));
	}

	close(p[0]);
	lp = mklist(mkterm(str(DEVFD_PATH, p[1]), NULL), NULL);
	varpush(&push, var, lp);

	if ((e = pushhandler(&h)) != NULL) {
		close(p[1]);
		ewaitfor(pid);
		throw(e);
	}

	lp = eval1(cmd, evalflags);

	pophandler(&h);
	close(p[1]);
	status = ewaitfor(pid);
	printstatus(0, status);
	varpop(&push);
	RefEnd3(cmd, output, var);
	RefReturn(lp);
}
#endif

#define	BUFSIZE	4096

static List *bqinput(const char *sep, int fd) {
	long n;
	char in[BUFSIZE];
	startsplit(sep, TRUE);

restart:
	while ((n = eread(fd, in, sizeof in)) > 0)
		splitstring(in, n, FALSE);
	SIGCHK();
	if (n == -1) {
		if (errno == EINTR)
			goto restart;
		close(fd);
		fail("$&backquote", "backquote read: %s", strerror(errno));
	}
	return endsplit();
}

PRIM(backquote) {
	int pid, p[2], status;
	
	caller = "$&backquote";
	if (list == NULL)
		fail(caller, "usage: backquote separator command [args ...]");

	Ref(List *, lp, list);
	Ref(char *, sep, getstr(lp->term));
	lp = lp->next;

	if ((pid = pipefork(p, NULL)) == 0) {
		mvfd(p[1], 1);
		close(p[0]);
		exit(exitstatus(eval(lp, NULL, evalflags | eval_inchild)));
	}

	close(p[1]);
	gcdisable(0);
	lp = bqinput(sep, p[0]);
	close(p[0]);
	status = ewaitfor(pid);
	printstatus(0, status);
	gcenable();
	list = lp;
	RefEnd2(sep, lp);
	SIGCHK();
	return list;
}

PRIM(newfd) {
	if (list != NULL)
		fail("$&newfd", "usage: $&newfd");
	return mklist(mkterm(str("%d", newfd()), NULL), NULL);
}

extern Dict *initprims_io(Dict *primdict) {
	X(open);
	X(create);
	X(append);
	X(close);
	X(dup);
	X(pipe);
	X(backquote);
	X(newfd);
	X(here);
#if DEVFD
	X(readfrom);
	X(writeto);
#endif
	return primdict;
}
