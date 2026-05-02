# stack.es --- trivial stack implementation
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1994-12-14
# Public domain

# $Id: stack.es,v 1.1 1994/12/14 19:01:02 friedman Exp $

# Commentary:

# Based on an idea by Harald Hanche-Olsen <hanche@imf.unit.no>.

# Code:

fn make-stack   s \
{
  result { result (
             # push
             @ i { s = $i $s }

             # pop
             { 
               let (i = $s(1)) 
                 { 
                   s = $s(2 ...)
                   result $i
                 }
             }

             # list
             { result $s }
           )
         }
}

fn stack-push!    obj args { let (fun = <={ $obj }) { $fun(1) $args } }
fn stack-pop!     obj      { let (fun = <={ $obj }) { $fun(2) } }
fn stack-list     obj      { let (fun = <={ $obj }) { $fun(3) } }

provide stack

# stack.es ends here
