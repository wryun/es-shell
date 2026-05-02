# setterm.es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-09-25
# Public domain

# $Id: setterm.es,v 1.2 1995/08/25 19:18:11 friedman Exp $

# Commentary:

# The function `set-termcap' expects the variable `termcapfiles' to be
# defined to a list of termcap files to search.  If this variable is unset,
# it will do nothing.

# The function `set-terminfo' expects the variable `terminfodirs' to be
# defined to a list of terminfo directories to search.  If this variable is
# unset, it will do nothing.

# Code:

###;;;autoload
fn setterm \
{
  catch @ e args \
      {
        if { ~ $e signal && ~ $args(1) sigint } \
             { result 1 } \
           { throw $e $args }
      } \
    {
      while {} \
        {
          echo -n 'TERM = ('^$^default-term^') ' >[1=2]
          read TERM

          if { ~ $TERM () } \
               { TERM = $default-term }

          if { ! set-terminfo } \
               { echo 'warning: no terminfo description for' $TERM >[1=2] }

          if { set-termcap } \
               { break } \
             { echo 'Unknown terminal type:' $TERM >[1=2] }
        }

      if { ~ $TERM aixterm } \
           {
             TERM = xterm
             tset -Q <[0=]
           } \
         { ~ $^TERM '' emacs emacs-virtual xsession } \
           {} \
         { tset -Q <[0=] }
    }
}

###;;;autoload
fn set-termcap \
{
  let (re = '(^'$TERM')|(\|'$TERM'\|)|(\|'$TERM':)'
       result = 1)
    {
      for (f = $termcapfiles)
        if { egrep $re $f > /dev/null >[2=1] } \
             {
               TERMCAP = $f
               result = 0
               break
             }
      result $result
    }
}

###;;;autoload
fn set-terminfo \
{
  let (result = 1)
    {
      for (f = $terminfodirs)
        if { access -f $f } \
             {
               TERMINFO = $f
               result = 0
               break
             }
      result $result
    }
}

provide setterm

# setterm.es ends here
