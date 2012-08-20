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

static List *redir(List *list, Boolean parent, RedirKind r) {
	int srcfd = 0 /* uninitialized use ok */, dstfd, pid;

	Ref(List *, lp, list);
	Ref(char *, str, getstr(lp->term));
	
	dstfd = getnumber(str);

	if (r == rDup) {
		lp = lp->next;
		str = getstr(lp->term);
		srcfd = getnumber(str);
	}

	if (parent && (pid = efork(parent, FALSE, FALSE)) != 0) {
		int status;
		RefPop2(str, lp);
		status = ewaitfor(pid);
		printstatus(0, status);
		return mklist(mkterm(mkstatus(status), NULL), NULL);
	}

	switch (r) {
	case rOpen: case rCreate: case rAppend:
		lp = lp->next;
		str = getstr(lp->term);
		srcfd = eopen(str, r);
		if (srcfd < 0) {
			uerror(str);
			RefPop2(str, lp);
			exit(1);	/* TODO */
		}
		mvfd(srcfd, dstfd);
		break;
	case rClose:
		close(dstfd);
		break;
	case rDup:
		mvfd(srcfd, dstfd);
		break;
	default:
		panic("redirection not handled");
	}
	lp = lp->next;
	exit(exitstatus(eval(lp, NULL, FALSE)));
	/* NOTREACHED */
	RefEnd2(str, lp);
}

PRIM(open) {
	return redir(list, parent, rOpen);
}
PRIM(create) {
	return redir(list, parent, rCreate);
}

PRIM(append) {
	return redir(list, parent, rAppend);
}

PRIM(close) {
	return redir(list, parent, rClose);
}

PRIM(dup) {
	return redir(list, parent, rDup);
}

PRIM(one) {
	if (list == NULL)
		fail("null filename used in redirection");
	if (list->next != NULL)
		fail("multi-word filename used in redirection");
	return list;
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
			if (inpipe != -1)
				mvfd(inpipe, infd);
			if (list->next != NULL) {
				close(p[0]);
				mvfd(p[1], getnumber(getstr(list->next->term)));
			}
			exit(exitstatus(eval1(list->term, FALSE)));
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
		exit(exitstatus(eval(lp, NULL, FALSE)));
	}
	close(p[1]);
	list = bqinput(sep, p[0]);
	close(p[0]);
	status = ewaitfor(pid);
	printstatus(0, status);
	RefEnd2(sep, lp);
	SIGCHK();
	return list;
}

extern Dict *initprims_io(Dict *primdict) {
	X(open);
	X(create);
	X(append);
	X(close);
	X(dup);
	X(pipe);
	X(backquote);
	X(one);
	return primdict;
}
