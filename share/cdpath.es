# cdpath.es -- Add rc-like $cdpath behavior to es.
#
# `cd'ing to an absolute directory (one which starts with a /, ./, or ../) is
# unchanged.  For any other directory, cd performs a path-search for the
# directory in the list given by the $cdpath variable.
#
# If multiple arguments are given, then path searching is not performed, as we
# don't know a priori which of the arguments are meant to be a searchable
# directory.  Perhaps a future improvement -- decomposing the cdpath behavior
# to allow more-extended definitions of cd to integrate $cdpath a bit more
# easily.

let (cd = $fn-cd)
fn cd dir {
	if {~ $#dir 1} {
		if {!~ $dir /* ./* ../*} {
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

# %cdpathsearch performs the cdpath-searching behavior for cd.  It is nearly
# identical to the default %pathsearch, except it searches for directories
# rather than executable files.

fn %cdpathsearch name { access -n $name -1e -d $cdpath }

# cdpath and CDPATH contain the list of directories to search for cd.  They
# follow the convention that the uppercase CDPATH contains a single colon-
# separated word as is common in UNIX, while the lowercase cdpath contains an
# es list.  CDPATH is exported while cdpath is not.

set-cdpath = @ {local (set-CDPATH = ) CDPATH = <={%flatten : $*}; result $*}
set-CDPATH = @ {local (set-cdpath = ) cdpath = <={%fsplit  : $*}; result $*}

noexport = $noexport cdpath

# The default value of cdpath is ''.  This corresponds with "the current
# directory".  If cdpath is the empty list, then path searching behavior will
# not work, and it will only be possible to cd to paths starting with one of
# (/ ./ ../).  This is more consistent behavior, but may be more surprising to
# an unsuspecting user.

cdpath = ''
