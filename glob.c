/* glob.c -- wildcard matching */

#define	REQUIRE_STAT	1
#define	REQUIRE_DIRENT	1
#define	REQUIRE_PWD	1

#include "es.h"

char QUOTED[] = "QUOTED", UNQUOTED[] = "RAW";

/* hasmeta -- true iff some unquoted character is a meta character */
static Boolean hasmeta(const char *s, const char *q) {
	if (q == QUOTED)
		return FALSE;
	if (q == UNQUOTED) {
		if (*s == '~')
			return TRUE;
		for (;;) {
			int c = *s++;
			if (c == '\0')
				return FALSE;
			if (c == '*' || c == '?' || c == '[')
				return TRUE;
		}
	}
	if (*s == '~' && *q == 'r')
		return TRUE;
	for (;;) {
		int c = *s++, r = *q++;
		if (c == '\0')
			return FALSE;
		if ((c == '*' || c == '?' || c == '[') && (r == 'r'))
			return TRUE;
	}
}

/* dirmatch -- match a pattern against the contents of directory */
static List *dirmatch(const char *prefix, const char *dirname, const char *pattern, const char *quote) {
	List *list, **prevp;
	static DIR *dirp;
	static Dirent *dp;
	static struct stat s;

	if (!hasmeta(pattern, quote)) {
		char *name = str("%s%s", prefix, pattern);
		if (stat(name, &s) == -1)
			return NULL;
		return mklist(mkterm(name, NULL), NULL);
	}

	if (*pattern == '~' && *quote == 'r' && *prefix == '\0')
		if (pattern[1] == '\0')
			return listcopy(varlookup("home", NULL));
		else {
			struct passwd *pw = getpwnam(pattern + 1);
			if (pw != NULL)
				return mklist(mkterm(gcdup(pw->pw_dir), NULL), NULL);
		}

	assert(gcblocked > 0);

	/* opendir succeeds on regular files on some systems, so the stat() call is necessary (sigh) */
	if (stat(dirname, &s) == -1 || (s.st_mode & S_IFMT) != S_IFDIR || (dirp = opendir(dirname)) == NULL)
		return NULL;	
	for (list = NULL, prevp = &list; (dp = readdir(dirp)) != NULL;)
		if (match(dp->d_name, pattern, quote) && (*dp->d_name != '.' || *pattern == '.')) {
			List *lp = mklist(mkterm(str("%s%s", prefix, dp->d_name), NULL), NULL);
			*prevp = lp;
			prevp = &lp->next;
		}
	closedir(dirp);
	return list;
}

/* listglob -- glob a directory plus a filename pattern into a list of names */
static List *listglob(List *list, char *pattern, char *quote, size_t slashcount) {
	List *result, **prevp;
	
	for (result = NULL, prevp = &result; list != NULL; list = list->next) {
		const char *dir;
		size_t dirlen;
		static char *prefix = NULL;
		static size_t prefixlen = 0;

		assert(list->term != NULL);
		assert(list->term->str != NULL);
		
		dir = list->term->str;
		dirlen = strlen(dir);
		if (dirlen + slashcount + 1 >= prefixlen) {
			prefixlen = dirlen + slashcount + 1;
			prefix = erealloc(prefix, prefixlen);
		}
		memcpy(prefix, dir, dirlen);
		memset(prefix + dirlen, '/', slashcount);
		prefix[dirlen + slashcount] = '\0';

		*prevp = dirmatch(prefix, dir, pattern, quote);
		while (*prevp != NULL)
			prevp = &(*prevp)->next;
	}
	return result;
}

/* glob1 -- glob pattern path against the file system */
static List *glob1(const char *pattern, const char *quote) {
	const char *s, *q;
	char *d, *p, *qd, *qp;
	size_t psize;
	List *matched;

	static char *dir = NULL, *pat = NULL, *qdir = NULL, *qpat = NULL, *raw = NULL;
	static size_t dsize = 0;

	assert(quote != QUOTED);

	if ((psize = strlen(pattern) + 1) > dsize || pat == NULL) {
		pat = erealloc(pat, psize);
		raw = erealloc(raw, psize);
		dir = erealloc(dir, psize);
		qpat = erealloc(qpat, psize);
		qdir = erealloc(qdir, psize);
		dsize = psize;
		memset(raw, 'r', psize);
	}
	d = dir;
	qd = qdir;
	q = (quote == UNQUOTED) ? raw : quote;

	s = pattern;
	if (*s == '/')
		while (*s == '/')
			*d++ = *s++, *qd++ = *q++;
	else
		while (*s != '/' && *s != '\0')
			*d++ = *s++, *qd++ = *q++; /* get first directory component */
	*d = '\0';

	/*
	 * Special case: no slashes in the pattern, i.e., open the current directory.
	 * Remember that w cannot consist of slashes alone (the other way *s could be
	 * zero) since doglob gets called iff there's a metacharacter to be matched
	 */
	if (*s == '\0')
		return dirmatch("", ".", dir, qdir);

	matched = (*pattern == '/')
			? mklist(mkterm(dir, NULL), NULL)
			: dirmatch("", ".", dir, qdir);
	do {
		size_t slashcount;
		SIGCHK();
		for (slashcount = 0; *s == '/'; s++, q++)
			slashcount++; /* skip slashes */
		for (p = pat, qp = qpat; *s != '/' && *s != '\0';)
			*p++ = *s++, *qp++ = *q++; /* get pat */
		*p = '\0';
		matched = listglob(matched, pat, qpat, slashcount);
	} while (*s != '\0' && matched != NULL);

	if (matched == NULL && *pattern == '~' && (quote == UNQUOTED || *quote == 'r')) {
		/* if we got here, we want to do a ~/file or ~user/file form, but no files match file */
		List *lp;
		s = strchr(pattern + 1, '/');
		assert(s != NULL);
		if (s == pattern + 1)
			matched = listcopy(varlookup("home", NULL));
		else {
			size_t len = s - pattern - 1;
			struct passwd *pw;
			memcpy(pat, pattern + 1, len);
			pat[len] = '\0';
			pw = getpwnam(pat);
			if (pw == NULL)
				return NULL;
			matched = mklist(mkterm(gcdup(pw->pw_dir), NULL), NULL);
		}
		for (lp = matched; lp != NULL; lp = lp->next) {
			assert(lp->term != NULL);
			lp->term = mkterm(str("%s%s", getstr(lp->term), s), NULL);
		}
	}

	return matched;
}

/* glob0 -- glob a list, (destructively) passing through entries we don't care about */
static List *glob0(List *list, StrList *quote) {
	List *result, **prevp, *expand1;
	
	for (result = NULL, prevp = &result; list != NULL; list = list->next, quote = quote->next)
		if (
			quote->str == QUOTED
			|| !hasmeta(list->term->str, quote->str)
			|| (expand1 = glob1(list->term->str, quote->str)) == NULL
		) {
			*prevp = list;
			prevp = &list->next;
		} else {
			*prevp = sortlist(expand1);
			while (*prevp != NULL)
				prevp = &(*prevp)->next;
		}
	return result;
}

/* glob -- globbing prepass (glob if we need to) */
extern List *glob(List *list, StrList *quote) {
	List *lp;
	StrList *qp;

	for (lp = list, qp = quote; lp != NULL; lp = lp->next, qp = qp->next)
		if (qp->str != QUOTED) {
			assert(lp->term != NULL && lp->term->str != NULL);
			assert(qp->str == UNQUOTED || strlen(qp->str) == strlen(lp->term->str));
			if (hasmeta(lp->term->str, qp->str)) {
				gcdisable(0);
				list = glob0(list, quote);
				Ref(List *, result, list);
				gcenable();
				RefReturn(result);
			}
		}
	return list;
}
