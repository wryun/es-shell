# tests/subscripts.es -- Test that variable subscripting works.

test 'single elements' {
	let (list = un du troix) {
		assert {~ $list(1) un}
		assert {~ $list(4) ()}
	}
	assert {~ $list(1) ()}
}

test 'multiple elements' {
	let (list = un du troix) {
		assert {~ $list(1 2 1 2 1) (un du un du un)}
		assert {~ $list(1 4 4 4 1) (un un)}
	}
	assert {~ $list(1 2 3) ()}
}

test 'single ranges' {
	let (list = un du troix) {
		assert {~ $list(2 ... 1) ()}
		assert {~ $list(2 ... 2) du}
		assert {~ $list(2 ... 3) du troix}
		assert {~ $list(2 ... 4) du troix}
	}
}

test 'multiple ranges' {
	let (list = un du troix) {
		assert {~ $list(2 ... 1 2 ... 1) ()}
		assert {~ $list(2 ... 2 3 ... 1) du}
		assert {~ $list(2 ... 3 1 ... 1) du troix un}
		assert {~ $list(2 ... 4 4 ... 6) du troix}
	}
}

test 'open ranges' {
	let (list = un du troix) {
		assert {~ $list(... 2) un du}
		assert {~ $list(3 ...) troix}
		assert {~ $list(5 ...) ()}
	}
}
