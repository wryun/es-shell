# tests/option.es -- verify that es handles command line arguments correctly

test 'es -c' {
	assert {!$es -c >[2] /dev/null} 'bad flags produce a bad exit status'
	assert {~ `` \n {$es -c 'echo $0 $2 $#*' a b c d e f} <={%flatten ' ' $es b 6}} 'shell positional args represented correctly'
	assert {~ `` \n {$es -c 'echo command # comment'} 'command'} 'comments in command strings ignored correctly'
}

test 'es -e' {
	let (temp = `{mktemp es-e-script.XXXX})
	unwind-protect {
		for (	(command 				continue) = (
			# commands
			'false'					false
			'if {false} {true}'			true
			'if {true} {false}'			false
			'if {false; true} {true}'		true
			'if {true} {false; true}'		false
			'if {false} {true} {false; true}'	false
			'{true; {true; {false; true}}}'		false
			'let (fn-x = {false; true})
				if {x} {true} {false}'		false

			# assignments
			'x = false'				true
			'fn x {false}'				true
			'{true; {true; {x = false}; true}}'	true
			'let (x = false) true'			true
			'local (x = false) true'		true
		)) {
			cat > $temp << EOF
echo -n one
$command
echo two
EOF
			let (want = <={if $continue {result 'onetwo'} {result 'one'}})
				assert {~ `` \n {$es -e $temp} $want} -e handles $command
		}
	} {
		rm -f $temp
	}

	let (output = ()) {
		local (fn %batch-loop {false; $&batchloop $*})
			output = `` \n {$es -ec 'echo okay'}
		assert {~ $output 'okay'} es -e does not stop execution outside of %dispatch
	}
}

test 'es -p' {
	local (
		variable = value
		fn-function = echo -n body
	) {
		assert {~ `` \n {$es -c '$fn-function; echo $variable'} 'bodyvalue'}
		assert {~ `` \n {$es -pc '$fn-function; echo $variable'} 'value'}
	}
}

test 'es -i' {
	local (
		fn %batch-loop {echo 'batch loop'}
		fn %interactive-loop {echo 'interactive loop'}
	) {
		assert {~ `` \n {$es -c 'echo fail'} 'batch loop'} 'es -c is non-interactive by default'
		assert {~ `` \n {$es -ic 'echo fail'} 'interactive loop'} 'es -i forces interactive'
	}
}
