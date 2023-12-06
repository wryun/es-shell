# test.es -- run the es test files
# Invoke like "path-to-es ./test.es"

#
# Helper functions
#

fn with-tmpfile file rest {
	local ($file = `mktemp)
	unwind-protect {
		if {~ $#rest 1} {
			$rest
		} {
			with-tmpfile $rest
		}
	} {
		rm -f $$file
	}
}

# example test function.
# if the command runs successfully, returns its stdout.
# if it fails (returns falsey), returns an error message including its stderr.
fn get-output cmd {
	with-tmpfile out err {
		let (result = <={fork {eval $cmd} > $out >[2] $err}) {
			if {!~ $result 0} {
				let (err-msg = `` \n {cat $err})
					result 'error code '^$^result^': '^$^err-msg
			} {
				let (out-msg = `` \n {cat $out})
					result $^out-msg
			}
		}
	}
}

#
# Driver functions
#

fn run-file file {
	echo $file
	local (
		passed = ();	failed = ();	skipped = ()
		test = ();	cases = ();	wants = ()

		fn skip { skipped = $skipped $^* }
		fn case { cases = $cases {result $*}}
		fn want { wants = $wants {result $*}}
	) {

# Each $file's job is simply to define:
#  a test:	The procedure to run to perform the test.
#  N cases:	The test cases specifying arguments for the test.
#  N wants:	For each test case, the desired result.
# See tests/example.es for more specifics on how it works.

		. $file

		if {!~ $#cases $#wants} {
			throw error run-file 'mismatch between number of cases and number of desired results'
		}

		for (args-thunk = $cases; want-thunk = $wants)
		let (args = <=$args-thunk; want = <=$want-thunk)
		catch @ e {
			echo ' - [31mFAILED[0m:' $args
			echo '   unhandled exception:' $e
			failed = $failed $^args
		} {
			let (got = <={$test $args}) {
				if @ {for (g = $got; w = $want) if {!~ $g $w} {return 1}} {
					passed = $passed $^args
				} {
					failed = $failed $^args
					echo ' - [31mFAILED[0m:' $args
					echo '   got:' $got
					echo '   want:' $want
				}
			}
		}

		echo -n ' -' $#passed cases passed
		if {!~ $#skipped 0} {echo ' ('^$#skipped 'skipped)'} {echo}
		result $#failed
	}
}

fn run-tests testdir {
	let (
		files = $testdir/*.es
		failed-files = ()
	) {
		for (f = $files) {
			if {!~ <={run-file $f} 0} {
				failed-files = $failed-files $f
			}
			echo
		}
		if {!~ $#files 1} {
			echo ' ================ '^\n
		}
		if {~ $#failed-files 0} {
			echo 'All tests passed!'
		} {
			echo 'Failed test files:' <={%flatten ', ' $failed-files}
		}
	}
}

run-tests tests
