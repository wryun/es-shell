# interactive-init.es -- Add a hook function %interactive-init.
#
# If defined, the function %interactive-init is called with no arguments before
# the interactive loop starts.  It is called with a simple exception handler so
# that any exceptions coming from %interactive-init don't break the shell.

let (loop = $fn-%interactive-loop)
fn %interactive-loop {
	if {!~ $#fn-%interactive-init 0} {
		catch @ e type msg {
			if {~ $e exit} {
				exit $type $msg
			} {~ $e error} {
				echo >[1=2] $msg
			} {!~ $e signal || !~ $type sigint} {
				echo >[1=2] uncaught exception: $e $type $msg
			}
		} {
			%interactive-init
		}
	}
	$loop $*
}
