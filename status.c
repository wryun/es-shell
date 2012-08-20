/* status.c -- status manipulations */

#include "es.h"
#include "sigmsgs.h"

extern long strtol(const char *nptr, char **endptr, int base);


static Term trueterm = { "0", NULL }, falseterm = { "1", NULL };
static List truelist = { &trueterm, NULL }, falselist = { &falseterm, NULL };
List *true = &truelist, *false = &falselist;

/* istrue -- is this status list true? */
extern Boolean istrue(List *status) {
	for (; status != NULL; status = status->next) {
		Term *term = status->term;
		if (term->closure != NULL)
			return FALSE;
		else {
			const char *str = term->str;
			assert(str != NULL);
			if (*str != '\0' && (*str != '0' || str[1] != '\0'))
				return FALSE;
		}
	}
	return TRUE;
}

/* exitstatus -- turn a status list into an exit(2) value */
extern int exitstatus(List *status) {
	Term *term;
	char *s;
	unsigned long n;

	if (status == NULL)
		return 0;
	if (status->next != NULL)
		return istrue(status) ? 0 : 1;
	term = status->term;
	if (term->closure != NULL)
		return 1;

	s = term->str;
	if (*s == '\0')
		return 0;
	n = strtol(s, &s, 0);
	if (*s != '\0' || n > 255)
		return 1;
	return n;
}

/* mkstatus -- turn a unix exit(2) status into a string */
extern char *mkstatus(int status) {
	if (status & 0xff) {
		const char *core = (status & 0x80) ? "+core" : "";
		int sig = (status & 0x7f);
		return (sig < NUMOFSIGNALS && *signals[sig].name != '\0')
			? str("%s%s", signals[sig].name, core)
			: str("sig%d%s", sig, core);
	}
	return str("%d", (status >> 8) & 0xff);
}

/* printstatus -- print the status if we should */
extern void printstatus(int pid, int status) {
	if (status & 0xff) {
		char *msg = ((status & 0x7f) < NUMOFSIGNALS ? signals[status & 0x7f].msg : "");
		if (pid != 0)
			print("%d: ", pid);
		if (status & 0x80) {
			if (*msg == '\0')
				print("core dumped\n");
			else
				print("%s--core dumped\n", msg);
		} else if (*msg != '\0')
			print("%s\n", msg);
	}
}
