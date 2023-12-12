# tests/embedded-eq.es -- test correct behavior of '=' parsing.

test 'eq is assignment in just the right places' {
	assert-output 'a=b; echo $a'			'b'\n
	assert-output 'echo a=b'			'a=b'\n
	assert-output 'echo a = b'			'a = b'\n
	assert-output 'echo a=b; a=b; echo $a'		'a=b'\n'b'\n
	assert-output 'a =b= c==d= e=f; echo $a'	'b= c==d= e=f'\n
	assert-output '''a=b'' = c=d; echo $''a=b'''	'c=d'\n
	assert-output 'a=b = c = d; echo $a'		'b = c = d'\n
}

test 'eq is handled correctly in let binders' {
	assert-output 'let ((a b)=(1=2 2=3 3=4 4=5)) echo -n $b' '2=3 3=4 4=5'
	assert-output 'let (a=b) echo -n $a'			 'b'
	assert-output 'let (a=b;c=d) echo -n $a, $c'		 'b, d'
	assert-output 'let (a=b) c=$a; echo -n $c'		 'b'
	assert-output 'let (a=b=c=d) echo -n $a'		 'b=c=d'
	assert-output 'let (''a=b''=c=d) echo -n $''a=b'''	 'c=d'
}

test 'eq is handled correctly in with parens in args' {
	assert-output 'echo -n (a=a a=b)'		'a=a a=b'
	assert-output 'echo -n (a b) a=b'		'a b a=b'
	assert-output '(a b)=(c=d d=e); echo -n $b $a'	'd=e c=d'
}

test 'eq is parsed with control flow syntax correctly' {
	assert-output '{echo a=b & a=c} > /dev/null; echo $a'	'c'\n
	assert-output 'a=b || a=c; echo $a'			'c'\n
	assert-output '!a=b c; echo $a'				'b c'\n
}

test 'eq is parsed in if correctly' {
	assert-output 'if {~ a a} {a=b; echo -n $a}'	'b'
	assert-output 'if {~ a=b a=b} {echo -n yes}'	'yes'
}
