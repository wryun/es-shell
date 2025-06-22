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
