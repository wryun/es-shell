/* prim-io.c -- input/output and redirection primitives ($Revision: 1.2 $) */

#include "es.h"
#include "gc.h"
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
	int destfd, srcfd;
	volatile int inparent = (evalflags & eval_inchild) == 0;
	volatile int ticket = UNREGISTERED;

	assert(list != NULL);
	Ref(List *, lp, list);
	destfd = getnumber(getstr(lp->term));
	lp = (*rop)(&srcfd, lp->next);

	ExceptionHandler
		ticket = (srcfd == -1)
			   ? defer_close(inparent, destfd)
			   : defer_mvfd(inparent, srcfd, destfd);
		lp = eval(lp, NULL, evalflags);
		undefer(ticket);
	CatchException (e)
		undefer(ticket);
		throw(e);
	EndExceptionHandler

	RefReturn(lp);
}

#define	REDIR(name)	static List *CONCAT(redir_,name)(int *srcfdp, List *list)

static noreturn argcount(const char *s) {
	fail(caller, "argument count: usage: %s", s);
}

REDIR(openfile) {
	int i, fd;
	char *mode, *name;
	OpenKind kind;
	static const struct {
		const char *name;
		OpenKind kind;
	} modes[] = {
		{ "r",	oOpen },
		{ "w",	oCreate },
		{ "a",	oAppend },
		{ "r+",	oReadWrite },
		{ "w+",	oReadCreate },
		{ "a+",	oReadAppend },
		{ NULL, 0 }
	};

	assert(length(list) == 3);
	Ref(List *, lp, list);

	mode = getstr(lp->term);
	lp = lp->next;
	for (i = 0;; i++) {
		if (modes[i].name == NULL)
			fail("$&openfile", "bad %%openfile mode: %s", mode);
		if (streq(mode, modes[i].name)) {
			kind = modes[i].kind;
			break;
		}
	}

	name = getstr(lp->term);
	lp = lp->next;
	fd = eopen(name, kind);
	if (fd == -1)
		fail("$&openfile", "%s: %s", name, esstrerror(errno));
	*srcfdp = fd;
	RefReturn(lp);
}

PRIM(openfile) {
	List *lp;
	caller = "$&openfile";
	if (length(list) != 4)
		argcount("%openfile mode fd file cmd");
	/* transpose the first two elements */
	lp = list->next;
	list->next = lp->next;
	lp->next = list;
	return redir(redir_openfile, lp, evalflags);
}

REDIR(dup) {
	int fd;
	assert(length(list) == 2);
	Ref(List *, lp, list);
	fd = dup(fdmap(getnumber(getstr(lp->term))));
	if (fd == -1)
		fail("$&dup", "dup: %s", esstrerror(errno));
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
	volatile int pid = 0;

	if (pipe(p) == -1)
		fail(caller, "pipe: %s", esstrerror(errno));

	registerfd(&p[0], FALSE);
	registerfd(&p[1], FALSE);
	if (extra != NULL)
		registerfd(extra, FALSE);

	ExceptionHandler
		pid = efork(TRUE, FALSE);
	CatchExceptionIf (pid != 0, e)
		unregisterfd(&p[0]);
		unregisterfd(&p[1]);
		if (extra != NULL)
			unregisterfd(extra);
		throw(e);
	EndExceptionHandler;

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
		t = mkstr(mkstatus(status));
		result = mklist(t, result);
	} while (0 < n);
	if (evalflags & eval_inchild)
		exit(exitstatus(result));
	RefReturn(result);
}

#if HAVE_DEV_FD
PRIM(readfrom) {
	int pid, p[2], status;
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
	lp = mklist(mkstr(str(DEVFD_PATH, p[0])), NULL);
	varpush(&push, var, lp);

	ExceptionHandler
		lp = eval1(cmd, evalflags);
	CatchException (e)
		close(p[0]);
		ewaitfor(pid);
		throw(e);
	EndExceptionHandler

	close(p[0]);
	status = ewaitfor(pid);
	printstatus(0, status);
	varpop(&push);
	RefEnd3(cmd, input, var);
	RefReturn(lp);
}

PRIM(writeto) {
	int pid, p[2], status;
	Push push;
	Handler h;

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
	lp = mklist(mkstr(str(DEVFD_PATH, p[1])), NULL);
	varpush(&push, var, lp);

	ExceptionHandler
		lp = eval1(cmd, evalflags);
	CatchException (e)
		close(p[1]);
		ewaitfor(pid);
		throw(e);
	EndExceptionHandler

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
		fail("$&backquote", "backquote read: %s", esstrerror(errno));
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
	gcdisable();
	lp = bqinput(sep, p[0]);
	close(p[0]);
	status = ewaitfor(pid);
	printstatus(0, status);
	lp = mklist(mkstr(mkstatus(status)), lp);
	gcenable();
	list = lp;
	RefEnd2(sep, lp);
	SIGCHK();
	return list;
}

PRIM(newfd) {
	if (list != NULL)
		fail("$&newfd", "usage: $&newfd");
	return mklist(mkstr(str("%d", newfd())), NULL);
}

/* read1 -- read one byte */
static int read1(int fd) {
	int nread;
	unsigned char buf;
	do {
		nread = eread(fd, (char *) &buf, 1);
		SIGCHK();
	} while (nread == -1 && errno == EINTR);
	if (nread == -1)
		fail("$&read", esstrerror(errno));
	return nread == 0 ? EOF : buf;
}

PRIM(read) {
	int c;
	int fd = fdmap(0);

	static Buffer *buffer = NULL;
	if (buffer != NULL)
		freebuffer(buffer);
	buffer = openbuffer(0);

	while ((c = read1(fd)) != EOF && c != '\n')
		buffer = bufputc(buffer, c);

	if (c == EOF && buffer->current == 0) {
		freebuffer(buffer);
		buffer = NULL;
		return NULL;
	} else {
		List *result = mklist(mkstr(sealcountedbuffer(buffer)), NULL);
		buffer = NULL;
		return result;
	}
}

extern Dict *initprims_io(Dict *primdict) {
	X(openfile);
	X(close);
	X(dup);
	X(pipe);
	X(backquote);
	X(newfd);
	X(here);
#if HAVE_DEV_FD
	X(readfrom);
	X(writeto);
#endif
	X(read);
	return primdict;
}
