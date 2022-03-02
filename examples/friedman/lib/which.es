# which.es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-05-01
# Last modified: 1993-05-01
# Public domain

# Commentary:
# Code:

#:docstring which:
# Usage: which PROG
#
# Like `where', but prints only first occurence. 
#:end docstring:

###;;;autoload
fn which { echo <={ access -1fx -n $1 $path } }

#:docstring where:
# Usage: where PROG
#
# Print the paths of the all occurances of PROG as an executable program
# (i.e. not a directory, character/block special device, etc) in PATH.
# Returns 0 if at least one program is found, 1 otherwise.
#:end docstring:

###;;;autoload
fn where \
{ 
  for (p = $path) { access -fx $p'/'$1 && echo $p'/'$1 }
  result 0
}

#:docstring nth-prog-in-path:
# Usage: nth-prog-in-path n program
#
# Find and echo path of the nth occurance of program in PATH. 
#:end docstring:

###;;;autoload
fn nth-prog-in-path  n prog \
{
  local (l =)
    {
      for (p = $path) { access -fx $p'/'$prog && l = $l $p'/'$prog }
      echo $l($n)
    }
}

provide which

# which.es ends here
