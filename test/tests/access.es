# tests/access.es -- verify $&access behaviors are correct

# these need improvement
# TODO: add tests for "search" behavior

test 'file permissions' {
	assert {access -x $es}
}

test 'access exceptions' {
	let (ex = ()) {
		catch @ e {ex = $e} {
			access -1e $es zzznonexistent
		}
		assert {~ $ex ()}
	}
	let (ex = ()) {
		catch @ e {ex = $e} {
			access -1e zzznonexistent $es
		}
		assert {~ $ex ()}
	}
	let (ex = ()) {
		catch @ e {ex = $e} {
			access -1e zzznonexistent xxxnonexistent
		}
		assert {!~ $ex ()}
	}
	let (ex = ()) {
		catch @ e {ex = $e} {
			access -n zzznonexistent -1e / .
		}
		assert {!~ $ex ()}
	}
}

test 'file types' {
	assert {access -d /}
	assert {!access -d $es}
	touch regular
	ln -s regular symbolic
	unwind-protect {
		assert {access -f regular}
		assert {access -l symbolic}
		assert {!access -l regular}
		assert {access -f symbolic}
	} {
		rm -f symbolic regular
	}
}
