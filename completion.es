# completion.es -- demo of programmable completion in es

#	This isn't an endorsement of any particular design for programmable
#	completion, and this "design" was largely tossed together for expediency.
#	That said, it's impressive how much can be done with so little assistance
#	from the runtime.
#
#	Some thoughts about how completions should/shouldn't be exposed to users:
#	 - certain syntactic constructs (variables, primitives, assignments/
#	   binders) are not extensible and should probably just be handled
#	   internally. this would also help make $(a b) and $'a b' work
#	 - certain syntactic constructs (e.g., redirections) ARE extensible and
#	   should ideally be done via completions on their corresponding hook
#	   functions -- e.g., `cmd > [TAB]` should use `%complete-%create` (same
#	   for, like, `$#a[TAB]`?  I guess variable completion should be exposed,
#	   if just so it can be called by other completion functions)
#	 - some parsing should be done internally before calling %complete.  for
#	   example:
#	    - dequoting + requoting
#	    - presenting the current command rather than the current line (e.g.,
#	      `a; b[TAB]` would only be called with `b`, but a multi-line command
#	      would get the whole command)
#	       - IDEALLY, `a; b[TAB]` would use `%complete-%seq`... but how to do
#	         that?
#	    - syntax-aware splitting (e.g., `a {b c d} [TAB]` should call %complete
#	      with ('a' '{b c d}') arguments)
#	    - maybe removing redirections? e.g., `cmd < input arg [TAB]` should
#	      call %complete with just `cmd arg`?
#	 - how do we expose certain completion functions so they can have custom
#	   (named) key bindings?
#	 - the `completions-are-filenames` variable is not the best way... right?


# Base/dispatcher completion function.

# - 'prefix' is a single word which contains the contents of the line before the
#   current word.
# - 'word' is the current word under completion.
fn %complete prefix word {
	# I don't love this variable as a way to do this.  It functions to signal
	# whether the results are file names (with the commensurate quoting rules) or
	# some other kind of string.
	completions-are-filenames = <=false
	if {~ $word '$&'*} {
		# primitive completion
		result $word^<={~~ '$&'^<=$&primitives $word^*}
	} {~ $word '$'*} {
		# variable completion
		result $word^<={~~ '$'^(<=$&vars <=$&internals) $word^*}
	} {
		let (line = <={%split ' '\t $prefix}) {
			if {~ $line () ||
				~ $^line *'<=' ||
				~ $^line *'{' ||
				~ $^line *'|' ||
				~ $^line *'|['*']'
			} {
				# command-position completion
				%complete-%whatis '' $word
			} {
				# non-command-position completion
				# in "real life" we'd load these from files I think
				if {!~ $#(fn-%complete-^$line(1)) 0} {
					%complete-^$line(1) $prefix $word
				} {
					%file-complete {} $word
				}
			}
		}
	}
}


# Specific completion logic, but not per-command

# Completions for path-searched binaries.
# In theory when overloading %pathsearch for things like autoloading,
# this function should probably be extended as well.
fn %pathsearch-complete word {
	let (files = ()) {
		for (d = $path)
		let (fw = $d/$word)
		for (b = $d/*)
		if {access -xf -- $b} {
			files = $files $word^<={~~ $b $fw^*}
		} {access -xd -- $b} {
			files = $files $word^<={~~ $b $fw^*}
		}
		result $files
	}
}

# This 'filter' argument is not super pretty
fn %file-complete filter word {
	completions-are-filenames = <=true
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


# Per-command completions.  This part is still relatively weak.

# %whatis completion, also used for generic command-position completion
fn %complete-%whatis prefix word {
	if {~ $word (/* ./* ../* '~'*)} {
		%file-complete @ {access -x -- $*} $word
	} {
		result $word^<={~~ (
			local let for fn %closure match
			<={~~ (<=$&vars <=$&internals) 'fn-'^*}
		) $word^*} <={%pathsearch-complete $word}
	}
}

fn-%complete-whatis = %complete-%whatis

# this is for the basic cd; things like cdpath should update it
fn %complete-cd prefix word {
	%file-complete @ {access -d -- $*} $word
}

# this is for %home and also used for file completion for '~*' files
fn %complete-%home prefix word {
	result $word^<={~~ `` \n {awk -F: '{print $1}' /etc/passwd} $word^*}
}

# incomplete ls completion to see how --option= completion works.  currently: poorly!
fn %complete-ls prefix word {
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

# total support for `man` arguments is surprisingly complicated.  This covers
# `man blah` and `man 1 blah` at least.
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
			# This `for` just kills performance on `man [TAB]` :/
			for ((nm ext) = <={~~ $manpath/man$sections/$word^* $manpath/man$sections/*.*})
				result = $result $nm
			result $result
		}
	}
}

# sudo -- use a "pass-through" completion.
# something similar can be used for nohup, nice, setsid, etc.
fn %complete-sudo prefix word {
	let (prefix = <={~~ $prefix 'sudo'*})
		%complete $prefix $word
}
