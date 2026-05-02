# help.es --- documentation system for runtime environment
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1992-06-18
# Last modified: 1993-09-26
# Public domain

# Commentary:

# TODO: Perhaps add a way of storing docstrings in variables so that future
# lookups in same session will be faster. 
#
# Get rid of the dependency on my personal shell environment.  This is
# really really yucky.

# Code:

# Ugh.
help-file-name = $sinit/es/lib/.docstrings

fn help-print-docstring \
{
  let (var = $1;
       dta-file = $help-file-name.dta
       idx-file = $help-file-name.idx)
    {
      * = `{ sed -ne '/^'$var' /{
                         s/  */ /g
                         s/^'$var' \([0-9][0-9]*\) \([0-9][0-9]*\)/\1 \2/
                         p;q;
                      }' $idx-file }

      if { ~ $#* 2 } \
           {
             * = `` '' { sed -ne $1^,^$2^p $dta-file }
             if { ! ~ $* () } \
                  { echo $* }
           } \
         { result 1 }
    }
}

#:docstring help:
# Provide help for documented shell functions.
#:end docstring:

###;;;autoload
fn help \
{
  if { ~ $* () } \
       {
         echo Usage: $0 '[function]' >[1=2]
         result 1
       } \
     {
       help-print-docstring $*
     }
}

###;;;autoload
fn mkdocstrings \
{
  local (fn-$0 =) 
    $0 --docstrings-file\=$help-file-name --verbose -- $sinit/es/lib/*.es
}

provide help

# help.es ends here
