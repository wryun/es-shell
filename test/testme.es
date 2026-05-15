# test/testme.es -- demo/test of closure splitting and looping fix

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

# Ideal externalized version of the above:
# fn-say = '%closure(time=$&ref 1 morning;fn-goodnight=''%closure(time=$&ref 1)@ *{%seq {echo -n $time ''''-> ''''} {time=night} {echo $time}}'';fn-goodmorning=''%closure(time=$&ref 1)@ *{%seq {echo -n $time ''''-> ''''} {time=morning} {echo $time}}'')@ greeting{$greeting}'

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

# Before, externalizing fn-countdown would crash the shell.

let (cmds = ()) {
	cmds = {echo 'liftoff!'}
	cmds = $cmds {echo 1...; $cmds(1)}
	cmds = $cmds {echo 2...; $cmds(2)}
	cmds = $cmds {echo 3...; $cmds(3)}
	fn countdown {$cmds(4)}
}

# Ideal externalized version of the above:
# fn-countdown = '%closure(cmds=$&ref 1 ''%closure(cmds=$&ref 1){echo ''''liftoff!''''}'' ''%closure(cmds=$&ref 1){%seq {echo 1...} {$cmds(1)}}'' ''%closure(cmds=$&ref 1){%seq {echo 2...} {$cmds(2)}}'' ''%closure(cmds=$&ref 1){%seq {echo 3...} {$cmds(3)}}'')@ *{$cmds(4)}'

# Should print
#   3...
#   2...
#   1...
#   liftoff!

countdown


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

# Ideal externalized version of the above:
# fn-hello   = '%closure(stuff=$&ref 1 lots)@ *{%seq {echo take my stuff} {stuff=none}}'
# fn-goodbye = '%closure(stuff=$&ref 1 lots)@ *{if {~ $stuff none} {echo have fun with my stuff} {echo why do I still have stuff?}}'

# Should print
#   take my stuff
#   have fun with my stuff

./es -c 'hello; goodbye'
