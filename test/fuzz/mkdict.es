#!/usr/local/bin/es -p
# generate an afl dictionary file for es.  to capture all internals, run like:
#
#   ; env -i /path/to/es ./test/fuzz/mkdict.es > ./test/fuzz/dict.txt

{for (v = (

	# variables and functions
	<=$&internals			# internal variables
	<={~~ <=$&internals fn-*}	# internal functions
	'$&'^<=$&primitives		# primitives with prefix

	# syntax - unfortunately hardcoded
	'^' '$' '(' ')' '()' '{' '}' '{}' '<=' '~' '~~'
	'&&' '||' '`' '``' '`^' '$&' ';' '&'
	'*' 'a*' '?' 'b?' '[a-z]' '[' ']'	# glob syntax
	'<' '>' '<<' '>>' '<<<' '|'		# redirection syntax
	'!' '~' '~~' 'local' 'let' 'for' 'fn' '%closure' 'match'	# keywords

)) echo $v} | sort | uniq | sed 's/\(.*\)/"\1"/'
