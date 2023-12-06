# tests/embedded-eq.es -- test correct behavior of '=' parsing.

# For parsing-related tests, it's a good idea to wrap each case in quotes and
# let the test-executors do the parsing in a subshell, which protects the test
# harness from any potential crashes.

fn-test = get-output

case 'a=b; echo $a'			; want 'b'
case 'echo a=b'				; want 'a=b'
case 'echo a = b'			; want 'a = b'
case 'echo a=b; a=b; echo $a'		; want 'a=b b'
case 'a =b= c==d= e=f; echo $a'		; want 'b= c==d= e=f'
case '''a=b'' = c=d; echo $''a=b'''	; want 'c=d'
case 'a=b = c = d; echo $a'		; want 'b = c = d'

case 'let ((a b)=(1=2 2=3 3=4 4=5)) echo $b'	; want '2=3 3=4 4=5'
case 'let (a=b) echo $a'			; want 'b'
case 'let (a=b;c=d) echo $a, $c'		; want 'b, d'
case 'let (a=b) c=$a; echo $c'			; want 'b'
case 'let (a=b=c=d) echo $a'			; want 'b=c=d'
case 'let (''a=b''=c=d) echo $''a=b'''		; want 'c=d'

case 'echo (a=a a=b)'			; want 'a=a a=b'
case 'echo (a b) a=b'			; want 'a b a=b'
case '(a b)=(c=d d=e); echo $b $a'	; want 'd=e c=d'

case 'echo a=b & a=c; echo $a'	; want 'c a=b'
case 'a=b || a=c; echo $a'	; want 'c'
case '!a=b c; echo $a'		; want 'b c'

case 'if {~ a a} {a=b; echo $a}'	; want 'b'
case 'if {~ a=b a=b} {echo yes}'	; want 'yes'
