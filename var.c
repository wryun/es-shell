/* var.c -- es variables */

#include "es.h"
#include "gc.h"
#include <ctype.h>

typedef struct Var Var;
struct Var {
	List *defn;
	char *env;
	Var *next;
};

Dict *vars;
static Vector *env;
static int envcount, envmin;
static Tag VarTag;
static Boolean isdirty;
static char notexported;

static Var *mkvar(List *defn, char *env, Var *next) {
	Ref(Var *, var, NULL);
	Ref(List *, lp, defn);
	Ref(char *, ep, env);
	Ref(Var *, np, next);
	var = gcnew(Var);
	var->defn = lp;
	var->env = ep;
	var->next = np;
	RefEnd3(np, ep, lp);
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
	var->next = forward(var->next);
	var->env = forward(var->env);
	return sizeof (Var);
}

static DefineTag(Var);


/*
 * public entry points
 */

/* varname -- validate a variable name */
extern char *varname(List *list) {
	char *var;
	if (list == NULL)
		fail("null variable name");
	if (list->next != NULL)
		fail("multi-word variable name");
	var = getstr(list->term);
	assert(var != NULL);
	if (*var == '\0')
		fail("zero-length variable name");
	if (strchr(var, '=') != NULL)
		fail("'=' in variable name");
	return var;
}

/* iscounting -- is it a counter number, i.e., an integer > 0 */
static Boolean iscounting(char *name) {
	int c;
	const char *s = name;
	while ((c = *s++) != '\0')
		if (!isdigit(c))
			return FALSE;
	if (streq(name, "0"))
		return FALSE;
	return TRUE;
}

extern List *varlookup(char *name, Binding *bp) {
	Var *var;

	if (iscounting(name)) {
		Term *term = nth(varlookup("*", bp), strtol(name, NULL, 10));
		if (term == NULL)
			return NULL;
		return mklist(term, NULL);
	}

	for (; bp != NULL; bp = bp->next)
		if (streq(name, bp->name))
			return bp->defn;

	var = dictget(vars, name);
	if (var == NULL)
		return NULL;
	return var->defn;
}

extern List *varlookup2(char *name1, char *name2) {
	Var *var = dictget2(vars, name1, name2);
	if (var == NULL)
		return NULL;
	return var->defn;
}

extern void vardef(char *name, Binding *bp, List *defn) {
	Var *var;
	List *setfunc;
	for (; bp != NULL; bp = bp->next)
		if (streq(name, bp->name)) {
			bp->defn = defn;
			return;
		}

	if ((setfunc = varlookup2("set-", name)) != NULL) {
		List *e;
		Handler h;

		if ((e = pushhandler(&h)) != NULL) {
			varpop("0");
			throw(e);
		}

		Ref(char *, np, name);
		Ref(List *, dp, defn);
		Ref(List *, sf, setfunc);
		varpush("0", mklist(mkterm(np, NULL), NULL));
		defn = eval(append(sf, dp), NULL, TRUE);
		name = np;
		RefEnd3(sf, dp, np);
		pophandler(&h);
	}

	isdirty = TRUE;
	var = dictget(vars, name);
	if (var != NULL)
		if (defn != NULL || var->next != NULL) {
			var->defn = defn;
			var->env = NULL;
		} else
			vars = dictput(vars, name, NULL);
	else if (defn != NULL) {
		Ref(char *, np, name);
		Ref(Var *, vp, mkvar(defn, NULL, NULL));
		vars = dictput(vars, np, vp);
		RefEnd2(vp, np);
	}
}

extern void varpush(char *np, List *lp) {
	List *setfunc;
	Ref(char *, name, np);
	Ref(List *, defn, lp);
	Ref(Var *, var, dictget(vars, name));

	if (!streq(np, "0") && (setfunc = varlookup2("set-", name)) != NULL) {
		List *e;
		Handler h;

		if ((e = pushhandler(&h)) != NULL) {
			varpop("0");
			throw(e);
		}

		Ref(List *, sf, setfunc);
		varpush("0", mklist(mkterm(name, NULL), NULL));
		defn = eval(append(sf, defn), NULL, TRUE);
		RefEnd(sf);
		pophandler(&h);
	}

	isdirty = TRUE;
	if (var == NULL) {
		var = mkvar(defn, NULL, NULL);
		vars = dictput(vars, name, var);
	} else {
		var->next = mkvar(var->defn, var->env, var->next);
		var->defn = defn;
		var->env = NULL;
	}

	RefEnd3(var, defn, name);
}

extern void varpop(char *name) {
	List *setfunc;

	isdirty = TRUE;
	Ref(Var *, var, dictget(vars, name));
	if (var == NULL) {
		RefPop(var);
		return;
	}
	Ref(Var *, next, var->next);

	if (!streq(name, "0") && (setfunc = varlookup2("set-", name)) != NULL) {
		List *e;
		Handler h;

		if ((e = pushhandler(&h)) != NULL) {
			varpop("0");
			throw(e);
		}

		Ref(char *, np, name);
		Ref(List *, sf, setfunc);
		varpush("0", mklist(mkterm(np, NULL), NULL));
		Ref(List *, defn, eval(append(sf, next == NULL ? NULL : next->defn), NULL, TRUE));
		if (defn == NULL)
			next = NULL;
		else if (next == NULL)
			next = mkvar(defn, NULL, NULL);
		else
			next->defn = defn;
		RefEnd(defn);
		name = np;
		RefEnd2(sf, np);
		pophandler(&h);
	}

	if (next == NULL)
		vars = dictput(vars, name, NULL);
	else
		*var = *next;
	RefEnd2(next, var);
}

static void mkenv0(void *dummy, char *key, void *value) {
	if (value == NULL || ((Var *) value)->env == &notexported)
		return;
	Ref(Var *, var, value);
	if (var->env == NULL) {
		char *envstr = str("%F=%L", key, var->defn, "\001");
		var->env = envstr;
	}
	assert(envcount < env->len);
	env->vector[envcount++] = var->env;
	RefEnd(var);
	if (envcount == env->len) {
		Vector *newenv = mkvector(env->len * 2);
		memcpy(&newenv->vector[0], &env->vector[0], envcount * sizeof *env->vector);
		env = newenv;
	}
}
	
extern Vector *mkenv(void) {
	envcount = envmin;
	dictforall(vars, mkenv0, NULL);
	env->vector[envcount] = NULL;
	return env;
}

static void hide(void *dummy, char *key, void *value) {
	((Var *) value)->env = &notexported;
}

static char *findleftsep(char *p) {
	do
		--p;
	while (*p != '\0' && *p != '\001');
	return p;
}

static List *mklistfromenv(char *envval) {
	char *endp, *realend = strchr(envval, '\0');
	char *sepp = findleftsep(realend);
	Ref(List *, tailp, NULL);

	endp = realend;

	while (sepp >= envval) {
		tailp = mklist(mkterm(gcdup(sepp+1), NULL), tailp);
		if (endp != realend)
			*endp = '\1';
		endp = sepp;
		sepp = findleftsep(sepp);
		*endp = '\0';
	}
	tailp = mklist(mkterm(gcdup(envval), NULL), tailp);
	if (endp != realend)
		*endp = '\1';
	RefReturn(tailp);
}

/* noexport -- make a variable as unexported */
extern void noexport(char *name) {
	Var *var;
	Ref(char *, s, name);
	var = dictget(vars, s);
	if (var == NULL)
		fail("noexport: %s is undefined", s);
	isdirty = TRUE;
	var->env = &notexported;
	RefEnd(s);
	
}

static void initpath(void) {
	int i;
	static const char * const path[] = { INITIAL_PATH };
	
	Ref(List *, list, NULL);
	for (i = sizeof path / sizeof path[0]; i-- > 0;)
		list = mklist(mkterm((char *) path[i], NULL), list);
	vardef("path", NULL, list);
	RefEnd(list);
}

static void initpid(void) {
	vardef("pid", NULL, mklist(mkterm(str("%d", getpid()), NULL), NULL));
}

#define	hasprefix(s, p)	strneq(s, p, (sizeof p) - 1)

extern void initvars(char **envp, const char *initial, Boolean protected) {
	char *envstr;
	Boolean save_printcmds = printcmds;
	Boolean save_noexecute = noexecute;

	printcmds = FALSE;
	noexecute = FALSE;

	globalroot(&vars);
	globalroot(&env);
	vars = mkdict();
	env = mkvector(10);
	envcount = 0;

	runstring(initial);
	initpath();
	initpid();
	dictforall(vars, hide, NULL);

	for (; (envstr = *envp) != NULL; envp++) {
		char *eq = strchr(envstr, '=');
		if (eq == NULL) {
			env->vector[envcount++] = envstr;
			if (envcount == env->len) {
				Vector *newenv = mkvector(env->len * 2);
				memcpy(&newenv->vector[0], &env->vector[0], envcount * sizeof (char *));
				env = newenv;
			}
			continue;
		}
		*eq = '\0';
		Ref(char *, name, str("%N", envstr));
		if (!protected || (!hasprefix(name, "fn-") && !hasprefix(name, "set-"))) {
			Ref(List *, defn, mklistfromenv(eq + 1)); /* needs *eq == '\0' */
			*eq = '=';
			vardef(name, NULL, defn);
			RefEnd(defn);
		}
		RefEnd(name);
	}

	envmin = envcount;
	printcmds = save_printcmds;
	noexecute = save_noexecute;
}
