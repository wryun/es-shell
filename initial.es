# initial.es -- initial machine state

#
# syntactic sugar
#

fn-%and		= $&and
fn-%append	= $&append
fn-%background	= $&background
fn-%backquote	= $&backquote
fn-%close	= $&close
fn-%count	= $&count
fn-%create	= $&create
fn-%dup		= $&dup
fn-%fork	= $&fork
fn-%flatten	= $&flatten
fn-%fsplit	= $&fsplit
fn-%here	= $&here
fn-%not		= $&not
fn-%one		= $&one
fn-%open	= $&open
fn-%or		= $&or
fn-%pipe	= $&pipe
fn-%seq		= $&seq
fn-%split	= $&split

#
# primitive functions
#

fn-.		= $&dot
fn-break	= $&break
fn-catch	= $&catch
fn-cd		= $&cd
fn-echo		= $&echo
fn-eval		= $&eval
fn-exec		= $&exec
fn-exit		= $&exit
fn-false	= $&false
fn-if		= $&if
fn-newpgrp	= $&newpgrp
fn-noexport	= $&noexport
fn-throw	= $&throw
fn-true		= $&true
fn-umask	= $&umask
fn-whatis	= $&whatis
fn-while	= $&while

#
# predefined functions
#

fn-break	= throw break
fn-return	= throw return

#
# settor functions
#

set-home = @ { let (set-HOME = ) HOME = $*; return $* }
set-HOME = @ { let (set-home = ) home = $*; return $* }

set-path = @ { let (set-PATH = ) PATH = <>{%flatten : $*}; return $* }
set-PATH = @ { let (set-path = ) path = <>{%fsplit : $*}; return $* }

set-cdpath = @ { let (set-CDPATH = ) CDPATH = <>{%flatten : $*}; return $* }
set-CDPATH = @ { let (set-cdpath = ) cdpath = <>{%fsplit : $*}; return $* }

set-prompt = $&setprompt
set-history = $&sethistory

#
# predefined variables
#

ifs = ' ' \t \n
prompt = ';; ' ''
home = /		# so home definitely exists, even if wrong
