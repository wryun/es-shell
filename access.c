/* access.c -- access testing and path searching ($Revision: 1.2 $) */

#define	REQUIRE_STAT	1
#define	REQUIRE_PARAM	1

#include "es.h"
#include "prim.h"

#define	READ	4
#define	WRITE	2
#define	EXEC	1

#define	USER	6
#define	GROUP	3
#define	OTHER	0

/* ingroupset -- determine whether gid lies in the user's set of groups */
static Boolean ingroupset(int gid) {
#ifdef NGROUPS
	int i;
	static int ngroups;
	static gidset_t gidset[NGROUPS];
	static Boolean initialized = FALSE;
	if (!initialized) {
		initialized = TRUE;
		ngroups = getgroups(NGROUPS, gidset);
	}
	for (i = 0; i < ngroups; i++)
		if (gid == gidset[i])
			return TRUE;
#endif
	return FALSE;
}

static int testperm(struct stat *stat, int perm) {
	int mask;
	static int uid, gid;
	static Boolean initialized = FALSE;
	if (perm == 0)
		return 0;
	if (!initialized) {
		initialized = TRUE;
		uid = geteuid();
		gid = getegid();
	}
	mask = (uid == 0)
		? (perm << USER) | (perm << GROUP) | (perm << OTHER)
		: (perm <<
			((uid == stat->st_uid)
				? USER
				: ((gid == stat->st_gid  || ingroupset(stat->st_gid))
					? GROUP
					: OTHER)));
	return (stat->st_mode & mask) ? 0 : EACCES;
}

static int testfile(char *path, int perm, int type) {
	struct stat st;
#ifdef S_IFLNK
	if (type == S_IFLNK) {
		if (lstat(path, &st) == -1)
			return errno;
	} else
#endif
		if (stat(path, &st) == -1)
			return errno;
	if (type != 0 && (st.st_mode & S_IFMT) != type)
		return EACCES;		/* what is an appropriate return value? */
	return testperm(&st, perm);
}

static char *pathcat(char *prefix, char *suffix) {
	char *s;
	size_t plen, slen, len;
	static char *pathbuf = NULL;
	static size_t pathlen = 0;

	if (*prefix == '\0')
		return suffix;
	if (*suffix == '\0')
		return prefix;

	plen = strlen(prefix);
	slen = strlen(suffix);
	len = plen + slen + 2;		/* one for '/', one for '\0' */
	if (pathlen < len) {
		pathlen = len;
		pathbuf = erealloc(pathbuf, pathlen);
	}

	memcpy(pathbuf, prefix, plen);
	s = pathbuf + plen;
	if (s[-1] != '/')
		*s++ = '/';
	memcpy(s, suffix, slen + 1);
	return pathbuf;
}

PRIM(access) {
	int c, perm = 0, type = 0, estatus = ENOENT;
	Boolean first = FALSE, exception = FALSE;
	char *suffix = NULL;
	List *lp;
	const char * const usage = "access [-n name] [-1e] [-rwx] [-fdcblsp] path ...";

	gcdisable();
	esoptbegin(list, "$&access", usage);
	while ((c = esopt("bcdefln:prswx1")) != EOF)
		switch (c) {
		case 'n':	suffix = getstr(esoptarg());	break;
		case '1':	first = TRUE;			break;
		case 'e':	exception = TRUE;		break;
		case 'r':	perm |= READ;			break;
		case 'w':	perm |= WRITE;			break;
		case 'x':	perm |= EXEC;			break;
		case 'f':	type = S_IFREG;			break;
		case 'd':	type = S_IFDIR;			break;
		case 'c':	type = S_IFCHR;			break;
		case 'b':	type = S_IFBLK;			break;
#ifdef S_IFLNK
		case 'l':	type = S_IFLNK;			break;
#endif
#ifdef S_IFSOCK
		case 's':	type = S_IFSOCK;		break;
#endif
#ifdef S_IFIFO
		case 'p':	type = S_IFIFO;			break;
#endif
		default:
			esoptend();
			fail("$&access", "access -%c is not supported on this system", c);
		}
	list = esoptend();

	for (lp = NULL; list != NULL; list = list->next) {
		int error;
		char *name;

		name = getstr(list->term);
		if (suffix != NULL)
			name = pathcat(name, suffix);
		error = testfile(name, perm, type);

		if (first) {
			if (error == 0) {
				Ref(List *, result,
					mklist(mkstr(suffix == NULL
							? name
							: gcdup(name)),
					       NULL));
				gcenable();
				RefReturn(result);
			} else if (error != ENOENT)
				estatus = error;
		} else
			lp = mklist(mkstr(error == 0 ? "0" : esstrerror(error)),
				    lp);
	}

	if (first && exception) {
		gcenable();
		if (suffix)
			fail("$&access", "%s: %s", suffix, esstrerror(estatus));
		else
			fail("$&access", "%s", esstrerror(estatus));
	}

	Ref(List *, result, reverse(lp));
	gcenable();
	RefReturn(result);
}

extern Dict *initprims_access(Dict *primdict) {
	X(access);
	return primdict;
}

extern char *checkexecutable(char *file) {
	int err = testfile(file, EXEC, S_IFREG);
	return err == 0 ? NULL : esstrerror(err);
}
