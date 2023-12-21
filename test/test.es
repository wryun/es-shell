#!/usr/local/bin/es

# Shared state used by the functions below.
let (
	passed = ()
	failed = ()
	test-execution-failure = ()
) {

# Test logging and reporting functions.
# This is the part that gets swapped out in different test execution contexts.
# (command-line, CircleCI, etc.)
	fn header title {
		echo -n $title^': '
	}

	fn fail-case test-name cmd msg {
		if {~ $#failed 0} {echo}
		echo - $cmd failed $msg
		failed = $failed $^cmd
	}
	fn pass-case test-name cmd got {
		passed = $passed $^cmd
	}

	fn report {
		if {~ $#failed 0 && ~ $test-execution-failure ()} {
			echo passed!
		} {~ $test-execution-failure ()} {
			echo - $#passed cases passed, $#failed failed.
		} {
			echo test execution failure: $test-execution-failure
		}
	}

	. $1 $0 $*(2 ...)
}
