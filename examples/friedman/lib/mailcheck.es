# mailcheck.es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-09-29
# Last modified: 1993-09-29
# Public domain

# Commentary:
# Code:

require hook

#:docstring mailcheck:
# See documentation for `mail-spool-check'.
#:end docstring:

#:docstring mail-spool-check:
# This package ties itself into `repl-prompt-hook' so that it
# is run immediately before every prompt.  The variable `mail-spool' is a
# list of files to search for new mail---if unset when this package is
# loaded, it is initialized to search for a file named the same as the
# value $USER in /var/mail, /usr/spool/mail, and /usr/mail.  Reassigning
# this variable can have it check for additions to any file
# (e.g. /var/log/messages).
#
# When new mail is found, the hook `mail-spool-check-hook' is run, with
# each function in the hook called with the name of the file where mail was
# found.  The hooks will be called separately for each file in $mail-spool
# that is new.  The default value of this hook is `mail-spool-check-notify'.
#
# mail-spool-check can be disabled either by unsetting `mail-spool' or by
# doing: remove-hook repl-prompt-hook mail-spool-check
#:end docstring:

# Initialize mail-spool variable if unset.  It's probably harmless to check
# in all three standard places, so do it and don't make user guess.
if { ~ $mail-spool () } \
     {
       for (dir = /var/mail /usr/spool/mail /usr/mail)
         mail-spool = $mail-spool $dir/$USER
     }

let (tmpfile = /tmp/mailcheck$pid)
  {
###;;;autoload
fn mail-spool-check \
    {
      if { ! ~ $mail-spool () } \
           {
             if { ! access $tmpfile } \
                  { > $tmpfile } \
                {
                  * = `{ ls -t $mail-spool $tmpfile >[2] /dev/null }
                  for (file = $*)
                    {
                      if { ~ $file $tmpfile } \
                           {
                             > $tmpfile
                             break
                           } \
                         { test -s $file } \
                           { run-hooks $0^-hook $file }
                    }
                }
           }
    }
  }

###;;;autoload
fn mail-spool-check-notify \
{
  echo '#' There is new mail in $* >[1=2]
}

add-hook repl-prompt-hook mail-spool-check
add-hook mail-spool-check-hook mail-spool-check-notify

provide mailcheck

# mailcheck.es ends here
