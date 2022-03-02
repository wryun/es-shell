# repl.es --- read-eval-print loops (with hooks) for es
# Author: Noah Friedman <friedman@splode.com>
# Created: 1993-04-25
# Public domain

# $Id: repl.es,v 1.4 2000/12/18 11:17:38 friedman Exp $

# Commentary:

# TODO:
#   * document REPL and related hooks
#   * write a batch REPL that supports similar hooks
#   * Create an exception-handler-alist which can be used to flexibly add
#     new exceptions or redefine existing ones, without having to redefine
#     the REPL itself.  This probably wants to be defined as a `box' so
#     that multiple alists can be defined and be lexically scoped, with
#     a well-defined settor/caller interface; make a separate package?

# Redefining the REPL while it is running won't cause the new instance to
# be used until it is explicitly called.  To accomplish this without
# running itself recursively (which will consume stack space) and while
# still allowing the current repl to finish any exception handling or other
# commands, there is a complicated interaction between repl-interactive,
# repl-prompt-hook, %interactive-hook, and set-fn-repl-interactive.  Here
# is what happens:
#
# When fn-repl-interactive is redefined, the settor set-fn-repl-interactive
# is invoked.  set-fn-repl-interactive adds `repl-send-restart-exception'
# to repl-prompt-hook.  When repl-interactive then calls repl-prompt-hook
# prior to reading the next command, repl-send-restart-exception will be
# called.  This function throws a `repl-restart' exception, which is caught
# by the exception handler in repl-interactive.  repl-interactive's handler
# then throws this exception again, causing it to be caught by the handler
# in %interactive-loop.  %interactive-loop will remove
# `repl-send-restart-exception' from repl-prompt-hook (to keep this loop
# from occuring endlessly), then call repl-interactive again.  In this way
# the old repl-interactive has returned and the new one is started.
#
# In order for this system to work when a new REPL is defined, any
# redefinition of repl-interactive must pass the `repl-restart' exception
# it catches up to the next handler via throw, and it must run
# repl-prompt-hook to make sure the exception gets thrown in the first
# place.
#
# Perhaps this necessitates some undesirably complex rules for one's own
# REPL to have to obey, but it seems necessary.  Because exception handlers
# in es catch *all* exceptions and are dynamically scoped, the exception
# handler in repl-interactive must know what to do with repl-restart no
# matter what.  And simply throwing the exception in
# set-fn-repl-interactive would abort any unfinished commands (the REPL
# might be redefined by a function, or there might simply be multiple
# commands on the command line); using repl-prompt-hook delays the
# exception until the current repl is finished processing all previous
# commands and possible exceptions.
#
# A possible improvement (to prevent other REPLs from having to run all of
# repl-prompt-hook) is to create a special hook (or even a simple flag!)
# for the repl-restart exception thrower, that is intended for no other
# purpose.  I haven't decided if this is worthwhile yet, but I could be
# easily convinced.
#
# The onus of handling repl-restart explicitly in repl-interactive would be
# resolved somewhat by using an exception-handler-alist.

# Code:

# Get better version of primitives, particularly to define `exit' in a way
# that causes an exception, rather than calling $&exit directly.
require primitives
require hook


fn repl-interactive    dispatch \
{
   if { ! ~ $dispatch () } \
        {
          repl-dispatch-function = $dispatch
        }

   let (result = 0;
        fn-print-exception-state = @ repl-msg exception source message \
        {
          repl-print-comment repl: $repl-msg: $exception
          for (sym = source message)
            {
              if { ! ~ $$sym () } \
                   {
                     repl-print-comment repl: $sym '=' $$sym
                   }
            }

        }

        # Note that this catches return.
        fn-simple-catch = @ body \
        {
          catch @ exception source message \
              {
                $fn-print-exception-state \
                  ('caught exception' $exception $source $message)
                result $exception $source
              } \
            { $body }
        })
     {
       $fn-simple-catch \
         {
           result = (<={ run-hooks repl-start-hook $repl-dispatch-function }
                     <={
                         run-hooks repl-interactive-start-hook \
                                   $repl-dispatch-function
                       })
         }
       catch @ exception source message \
           {
             # If this is converted into an external alist, then there
             # should be a way of resetting the `result' variable.  This
             # could probably either turn into a global
             # `repl-command-result' variable or allow handlers to set some
             # other dynamically-scoped variable and set `result' based on
             # that.  We want to be able to reset this variable so that the
             # prompt-hook won't do anything unecessary with return values
             # if they have already been taken care of specially.
             if { ~ $exception repl-restart } \
                  {
                    repl-print-comment repl: restarting
                    throw $exception
                  } \
                { ~ $exception eof exit } \
                  {
                    if { ~ $exception eof } \
                         {
                           echo >[1=2]
                         }
                    $fn-simple-catch \
                      {
                        run-hooks repl-interactive-exit-hook $source
                        run-hooks repl-exit-hook $source
                      }
                    result $source
                  } \
                { ~ $exception error } \
                  {
                    repl-print-comment repl: $exception: $message
                    result = 1
                    throw retry
                  } \
                { ~ $exception signal } \
                  {
                    # In the case of signals, $source is actually the
                    # signal name.
                    repl-print-comment repl: $exception: $source
                    result =
                    throw retry
                  } \
                {
                  $fn-print-exception-state \
                    ('unknown exception' $exception $source $message)
                  result = $exception $source
                  throw retry
                }
           } \
         {
           forever \
             {
               run-hooks repl-prompt-hook $result
               let (code = <={ %parse $prompt })
                 {
                   result = <={ run-hooks repl-interactive-parse-hook $code }
                 }
           }
         }
     }
}


# Note that this is the actual entry to the repl that es uses internally.
# We use this to our advantage by defining an exception handler capable of
# restarting the real repl under the right circumstances.
fn %interactive-loop    dispatch \
{
  # For backward compatibility, set repl-dispatcher to the
  # dispatch function passed to this function.
  repl-dispatch-function = $dispatch

  catch @ exception \
      {
        if { ~ $exception repl-restart } \
             {
               remove-hook repl-prompt-hook repl-send-restart-exception
               throw retry
             }
      } \
    {
      repl-interactive $repl-dispatch-function
    }
}

# This is defined after the initial definition of repl-interactive, to
# prevent an exception from occuring just by loading this file!
set-fn-repl-interactive = @ \
  {
    add-hook repl-prompt-hook repl-send-restart-exception
    result $*
  }

fn repl-send-restart-exception \
{
  throw repl-restart
}


fn repl-interactive-do-parsed-commands \
{
  if { ! ~ $* () } \
       { $repl-dispatch-function $* }
}

fn repl-increment-shlvl \
{
  es-SHLVL = $es-SHLVL ''
  result ()
}

#:docstring repl-print-result-comment:
# If set to be run by repl-prompt-hook, this function will
# check the return value(s) of the last executed command(s) and report if
# any were nonzero.
#:end docstring:

fn repl-print-result-comment    result \
{
  if { ~ $result *[~0]* } \
       { repl-print-comment 'result =' $result }
}

#:docstring repl-print-multi-line-result-comment:
# If set to be run by repl-prompt-hook, this function will
# check the return value(s) of the last executed command(s) and report if
# any were nonzero.  If there is more than one return value, each will be
# printed on a separate line of the form `# result n = value'.  This
# helps to disambiguate any whitespace that may appear in return values.
#:end docstring:

# This version prints each value of the result on a separate line, to
# disambiguate from whitespace in any of the values
fn repl-print-multi-line-result-comment    result \
{
  if { ~ $result *[~0]* } \
       {
         if { ~ $#result 1 } \
              { repl-print-comment 'result =' $result } \
            {
              let (cnt = '')
                {
                  for (res = $result)
                    {
                      repl-print-comment result $#cnt '=' $res
                      cnt = $cnt ''
                    }
                }
            }
       }
}

fn repl-print-comment \
{
  {
    if { ~ $* *\n* } \
         {
           echo $* \
            | sed -ne '1s/^/# /
                       1!s/^/#/
                       p'
         } \
       { ! ~ $* () } \
         { echo '#' $* }
  } >[1=2]
}


# This is necessary if you want to be able to run any commands interactively!
add-hook repl-interactive-parse-hook repl-interactive-do-parsed-commands

# These shouldn't be done by default, but they are recommended by the author.
#add-hook repl-prompt-hook            repl-print-multi-line-result-comment
#add-hook repl-interactive-start-hook repl-increment-shlvl

provide repl

# repl.es ends here
