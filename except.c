/* except.c -- exception mechanism */

#include "es.h"
#include "print.h"

/* globals */
Handler childhandler = { NULL, NULL };
Handler *bottomhandler = NULL;
List *exception = NULL;

/* pophandler -- remove a handler */
extern void pophandler(Handler *handler) {
	assert(bottomhandler == handler);
	assert(handler->rootlist == rootlist);
	bottomhandler = handler->up;
}

/* throw -- raise an exception */
extern noreturn throw(List *exc) {
	Handler *handler = bottomhandler;

	assert(gcblocked == 0);
	assert(exc != NULL);
	assert(handler != NULL);
	bottomhandler = handler->up;
	for (; rootlist != handler->rootlist; rootlist = rootlist->next)
		assert(rootlist != NULL);
	exception = exc;
	longjmp(handler->label, 1);
}

/* fail -- pass a user catchable error up the exception chain */
extern noreturn fail VARARGS1(const char *, fmt) {
	char *s;
	va_list args;

	VA_START(args, fmt);
	s = strv(fmt, args);
	va_end(args);

	gcdisable(0);
	Ref(List *, e, mklist(mkterm("error", NULL),
			      mklist(mkterm(s, NULL), NULL)));
	gcenable();
	throw(e);
	RefEnd(e);
}

/* newchildcatcher -- remove the current handler chain for a new child */
extern void newchildcatcher(void) {
	bottomhandler = &childhandler;
	/* TODO: flush some of the rootlist? */
}
