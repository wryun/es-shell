/* token.c -- lexical analyzer for es ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "input.h"
#include "syntax.h"
#include "token.h"

#define	isodigit(c)	('0' <= (c) && (c) < '8')

#define	BUFSIZE	((size_t) 2048)
#define	BUFMAX	(8 * BUFSIZE)

#define	InsertFreeCaret()	STMT(if (p->ws != NW) { p->ws = NW; unget(p, c); return '^'; })


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
extern void print_prompt2(Parser *p) {
	Input *in = p->input;
	in->lineno++;
#if HAVE_READLINE
	in->prompt = in->prompt2;
#else
	if ((p->input->runflags & run_interactive) && in->prompt2 != NULL)
		eprint("%s", in->prompt2);
#endif
}

/* scanerror -- called for lexical errors */
static void scanerror(Parser *p, int c, char *s) {
	while (c != '\n' && c != EOF)
		c = get(p);
	p->goterror = TRUE;
	yyerror(p, s);
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

static Boolean getfds(Parser *p, int fd[2], int c, int default0, int default1) {
	int n;
	fd[0] = default0;
	fd[1] = default1;

	if (c != '[') {
		unget(p, c);
		return TRUE;
	}
	if ((unsigned int) (n = get(p) - '0') > 9) {
		scanerror(p, n + '0', "expected digit after '['");
		return FALSE;
	}

	while ((unsigned int) (c = get(p) - '0') <= 9)
		n = n * 10 + c;
	fd[0] = n;

	switch (c += '0') {
	case '=':
		if ((unsigned int) (n = get(p) - '0') > 9) {
			if (n != ']' - '0') {
				scanerror(p, n + '0', "expected digit or ']' after '='");
				return FALSE;
			}
			fd[1] = CLOSED;
		} else {
			while ((unsigned int) (c = get(p) - '0') <= 9)
				n = n * 10 + c;
			if (c != ']' - '0') {
				scanerror(p, c + '0', "expected ']' after digit");
				return FALSE;
			}
			fd[1] = n;
		}
		break;
	case ']':
		break;
	default:
		scanerror(p, c, "expected '=' or ']' after digit");
		return FALSE;
	}
	return TRUE;
}

extern int yylex(YYSTYPE *y, Parser *p) {
	int c;
	size_t i;			/* The purpose of all these local assignments is to	*/
	const char *meta;		/* allow optimizing compilers like gcc to load these	*/
	char *buf = p->tokenbuf;	/* values into registers. */

	if (p->goterror) {
		p->goterror = FALSE;
		return NL;
	}

	/* rc variable-names may contain only alnum, '*' and '_', so use dnw if we are scanning one. */
	meta = (p->dollar ? dnw : nw);
	p->dollar = FALSE;
	if (p->newline) {
		--p->input->lineno; /* slight space optimization; print_prompt2() always increments lineno */
		print_prompt2(p);
		p->newline = FALSE;
	}
top:	while ((c = get(p)) == ' ' || c == '\t')
		p->ws = NW;
	if (c == EOF)
		return ENDFILE;
	if (!meta[(unsigned char) c]) {	/* it's a word or keyword. */
		InsertFreeCaret();
		p->ws = RW;
		i = 0;
		do {
			buf[i++] = c;
			if (i >= p->bufsize)
				buf = p->tokenbuf = erealloc(buf, p->bufsize *= 2);
		} while ((c = get(p)) != EOF && !meta[(unsigned char) c]);
		unget(p, c);
		buf[i] = '\0';
		p->ws = KW;
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
		else if (streq(buf, "match"))
			return MATCH;
		p->ws = RW;
		y->str = pdup(buf);
		return WORD;
	}
	if (c == '`' || c == '!' || c == '$' || c == '\'' || c == '=') {
		InsertFreeCaret();
		if (c == '!' || c == '=')
			p->ws = KW;
	}
	switch (c) {
	case '!':
	case '=':
		return c;
	case '`':
		c = get(p);
		if (c == '`') {
			c = get(p);
			if (c == '^')
				return BBFLAT;
			unget(p, c);
			return BACKBACK;
		} else if (c == '^')
			return BFLAT;
		unget(p, c);
		return '`';
	case '$':
		p->dollar = TRUE;
		switch (c = get(p)) {
		case '#':	return COUNT;
		case '^':	return FLAT;
		case '&':	return PRIM;
		default:	unget(p, c); return '$';
		}
	case '\'':
		p->ws = RW;
		i = 0;
		while ((c = get(p)) != '\'' || (c = get(p)) == '\'') {
			buf[i++] = c;
			if (c == '\n')
				print_prompt2(p);
			if (c == EOF) {
				p->ws = NW;
				scanerror(p, c, "eof in quoted string");
				return ERROR;
			}
			if (i >= p->bufsize)
				buf = p->tokenbuf = erealloc(buf, p->bufsize *= 2);
		}
		unget(p, c);
		buf[i] = '\0';
		y->str = pdup(buf);
		return QWORD;
	case '\\':
		if ((c = get(p)) == '\n') {
			print_prompt2(p);
			unget(p, ' ');
			goto top; /* Pretend it was just another space. */
		}
		if (c == EOF) {
			unget(p, EOF);
			goto badescape;
		}
		unget(p, c);
		c = '\\';
		InsertFreeCaret();
		p->ws = RW;
		c = get(p);
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
				c = get(p);
				if (!isxdigit(c))
					break;
				n = (n << 4)
				  | (c - (isdigit(c) ? '0' : ((islower(c) ? 'a' : 'A') - 0xA)));
			}
			if (n == 0)
				goto badescape;
			unget(p, c);
			*buf = n;
			break;
		}
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
			int n = 0;
			do {
				n = (n << 3) | (c - '0');
				c = get(p);
			} while (isodigit(c));
			if (n == 0)
				goto badescape;
			unget(p, c);
			*buf = n;
			break;
		}
		default:
			if (isalnum(c)) {
			badescape:
				scanerror(p, c, "bad backslash escape");
				return ERROR;
			}
			*buf = c;
			break;
		}
		buf[1] = 0;
		y->str = pdup(buf);
		return QWORD;
	case '#':
		while ((c = get(p)) != '\n') /* skip comment until newline */
			if (c == EOF)
				return ENDFILE;
		FALLTHROUGH;
	case '\n':
		p->input->lineno++;
		p->newline = TRUE;
		p->ws = NW;
		return NL;
	case '(':
		if (p->ws == RW)	/* not keywords, so let & friends work */
			c = SUB;
		FALLTHROUGH;
	case ';':
	case '^':
	case ')':
	case '{': case '}':
		p->ws = NW;
		return c;
	case '&':
		p->ws = NW;
		c = get(p);
		if (c == '&')
			return ANDAND;
		unget(p, c);
		return '&';

	case '|': {
		int pi[2];
		p->ws = NW;
		c = get(p);
		if (c == '|')
			return OROR;
		if (!getfds(p, pi, c, 1, 0))
			return ERROR;
		if (pi[1] == CLOSED) {
			scanerror(p, c, "expected digit after '='");	/* can't close a pipe */
			return ERROR;
		}
		y->tree = mk(nPipe, pi[0], pi[1]);
		return PIPE;
	}

	{
		char *cmd;
		int fd[2];
	case '<':
		fd[0] = 0;
		if ((c = get(p)) == '>')
			if ((c = get(p)) == '>') {
				c = get(p);
				cmd = "%open-append";
			} else
				cmd = "%open-write";
		else if (c == '<')
			if ((c = get(p)) == '<') {
				c = get(p);
				cmd = "%here";
			} else
				cmd = "%heredoc";
		else if (c == '=') {
			p->ws = NW;
			return CALL;
		} else
			cmd = "%open";
		goto redirection;
	case '>':
		fd[0] = 1;
		if ((c = get(p)) == '>')
			if ((c = get(p)) == '<') {
				c = get(p);
				cmd = "%open-append";
			} else
				cmd = "%append";
		else if (c == '<') {
			c = get(p);
			cmd = "%open-create";
		} else
			cmd = "%create";
		goto redirection;
	redirection:
		p->ws = NW;
		if (!getfds(p, fd, c, fd[0], DEFAULT))
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
		p->ws = NW;
		return c; /* don't know what it is, let yacc barf on it */
	}
}

extern void inityy(Parser *p) {
	p->newline = p->dollar = p->goterror = FALSE;
	p->ws = NW;
	p->bufsize = BUFSIZE;
}
