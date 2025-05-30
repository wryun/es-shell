#!/usr/local/bin/es

# Demo of programmable completion in es.
#
# This isn't an endorsement of any particular design for programmable completion,
# and this "design" was largely tossed together for expediency.  That said, it's
# impressive how much can be done with so little assistance from the runtime.
#
# It would be very nice if the es-level design for programmable completion could be
# agnostic to the exact line-editing library being used.
#
# Some top-level problems with this totally-naive setup:
#  - doesn't handle multi-command lines, or multi-line commands, very well at all
#  - doesn't colorize results
#  - presents directories ugly
#  - doesn't know when not to add a space at the end like for blah/ or --blah=
#  - can't handle complicated variable terms like $(a b) or $'a b'


# Base/dispatcher completion function.

# - 'prefix' is a single word which contains the contents of the line before the
#   current word.
# - 'word' is the current word under completion.
fn %complete prefix word {
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
				%whatis-complete $word
			} {
				# non-command-position completion
				# wouldn't be crazy to add a completion-path var for these
				if {!~ $#(fn-%complete-^$line(1)) 0} {
					%complete-^$line(1) $word
				} {
					%file-complete $word
				}
			}
		}
	}
}


# Specialized completion logic, but not per-command

# Completions for general commands.  Calls %pathsearch-complete.
fn %whatis-complete word {
	if {~ $word (/* ./* ../)} {
		# this should filter to executable files only.
		%file-complete $word
	} {
		result $word^<={~~ (
			local let for fn %closure match
			<={~~ (<=$&vars <=$&internals) 'fn-'^*}
		) $word^*} <={%pathsearch-complete $word}
	}
}

# Completions for path-searched binaries.
# In theory when overloading %pathsearch for things like autoloading,
# this function should probably be extended as well.
fn %pathsearch-complete word {
	let (files = ()) {
		for (d = $path)
		let (fw = $d/$word)
		for (b = $d/*)
		if {access -x $b} {
			files = $files $word^<={~~ $b $fw^*}
		}
		result $files
	}
}

# This should handle paths that start with ~
fn %file-complete word {
	let (files = ()) {
		for (f = $word^*) {
			if {access -d $f} {
				files = $files $f/
			} {access $f} {
				files = $files $f
			}
		}
		result $files
	}
}


# Per-command completion.  This part is still pretty weak.  In particular,
# earlier arguments should be provided.

# This should only return pages in a certain section if one has been provided in
# a prior argument.
fn %complete-man word {
	if {~ $#MANPATH 0} {
		MANPATH = `manpath
	}
	let (manpath = <={%fsplit : $MANPATH}) {
		let (result = ()) {
			for (mandir = <={%fsplit : $MANPATH})
			for ((sect nm ext) = <={~~ $mandir/man*/$word^* $mandir/man*/*.*})
				result = $result $nm
			result $result
		}
	}
}

# sudo SHOULD be a "pass-through" completion; just cut itself out of the prefix
# and call %complete recursively.
fn-%complete-sudo = %file-complete

# This is just a demo of argument completion.  Not pretty or exciting, especially
# given long opts for ls are rarely ever used?
fn %complete-ls word {
	if {~ $word -*} {
		result $word^<={~~ (
			--all
			--almost-all
			--author
			--block-size=
			--classify
			--color
			--color=
			--context
			--dereference
			--dereference-command-line
			--dereference-command-line-symlink-to-dir
			--directory
			--dired
			--escape
			--file-type
			--format=
			--full-time
			--group-directories-first
			--help
			--hide
			--hide-control-chars
			--human-readable
			--hyperlink
			--ignore=
			--ignore-backups
			--indicator-style=
			--inode
			--kibibytes
			--literal
			--no-group
			--numeric-uid-gid
			--quote-name
			--quoting-style=
			--recursive
			--reverse
			--show-control-chars
			--si
			--size
			--sort
			--sort=
			--tabsize=
			--time
			--time=
			--time-style=
			--version
			--width=
			--zero
		) $word^*}
	} {
		%file-complete $word
	}
}
