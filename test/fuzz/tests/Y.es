# Y.es --- Y combinator
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-05-19
# Last modified: 1993-05-19
# Public domain

# Commentary:

# The example in the docstring came from 
# Harald Hanche-Olsen <hanche@ams.sunysb.edu>

# Code:

#:docstring Y:
# For a full explanation of the Y combinator, refer to any good tutorial 
# on functional programming languages.
#
# Here is an example of its usage in es:
#
#    ; foo = <={Y @ r {
#                result @ x {
#                  if {~ $#x 0} {result done} {echo $x; <=$r $x(2 ...)}}}}
# 
#    ; echo <={$foo a b c}
#    a b c
#    b c
#    c
#    done
#:end docstring:

###;;;autoload
fn Y f {@ g {$g $g} @ h {$f {$h $h}}}

provide Y

# Y.es ends here
