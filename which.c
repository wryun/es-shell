/* which.c -- path searching */

#define	REQUIRE_STAT	1
#define	REQUIRE_PARAM	1

#include "es.h"

#define	X_USR 0100
#define	X_GRP 0010
#define	X_OTH 0001
#define	X_ALL (X_USR|X_GRP|X_OTH)

static Boolean initialized = FALSE;
static int uid, gid;

#ifdef NGROUPS
static int ngroups, gidset[NGROUPS];

/* ingidset -- determine whether gid lies in gidset */
static int ingidset(int g) {
	int i;
	for (i = 0; i < ngroups; i++)
		if (g == gidset[i])
			return 1;
	return 0;
}
#endif

/* eaccess -- check if a file is executable */
static Boolean eaccess(char *path) {
	struct stat st;
	int mask;
	if (stat(path, &st) != 0)
		return FALSE;
	if (uid == 0)
		mask = X_ALL;
	else if (uid == st.st_uid)
		mask = X_USR;
#ifdef NGROUPS
	else if (gid == st.st_gid || ingidset(st.st_gid))
#else
	else if (gid == st.st_gid)
#endif
		mask = X_GRP;
	else
		mask = X_OTH;
	if (((st.st_mode & S_IFMT) == S_IFREG) && (st.st_mode & mask))
		return TRUE;
	errno = EACCES;
	return FALSE;
}

/* which0 -- return a full pathname by searching $path */
static char *which0(char *name) {
	static char *test = NULL;
	static size_t testlen = 0;
	int len;
	if (name == NULL)	/* no filename? can happen with "> foo" as a command */
		return NULL;
	if (!initialized) {
		initialized = TRUE;
		uid = geteuid();
		gid = getegid();
#ifdef NGROUPS
		ngroups = getgroups(NGROUPS, gidset);
#endif
	}
	if (isabsolute(name))
		if (eaccess(name))
			return name;
		else {
			gcenable();
			fail("%s: %s", name, strerror(errno));
		}
	len = strlen(name);
	Ref(List *, path, varlookup("path", NULL));
	for (; path != NULL; path = path->next) {
		char *dir = getstr(path->term);
		size_t need = strlen(dir) + len + 2; /* one for null terminator, one for the '/' */
		if (testlen < need) {
			if (test != NULL)
				efree(test);
			test = ealloc(testlen = need);
		}
		if (*dir == '\0')
			strcpy(test, name);
		else {
			strcpy(test, dir);
			if (!streq(test, "/")) /* "//" is special to POSIX */
				strcat(test, "/");
			strcat(test, name);
		}
		if (eaccess(test)) {
			RefPop(path);
			return gcdup(test);
		}
	}
	RefEnd(path);
	gcenable();
	fail("%s not found", name);
	return NULL;
}

/* which -- wrapper for which0 */
extern char *which(char *name) {
	char *result;
	gcdisable(0);
	result = which0(name);
	gcenable();
	return result;
}
