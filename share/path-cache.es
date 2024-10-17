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

# precache also takes a list of binaries.  For each one, if the binary is
# valid, it caches the binary in the path cache without running it.  This can
# be useful in ~/.esrc for pre-caching binaries which are known ahead of time to
# be frequently used.

fn precache progs {
	let (result = ())
	for (p = $progs) {
		catch @ e type msg {
			if {~ $e error} {
				echo >[1=2] $p: $msg
			} {
				throw $e $type $msg
			}
		} {
			result = $result <={%pathsearch $p}
		}
	}
}

# path-cache and the fn-$progs defined by %pathsearch are exported to the
# environment, under the assumption that subshells will also benefit from the
# already-built cache.

path-cache = ()

# cache-path, along with this set of settor functions, ensure that when the path
# is changed, recache is called.

cache-path = $path

set-cache-path = @ {
	for (o = $cache-path; n = $*) {
		if {!~ $o $n} {
			recache
			break
		}
	}
	result $*
}

let (sp = $set-path) set-path = @ {cache-path = $*; $sp $*}
let (sp = $set-PATH) set-PATH = @ {cache-path = <={%fsplit : $*}; $sp $*}
