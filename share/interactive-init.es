# interactive-init.es -- Add a hook function %interactive-init.
#
# If defined, the function %interactive-init is called with no arguments before
# the interactive loop starts.  It is called within a very simple exception
# handler so that any exceptions coming from %interactive-init don't disrupt the
# user's ability to use the shell.

let (loop = $fn-%interactive-loop)
fn %interactive-loop {
	if {!~ $#fn-%interactive-init 0} {
		catch @ e {
			echo >[1=2] uncaught exception from %interactive-init: $e
		} {
			%interactive-init
		}
	}
	$loop $*
}
