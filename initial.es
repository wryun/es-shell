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
fn-%flatten	= $&flatten
fn-%fsplit	= $&fsplit
fn-%here	= $&here
fn-%not		= $&not
fn-%open	= $&open
fn-%or		= $&or
fn-%pipe	= $&pipe
fn-%seq		= $&seq
fn-%split	= $&split

#
# internal shell mechanisms (non-syntax hooks)
#

fn-%batch-loop	= $&batchloop
fn-%newfd	= $&newfd
fn-%parse	= $&parse
fn-%pathsearch	= $&pathsearch
fn-%whatis	= $&whatis
fn-%var		= $&var

#
# the read-eval-print loop
#

fn %interactive-loop {
	let (result = <>$&true) {
		catch @ e msg {
			if {~ $e eof} {
				return $result
			} {~ $e error} {
				echo >[1=2] $msg
			} {!~ $e(1) signal && !~ $e(2) sigint} {
				echo >[1=2] uncaught exception: $e $msg
			}
			throw retry
		} {
			while {} {
				%prompt
				result = <>{<>{%parse $prompt}}
			}
		}
	}
}

fn %prompt {}
prompt = '; ' ''

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
fn-fork		= $&fork
fn-if		= $&if
fn-isnoexport	= $&isnoexport
fn-newpgrp	= $&newpgrp
fn-noexport	= $&noexport
fn-throw	= $&throw
fn-true		= $&true
fn-umask	= $&umask
fn-while	= $&while

#
# predefined functions
#

fn-break	= throw break
fn-retry	= throw retry
fn-return	= throw return

fn var		{ for (i = $*) echo <>{%var $i} }
fn whatis	{ for (i = $*) echo <>{%whatis $i} }

fn vars {
	if {~ $* -a} {
		* = -v -f -s -e -p
	} {
		if {!~ $* -[vfs]}	{ * = $* -v }
		if {!~ $* -[ep]}	{ * = $* -e }
	}
	for (i = $*) if {!~ $i -[vfsep]} 

	let (
		vars	= $&false
		fns	= $&false
		sets	= $&false
		export	= $&false
		priv	= $&false
	) {
		for (i = $*) if (
			{~ $i -v}	{vars	= $&true}
			{~ $i -f}	{fns	= $&true}
			{~ $i -s}	{sets	= $&true}
			{~ $i -e}	{export	= $&true}
			{~ $i -p}	{priv	= $&true}
			{throw error var: bad option: $i}
		)
		for (var = <>{$&vars}) {
			if {isnoexport $var} $priv $export &&
			if {~ $var fn-*} $fns {~ $var set-*} $sets $vars &&
			echo <>{%var $var}
		}
	}
}

#
# settor functions
#

set-home = @ { local (set-HOME = ) HOME = $*; return $* }
set-HOME = @ { local (set-home = ) home = $*; return $* }

set-path = @ { local (set-PATH = ) PATH = <>{%flatten : $*}; return $* }
set-PATH = @ { local (set-path = ) path = <>{%fsplit : $*}; return $* }

set-cdpath = @ { local (set-CDPATH = ) CDPATH = <>{%flatten : $*}; return $* }
set-CDPATH = @ { local (set-cdpath = ) cdpath = <>{%fsplit : $*}; return $* }

set-history = $&sethistory
set-signals = $&setsignals

#
# predefined variables
#

ifs = ' ' \t \n
home = /		# so home definitely exists, even if wrong
cdpath = ''
