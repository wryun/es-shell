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
fn-%newfd	= $&newfd
fn-%one		= $&one
fn-%open	= $&open
fn-%or		= $&or
fn-%pipe	= $&pipe
fn-%seq		= $&seq
fn-%split	= $&split
fn-%var		= $&var
fn-%whatis	= $&whatis

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
	if {!~ $* -[vfs]}	{ * = $* -v }
	if {!~ $* -[ep]}	{ * = $* -e }
	for (i = $*) if {!~ $i -[vfsep]} 
	local (
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
		for (var = <>{$&vars})
			if {isnoexport $var} $priv $export &&
			if (
				{~ $var fn-*}	$fns
				{~ $var set-*}	$sets
						$vars
			) &&
			var $var
	}
}

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
set-signals = $&setsignals

#
# predefined variables
#

ifs = ' ' \t \n
prompt = ';; ' ''
home = /		# so home definitely exists, even if wrong
cdpath = ''
