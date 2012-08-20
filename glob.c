/* glob.c -- wildcard matching ($Revision: 1.1.1.1 $) */

#define	REQUIRE_STAT	1
#define	REQUIRE_DIRENT	1

#include "es.h"
#include "gc.h"

char QUOTED[] = "QUOTED", UNQUOTED[] = "RAW";

/* hastilde -- true iff the first character is a ~ and it is not quoted */
static Boolean hastilde(const char *s, const char *q) {
	return *s == '~' && (q == UNQUOTED || *q == 'r');
}

/* haswild -- true iff some unquoted character is a wildcard character */
extern Boolean haswild(const char *s, const char *q) {
	if (q == QUOTED)
		return FALSE;
	if (q == UNQUOTED)
		for (;;) {
			int c = *s++;
			if (c == '\0')
				return FALSE;
			if (c == '*' || c == '?' || c == '[')
				return TRUE;
		}
	for (;;) {
		int c = *s++, r = *q++;
		if (c == '\0')
			return FALSE;
		if ((c == '*' || c == '?' || c == '[') && (r == 'r'))
			return TRUE;
	}
}

/* ishiddenfile -- return true if the file is a dot file to be hidden */
static int ishiddenfile(const char *s) {
#if SHOW_DOT_FILES
	return *s == '.' && (!s[1] || (s[1] == '.' && !s[2]));
#else
	return *s == '.';
#endif
}

/* dirmatch -- match a pattern against the contents of directory */
static List *dirmatch(const char *prefix, const char *dirname, const char *pattern, const char *quote) {
	List *list, **prevp;
	static DIR *dirp;
	static Dirent *dp;
	static struct stat s;

	/*
	 * opendir succeeds on regular files on some systems, so the stat call
	 * is necessary (sigh);  the check is done here instead of with the
	 * opendir to handle a trailing slash.
	 */
	if (stat(dirname, &s) == -1 || (s.st_mode & S_IFMT) != S_IFDIR)
		return NULL;	

	if (!haswild(pattern, quote)) {
		char *name = str("%s%s", prefix, pattern);
		if (lstat(name, &s) == -1)
			return NULL;
		return mklist(mkstr(name), NULL);
	}

	assert(gcisblocked());

	dirp = opendir(dirname);
	if (dirp == NULL)
		return NULL;	
	for (list = NULL, prevp = &list; (dp = readdir(dirp)) != NULL;)
		if (match(dp->d_name, pattern, quote)
		    && (!ishiddenfile(dp->d_name) || *pattern == '.')) {
			List *lp = mklist(mkstr(str("%s%s",
						    prefix, dp->d_name)),
					  NULL);
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
		assert(!isclosure(list->term));
		
		dir = getstr(list->term);
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
			? mklist(mkstr(dir), NULL)
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

	return matched;
}

/* glob0 -- glob a list, (destructively) passing through entries we don't care about */
static List *glob0(List *list, StrList *quote) {
	List *result, **prevp, *expand1;
	
	for (result = NULL, prevp = &result; list != NULL; list = list->next, quote = quote->next) {
		char *str;
		if (
			quote->str == QUOTED
			|| !haswild(str = getstr(list->term), quote->str)
			|| (expand1 = glob1(str, quote->str)) == NULL
		) {
			*prevp = list;
			prevp = &list->next;
		} else {
			*prevp = sortlist(expand1);
			while (*prevp != NULL)
				prevp = &(*prevp)->next;
		}
	}
	return result;
}

/* expandhome -- do tilde expansion by calling fn %home */
static char *expandhome(char *s, StrList *qp) {
	int c;
	size_t slash;
	List *fn = varlookup("fn-%home", NULL);

	assert(*s == '~');
	assert(qp->str == UNQUOTED || *qp->str == 'r');

	if (fn == NULL)
		return s;

	for (slash = 1; (c = s[slash]) != '/' && c != '\0'; slash++)
		;

	Ref(char *, string, s);
	Ref(StrList *, quote, qp);
	Ref(List *, list, NULL);
	RefAdd(fn);
	if (slash > 1)
		list = mklist(mkstr(gcndup(s + 1, slash - 1)), NULL);
	RefRemove(fn);

	list = eval(append(fn, list), NULL, 0);

	if (list != NULL) {
		if (list->next != NULL)
			fail("es:expandhome", "%%home returned more than one value");
		Ref(char *, home, getstr(list->term));
		if (c == '\0') {
			string = home;
			quote->str = QUOTED;
		} else {
			char *q;
			size_t pathlen = strlen(string);
			size_t homelen = strlen(home);
			size_t len = pathlen - slash + homelen;
			s = gcalloc(len + 1, &StringTag);
			memcpy(s, home, homelen);
			memcpy(&s[homelen], &string[slash], pathlen - slash);
			s[len] = '\0';
			string = s;
			q = quote->str;
			if (q == UNQUOTED) {
				q = gcalloc(len + 1, &StringTag);
				memset(q, 'q', homelen);
				memset(&q[homelen], 'r', pathlen - slash);
				q[len] = '\0';
			} else if (strchr(q, 'r') == NULL)
				q = QUOTED;
			else {
				q = gcalloc(len + 1, &StringTag);
				memset(q, 'q', homelen);
				memcpy(&q[homelen], &quote->str[slash], pathlen - slash);
				q[len] = '\0';
			}
			quote->str = q;
		}
		RefEnd(home);
	}
	RefEnd2(list, quote);
	RefReturn(string);
}

/* glob -- globbing prepass (glob if we need to, and dispatch for tilde expansion) */
extern List *glob(List *list, StrList *quote) {
	List *lp;
	StrList *qp;
	Boolean doglobbing = FALSE;

	for (lp = list, qp = quote; lp != NULL; lp = lp->next, qp = qp->next)
		if (qp->str != QUOTED) {
			assert(lp->term != NULL);
			assert(!isclosure(lp->term));
			Ref(char *, str, getstr(lp->term));
			assert(qp->str == UNQUOTED || strlen(qp->str) == strlen(str));
			if (hastilde(str, qp->str)) {
				Ref(List *, l0, list);
				Ref(List *, lr, lp);
				Ref(StrList *, q0, quote);
				Ref(StrList *, qr, qp);
				str = expandhome(str, qp);
				lr->term = mkstr(str);
				lp = lr;
				qp = qr;
				list = l0;
				quote = q0;
				RefEnd4(qr, q0, lr, l0);
			}
			if (haswild(str, qp->str))
				doglobbing = TRUE;
			RefEnd(str);
		}

	if (!doglobbing)
		return list;
	gcdisable();
	list = glob0(list, quote);
	Ref(List *, result, list);
	gcenable();
	RefReturn(result);
}
