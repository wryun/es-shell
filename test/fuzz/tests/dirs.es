# dirs.es --- directory tracking and directory stacks
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-04-26
# Last modified: 1994-03-16
# Public domain

# Commentary:

# This implementation has one important variation among most directory
# stack mechanisms.  If you set the global variable `dirs:file-name-prefix',
# the `dirs' command will prepend this prefix to all pathnames in the
# directory stack.  This can be used in conjunction with the emacs shell
# mode M-x dirs command to set the current directory. 

# Code:

require hook
require subr

set-cdpath = @ { local (set-CDPATH = ) CDPATH = <={%flatten : $*}; result $* }
set-CDPATH = @ { local (set-cdpath = ) cdpath = <={%fsplit  : $*}; result $* }

# Because shared lexically scoped environments become separate across shell
# invocations (at least as of 0.83), we have just one function that
# performs all operations on the dirstack variable.  Other routines merely
# access it indirectly.
# Attempt to preserve old value of dirstack if this file is reloaded.
let (dirstack = `{ if { ! ~ $fn-dirs () } dirs pwd })
  {
    fn dirs:access-dirstack   cmd op args \
    {
      # Ops basically listed in order of most likely frequency. 
      # `set1' and `set' are currently unused, but provided in case I need
      # to repair damage manually. 
      if { ~ $op pwd } \
           { $cmd $dirstack(1) } \
         { ~ $op chdir } \
           { dirstack = <={ expand-file-name $args(1) $dirstack(1) } $dirstack(2 ...) } \
         { ~ $op dirs } \
           { $cmd $^(dirs:file-name-prefix)^$dirstack } \
         { ~ $op push } \
           { dirstack = <={ expand-file-name $args(1) $dirstack(1) } $dirstack } \
         { ~ $op swap } \
           {           
             if { ~ $#dirstack 1 } \
                  { throw error $0 pushd: No other directory }
             # This shouldn't be here, but it's faster & convenient.
             $&cd $dirstack(2)
             dirstack = $dirstack(2) $dirstack(1) $dirstack(3 ...)
           } \
         { ~ $op pop } \
           { 
             # Do not actually want to pop last element off dirstack. 
             if { ~ $#dirstack 1 } \
                  { throw error $0 popd: Directory stack empty }
             dirstack = $dirstack(2 ...) 
           } \
         { ~ $op set1 } \
            { dirstack = $args(1) $dirstack(2 ...) } \
         { ~ $op set } \
            { dirstack = $args } \
         { throw error $0 $0: $op: invalid op. }
    }
  }

###;;;autoload
fn chdir \
{
   if { ~ $* () } { * = $home }

   if { ~ $1 ./* ../* /* } \
        {
          $&cd $1
          dirs:access-dirstack result chdir $1
        } \
      {
        let (dir =)
          {
            dir = <={ access -1 -d -n $1 $cdpath } ;
            if { ~ $dir () } { throw error $0 $0: $*(1) not in '$cdpath' }
            $&cd $dir
            dirs:access-dirstack result chdir $dir
          }
      }

   run-hooks cd-hook $*
}

###;;;autoload
fn popd \
{
   dirs:access-dirstack result pop
   $&cd `pwd
   run-hooks popd-hook $*
}

###;;;autoload
fn pushd \
{
   if { ~ $* () } \
        { dirs:access-dirstack result swap } \
      { ~ $1 ./* ../* /* } \
        {
          $&cd $1
          dirs:access-dirstack result push $1 
        } \
      {
        let (dir =)
          {
            dir = <={ access -1 -d -n $1 $cdpath } ;
            if { ~ $dir () } { throw error $0 $0: $1 not in '$cdpath' }
            $&cd $dir
            dirs:access-dirstack result push $1 
          }
      }

   run-hooks pushd-hook $*
}

###;;;autoload
fn-dirs = { dirs:access-dirstack echo dirs }

###;;;autoload
fn-pwd = { dirs:access-dirstack echo pwd }

###;;;autoload
fn-cwd = { dirs:access-dirstack result pwd }

# Aliases

###;;;autoload
fn-[d = pushd

###;;;autoload
fn-]d = popd

###;;;autoload
fn-cd = chdir

provide dirs

# dirs.es ends here
