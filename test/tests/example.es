# tests/example.es -- verify the test harness, and demonstrate how it works.

#
# The basic structure of a test: `test NAME BLOCK`
#
# The block should (if you want the test to actually be useful) contain one or
# more `assert-*` commands.
#
# Note that if an exception is thrown and not handled in the test block, the
# entire test will automatically be considered failed.
#

test 'result results' {
	# assert-result CMD WANT
	# Evaluates CMD and compares the result against WANT.  Note that
	# assert-result correctly handles lists, so ('a b') and (a b) will not
	# match.
	assert-result {result 'a b'} 'a b'
	assert-result {result ()} ()
	assert-result {result a b} a b

	# CMD may also be given to assert-result as a string.  This is often
	# necessary if the shell's parsing behavior is under test -- you'd want
	# failures to be reported as part of the test, not while the shell is reading
	# the file!
	assert-result 'result a b' a b

	# Asserts can take place within more complex control flow contexts.
	let (lexical-binding = a b c)
		assert-result {result a b c} $lexical-binding

	if {~ <=$&primitives result} {
		assert-result {$&result yes} yes
	}
}

test 'matches match' {
	# assert CMD NAME
	# The simplest and most flexible assertion.  Passes if and only if the first
	# argument returns true.
	assert {~ x (x y z)}

	# This type of assert is mostly useful within larger contexts (so it is used
	# a lot in tests/trip.es)
	let (list = `{echo a b c d})
		assert {~ $list(2) b}

	# Because of these two things, assert can be passed a name, which is useful
	# for diagnostics.
	assert {~ $undefined-variable ()} 'undefined variable produces an empty list'
}

test 'echo echoes' {
	# assert-output CMD WANT
	# evaluates CMD, collects its stdout, and compares it against WANT.
	assert-output {echo ''} \n

	# assert-output doesn't split the output and doesn't strip the trailing newline.
	assert-output {echo -n a b c d} 'a b c d'
	assert-output {echo -n 'a b c d'} 'a b c d'
	assert-output {echo -n 'a  b  c  d'} 'a  b  c  d'
	assert-output 'echo -n ''a  b  c  d''' 'a  b  c  d'

	assert-output {echo a\nb\nc} a\nb\nc\n
	assert-output {echo a\n\nb\n\nc} a\n\nb\n\nc\n
}

test 'redirects redirect' {
	# Here's a more complex test where we actually do some setup and the test cases
	# build upon each other.
	let (test-file = `mktemp)
	unwind-protect {
		assert-output {cat <<< $test-file} $test-file

		echo 'hi' > $test-file
		assert-output {cat $test-file} 'hi'\n
		echo 'hello' >> $test-file
		assert-output {cat $test-file} 'hi'\n'hello'\n
		echo 'overwrite' > $test-file
		assert-output {cat $test-file} 'overwrite'\n

		# Tricky edge case: String-form commands like the following produce
		# potentially surprising results with the lexically bound $test-file.
		assert-output 'cat $test-file' ()
		assert-result 'cat $test-file' 0
	} {
		rm -f $test-file
	}
}
