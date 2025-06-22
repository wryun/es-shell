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
		ngroups = getgroups(0, NULL);
		if (ngroups == -1)
			fail("$&access", "getgroups: %s", esstrerror(errno));
		gidset = ealloc(ngroups * sizeof(gidset_t));
		assert(getgroups(ngroups, gidset) != -1);
		initialized = TRUE;
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
	FLAGTAB(type, f, IFREG);
	FLAGTAB(type, d, IFDIR);
	FLAGTAB(type, c, IFCHR);
	FLAGTAB(type, b, IFBLK);
	FLAGTAB(type, l, IFLNK);
	FLAGTAB(type, s, IFSOCK);
	FLAGTAB(type, p, IFIFO);

	X(access);
	return primdict;
}

extern char *checkexecutable(char *file) {
	int err = testfile(file, EXEC, IFREG);
	return err == 0 ? NULL : esstrerror(err);
}
