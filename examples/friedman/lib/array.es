# array.es --- flat array manipulation functions
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-11-07
# Last modified: 1993-11-07
# Public domain

# Commentary:
# Code:

#:docstring array-delq:
# Usage: array-delq [elt] [list]
#:end docstring:

###;;;autoload
fn array-delq   elt list \
{
  # Speed hack: if it looks like the element isn't a member via pattern
  # matching, just return the list and avoid a possibly expensive
  # element-by-element search.
  if { ! ~ $elt $list } \
       { result $list } \
     {
       let (new-list =)
         {
           while { ! ~ $list () } \
             {
               if { ~ $list(1) $elt } \
                    {
                      new-list = $new-list $list(2 ...)
                      break
                    }
               new-list = $new-list $list(1)
               list = $list(2 ...)
             }
           result $new-list
         }
     }
}

#:docstring array-reverse:
# Usage: array-reverse [args ...]
#
# Return a list of ARGS which is in the opposite order from the order
# given.
#
# This function works on flat es arrays, not consed lists or vectors.
#:end docstring:

###;;;autoload
fn array-reverse  list \
{
   let (new =)
     {
       for (nlist = $list) 
         { new = $nlist $new }
       result $new
     }
}

provide array

# array.es ends here
