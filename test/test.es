#!/usr/local/bin/es

# test.es -- The entry point for es tests.

# Invoke like:
# ; /path/to/es -s < test.es (--junit) (tests/test1.es tests/test2.es ...)
#
# --junit makes test.es report test results in junit xml compatible with
# circleci.  Don't use it if you're planning on using human eyes to parse the
# results.

# Test state tracking variables.  Once the test finishes running, the `report`
# function uses these to print the results of the test.
let (
	name = ()
	cases = ()
	passed-cases = ()
	failed-cases = ()
	failure-msgs = ()
	test-execution-failure = ()
)
# xml-escape is necessary to smush arbitrary text coming from es into junit XML.
let (
	fn xml-escape {
		let (result = ()) {
			for (string = $*) {
				string = <={%flatten '&amp;' <={%fsplit '&' $string}}
				string = <={%flatten '&quot;' <={%fsplit " $string}}
				string = <={%flatten '&apos;' <={%fsplit '''' $string}}
				string = <={%flatten '&lt;' <={%fsplit '<' $string}}
				result = $result <={%flatten '&gt;' <={%fsplit '>' $string}}
			}
			result $result
		}
	}
)
# These functions manage the test state variables.  report prints out the
# results of the test, and returns false if any cases failed.
let (
	fn new-test title {
		name = $title
		cases = ()
		passed-cases = ()
		failed-cases = ()
		failure-msgs = ()
		test-execution-failure = ()
	}

	fn fail-case test-name cmd msg {
		cases = $cases $^cmd
		failed-cases = $failed-cases $^cmd
		failure-msgs = $failure-msgs $^msg
	}

	fn pass-case test-name cmd {
		cases = $cases $^cmd
		passed-cases = $passed-cases $^cmd
	}

	fn report {
		if $junit {
			echo <={%flatten '' \
				'    <testsuite errors="0" failures="' $#failed-cases \
					'" name="' <={xml-escape $name} \
					'" tests="' $#cases \
					'">'}

			for (case = $cases) {
				echo -n <={%flatten '' '        <testcase name="' <={xml-escape $case} '"'}
				if {~ $case $failed-cases} {
					echo '>'
					for (fcase = $failed-cases; msg = $failure-msgs)
					if {~ $case $fcase} {
						echo <={%flatten '' '            <failure message="' <={xml-escape $msg} \
							'" type="WARNING">'}
						echo <={xml-escape $msg}
						echo '            </failure>'
						echo '        </testcase>'
					}
				} {
					echo '/>'
				}
			}

			echo '    </testsuite>'
		} {
			if {~ $failed-cases ()} {
				echo -n $^name^': '
			} {
				echo $name
				for (case = $failed-cases; msg = $failure-msgs)
					echo - $case failed $msg
			}
			if {!~ $test-execution-failure ()} {
				echo test execution failure: $test-execution-failure
			} {~ $#failed-cases 0} {
				echo passed!
			} {
				echo - $#passed-cases cases passed, $#failed-cases failed.
			}
		}
		result $#failed-cases
	}
)
# test is the function which can be called in the test files to actually run
# tests.  It locally defines an assert function, which is invoked within the
# test body to make up each test case.  after the test body is done executing,
# test will call report to print the results of the test.
let (status = ()) {
	fn test title testbody {
		local (
			fn assert cmd message {
				let (result = ()) {
					catch @ e {
						fail-case $title $cmd $e
						return
					} {
						result = <={$cmd}
					}
					if {!result $result} {
						if {!~ $message ()} {
							fail-case $title $^message
						} {
							fail-case $title $cmd
						}
					} {
						pass-case $title $cmd
					}
				}
			}
		) {
			new-test $title
			catch @ e {
				test-execution-failure = $e
			} {
				$testbody
			}
			status = $status <=report
		}
	}

	fn report-testfile {
		let (s = $status) {
			status = ()
			result $s
		}
	}
}

# $es contains the path to es which the tests can use to refer to "the es binary
# under test".
es = $0
junit = false

if {~ $1 --junit} {
	junit = true
	* = $*(2 ...)
}

if $junit {
	echo '<?xml version="1.0"?>'
	echo '<testsuites>'
}

# The status variable tracks the successes/failures of all the test files being
# invoked so that test.es can correctly exit true or false based on whether all
# the tests passed.
let (status = ()) {
	for (testfile = $*) {
		. $testfile
		status = $status <=report-testfile
	}

	if $junit {
		echo '</testsuites>'
	}

	result $status
}
