# primitives.es --- primitive es definitions
# Author: Noah Friedman <friedman@splode.com>
# Created: 1993-05-01
# Public domain

# $Id: primitives.es,v 1.2 2000/12/18 11:17:38 friedman Exp $

# Commentary:
# Code:

require hook

###;;;autoload
fn-%and = $&noreturn @ first rest \
{
  let (result = <={ $first })
    {
      if { ~ $rest () } \
           { result $result } \
         {
           if { result $result } \
                { %and $rest } \
              { result $result }
         }
    }
}

# This is no longer needed in es 0.91.
# # Kludge to prevent the handler from catching return.
# ###;;;autoload
# fn-catch = $&noreturn @ handler body \
# {
#   local (fn-'$$catch-handler$$' = $&noreturn $handler)
#     $&catch '$$catch-handler$$' { $body }
# }

###;;;autoload
fn-eval = $&noreturn @ { '{'^$^*^'}' }

###;;;autoload
fn exit { throw exit $* }

###;;;autoload
fn-false = { result 1 }

if { ! ~ $fn-%flatten $&flatten } \
     {
###;;;autoload
fn %flatten   separator args \
       {
         if { ~ $#args 0 } \
              { throw error $0 usage: $0 separator [args ...] }
         let (result =)
           {
             result = $args(1)
             for (elt = $args(2 ...))
               result = $result^$separator^$elt
             result $result
           }
       }
     }

###;;;autoload
fn fork \
{
  $&fork \
    {
      run-hooks fork-hook $*
      { $* }
    }
}

###;;;autoload
fn-%not = $&noreturn @ { if { $* } { result 1 } {} }

###;;;autoload
fn-%or = $&noreturn @ first rest \
{
  if { ~ $first () } { first = false }
  let (result = <={ $first })
    {
      if { ~ $rest () } \
           { result $result } \
         {
           if { result $result } \
                 { result $result } \
              { %or $rest }
         }
    }
}

###;;;autoload
fn %pathsearch \
{
  let (result = <={ run-hooks-until @ { ! result $1 } %pathsearch-hook $* })
     result $result
}

###;;;autoload
fn %pathsearch-default { access -n $* -1e -xf $path }

if { ! ~ $fn-%split $&split } \
     {
###;;;autoload
fn %split   separator args \
       {
         if { ~ $#args 0 } \
              { throw error $0 usage: $0 separator [args ...] }
         let (result =)
           {
             for (elt = <={ %fsplit $separator $args })
               if { ! ~ $elt '' } { result = $result $elt }
             result $result
           }
       }
     }

###;;;autoload
fn-true = {}

###;;;autoload
fn-while = $&noreturn @ cond body \
{
  catch @ e value \
      {
        if { ! ~ $e break } \
             { throw $e $value }
        result $value
      } \
    {
      let (result =)
        forever \
          {
            if { ! $cond } \
                 { throw break $result } \
               { result = <={ $body } }
          }
    }
}


add-hook %pathsearch-hook %pathsearch-default

provide primitives

# primitives.es ends here
