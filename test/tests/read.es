# tests/read.es -- test that reading handles edge cases

test 'null reading' {
	let (tmp = `{mktemp test-nul.XXXXXX})
	unwind-protect {
		echo first line	 > $tmp
		./testrun 0	>> $tmp

		let (first = (); second = ()) {
			{
				first = <=$&read
				second = <=$&read
			} < $tmp
			assert {~ $first 'first line'} '$&read reads valid line'
			assert {~ $second(1) 're'} '$&read reads line with zero (1)'
			assert {~ $second(2) 'sult 6'} '$&read reads line with zero (2)'
		}

		let (first = (); second = ()) {
			{
				first = <=%read
				second = <=%read
			} < $tmp
			assert {~ $first 'first line'} '%read reads valid line'
			assert {~ $second 'result 6'} '%read reads line with zero'
		}

		let ((first second) = `` \n {
			let (fl = ())
			cat $tmp | {echo <=$&read; echo <=$&read}
		}) {
			assert {~ $first 'first line'} 'pipe $&read reads valid line'
			assert {~ $second 're sult 6'} 'pipe $&read reads line with zero'
		}

		let ((first second) = `` \n {
			let (fl = ())
			cat $tmp | {echo <=%read; echo <=%read}
		}) {
			assert {~ $first 'first line'} 'pipe %read reads valid line'
			assert {~ $second 'result 6'} 'pipe %read reads line with zero'
		}


		if {~ <=$&primitives readline} {
			let (first = (); second = ()) {
				{
					first = <=$&readline
					second = <=$&readline
				} < $tmp >[2] /dev/null  # hush the echoing
				assert {~ $first 'first line'} '$&readline reads valid line'
				assert {~ $second 'result 6'} '$&readline reads line with zero'
			}
		}
	} {
		rm -f $tmp
	}
}

test 'eof' {
	let (l = <={$&read <<< ''})
	assert {~ $#l 0} '$&read returns empty list on eof'
	let (l = <={%read <<< ''})
	assert {~ $#l 0} '%read returns empty list on eof'
	if {~ <=$&primitives readline} {
		let (l = <={$&readline <<< ''})
		assert {~ $#l 0} '$&readline returns empty list on eof'
	}
}

test 'fd error handling' {
	assert {catch @ {true}  {$&read <<< '' <[0=]; false}}
	assert {catch @ {false} {$&read <<< '' >[1=]; true}}
	assert {catch @ {false} {$&read <<< '' >[2=]; true}}

	if {~ <=$&primitives readline} {
		assert {catch @ {true}  {$&readline <<< '' <[0=]; false}}
		assert {catch @ {false} {$&readline <<< '' >[1=]; true}}
		assert {catch @ {true}  {$&readline <<< '' >[2=]; false}}
	}
}
