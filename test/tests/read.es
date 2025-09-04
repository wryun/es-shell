#!/usr/local/bin/es

test 'null reading' {
	let (tmp = `{mktemp test-nul.XXXXXX})
	unwind-protect {
		echo first line	 > $tmp
		./testrun 0	>> $tmp

		let (fl = (); ex = (); remainder = ()) {
			catch @ e {
				ex = $e
				remainder = <=%read
			} {
				fl = <=%read
				%read
			} < $tmp
			assert {~ $fl 'first line'} 'seeking read reads valid line'
			assert {~ $ex(3) *'null character encountered'*} 'seeking read throws exception correctly'
			assert {~ $remainder 'sult 6'} 'seeking read leaves file in correct state:' $remainder
		}

		let ((fl ex remainder) = `` \n {
			let (fl = ())
			cat $tmp | catch @ e {
				echo $fl\n$e(3)\n^<=%read
			} {
				fl = <=%read
				%read
			}
		}) {
			assert {~ $fl 'first line'} 'non-seeking read reads valid line'
			assert {~ $ex *'null character encountered'*} 'non-seeking read throws exception correctly'
			assert {~ $remainder 'sult 6'} 'non-seeking read leaves file in correct state'
		}
	} {
		rm -f $tmp
	}
}
