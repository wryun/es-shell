# mkautoloads.es --- es front end for mkautoloads script
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-09-26
# Last modified: 1993-09-28
# Public domain

# Commentary:
# Code:

mkautoloads-file-name = $sinit/es/lib/.autoloads.es

#:docstring mkautoloads:
# Usage: mkautoloads
#
# Front end for the `mkautoloads' shell script.  This function calls the
# shell script with the appropriate arguments for es.
#:end docstring:

###;;;autoload
fn mkautoloads \
{
  local (fn-$0 =)
    $0 --autoload-file\=$mkautoloads-file-name --verbose -- $sinit/es/lib/*.es
}

provide mkautoloads

# mkautoloads.es ends here
