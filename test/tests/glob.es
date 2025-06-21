# tests/glob.es -- verify basic filesystem globbing works

test 'file globbing' {
	let (dir = `{mktemp -d glob-dir.XXXXXX})
	let (files = $dir^/^(aa bb cc)) {
		touch $files $dir/.hidden
		unwind-protect {
			for (want = $files; got = $dir/*) assert {~ $got $want}
			for (want = $files; got = $dir/??) assert {~ $got $want}
			let (bogus = $dir/???) assert {~ $bogus $dir^'/???'}
			let (got = $dir/?b) assert {~ $#got 1 && ~ $got $dir/bb}
			for (want = $dir/. $dir/.. $dir/.hidden; got = $dir/.*)
				assert {~ $got $want}
			let (quoted = $dir/'??') assert {~ $quoted $dir^'/??'}
		} {
			rm -rf $dir
		}
	}
}

# From https://research.swtch.com/glob.go
test 'asterisk patterns' {
	for ((test want) = (
		{~ ''	''}	true
		{~ x	''}	false
		{~ ''	x}	false
		{~ abc	abc}	true
		{~ abc	*}	true
		{~ abc	*c}	true
		{~ abc	*b}	false
		{~ abc	a*}	true
		{~ abc	b*}	false
		{~ a	a*}	true
		{~ a	*a}	true
		{~ axbxcxdxe	a*b*c*d*e*}	true
		{~ axbxcxdxexxx	a*b*c*d*e*}	true
		{~ abxbbxdbxebxczzx	a*b?c*x}	true
		{~ abxbbxdbxebxczzy	a*b?c*x}	false
		{~ xxx	*x}	true
		{~ aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa	a*a*a*a*b}	false
	)) {
		assert {~ <={$test} <={$want}}
	}
}

test 'range patterns' {
	for ((test want) = (
		{~ x  [x]}	true
		{~ x  [y]}	false
		{~ x  [xyz]}	true
		{~ y  [xyz]}	true
		{~ z  [xyz]}	true
		{~ x  [xy  }	false
		{~ px p[~a]}	true
		{~ pa p[~a]}	false
		{~ p  p[~a]}	false	# https://github.com/rakitzis/rc/issues/115
	)) {
		assert {~ <={$test} <={$want}}
	}
}
