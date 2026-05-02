# startup.es
# Author: Noah Friedman <friedman@splode.com>
# Created: 1993-05-18
# Public domain

# $Id: startup.es,v 1.5 2002/08/09 11:54:07 friedman Exp $

# Commentary:
# Code:

# This defines a null function that allows one to put form feeds (^L) in
# scripts without causing an undefined command to be executed.
fn-\f = {}

let (indent-level =)
{
  fn verbose-startup \
  {
    if { ~ $(verbose-startup?) t } \
         {
           let (indent-padding = '  '; padding = '')
             {
               if { ~ $* () } \
                    {
                      echo -n ')'
                      indent-level = $indent-level(2 ...)
                      if { ~ $indent-level () } echo
                    } \
                  {
                    for (i = $indent-level)
                      padding = $padding^$indent-padding
                    if { ! ~ $indent-level () } echo
                    echo -n '# '^$padding^'('^$^*
                    indent-level = $indent-level ''
                  }
             }
         }
  }
}

fn source-es-init-file \
{
  do-if-exist $sinit/es/$1.es \
    {
      verbose-startup $1.es
      . $sinit/es/$1.es
      verbose-startup
    }
}

fn source-local-es-init-file \
{
  do-if-exist $sinit-local/es/$1.es \
    {
      verbose-startup local/$1.es
      . $sinit-local/es/$1.es
      verbose-startup
    }
}

fn login-mail-check \
{
  let (spool = /var/mail
       user = $USER)
    {
      for (d = /var/mail /usr/spool/mail /usr/mail)
        if { access -d $d } \
             {
               spool = $d
               break
             }

      # If access succeeds, it returns the pathname.  If not it returns
      # nothing ("true").  Thus, use !  since the sense of success and
      # failure is reversed in this case.
      if { ! access -n test -1 $path && test -s $spool/$user } \
           {
             * = `{ grep '^From ' $spool/$user | wc -l }
             echo '#' There are $1 messages in $spool/$user
           }
    }
}

fn set-USER \
{
  # Set USER (and also set LOGNAME, via settor in settor.es)
  if { ! ~ $* () } \
       { USER = $* } \
     { ! ~ $USER () } \
       { USER = $USER } \
     { ! ~ $LOGNAME () } \
       { USER = $LOGNAME } \
     { USER = `{ whoami } }
}


. $sinit/es/main/options.es

set-USER
if { ~ $(login-mail-check?) t && access ~/.hushlogin } \
     { login-mail-check }

fpath = $sinit/es/lib $home/lib/es
. $fpath(1)^/require.es


verbose-startup main/startup.es

for (lib = primitives hook repl setterm load hash dirs prompt exec)
  {
    verbose-startup lib/$lib.es
    require $lib
    verbose-startup
  }

for (file = settor env misc)
  { source-es-init-file main/$file }

for (file = os/$OS term/$TERM)
  { source-es-init-file $file }

verbose-startup

# startup.es ends here
