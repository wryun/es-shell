# runtime.es -- set up initial interpreter runtime ($Revision: 1.1.1.1 $)

#
# Es entry points
#

#
# %main is the entry point to the shell.  It parses the binary's argv,
# initializes runflags (described in detail below), runs .esrc (if login),
# and hands off execution to `%run-file` or `%run-string`.
#
# In several of the functions in this file, we call '$fn-<foo>' instead
# of the usual '<foo>'.  This is to prevent these infrastructural
# functions from mangling the expected value of '$0'.
#

# A little tricky, but these primitives are only available at dump time, so
# lexically binding their output here lets us cache the results of conditional
# compilation for use at shell startup.  Note that build-time lexical bindings
# like these MUST NOT be reassigned to GC'd values!  Treat them as read-only!
let (
	usage = <=$&usage
	runflag-flagpairs = (
		(e exitonfalse)
		(i interactive)
		(v echoinput)
		(x printcmds)
		(n noexec)
		(l login)
		# (L lisptrees) (G gcverbose) (I gcinfo)
		<=$&conditionalflags
	)
)
let (
	runflag-args = <={let (accum = ()) for ((a _) = $runflag-flagpairs) accum = $accum $a}
)
fn %main argv {
	fn-%main = ()

	let (
		es = es
		flags = ()
		cmd = ()
		runcmd = false
		keepclosed = false
		stdin = false
		allowdumps = false
		protected = false

		fn usage {
			echo $usage
			exit 1
		}
	) {
		if {!~ $#argv 0} {
			(es argv) = $argv
		}
		if {~ $es -*} {
			flags = $flags login
		}

		let (rfarg = ()) {
			argv = <={%getopt @ arg c {
				if {~ $arg --} {throw break-getopt}
				match $c (
					c {runcmd = true; cmd = <=optarg}
					o {keepclosed = true}
					d {allowdumps = true}
					p {protected = true}
					s {stdin = true; throw break-getopt}
					$runflag-args {rfarg = $rfarg $f}
					* {usage}
				)
			} $argv}
			for ((arg flag) = $runflag-flagpairs) {
				if {~ $arg $rfarg} {
					flags = $flags $flag
				}
			}
		}

		if {$runcmd && ~ $cmd ()} {
			usage >[1=2]
		}

		if {!$keepclosed} {
			if {!access /dev/stdin} {exec {< /dev/null}}
			if {!access /dev/stdout} {exec {> /dev/null}}
			if {!access /dev/stderr} {exec {>[2] /dev/null}}
		}

		if {$stdin && !~ $cmd ()} {
			echo >[1=2] 'es: -s and -c are incompatible'
			exit 1
		}

		if {~ $cmd () && {$stdin || ~ $#argv 0} && $&isatty 0} {
			flags = $flags interactive
		}
		runflags = $flags

		let (s = $signals) {
			if {~ $runflags interactive || !~ $s sigint} {
				s = $s .sigint
			}
			if {!$allowdumps} {
				if {~ $runflags interactive} {
					s = $s /sigterm
				}
				if {~ $runflags interactive || !~ $signals sigquit} {
					s = $s /sigquit
				}
			}
			signals = $s
		}

		if {!$protected} {
			$&importenvfuncs
		}

		if {~ $runflags login} {
			catch @ e type msg {
				if {~ $e exit} {
					throw $e $type $msg
				} {~ $e error} {
					echo >[1=2] $msg
				} {~ $e signal && ~ $type sigint} {
					# sigint: the silent signal
				} {
					echo >[1=2] uncaught exception: $e $type $msg
				}
			} {
				. ~/.esrc
			}
		}

		# This is the main "root" exception handler for the shell.
		catch @ e type msg {
			if {~ $e exit} {
				throw $e $type $msg
			} {~ $e error} {
				echo >[1=2] $msg
			} {~ $e signal && ~ $type sigint} {
				# sigint: the silent signal
			} {
				echo >[1=2] uncaught exception: $e $type $msg
			}
			result 1
		} {
			if {~ $cmd () && !$stdin && !~ $#argv 0} {
				local ((0 *) = $argv)
					$fn-%run-file $0
			} {!~ $cmd ()} {
				local ((0 *) = $es $argv)
					$fn-%run-string $cmd
			} {
				local ((0 *) = $es $argv)
					$fn-%run-file
			}
		}
	}
}


# %dot is the engine for running outside scripts in the current shell.
# It manages runflags based on args passed to it, sets $0 and $*, and
# calls `%run-file`.

fn %dot args {
	let (
		fn usage {
			throw error %dot 'usage: . [-einvx] file [arg ...]'
		}
		flags = ()
	) {
		args = <={%getopt @ arg c {
			if {~ $arg --} {throw break-getopt}
			match $c (
				e {flags = $flags exitonfalse}
				i {flasg = $flags interactive}
				n {flags = $flags noexec}
				v {flags = $flags echoinput}
				x {flags = $flags printcmds}
				* {usage}
			)
		} $args}
		if {~ $#args 0} {usage}
		local (
			0 = $args(1)
			* = $args(2 ...)
			runflags = $flags
		) $fn-%run-file $args(1)
	}
}

fn-. = %dot


# %run-file wraps the '$&runfile' primitive with a default command
# (which calls one of the REPL functions defined below).  When called
# on its own, the function is a worse (but technically passable)
# version of %dot.

fn %run-file file {
	$&runfile {
		if {~ $runflags interactive} {
			$fn-%interactive-loop
		} {
			$fn-%batch-loop
		}
	} $file
}

# %run-string is the same as %run-file, but instead of taking a file
# as an argument to read, it takes a string to parse and run.

fn-%run-string = $&runstring {$fn-%dispatch <=%parse}


#
# runflags
#

# Es execution is configured with the $runflags variable, which is
# similar to `$-` in POSIX shells.  Unlike `$-`, the value of
# $runflags may be modified, and will (typically) modify the shell's
# behavior when changed.
#
# $runflags may contain arbitrary words.  The only words that have any
# effect by default are
#  - exitonfalse
#  - interactive
#  - noexec
#  - echoinput
#  - printcmds
# and, if support is compiled in,
#  - lisptrees
#  - gcinfo
#  - gcverbose

fn-%is-interactive = {~ $runflags interactive}

set-runflags = @ new {
	let (nf = ()) {
		for (flag = $new) {
			if {!~ $nf $flag} {nf = $nf $flag}
		}
		let (
			dp-p = <={if {~ $nf printcmds} {result print} {result noprint}}
			dp-e = <={if {~ $nf noexec} {result noeval} {result eval}}
			dp-f = <={if {~ $nf exitonfalse} {result %exit-on-false} {result ()}}
		) fn-%dispatch = $dp-f $(fn-%^$(dp-e)^-^$(dp-p))
		$&setrunflags $nf
	}
}


# These functions are potentially passed to a REPL in the %dispatch function,
# depending on the value of $runflags.  (For %eval-noprint, note that an empty
# list prepended to a command just causes the command to be executed.)

fn %eval-noprint				# <default>
fn %eval-print		{ echo $* >[1=2]; $* }	# -x
fn %noeval-noprint	{ }			# -n
fn %noeval-print	{ echo $* >[1=2] }	# -n -x
fn-%exit-on-false = $&exitonfalse		# -e

noexport = $noexport fn-%dispatch runflags


#
# Read-eval-print loops
#

#	es contains two read-eval-print loops (REPLs) which are central to the
#	operation of the shell.  It is the responsibility of the REPL
#	to call the parser for reading commands, hand those commands to an
#	appropriate dispatch function, and handle any exceptions that may be
#	raised.  The REPLs are invoked by the %run-file function, which is
#	itself typically called by either %main or %dot.
#
#	%interactive-loop is invoked if the -i flag is used or if the shell
#	determines that its input is interactive; otherwise, %batch-loop is
#	used.  The 'interactive' runflag can be used to determine whether the
#	most closely binding REPL is interactive or batch.
#
#	The function %parse can be used to call the parser, which returns
#	an es command.  %parse takes two arguments, which are used as the
#	main and secondary prompts, respectively.  %parse typically returns
#	one line of input, but es allows commands (notably those with braces
#	or backslash continuations) to continue across multiple lines; in
#	that case, the complete command and not just one physical line is
#	returned.
#
#	The %parse function raises the eof exception when it encounters
#	an end-of-file on input.  You can probably simulate the C shell's
#	ignoreeof by restarting appropriately in this circumstance.
#	Other than eof, %interactive-loop does not exit on exceptions,
#	where %batch-loop does.
#
#	By convention, the REPL must pass commands to the fn %dispatch,
#	which has the actual responsibility for executing the command.
#
#	The looping construct forever is used rather than while, because
#	while catches the break exception, which would make it difficult
#	to print ``break outside of loop'' errors.
#
#	The parsed code is executed only if it is non-empty, because otherwise
#	result gets set to zero when it should not be.

fn-%parse = $&parse

fn %batch-loop {
	let (result = <=true) {
		catch @ e rest {
			if {~ $e eof} {
				return $result
			} {
				throw $e $rest
			}
		} {
			forever {
				let (cmd = <=%parse) {
					if {!~ $#cmd 0} {
						result = <={$fn-%dispatch $cmd}
					}
				}
			}
		}
	}
}

fn %interactive-loop {
	let (result = <=true) {
		catch @ e type msg {
			if {~ $e eof} {
				return $result
			} {~ $e exit} {
				throw $e $type $msg
			} {~ $e error} {
				echo >[1=2] $msg
				$fn-%dispatch false
			} {~ $e signal} {
				if {!~ $type sigint sigterm sigquit} {
					echo >[1=2] caught unexpected signal: $type
				}
			} {
				echo >[1=2] uncaught exception: $e $type $msg
			}
			throw retry # restart forever loop
		} {
			forever {
				if {!~ $#fn-%prompt 0} {
					%prompt
				}
				let (code = <={%parse $prompt}) {
					if {!~ $#code 0} {
						result = <={$fn-%dispatch $code}
					}
				}
			}
		}
	}
}
