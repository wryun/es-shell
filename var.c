/* var.c -- es variables ($Revision: 1.1.1.1 $) */
/* stdgetenv is based on the FreeBSD getenv */

#include "es.h"
#include "gc.h"
#include "var.h"
#include "term.h"

#if PROTECT_ENV
#define	ENV_FORMAT	"%F=%W"
#define	ENV_DECODE	"%N"
#else
#define	ENV_FORMAT	"%s=%W"
#define	ENV_DECODE	"%s"
#endif

#define	ENVSIZE	40

#define VECPUSH(vec, elt) STMT( \
	(vec)->vector[(vec)->count++] = (elt); \
	if ((vec)->count == (vec)->alloclen) { \
		Vector *CONCAT(new_,vec) = mkvector((vec)->alloclen * 2); \
		CONCAT(new_,vec)->count = (vec)->count; \
		memcpy(CONCAT(new_,vec)->vector, (vec)->vector, \
			(vec)->count * sizeof *(vec)->vector); \
		(vec) = CONCAT(new_,vec); \
	} \
)

Dict *vars = NULL;
static Dict *noexport;
static Vector *env, *sortenv;
static int envmin;
static Boolean isdirty = TRUE;
static Boolean rebound = TRUE;

DefineTag(Var, static);

static Boolean specialvar(const char *name) {
	return (*name == '*' || *name == '0') && name[1] == '\0';
}

static Boolean hasbindings(List *list) {
	for (; list != NULL; list = list->next)
		if (isclosure(list->term)) {
			Closure *closure = getclosure(list->term);
			assert(closure != NULL);
			if (closure->binding != NULL)
				return TRUE;
		}
	return FALSE;
}

static Var *mkvar(List *defn) {
	Ref(Var *, var, NULL);
	Ref(List *, lp, defn);
	var = gcnew(Var);
	var->env = NULL;
	var->defn = lp;
	var->flags = hasbindings(lp) ? var_hasbindings : 0;
	RefEnd(lp);
	RefReturn(var);
}

static void *VarCopy(void *op) {
	void *np = gcnew(Var);
	memcpy(np, op, sizeof (Var));
	return np;
}

static size_t VarScan(void *p) {
	Var *var = p;
	var->defn = forward(var->defn);
	var->env = ((var->flags & var_hasbindings) && rebound) ? NULL : forward(var->env);
	return sizeof (Var);
}

/* iscounting -- is it a counter number, i.e., an integer > 0 */
static Boolean iscounting(const char *name) {
	while (*name == '0')
		name++;
	if (*name == '\0')
		return FALSE;
	for (; *name != '\0'; name++)
		if (!isdigit(*name))
			return FALSE;
	return TRUE;
}


/*
 * public entry points
 */

/* validatevar -- ensure that a variable name is valid */
extern void validatevar(const char *var) {
	if (*var == '\0')
		fail("es:var", "zero-length variable name");
	if (iscounting(var))
		fail("es:var", "illegal variable name: %S", var);
#if !PROTECT_ENV
	if (strchr(var, '=') != NULL)
		fail("es:var", "'=' in variable name: %S", var);
#endif
}

/* isexported -- is a variable exported? */
static Boolean isexported(const char *name) {
	if (specialvar(name))
		return FALSE;
	if (noexport == NULL)
		return TRUE;
	return dictget(noexport, name) == NULL;
}

/* setnoexport -- mark a list of variable names not for export */
extern void setnoexport(List *list) {
	static char noexportchar = '!';

	isdirty = TRUE;
	if (list == NULL) {
		noexport = NULL;
		return;
	}
	gcdisable();
	for (noexport = mkdict(); list != NULL; list = list->next)
		noexport = dictput(noexport, getstr(list->term), &noexportchar);
	gcenable();
}

/* varlookup -- lookup a variable in the current context */
extern List *varlookup(const char *name, Binding *bp) {
	Var *var;

	if (iscounting(name)) {
		Term *term = nth(varlookup("*", bp), strtol(name, NULL, 10));
		if (term == NULL)
			return NULL;
		return mklist(term, NULL);
	}

	validatevar(name);
	for (; bp != NULL; bp = bp->next)
		if (streq(name, bp->name))
			return bp->defn;

	var = dictget(vars, name);
	if (var == NULL)
		return NULL;
	return var->defn;
}

extern List *varlookup2(char *name1, char *name2, Binding *bp) {
	Var *var;
	
	for (; bp != NULL; bp = bp->next)
		if (streq2(bp->name, name1, name2))
			return bp->defn;

	var = dictget2(vars, name1, name2);
	if (var == NULL)
		return NULL;
	return var->defn;
}

static List *callsettor(char *name, List *defn) {
	Push p;
	List *settor;

	if (specialvar(name) || (settor = varlookup2("set-", name, NULL)) == NULL)
		return defn;

	Ref(List *, lp, defn);
	Ref(List *, fn, settor);
	varpush(&p, "0", mklist(mkstr(name), NULL));

	lp = listcopy(eval(append(fn, lp), NULL, 0));

	varpop(&p);
	RefEnd(fn);
	RefReturn(lp);
}

static void vardef0(char *name, Binding *binding, List *defn, Boolean startup) {
	Var *var;

	validatevar(name);
	for (; binding != NULL; binding = binding->next)
		if (streq(name, binding->name)) {
			binding->defn = defn;
			rebound = TRUE;
			return;
		}

	RefAdd(name);
	if (!startup) {
		defn = callsettor(name, defn);
		if (isexported(name))
			isdirty = TRUE;
	}

	var = dictget(vars, name);
	if (var != NULL)
		if (defn != NULL) {
			var->defn = defn;
			var->env = NULL;
			var->flags = hasbindings(defn) ? var_hasbindings : 0;
		} else
			vars = dictput(vars, name, NULL);
	else if (defn != NULL) {
		var = mkvar(defn);
		vars = dictput(vars, name, var);
	}
	RefRemove(name);
}

extern void vardef(char *name, Binding *binding, List *defn) {
	vardef0(name, binding, defn, FALSE);
}

extern void varpush(Push *push, char *name, List *defn) {
	Var *var;

	validatevar(name);
	push->name = name;
	push->nameroot.next = rootlist;
	push->nameroot.p = (void **) &push->name;
	rootlist = &push->nameroot;

	if (isexported(name))
		isdirty = TRUE;
	defn = callsettor(name, defn);

	var = dictget(vars, push->name);
	if (var == NULL) {
		push->defn	= NULL;
		push->flags	= 0;
		var		= mkvar(defn);
		vars		= dictput(vars, push->name, var);
	} else {
		push->defn	= var->defn;
		push->flags	= var->flags;
		var->defn	= defn;
		var->env	= NULL;
		var->flags	= hasbindings(defn) ? var_hasbindings : 0;
	}

	push->next = pushlist;
	pushlist = push;

	push->defnroot.next = rootlist;
	push->defnroot.p = (void **) &push->defn;
	rootlist = &push->defnroot;
}

extern void varpop(Push *push) {
	Var *var;
	List *volatile except = NULL;

	assert(pushlist == push);
	assert(rootlist == &push->defnroot);
	assert(rootlist->next == &push->nameroot);

	if (isexported(push->name))
		isdirty = TRUE;

	ExceptionHandler

		push->defn = callsettor(push->name, push->defn);

	CatchException (e)

		except = e;

	EndExceptionHandler;

	var = dictget(vars, push->name);

	if (var != NULL)
		if (push->defn != NULL) {
			var->defn = push->defn;
			var->flags = push->flags;
			var->env = NULL;
		} else
			vars = dictput(vars, push->name, NULL);
	else if (push->defn != NULL) {
		var = mkvar(NULL);
		var->defn = push->defn;
		var->flags = push->flags;
		vars = dictput(vars, push->name, var);
	}

	pushlist = pushlist->next;
	rootlist = rootlist->next->next;

	if (except)
		throw(except);
}

static void mkenv0(void UNUSED *dummy, char *key, void *value) {
	Var *var = value;
	assert(gcisblocked());
	if (
		   var == NULL
		|| var->defn == NULL
		|| (var->flags & var_isinternal)
		|| !isexported(key)
	)
		return;
	if (var->env == NULL || (rebound && (var->flags & var_hasbindings))) {
		char *envstr = str(ENV_FORMAT, key, var->defn);
		var->env = envstr;
	}
	assert(env->count < env->alloclen);
	VECPUSH(env, var->env);
}
	
extern Vector *mkenv(void) {
	if (isdirty || rebound) {
		env->count = envmin;
		gcdisable();		/* TODO: make this a good guess */
		dictforall(vars, mkenv0, NULL);
		gcenable();
		env->vector[env->count] = NULL;
		isdirty = FALSE;
		rebound = FALSE;
		if (sortenv == NULL || env->count > sortenv->alloclen)
			sortenv = mkvector(env->count * 2);
		sortenv->count = env->count;
		memcpy(sortenv->vector, env->vector, sizeof (char *) * (env->count + 1));
		sortvector(sortenv);
	}
	return sortenv;
}

/* addtolist -- dictforall procedure to create a list */
extern void addtolist(void *arg, char *key, void UNUSED *value) {
	List **listp = arg;
	Term *term = mkstr(key);
	*listp = mklist(term, *listp);
}

static void listexternal(void *arg, char *key, void *value) {
	if ((((Var *) value)->flags & var_isinternal) == 0 && !specialvar(key))
		addtolist(arg, key, value);
}

static void listinternal(void *arg, char *key, void *value) {
	if (((Var *) value)->flags & var_isinternal)
		addtolist(arg, key, value);
}

static char *list_prefix;

static void listwithprefix(void *arg, char *key, void *value) {
	if (strneq(key, list_prefix, strlen(list_prefix)))
		addtolist(arg, key, value);
}

/* listvars -- return a list of all the (dynamic) variables */
extern List *listvars(Boolean internal) {
	Ref(List *, varlist, NULL);
	dictforall(vars, internal ? listinternal : listexternal, &varlist);
	varlist = sortlist(varlist);
	RefReturn(varlist);
}

/* varswithprefix -- return a list of all the (dynamic) variables
 * matching the given prefix */
extern List *varswithprefix(char *prefix) {
	Ref(List *, varlist, NULL);
	list_prefix = prefix;
	dictforall(vars, listwithprefix, &varlist);
	RefReturn(varlist);
}

/* hide -- worker function for dictforall to hide initial state */
static void hide(void UNUSED *dummy, char UNUSED *key, void *value) {
	((Var *) value)->flags |= var_isinternal;
}

/* hidevariables -- mark all variables as internal */
extern void hidevariables(void) {
	dictforall(vars, hide, NULL);
}

/* initvars -- initialize the variable machinery */
extern void initvars(void) {
	globalroot(&vars);
	globalroot(&noexport);
	globalroot(&env);
	globalroot(&sortenv);
	vars = mkdict();
	noexport = NULL;
	env = mkvector(ENVSIZE);
}

/* importvar -- import a single environment variable */
static void importvar(char *name0, char *value) {
	char sep[2] = { ENV_SEPARATOR, '\0' };

	Ref(char *, name, name0);
	Ref(List *, defn, NULL);
	defn = fsplit(sep, mklist(mkstr(value), NULL), FALSE);

	if (strchr(value, ENV_ESCAPE) != NULL) {
		List *list;
		gcdisable();
		for (list = defn; list != NULL; list = list->next) {
			int offset = 0;
			const char *word = list->term->str;
			const char *escape;
			while ((escape = strchr(word + offset, ENV_ESCAPE))
			       != NULL) {
				offset = escape - word + 1;
				switch (escape[1]) {
				    case '\0':
					if (list->next != NULL) {
						const char *str2
						  = list->next->term->str;
						char *str
						  = gcalloc(offset
							    + strlen(str2) + 1,
							    &StringTag);
						memcpy(str, word, offset - 1);
						str[offset - 1]
						  = ENV_SEPARATOR;
						strcpy(str + offset, str2);
						list->term->str = str;
						list->next = list->next->next;
					}
					break;
				    case ENV_ESCAPE: {
					char *str
					  = gcalloc(strlen(word), &StringTag);
					memcpy(str, word, offset);
					strcpy(str + offset, escape + 2);
					list->term->str = str;
					offset += 1;
					break;
				    }
				}
			}
		}
		gcenable();
	}
	vardef0(name, NULL, defn, TRUE);
	RefEnd2(defn, name);
}

#if LOCAL_GETENV
static char *stdgetenv(const char *);
static char *esgetenv(const char *);
static char *(*realgetenv)(const char *) = stdgetenv;

/* esgetenv -- fake version of getenv for readline (or other libraries) */
static char *esgetenv(const char *name) {
	List *value = varlookup(name, NULL);
	if (value == NULL)
		return NULL;
	else {
		char *export;
		static Dict *envdict;
		static Boolean initialized = FALSE;
		Ref(char *, string, NULL);

		gcdisable();
		if (!initialized) {
			initialized = TRUE;
			envdict = mkdict();
			globalroot(&envdict);
		}

		string = dictget(envdict, name);
		if (string != NULL)
			efree(string);

		export = str("%W", value);
		string = ealloc(strlen(export) + 1);
		strcpy(string, export);
		envdict = dictput(envdict, (char *) name, string);

		gcenable();
		RefReturn(string);
	}
}

static char *stdgetenv(const char *name) {
	extern char **environ;
	register int len;
	register const char *np;
	register char **p, *c;

	if (name == NULL || environ == NULL)
		return (NULL);
	for (np = name; *np && *np != '='; ++np)
		continue;
	len = np - name;
	for (p = environ; (c = *p) != NULL; ++p)
		if (strncmp(c, name, len) == 0 && c[len] == '=') {
			return (c + len + 1);
		}
	return (NULL);
}

char *getenv(const char *name) {
	return realgetenv(name);
}

extern int setenv(const char *name, const char *value, int overwrite) {
	assert(vars != NULL);
	if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
		errno = EINVAL;
		return -1;
	}
	Ref(char *, envname, str(ENV_DECODE, name));
	if (overwrite || varlookup(envname, NULL) == NULL)
		importvar(envname, (char *)value);
	RefEnd(envname);
	return 0;
}

extern int unsetenv(const char *name) {
	assert(vars != NULL);
	if (name[0] == '\0' || strchr(name, '=') != NULL) {
		errno = EINVAL;
		return -1;
	}
	vardef0(str(ENV_DECODE, name), NULL, NULL, TRUE);
	return 0;
}

extern int putenv(char *envstr) {
	size_t n = strcspn(envstr, "=");
	char *envname;
	int status;
	assert(vars != NULL);
	if (n == 0 || envstr[n] != '=') {
		/* null variable name or missing '=' char */
		errno = EINVAL;
		return -1;
	}
	envname = ealloc(n+1);
	memcpy(envname, envstr, n);
	envname[n] = '\0';
	status = setenv(envname, envstr + n + 1, 1);
	efree(envname);
	return status;
}
#endif

extern char **environ;

/* importenv -- load variables from the environment */
extern void importenv(Boolean funcs) {
	int i;
	char *envstr;
	size_t bufsize = 1024;
	char *buf = ealloc(bufsize);
	char **envp = environ;

	Ref(Vector *, imported, mkvector(ENVSIZE));
	Ref(char *, name, NULL);
	for (; (envstr = *envp) != NULL; envp++) {
		size_t nlen;
		char *eq = strchr(envstr, '=');
		if (eq == NULL) {
			VECPUSH(env, envstr);
			continue;
		}
		for (nlen = eq - envstr; nlen >= bufsize; bufsize *= 2)
			buf = erealloc(buf, bufsize);
		memcpy(buf, envstr, nlen);
		buf[nlen] = '\0';
		name = str(ENV_DECODE, buf);
		if (funcs == (hasprefix(name, "fn-") || hasprefix(name, "set-"))) {
			importvar(name, eq+1);
			VECPUSH(imported, name);
		}
	}
	RefEnd(name);

	sortvector(imported);
	Ref(Var *, var, NULL);
	for (i = 0; i < imported->count; i++) {
		char *name = imported->vector[i];
		List *defn;
		if (specialvar(name) || varlookup2("set-", name, NULL) == NULL)
			continue;
		var = dictget(vars, name);
		defn = callsettor(name, var->defn);
		var->defn = defn;
	}

	RefEnd2(var, imported);
	envmin = env->count;
	efree(buf);

#if LOCAL_GETENV
	realgetenv = esgetenv;
#endif
}
