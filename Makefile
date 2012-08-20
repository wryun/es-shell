CC	= gcc
CFLAGS	= -g -O -Wall -D__LANGUAGE_C -D__mips

HFILES	= config.h es.h gc.h input.h prim.h print.h stdenv.h syntax.h
CFILES	= closure.c conv.c dict.c eval.c except.c gc.c glob.c glom.c \
	  input.c heredoc.c list.c main.c match.c open.c prim-ctl.c \
	  prim-etc.c prim-io.c prim.c print.c proc.c sigmsgs.c signal.c \
	  split.c status.c str.c syntax.c term.c token.c tree.c util.c \
	  var.c vec.c version.c which.c y.tab.c
OFILES	= closure.o conv.o dict.o eval.o except.o gc.o glob.o glom.o \
	  input.o heredoc.o list.o main.o match.o open.o prim-ctl.o \
	  prim-etc.o prim-io.o prim.o print.o proc.o sigmsgs.o signal.o \
	  split.o status.o str.o syntax.o term.o token.o tree.o util.o \
	  var.o vec.o version.o which.o y.tab.o

es	: ${OFILES}
	${CC} -o es ${OFILES}

y.tab.c y.tab.h : parse.y
	yacc -vd parse.y

sigmsgs.c sigmsgs.h : mksignal
	sh ../mksignal /usr/include/sys/signal.h

initial.h : initial.es
	sh mkinitial initial.es > initial.h

token.h : y.tab.h
	cmp -s y.tab.h token.h || cp y.tab.h token.h

# --- dependencies ---

closure.o : closure.c es.h config.h stdenv.h gc.h 
conv.o : conv.c es.h config.h stdenv.h print.h 
dict.o : dict.c es.h config.h stdenv.h gc.h 
eval.o : eval.c es.h config.h stdenv.h 
except.o : except.c es.h config.h stdenv.h print.h 
gc.o : gc.c es.h config.h stdenv.h gc.h 
glob.o : glob.c es.h config.h stdenv.h 
glom.o : glom.c es.h config.h stdenv.h gc.h 
input.o : input.c es.h config.h stdenv.h input.h 
heredoc.o : heredoc.c es.h config.h stdenv.h gc.h input.h syntax.h 
list.o : list.c es.h config.h stdenv.h gc.h 
main.o : main.c es.h config.h stdenv.h initial.h 
match.o : match.c es.h config.h stdenv.h 
open.o : open.c es.h config.h stdenv.h 
prim.o : prim.c es.h config.h stdenv.h prim.h 
prim-ctl.o : prim-ctl.c es.h config.h stdenv.h prim.h 
prim-etc.o : prim-etc.c es.h config.h stdenv.h prim.h sigmsgs.h 
prim-io.o : prim-io.c es.h config.h stdenv.h prim.h 
print.o : print.c es.h config.h stdenv.h print.h 
proc.o : proc.c es.h config.h stdenv.h 
signal.o : signal.c es.h config.h stdenv.h sigmsgs.h 
split.o : split.c es.h config.h stdenv.h gc.h 
status.o : status.c es.h config.h stdenv.h sigmsgs.h 
str.o : str.c es.h config.h stdenv.h gc.h print.h 
syntax.o : syntax.c es.h config.h stdenv.h input.h syntax.h 
term.o : term.c es.h config.h stdenv.h gc.h 
token.o : token.c es.h config.h stdenv.h input.h syntax.h token.h 
tree.o : tree.c es.h config.h stdenv.h gc.h 
util.o : util.c es.h config.h stdenv.h 
var.o : var.c es.h config.h stdenv.h gc.h 
vec.o : vec.c es.h config.h stdenv.h gc.h 
version.o : version.c es.h config.h stdenv.h 
which.o : which.c es.h config.h stdenv.h 
