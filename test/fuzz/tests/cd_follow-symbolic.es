# es hack to make cd "follow" symbolic links, so that cd symlink/..
# returns one to the intial directory, not the parent of the directory
# pointed to by the symlink.

fn pwd {
    if {~ $#cwd 0} {
		noexport = $noexport cwd
		cwd = `` \n /bin/pwd
    }
    echo $cwd
}

let (cd = $fn-cd) fn cd dir {
    if {~ $#cwd 0} {
		noexport = $noexport cwd
    }
    if {~ $#dir 0} {
		$cd
		cwd = ~
    } {
		let (current = <={
			if {~ $dir /*} {
				result
			} {
				if {~ $#cwd 0} {
					cwd = `` \n /bin/pwd
				}
				%split / $cwd
			}
		}) {
			for (name = <={%split / $dir}) {
				if {~ $name ..} {
					if {!~ $#current 0} {
						let (x = 1 $current) current = $x(2 ... $#current)
					}
				} {!~ $name . ''} {
					current = $current $name
				}
			}
			let (path = / ^ <={ %flatten / $current }) {
				$cd $path
				cwd = $path
			}
		}
    }
}
