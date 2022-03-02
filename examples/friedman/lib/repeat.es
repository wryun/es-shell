# repeat.es --- repeat a complex command multiple times
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-05-26
# Last modified: 1993-05-26
# Public domain

# Commentary:
# Code:

#:docstring repeat:
# Usage: repeat n command ...
#
# Repeat `command' `n' number of times. 
#:end docstring:

###;;;autoload
fn-repeat = $&noreturn @ n cmd \
{
  if { ~ $n *[~0-9]* } \
       { throw error $0 $0: argument '`n''' must be a number. }
  catch @ e value \
      {
        if { ! ~ $e fn-repeat:break } \
             { throw $e $value }
        result $value
      } \
    {
      let (result = 0; i =)
        forever \
          {
            if { ~ $#i $n } { throw fn-repeat:break $result }
            i = $i ''
            result = <={ $cmd }
          }
    }
}

provide repeat

# repeat.es ends here
