# tests/example.es -- verify the test harness, and demonstrate how it works.

#
# The basic structure of a test: `test NAME BLOCK`
#
# The block should (if you want the test to actually be useful) contain one or
# more `assert` commands.
#
# Note that if an exception is thrown and not handled in the test block, the
# entire test will automatically be considered failed.
#

test 'matches match' {
	# Usage: assert CMD NAME
	assert {~ x (x y z)}

	# It can be used within more complicated code as well.
	let (list = `{echo a b c d})
		assert {~ $list(2) b}

	# Because of these two things, assert can be passed a name, which is useful
	# for diagnostics.
	assert {~ $undefined-variable ()} 'undefined variable produces an empty list'
}

test 'redirects redirect' {
	# Here's a more complex test where we actually do some setup and the test cases
	# build upon each other.
	let (test-file = `mktemp)
	unwind-protect {
		assert {~ `{cat <<< $test-file} $test-file} 'herestring herestrings'

		echo 'hi' > $test-file
		assert {~ `` () {cat $test-file} 'hi'\n} 'write writes'
		echo 'hello' >> $test-file
		assert {~ `` () {cat $test-file} 'hi'\n'hello'\n} 'append appends'
		echo 'overwrite' > $test-file
		assert {~ `` () {cat $test-file} 'overwrite'\n} 'write overwrites'
	} {
		rm -f $test-file
		assert {!access -f $test-file} 'file is deleted after test'
	}
}
