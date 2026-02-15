#!/usr/bin/env es

# release.es -- release a new es version.  This script does the following:
#  - Updates the return value of $&version
#  - Creates an "update" commit and tags the commit with the new version
#    (if write-to-git = true).
#  - Generates a tarball for the new release.
#
# To use it to release a new version (e.g., 0.9.3), do the following:
#  1. Start in an es git repo directory called 'es-0.9.3' with a clean
#     working tree
#  2. Edit CHANGES to describe the new release
#  3. Run `./release.es 0.9.3`.  It will open an editor for you to write the
#     tag message.  Presumably you should just copy what you added to CHANGES.
#  4. Assuming everything looks good, run `git push && git push origin v0.9.3`
#
# At this point you'll be able to muck about in Github to create a "Release".

# Whether to actually do the git commit and tag.  Mostly for testing purposes.
write-to-git = true

# make sure everything is on the up and up

if {!~ $#* 1} {
	echo >[1=2] 'Must provide a single argument: the release version (e.g. 0.9.1)'
	exit 1
}

version = $1
cwd = `{basename `pwd}

if {!~ $cwd es-$version} {
	echo >[1=2] 'Must release from a directory named es-'^$version
	exit 1
}

let (changes = `` \n {git status -s})
if {$write-to-git && {!~ $#changes 1 || !~ $changes ' M CHANGES'}} {
	echo >[1=2] 'Must start with a clean workspace with updated CHANGES file'
	exit 1
}

# update mkversion, commit and then tag

sed -i \
	-e 's/VERSION=.*/VERSION='^$version^'/' \
	-e 's/RELEASEDATE=.*/RELEASEDATE='^`^{date +%e-%b-%Y}^'/' \
	mkversion

if $write-to-git {
	git commit -am 'Version '^$version
	git tag -a v$version
}

# generate tarball

files = `{git ls-files}
cd ..
tar chzvf es-$version.tar.gz \
	--exclude=.circleci --exclude=.gitignore \
	--exclude=.gitattributes --exclude=release.es \
	$cwd/^($files config.guess config.sub config.h.in configure install-sh)
