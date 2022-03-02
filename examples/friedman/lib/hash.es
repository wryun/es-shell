# hash.es --- command hashing for es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-04-28
# Last modified: 1993-04-28
# Public domain

# Commentary:

# This implementation seems to make executing commands a tiny bit faster.
# Another advantage is that hashing may avoid hanging the shell if an NFS
# fileysystem goes down (assuming you only run commands that are already
# hashed).
#
# This has yet to be benchmarked.

# Code:

require primitives
require hook

#:docstring hash:
# Usage: hash [name ...]
#
# For each NAME, the full pathname of the command is determined and
# remembered.  If any of the NAMEs given do not exist in $path, an error
# will be signalled and the remaining arguments will not be processed.
#:end docstring:

###;;;autoload
fn hash  fns \
{
  let (result =; loc =)
    {
      for (f = $fns)
        if { ~ $(hash:cmd-$f) () } \
             {
               loc = <={ access -n $f -1e -xf $path }
               result = $result $loc
               if { ~ $loc () } \
                    {} \
                  { hash:cmd-^$f = $loc }
             } \
           { result = $result $(hash:cmd-$f) }

      result $result
    }
}         

###;;;autoload
fn unhash   fns \
{
  if { ~ $fns(1) '-a' } \
       {
         for (var = <=$&vars)
           if { ~ $var 'hash:cmd-'* } \
                { $var = }
         result 0
       } \
     {
       for (var = $fns) \
         hash:cmd-$var =
     }
}

###;;;autoload
fn showhash \
{
  for (var = <=$&vars)
    if { ~ $var 'hash:cmd-'* } \
         { echo $var '=' $$var }
  result 0
}

###;;;autoload
fn hash-do-pathsearch \
{
  if { ~ $(hash:cmd-$1) () } \
       { hash $1 } \
     { result $(hash:cmd-$1) }
}


# Insert this hook before %pathsearch-default so that this supersedes it.
insert-hook %pathsearch-hook hash-do-pathsearch

provide hash

# hash.es ends here
