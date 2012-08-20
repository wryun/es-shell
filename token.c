/* token.c -- lexical analyzer for es ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "input.h"
#include "syntax.h"
#include "token.h"

#define	isodigit(c)	('0' <= (c) && (c) < '8')

#define	BUFSIZE	((size_t) 1000)
#define	BUFMAX	(8 * BUFSIZE)

typedef enum { NW, RW, KW } State;	/* "nonword", "realword", "keyword" */

static State w = NW;
static Boolean newline = FALSE;
static Boolean goterror = FALSE;
static size_t bufsize = 0;
static char *tokenbuf = NULL;

#define	InsertFreeCaret()	STMT(if (w != NW) { w = NW; UNGETC(c); return '^'; })


/*
 *	Special characters (i.e., "non-word") in es:
 *		\t \n # ; & | ^ $ = ` ' ! { } ( ) < > \
 */

const char nw[] = {
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,		/*   0 -  15 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/*  16 -  32 */
	1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,		/* ' ' - '/' */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,		/* '0' - '?' */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* '@' - 'O' */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0,		/* 'P' - '_' */
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* '`' - 'o' */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,		/* 'p' - DEL */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 128 - 143 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 144 - 159 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 160 - 175 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 176 - 191 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 192 - 207 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 208 - 223 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 224 - 239 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* 240 - 255 */
};

const char dnw[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/*   0 -  15 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/*  16 -  32 */
	1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1,		/* ' ' - '/' */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,		/* '0' - '?' */
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* '@' - 'O' */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,		/* 'P' - '_' */
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,		/* '`' - 'o' */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,		/* 'p' - DEL */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* 128 - 143 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* 144 - 159 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* 160 - 175 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* 176 - 191 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* 192 - 207 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* 208 - 223 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* 224 - 239 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,		/* 240 - 255 */
};


/* print_prompt2 -- called before all continuation lines */
extern void print_prompt2(void) {
	input->lineno++;
#if READLINE
	prompt = prompt2;
#else
	if ((input->runflags & run_interactive) && prompt2 != NULL)
		eprint("%s", prompt2);
#endif
}

/* scanerror -- called for lexical errors */
static void scanerror(char *s) {
	int c;
	/* TODO: check previous character? rc's last hack? */
	while ((c = GETC()) != '\n' && c != EOF)
		;
	goterror = TRUE;
	yyerror(s);
}

/*
 * getfds
 *	Scan in a pair of integers for redirections like >[2=1]. CLOSED represents
 *	a closed file descriptor (i.e., >[2=]).
 *
 *	This function makes use of unsigned compares to make range tests in
 *	one compare operation.
 */

#define	CLOSED	-1
#define	DEFAULT	-2

static Boolean getfds(int fd[2], int c, int default0, int default1) {
	int n;
	fd[0] = default0;
	fd[1] = default1;

	if (c != '[') {
		UNGETC(c);
		return TRUE;
	}
	if ((unsigned int) (n = GETC() - '0') > 9) {
		scanerror("expected digit after '['");
		return FALSE;
	}

	while ((unsigned int) (c = GETC() - '0') <= 9)
		n = n * 10 + c;
	fd[0] = n;

	switch (c += '0') {
	case '=':
		if ((unsigned int) (n = GETC() - '0') > 9) {
			if (n != ']' - '0') {
				scanerror("expected digit or ']' after '='");
				return FALSE;
			}
			fd[1] = CLOSED;
		} else {
			while ((unsigned int) (c = GETC() - '0') <= 9)
				n = n * 10 + c;
			if (c != ']' - '0') {
				scanerror("expected ']' after digit");
				return FALSE;
			}
			fd[1] = n;
		}
		break;
	case ']':
		break;
	default:
		scanerror("expected '=' or ']' after digit");
		return FALSE;
	}
	return TRUE;
}

extern int yylex(void) {
	static Boolean dollar = FALSE;
	int c;
	size_t i;			/* The purpose of all these local assignments is to	*/
	const char *meta;		/* allow optimizing compilers like gcc to load these	*/
	char *buf = tokenbuf;		/* values into registers. On a sparc this is a		*/
	YYSTYPE *y = &yylval;		/* win, in code size *and* execution time		*/

	if (goterror) {
		goterror = FALSE;
		return NL;
	}

	/* rc variable-names may contain only alnum, '*' and '_', so use dnw if we are scanning one. */
	meta = (dollar ? dnw : nw);
	dollar = FALSE;
	if (newline) {
		--input->lineno; /* slight space optimization; print_prompt2() always increments lineno */
		print_prompt2();
		newline = FALSE;
	}
top:	while ((c = GETC()) == ' ' || c == '\t')
		w = NW;
	if (c == EOF)
		return ENDFILE;
	if (!meta[(unsigned char) c]) {	/* it's a word or keyword. */
		InsertFreeCaret();
		w = RW;
		i = 0;
		do {
			buf[i++] = c;
			if (i >= bufsize)
				buf = tokenbuf = erealloc(buf, bufsize *= 2);
		} while ((c = GETC()) != EOF && !meta[(unsigned char) c]);
		UNGETC(c);
		buf[i] = '\0';
		w = KW;
		if (buf[1] == '\0') {
			int k = *buf;
			if (k == '@' || k == '~')
				return k;
		} else if (*buf == 'f') {
			if (streq(buf + 1, "n"))	return FN;
			if (streq(buf + 1, "or"))	return FOR;
		} else if (*buf == 'l') {
			if (streq(buf + 1, "ocal"))	return LOCAL;
			if (streq(buf + 1, "et"))	return LET;
		} else if (streq(buf, "~~"))
			return EXTRACT;
		else if (streq(buf, "%closure"))
			return CLOSURE;
		w = RW;
		y->str = gcdup(buf);
		return WORD;
	}
	if (c == '`' || c == '!' || c == '$' || c == '\'') {
		InsertFreeCaret();
		if (c == '!')
			w = KW;
	}
	switch (c) {
	case '!':
		return '!';
	case '`':
		c = GETC();
		if (c == '`')
			return BACKBACK;
		UNGETC(c);
		return '`';
	case '$':
		dollar = TRUE;
		switch (c = GETC()) {
		case '#':	return COUNT;
		case '^':	return FLAT;
		case '&':	return PRIM;
		default:	UNGETC(c); return '$';
		}
	case '\'':
		w = RW;
		i = 0;
		while ((c = GETC()) != '\'' || (c = GETC()) == '\'') {
			buf[i++] = c;
			if (c == '\n')
				print_prompt2();
			if (c == EOF) {
				w = NW;
				scanerror("eof in quoted string");
				return ERROR;
			}
			if (i >= bufsize)
				buf = tokenbuf = erealloc(buf, bufsize *= 2);
		}
		UNGETC(c);
		buf[i] = '\0';
		y->str = gcdup(buf);
		return QWORD;
	case '\\':
		if ((c = GETC()) == '\n') {
			print_prompt2();
			UNGETC(' ');
			goto top; /* Pretend it was just another space. */
		}
		if (c == EOF) {
			UNGETC(EOF);
			goto badescape;
		}
		UNGETC(c);
		c = '\\';
		InsertFreeCaret();
		w = RW;
		c = GETC();
		switch (c) {
		case 'a':	*buf = '\a';	break;
		case 'b':	*buf = '\b';	break;
		case 'e':	*buf = '\033';	break;
		case 'f':	*buf = '\f';	break;
		case 'n':	*buf = '\n';	break;
		case 'r':	*buf = '\r';	break;
		case 't':	*buf = '\t';	break;
		case 'x': case 'X': {
			int n = 0;
			for (;;) {
				c = GETC();
				if (!isxdigit(c))
					break;
				n = (n << 4)
				  | (c - (isdigit(c) ? '0' : ((islower(c) ? 'a' : 'A') - 0xA)));
			}
			if (n == 0)
				goto badescape;
			UNGETC(c);
			*buf = n;
			break;
		}
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
			int n = 0;
			do {
				n = (n << 3) | (c - '0');
				c = GETC();
			} while (isodigit(c));
			if (n == 0)
				goto badescape;
			UNGETC(c);
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
	case '#':
		while ((c = GETC()) != '\n') /* skip comment until newline */
			if (c == EOF)
				return ENDFILE;
		/* FALLTHROUGH */
	case '\n':
		input->lineno++;
		newline = TRUE;
		w = NW;
		return NL;
	case '(':
		if (w == RW)	/* not keywords, so let & friends work */
			c = SUB;
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
		c = GETC();
		if (c == '&')
			return ANDAND;
		UNGETC(c);
		return '&';

	case '|': {
		int p[2];
		w = NW;
		c = GETC();
		if (c == '|')
			return OROR;
		if (!getfds(p, c, 1, 0))
			return ERROR;
		if (p[1] == CLOSED) {
			scanerror("expected digit after '='");	/* can't close a pipe */
			return ERROR;
		}
		y->tree = mk(nPipe, p[0], p[1]);
		return PIPE;
	}

	{
		char *cmd;
		int fd[2];
	case '<':
		fd[0] = 0;
		if ((c = GETC()) == '>')
			if ((c = GETC()) == '>') {
				c = GETC();
				cmd = "%open-append";
			} else
				cmd = "%open-write";
		else if (c == '<')
			if ((c = GETC()) == '<') {
				c = GETC();
				cmd = "%here";
			} else
				cmd = "%heredoc";
		else if (c == '=')
			return CALL;
		else
			cmd = "%open";
		goto redirection;
	case '>':
		fd[0] = 1;
		if ((c = GETC()) == '>')
			if ((c = GETC()) == '<') {
				c = GETC();
				cmd = "%open-append";
			} else
				cmd = "%append";
		else if (c == '<') {
			c = GETC();
			cmd = "%open-create";
		} else
			cmd = "%create";
		goto redirection;
	redirection:
		w = NW;
		if (!getfds(fd, c, fd[0], DEFAULT))
			return ERROR;
		if (fd[1] != DEFAULT) {
			y->tree = (fd[1] == CLOSED)
					? mkclose(fd[0])
					: mkdup(fd[0], fd[1]);
			return DUP;
		}
		y->tree = mkredircmd(cmd, fd[0]);
		return REDIR;
	}

	default:
		assert(c != '\0');
		w = NW;
		return c; /* don't know what it is, let yacc barf on it */
	}
}

extern void inityy(void) {
	newline = FALSE;
	w = NW;
	if (bufsize > BUFMAX) {		/* return memory to the system if the buffer got too large */
		efree(tokenbuf);
		tokenbuf = NULL;
	}
	if (tokenbuf == NULL) {
		bufsize = BUFSIZE;
		tokenbuf = ealloc(bufsize);
	}
}
