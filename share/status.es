# status.es -- Make $status available in the interactive loop.
#
# With this, users can get the return value of the previous command invoked
# at the REPL.  This corresponds with $? in Bourne-compatible shells, or $status
# in rc.  Modifying $status is not very useful.
#
# One unique limitation of this $status is that it doesn't understand groups of
# commands like:
#
# 	false; echo $status
#
# This will reflect the return value of the previous command typed at the REPL,
# not the false.  Because of this, users should, in general, still prefer to use
# <= whenever possible.

let (loop = $fn-%interactive-loop)
fn %interactive-loop {
	let (d = $fn-%dispatch)
	local (
		noexport = $noexport status
		status = <=true
		fn-%dispatch = $&noreturn @ {
			catch @ e rest {
				if {~ $e return} {
					status = $rest
				} {~ $e caught-false} {
					status = $rest
					e = false
				}
				throw $e $rest
			} {
				status = <={catch @ e rest {
					if {~ $e false} {
						throw caught-false $rest
					} {
						throw $e $rest
					}
				} {
					$d $*
				}}
			}
		}
	) $loop $*
}
