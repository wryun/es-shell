fn %complete-%run prefix word {
	let (cmd = <={%split ' '\t $prefix})
	if {~ $#cmd 0} {
		let (result = ()) {
			# enforce an absolute path
			for (r = <={%file-complete @ {access -x $*} $word}) {
				if {~ $r /* ./* ../*} {
					result = $result $r
				} {
					result = $result ./$r
				}
			}
			result $result
		}
	} {~ $#cmd 1} {
		# assume basename of the first term
		let (ps = <={%split '/' $cmd(1)}) result $ps($#ps)
	} {
		# try to pass through to completion on second term
		%complete <={%flatten ' ' $cmd(2 ...)} $word
	}
}

