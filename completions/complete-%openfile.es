fn %complete-%openfile prefix word {
	let (cmd = <={%split ' '\t $prefix}) {
		if {~ $#cmd 0} {
			# mode
			result $word^<={~~ (r w a r+ w+ a+) $word^*}
		} {~ $#cmd 1} {
			# fd
			if {~ $cmd(1) r*} {
				result 0
			} {
				result 1
			}
		} {~ $#cmd 2} {
			# file
			%file-complete {} $word
		} {
			# cmd: pass-through completion
			%complete <={%flatten ' ' $cmd(4 ...)} $word
		}
	}
}
