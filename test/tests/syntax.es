# tests/syntax.es -- verify that basic syntax handling is correct

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

		# Expressions
		'$#var'			'<={%count $var}'
		'$^var'			'<={%flatten '' '' $var}'
		'`{cmd args}'		'<={%backquote <={%flatten '''' $ifs} {cmd args}}'
		'``ifs {cmd args}'	'<={%backquote <={%flatten '''' ifs} {cmd args}}'
		# NOTE: these lines are missing from this section of the man page!
		'`^{cmd args}'		'<={%flatten '' '' <={%backquote <={%flatten '''' $ifs} {cmd args}}}'
		'``^ifs {cmd args}'	'<={%flatten '' '' <={%backquote <={%flatten '''' ifs} {cmd args}}}'
	)) {
		# Awkward-looking quotes around braces avoid capturing lexical bindings
		assert {~ `` \n {eval echo '{'$have'}'} '{'$want'}'} $have 'is rewritten to' $want
	}
}

# TODO: readfrom/writeto sugar

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

test 'odd var formatting' {
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
