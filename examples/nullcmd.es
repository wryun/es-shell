#!/usr/local/bin/es

# Implementation of zsh's customizable NULLCMD behavior for es (see the
# "REDIRECTIONS WITH NO COMMAND" section in zshmisc(1) for what that means).
# If a redirection with an empty command is run, like
#
#     > file
#
# then with these hooks, that empty command is replaced with a call to the
# %nullcmd function, which is called with the "mode" of the redirection as its
# only argument, which will be one of the list (r w a r+ w+ a+ here).
#
# Like in zsh, %nullcmd is initially defined as `cat`, but csh
# or sh behavior can be implemented by setting %nullcmd to an error-throwing
# command or {}, respectively.  Note that since es' default behavior here is
# already like sh's, there's probably no reason to source this file and then set
# fn-%nullcmd = {}.
#
# Note that some scripts may expect `> file` to be a quick way to create/empty a
# file, and those scripts may break with these hooks.  To avoid that, you may
# want to define your %nullcmd to only do something special if %is-interactive.

fn %nullcmd {cat}

let (of = $fn-%openfile)
fn %openfile mode fd file cmd {
	if {!~ $#fn-%nullcmd 0 && ~ $cmd *'{}'} {
		$of $mode $fd $file {%nullcmd $mode}
	} {
		$of $mode $fd $file $cmd
	}
}


# Try to unset `%nullcmd` in `exec {> foo}` cases.
# FIXME: This is reeeally ugly.

let (e = $fn-exec)
fn exec cmd {
	if {~ $cmd *'{'^(%create %append %open-create)^*'{}}'} {
		$e {local (fn-%nullcmd = ()) $cmd}
	} {
		$e $cmd
	}
}


# %here doesn't use %openfile, so we override it as well so that we can do neat
# things like `> file << EOF`.

let (h = $fn-%here)
fn %here fd str cmd {
	if {!~ $#fn-%nullcmd 0 && ~ $cmd *'{}'} {
		$h $fd $str {%nullcmd here}
	} {
		$h $fd $str $cmd
	}
}
