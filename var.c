/* var.c -- es variables */

#define	REQUIRE_CTYPE	1

#include "es.h"
#include "gc.h"

extern const char initial[];	/* from initial.c, generated from initial.es */

typedef struct Var Var;
struct Var {
	List *defn;
	char *env;
	Var *next;
	Boolean binder;
};

Dict *vars;
static Vector *env, *sortenv;
static int envmin;
static Boolean isdirty = TRUE;
static Boolean rebound = TRUE;
static char notexported;

DefineTag(Var, static);

static Boolean specialvar(const char *name) {
	return (*name == '*' || *name == '0') && name[1] == '\0';
}

static Boolean hasbindings(List *list) {
	for (; list != NULL; list = list->next) {
		Closure *closure = list->term->closure;
		if (closure != NULL && closure->binding != NULL)
			return TRUE;
	}
	return FALSE;
}

static Var *mkvar(List *defn, char *env, Var *next) {
	Ref(Var *, var, NULL);
	Ref(List *, lp, defn);
	Ref(char *, ep, env);
	Ref(Var *, np, next);
	var = gcnew(Var);
	var->defn = lp;
	var->env = ep;
	var->next = np;
	var->binder = hasbindings(lp);
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
	var->env = (var->binder && rebound) ? NULL : forward(var->env);
	return sizeof (Var);
}


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
static Boolean iscounting(const char *name) {
	int c;
	const char *s = name;
	while ((c = *s++) != '\0')
		if (!isdigit(c))
			return FALSE;
	if (streq(name, "0"))
		return FALSE;
	return TRUE;
}

extern List *varlookup(const char *name, Binding *bp) {
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
			rebound = TRUE;
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
		defn = eval(append(sf, dp), NULL, TRUE, FALSE);
		name = np;
		RefEnd3(sf, dp, np);
		pophandler(&h);
	}

	if (!specialvar(name))
		isdirty = TRUE;
	var = dictget(vars, name);
	if (var != NULL)
		if (defn != NULL || var->next != NULL) {
			var->defn = defn;
			var->env = specialvar(name) ? &notexported : NULL;
		} else
			vars = dictput(vars, name, NULL);
	else if (defn != NULL) {
		Ref(char *, np, name);
		Ref(Var *, vp, mkvar(defn, specialvar(np) ? &notexported : NULL, NULL));
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
		defn = eval(append(sf, defn), NULL, TRUE, FALSE);
		RefEnd(sf);
		pophandler(&h);
	}

	if (!specialvar(name))
		isdirty = TRUE;
	if (var == NULL) {
		var = mkvar(defn, specialvar(name) ? &notexported : NULL, NULL);
		vars = dictput(vars, name, var);
	} else {
		var->next = mkvar(var->defn, var->env, var->next);
		var->defn = defn;
		var->env = specialvar(name) ? &notexported : NULL;
		var->binder = hasbindings(defn);
	}

	RefEnd3(var, defn, name);
}

extern void varpop(char *name) {
	List *setfunc;

	if (!specialvar(name))
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
		Ref(List *, defn, eval(append(sf, next == NULL ? NULL : next->defn), NULL, TRUE, FALSE));
		if (defn == NULL)
			next = NULL;
		else if (next == NULL)
			next = mkvar(defn, specialvar(np) ? &notexported : NULL, NULL);
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
	Var *var = value;
	assert(gcisblocked());
	if (var == NULL || var->env == &notexported)
		return;
	if (var->env == NULL || (rebound && var->binder)) {
		char *envstr = str("%F=%L", key, var->defn, "\001");
		var->env = envstr;
	}
	assert(env->count < env->alloclen);
	env->vector[env->count++] = var->env;
	if (env->count == env->alloclen) {
		Vector *newenv = mkvector(env->alloclen * 2);
		newenv->count = env->count;
		memcpy(newenv->vector, env->vector, env->count * sizeof *env->vector);
		env = newenv;
	}
}
	
extern Vector *mkenv(void) {
	if (isdirty || rebound) {
		env->count = envmin;
		gcdisable(0);		/* TODO: make this a good guess */
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

/* isnoexport -- is a variable unexported? */
extern Boolean isnoexport(const char *name) {
	Var *var = dictget(vars, name);
	return var != NULL && var->env == &notexported;
}

/* initpath -- set $path based on the configuration default */
static void initpath(void) {
	int i;
	static const char * const path[] = { INITIAL_PATH };
	
	Ref(List *, list, NULL);
	for (i = arraysize(path); i-- > 0;)
		list = mklist(mkterm((char *) path[i], NULL), list);
	vardef("path", NULL, list);
	RefEnd(list);
}

/* initpath -- set $pid for this shell */
static void initpid(void) {
	vardef("pid", NULL, mklist(mkterm(str("%d", getpid()), NULL), NULL));
}

/* hide -- worker function for dictforall to hide initial state */
static void hide(void *dummy, char *key, void *value) {
	((Var *) value)->env = &notexported;
}

extern void initvars(char **envp, Boolean protected) {
	char *envstr;

	globalroot(&vars);
	globalroot(&env);
	globalroot(&sortenv);
	vars = mkdict();
	env = mkvector(10);

	runstring(initial, "initial.es", 0);
	initpath();
	initpid();
	dictforall(vars, hide, NULL);

	for (; (envstr = *envp) != NULL; envp++) {
		char *eq = strchr(envstr, '=');
		if (eq == NULL) {
			env->vector[env->count++] = envstr;
			if (env->count == env->alloclen) {
				Vector *newenv = mkvector(env->alloclen * 2);
				newenv->count = env->count;
				memcpy(newenv->vector, env->vector, env->count * sizeof *env->vector);
				env = newenv;
			}
			continue;
		}
		*eq = '\0';
		Ref(char *, name, str("%N", envstr));
		*eq = '=';
		if (!protected || (!hasprefix(name, "fn-") && !hasprefix(name, "set-"))) {
			List *defn = fsplit("\1", mklist(mkterm(eq + 1, NULL), NULL));
			vardef(name, NULL, defn);
		}
		RefEnd(name);
	}

	envmin = env->count;
}
