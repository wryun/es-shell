# initial.es -- set up initial machine state ($Revision: 1.22 $)

#
# syntactic sugar
#

fn-%and		= $&and
fn-%append	= $&append
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

fn %background cmd {
	let (pid = <>{$&background $cmd}) {
		if {%is-interactive} {
			echo $pid >[1=2]
		}
		apid = $pid
	}
}


#
# internal shell mechanisms (non-syntax hooks)
#

fn-%batch-loop	= $&batchloop
fn-%home	= $&home
fn-%is-interactive = $&isinteractive
fn-%newfd	= $&newfd
fn-%parse	= $&parse
fn-%whatis	= $&whatis
fn-%var		= $&var

fn %pathsearch name { access -n $name -1e -xf $path }
fn %cdpathsearch name {
	let (dir = <>{access -n $name -1e -d  $cdpath}) {
		if {!~ $dir $name} {
			echo >[1=2] $dir
		}
		$&result $dir
	}
}

$&if {~ <>$&primitives execfailure} {fn-%exec-failure = $&execfailure}

#
# the read-eval-print loop
#

fn %interactive-loop dispatch {
	let (result = <>$&true) {
		catch @ e from msg {
			if {~ $e eof} {
				return $result
			} {~ $e error} {
				echo >[1=2] $msg
			} {!~ $e(1) signal && !~ $e(2) sigint} {
				echo >[1=2] uncaught exception: $e $from $msg
			}
			throw retry
		} {
			forever {
				if {!~ $#fn-%prompt 0} {
					%prompt
				}
				result = <>{$dispatch <>{%parse $prompt}}
			}
		}
	}
}

prompt = '; ' ''

# the dispatch functions
fn %eval-noprint				# <default>
fn %eval-print		{ echo $* >[1=2] $* }	# -x
fn %noeval-noprint	{ }			# -n
fn %noeval-print	{ echo $* >[1=2] }	# -n -x

fn-%exit-on-false = $&exitonfalse

#
# builtins
#

fn-.		= $&dot
fn-access	= $&access
fn-break	= $&break
fn-catch	= $&catch
fn-cd		= $&cd
fn-echo		= $&echo
fn-eval		= $&eval
fn-exec		= $&exec
fn-exit		= $&exit
fn-false	= $&false
fn-forever	= $&forever
fn-fork		= $&fork
fn-if		= $&if
fn-newpgrp	= $&newpgrp
fn-result	= $&result
fn-throw	= $&throw
fn-true		= $&true
fn-umask	= $&umask
fn-unwind-protect = $&unwindprotect
fn-wait		= $&wait
fn-while	= $&while
$&if {~ <>$&primitives limit} {fn-limit = $&limit}
$&if {~ <>$&primitives time}  {fn-time  = $&time}

#
# predefined functions
#

fn-break	= throw break
fn-return	= throw return

fn apids	{ echo <>$&apids }
fn var		{ for (i = $*) echo <>{%var $i} }
fn whatis	{ for (i = $*) echo <>{%whatis $i} }

fn vars {
	if {~ $* -a} {
		* = -v -f -s -e -p -i
	} {
		if {!~ $* -[vfs]}	{ * = $* -v }
		if {!~ $* -[epi]}	{ * = $* -e }
	}
	for (i = $*)
		if {!~ $i -[vfsepi]} {
			throw error vars illegal option: $i -- usage: vars '-[vfsepi]'
		}
	let (
		vars	= $&false
		fns	= $&false
		sets	= $&false
		export	= $&false
		priv	= $&false
		intern	= $&false
	) {
		for (i = $*) if (
			{~ $i -v}	{vars	= $&true}
			{~ $i -f}	{fns	= $&true}
			{~ $i -s}	{sets	= $&true}
			{~ $i -e}	{export	= $&true}
			{~ $i -p}	{priv	= $&true}
			{~ $i -i}	{intern = $&true}
			{throw error vars var: bad option: $i}
		)
		let (
			dovar = @ var {
				if {if {~ $var fn-*} $fns {~ $var set-*} $sets $vars} {
					echo <>{%var $var}
				}
			}
		) {
			if {$export || $priv} {
				for (var = <>$&vars)
					if {if {~ $var $noexport} $priv $export} {
						$dovar $var
					}
			}
			if {$intern} {
				for (var = <>$&internals)
					$dovar $var
			}
		}
	}
}

#
# >{} and <{} redirections -- either use /dev/fd (builtin) or /tmp
#

if {~ <>$&primitives readfrom} {
	fn-%readfrom = $&readfrom
} {
	fn %readfrom var cmd body {
		local ($var = /tmp/es.$var.$pid) {
			unwind-protect {
				$cmd > $$var
				$body
			} {
				rm -f $$var
			}
		}
	}
}

if {~ <>$&primitives writeto} {
	fn-%writeto = $&writeto
} {
	fn %writeto var cmd body {
		local ($var = /tmp/es.$var.$pid) {
			unwind-protect {
				> $$var
				$body
				$cmd < $$var
			} {
				rm -f $$var
			}
		}
	}
}


#
# settor functions
#

set-home = @ { local (set-HOME = ) HOME = $*; result $* }
set-HOME = @ { local (set-home = ) home = $*; result $* }

set-path = @ { local (set-PATH = ) PATH = <>{%flatten : $*}; result $* }
set-PATH = @ { local (set-path = ) path = <>{%fsplit  : $*}; result $* }

set-cdpath = @ { local (set-CDPATH = ) CDPATH = <>{%flatten : $*}; result $* }
set-CDPATH = @ { local (set-cdpath = ) cdpath = <>{%fsplit  : $*}; result $* }

set-history	= $&sethistory
set-signals	= $&setsignals
set-noexport	= $&setnoexport

#
# predefined variables
#

ifs = ' ' \t \n
home = /		# so home definitely exists, even if wrong
cdpath = ''
noexport = noexport apid

#
# title for the initial memory state
#

result es initial state built in `/bin/pwd on `/bin/date
