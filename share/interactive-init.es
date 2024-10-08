# interactive-init.es -- Add a hook function %interactive-init.
#
# If defined, the function %interactive-init is called with no arguments before
# the interactive loop starts.

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
