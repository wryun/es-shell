# noclobber.es -- do not accidentally clobber files

# Sloppy demo of an imitation of the "noclobber" option in some shells -- simply
# throws an error if the file exists.  Not 100% confident this plays nice with
# other spoofs to these hooks, though I don't actually know of any spoofs which
# change the files or pre-create them (in theory, something dwimming would.)
# Note also that this has TOCTTOU risks.
#
# %clobbers can be overwritten to modify our idea of what counts as clobbering:
# for example, perhaps we don't mind clobbering an empty file.

fn %clobbers file {
	access -f $file

	# simple alternate empty-file check might look like the following;
	# this may act unexpectedly for unreadable files:
	# access -f $file && !~ <={%read < $file} ()
}

let (o = $fn-%create)
fn %create fd file cmd {
	if {%clobbers $file} {
		throw error %create $cmd would clobber $file
	} {
		$o $fd $file $cmd
	}
}

# This function lets someone override noclobber.  Note that it needs to be run
# "around" the redirection, like
#
#     clobber {command > file}
#
# as opposed to
#
#     clobber command > file
#
# Why this is the case is left as an exercise for the reader.

fn clobber cmd {
	local (fn-%clobbers = false) $cmd
}

# Imitates the APPEND_CREATE option in zsh, where we only successfully append to
# files which already exist.
# TODO: figure out a nicer way to toggle this on or off.

let (o = $fn-%append)
fn %append fd file cmd {
	if {!access $file} {
		throw error %append file $file does not exist
	} {
		$o $fd $file $cmd
	}
}
