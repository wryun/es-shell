/* prim-sys.c -- system call primitives */

#define	REQUIRE_IOCTL	1

#include "es.h"
#include "prim.h"
#include "sigmsgs.h"

extern int umask(int mask);

PRIM(newpgrp) {
	int pid;
	if (list != NULL)
		fail("usage: newpgrp");
	pid = getpid();
	setpgrp(pid, pid);
#ifdef TIOCSPGRP
	{
		sigresult (*sigtstp)(int) = esignal(SIGTSTP, SIG_IGN);
		sigresult (*sigttin)(int) = esignal(SIGTTIN, SIG_IGN);
		sigresult (*sigttou)(int) = esignal(SIGTTOU, SIG_IGN);
		ioctl(2, TIOCSPGRP, &pid);
		esignal(SIGTSTP, sigtstp);
		esignal(SIGTTIN, sigttin);
		esignal(SIGTTOU, sigttou);
	}
#endif
	return true;
}

PRIM(background) {
	int pid = efork(TRUE, TRUE);
	if (pid == 0) {
#if JOB_PROTECT && SIGTTOU && SIGTTIN && SIGTSTP
		/* job control safe version: put it in a new pgroup. */
		/* TODO:
		esignal(SIGTTOU, SIG_IGN);
		esignal(SIGTTIN, SIG_IGN);
		esignal(SIGTSTP, SIG_IGN); */
		setpgrp(0, getpid());
#endif
		mvfd(eopen("/dev/null", oOpen), 0);
		exit(exitstatus(eval(list, NULL, FALSE, exitonfalse)));
	}
	if (isinteractive())
		eprint("%d\n", pid);
	vardef("apid", NULL, mklist(mkterm(str("%d", pid), NULL), NULL));
	return true;
}

PRIM(exit) {
	exit(exitstatus(list));
	unreached(NULL);
}

PRIM(fork) {
	int pid, status;
	pid = efork(TRUE, FALSE);
	if (pid == 0)
		exit(exitstatus(eval(list, NULL, FALSE, exitonfalse)));
	status = ewaitfor(pid);
	SIGCHK();
	printstatus(0, status);
	return mklist(mkterm(mkstatus(status), NULL), NULL);
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
	unreached(NULL);
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
					if (isinteractive() && *dir != '\0' && !streq(dir, "."))
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
	unreached(NULL);
}

PRIM(setsignals) {
	int i;
	Boolean sigs[NUMOFSIGNALS];
	for (i = 0; i < NUMOFSIGNALS; i++)
		sigs[i] = FALSE;
	Ref(List *, result, list);
	Ref(List *, lp, list);
	for (; lp != NULL; lp = lp->next) {
		for (i = 1; !streq(getstr(lp->term), signals[i].name); )
			if (++i == NUMOFSIGNALS)
				fail("unknown signal: %s", getstr(lp->term));
		sigs[i] = TRUE;
	}
	RefEnd(lp);
	trapsignals(sigs);
	RefReturn(result);
}

extern Dict *initprims_sys(Dict *primdict) {
	X(newpgrp);
	X(background);
	X(exit);
	X(umask);
	X(cd);
	X(fork);
	X(setsignals);
	return primdict;
}
