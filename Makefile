# Makefile for es ($Revision: 1.4 $)

CC	= cc
CFLAGS	= -g -O -Wall

HFILES	= config.h es.h gc.h input.h prim.h print.h stdenv.h syntax.h var.h
CFILES	= access.c closure.c conv.c dict.c eval.c except.c fd.c gc.c glob.c \
	  glom.c input.c heredoc.c list.c main.c match.c open.c opt.c \
	  prim-ctl.c prim-etc.c prim-io.c prim-sys.c prim.c print.c proc.c \
	  sigmsgs.c signal.c split.c status.c str.c syntax.c term.c token.c \
	  tree.c util.c var.c vec.c version.c y.tab.c dump.c
OFILES	= access.o closure.o conv.o dict.o eval.o except.o fd.o gc.o glob.o \
	  glom.o input.o heredoc.o list.o main.o match.o open.o opt.o \
	  prim-ctl.o prim-etc.o prim-io.o prim-sys.o prim.o print.o proc.o \
	  sigmsgs.o signal.o split.o status.o str.o syntax.o term.o token.o \
	  tree.o util.o var.o vec.o version.o y.tab.o
OTHER	= Makefile parse.y mksignal
GEN	= esdump y.tab.c y.tab.h y.output token.h sigmsgs.c sigmsgs.h initial.c

es	: ${OFILES} initial.o
	${CC} -o es ${OFILES} initial.o

esdump	: ${OFILES} dump.o
	${CC} -o esdump ${OFILES} dump.o

clean	:
	rm -f es ${OFILES} ${GEN} dump.o initial.o

src	:
	@echo ${OTHER} ${CFILES} ${HFILES}

y.tab.c y.tab.h : parse.y
	${YACC} -vd parse.y

token.h : y.tab.h
	-cmp -s y.tab.h token.h || cp y.tab.h token.h

initial.c : esdump initial.es
	./esdump < initial.es > initial.c

sigmsgs.c sigmsgs.h : mksignal /usr/include/sys/signal.h
	sh mksignal /usr/include/sys/signal.h

# --- dependencies ---

access.o : access.c es.h config.h stdenv.h prim.h 
closure.o : closure.c es.h config.h stdenv.h gc.h 
conv.o : conv.c es.h config.h stdenv.h print.h 
dict.o : dict.c es.h config.h stdenv.h gc.h 
eval.o : eval.c es.h config.h stdenv.h 
except.o : except.c es.h config.h stdenv.h print.h 
fd.o : fd.c es.h config.h stdenv.h 
gc.o : gc.c es.h config.h stdenv.h gc.h 
glob.o : glob.c es.h config.h stdenv.h gc.h 
glom.o : glom.c es.h config.h stdenv.h gc.h 
input.o : input.c es.h config.h stdenv.h input.h 
heredoc.o : heredoc.c es.h config.h stdenv.h gc.h input.h syntax.h 
list.o : list.c es.h config.h stdenv.h gc.h 
main.o : main.c es.h config.h stdenv.h 
match.o : match.c es.h config.h stdenv.h 
open.o : open.c es.h config.h stdenv.h 
opt.o : opt.c es.h config.h stdenv.h 
prim.o : prim.c es.h config.h stdenv.h prim.h 
prim-ctl.o : prim-ctl.c es.h config.h stdenv.h prim.h 
prim-etc.o : prim-etc.c es.h config.h stdenv.h prim.h 
prim-io.o : prim-io.c es.h config.h stdenv.h prim.h 
prim-sys.o : prim-sys.c es.h config.h stdenv.h prim.h sigmsgs.h 
print.o : print.c es.h config.h stdenv.h print.h 
proc.o : proc.c es.h config.h stdenv.h prim.h 
signal.o : signal.c es.h config.h stdenv.h sigmsgs.h 
split.o : split.c es.h config.h stdenv.h gc.h 
status.o : status.c es.h config.h stdenv.h sigmsgs.h 
str.o : str.c es.h config.h stdenv.h gc.h print.h 
syntax.o : syntax.c es.h config.h stdenv.h input.h syntax.h \
  token.h 
term.o : term.c es.h config.h stdenv.h gc.h 
token.o : token.c es.h config.h stdenv.h input.h syntax.h token.h 
tree.o : tree.c es.h config.h stdenv.h gc.h 
util.o : util.c es.h config.h stdenv.h 
var.o : var.c es.h config.h stdenv.h gc.h var.h 
vec.o : vec.c es.h config.h stdenv.h gc.h 
version.o : version.c es.h config.h stdenv.h 
