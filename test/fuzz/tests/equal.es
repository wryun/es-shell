# equal.es --- type-extensible equality predicates
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-11-07
# Last modified: 1993-11-07
# Public domain

# Commentary:

# TODO: Document how to extend them.

# Code:

require plist

let (fn-equal-subr = @ caller obj1 obj2 \
     {
       let (result =
            method-found? = <=false)
         {
           for (propname = <={ symbol-property-names $caller })
             {
               if { $propname $obj1 && $propname $obj2 } \
                    {
                      result = <={ <={ get $caller $propname } $obj1 $obj2 }
                      method-found? = <=true
                      break
                    }
             }
           if { result $method-found? } \
                { result $result } \
              { ~ $obj1 $obj2 }
         }
     })
  {

#:docstring eq?:
# Usage: eq? obj1 obj2
#:end docstring:

###;;;autoload
fn eq? { $fn-equal-subr $0 $* }

#:docstring eqv?:
# Usage: eqv? obj1 obj2
#:end docstring:

###;;;autoload
fn eqv? { $fn-equal-subr $0 $* }

#:docstring equal?:
# Usage: equal? obj1 obj2
#:end docstring:

###;;;autoload
fn equal? { $fn-equal-subr $0 $* }

  }

provide equal

# equal.es ends here
