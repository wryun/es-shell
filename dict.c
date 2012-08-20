/* dict.c -- hash-table based dictionaries ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"

#define	INIT_DICT_SIZE	2
#define	REMAIN(n)	(((n) * 2) / 3)
#define	GROW(n)		((n) * 2)

/*
 * hashing
 */

/* strhash2 -- the (probably too slow) haahr hash function */
static unsigned long strhash2(const char *str1, const char *str2) {

#define	ADVANCE() { \
		if ((c = *s++) == '\0') \
			if (str2 == NULL) \
				break; \
			else { \
				s = (unsigned char *) str2; \
				str2 = NULL; \
				if ((c = *s++) == '\0') \
					break; \
			} \
	}

	int c;
	unsigned long n = 0;
	unsigned char *s = (unsigned char *) str1;
	assert(str1 != NULL);
	while (1) {
		ADVANCE();
		n += (c << 17) ^ (c << 11) ^ (c << 5) ^ (c >> 1);
		ADVANCE();
		n ^= (c << 14) + (c << 7) + (c << 4) + c;
		ADVANCE();
		n ^= (~c << 11) | ((c << 3) ^ (c >> 1));
		ADVANCE();
		n -= (c << 16) | (c << 9) | (c << 2) | (c & 3);
	}
	return n;
}

/* strhash -- hash a single string */
static unsigned long strhash(const char *str) {
	return strhash2(str, NULL);
}


/*
 * data structures and garbage collection
 */

DefineTag(Dict, static);

typedef struct {
	char *name;
	void *value;
} Assoc;

struct Dict {
	int size, remain;
	Assoc table[1];		/* variable length */
};


static Dict *mkdict0(int size) {
	size_t len = offsetof(Dict, table[size]);
	Dict *dict = gcalloc(len, &DictTag);
	memzero(dict, len);
	dict->size = size;
	dict->remain = REMAIN(size);
	return dict;
}

static void *DictCopy(void *op) {
	Dict *dict = op;
	size_t len = offsetof(Dict, table[dict->size]);
	void *np = gcalloc(len, &DictTag);
	memcpy(np, op, len);
	return np;
}

static size_t DictScan(void *p) {
	Dict *dict = p;
	int i;
	for (i = 0; i < dict->size; i++) {
		Assoc *ap = &dict->table[i];
		ap->name  = forward(ap->name);
		ap->value = forward(ap->value);
	}
	return offsetof(Dict, table[dict->size]);
}


/*
 * private operations
 */

static char DEAD[] = "DEAD";

static Assoc *get(Dict *dict, const char *name) {
	Assoc *ap;
	unsigned long n = strhash(name), mask = dict->size - 1;
	for (; (ap = &dict->table[n & mask])->name != NULL; n++)
		if (ap->name != DEAD && streq(name, ap->name))
			return ap;
	return NULL;
}

static Dict *put(Dict *dict, char *name, void *value) {
	unsigned long n, mask;
	Assoc *ap;
	assert(get(dict, name) == NULL);
	assert(value != NULL);

	if (dict->remain <= 1) {
		Dict *new;
		Ref(Dict *, old, dict);
		Ref(char *, np, name);
		Ref(void *, vp, value);
		new = mkdict0(GROW(old->size));
		dictforall(old, (void (*)(void *, char *, void *)) put, new);
		dict = new;
		name = np;
		value = vp;
		RefEnd3(vp, np, old);
	}

	n = strhash(name);
	mask = dict->size - 1;
	for (; (ap = &dict->table[n & mask])->name != DEAD; n++)
		if (ap->name == NULL) {
			--dict->remain;
			break;
		}

	ap->name = name;
	ap->value = value;
	return dict;
}

static void rm(Dict *dict, Assoc *ap) {
	unsigned long n, mask;
	assert(dict->table <= ap && ap < &dict->table[dict->size]);

	ap->name = DEAD;
	ap->value = NULL;
	n = ap - dict->table;
	mask = dict->size - 1;
	for (n++; (ap = &dict->table[n & mask])->name == DEAD; n++)
		;
	if (ap->name != NULL)
		return;
	for (n--; (ap = &dict->table[n & mask])->name == DEAD; n--) {
		ap->name = NULL;
		++dict->remain;
	}
}



/*
 * exported functions
 */

extern Dict *mkdict(void) {
	return mkdict0(INIT_DICT_SIZE);
}

extern void *dictget(Dict *dict, const char *name) {
	Assoc *ap = get(dict, name);
	if (ap == NULL)
		return NULL;
	return ap->value;
}

extern Dict *dictput(Dict *dict, char *name, void *value) {
	Assoc *ap = get(dict, name);
	if (value != NULL)
		if (ap == NULL)
			dict = put(dict, name, value);
		else
			ap->value = value;
	else if (ap != NULL)
		rm(dict, ap);
	return dict;
}

extern void dictforall(Dict *dp, void (*proc)(void *, char *, void *), void *arg) {
	int i;
	Ref(Dict *, dict, dp);
	Ref(void *, argp, arg);
	for (i = 0; i < dict->size; i++) {
		Assoc *ap = &dict->table[i];
		if (ap->name != NULL && ap->name != DEAD)
			(*proc)(argp, ap->name, ap->value);
	}
	RefEnd2(argp, dict);
}

/* dictget2 -- look up the catenation of two names (such a hack!) */
extern void *dictget2(Dict *dict, const char *name1, const char *name2) {
	Assoc *ap;
	unsigned long n = strhash2(name1, name2), mask = dict->size - 1;
	for (; (ap = &dict->table[n & mask])->name != NULL; n++)
		if (ap->name != DEAD && streq2(ap->name, name1, name2))
			return ap->value;
	return NULL;
}
