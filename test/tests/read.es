#!/usr/local/bin/es

test 'null reading' {
	let (tmp = `{mktemp test-nul.XXXXXX})
	unwind-protect {
		echo first line	 > $tmp
		./testrun 0	>> $tmp

		let (first = (); second = ()) {
			{
				first = <=%read
				second = <=%read
			} < $tmp
			assert {~ $first 'first line'} 'read reads valid line'
			assert {~ $second 'result 6'} 'read reads line with zero'
		}

		let ((first second) = `` \n {
			let (fl = ())
			cat $tmp | {echo <=%read^\n^<=%read}
		}) {
			assert {~ $first 'first line'} 'pipe read reads valid line'
			assert {~ $second 'result 6'} 'pipe read reads line with zero'
		}
	} {
		rm -f $tmp
	}
}
