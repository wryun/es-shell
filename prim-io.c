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

static List *redir(List *(*rop)(int fd, List *list), List *list, Boolean parent) {
	int pid, fd;
	if (parent && (pid = efork(parent, FALSE, FALSE)) != 0) {
		int status;
		status = ewaitfor(pid);
		SIGCHK();
		printstatus(0, status);
		return mklist(mkterm(mkstatus(status), NULL), NULL);
	}

	Ref(List *, lp, list);
	assert(list != NULL);
	fd = getnumber(getstr(lp->term));
	releasefd(fd);
	list = lp->next;
	RefEnd(lp);

	list = eval((*rop)(fd, list), NULL, FALSE, exitonfalse);
	if (parent)
		exit(exitstatus(list));
	return list;
}

static List *simple(OpenKind o, int destfd, List *list) {
	char *name;
	int srcfd;
	assert(length(list) == 2);
	Ref(List *, lp, list);
	name = getstr(lp->term);
	lp = lp->next;
	srcfd = eopen(name, o);
	if (srcfd == -1)
		fail("%s: %s", name, strerror(errno));
	mvfd(srcfd, destfd);
	RefReturn(lp);
}

#define	REDIR(name)	static List *CONCAT(redir_, name)(int destfd, List *list)

static noreturn argcount(const char *s) {
	fail("argument count: usage: %s (too many files in redirection)", s);
}

REDIR(open) {
	return simple(oOpen, destfd, list);
}

PRIM(open) {
	if (length(list) != 3)
		argcount("%open fd file cmd");
	return redir(redir_open, list, parent);
}

REDIR(create) {
	return simple(oCreate, destfd, list);
}

PRIM(create) {
	if (length(list) != 3)
		argcount("%create fd file cmd");
	return redir(redir_create, list, parent);
}

REDIR(append) {
	return simple(oAppend, destfd, list);
}

PRIM(append) {
	if (length(list) != 3)
		argcount("%append fd file cmd");
	return redir(redir_append, list, parent);
}

REDIR(dup) {
	int srcfd;
	assert(length(list) == 2);
	Ref(List *, lp, list);
	srcfd = getnumber(getstr(lp->term));
	mvfd(srcfd, destfd);
	lp = lp->next;
	RefReturn(lp);
}

PRIM(dup) {
	if (length(list) != 3)
		argcount("%dup newfd oldfd cmd");
	return redir(redir_dup, list, parent);
}

REDIR(close) {
	close(destfd);
	return list;
}

PRIM(close) {
	if (length(list) != 2)
		argcount("%close fd cmd");
	return redir(redir_close, list, parent);
}

REDIR(here) {
	int pid, p[2];
	List *doc, *tail, **tailp;

	assert(list != NULL);
	for (tailp = &list; (tail = *tailp)->next != NULL; tailp = &tail->next)
		;
	doc = (list == tail) ? NULL : list;
	*tailp = NULL;

	if (pipe(p) == -1)
		fail("pipe: %s", strerror(errno));
	pid = efork(TRUE, FALSE, FALSE);
	if (pid == 0) {		/* child that writes to pipe */
		close(p[0]);
		fprint(p[1], "%L", doc, "");
		exit(0);
	}

	close(p[1]);
	mvfd(p[0], destfd);
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
		if (list->next != NULL)
			if (pipe(p) == -1)
				fail("pipe: %s", strerror(errno));
		pid = efork(TRUE, FALSE, FALSE);
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
	if (pipe(p) == -1)
		fail("pipe: %s", strerror(errno));
	if ((pid = efork(TRUE, FALSE, FALSE)) == 0) {
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
	int fd;
	if (list != NULL)
		fail("usage: $&newfd");
	fd = dup(3);
	if (fd == -1) {
		if (errno != EBADF)
			fail("newfd: %s", strerror(errno));
		return mklist(mkterm("3", NULL), NULL);
	}
	close(fd);
	return mklist(mkterm(str("%d", fd), NULL), NULL);
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
