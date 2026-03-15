/* heredoc.c -- in-line files (here documents) ($Revision: 1.1.1.1 $) */

#include "es.h"
#include "gc.h"
#include "input.h"
#include "syntax.h"

struct Here {
	Here *next;
	Tree *marker;
};

/* getherevar -- read a variable from a here doc */
extern Tree *getherevar(Parser *p) {
	int c;
	char *s;
	size_t len;
	Buffer *buf = openbuffer(0);
	while ((c = get(p)) != EOF && !dnw[c])
		buf = bufputc(buf, c);
	len = buf->len;
	s = psealcountedbuffer(buf);
	if (len == 0) {
		yyerror(p, "null variable name in here document");
		return NULL;
	}
	if (c != '^')
		unget(p, c);
	return flatten(mk(nVar, mk(nWord, s)), " ");
}

/* snarfheredoc -- read a heredoc until the eof marker */
extern Tree *snarfheredoc(Parser *p, const char *eof, Boolean quoted) {
	Tree *tree, **tailp;
	Buffer *buf;
	unsigned char *s;

	assert(quoted || strchr(eof, '$') == NULL);	/* can never be typed (whew!) */
	if (strchr(eof, '\n') != NULL) {
		yyerror(p, "here document eof-marker contains a newline");
		return NULL;
	}

	for (tree = NULL, tailp = &tree, buf = openbuffer(0);;) {
		int c;
		print_prompt2(p);
		for (s = (unsigned char *) eof; (c = get(p)) == *s; s++)
			;
		if (*s == '\0' && (c == '\n' || c == EOF)) {
			if (buf->current == 0 && tree != NULL)
				freebuffer(buf);
			else
				*tailp = treecons(mk(nQword, psealcountedbuffer(buf)), NULL);
			break;
		}
		if (s != (unsigned char *) eof)
			buf = bufncat(buf, eof, s - (unsigned char *) eof);
		for (;; c = get(p)) {
			if (c == EOF) {
				yyerror(p, "incomplete here document");
				freebuffer(buf);
				p->input->eof = FALSE;
				return NULL;
			}
			if (c == '$' && !quoted && (c = get(p)) != '$') {
				Tree *var;
				unget(p, c);
				if (buf->current == 0)
					freebuffer(buf);
				else {
					*tailp = treecons(mk(nQword, psealcountedbuffer(buf)), NULL);
					tailp = &(*tailp)->CDR;
				}
				var = getherevar(p);
				if (var == NULL) {
					freebuffer(buf);
					p->input->eof = FALSE;
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
	return tree->CDR == NULL ? tree->CAR : tree;
}

/* readheredocs -- read all the heredocs at the end of a line (or fail if at end of file) */
extern Boolean readheredocs(Parser *p, Boolean endfile) {
	for (; p->hereq != NULL; p->hereq = p->hereq->next) {
		Tree *marker, *eof;
		if (endfile) {
			yyerror(p, "end of file with pending here documents");
			return FALSE;
		}
		marker = p->hereq->marker;
		eof = marker->CAR;
		marker->CAR = snarfheredoc(p, eof->u[0].s, eof->kind == nQword);
		if (marker->CAR == NULL)
			return FALSE;
	}
	return TRUE;
}

/* queueheredoc -- add a heredoc to the queue to process at the end of the line */
extern Boolean queueheredoc(Parser *p, Tree *t) {
	Tree *eof;
	Here *here;

	assert(p->hereq == NULL || p->hereq->marker->kind == nList);
	assert(t->kind == nList);
	assert(t->CAR->kind == nWord);
#if !REISER_CPP
	assert(streq(t->CAR->u[0].s, "%heredoc"));
#endif
	t->CAR->u[0].s = "%here";
	assert(t->CDR->kind == nList);
	eof = t->CDR->CDR;
	assert(eof->kind == nList);
	if (!eof->CAR || (eof->CAR->kind != nWord && eof->CAR->kind != nQword)) {
		yyerror(p, "here document eof-marker not a single literal word");
		return FALSE;
	}

	here = palloc(sizeof (Here), NULL);
	here->next = p->hereq;
	here->marker = eof;
	p->hereq = here;
	return TRUE;
}
