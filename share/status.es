# status.es -- Make $status available in the interactive loop.
#
# With this, users can get the return value of the previous command invoked
# at the REPL.  This corresponds with $? in Bourne-compatible shells, or $status
# in rc.  Modifying $status is not very useful.

let (loop = $fn-%interactive-loop)
fn %interactive-loop {
	let (d = $fn-%dispatch)
	local (
		noexport = $noexport status
		status = <=true
		fn %dispatch {status = <={$d $*}}
	) $loop $*
}
