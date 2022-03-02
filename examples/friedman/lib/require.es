# require.es --- simple package system for es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-04-24
# Last modified: 1993-10-02
# Public domain

# Commentary:

#   "Yes!  I know how you think, you Lisp fanatic!"
#      --Ben A. Mesander

# The search path for libraries comes from the `fpath' array (or the
# colon-separated `FPATH' variable; setting one will update the other
# automatically).  It has no default value and must be set by the user.

# (TODO: work around this.  It should be possible in es to save and restore
# previous definitions by messing with the various hooks.)
# Warning: because `require' ultimately uses the builtin `.' command to
# read in files, it has no way of undoing the commands contained in the
# file if there is an error or if no `provide' statement appeared (this
# differs from the lisp implementation of require, which normally undoes
# most of the forms that were loaded if the require fails).  Therefore, to
# minimize the number of problems caused by requiring a faulty package
# (such as syntax errors in the source file) it is better to put the
# provide at the end of the file, rather than at the beginning.

# Code:

# This defines a null function that allows one to put form feeds (^L) in
# scripts without causing an undefined command to be executed.
fn-\f = {}

# Define settors for `fpath' and FPATH to keep them in sync.
# (Adapted from the default PATH/path settors in initial.es.)
set-fpath = @ { local (set-FPATH = ) FPATH = <={ %flatten : $* }; result $* }
set-FPATH = @ { local (set-fpath = ) fpath = <={ %fsplit  : $* }; result $* }

# FEATURES is a lexically scoped variable that should only be directly
# accessible to the various functions which actually manipulate it.
# Because shared lexically scoped environments become separate across shell
# invocations (at least as of 0.83), we have just one function that
# performs all operations on the FEATURES variable.  Other routines merely
# access it indirectly.

# Preserve value if this file is reloaded.
let (FEATURES = <={ if { ~ $fn-features () } \
                         { result () } \
                       { result <=features } 
                  } )
{
   fn require:access-feature \
   {
      if { ~ $1 featurep } \
           { ~ $FEATURES $2 } \
         { ~ $1 features } \
           { result $FEATURES } \
         { ~ $1 provide } \
           {     
             for (feature = $*(2 ...))
               if { ! ~ $FEATURES $feature } \
                    { FEATURES = $feature $FEATURES }
             result 0
           }
   }
}

#:docstring featurep:
# Usage: featurep argument
#
# Returns 0 (true) if argument is a provided feature.  Returns 1 (false)
# otherwise.
#:end docstring:

###;;;autoload
fn-featurep = require:access-feature featurep

#:docstring features:
# Usage: features
#
# Returns a list of all currently provided features.
#:end docstring:

###;;;autoload
fn-features = require:access-feature features

#:docstring provide:
# Usage: provide symbol ...
#
# Register a list of symbols as provided features.
#:end docstring:

###;;;autoload
fn-provide = require:access-feature provide

#:docstring require:
# Usage: require feature {file}
#
# load feature if it is not already provided.  Note that `require' does not
# call `provide' to register features.  The loaded file must do that
# itself.  If the package does not explicitly do a `provide' after being
# loaded, `require' will cause an error exception.
#
# Optional argument `file' means to try to load feature from `file'.  If no
# file argument is given, or if `file' doesn't contain any slashes,
# `require' searches through `fpath' (see `fpath-search') for the
# appropriate file.
#:end docstring:

###;;;autoload
fn require  feature file \
{
  if { ! featurep $feature } \
       {
         if { ~ $file */* } \
              { . $file } \
            {
              let (f = $feature)
                {
                  if { ! ~ $file () } { f = $file }
                  f = <={ fpath-search $f }
                  if { ! ~ $f () } { . $f }
                }
            }

         if { ! featurep $feature } \
              { throw error $0 $0: $feature: feature was not provided. }
       }

  result 0
}

#:docstring fpath-search:
# Usage: fpath-search filename {path ...}
#
# Search $fpath for `filename' or, if `path' (a list) is specified, search
# those directories instead of $fpath.  First the path is searched for an
# occurrence of `filename.es', then a second search is made for just
# `filename'.
#:end docstring:

###;;;autoload
fn fpath-search   name path \
{
  if { ~ $path () } \
       { path = $fpath }

  let (result =)
    {
      for (file = $name^'.es' $name)
        {
          file = <={ access -f -1 -n $file $path }
          if { ! ~ $file () } \
               { 
                 result = $file 
                 break
               }
        }
      result $result
    }
}

provide require

# require.es ends here
