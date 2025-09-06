# preparse.es -- parse scripts strictly before evaluating

# Sourcing this script redefines %batch-loop such that it reads and parses the
# entire input, saving the commands in the input to $cmds, before evaluating the
# whole script at once.  This allows the shell to catch any syntax errors before
# evaluating any part of a potentially-buggy script, which is a nice "sanity"
# feature.  In theory, buffering all the commands has an impact on memory use as
# well as script "start-up" time, but in practice these impacts are quite small.
#
# It's nice that %batch-loop was made hookable despite the original authors'
# reservations on whether hooking it would ever be a good idea :)  It's also
# impressive how relatively simple this is to do.  However, note the couple of
# "gotchas" in the function, each of which represent potential issues in the
# shell.  (Crashing while printing circular bindings is a known problem; the
# other note indicates more marginal issues.)

fn %batch-loop {
	let (cmds = ()) {
		# Read and buffer the commands until we get an eof.
		catch @ e rest {
			if {!~ $e eof} {
				throw $e $rest
			}
		} {
			forever {
				# NOTE: Don't echo $cmds or the shell will die.
				let (cmd = <=%parse)
				if {!~ $#cmd 0} {
					cmds = $cmds {$fn-%dispatch $cmd}
				}
			}
		}
		# Evaluate the commands.
		# NOTE: using a for loop here catches the `break` exception,
		# which we want to avoid.  Also, due to some particular details
		# about how %seq is parsed, we need the {} around this
		# invocation or else $cmds won't get called at all.
		{%seq $cmds}
	}
}
