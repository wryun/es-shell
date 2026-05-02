# load.es --- load or autoload es library files
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-04-24
# Last modified: 1993-05-26
# Public domain

# Commentary:

# This file uses the `fpath-search' function defined in the `require'
# package to do searches through fpath.  

# Code:

# This require may look like a catch-22, but signalling an error will draw
# the attention of whoever is interacting with the interpreter.
require require

#:docstring autoload:
# Usage: autoload function pathname
#
# Declare a function that does not yet have a definition.  The definition
# is loaded from a file the first time the function is run.
#
# When the function actually needs to be loaded, the variable `fpath' is
# searched as a pathlist for a file of the same name as the autoloaded
# function.  First, the file name with a `.es' suffix is appended and
# searched for through all of fpath, then the file name itself is tried if
# it hasn't been found already.  The file is then loaded and the function
# is executed.
#
# Note: if 2nd (optional) argument to autoload is given, then autoload will
# expect to be able to load the definition of the function from that file
# (with or without a `.es' suffix).  For more details consult the docstring
# for fpath-search.
#:end docstring:

###;;;autoload
fn autoload 	func file \
{
  if { ~ $func () } \
       { throw error $0 Usage: $0 '[function] {filename}' }

  # Don't do anything if func is already defined.
  if { ~ $(fn-^$func) () } \
       { 
         file = $file(1)    # Only one filename arg should have been given.
         if { ~ $file () } \
              { file = $func }
         fn-$func = @ { autoload-internal $func $file $* }
       }
}

fn autoload-internal    func file * \
{
  let (orig-fn = $(fn-$func))
    {
      load $file
      if { ~ $orig-fn $(fn-$func) } \
           { throw error $0 $0: autoload failed. }
    }
  $func $*
}

#:docstring load:
# Usage: load [library]
#
# Load (source) contents of an es library, searching fpath for the file
# (see fpath-search) if library name has no `/' chars in it (i.e. no path
# name was specified).
#:end docstring:

###;;;autoload
fn load \
{
    if { ~ $* () } { throw error $0 Usage: $0 [file] }

    if { ~ $1 */* } \
         { . $1 } \
       {
         let (file = <={ fpath-search $1 })
           if { ! ~ $file () } \
                { . $file } \
              { throw error $0 $0: could not find '`'$1'''' in fpath }
       }
}

provide load

# load.es ends here
