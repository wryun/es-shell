# hook.es --- hook creation, modification, and execution routines
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-04-25
# Last modified: 1994-04-01
# Public domain

# Commentary:
# Code:

#:docstring run-hooks:
# Usage: run-hooks [hook-name] {args}
#
# If `hook-name' (a symbol) has a non-empty value, that value may be a
# function or a list of functions to be called.  Each function is called in
# turn with the argument(s) `args'.  Returns a list of all the return
# values from each hook.
#:end docstring:

# We could just define run-hooks as `run-hooks-until false', but that would
# incur additional overhead and it's advantageous for run-hooks to be speedy.
###;;;autoload
fn run-hooks   hook-name args \
{
  let (result =)
    {
      for (do-hook = $$hook-name) 
        result = $result <={ $do-hook $args }
      result $result
    }
}

#:docstring run-hooks:
# Usage: run-hooks-until [condition] [hook-name] {args}
#
# Like `run-hooks', but only call successive hooks if previously-run hooks
# do not satisfy the function `condition', which is called with the result
# of the last hook executed.
#
# The return value is the result of the hook which finally satisfied
# `condition' or the result of the last hook.
#:end docstring:

###;;;autoload
fn run-hooks-until   condition hook-name args \
{
  let (result =)
    {
      for (do-hook = $$hook-name)
        {
          result = <={ $do-hook $args }
          if { $condition $result } \
               { break }
        }
      result $result
    }
}

#:docstring add-hook:
# Usage: add-hook hook function
#
# Append to the value of `hook' the function `function' unless already
# present.  `hook' should be a symbol and `function' may be any valid
# function name or lambda expression.  hook's value should be a list of
# functions, not a single function.
#
# To detect whether `function' is already present in the hook, `add-hook'
# does simple pattern matching.  It cannot identify equivalent but
# different lambda expressions.  For example, it would consider
# @ x {result x} and @ y {result y} different.
#
# The return value of `add-hook' is the new value of `hook'.
#:end docstring:

###;;;autoload
fn add-hook    hook function \
{
   if { ! ~ $function $$hook } \
        { $hook = $$hook $function }
   result $$hook
}

#:docstring insert-hook:
# Usage: insert-hook hook function
#
# Like `add-hook', but prepend function to the beginning of hook list
# rather than appending it to the end.  See docstring for `add-hook' for
# caveats about pattern matching.
#
# The return value of `insert-hook' is the new value of `hook'.
#:end docstring:

###;;;autoload
fn insert-hook    hook function \
{
   if { ! ~ $function $$hook } \
        { $hook = $function $$hook }
   result $$hook
}

#:docstring remove-hook:
# Usage: remove-hook hook function
#
# Remove from the value of `hook' the function `function' if present.
# `hook' should be a symbol and `function' may be any valid function name
# or lambda expression.  hook's value should be a list of functions, not a
# single function.
#
# To detect whether `function' is present in the hook, `remove-hook' does
# simple pattern matching.  It cannot identify equivalent but different
# lambda expressions.  For example, it would consider
# @ x {result x} and @ y {result y} different.
#
# The return value of `remove-hook' is the new value of `hook'.
#:end docstring:

###;;;autoload
fn remove-hook    hook function \
{
  # speed hack: if it looks like the function isn't a member via pattern
  # matching, avoid an expensive element-by-element search.
  if { ! ~ $function $$hook } \
       { result $$hook } \
     {
       let (list = $$hook; nlist =)
         {
           while { ! ~ $list () } \
             {
               if { ~ $list(1) $function } \
                    {
                      $hook = $nlist $list(2 ...)
                      list =
                    } \
                  {
                    nlist = $nlist $list(1)
                    list = $list(2 ...)
                  }
             }
           result $$hook
         }
     }
}

provide hook

# hook.es ends here
