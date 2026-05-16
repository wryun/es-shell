# test/testme.es -- demo/test of closure splitting and looping fix
# run like `/path/to/es < test/testme.es`

#
# First test: Splitting within a single %closure
#

let (time = morning) {
	let (
		fn goodnight {
			echo -n $time '-> '
			time = night
			echo $time
		}
		fn goodmorning {
			echo -n $time '-> '
			time = morning
			echo $time
		}
	) {
		fn say greeting {
			$greeting
		}
	}
}

fn-say = `` \n {whatis say}

# Should print
#   morning -> night
#   night -> morning
#   morning -> night
#   night -> morning

say goodnight
say goodmorning
say goodnight
say goodmorning


#
# Second test: Closure loops
#

# Avoid crashing the shell if it can't handle circular closures
noexport = $noexport fn-countdown

let (cmds = ()) {
	cmds = {echo 'liftoff!'}
	cmds = $cmds {echo 1...; $cmds(1)}
	cmds = $cmds {echo 2...; $cmds(2)}
	cmds = $cmds {echo 3...; $cmds(3)}
	fn countdown {$cmds(4)}
}

fn-countdown = `` \n {whatis countdown}

# Should print
#   3...
#   2...
#   1...
#   liftoff!

# Refer to it like this, in case it isn't defined
$fn-countdown

#
# Third test: Splitting across the whole environment
#

let (stuff = lots) {
	fn hello {
		echo take my stuff
		stuff = none
	}
	fn goodbye {
		if {~ $stuff none} {
			echo have fun with my stuff
		} {
			echo why do I still have stuff?
		}
	}
}

# Should print
#   take my stuff
#   have fun with my stuff

$0 -c 'hello; goodbye'
