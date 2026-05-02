# exports.es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-10-18
# Public domain

# $Id: exports.es,v 1.2 1995/08/25 19:26:23 friedman Exp $

# Commentary:
# Code:

require subr

#:docstring noexport-exceptions:
# List of variables that should normally be exported to external programs
# when using `with-minimal-exports' form except when overriden by explicit
# list of export variables passed to the function (which see).
#
# Example:
#   ; local (noexport-exceptions = HOME SHELL TERM) with-minimal-exports env
#   HOME=/home/fsf/friedman
#   SHELL=/usr/local/bin/es
#   TERM=aaa-60
#   ;
#
#:end docstring:

defvar noexport-exceptions HOME LOGNAME PATH SHELL TERM USER

#:docstring with-minimal-exports:
# Usage: with-minimal-exports bodyform exceptions ...
#
# Execute bodyform while inhibiting the export of all shell variables except
# those passed as `exceptions'.  If no exceptions are listed,
# those symbols listed in the variable `noexport-exceptions' are used.
# To run a program with no exports at all, use `with-no-exports' function.
#
# If bodyform is more than one word, it should be wrapped in a lambda, e.g.
# { du -sk }
#
# Example:
#   ; with-minimal-exports { env }  HOME SHELL TERM
#   HOME=/home/fsf/friedman
#   SHELL=/usr/local/bin/es
#   TERM=aaa-60
#   ;
#
#:end docstring:

###;;;autoload
fn-with-minimal-exports = $&noreturn @ body exceptions \
{
  if { ~ $exceptions () } \
       { exceptions = $noexport-exceptions }
  local (noexport = noexport)
    {
      for (v = <={ $&vars })
        {
          if { ! ~ $v $exceptions } \
               { noexport = $noexport $v }
        }
      { $body }
    }
}

#:docstring with-no-exports:
# Usage: with-no-exports bodyform
#
# Execute bodyform while inhibiting the export of all variables.
#:end docstring:

###;;;autoload
fn-with-no-exports = $&noreturn @ \
{
  local (noexport = <={ $&vars }) { $* }
}

provide exports

# exports.es ends here
