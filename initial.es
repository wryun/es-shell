# initial.es -- set up initial interpreter state ($Revision: 1.1.1.1 $)


#
# Introduction
#

#	Initial.es contains a series of definitions used to set up the
#	initial state of the es virtual machine.  In early versions of es
#	(those before version 0.8), initial.es was turned into a string
#	which was evaluated when the program started.  This unfortunately
#	took a lot of time on startup and, worse, created a lot of garbage
#	collectible memory which persisted for the life of the shell,
#	causing a lot of extra scanning.
#
#	Since version 0.8, building es is a two-stage process.  First a
#	version of the shell called esdump is built.  Esdump reads and
#	executes commands from standard input, similar to a normal interpreter,
#	but when it is finished, it prints on standard ouput C code for
#	recreating the current state of interpreter memory.  (The code for
#	producing the C code is in dump.c.)  Esdump starts up with no
#	variables defined.  This file (initial.es) is run through esdump
#	to produce a C file, initial.c, which is linked with the rest of
#	the interpreter, replacing dump.c, to produce the actual es
#	interpreter.
#
#	Because the shell's memory state is empty when initial.es is run,
#	you must be very careful in what instructions you put in this file.
#	For example, until the definition of fn-%pathsearch, you cannot
#	run external programs other than those named by absolute path names.
#	An error encountered while running esdump is fatal.
#
#	The bulk of initial.es consists of assignments of primitives to
#	functions.  A primitive in es is an executable object that jumps
#	directly into some C code compiled into the interpreter.  Primitives
#	are referred to with the syntactic construct $&name;  by convention,
#	the C function implementing the primitive is prim_name.  The list
#	of primitives is available as the return value (exit status) of
#	the primitive $&primitives.  Primitives may not be reassigned.
#
#	Functions in es are simply variables named fn-name.  When es
#	evaluates a command, if the first word is a string, es checks if
#	an appropriately named fn- variable exists.  If it does, then the
#	value of that variable is substituted for the function name and
#	evaluation starts over again.  Thus, for example, the assignment
#		fn-echo = $&echo
#	means that the command
#		echo foo bar
#	is internally translated to
#		$&echo foo bar
#	This mechanism is used pervasively.
#
#	Note that definitions provided in initial.es can be overriden (aka,
#	spoofed) without changing this file at all, just by redefining the
#	variables.  The only purpose of this file is to provide initial
#	values.


#
# Builtin functions
#

#	These builtin functions are straightforward calls to primitives.
#	See the manual page for details on what they do.

fn-break	= $&break
fn-catch	= $&catch
fn-echo		= $&echo
fn-exec		= $&exec
fn-forever	= $&forever
fn-fork		= $&fork
fn-if		= $&if
fn-newpgrp	= $&newpgrp
fn-result	= $&result
fn-throw	= $&throw
fn-umask	= $&umask
fn-wait		= $&wait

fn-%read	= $&read

#	eval runs its arguments by turning them into a code fragment
#	(in string form) and running that fragment.

fn-eval = $&noreturn @ { '{' ^ $^* ^ '}' }

#	Through version 0.84 of es, true and false were primitives,
#	but, as many pointed out, they don't need to be.  These
#	values are not very clear, but unix demands them.

fn-true		= { result 0 }
fn-false	= { result 1 }

#	These functions just generate exceptions for control-flow
#	constructions.  The for command and the while builtin both
#	catch the break exception, and lambda-invocation catches
#	return.  The interpreter main() routine (and nothing else)
#	catches the exit exception.

fn-break	= throw break
fn-exit		= throw exit
fn-return	= throw return

#	unwind-protect is a simple wrapper around catch that is used
#	to ensure that some cleanup code is run after running a code
#	fragment.  This function must be written with care to make
#	sure that the return value is correct.

fn-unwind-protect = $&noreturn @ body cleanup {
	if {!~ $#cleanup 1} {
		throw error unwind-protect 'unwind-protect body cleanup'
	}
	let (exception = ) {
		let (
			result = <={
				catch @ e {
					exception = caught $e
				} {
					$body
				}
			}
		) {
			$cleanup
			if {~ $exception(1) caught} {
				throw $exception(2 ...)
			}
			result $result
		}
	}
}

#	These builtins are not provided on all systems, so we check
#	if the accompanying primitives are defined and, if so, define
#	the builtins.  Otherwise, we'll just not have a limit command
#	and get time from /bin or wherever.

if {~ <=$&primitives limit} {fn-limit = $&limit}
if {~ <=$&primitives time}  {fn-time  = $&time}

#	These builtins are mainly useful for internal functions, but
#	they're there to be called if you want to use them.

fn-%apids	= $&apids
fn-%fsplit      = $&fsplit
fn-%newfd	= $&newfd
fn-%run         = $&run
fn-%split       = $&split
fn-%var		= $&var
fn-%whatis	= $&whatis

#	These builtins are only around as a matter of convenience, so
#	users don't have to type the infamous <= (nee <>) operator.
#	Whatis also protects the used from exceptions raised by %whatis.

fn var		{ for (i = $*) echo <={%var $i} }

fn whatis {
	let (result = ) {
		for (i = $*) {
			catch @ e from message {
				if {!~ $e error} {
					throw $e $from $message
				}
				echo >[1=2] $message
				result = $result 1
			} {
				echo <={%whatis $i}
				result = $result 0
			}
		}
		result $result
	}
}

#	The while function is implemented with the forever looping primitive.
#	While uses $&noreturn to indicate that, while it is a lambda, it
#	does not catch the return exception.  It does, however, catch break.

fn-while = $&noreturn @ cond body {
	catch @ e value {
		if {!~ $e break} {
			throw $e $value
		}
		result $value
	} {
		let (result = <=true)
			forever {
				if {!$cond} {
					throw break $result
				} {
					result = <=$body
				}
			}
	}
}

#	The cd builtin provides a friendlier veneer over the cd primitive:
#	it knows about no arguments meaning ``cd $home'' and has friendlier
#	error messages than the raw $&cd.  (It also used to search $cdpath,
#	but that's been moved out of the shell.)

fn cd dir {
	if {~ $#dir 1} {
		$&cd $dir
	} {~ $#dir 0} {
		if {!~ $#home 1} {
			throw error cd <={
				if {~ $#home 0} {
					result 'cd: no home directory'
				} {
					result 'cd: home directory must be one word'
				}
			}
		}
		$&cd $home
	} {
		throw error cd 'usage: cd [directory]'
	}
}

#	The %getopt function is likely a mistake to include in the shell,
#	but multiple built-ins in es require some option-parsing behavior
#	and having a "librarified" version ensures some degree of consistency.
#
#	This version of %getopt is designed to take advantage of the features of
#	es.  Its first argument, the "optfunc", is a lambda expression which will
#	be repeatedly called to do the user's specific work.
#
#	The remainder of the arguments to %getopt are the arguments to be parsed.
#	%getopt will iterate through these until it finds one which doesn't match
#	-*, or until a break-getopt exception is thrown.  It will call optfunc
#	once for each option, passing the full option argument and the specific
#	option.  The full option argument is useful for things like testing for
#	`--`.  When %getopt finishes, either due to an exception or a non-option
#	argument, it will return the remainder of the arguments it was given.
#
#	%getopt dynamically binds the function `optarg`, which when called, will
#	remove the next argument from its iteration and return it to the caller.
#
#	%getopt is $&noreturn and it calls its optfunc as $&noreturn, in order to
#	allow the calling context to call return without any dynamic-behavior
#	surprises.

fn-%getopt = $&noreturn @ optfunc args {
	catch @ e rest {
		if {~ $e break-getopt} {
			result $args
		} {
			throw $e $rest
		}
	} {
		local (fn-optarg = {
			let (result = $args(1)) {
				args = $args(2 ...)
				result $result
			}
		})
		while {~ $args(1) -*} {
			catch @ e rest {
				if {!~ $e continue-getopt} {
					throw $e $rest
				}
			} {
				let (arg = $args(1)) {
					args = $args(2 ...)
					for (c = <={%fsplit '' <={~~ $arg -*}}) {
						$&noreturn $optfunc $arg $c
					}
				}
			}
		}
		result $args
	}
}


#	The vars function is provided for cultural compatibility with
#	rc's whatis when used without arguments.
#
#	The options to vars can be partitioned into two categories:
#	those which pick variables based on their source (-e for
#	exported variables, -p for unexported, and -i for internal)
#	and their type (-f for functions, -s for settor functions,
#	and -v for all others).
#
#	Internal variables are those defined in initial.es (along
#	with pid and path), and behave like unexported variables,
#	except that they are known to have an initial value.
#	When an internal variable is modified, it becomes exportable,
#	unless it is on the noexport list.

fn vars {
	let (
		all	= false
		vars	= false
		fns	= false
		sets	= false
		export	= false
		priv	= false
		intern	= false
		all	= false
	) {
		if {!~ <={%getopt @ arg c {
			match $c (
				a	{all	= true}
				v 	{vars	= true}
				f	{fns	= true}
				s	{sets	= true}
				e	{export	= true}
				p	{priv	= true}
				i	{intern	= true}
				*	{throw error vars vars: bad option: -$c -- usage: vars '-[vfsepia]'}
			)
		} $*} ()} {
			throw error vars vars: takes no positional arguments
		}
		if {!$vars && !$fns && !$sets} {
			vars = true
		}
		if {!$export && !$priv && !$intern} {
			export = true
		}
		let (
			fn dovar var {
				# print functions and/or settor vars
				if {$all || if {~ $var fn-*} $fns {~ $var set-*} $sets $vars} {
					echo <={%var $var}
				}
			}
		) {
			if {$all || $export || $priv} {
				for (var = <=$&vars)
					# if not exported but in priv
					if {$all || if {~ $var $noexport} $priv $export} {
						dovar $var
					}
			}
			if {$all || $intern} {
				for (var = <=$&internals)
					dovar $var
			}
		}
	}
}


#	The access function provides a more capable and convenient wrapper
#	around the $&access primitive: it can test multiple files at once,
#	perform path-like searching (in fact, it is used for path searching),
#	and react more thoroughly to successes and failures than $&access can.

fn access {
	let (
		perm = ''
		type = a

		suffix = ()
		first = false
		exception = false

		result = ()
	) {
		* = <={%getopt @ arg c {
			match $c (
				n {suffix = <=optarg}
				1 {first = true}
				e {exception = true}
				(r w x) {perm = $perm^$c}
				(f d c b l s p) {type = $c}
				* {usage}
			)
		} $*}

		let (r = ()) {
			for (file = $*) {
				if {!~ $suffix ()} {
					if {~ $file */} {
						file = $file$suffix
					} {
						file = $file/$suffix
					}
				}
				if {r = <={$&access $perm $type $file} && $first} {
					return $file
				}
				result = $result $r
			}
			if {$first && $exception} {
				throw error access <={%flatten ' ' $suffix^':' $r}
			}
		}
		result $result
	}
}


#
# Syntactic sugar
#

#	Much of the flexibility in es comes from its use of syntactic rewriting.
#	Traditional shell syntax is rewritten as it is parsed into calls
#	to ``hook'' functions.  Hook functions are special only in that
#	they are the result of the rewriting that goes on.  By convention,
#	hook function names begin with a percent (%) character.

#	One piece of syntax rewriting invokes no hook functions:
#
#		fn name args { cmd }	fn-^name=@ args{cmd}

#	The following expressions are rewritten:
#
#		$#var                  <={%count $var}
#		$^var                  <={%flatten ' ' $var}
#		`{cmd args}            <={%backquote <={%flatten '' $ifs} {cmd args}}
#		`^{cmd args}           <={%flatten ' ' <={backquote <={%flatten '' $ifs} {cmd args}}}
#		``ifs {cmd args}       <={%backquote <={%flatten '' ifs} {cmd args}}
#		``^ifs {cmd args}      <={%flatten ' ' <={backquote <={%flatten '' ifs} {cmd args}}}

fn-%count	= $&count
fn-%flatten	= $&flatten

#	Note that $&backquote returns the status of the child process
#	as the first value of its result list.  The default %backquote
#	puts that value in $bqstatus.

fn %backquote {
	let ((status output) = <={ $&backquote $* }) {
		bqstatus = $status
		result $output
	}
}

#	The following syntax for control flow operations are rewritten
#	using hook functions:
#
#		! cmd			%not {cmd}
#		cmd1; cmd2		%seq {cmd1} {cmd2}
#		cmd1 && cmd2		%and {cmd1} {cmd2}
#		cmd1 || cmd2		%or {cmd1} {cmd2}
#
#	Note that %seq is also used for newline-separated commands within
#	braces.  The logical operators are implemented in terms of if.
#
#	%and and %or are recursive, which is slightly inefficient given
#	the current implementation of es -- it is not properly tail recursive
#	-- but that can be fixed and it's still better to write more of
#	the shell in es itself.

fn-%seq		= $&seq

fn-%not = $&noreturn @ cmd {
	if {$cmd} {false} {true}
}

fn-%and = $&noreturn @ first rest {
	let (result = <={$first}) {
		if {~ $#rest 0} {
			result $result
		} {result $result} {
			%and $rest
		} {
			result $result
		}
	}
}

fn-%or = $&noreturn @ first rest {
	if {~ $#first 0} {
		false
	} {
		let (result = <={$first}) {
			if {~ $#rest 0} {
				result $result
			} {!result $result} {
				%or $rest
			} {
				result $result
			}
		}
	}
}

#	Background commands could use the $&background primitive directly,
#	but some of the user-friendly semantics ($apid, printing of the
#	child process id) were easier to write in es.
#
#		cmd &			%background {cmd}

fn %background cmd {
	let (bg = $&background) {
		if {~ $runflags interactive} {
			bg = newpgrp $&background
		}
		let (pid = <={$bg $cmd}) {
			apid = $pid
			if {~ $runflags interactive} {
				echo >[1=2] $pid
			}
		}
	}
}

#	These redirections are rewritten:
#
#		cmd < file		%open 0 file {cmd}
#		cmd > file		%create 1 file {cmd}
#		cmd >[n] file		%create n file {cmd}
#		cmd >> file		%append 1 file {cmd}
#		cmd <> file		%open-write 0 file {cmd}
#		cmd <>> file		%open-append 0 file {cmd}
#		cmd >< file		%open-create 1 file {cmd}
#		cmd >>< file		%open-append 1 file {cmd}
#
#	All the redirection hooks reduce to calls on the %openfile hook
#	function, which interprets an fopen(3)-style mode argument as its
#	first parameter.  The other redirection hooks (e.g., %open and
#	%create) exist so that they can be spoofed independently of %openfile.
#
#	The %one function is used to make sure that exactly one file is
#	used as the argument of a redirection.

fn-%openfile	= $&openfile
fn-%open	= %openfile r		# < file
fn-%create	= %openfile w		# > file
fn-%append	= %openfile a		# >> file
fn-%open-write	= %openfile r+		# <> file
fn-%open-create	= %openfile w+		# >< file
fn-%open-append	= %openfile a+		# >>< file, <>> file

fn %one {
	if {!~ $#* 1} {
		throw error %one <={
			if {~ $#* 0} {
				result 'null filename in redirection'
			} {
				result 'too many files in redirection: ' $*
			}
		}
	}
	result $*
}

#	Here documents and here strings are internally rewritten to the
#	same form, the %here hook function.
#
#		cmd << tag input tag	%here 0 input {cmd}
#		cmd <<< string		%here 0 string {cmd}

fn-%here	= $&here

#	These operations are like redirections, except they don't include
#	explicitly named files.  They do not reduce to the %openfile hook.
#
#		cmd >[n=]		%close n {cmd}
#		cmd >[m=n]		%dup m n {cmd}
#		cmd1 | cmd2		%pipe {cmd1} 1 0 {cmd2}
#		cmd1 |[m=n] cmd2	%pipe {cmd1} m n {cmd2}

fn-%close	= $&close
fn-%dup		= $&dup
fn-%pipe	= $&pipe

#	Input/Output substitution (i.e., the >{} and <{} forms) provide an
#	interesting case.  If es is compiled for use with /dev/fd, these
#	functions will be built in.  Otherwise, versions of the hooks are
#	provided here which use files in /tmp.
#
#	The /tmp versions of the functions are straightforward es code,
#	and should be easy to follow if you understand the rewriting that
#	goes on.  First, an example.  The pipe
#		ls | wc
#	can be simulated with the input/output substitutions
#		cp <{ls} >{wc}
#	which gets rewritten as (formatting added):
#		%readfrom _devfd0 {ls} {
#			%writeto _devfd1 {wc} {
#				cp $_devfd0 $_devfd1
#			}
#		}
#	What this means is, run the command {ls} with the output of that
#	command available to the {%writeto ....} command as a file named
#	by the variable _devfd0.  Similarly, the %writeto command means
#	that the input to the command {wc} is taken from the contents of
#	the file $_devfd1, which is assumed to be written by the command
#	{cp $_devfd0 $_devfd1}.
#
#	All that, for example, the /tmp version of %readfrom does is bind
#	the named variable (which is the first argument, var) to the name
#	of a (hopefully unique) file in /tmp.  Next, it runs its second
#	argument, input, with standard output redirected to the temporary
#	file, and then runs the final argument, cmd.  The unwind-protect
#	command is used to guarantee that the temporary file is removed
#	even if an error (exception) occurs.  (Note that the return value
#	of an unwind-protect call is the return value of its first argument.)
#
#	By the way, creative use of %newfd and %pipe would probably be
#	sufficient for writing the /dev/fd version of these functions,
#	eliminating the need for any builtins.  For now, this works.
#
#		cmd <{input}		%readfrom var {input} {cmd $var}
#		cmd >{output}		%writeto var {output} {cmd $var}

if {~ <=$&primitives readfrom} {
	fn-%readfrom = $&readfrom
} {
	fn-%readfrom = $&noreturn @ var input cmd {
		local ($var = /tmp/es.$var.$pid) {
			unwind-protect {
				$input > $$var
				# text of $cmd is   command file
				$cmd
			} {
				rm -f $$var
			}
		}
	}
}

if {~ <=$&primitives writeto} {
	fn-%writeto = $&writeto
} {
	fn-%writeto = $&noreturn @ var output cmd {
		local ($var = /tmp/es.$var.$pid) {
			unwind-protect {
				> $$var
				$cmd
				$output < $$var
			} {
				rm -f $$var
			}
		}
	}
}

#	These versions of %readfrom and %writeto (contributed by Pete Ho)
#	support the use of System V FIFO files (aka, named pipes) on systems
#	that have them.  They seem to work pretty well.  The authors still
#	recommend using files in /tmp rather than named pipes.

#fn %readfrom var cmd body {
#	local ($var = /tmp/es.$var.$pid) {
#		unwind-protect {
#			/etc/mknod $$var p
#			$&background {$cmd > $$var; exit}
#			$body
#		} {
#			rm -f $$var
#		}
#	}
#}

#fn %writeto var cmd body {
#	local ($var = /tmp/es.$var.$pid) {
#		unwind-protect {
#			/etc/mknod $$var p
#			$&background {$cmd < $$var; exit}
#			$body
#		} {
#			rm -f $$var
#		}
#	}
#}


#
# Hook functions
#

#	These hook functions aren't produced by any syntax rewriting, but
#	are still useful to override.  Again, see the manual for details.

#	%home, which is used for ~expansion.  ~ and ~/path generate calls
#	to %home without arguments;  ~user and ~user/path generate calls
#	to %home with one argument, the user name.

fn-%home	= $&home

#	Path searching used to be a primitive, but the access function
#	means that it can be written easier in es.  Is is not called for
#	absolute path names or for functions.

fn %pathsearch name { access -n $name -1e -xf $path }

#	The exec-failure hook is called in the child if an exec() fails.
#	A default version is provided (under conditional compilation) for
#	systems that don't do #! interpretation themselves.

if {~ <=$&primitives execfailure} {fn-%exec-failure = $&execfailure}

#	The %write-history hook is used in interactive contexts to write
#	command input to the history file (and/or readline's in-memory
#	history log).  By default, $&writehistory (which is available if
#	readline is compiled in) will write to readline's history log if
#	$max-history-length allows, and will write to the file designated
#	by $history if that variable is set and the file it points to
#	exists and is writeable.

if {~ <=$&primitives writehistory} {
	fn-%write-history = $&writehistory
} {
	fn %write-history input {
		if {!~ $history ()} {
			if {access -w $history} {
				echo $input >> $history
			} {
				history = ()
			}
		}
	}
}


#
# Settor functions
#

#	Settor functions are called when the appropriately named variable
#	is set, either with assignment or local binding.  The argument to
#	the settor function is the assigned value, and $0 is the name of
#	the variable.  The return value of a settor function is used as
#	the new value of the variable.  (Most settor functions just return
#	their arguments, but it is always possible for them to modify the
#	value.)

#	These functions are used to alias the standard unix environment
#	variables HOME and PATH with their es equivalents, home and path.
#	With path aliasing, colon separated strings are split into lists
#	for their es form (using the %fsplit builtin) and are flattened
#	with colon separators when going to the standard unix form.
#
#	These functions are pretty idiomatic.  set-home disables the set-HOME
#	settor function for the duration of the actual assignment to HOME,
#	because otherwise there would be an infinite recursion.  So too for
#	all the other shadowing variables.

set-home = @ { local (set-HOME = ) HOME = $*; result $* }
set-HOME = @ { local (set-home = ) home = $*; result $* }

set-path = @ { local (set-PATH = ) PATH = <={%flatten : $*}; result $* }
set-PATH = @ { local (set-path = ) path = <={%fsplit  : $*}; result $* }

#	These settor functions call primitives to set data structures used
#	inside of es.

set-signals		= $&setsignals
set-noexport		= $&setnoexport
set-max-eval-depth	= $&setmaxevaldepth

#	If the primitives $&sethistory or $&resetterminal are defined (meaning
#	that readline or editline is being used), setting the variables $TERM,
#	$TERMCAP, or $history should notify the line editor library.

if {~ <=$&primitives sethistory} {
	set-history = $&sethistory
}

if {~ <=$&primitives resetterminal} {
	set-TERM	= @ { $&resetterminal; result $* }
	set-TERMCAP	= @ { $&resetterminal; result $* }
}

#	The primitive $&setmaxhistorylength is another readline-only primitive
#	which limits the length of the in-memory history list, to reduce memory
#	size implications of a large history file.  Setting max-history-length
#	to 0 clears the history list and disables adding anything more to it.
#	Unsetting max-history-length allows the history list to grow unbounded.

if {~ <=$&primitives setmaxhistorylength} {
	set-max-history-length = $&setmaxhistorylength
	max-history-length = 5000
}


#
# Variables
#

#	These variables are given predefined values so that the interpreter
#	can run without problems even if the environment is not set up
#	correctly.

home		= /
ifs		= ' ' \t \n
prompt		= '; ' ''
max-eval-depth	= 640
path		= <=$&defaultpath

#	noexport lists the variables that are not exported.  It is not
#	exported, because none of the variables that it refers to are
#	exported. (Obviously.)  apid is not exported because the apid value
#	is for the parent process.  pid is not exported so that even if it
#	is set explicitly, the one for a child shell will be correct.
#	Signals are not exported, but are inherited, so $signals will be
#	initialized properly in child shells.  bqstatus is not exported
#	because it's almost certainly unrelated to what a child process
#	is does.  fn-%dispatch is really only important to the current
#	interpreter loop.

noexport = noexport pid signals apid bqstatus path home matchexpr

fn-. = $&runfile $&batchloop

# source the runtime init script.
. ./runtime.es

#
# Title
#

#	This is silly and useless, but whatever value is returned here
#	is printed in the header comment in initial.c;  nobody really
#	wants to look at initial.c anyway.

result es initial state built in `/bin/pwd on `/bin/date for <=$&version
