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
				# Strip the first term from the line.
				%complete-fn $line(1) <={%flatten ' ' $line(2 ...)} $word
			}
		}
	}
}


#	%complete-fn finds if necessary, and evaluates if possible, a particular
#	completion function.  Calling this is strongly recommended instead of
#	directly calling `%complete-$fnname` for any completion function other
#	than those found in this file.

completion-path = /home/jpco/git/es-fork/completions

fn %complete-fn func prefix word {
	if {~ $#(fn-%complete-^$func) 0} {
		let (f = ()) {
			f = <={access -n complete-$func^.es -1 -f $completion-path}
			if {!~ $f ()} {
				. $f
			}
		}
	}
	if {!~ $#(fn-%complete-^$func) 0} {
		%complete-^$func $prefix $word
	} {
		%file-complete {} $word
	}
}


#
# Completion logic for built-in concepts.
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
		) $word^*} <={%complete-fn %pathsearch '' $word}
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
	# completions/complete-%pathsearch.es for an example of this.
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
