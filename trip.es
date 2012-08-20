#! /bin/es

fn failed {
	echo test failed: $*
	exit 1
}

fn check {
	if {!~ $#* 3} {
		echo too many args too check on test $1
		exit 1
	}
	if {!~ $2 $3} {
		failed $1
	}
}

check catch/retry \
	`` '' {
		local (x = a b c d e f g)
			catch @ e {
				echo caught $e
				if {!~ $#x 0} {
					x = $x(2 ...)
					throw retry
				}
				echo never succeeded
			} {
				echo trying ...
				eval '<>'
				echo succeeded -- something''''s wrong
			} 
	} \
'trying ...
caught error 1: syntax error near end of file
trying ...
caught error 1: syntax error near end of file
trying ...
caught error 1: syntax error near end of file
trying ...
caught error 1: syntax error near end of file
trying ...
caught error 1: syntax error near end of file
trying ...
caught error 1: syntax error near end of file
trying ...
caught error 1: syntax error near end of file
trying ...
caught error 1: syntax error near end of file
never succeeded
'
