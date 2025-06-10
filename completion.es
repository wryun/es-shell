# completion.es -- demo of programmable completion in es

#	This file exists to explore some options for programmable completion in
#	es; it's not an endorsement of any particular design for completion.
#	However, this setup already performs much better than the current
#	"built-in" readline completion that es has, with surprisingly little
#	direct support from the readline library.  This corresponds well with
#	how much es is built on top of es, and (hopefully) indicates that es
#	could switch between different line-editing libraries while using a
#	single common user-visible completion "API".
#
#	Syntax isn't handled really robustly with this setup, and probably
#	requires some kind of internal parsing machinery to do right; for
#	example, we don't have great behavior with  `$(var1 var2[TAB]`,
#	`let (f[TAB]`, `let (a = b) command[TAB]`, or `cmd > fi[TAB]`.
#
#	Some hook functions back syntax and ideally modifying the hook functions'
#	completion functions (e.g., `%complete-%create`) would also modify how
#	the syntax is completed (e.g., `command arg > fi[TAB]`).  In theory, this
#	could even extend to things like %complete-%seq and %complete-%pipe,
#	though there's some trickiness in designing how that would actually be
#	executed.
#
#	We also want good automatic de-quoting and re-quoting of terms, and
#	splitting arguments in a syntax-aware way.
#
#	Closer integration with readline is another open question.  For example,
#	it's common to have specific key bindings refer to specific types of
#	completion.  How do we implement that?  Moreover, how do we do so in a
#	way that works with .inputrc?  How might we design this in a way that's
#	library-agnostic?
#
#	This setup produces a fairly large amount of overhead in the es function
#	namespace.  We would likely want to reduce that overhead, especially since
#	all of this has absolutely no value in the non-interactive case.  Perhaps
#	all of the per-command completions should come from autoloadable files and
#	be marked noexport.  I suspect that while it's quite nice to have good
#	coverage for autocompletion, in practical use, only a few commands are
#	actually auto-completed in any interactive session.


#
# Base/dispatcher completion function
#

#	%complete is called by $&readline whenever the user hits "tab".  It is
#	called with two arguments: 'prefix' contains the entire line (in string
#	form) before the current word being completed, and 'word' is the current
#	word.
#
#	It uses some fairly simple heuristics to try to decide what kind of
#	completion it is performing, and then dispatches to other completion
#	functions.  While the heuristics leave something to be desired, calling
#	out to other functions (and allowing those other functions to recursively
#	call %complete again) enables quite a bit of power, especially given
#	how much of es' "internal" behavior is based on hook functions.

fn %complete prefix word {
	if {~ $word '$&'*} {
		# Primitive completion.  So far, no need to make a function of this.
		result $word^<={~~ '$&'^<=$&primitives $word^*}
	} {~ $word '$#'*} {
		result '$#'^<={%var-complete <={~~ $word '$#'*}}
	} {~ $word '$^'*} {
		result '$^'^<={%var-complete <={~~ $word '$^'*}}
	} {~ $word '$'*} {
		result '$'^<={%var-complete <={~~ $word '$'*}}
	} {
		let (line = <={%split ' '\t $prefix}) {
			# Basic "start-of-command" detection.
			if {~ $line () ||
				~ $^line *'<=' ||
				~ $^line *'{' ||
				~ $^line *'|' ||
				~ $^line *'`' ||
				~ $^line *'|['*']' ||
				~ $^line *'&'
			} {
				# Command-position completion.
				%whatis-complete $word
			} {
				# Non-command-position completion.
				# TODO: Provide a way to define %complete-foo in an
				# auto-loadable file!
				if {!~ $#(fn-%complete-^$line(1)) 0} {
					# Strip the first term from the line.
					%complete-^$line(1) <={%flatten ' ' $line(2 ...)} $word
				} {
					%file-complete {} $word
				}
			}
		}
	}
}


#
# Specific, not-per-command completion logic.
#

#	These functions (named according to the pattern %foo-complete) provide
#	completion for specific internal behaviors in the shell.  They're pulled
#	out of %complete largely so that they can be called by per-command
#	completions.

#	Completion of variable names.

fn %var-complete word {
	result $word^<={~~ (<=$&vars <=$&internals) $word^*}
}

#	Generic command-position completion.
#	This calls out to %complete-%pathsearch, which is what should be
#	overridden when %pathsearch is overridden.

fn %whatis-complete word {
	if {~ $word (/* ./* ../* '~'*)} {
		%file-complete @ {access -x -- $*} $word
	} {
		result $word^<={~~ (
			local let for fn %closure match
			<={~~ (<=$&vars <=$&internals) 'fn-'^*}
		) $word^*} <={%complete-%pathsearch '' $word}
	}
}

#	%file-complete calls out to %complete-%home to perform tilde completion,
#	and to %home to perform tilde expansion for subdirectories.
#	The `filter` argument allows callers to only get specific files, like
#	directories or executable files.

fn %file-complete filter word {
	# Defining the %completion-to-file function during %complete signals to
	# the line editing library that the results of this function are meant
	# to be treated as files, and defines a function for the line editing
	# library to use to map from each entry to a valid file.  This enables
	# nice behavior for things like path-searching commands; see
	# %complete-%pathsearch for an example of this.
	fn-%completion-to-file = result

	let (files = (); homepat = ()) {
		if {!~ <={homepat = <={~~ $word '~'*'/'*}} ()} {
			let (homedir = (); path = $homepat(2)) {
				if {~ $homepat(1) ''} {
					homedir = <=%home
				} {
					homedir = <={%home $homepat(1)}
				}
				result '~'^$homepat(1)^'/'^<={~~ <={%file-complete $filter $homedir/$path} $homedir/*}
			}
		} {!~ <={homepat = <={~~ $word '~'*}} ()} {
			result '~'^<={%complete-%home '' $homepat}
		} {
			for (f = $word^*) {
				if {$filter $f} {
					if {access -d -- $f} {
						files = $files $f
					} {access -- $f} {
						files = $files $f
					}
				}
			}
			result $files
		}
	}
}

fn %pid-complete word {
	result $word^<={~~ <=%apids $word^*}
}

#	TODO: %fd-complete? %ifs-complete?


#
# Per-command completions
#

#	We supply several of our own completion functions for shell built-ins
#	and to demonstrate how the "API" works.
#
#		fn %complete-[command] prefix word {
#			return (completion candidates)
#		}
#
#	This "API" is rather weak.  Ideally, these functions would probably
#	receive pre-split argument lines (e.g., `cat foo bar ba[TAB]` would call
#	something like `%complete-cat foo bar ba`, perhaps actually with a
#	numeric index argument so that the entire line can be given to a
#	function where completion is happening in the middle.  Maybe this?
#
#		fn %complete-[command] index command {...}
#
#	A challenge is what to do for cases like %seq {blah[TAB]} {blah blah}.
#	The less-fancy but much-more-straightforward option is to just do the
#	`blah[TAB]` completion by itself.
#
#	In addition, we may consider filtering things like redirections out from
#	the arguments before passing them to these functions, though in some cases
#	like input substitution, we'd want to keep the argument in some form.


# Built-ins

#	In "hook-ish function" cases like %var where the user-level command uses
#	common code with internal shell logic but the shell doesn't actually call
#	these functions, we have %complete-%var and internal completion logic refer
#	to a common "internal" %var-complete function.  This is also used for
#	%whatis.

fn %complete-%var _ word {
	%var-complete $word
}

fn %complete-%whatis _ word {
	%whatis-complete $word
}

#	In cases like %pathsearch and %home where the shell actually calls a hook
#	function to get something done, internal completion functions also directly
#	refer to %complete-%pathsearch and %complete-%home.

fn %complete-%pathsearch _ word {
	fn %completion-to-file f {
		catch @ e {result $f} {
			# Like %pathsearch, but don't filter file types.
			access -n $f -1e $path
		}
	}
	let (files = ()) {
		for (p = $path)
		for (w = $p/$word^*)
		if {access -x -- $w} {
			files = $files <={~~ $w $p/*}
		}
		result $files
	}
}

fn %complete-%home _ word {
	result $word^<={~~ `` \n {awk -F: '{print $1}' /etc/passwd} $word^*}
}

#	These functions which use 'cmd' are good demos of why the 'prefix' arg
#	just isn't quite enough.

fn %complete-%run prefix word {
	let (cmd = <={%split ' '\t $prefix})
	if {~ $#cmd 0} {
		let (result = ()) {
			# enforce an absolute path
			for (r = <={%file-complete @ {access -x $*} $word}) {
				if {~ $r /* ./* ../*} {
					result = $result $r
				} {
					result = $result ./$r
				}
			}
			result $result
		}
	} {~ $#cmd 1} {
		# assume basename of the first term
		let (ps = <={%split '/' $cmd(1)}) result $ps($#ps)
	} {
		# try to pass through to completion on second term
		%complete <={%flatten ' ' $cmd(2 ...)} $word
	}
}

fn %complete-%openfile prefix word {
	let (cmd = <={%split ' '\t $prefix}) {
		if {~ $#cmd 0} {
			# mode
			result $word^<={~~ (r w a r+ w+ a+) $word^*}
		} {~ $#cmd 1} {
			# fd
			if {~ $cmd(1) r*} {
				result 0
			} {
				result 1
			}
		} {~ $#cmd 2} {
			# file
			%file-complete {} $word
		} {
			# cmd: pass-through completion
			%complete <={%flatten ' ' $cmd(4 ...)} $word
		}
	}
}

fn %complete-%open p w 		{%complete-%openfile 'r ' ^$p $w}
fn %complete-%create p w	{%complete-%openfile 'w ' ^$p $w}
fn %complete-%append p w	{%complete-%openfile 'a ' ^$p $w}
fn %complete-%open-write p w	{%complete-%openfile 'r+ '^$p $w}
fn %complete-%open-create p w	{%complete-%openfile 'w+ '^$p $w}
fn %complete-%open-append p w	{%complete-%openfile 'a+ '^$p $w}

#	Note that `cd' is consistently the most overloaded function es has;
#	This function performs the basic completion, but doesn't know about
#	things like cdpath, dir stacks, or anything else folks have done to
#	cd in their setups.

fn %complete-cd _ word {
	%file-complete @ {access -d -- $*} $word
}

fn %complete-wait _ word {
	%pid-complete $word
}

fn %complete-throw prefix word {
	let (cmd = <={%split ' '\t $prefix}) {
		if {~ $#cmd 0} {
			return $word^<={~~ (break eof error retry return signal) $word^*}
		}
		match $cmd(1) (
		(break eof retry return)	{result ()}	# no good guesses :/
		error	{
			if {~ $#cmd 1} {
				%whatis-complete $word
			} {
				result ()
			}
		}
		signal {
			# The shell should be able to give us this list...
			result $word^<={~~ (
				sigabrt
				sigalrm
				sigbus
				sigchld
				sigcont
				sigfpe
				sighup
				sigill
				sigint
				sigkill
				sigpipe
				sigpoll
				sigprof
				sigquit
				sigsegv
				sigstop
				sigtstp
				sigsys
				sigterm
				sigtrap
				sigttin
				sigttou
				sigurg
				sigusr1
				sigusr2
				sigvtalrm
				sigxcpu
				sigxfsz
				sigwinch
			) $word^*}
		}
		*	{result ()}	# Not sure :/
		)
	}
}

#	Functions which just wrap %functions.

fn-%complete-var	= %complete-%var
fn-%complete-whatis	= %complete-%whatis

#	"Pass-through" completions for functions which take commands as arguments.

fn-%complete-eval		= %complete
fn-%complete-exec		= %complete
fn-%complete-time		= %complete
fn-%complete-%not		= %complete
fn-%complete-%background	= %complete

#	"Null" completions for commands which simply take no arguments.

fn-%complete-true		= {result}
fn-%complete-false		= {result}
fn-%complete-newpgrp		= {result}
fn-%complete-%read		= {result}
fn-%complete-%is-interactive	= {result}

#	Technically, all of the arguments to these are command words.

fn %complete-if _ word			{%whatis-complete $word}
fn %complete-unwind-protect _ word	{%whatis-complete $word}
fn %complete-while _ word		{%whatis-complete $word}
fn %complete-%and _ word		{%whatis-complete $word}
fn %complete-%or _ word			{%whatis-complete $word}


# "Demo" completions of external binaries

#	Very incomplete ls completion to see how --option= completion works.
#	Not great so far!
#	TODO: enable --opt[TAB] to complete to '--option=', not '--option= '.
#	TODO: some kind of fanciness to enable good short-option support?

fn %complete-ls _ word {
	if {~ $word -*} {
		result $word^<={~~ (
			--all
			--author
			--block-size=
			--color=
		) $word^*}
	} {
		%file-complete {} $word
	}
}

#	Total support for `man` arguments is surprisingly complicated.
#	This covers `man blah` and `man 1 blah` at least.

fn %complete-man prefix word {
	let (sections = 1 1p n l 8 3 3p 0 0p 2 3type 5 4 9 6 7) {
		if {~ $#MANPATH 0} {
			MANPATH = `manpath
		}
		for (a = <={%split ' '\t $prefix}) if {~ $a $sections} {
			sections = $a
			break
		}
		let (result = (); manpath = <={%fsplit : $MANPATH}) {
			# This `for` kills performance on `man [TAB]` :/
			# `*.*` doesn't work for things like `man sysupdate.d`
			for ((nm ext) = <={~~ $manpath/man$sections/$word^* $manpath/man$sections/*.*})
				result = $result $nm
			result $result
		}
	}
}
