/* prim-io.c -- input/output and redirection primitives */

#include "es.h"
#include "prim.h"

static int getnumber(const char *s) {
	char *end;
	int result = strtol(s, &end, 0);

	if (*end != '\0' || result < 0)
		fail("bad number: %s", s);
	return result;
}

static List *redir(List *(*rop)(int *fd, List *list), List *list, Boolean parent) {
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
			? defer_close(parent, destfd)
			: defer_mvfd(parent, srcfd, destfd);
	lp = eval(lp, NULL, parent, exitonfalse);
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
		fail("%s: %s", name, strerror(errno));
	*srcfdp = fd;
	RefReturn(lp);
}

#define	REDIR(name)	static List *CONCAT(redir_,name)(int *srcfdp, List *list)

static noreturn argcount(const char *s) {
	fail("argument count: usage: %s (too many files in redirection)", s);
}

REDIR(open) {
	return simple(oOpen, srcfdp, list);
}

PRIM(open) {
	if (length(list) != 3)
		argcount("%open fd file cmd");
	return redir(redir_open, list, parent);
}

REDIR(create) {
	return simple(oCreate, srcfdp, list);
}

PRIM(create) {
	if (length(list) != 3)
		argcount("%create fd file cmd");
	return redir(redir_create, list, parent);
}

REDIR(append) {
	return simple(oAppend, srcfdp, list);
}

PRIM(append) {
	if (length(list) != 3)
		argcount("%append fd file cmd");
	return redir(redir_append, list, parent);
}

REDIR(dup) {
	int fd;
	assert(length(list) == 2);
	Ref(List *, lp, list);
	fd = dup(fdmap(getnumber(getstr(lp->term))));
	if (fd == -1)
		fail("dup: %s", strerror(errno));
	*srcfdp = fd;
	lp = lp->next;
	RefReturn(lp);
}

PRIM(dup) {
	if (length(list) != 3)
		argcount("%dup newfd oldfd cmd");
	return redir(redir_dup, list, parent);
}

REDIR(close) {
	*srcfdp = -1;
	return list;
}

PRIM(close) {
	if (length(list) != 2)
		argcount("%close fd cmd");
	return redir(redir_close, list, parent);
}

/* pipefork -- create a pipe and fork */
static int pipefork(int p[2], int *extra) {
	int pid;
	List *e;
	Handler h;

	if (pipe(p) == -1)
		fail("pipe: %s", strerror(errno));

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
	if (length(list) < 2)
		argcount("%here fd [word ...] cmd");
	return redir(redir_here, list, parent);
}

PRIM(pipe) {
	int n, infd, inpipe;
	static int *pids = NULL, pidmax = 0;

	n = length(list);
	if ((n % 3) != 1)
		fail("usage: pipe cmd [ outfd infd cmd ] ...");
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
			exit(exitstatus(eval1(list->term, FALSE, exitonfalse)));
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
	if (!parent)
		exit(exitstatus(result));
	RefReturn(result);
}

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
		fail("backquote read: %s", strerror(errno));
	}
	return endsplit();
}

PRIM(backquote) {
	int pid, p[2], status;
	Ref(List *, lp, list);
	Ref(char *, sep, getstr(lp->term));
	lp = lp->next;

	if ((pid = pipefork(p, NULL)) == 0) {
		mvfd(p[1], 1);
		close(p[0]);
		exit(exitstatus(eval(lp, NULL, FALSE, exitonfalse)));
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
		fail("usage: $&newfd");
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
	return primdict;
}
