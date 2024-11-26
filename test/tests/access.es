# tests/access.es -- verify $&access behaviors are correct

# these need improvement
# TODO: add tests for "search" behavior

test 'file permissions' {
	assert {access -x $es}
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
