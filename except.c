/* except.c -- exception mechanism */

#include "es.h"
#include "print.h"

/* globals */
Handler childhandler = { NULL, NULL };
Handler *tophandler = NULL;
List *exception = NULL;

/* pophandler -- remove a handler */
extern void pophandler(Handler *handler) {
	assert(tophandler == handler);
	assert(handler->rootlist == rootlist);
	tophandler = handler->up;
}

/* throw -- raise an exception */
extern noreturn throw(List *exc) {
	Handler *handler = tophandler;

	assert(!gcisblocked());
	assert(exc != NULL);
	assert(handler != NULL);
	tophandler = handler->up;
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
	while (gcisblocked())
		gcenable();
	throw(e);
	RefEnd(e);
}

/* newchildcatcher -- remove the current handler chain for a new child */
extern void newchildcatcher(void) {
	tophandler = &childhandler;
	/* TODO: flush some of the rootlist? */
}
