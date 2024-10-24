#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Print an es script with a \0 in the middle.
 * Blatant ripoff of rc's version. */
static int print0(void) {
	putchar('r'); putchar('e');
	putchar('\0');
	putchar('s'); putchar('u');
	putchar('l'); putchar('t');
	putchar(' '); putchar('6');
	putchar('\n');
	return 0;
}

/* Sleep for a while. */
static int dosleep(void) {
	return sleep(5);
}

int main(int argc, char **argv) {
	if (argc < 2 || argv[1][0] == '\0') {
		fprintf(stderr, "give testrun a command\n");
		exit(2);
	}
	switch (argv[1][0]) {
	case '0':
		return print0();
	case 's':
		return dosleep();
	}
}
