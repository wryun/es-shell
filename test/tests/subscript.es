# tests/subscripts.es -- Test that variable subscripting works.

test = eval

# single elements
case 'let (list = un du troix) result $list(1)'	; want un
case 'let (list = un du troix) result $list(4)'	; want ()
case 'let (list = ) result $list(1)'		; want ()

# multiple elements
case 'let (list = un du troix) result $list(1 2 1 2 1)'	; want un du un du un
case {let (list = un du troix) result $list(1 4 4 4 1)}	; want un un
case {let (list = ) result $list(1)}		; want ()

# single ranges
case {let (list = un du troix) result $list(2 ... 1)}	; want ()
case {let (list = un du troix) result $list(2 ... 2)}	; want du
case {let (list = un du troix) result $list(2 ... 3)}	; want du troix
case {let (list = un du troix) result $list(2 ... 4)}	; want du troix

# multiple ranges
case {let (list = un du troix) result $list(2 ... 1 2 ... 1)}	; want ()
case {let (list = un du troix) result $list(2 ... 2 3 ... 1)}	; want du
case {let (list = un du troix) result $list(2 ... 3 1 ... 1)}	; want du troix un
case {let (list = un du troix) result $list(2 ... 4 4 ... 6)}	; want du troix

# open ranges
case {let (list = un du troix) result $list(... 2)}	; want un du
case {let (list = un du troix) result $list(3 ...)}	; want troix
case {let (list = un du troix) result $list(5 ...)}	; want ()
