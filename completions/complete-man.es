#	Total support for `man` arguments is surprisingly complicated.
#	This covers `man blah` and `man 1 blah` at least.

fn %complete-man prefix word {
	let (sections = 1 1p n l 8 3 3p 0 0p 2 3type 5 4 9 6 7) {
		if {~ $#MANPATH 0} {
			MANPATH = `manpath
		}
		for (a = <={%split ' '\t $prefix}) if {~ $a $sections} {
			sections = $a
			break
		}
		let (result = (); manpath = <={%fsplit : $MANPATH}) {
			# This whole `for` kills performance on `man [TAB]` :/
			# Slightly buggy :/
			for (fi = $manpath/man$sections/$word^*) {
				if {access $fi} {
					let (sp = <={%fsplit . <={
						~~ $fi $manpath/man$sections/*
					}}) {
						if {~ $sp($#sp) gz} {
							let (nsp = 1 2 $sp)
							sp = $nsp(3 ... $#sp)
						} {
							let (nsp = 1 $sp)
							sp = $nsp(2 ... $#sp)
						}
						result = $result <={%flatten . $sp}
					}
				}
			}
			result $result
		}
	}
}
