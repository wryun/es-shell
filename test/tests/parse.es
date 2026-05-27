# tests/parse.es -- test that $&parse works with the chaos of various reader commands

test 'parser' {
	let (ex = ()) {
		catch @ e {
			ex = $e
		} {
			$&parse {throw test-exception}
		}
		assert {~ $ex test-exception}
	}

	let ((e type msg) = ()) {
		catch @ exc {
			(e type msg) = $exc
		} {
			$&parse {result ')'}
		}
		assert {~ $e error && ~ $msg *'syntax error'*} \
			'parser handles syntax error'
	}

	# run these two in subshells, they cause their inputs to go "eof"
	let (msg = `` \n {catch @ exc {
			echo $exc
		} {
			$&parse {result '('}
		}
	}) {
		assert {~ $msg 'error'*'memory exhausted'* || ~ $msg 'error'*'stack overflow'*} \
			'parser handles infinite recursion'
	}

	let (msg = `` \n {
		catch @ exc {
			echo 'caught' $exc
		} {
			let (line = 'aaaa ( bbbbb')
			echo 'parsed' <={$&parse {let (l = $line) {line = (); result $l}}}
		}
	}) {
		assert {~ $msg 'caught'*'syntax error'*}
	}

	# test parsing from string while parsing from input
	let (e = ()) {
		catch @ exc {
			e = $exc
		} {
			$&parse {eval result true}
		}
		assert {~ $e ()}
	}

	# do normal cases last to see if previous ones broke anything
	assert {~ <={$&parse {result 'echo >[1=2]'}} '{%dup 1 2 {echo}}'}

	# force GCs during parsing
	let (lines = 'fn zoom {' 'this is one' 'let (z = a a a) {' 'this is three' '}' '}')
	assert {~ <={$&parse {
		let (l = ()) {
			(l lines) = $lines
			$&collect
			result $l
		}
	}} '{fn-^zoom=@ *{%seq {this is one} {let(z=a a a){this is three}}}}'}
}
