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

#define	IFREG	1
#define	IFDIR	2
#define	IFCHR	3
#define	IFBLK	4
#define	IFLNK	5
#define	IFSOCK	6
#define	IFIFO	7

/* ingroupset -- determine whether gid lies in the user's set of groups */
static Boolean ingroupset(gidset_t gid) {
	int i;
	static int ngroups;
	static gidset_t *gidset;
	static Boolean initialized = FALSE;
	if (!initialized) {
		initialized = TRUE;
		ngroups = getgroups(0, gidset);
		gidset = ealloc(ngroups * sizeof(gidset_t));
		getgroups(ngroups, gidset);
	}
	for (i = 0; i < ngroups; i++)
		if (gid == gidset[i])
			return TRUE;
	return FALSE;
}

static int testperm(struct stat *stat, unsigned int perm) {
	unsigned int mask;
	static gidset_t uid, gid;
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
	return (stat->st_mode & mask) == mask ? 0 : EACCES;
}

static int testfile(char *path, unsigned int perm, unsigned int type) {
	struct stat st;
	if ((type == IFLNK ? lstat(path, &st) : stat(path, &st)) == -1)
		return errno;
	/* is EACCES the right return value? */
	switch(type) {
	case IFREG:	if (!S_ISREG(st.st_mode)) return EACCES; break;
	case IFDIR:	if (!S_ISDIR(st.st_mode)) return EACCES; break;
	case IFBLK:	if (!S_ISBLK(st.st_mode)) return EACCES; break;
	case IFLNK:	if (!S_ISLNK(st.st_mode)) return EACCES; break;
	case IFSOCK:	if (!S_ISSOCK(st.st_mode)) return EACCES; break;
	case IFIFO:	if (!S_ISFIFO(st.st_mode)) return EACCES; break;
	}
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
	int c, estatus = ENOENT;
	unsigned int perm = 0, type = 0;
	Boolean first = FALSE, throws = FALSE;
	char *suffix = NULL;
	List *lp;
	const char * const usage = "access [-n name] [-1e] [-rwx] [-fdcblsp] path ...";

	gcdisable();
	esoptbegin(list, "$&access", usage, TRUE);
	while ((c = esopt("bcdefln:prswx1")) != EOF)
		switch (c) {
		case 'n':	suffix = getstr(esoptarg());	break;
		case '1':	first = TRUE;			break;
		case 'e':	throws = TRUE;			break;
		case 'r':	perm |= READ;			break;
		case 'w':	perm |= WRITE;			break;
		case 'x':	perm |= EXEC;			break;
		case 'f':	type = IFREG;			break;
		case 'd':	type = IFDIR;			break;
		case 'c':	type = IFCHR;			break;
		case 'b':	type = IFBLK;			break;
		case 'l':	type = IFLNK;			break;
		case 's':	type = IFSOCK;			break;
		case 'p':	type = IFIFO;			break;
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
			lp = mklist(mkstr(error == 0 ? "0" : gcdup(esstrerror(error))),
				    lp);
	}

	if (first && throws) {
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
	int err = testfile(file, EXEC, IFREG);
	return err == 0 ? NULL : esstrerror(err);
}
