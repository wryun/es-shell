/* signal.c -- signal handling */

#include "es.h"
#include "sigmsgs.h"

Boolean sigint_newline = TRUE;

jmp_buf slowlabel;
volatile SIG_ATOMIC_T slow = FALSE;
volatile SIG_ATOMIC_T interrupted = FALSE;
static void (*sighandlers[NUMOFSIGNALS])(int);
static volatile SIG_ATOMIC_T sigcount;
static volatile SIG_ATOMIC_T caught[NUMOFSIGNALS];

static void (*default_sigint)(int) = SIG_DFL,
	    (*default_sigquit)(int) = SIG_DFL,
	    (*default_sigterm)(int) = SIG_DFL;

/* catcher -- the signal handler routine */
extern void catcher(int sig) {
	if (hasforked)
		exit(1); /* exit unconditionally on a signal in a child process */
	if (caught[sig] == 0) {
		caught[sig] = TRUE;
		++sigcount;		/* TODO: unsafe? */
	}
	signal(sig, catcher);
	interrupted = TRUE;
#if !SYSV_SIGNALS
	if (slow)
		longjmp(slowlabel, 1);
#endif
}

extern void sigchk(void) {
	if (sigcount != 0) {
		int sig;
		void (*h)(int);
		if (hasforked)
			exit(1); /* exit unconditionally on a signal in a child process */
		for (sig = 0;; sig++) {
			if (caught[sig] != 0) {
				--sigcount;	/* TODO: this is unsafe */
				caught[sig] = 0;
				break;
			}
			if (sig >= NUMOFSIGNALS)
				panic("all-zero sig vector with nonzero sigcount");
		}
		if ((h = sighandlers[sig]) == SIG_DFL)
			panic("caught signal set to SIG_DFL");
		if (h == SIG_IGN)
			panic("caught signal set to SIG_IGN");
		(*h)(sig);
	}
}

extern void (*esignal(int sig, void (*h)(int)))(int) {
	void (*old)(int);
	SIGCHK();
	old = sighandlers[sig];
	if (h == SIG_DFL || h == SIG_IGN) {
		signal(sig, h);
		sighandlers[sig] = h;
	} else {
		sighandlers[sig] = h;
		signal(sig, catcher);
	}
	return old;
}

static void sigint(int sig) {
	assert(sig == SIGINT);
	/* this is the newline you see when you hit ^C while typing a command */
	if (sigint_newline)
		eprint("\n");
	sigint_newline = TRUE;
	/* TODO: redirq = NULL;	cond = FALSE; */
	while (gcblocked)
		gcenable();
	throw(mklist(mkterm("signal", NULL), mklist(mkterm("sigint", NULL), NULL)));
}

static void noop(int sig) {
}

extern void initsignals(Boolean allowdumps) {
	int sig;
	for (sig = 1; sig < NUMOFSIGNALS; sig++) {
		void (*h)(int) = signal(sig, SIG_DFL);
		if (h != SIG_DFL)
			signal(sig, h);
		sighandlers[sig] = h;
	}
	if (interactive || sighandlers[SIGINT] != SIG_IGN)
		esignal(SIGINT, default_sigint = sigint);
	if (!allowdumps) {
		if (interactive)
			esignal(SIGTERM, default_sigterm = noop);
		if (interactive || sighandlers[SIGQUIT] != SIG_IGN)
			esignal(SIGQUIT, default_sigquit = noop);
	}
}

extern Boolean issilentsignal(List *e) {
	return streq(getstr(e->term), "signal")
		&& e->next != NULL
		&& streq(getstr(e->next->term), "sigint");
}

extern void setsigdefaults(Boolean background) {

#if JOB_PROTECT && SIGTTOU && SIGTTIN && SIGTSTP
#define	IGNORE(signal) ((default_ ## signal) = SIG_DFL)
#else
#define	IGNORE(signal) \
	if (!background) \
		(default_ ## signal) = SIG_DFL; \
	else { \
		(default_ ## signal) = SIG_IGN; \
		esignal(sig, SIG_IGN); \
		/* TODO: remove #signal from $signals */ \
		break; \
	}
#endif

	int sig;
	closefds();
	for (sig = 1; sig < NUMOFSIGNALS; sig++)
		if (sighandlers[sig] != SIG_IGN) {
			sighandlers[sig] = NULL;
			switch (sig) {
			case SIGINT:
				IGNORE(sigint);
				goto common;
			case SIGQUIT:
				IGNORE(sigquit);
				goto common;
			case SIGTERM:
				default_sigterm = SIG_DFL;
				goto common;
			default:
			common:
				if (sighandlers[sig] != SIG_DFL) {
					esignal(sig, SIG_DFL);
					/* TODO: delete_fn(signals[i].name); */
				}
			}
		}
	/* TODO: sigexit */
}
