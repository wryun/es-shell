# autoload.es -- Auto-load shell functions on demand.
#
# The function, say, foo, should be in the file foo and should contain the
# definition of the function, as in
#
#	fn foo args {
#		body
#	}
#
# By default, the autoload directory is ~/bin/es.  The $es-autoload variable can
# be set in order to use a different directory.
#
# This is a light adaptation of the original version written by Paul Haahr.
# See esrc.haahr in the examples directory for more of his setup.

let (search = $fn-%pathsearch)
fn %pathsearch prog {
	let (autoload = ~/bin/es) {
		if {!~ $#es-autoload 0} {
			autoload = $es-autoload
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
