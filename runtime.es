# runtime.es -- set up initial interpreter runtime ($Revision: 1.1.1.1 $)

#
# Read-eval-print loops
#

#	In es, the main read-eval-print loop (REPL) can lie outside the
#	shell itself.  Es can be run in one of two modes, interactive or
#	batch, and there is a hook function for each form.  It is the
#	responsibility of the REPL to call the parser for reading commands,
#	hand those commands to an appropriate dispatch function, and handle
#	any exceptions that may be raised.  The 'interactive' runflag can be
#	used to determine whether the most closely binding REPL is interactive
#	or batch.
#
#	The REPLs are invoked by the shell's es:main function or the . or
#	eval builtins.  If the -i flag is used or the shell determines that
#	it's input is interactive, %interactive-loop is invoked; otherwise
#	%batch-loop is used.
#
#	The function %parse can be used to call the parser, which returns
#	an es command.  %parse takes two arguments, which are used as the
#	main and secondary prompts, respectively.  %parse typically returns
#	one line of input, but es allows commands (notably those with braces
#	or backslash continuations) to continue across multiple lines; in
#	that case, the complete command and not just one physical line is
#	returned.
#
#	By convention, the REPL must pass commands to the fn %dispatch,
#	which has the actual responsibility for executing the command.
#	Whatever routine invokes the REPL has the responsibility of setting up
#	fn %dispatch appropriately; it is used for implementing the -e, -n, and
#	-x options.  Typically, fn %dispatch is locally bound.
#
#	The %parse function raises the eof exception when it encounters
#	an end-of-file on input.  You can probably simulate the C shell's
#	ignoreeof by restarting appropriately in this circumstance.
#	Other than eof, %interactive-loop does not exit on exceptions,
#	where %batch-loop does.
#
#	The looping construct forever is used rather than while, because
#	while catches the break exception, which would make it difficult
#	to print ``break outside of loop'' errors.
#
#	The parsed code is executed only if it is non-empty, because otherwise
#	result gets set to zero when it should not be.

fn-%parse	= $&parse

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
						result = <={$fn-%dispatch $cmd(1)}
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

#
# Es entry points
#

# Es execution is configured with the $runflags variable, which is
# similar to `$-` in POSIX shells.  Unlike `$-`, the value of
# $runflags may be modified, and will (typically) modify the shell's
# behavior when changed.
#
# $runflags may contain arbitrary words.  The only words that have an
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
#
# In several of the following functions, we call '$fn-<foo>' instead
# of the usual '<foo>'.  This is to prevent these infrastructural
# functions from mangling the expected value of '$0'.

set-runflags = @ new {
	let (nf = ()) {
		for (flag = $new) {
			if {!~ $nf $flag} {nf = $nf $flag}
		}
		let (
			dp-p = <={if {~ $nf printcmds} {result 'print'} {result 'noprint'}}
			dp-e = <={if {~ $nf noexec} {result 'noeval'} {result 'eval'}}
			dp-f = <={if {~ $nf exitonfalse} {result '%exit-on-false'} {result ()}}
		) fn-%dispatch = $dp-f $(fn-%^$(dp-e)^-^$(dp-p))
		$&setrunflags $nf
	}
}


# These functions are potentially passed to a REPL as the %dispatch
# function.  (For %eval-noprint, note that an empty list prepended
# to a command just causes the command to be executed.)

fn %eval-noprint				# <default>
fn %eval-print		{ echo $* >[1=2]; $* }	# -x
fn %noeval-noprint	{ }			# -n
fn %noeval-print	{ echo $* >[1=2] }	# -n -x
fn-%exit-on-false = $&exitonfalse		# -e

noexport = $noexport fn-%dispatch runflags

# %run-file wraps the '$&runfile' primitive with a default command
# (which calls one of the REPL functions defined above).  When called
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
		for (a = $args) {
			if {!~ $a -*} {
				break
			}
			args = $args(2 ...)
			if {~ $a --} {
				break
			}
			for (f = <={%fsplit '' <={~~ $a -*}}) {
				match $f (
					e {flags = $flags exitonfalse}
					i {flags = $flags interactive}
					n {flags = $flags noexec}
					v {flags = $flags echoinput}
					x {flags = $flags printcmds}
					* {usage}
				)
			}
		}
		if {~ $#args 0} {usage}
		local (
			0 = $args(1)
			* = $args(2 ...)
			runflags = $flags
		) $fn-%run-file $args(1)
	}
}

fn-. = %dot


#
# es:main is the entry point to the shell.  It parses the binary's argv,
# initializes runflags, runs .esrc (if appropriate), and starts the correct
# run loop.
#

es:main = @ argv {
	es:main = ()

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
			echo 'usage: es [-c command] [-silevxnpo] [file [args ...]]'
			echo '	-c cmd	execute argument'
			echo '	-s	read commands from standard input; stop option parsing'
			echo '	-i	interactive shell'
			echo '	-l	login shell'
			echo '	-e	exit if any command exits with false status'
			echo '	-v	print input to standard error'
			echo '	-x	print commands to standard error before executing'
			echo '	-n	just parse; don''t execute'
			echo '	-p	don''t load functions from the environment'
			echo '	-o	don''t open stdin, stdout, and stderr if they were closed'
			echo '	-d	don''t ignore SIGQUIT or SIGTERM'
			echo '	-I	print garbage collector information (if compiled in)'
			echo '	-G	print verbose garbage collector information (if compiled in)'
			echo '	-L	print parser results in LISP format (if compiled in)'
			exit 1
		}
	) {
		if {!~ $#argv 0} {
			(es argv) = $argv
		}
		if {~ $es -*} {
			flags = $flags login
		}

		for (a = $argv) {
			if {!~ $a -*} {
				break
			}
			argv = $argv(2 ...)
			if {~ $a --} {
				break
			}
			for (f = <={%fsplit '' <={~~ $a -*}}) {
				match $f (
					c {runcmd = true; (cmd argv) = $argv}
					e {flags = $flags exitonfalse}
					i {flags = $flags interactive}
					v {flags = $flags echoinput}
					L {flags = $flags lisptrees}
					x {flags = $flags printcmds}
					n {flags = $flags noexec}
					l {flags = $flags login}
					G {flags = $flags gcverbose}
					I {flags = $flags gcinfo}
					o {keepclosed = true}
					d {allowdumps = true}
					p {protected = true}
					s {stdin = true; break}
					* {usage}
				)
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
				} {
					echo >[1=2] uncaught exception: $e $type $msg
				}
			} {
				. ~/.esrc
			}
		}

		if {~ $cmd () && !$stdin && !~ $#argv 0} {
			local ((0 *) = $argv)
				$fn-%run-file $0
		} {!~ $cmd ()} {
			local ((0 *) = $es $argv)
				$&runstring {
					let (c = <=%parse) {
						# echo >[1=2] $c
						$fn-%dispatch $c
					}
				} $cmd
		} {
			local ((0 *) = $es $argv)
				$fn-%run-file
		}
	}
}
