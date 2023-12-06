# tests/example.es -- tiny test file to demonstrate how the test machinery works.

#
# The `test` function defines how the test cases in this file will be run.
#
# It will be run once per `case` command in the file, with the arguments from
# the case command passed to it.  Then, the test harness will compare the `test`
# function's return value to the `case`'s corresponding `want`; if the two
# match, the case is considered to have passed.  Otherwise, the case failed.
#
# It's not exercised here, but note that exceptions escaping the test function
# cause the case to automatically be considered a failure.  If you want to test
# exception-throwing, then you need to catch and handle exceptions within the
# test function itself.
#
# Not all test files need a custom test function -- `get-output`, defined in
# test.es, works well for many cases.  All test files need to define _a_ test
# function, however.
#
# In this (admittedly silly) example, the test function just places the word `x`
# before each element of the list.
#

fn test args {
	let (result = ()) {
		for (arg = $args) {
			result = $result x $arg
		}
		result $result
	}
}

# Some working test cases with their desired results.
case one	; want x one
case test two	; want x test x two

# An empty case like this is perfectly valid.
case ()	; want ()

# We can skip certain cases if they're not doing what we want.
skip case 'one-word case'	; # want x one-word x case
