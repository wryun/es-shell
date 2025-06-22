# autoload.es -- Auto-load shell functions on demand.
#
# An autoloadable function, say, foo, should be in the file foo and should
# contain the definition of the function, as in
#
#	; cat foo
#	fn foo args {
#		body
#	}
#
# By default, the autoload directory is $XDG_DATA_HOME/es/autoload (if
# $XDG_DATA_HOME is unset, that defaults to ~/.local/share/es/autoload).
# The autoload directory can be changed by setting $es-autoload to the desired
# location.
#
# This is a light adaptation of the original version written by Paul Haahr.
# See esrc.haahr in the examples directory for more of his setup.

let (search = $fn-%pathsearch)
fn %pathsearch prog {
	let (autoload = $XDG_DATA_HOME/es/autoload) {
		if {!~ $#es-autoload 0} {
			autoload = $es-autoload(1)
		} {!~ $#autoload 1} {
			autoload = ~/.local/share/es/autoload
		}
		if {access -f -r $autoload/$prog} {
			. $autoload/$prog
			if {!~ $#(fn-$prog) 0} {
				return $(fn-$prog)
			}
		}
	}
	$search $prog
}
