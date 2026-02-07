# tests/syntax.es -- verify that basic syntax handling is correct

# Note the awkward-looking '{' and '}' around commands here.  This prevents
# capturing lexical bindings, so that
#
#     {want}
#
# doesn't turn into
#
#     %closure(have='garbage text';want='essentially line noise'){want}
#
# which would make tests unnecessarily annoying to write.

test 'syntactic sugar' {
	for (	(have 			want) = (
		# Control Flow
		'! cmd'			'%not {cmd}'
		'cmd &'			'%background {cmd}'
		'cmd1 ; cmd2'		'%seq {cmd1} {cmd2}'
		'cmd1 && cmd2'		'%and {cmd1} {cmd2}'
		'cmd1 || cmd2'		'%or {cmd1} {cmd2}'
		# NOTE: different whitespace from the man page
		'fn name args { cmd }'	'fn-^name=@ args{cmd}'
		# added case for * which is handled separately
		'fn name { cmd }'	'fn-^name=@ *{cmd}'

		# Input/Output Commands
		# NOTE: the <={%one file} part of these is not mentioned in the man page
		'cmd < file'		'%open 0 <={%one file} {cmd}'
		'cmd > file'		'%create 1 <={%one file} {cmd}'
		'cmd >[6] file'		'%create 6 <={%one file} {cmd}'
		'cmd >> file'		'%append 1 <={%one file} {cmd}'
		'cmd <> file'		'%open-write 0 <={%one file} {cmd}'
		'cmd <>> file'		'%open-append 0 <={%one file} {cmd}'
		'cmd >< file'		'%open-create 1 <={%one file} {cmd}'
		'cmd >>< file'		'%open-append 1 <={%one file} {cmd}'
		'cmd >[7=]'		'%close 7 {cmd}'
		'cmd >[8=9]'		'%dup 8 9 {cmd}'
		'cmd <<< string'	'%here 0 string {cmd}'
		'cmd1 | cmd2'		'%pipe {cmd1} 1 0 {cmd2}'
		'cmd1 |[10=11] cmd2'	'%pipe {cmd1} 10 11 {cmd2}'
		# readfrom/writeto handled specially below

		# Expressions
		'$#var'			'<={%count $var}'
		'$^var'			'<={%flatten '' '' $var}'
		'`{cmd args}'		'<={%backquote <={%flatten '''' $ifs} {cmd args}}'
		'``ifs {cmd args}'	'<={%backquote <={%flatten '''' ifs} {cmd args}}'
		# NOTE: these lines are missing from this section of the man page!
		'`^{cmd args}'		'<={%flatten '' '' <={%backquote <={%flatten '''' $ifs} {cmd args}}}'
		'``^ifs {cmd args}'	'<={%flatten '' '' <={%backquote <={%flatten '''' ifs} {cmd args}}}'
	)) {
		assert {~ `` \n {eval echo '{'$have'}'} '{'$want'}'} $have 'is rewritten to' $want
	}
}

test 'readfrom/writeto sugar' {
	for ((have want) = (
		'cmd1 >{ cmd2 }' '%writeto _devfd0 {cmd2} {cmd1 $_devfd0}'
		'cmd1 <{ cmd2 }' '%readfrom _devfd0 {cmd2} {cmd1 $_devfd0}'
	)) {
		# using a totally new es forces _devfd0 specifically
		assert {~ `` \n {$es -c 'echo {'$have'}'} '{'$want'}'}
	}
}

test 'heredoc sugar' {
	let (
		# NOTE: is it a bug that this only works with the closing newline?
		have = 'cmd << tag
input
tag
'
		want = '%here 0 ''input''^\n {cmd}'
	) {
		assert {~ `` \n {eval echo '{'$have'}'} '{'$want'}'}
	}
}

test 'match sugar' {
	let (
		have = 'match $sound (
				$bc {result 3}
				($bp $bw *ow) {}
				* {
					false
				}
			)'

		# bit awkward
		want = 'local(matchexpr=$sound){'^\
				'if {~ $matchexpr $bc} {result 3} '^\
				   '{~ $matchexpr $bp $bw *ow} {} '^\
				   '{~ $matchexpr *} {false}'^\
			'}'
	) {
		assert {~ `` \n {eval echo '{'$have'}'} '{'$want'}'}
	}
}

test 'complex variables' {
	for (syntax = (
		'$()'
		'$foo'
		'$(foo bar)'
		'$(foo^$bar)'
		'$(<={foo})'
	)) {
		assert {~ `` \n {eval echo '{'$syntax'}'} '{'^$syntax^'}'}
	}
}

test 'precedence' {
	for ((have want) = (
		'a || b | c'		'%or {a} {%pipe {b} 1 0 {c}}'
		'a | b || c'		'%or {%pipe {a} 1 0 {b}} {c}'
		'!a && b'		'%and {%not {a}} {b}'
		'!a || b'		'%or {%not {a}} {b}'
		'!a & b'		'%seq {%background {%not {a}}} {b}'
		'!a | b'		'%not {%pipe {a} 1 0 {b}}'
		'let (a=b) x && y'	'let(a=b)%and {x} {y}'
		'let (a=b) x || y'	'let(a=b)%or {x} {y}'
		'let (a=b) x & y'	'%seq {%background {let(a=b)x}} {y}'
		'let (a=b) x | y'	'let(a=b)%pipe {x} 1 0 {y}'
		'a && b > c'		'%and {a} {%create 1 <={%one c} {b}}'
		'a | b > c'		'%pipe {a} 1 0 {%create 1 <={%one c} {b}}'
		'let (a=b) c > d'	'let(a=b)%create 1 <={%one d} {c}'
	)) {
		assert {~ `` \n {eval echo '{'$have'}'} '{'$want'}'}
	}
}

# These %closures are relatively easy to parse, even if they aren't expected to
# be produced by normal code.
test 'valid %closures' {
	for (test = (
		{result a}
		{result a b}
		{result 'a b'}
		{result $&b}
		{result \e^';'}
		{result {}}
		{result @ {}}
	)) {
		let (a = <=$test)
		let (
			c = `` \n {$es -c '''%closure(a='^$a^')echo{}''' >[2=1]}
			l = `` \n {$es -c 'let (a='^$a^')echo {}'}
		) {
			assert {~ $^c $^l} $a
		}
	}
}

# These closures would require full glomming to make work right, which is a can
# of worms.  Throw errors for them; they shouldn't be possible to produce with
# normal code anyway.
test 'bad %closures' {
	for (test = (
		'<={}'
		'$x'
		'$x(2)'
	)) {
		let ((status output) = <={$&backquote \n {
			$es -c 'catch @ e {echo caught $e} {''%closure(a='^$test^')echo {ok}''}'
		}}) {
			assert {~ $output 'caught error $&parse'*} $test output
			assert {~ $status 0} $test result
		}
	}
}
