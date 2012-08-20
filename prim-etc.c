/* prim-etc.c -- miscellaneous primitives */

#include "es.h"
#include "prim.h"

#include <sys/types.h>
#include <sys/ioctl.h>	/* for ioctl() in newpgrp */

extern int setpgrp(int, int);
extern int ioctl(int, int, void *);
extern int umask(int);
extern int chdir(const char *);

PRIM(true) {
	return true;
}

PRIM(false) {
	return false;
}

PRIM(echo) {
	const char *eol = "\n";
	if (list != NULL) {
		const char *opt;
		assert(list->term != NULL);
		opt = list->term->str;
		if (opt != NULL && opt[0] == '-' && opt[2] == '\0') {
			switch (opt[1]) {
			case 'n':
				eol = "";
				list = list->next;
				break;
			case '-':
				list = list->next;
				break;
			default:
				break;
			}
		}
	}
	print("%L%s", list, " ", eol);
	return true;
}

PRIM(count) {
	return mklist(mkterm(str("%d", length(list)), NULL), NULL);
}

PRIM(noexport) {
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next)
		noexport(getstr(lp->term));
	RefEnd(lp);
	return true;
}

PRIM(version) {
	print("%s\n", version);
	return true;
}

PRIM(newpgrp) {
	int pid;
	if (list != NULL)
		fail("usage: newpgrp");
	pid = getpid();
	setpgrp(pid, pid);
#ifdef TIOCSPGRP
	{
		void (*sigtstp)(int) = esignal(SIGTSTP, SIG_IGN);
		void (*sigttin)(int) = esignal(SIGTTIN, SIG_IGN);
		void (*sigttou)(int) = esignal(SIGTTOU, SIG_IGN);
		ioctl(2, TIOCSPGRP, &pid);
		esignal(SIGTSTP, sigtstp);
		esignal(SIGTTIN, sigttin);
		esignal(SIGTTOU, sigttou);
	}
#endif
	return true;
}

PRIM(background) {
	int pid = efork(TRUE, TRUE, TRUE);
	if (pid == 0) {
		setsigdefaults(TRUE);
#if JOB_PROTECT && SIGTTOU && SIGTTIN && SIGTSTP
		/* job control safe version: put it in a new pgroup. */
		/* TODO:
		esignal(SIGTTOU, SIG_IGN);
		esignal(SIGTTIN, SIG_IGN);
		esignal(SIGTSTP, SIG_IGN); */
		setpgrp(0, getpid());
#endif
		mvfd(eopen("/dev/null", rOpen), 0);
		exit(exitstatus(eval(list, NULL, FALSE)));
	}
	if (interactive)
		eprint("%d\n", pid);
	vardef("apid", NULL, mklist(mkterm(str("%d", pid), NULL), NULL));
	return true;
}

PRIM(exit) {
	exit(exitstatus(list));
}

PRIM(exec) {
	return eval(list, NULL, FALSE);
}

PRIM(eval) {
	return runstring(str("%L", list, " "));
}

PRIM(dot) {
	int fd;
	List *e;
	Handler h;
	Boolean save_interactive;

	/* TODO: merge interactive into input state */

	save_interactive = interactive;
	interactive = FALSE;
	Ref(List *, result, NULL);
	Ref(List *, lp, list);
	Ref(char *, file, NULL);
	for (;;) {
		if (lp == NULL) {
			interactive = save_interactive;
			fail("usage: . [-i] file");
		}
		file = getstr(lp->term);
		lp = lp->next;
		if (interactive || !streq(file, "-i"))
			break;
		interactive = TRUE;
	}

	fd = eopen(file, rOpen);
	if (fd == -1) {
		interactive = save_interactive;
		fail("%s: %s", file, strerror(errno));
	}

	if ((e = pushhandler(&h)) != NULL) {
		interactive = save_interactive;
		varpop("*");
		varpop("0");
		throw(e);
	}

	varpush("*", lp);
	varpush("0", mklist(mkterm(file, NULL), NULL));
	noexport("0");

	result = runfd(fd);

	interactive = save_interactive;
	pophandler(&h);
	varpop("*");
	varpop("0");
	RefEnd2(file, lp);
	RefReturn(result);
}

PRIM(umask) {
	if (list == NULL) {
		int mask = umask(0);
		umask(mask);
		print("%04o\n", mask);
		return true;
	}
	if (list->next == NULL) {
		int mask;
		char *s, *t;
		s = getstr(list->term);
		mask = strtol(s, &t, 8);
		if (*t != '\0' || ((unsigned) mask) > 0777)
			fail("bad umask: %s", s);
		if (umask(mask) == -1)
			fail("umask %04d: %s", mask, strerror(errno));
		return true;
	}
	fail("usage: umask [mask]");
}

PRIM(cd) {
	if (list == NULL) {
		char *dir;
		List *home = varlookup("home", NULL);
		if (home == NULL)
			fail("cd: no home directory");
		if (home->next != NULL)
			fail("cd: $home must be 1 word long");
		dir = getstr(home->term);
		if (chdir(dir) == -1)
			fail("chdir %s: %s", dir, strerror(errno));
		return true;
	}
	if (list->next == NULL) {
		char *name = getstr(list->term);
		if (isabsolute(name) || streq(name, ".") || streq(name, ".."))
			if (chdir(name) == -1)
				fail("chdir %s: %s", name, strerror(errno));
			else
				return true;
		else {
			int len = strlen(name);
			Ref(List *, path, varlookup("cdpath", NULL));
			for (; path != NULL; path = path->next) {
				static char *test;
				static size_t testlen = 0;
				
				const char *dir = getstr(path->term);
				size_t pathlen = strlen(dir) + len + 2;
				
				if (testlen < pathlen) {
					testlen = pathlen;
					test = erealloc(test, testlen);
				}
				if (*dir == '\0')
					strcpy(test, name);
				else {
					strcpy(test, dir);
					if (!streq(test, "/"))		/* "//" is special to POSIX */
						strcat(test, "/");
					strcat(test, name);
				}
				if (chdir(test) >= 0) {
					if (interactive && *dir != '\0' && !streq(dir, "."))
						print("%s\n", test);
					RefPop(path);
					return true;
				}
			}
			RefEnd(path);
			fail("cd %s: directory not found", name);
		}
	}
	fail("usage: cd [directory]");
}

PRIM(flatten) {
	char *sep;
	if (list == NULL)
		fail("usage: $&flatten separator [args ...]");
	Ref(List *, lp, list);
	sep = getstr(lp->term);
	lp = mklist(mkterm(str("%L", lp->next, sep), NULL), NULL);
	RefReturn(lp);
}

PRIM(whatis) {
	List *result = true;
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		Term *term = lp->term;
		if (term->closure != NULL)
			print("%C\n", term->closure);
		else {
			Ref(char *, s, term->str);
			if ((list = varlookup2("fn-", s)) != NULL)
				print("%L\n", list, " ");
			else if ((s = which(s, TRUE)) != NULL)
				print("%s\n", s);
			else
				result = false;
			RefEnd(s);
		}
	}
	RefEnd(lp);
	return result;
}

PRIM(split) {		/* should be elsewhere, but also belongs with bqinput */
	if (list == NULL)
		fail("usage: $&split separator [args ...]");
	Ref(List *, lp, list);
	startsplit(getstr(lp->term), TRUE);
	while ((lp = lp->next) != NULL) {
		char *s = getstr(lp->term);
		splitstring(s, strlen(s), TRUE);
	}
	RefEnd(lp);
	return endsplit();
}

PRIM(fsplit) {		/* should be elsewhere, but also belongs with bqinput */
	if (list == NULL)
		fail("usage: $&split separator [args ...]");
	Ref(List *, lp, list);
	startsplit(getstr(lp->term), FALSE);
	while ((lp = lp->next) != NULL) {
		char *s = getstr(lp->term);
		splitstring(s, strlen(s), TRUE);
	}
	RefEnd(lp);
	return endsplit();
}


/*
 * settor functions
 */

PRIM(setprompt) {
	if (length(list) > 2)
		fail("usage: prompt = [main-prompt [continuation-prompt]]");
	if (list == NULL) {
		prompt = NULL;
		prompt2 = NULL;
		return NULL;
	}
	Ref(List *, lp, list);
	prompt = getstr(lp->term);
	prompt2 = (lp->next == NULL) ? NULL : getstr(lp->next->term);
	RefReturn(lp);
}

PRIM(sethistory) {
	if (list == NULL) {
		sethistory(NULL);
		return NULL;
	}
	if (list->next != NULL)
		fail("usage: history = [history-file]");
	Ref(List *, lp, list);
	sethistory(getstr(lp->term));
	RefReturn(lp);
}


/*
 * initialization
 */

extern Dict *initprims_etc(Dict *primdict) {
	X(true);
	X(false);
	X(echo);
	X(count);
	X(noexport);
	X(version);
	X(newpgrp);
	X(background);
	X(exit);
	X(exec);
	X(eval);
	X(dot);
	X(umask);
	X(cd);
	X(flatten);
	X(whatis);
	X(setprompt);
	X(sethistory);
	X(split);
	X(fsplit);
	return primdict;
}
