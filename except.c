/* except.c -- exception mechanism ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "print.h"

/* globals */
Handler *tophandler = NULL;
Handler *roothandler = NULL;
List *exception = NULL;
Push *pushlist = NULL;

/* pophandler -- remove a handler */
extern void pophandler(Handler *handler) {
	assert(tophandler == handler);
	assert(handler->rootlist == rootlist);
	tophandler = handler->up;
}

/* throw -- raise an exception */
extern noreturn throw(List *e) {
	Handler *handler = tophandler;

	assert(!gcisblocked());
	assert(e != NULL);
	assert(handler != NULL);
	tophandler = handler->up;
	
	while (pushlist != handler->pushlist) {
		rootlist = &pushlist->defnroot;
		varpop(pushlist);
	}
	evaldepth = handler->evaldepth;

#if ASSERTIONS
	for (; rootlist != handler->rootlist; rootlist = rootlist->next)
		assert(rootlist != NULL);
#else
	rootlist = handler->rootlist;
#endif
	exception = e;
	longjmp(handler->label, 1);
	NOTREACHED;
}

/* fail -- pass a user catchable error up the exception chain */
extern noreturn fail VARARGS2(const char *, from, const char *, fmt) {
	char *s;
	va_list args;

	VA_START(args, fmt);
	s = strv(fmt, args);
	va_end(args);

	gcdisable();
	Ref(List *, e, mklist(mkstr("error"),
			      mklist(mkstr((char *) from),
				     mklist(mkstr(s), NULL))));
	while (gcisblocked())
		gcenable();
	throw(e);
	RefEnd(e);
}

/* newchildcatcher -- remove the current handler chain for a new child */
extern void newchildcatcher(void) {
	tophandler = roothandler;
}

#if DEBUG_EXCEPTIONS
/* raised -- print exceptions as we climb the exception stack */
extern List *raised(List *e) {
	eprint("raised (sp @ %x) %L\n", &e, e, " ");
	return e;
}
#endif
