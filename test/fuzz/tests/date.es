# date.es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-03-20
# Last modified: 1993-04-26
# Public domain

# Commentary:
# Code:

#:docstring numeric-date:
# Usage: numeric-date [string]
#
# Echoes a numeric date in the form YYYY-MM-DD, e.g. 1993-03-20
# Argument STRING should be a date string in the same format as the default
# output from `date' command.  If argument is not provided, STRING is
# obtained by running `date'. 
#:end docstring:

###;;;autoload
fn numeric-date \
{
    if { ~ $#1 0 } { local (ifs =) { * = `date } }

    echo $1 \
     | sed -ne '
          s/[^ ]*  *\([^ ]*\)  *\([^ ]*\).* \([^ ]*\)$/\3-\1-\2/
          /-[0-9]$/s/\([0-9]\)$/0\1/
          /Jan/{s/Jan/01/p;q;}
          /Feb/{s/Feb/02/p;q;}
          /Mar/{s/Mar/03/p;q;}
          /Apr/{s/Apr/04/p;q;}
          /May/{s/May/05/p;q;}
          /Jun/{s/Jun/06/p;q;}
          /Jul/{s/Jul/07/p;q;}
          /Aug/{s/Aug/08/p;q;}
          /Sep/{s/Sep/09/p;q;}
          /Oct/{s/Oct/10/p;q;}
          /Nov/{s/Nov/11/p;q;}
          /Dec/{s/Dec/12/p;q;}'
}

provide date

# date.es ends here
