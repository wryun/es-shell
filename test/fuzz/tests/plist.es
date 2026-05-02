# plist.es --- property lists for symbols
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-11-07
# Last modified: 1993-11-07
# Public domain

# Commentary:

# TODO: Keep the plist symbols in a boxed closure such that it's possible
# to add new symbols dynamically to the separate environment.  Might
# consider waiting until 1st-class environments are implemented.

# Code:

#:docstring get:
# Usage: get [symbol] [propname]
#
# Return the value of SYMBOL's PROPNAME property.
# This is the last VALUE stored with `put SYMBOL PROPNAME VALUE'.
#:end docstring:

###;;;autoload
fn get   sym prop \
{
  let (plist = $(plist-$sym)
       result =)
    {
      if { ~ $plist $prop } \
           {
             while { ! ~ $plist () } \
               {
                 if { ~ $plist(1) $prop } \
                      {
                        result = <={ $plist(2) }
                        plist =
                      }
                 plist = $plist(3 ...)
               }
           }
      result $result
    }
}

#:docstring put:
# Usage: put [symbol] [propname] [value]
#
# Store SYMBOL's PROPNAME property with value VALUE.
# It can be retrieved with `get SYMBOL PROPNAME'.
#:end docstring:

###;;;autoload
fn put   sym prop val \
{
  # box-plist-value's definition is quoted to avoid creating an extra closure.
  # (There's no real harm in this except that it consumes space.
  let (box-plist-value = '@ { result { result $* } }'
       plist = $(plist-$sym)
       value =
       new-plist =)
    {
      value = <={ $box-plist-value $val }
      if { ~ $plist $prop } \
           {
             while { ! ~ $plist () } \
               {
                 if { ~ $plist(1) $prop } \
                      {
                        new-plist = $new-plist $plist(1) $value $plist(3 ...)
                        plist =
                      } \
                    {
                      new-plist = $new-plist $plist(... 2)
                      plist = $plist(3 ...)
                    }
               }
           } \
         { new-plist = $prop $value $plist }

      plist-^$sym = $new-plist
    }
}

#:docstring symbol-plist:
# Usage: symbol-plist [symbol]
#
# Return SYMBOL's property list.
#:end docstring:

###;;;autoload
fn symbol-plist { result $(plist-$1) }

#:docstring symbol-property-names:
# Usage: symbol-property-names [symbol]
#
# Return a list of all SYMBOL's property names.
#:end docstring:

###;;;autoload
fn symbol-property-names \
{
  * = <={ symbol-plist $1 }
  let (res =)
    {
      while { ! ~ $* () } \
        {
          res = $res $1
          * = $*(3 ...)
        }
      result $res
    }
}

#:docstring symbol-property-values:
# Usage: symbol-property-values [symbol]
#
# Return a list of all values associated with SYMBOL's property names.
#
# The values are boxed to get around es' array flattening.  To get at the
# actual values, call the value as a procedure and save the result.
#:end docstring:

###;;;autoload
fn symbol-property-values \
{
  * = <={ symbol-plist $1 }
  let (res =)
    {
      while { ! ~ $* () } \
        {
          res = $res $2
          * = $*(3 ...)
        }
      result $res
    }
}

provide plist

# plist.es ends here
