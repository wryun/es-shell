# esrc.es --- start of initialization for `es' shell
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-04-25
# Public domain

# $Id: esrc.es,v 1.2 1994/05/22 01:37:58 friedman Exp $

# Commentary:
# Code:

catch @ exception source message \
    {
      local (fn-print = @ \
             {
               if { ~ $* *\n* } \
                    { 
                      echo esrc.es: $* \
                       | sed -ne '1s/^/# /
                                  1!s/^/#/
                                  p' >[1=2]
                    } \
                  { ! ~ $* () } \
                    { echo '#' esrc.es: $* >[1=2] }
             })
        {
          echo >[1=2]
          print caught exception: $exception
          for (sym = source message)
            {
              if { ! ~ $$sym () } \
                   { print $sym '=' $$sym }
            }
        }

      result $exception $source
    } \
  { 
    sinit = $home/etc/init
    sinit-local = $sinit/local

    . $sinit/es/main/startup.es
  }

# esrc.es ends here
