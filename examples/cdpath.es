let (cd = $fn-cd) fn cd dir {
	if {~ $#dir 1} {
		if {!%is-absolute $dir} {
			let (old = $dir) {
				dir = <={%cdpathsearch $dir}
				if {!~ $dir $old} {
					echo >[1=2] $dir
				}
			}
		}
		$cd $dir
	} {
		$cd $dir
	}
}

fn %cdpathsearch name { access -n $name -1e -d  $cdpath }
fn %is-absolute path { ~ $path /* ./* ../* }

set-cdpath = @{local (set-CDPATH=) CDPATH=<={%flatten : $*}; result $*}
set-CDPATH = @{local (set-cdpath=) cdpath=<={%fsplit  : $*}; result $*}
noexport = $noexport cdpath
cdpath = ''
