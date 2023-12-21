#!/usr/local/bin/es

# Shared state used by the functions below.
let (
	name = ()
	cases = ()
	failed-cases = ()
	failed-msgs = ()
	test-execution-failure = ()
) {

# Test logging and reporting functions.
# This is the part that gets swapped out in different test execution contexts.
# (command-line, CircleCI, etc.)
	fn header title {
		name = $title
		cases = ()
		failed = ()
		test-execution-failure = ()
	}

	fn fail-case test-name cmd msg {
		cases = $cases $^cmd
		failed-cases = $failed-cases $^cmd
		failed-msgs = $failed-msgs $^msg
	}
	fn pass-case test-name cmd got {
		cases = $cases $^cmd
	}

# <?xml version="1.0"?>
# <testsuites>
#   <testsuite errors="0" failures="0" name="test-name" tests="n" time="???">
#     <testcase name="case-name" time="???">
#     ...
#   </testsuite>
# </testsuites>

	fn report {
		echo <={%flatten '' \
			'    <testsuite errors="0" failures="' $#failed-cases \
				'" name="' $name \
				'" tests="' $#cases \
				'">'}

		for (case = $cases) {
			echo -n <={%flatten '' '        <testcase name="' $case '"'}
			if {~ $case $failed-cases} {
				echo '>'
				for (fcase = $failed-cases; msg = $failed-msgs) if {~ $case $fcase} {
					echo <={%flatten '' '            <failure message="' $msg \
						'" type="WARNING">'}
					echo $msg
					echo '            </failure>'
					echo '        </testcase>'
				}
			} {
				echo '/>'
			}
		}

		echo '    </testsuite>'
	}

	echo '<?xml version="1.0"?>'
	echo '<testsuites>'

	. $1 $0 $*(2 ...)

	echo '</testsuites>'
}
