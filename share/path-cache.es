# path-cache.es -- Cache paths as functions.
#
# When a binary $prog is searched for in $path and run, then fn-$prog is set to
# $path (if not already defined).  This short-circuits the process of searching
# path for that $prog in the future, which can be a performance win when path
# searching is slow for whatever reason.  Many Bourne-based shells do something
# similar under the name "hashing".
#
# Caching the path also adds $prog to the path-cache variable.  This is used by
# the recache function described below.  It can also be inspected by the user,
# but should probably not be modified by anything other than recache.
#
# This implementation avoids redefining any already-defined functions, which may
# be a bit over-conservative, but seems less surprising than the alternative.
#
# This is adapted from a version originally written by Paul Haahr.
# See esrc.haahr in the examples directory for more of his setup.

let (search = $fn-%pathsearch)
fn %pathsearch prog {
	let (path = <={$search $prog}) {
		if {~ $path /*} {
			if {~ $#(fn-$prog) 0} {
				path-cache = $path-cache $prog
				fn-$prog = $path
			}
		}
		result $path
	}
}

# recache takes a list of binaries.  For each one, if that prog is in
# $path-cache, then fn-$prog is set to () and $prog is removed from
# path-cache.  If recache is called with no arguments, then the entire cache is
# reset.

fn recache progs {
	let (cache = $path-cache; new-cache = ()) {
		if {~ $#progs 0} {
			progs = $path-cache
		}
		for (p = $cache) {
			if {~ $p $progs} {
				fn-$p = ()
			} {
				new-cache = $new-cache $p
			}
		}
		path-cache = $new-cache
	}
}

# path-cache and the fn-$progs defined by %pathsearch, are exported to the
# environment, under the assumption that subshells will also benefit from the
# already built cache.

# TODO: recache should be called if PATH is changed.  Exported path-cache
# complicates this a bit (consider es calling a binary which changes PATH and
# then calls es again).

path-cache = ()
