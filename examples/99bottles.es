ow='on the wall'
b=bottle
n='
'

let(
	t=9 8 7 6 5 4 3 2 1
	o=9 8 7 6 5 4 3 2 1 0
	c=
	r=x 
	) {
	c=$t$o $o
	fn-ne=@ {
		if {~ $#c 0} {throw no}
		if {~ $#c 1} {c=no}
		return $c(1)
		}
	fn-bb=@ {
		return $b ^ <=@{
			unwind-protect {
				if {! ~ $#c 2} {return s} {return ''}
				} {
				if {~ $#r 0} {
					r=x x x 
					c=$c(2 ...)
					}
				r=$r(2 ...)
				}
			} of beer
		}
	}

catch @ e { 
	echo all done
	} {
	forever {
		echo '' <=ne <=bb $ow,$n <=ne <=bb $n if <=@{
			if {~ <=ne no} {
				return that $b
				} {
				return one of those $b^s
				}
			} should happen to fall $n <=ne <=bb $ow $n
		}
	} 
