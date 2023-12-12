#!/usr/local/bin/es

# Test executor.  This adds the special sauce for testing purposes.
fn test title tests {
	# Shared state used by the functions below.
	let (
		passed = ()
		failed = ()
		test-execution-failure = ()
	)

	# Test logging and reporting functions.
	# This is the part that gets swapped out in different test execution contexts.
	# (command-line, CircleCI, etc.)
	let (
		fn header {
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
	)

	# `assert-` functions.  These get used directly by the test files and rely on
	# the functions above to gather and report passes and failures.  These should be agnostic
	# to the execution context of the test.
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
						fail-case $title $message
					} {
						fail-case $title $cmd
					}
				} {
					pass-case $title $cmd
				}
			}
		}

		fn assert-output cmd want {
			let (
				f = `mktemp
				out = ()
			)
			unwind-protect {
				catch @ e {
					fail-case $title $cmd $e
					return
				} {
					{eval $cmd} < /dev/null > $f >[2] /dev/null
				}
				let (output = `` '' {cat $f})
				if {!~ $output $want} {
					fail-case $title $cmd got $output want $want
				} {
					pass-case $title $cmd $output
				}
			} {
				rm $f
			}
		}

		fn assert-stderr cmd want {
			let (
				f = `mktemp
				out = ()
			)
			unwind-protect {
				catch @ e {
					fail-case $title $cmd $e
					return
				} {
					{eval $cmd} < /dev/null >[2] $f > /dev/null
				}
				let (output = `` '' {cat $f})
				if {!~ $output $^want^\n} {
					fail-case $title $cmd got $^output want $^want
				} {
					pass-case $title $cmd $output
				}
			} {
				rm $f
			}
		}

		fn assert-stderr-contains cmd want {
			let (
				f = `mktemp
				out = ()
			)
			unwind-protect {
				catch @ e {
					fail-case $title $cmd $e
					return
				} {
					{eval $cmd} < /dev/null >[2] $f > /dev/null
				}
				let (output = `` '' {cat $f})
				if {!~ $output *^$want^*} {
					fail-case $title $cmd got $^output want $^want
				} {
					pass-case $title $cmd $output
				}
			} {
				rm $f
			}
		}

		fn assert-result cmd want {
			let (result = ()) {
				catch @ e {
					fail-case $title $cmd $e
					return
				} {
					result = <={eval $cmd < /dev/null}
				}
				for (g = $result; w = $want) {
					if {!~ $g $w} {
						fail-case $title $cmd got $result '(length' $#result')' want $want '(length' $#want')'
						return 1
					}
				}
				pass-case $title $cmd $result
			}
		}

		fn assert-false cmd {
			let (result = ()) {
				catch @ e {
					fail-case $title $cmd $e
					return
				} {
					result = <={eval $cmd < /dev/null}
				}
				if {result $result} {
					fail-case $title $cmd returned true
				} {
					pass-case $title $cmd
				}
			}
		}

		fn assert-exception-contains cmd want {
			catch @ e {
				if {~ $^e *^$want^*} {
					pass-case $title $cmd $e
					return
				} {
					fail-case $title $cmd got $e want $^want
				}
			} {
				{eval $cmd} < /dev/null > /dev/null >[2] /dev/null
			}
			fail-case $title $cmd got no exception want $^want
		}

	) {
		header
		catch @ e {
			# TODO: handle this, including an `abort-test` exception type!
			test-execution-failure = $e
		} {
			$tests
		}
		report
	}
}

local (es = $0)
for (f = $*) . $f
