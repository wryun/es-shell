/* token.c -- lexical analyzer for es */

#include <ctype.h>
#include "es.h"
#include "token.h"

#define	isodigit(c)	('0' <= (c) && (c) < '8')


#define BUFSIZE	((size_t) 1000)	/*	malloc hates power of 2 buffers? */
#define BUFMAX	(8 * BUFSIZE)	/* 	How big the buffer can get before we re-allocate the
				 *	space at BUFSIZE again. Premature optimization? Maybe.
				 */

typedef enum wordstates {
	NW, RW, KW /* "nonword", "realword", "keyword" */
} wordstates;

int lineno;

/*
 *	Special characters (i.e., "non-word") in rc:
 *		\t \n # ; & | ^ $ = ~ ` ' ! { } ( ) < > \
 */

const char nw[] = {
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char dnw[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static size_t bufsize = BUFSIZE;
static char *realbuf = NULL;
static Boolean newline = FALSE;
static wordstates w = NW;
static Boolean goterror = FALSE;

#define checkfreecaret {if (w != NW) { w = NW; ugchar(c); return '^'; }}


/* pr_error -- print error with line number on noninteractive shells (i.e., scripts) */
extern void pr_error(char *s) {
	assert(s != NULL);
	if (interactive)
		eprint("%s\n", s);
	else
		eprint("line %d: %s\n", lineno - 1, s);
}


/*
 * getredir
 *	Scan in a pair of integers for redirections like >[2=1]. CLOSED represents
 *	a closed file descriptor (i.e., >[2=]).
 *
 *	This function makes use of unsigned compares to make range tests in
 *	one compare operation.
 */

#define	CLOSED	-1
#define	DEFAULT	-2

static Redir *getredir(int c, RedirKind k, int default0, int default1) {
	Redir *redir;
	int n;

	redir = gcalloc(sizeof (Redir), NULL);		/* shouldn't live at collection time */
	redir->kind = k;
	redir->next = NULL;
	redir->tree = NULL;
	redir->fd[0] = default0;
	redir->fd[1] = default1;

	if (c != '[') {
		ugchar(c);
		return redir;
	}
	if ((unsigned int) (n = gchar() - '0') > 9) {
		scanerror("expected digit after '['");
		return NULL;
	}

	while ((unsigned int) (c = gchar() - '0') <= 9)
		n = n * 10 + c;
	redir->fd[0] = n;

	switch (c += '0') {
	case '=':
		if ((unsigned int) (n = gchar() - '0') > 9) {
			if (n != ']' - '0') {
				scanerror("expected digit or ']' after '='");
				return NULL;
			}
			redir->fd[1] = CLOSED;
		} else {
			while ((unsigned int) (c = gchar() - '0') <= 9)
				n = n * 10 + c;
			if (c != ']' - '0') {
				scanerror("expected ']' after digit");
				return NULL;
			}
			redir->fd[1] = n;
		}
		break;
	case ']':
		break;
	default:
		scanerror("expected '=' or ']' after digit");
		return NULL;
	}
	return redir;
}

/* print_prompt2 -- called before all continuation lines */
extern void print_prompt2() {
	lineno++;
#if READLINE
	prompt = prompt2;
#else
	if (interactive)
		eprint("%s", prompt2);
#endif
}

extern int yylex(void) {
	static Boolean dollar = FALSE;
	int c;
	size_t i;			/* The purpose of all these local assignments is to	*/
	const char *meta;		/* allow optimizing compilers like gcc to load these	*/
	char *buf = realbuf;		/* values into registers. On a sparc this is a		*/
	YYSTYPE *y = &yylval;		/* win, in code size *and* execution time		*/

	if (goterror) {
		goterror = FALSE;
		return '\n';
	}

	/* rc variable-names may contain only alnum, '*' and '_', so use dnw if we are scanning one. */
	meta = (dollar ? dnw : nw);
	dollar = FALSE;
	if (newline) {
		--lineno; /* slight space optimization; print_prompt2() always increments lineno */
		print_prompt2();
		newline = FALSE;
	}
top:	while ((c = gchar()) == ' ' || c == '\t')
		w = NW;
	if (c == EOF)
		return END;
	if (!meta[(unsigned char) c]) {	/* it's a word or keyword. */
		checkfreecaret;
		w = RW;
		i = 0;
		do {
			buf[i++] = c;
			if (i >= bufsize)
				buf = realbuf = erealloc(buf, bufsize *= 2);
		} while ((c = gchar()) != EOF && !meta[(unsigned char) c]);
		ugchar(c);
		buf[i] = '\0';
		w = KW;
		if (streq(buf, "@"))			return '@';
		if (*buf == 'f') {
			if (streq(buf + 1, "n"))	return FN;
			if (streq(buf + 1, "or"))	return FOR;
		}
		if (*buf == 'l') {
			if (streq(buf + 1, "ocal"))	return LOCAL;
			if (streq(buf + 1, "et"))	return LET;
		}
		if (streq(buf, "case"))			return CASE;
		if (streq(buf, "switch"))		return SWITCH;
		w = RW;
		y->str = gcdup(buf);
		return WORD;
	}
	if (c == '`' || c == '!' || c == '~' || c == '$' || c == '\'') {
		checkfreecaret;
		if (c == '!' || c == '~')
			w = KW;
	}
	switch (c) {
	case '\0':
		pr_error("warning: null character ignored");
		goto top;
	case '!':
		return '!';
	case '~':
		return '~';
	case '`':
		c = gchar();
		if (c == '`')
			return BACKBACK;
		ugchar(c);
		return '`';
	case '$':
		dollar = TRUE;
		switch (c = gchar()) {
		case '#':	return COUNT;
		case '^':	return FLAT;
		case '&':	return PRIM;
		default:	ugchar(c); return '$';
		}
	case '\'':
		w = RW;
		i = 0;
		while ((c = gchar()) != '\'' || (c = gchar()) == '\'') {
			buf[i++] = c;
			if (c == '\n')
				print_prompt2();
			if (c == EOF) {
				w = NW;
				scanerror("eof in quoted string");
				return ERROR;
			}
			if (i >= bufsize)
				buf = realbuf = erealloc(buf, bufsize *= 2);
		}
		ugchar(c);
		buf[i] = '\0';
		y->str = gcdup(buf);
		return QWORD;
	case '\\':
		if ((c = gchar()) == '\n') {
			print_prompt2();
			ugchar(' ');
			goto top; /* Pretend it was just another space. */
		}
		if (c == EOF) {
			ugchar(EOF);
			goto badescape;
		}
		ugchar(c);
		c = '\\';
		checkfreecaret;
		w = RW;
		c = gchar();
		switch (c) {
		case 'a':	*buf = '\a';	break;
		case 'b':	*buf = '\b';	break;
		case 'e':	*buf = '\033';	break;
		case 'f':	*buf = '\f';	break;
		case 'n':	*buf = '\n';	break;
		case 'r':	*buf = '\r';	break;
		case 't':	*buf = '\t';	break;
		case 'x': {
			int n = 0;
			for (;;) {
				c = gchar();
				if (!isxdigit(c))
					break;
				n = (n << 4) | (c - (isdigit(c) ? '0' : islower(c) ? 'a' : 'A'));
			}
			if (n == 0)
				goto badescape;
			ugchar(c);
			*buf = n;
			break;
		}
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
			int n = 0;
			do {
				n = (n << 3) | (c - '0');
				c = gchar();
			} while (isodigit(c));
			if (n == 0)
				goto badescape;
			ugchar(c);
			*buf = n;
			break;
		}
		default:
			if (isalnum(c)) {
			badescape:
				scanerror("bad backslash escape");
				return ERROR;
			}
			*buf = c;
			break;
		}
		buf[1] = 0;
		y->str = gcdup(buf);
		return QWORD;
	case '(':
		if (w == RW) /* SUB's happen only after real words, not keyowrds, so if () and while () work */
			c = SUB;
		w = NW;
		return c;
	case '#':
		while ((c = gchar()) != '\n') /* skip comment until newline */
			if (c == EOF)
				return END;
		/* FALLTHROUGH */
	case '\n':
		lineno++;
		newline = TRUE;
		/* FALLTHROUGH */
	case ';':
	case '^':
	case ')':
	case '=':
	case '{': case '}':
		w = NW;
		return c;
	case '&':
		w = NW;
		c = gchar();
		if (c == '&')
			return ANDAND;
		ugchar(c);
		return '&';
	case '|':
		w = NW;
		c = gchar();
		if (c == '|')
			return OROR;
		y->redir = getredir(c, rPipe, 1, 0);
		if (y->redir == NULL)
			return ERROR;
		if (y->redir->fd[1] == CLOSED) {
			scanerror("expected digit after '='");		/* can't close a pipe */
			return ERROR;
		}
		return '|';

	{
		RedirKind kind;
		int fd0;

	case '<':
		fd0 = 0;
		if ((c = gchar()) == '>')
			return BOX;
		else if (c == '<')
			if ((c = gchar()) == '<') {
				c = gchar();
				kind = rHerestring;
			} else
				kind = rHeredoc;
		else
			kind = rOpen;
		goto redirection;
	case '>':
		fd0 = 1;
		if ((c = gchar()) == '>') {
			c = gchar();
			kind = rAppend;
		} else
			kind = rCreate;
		goto redirection;
	redirection:
		w = NW;
		y->redir = getredir(c, kind, fd0, DEFAULT);
		if (y->redir == NULL)
			return ERROR;
		if (y->redir->fd[1] != DEFAULT) {
			y->redir->kind = (y->redir->fd[1] == CLOSED) ? rClose : rDup;
			return DUP;
		}
		return REDIR;
	}

	default:
		w = NW;
		return c; /* don't know what it is, let yacc barf on it */
	}
}

extern void inityy() {
	newline = FALSE;
	w = NW;
	/* TODO: hq = NULL; */
	/* return memory to the system if the buffer got too large */
	if (bufsize > BUFMAX && realbuf != NULL) {
		efree(realbuf);
		bufsize = BUFSIZE;
		realbuf = ealloc(bufsize);
	} else if (realbuf == NULL)
		realbuf = ealloc(bufsize);
}

/* locate -- identify where an error came from */
extern char *locate(char *s) {
	if (interactive)
		return s;
	return str("%d: %s near %s", lineno - (last == '\n'), s,
			w == NW
			  ? last == EOF
			    ? "end of file"
			    : last == '\n'
			      ? "end of line"
			      : str((last < 32 || last > 126) ? "<ascii %d>" : "'%c'", last)
			  : realbuf);
}

/* scanerror -- called for lexical errors */
extern void scanerror(char *s) {
	goterror = TRUE;
	flushu(); /* flush upto newline */
	yyerror(s);
}
