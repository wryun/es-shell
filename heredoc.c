/* heredoc.c -- in-line files (here documents) ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"
#include "input.h"
#include "syntax.h"

typedef struct Here Here;
struct Here {
	Here *next;
	Tree *marker;
};

static Here *hereq;

/* getherevar -- read a variable from a here doc */
extern Tree *getherevar(void) {
	int c;
	char *s;
	Buffer *buf = openbuffer(0);
	while (!dnw[c = GETC()])
		buf = bufputc(buf, c);
	s = sealcountedbuffer(buf);
	if (buf->len == 0) {
		yyerror("null variable name in here document");
		return NULL;
	}
	if (c != '^')
		UNGETC(c);
	return flatten(mk(nVar, mk(nWord, s)), " ");
}

/* snarfheredoc -- read a heredoc until the eof marker */
extern Tree *snarfheredoc(const char *eof, Boolean quoted) {
	Tree *tree, **tailp;
	Buffer *buf;
	unsigned char *s;

	assert(quoted || strchr(eof, '$') == NULL);	/* can never be typed (whew!) */
	if (strchr(eof, '\n') != NULL) {
		yyerror("here document eof-marker contains a newline");
		return NULL;
	}
	disablehistory = TRUE;

	for (tree = NULL, tailp = &tree, buf = openbuffer(0);;) {
		int c;
		print_prompt2();
		for (s = (unsigned char *) eof; (c = GETC()) == *s; s++)
			;
		if (*s == '\0' && (c == '\n' || c == EOF)) {
			if (buf->current == 0 && tree != NULL)
				freebuffer(buf);
			else
				*tailp = treecons(mk(nQword, sealcountedbuffer(buf)), NULL);
			break;
		}
		if (s != (unsigned char *) eof)
			buf = bufncat(buf, eof, s - (unsigned char *) eof);
		for (;; c = GETC()) {
			if (c == EOF) {
				yyerror("incomplete here document");
				freebuffer(buf);
				disablehistory = FALSE;
				return NULL;
			}
			if (c == '$' && !quoted && (c = GETC()) != '$') {
				Tree *var;
				UNGETC(c);
				if (buf->current == 0)
					freebuffer(buf);
				else {
					*tailp = treecons(mk(nQword, sealcountedbuffer(buf)), NULL);
					tailp = &(*tailp)->CDR;
				}
				var = getherevar();
				if (var == NULL) {
					freebuffer(buf);
					disablehistory = FALSE;
					return NULL;
				}
				*tailp = treecons(var, NULL);
				tailp = &(*tailp)->CDR;
				buf = openbuffer(0);
				continue;
			}
			buf = bufputc(buf, c);
			if (c == '\n')
				break;
		}
	}

	disablehistory = FALSE;
	return tree->CDR == NULL ? tree->CAR : tree;
}

/* readheredocs -- read all the heredocs at the end of a line (or fail if at end of file) */
extern Boolean readheredocs(Boolean endfile) {
	for (; hereq != NULL; hereq = hereq->next) {
		Tree *marker, *eof;
		if (endfile) {
			yyerror("end of file with pending here documents");
			return FALSE;
		}
		marker = hereq->marker;
		eof = marker->CAR;
		marker->CAR = snarfheredoc(eof->u[0].s, eof->kind == nQword);
		if (marker->CAR == NULL)
			return FALSE;
	}
	return TRUE;
}

/* queueheredoc -- add a heredoc to the queue to process at the end of the line */
extern Boolean queueheredoc(Tree *t) {
	Tree *eof;
	Here *here;

	assert(hereq == NULL || hereq->marker->kind == nList);
	assert(t->kind == nList);
	assert(t->CAR->kind == nWord);
#if !REISER_CPP
	assert(streq(t->CAR->u[0].s, "%heredoc"));
#endif
	t->CAR->u[0].s = "%here";
	assert(t->CDR->kind == nList);
	eof = t->CDR->CDR;
	assert(eof->kind == nList);
	if (eof->CAR->kind != nWord && eof->CAR->kind != nQword) {
		yyerror("here document eof-marker not a single literal word");
		return FALSE;
	}

	here = gcalloc(sizeof (Here), NULL);
	here->next = hereq;
	here->marker = eof;
	hereq = here;
	return TRUE;
}

extern void emptyherequeue(void) {
	hereq = NULL;
	disablehistory = FALSE;
}
