# exec.es --- bash-compatible `exec' command
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-05-20
# Last modified: 1993-05-20
# Public domain

# Commentary:

# This version of exec prepends a `-' to the name of the program run if the
# first argument is `-'.  This is like the bash's exec command.  Don't use
# The `-' argument if you just want to do redirections in the current
# shell.  It will lose (but there's no reason why you would want to do such
# a thing anyway).

# Code:

###;;;autoload
fn exec \
{
  if { ~ $1 '-' } \
       { $&exec %run <={%pathsearch $2} -$2 $*(3 ...) } \
     { $&exec $* }
}

provide exec

# exec.es ends here
