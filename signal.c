/* signal.c -- signal handling ($Revision: 1.12 $) */

#include "es.h"
#include "sigmsgs.h"

typedef Sigresult (*Sighandler)(int);

Boolean sigint_newline = TRUE;

jmp_buf slowlabel;
Atomic slow = FALSE;
Atomic interrupted = FALSE;
static Atomic sigcount;
static Atomic caught[NSIG];
static Sigeffect sigeffect[NSIG];
static Sighandler defaulthandler[NSIG];

#if USE_SIGACTION
#ifndef	SA_NOCLDSTOP
#define	SA_NOCLDSTOP	0
#endif
#ifndef	SA_NOCLDWAIT
#define	SA_NOCLDWAIT	0
#endif
#ifndef	SA_INTERRUPT		/* for sunos */
#define	SA_INTERRUPT	0
#endif
#endif


/*
 * name<->signal mappings
 */

extern int signumber(const char *name) {
	int i;
	for (i = 0; i < nsignals; i++)
		if (streq(signals[i].name, name))
			return signals[i].sig;
	return -1;
}

extern char *signame(int sig) {
	int i;
	for (i = 0; i < nsignals; i++)
		if (signals[i].sig == sig)
			return (char *) signals[i].name;
	return str("sig%d", sig);
}

extern char *sigmessage(int sig) {
	int i;
	for (i = 0; i < nsignals; i++)
		if (signals[i].sig == sig)
			return (char *) signals[i].msg;
	return str("unkown signal %d", sig);
}


/*
 * the signal handler
 */

/* catcher -- catch (and defer) a signal from the kernel */
static void catcher(int sig) {
#if SYSV_SIGNALS /* only do this for unreliable signals */
	signal(sig, catcher);
#endif
	if (hasforked)
		exit(1); /* exit unconditionally on a signal in a child process */
	if (caught[sig] == 0) {
		caught[sig] = TRUE;
		++sigcount;
	}
	interrupted = TRUE;
#if !SYSV_SIGNALS
	if (slow)
		longjmp(slowlabel, 1);
#endif
}


/*
 * setting and getting signal effects
 */

static Sighandler setsignal(int sig, Sighandler handler) {
#if USE_SIGACTION
	struct sigaction nsa, osa;
	sigemptyset(&nsa.sa_mask);
	nsa.sa_handler = handler;
	nsa.sa_flags = SA_INTERRUPT;
	if (sigaction(sig, &nsa, &osa) == -1)
		return SIG_ERR;
	return osa.sa_handler;
#else /* !USE_SIGACTION */
#if SPECIAL_SIGCLD
	if (sig == SIGCLD && handler != SIG_DFL)
		return SIG_ERR;
#endif
	return signal(sig, handler);
#endif /* !USE_SIGACTION */
}

extern Sigeffect esignal(int sig, Sigeffect effect) {
	Sigeffect old;
	assert(0 < sig && sig <= NSIG);
	old = sigeffect[sig];
	if (effect != sig_nochange && effect != old) {
		switch (effect) {
		case sig_ignore:
			if (setsignal(sig, SIG_IGN) == SIG_ERR) {
				eprint("$&setsignals: cannot ignore %s\n", signame(sig));
				return old;
			}
			break;
		case sig_catch:
			if (setsignal(sig, catcher) == SIG_ERR) {
				eprint("$&setsignals: cannot catch %s\n", signame(sig));
				return old;
			}
			break;
		case sig_default:
			setsignal(sig, defaulthandler[sig]);
			break;
		default:
			NOTREACHED;
		}
		sigeffect[sig] = effect;
	}
	return old;
}

extern void setsigeffects(const Sigeffect effects[]) {
	int sig;
	for (sig = 1; sig < NSIG; sig++)
		esignal(sig, effects[sig]);
}

extern void getsigeffects(Sigeffect effects[]) {
	memcpy(effects, sigeffect, sizeof sigeffect);
}


/*
 * initialization
 */

static void markspecial(int sig) {
	defaulthandler[sig] = catcher;
	setsignal(sig, catcher);
}

extern void initsignals(Boolean interactive, Boolean allowdumps) {
	int sig;
	Push settor;

	for (sig = 0; sig < nsignals; sig++)
		if (signals[sig].sig < 1 || NSIG <= signals[sig].sig)
			panic(
				"initsignals: bad signal in sigmsgs.c: %s (see mksignal)",
				signals[sig].name
			);

	for (sig = 1; sig < NSIG; sig++)
		defaulthandler[sig] = SIG_DFL;
	for (sig = 1; sig < NSIG; sig++) {
		Sighandler h;
#if USE_SIGACTION
		struct sigaction sa;
		sigaction(sig, NULL, &sa);
		h = sa.sa_handler;
		if (h == SIG_IGN)
			sigeffect[sig] = sig_ignore;
#else /* !USE_SIGACTION */
		h = signal(sig, SIG_DFL);
		if (h == SIG_IGN) {
			setsignal(sig, SIG_IGN);
			sigeffect[sig] = sig_ignore;
		}
#endif /* !USE_SIGACTION */
		else if (h == SIG_DFL || h == SIG_ERR)
			sigeffect[sig] = sig_default;
		else
			panic(
				"initsignals: bad incoming signal value for %s: %x",
				signame(sig), h
			);
	}

	if (interactive || sigeffect[SIGINT] == sig_default)
		markspecial(SIGINT);
	if (!allowdumps) {
		if (interactive)
			markspecial(SIGTERM);
		if (interactive || sigeffect[SIGQUIT] == sig_default)
			markspecial(SIGQUIT);
	}

	/* here's the end-run around set-signals */
	varpush(&settor, "set-signals", NULL);
	vardef("signals", NULL, mksiglist());
	varpop(&settor);
}

extern void setsigdefaults(void) {
	int sig;

	for (sig = 1; sig < NSIG; sig++) {
		Sigeffect e = sigeffect[sig];
		if (e == sig_catch || (e == sig_default && defaulthandler[sig] != SIG_DFL)) {
			defaulthandler[sig] = SIG_DFL;
			esignal(sig, sig_default);
		}
	}

	defaulthandler[SIGINT] = SIG_DFL;
	defaulthandler[SIGQUIT] = SIG_DFL;
	defaulthandler[SIGTERM] = SIG_DFL;
}


/*
 * utility functions
 */

extern Boolean issilentsignal(List *e) {
	return streq(getstr(e->term), "signal")
		&& e->next != NULL
		&& streq(getstr(e->next->term), "sigint");
}

extern List *mksiglist(void) {
	int sig = NSIG;
	Sigeffect effects[NSIG];
	getsigeffects(effects);
	Ref(List *, lp, NULL);
	while (--sig > 0)
		switch (effects[sig]) {
		case sig_default:
			break;
		case sig_catch:
			lp = mklist(mkterm(signame(sig), NULL), lp);
			break;
		case sig_ignore:
			lp = mklist(mkterm(str("-%s", signame(sig)), NULL), lp);
			break;
		default:
			panic("mksiglist: getsigeffects returned bad value for %s: %d",
			      signame(sig), effects[sig]);
		}
	RefReturn(lp);
}


/*
 * signal delivery
 */

static int blocked = 0;

/* blocksignals -- turn off delivery of signals as exceptions */
extern void blocksignals(void) {
	assert(blocked >= 0);
	++blocked;
}

/* unblocksignals -- turn on delivery of signals as exceptions */
extern void unblocksignals(void) {
	assert(blocked > 0);
	--blocked;
}

/* sigchk -- throw the signal as an exception */
extern void sigchk(void) {
	int sig;

	if (sigcount == 0 || blocked)
		return;
	if (hasforked)
		exit(1);	/* exit unconditionally on a signal in a child process */

	for (sig = 0;; sig++) {
		if (caught[sig] != 0) {
			--sigcount;
			caught[sig] = 0;
			break;
		}
		if (sig >= NSIG) {
			/* panic("all-zero sig vector with nonzero sigcount"); */
			sigcount = 0;
			return;
		}
	}
	Ref(List *, e, mklist(mkterm("signal", NULL), mklist(mkterm(signame(sig), NULL), NULL)));

	switch (sigeffect[sig]) {
	case sig_catch:
		while (gcisblocked())
			gcenable();
		throw(e);
		NOTREACHED;
	case sig_default:
		if (sig == SIGINT) {
			/* this is the newline you see when you hit ^C while typing a command */
			if (sigint_newline)
				eprint("\n");
			sigint_newline = TRUE;
			while (gcisblocked())
				gcenable();
			throw(e);
			NOTREACHED;
		}
		break;
	default:
		/* panic("sigchk: caught %L with sigeffect %d", e, " ", sigeffect[sig]) */ ;
	}
	RefEnd(e);
}
