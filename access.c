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
static Boolean ingroupset(gidset_t gid) {
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
	return (stat->st_mode & mask) ? 0 : EACCES;
}

static int testfile(char *path, int perm, unsigned int type) {
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

static int *permtab[256] = { NULL };
static int *typetab[256] = { NULL };

PRIM(access) {
	int c, *p, perm = 0, type = 0, estatus;

	if (length(list) != 3)
		fail("$&access", "usage: access access-type file-type name");

	Ref(List *, result, NULL);
	Ref(char *, atype, getstr(list->term));
	Ref(char *, ftype, getstr(list->next->term));
	Ref(char *, name, getstr(list->next->next->term));

	for (c = 0; atype[c] != '\0'; c++) {
		if ((p = permtab[(unsigned char)atype[c]]) == NULL)
			fail("$&access", "bad access type '%c'", c);
		perm |= *p;
	}

	if (ftype[0] == '\0' || ftype[1] != '\0')
		fail("$&access", "file type argument must be one character");
	if ((p = typetab[(unsigned char)ftype[0]]) == NULL)
		fail("$&access", "bad file type argument '%c'", ftype[0]);
	type = *p;

	estatus = testfile(name, perm, type);
	result = mklist(mkstr(estatus == 0 ? "0" : esstrerror(estatus)), NULL);

	RefEnd3(name, ftype, atype);
	RefReturn(result);
}

#define FLAGTAB(type, c, def) \
	STMT(static int CONCAT(type,c) = def; \
	     CONCAT(type,tab)[(unsigned char)(STRING(c)[0])] = &CONCAT(type,c))

extern Dict *initprims_access(Dict *primdict) {
	FLAGTAB(perm, e, 0);
	FLAGTAB(perm, r, READ);
	FLAGTAB(perm, w, WRITE);
	FLAGTAB(perm, x, EXEC);

	FLAGTAB(type, a, 0);
	FLAGTAB(type, f, S_IFREG);
	FLAGTAB(type, d, S_IFDIR);
	FLAGTAB(type, c, S_IFCHR);
	FLAGTAB(type, b, S_IFBLK);
#ifdef S_IFLNK
	FLAGTAB(type, l, S_IFLNK);
#endif
#ifdef S_IFSOCK
	FLAGTAB(type, s, S_IFSOCK);
#endif
#ifdef S_IFIFO
	FLAGTAB(type, p, S_IFIFO);
#endif

	X(access);
	return primdict;
}

extern char *checkexecutable(char *file) {
	int err = testfile(file, EXEC, S_IFREG);
	return err == 0 ? NULL : esstrerror(err);
}
