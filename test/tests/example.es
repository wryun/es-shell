# tests/example.es -- tiny test file to demonstrate how the test machinery works.

#
# The `test` variable defines how the test cases in this file will be run.
#
# It will be run once per `case` command in the file, with the arguments from
# the case command passed to it.  Then, the test harness will compare the `test`
# function's return value to the `case`'s corresponding `want`; if the two
# match, the case is considered to have passed.  Otherwise, the case failed.
#
# Not all test files need a bespoke test variable -- `get-output`, defined in
# test.es, works well for many cases.
#
# In this (admittedly silly) example, the test function just places the word `x`
# before each element of the list.
#

test = @ args {
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
