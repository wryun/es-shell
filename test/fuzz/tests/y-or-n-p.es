# y-or-n-p.es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-05-26
# Last modified: 1993-05-26
# Public domain

# Commentary:
# Code:

#:docstring y-or-n-p:
# Usage: y-or-n-p QUERY
#
# Print QUERY on stderr, then read stdin for a y-or-n response.  Actually,
# the user may type anything they like, but the first character must be a
# `y', `n', `q', or `!', otherwise the question is repeated until such an
# answer is obtained.
#
# If user typed `y', y-or-n-p returns `y'.
#
# If user typed `n', y-or-n-p returns `n'.
#
# If user typed `!', y-or-n-p returns `!'.  This is an indication to the
# caller that no more queries should be made.  Assume `y' for all the rest.
#
# If user typed `q', y-or-n-p returns `q'.  This is an indication to the
# caller that no more queries should be made.  Assume `n' for all the rest.
#
#:end docstring:

###;;;autoload
fn y-or-n-p \
{
  let (ans =)
    {
      while { ~ $ans () } \
        {
          echo -n $* >[1=2]
          ans = `line
          if { ~ $ans y* Y* } \
               { ans = y } \
             { ~ $ans n* N* } \
               { ans = n } \
             { ~ $ans q* Q* } \
               { ans = q } \
             { ! ~ $ans ! } \
               { 
                 ans =
                 echo 'Please answer one of `y'', `n'', `q'', or `!''' >[1=2]
               }
        }
      result $ans
    }
}

#:docstring yes-or-no-p:
# Usage: yes-or-no-p QUERY
#
# Like y-or-n-p, but require a full `yes', `no', `yes!', or `quit' response. 
# Return values are the same as y-or-n-p's. 
#:end docstring:

###;;;autoload
fn yes-or-no-p \
{
  let (ans =)
    {
      while { ~ $ans () } \
        {
          echo -n $* >[1=2]
          ans = `line
          if { ~ $ans yes YES } \
               { ans = y } \
             { ~ $ans no NO } \
               { ans = n } \
             { ~ $ans quit QUIT } \
               { ans = q } \
             { ! ~ $ans yes! YES! } \
               { 
                 ans =
                 echo 'Please answer one of `yes'', `no'', `quit'', or `yes!''' >[1=2]
               }
        }
      result $ans
    }
}


provide y-or-n-p

# y-or-n-p.es ends here
