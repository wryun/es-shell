# misc.es
# Author: Noah Friedman <friedman@splode.com>
# Created: 1993-04-24
# Public domain

# $Id: misc.es,v 1.9 2002/08/09 11:54:07 friedman Exp $

# Commentary:
# Code:

require subr

# If `ls -g' prints the user and group of a file (GNU, POSIX, and BSD),
# rather than just the group (SYSV), return true.
fn ls-g-user+group? \
{
  let (str =)
    {
      str = `{
               ls -gld . \
                | {
                    sed -ne 's/[ 	][ 	]*/ /g
                             s/^.[sSrwx-]* *[0-9]* *\([^0-9]*\)  *.*/\1/
                             p
                            '
                  }
             }
      if { ~ $#str 2 } \
           { result 0 } \
         { result 1 }
    }
}

# Raise all soft limits to hard limits.
# Ignore exceptions that may be cause by buggy limit code on
# some machines (or if it's simply not defined); this is not too important.
fn raise-limits \
{
  ignore-exceptions \
    {
      # For some crazed reason, in es 0.84, some of the output from limit goes
      # to stderr instead of stdout.  Hopefully that will be fixed someday.
      * = `{ limit -h >[2=1] }
      while { ! ~ $* () } \
        {
          ignore-exceptions { limit $1 $2 }
          * = $*(3 ...)
        }
    }
}

fn ls-page \
{
  local (fn-ls =; fn-more =) { ls $* | more }
}

if { ls-g-user+group? } \
     {
       fn ll { ls -lg $* }
     } \
   {
     fn ll { ls -l $* }
   } \

fn lf  { ls -aF $* }
fn lh  { li -L  $* }
fn li  { ll -a  $* }
fn lid { li -d  $* }
fn lis { li -si $* }

fn ls \
{
  local (fn-ls =)
    {
      if { ~ $LSCLEAN () } \
           {
             ls-page -C $*
           } \
         { ls-page -CB -I '*.o' $* }
    }
}

fn-f   = finger
fn-md  = mkdir
fn-mdh = mkdirhier
fn-rd  = rmdir

fn stty-canon { stty cs8 -istrip -parenb -iexten -ixon -ixoff -ixany $* }

if { ! ~ $TERM emacs } \
     {
       fn ls \
       {
         local (fn-ls =; fn-more =) { ls -C $* | more }
       }
     }

{
  stty icanon tabs
  stty intr '^C' kill '^U' quit '^\\' eof '^D'
} >[2] /dev/null

raise-limits >[2] /dev/null

umask 000

add-hook repl-start-hook             set-USER
add-hook repl-interactive-start-hook repl-increment-shlvl
add-hook repl-prompt-hook            repl-print-multi-line-result-comment

source-local-es-init-file misc

# misc.es ends here
