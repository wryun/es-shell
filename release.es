#!/usr/bin/env es

# Create a new tarball

if {!~ $#* 1} {
    echo Must provide a single argument: the release version (e.g. 0.9.1)
    exit 1
}

if {! grep -q $1 version.c} {
    echo 'Must update version.c to '^$1
    echo '(yes, this should be automated)'
}

tar chzvf ../es-$1.tar.gz --exclude\=.gitignore --exclude\=release.es `{git ls-files} config.guess config.sub configure install-sh

