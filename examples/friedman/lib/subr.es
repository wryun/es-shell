# subr.es --- various useful routines for es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-04-24
# Public domain

# $Id: subr.es,v 1.2 1994/06/03 16:11:58 friedman Exp $

# Commentary:
# Code:

#:docstring command:
# Usage: command [cmd] {...}
# 
# Invoke `cmd' as an external command, even if a shell function by that
# name exists.  
#:end docstring:

###;;;autoload
fn command { local (fn-$1 =) $* }

#:docstring console?:
# Usage: console?
#
# Returns true if controlling tty is a console (/dev/console on most
# machines, but AIX considers /dev/hft/* to be consoles as well).  Returns
# false otherwise.
#:end docstring:

# TODO: make this more easily extensible or remove it.
###;;;autoload
fn console? \
{
  if { ~ $1 () } \
       {
         if { ~ $TTY () } \
              { * = `{ tty >[2] /dev/null } } \
            { * = $TTY }
       }

  # /dev/hft/* are IBM AIX consoles (of which there may be more than one)
  # /dev/vga is a NetBSD console
  ~ $1 /dev/console /dev/hft/* /dev/vga
}

#:docstring defvar:
# Usage: defvar variable value ...
#
# Set value of VARIABLE to VALUEs only if VARIABLE is currently unset.
#:end docstring:

###;;;autoload
fn defvar  variable value \
{
    if { ~ $$variable () } { $variable = $value }
    result $value
}       

#:docstring do-if-exist:
# Usage: do-if-exist [file] [command]
#
# Execute `command' only if `file' exists. 
#:end docstring:

###;;;autoload
fn do-if-exist  file cmd \
{ 
  if { access $file } { $cmd } 
}

#:docstring expand-file-name:
# Usage: expand-file-name filename {cwd}
#
# Convert filename to absolute (canonicalize it), and return result.
#
# Filenames containing . or .. as components are simplified.
# Second arg is directory to be considered the "current working directory"
# for the purposes of resolving "./foo" or "foo" if not the default cwd. 
#:end docstring:

###;;;autoload
fn expand-file-name \
{
  if { ! ~ $1 /* } \
       { 
         if { ~ $2 () } \
              { * = `pwd^/$1 } \
            { * = $2^/^$1 }
       }
  let (list = <={ %split '/' $1 }; nlist =; dotdot =)
    {
      # Reverse list. 
      for (elt = $list) { nlist = $elt $nlist }
      list = $nlist
      nlist =

      for (elt = $list)
        if { ~ $elt '.' } \
             { result 0 } \
           { ~ $elt '..' } \
             { dotdot = t $dotdot } \
           { ~ $dotdot () } \
             { nlist = $elt $nlist } \
           { dotdot = $dotdot(2 ...) }

      if { ~ $nlist () } { nlist = '' }
      %flatten '/' '' $nlist
    }
}

#:docstring genvar:
# Usage: genvar
#
# Generate a symbol not presently the name of a variable in the global
# environment (which includes dynamicly-scoped variables, but not
# lexically-scoped ones).
#:end docstring

###;;;autoload
fn genvar \
{
  let (vars = <={ $&vars };
       sym-base = '$$genvar$$-';
       new-sym =;
       n = <={ $&vars } '')
    {
      new-sym = $sym-base^$#n
      while { ~ $vars $new-sym } \
        {   
          n = $n ''
          new-sym = $sym-base^$#n
        }
      result $new-sym
    }
}

#:docstring ignore-exceptions:
# Usage: ignore-exceptions exception-list body
#
# Execute body forms while catching any exceptions included in
# exception-list.  If exception-list is empty, catch all exceptions.
# If an exception is caught that is not in the exception-list, the
# exception is thrown again to be caught by whatever superior handler may
# exist.  `ignore-exceptions' never catches return.
#
# If no exceptions are caught, then the return value of ignore-exceptions is
# the result of the last expression in the body forms.  If an exception is
# caught and handled, the return value is the exception name, source, and
# message associated with the exception.
# 
# Example:
# 
# ignore-exceptions gratuitous-error another-random-error \
#   {
#     frob me baby && throw gratuitous-error frobme 'ignore me'
#     throw real-error frobme 'some error not handled by ignore-exceptions'
#   } 
# 
#:end docstring:

fn-ignore-exceptions = $&noreturn @ \
{
  let (argc = $#*;
       exception-list = '' $*;
       body =)
    {
      body = $*($argc)
      exception-list = $exception-list(2 ... $argc)

      catch @ exception source message \
          {
            if { ~ $exception-list () || ~ $exception $exception-list } \
                 { result $exception $source $message } \
               { throw $exception $source $message }
          } \
        { $body }
    }
}

#:docstring read:
# Usage: read [name ...]
#
# One line is read from the standard input, and the first word is assigned
# to the first NAME, the second word to the second NAME, etc.  with
# leftover words assigned to the last NAME.  Only the characters found in
# the variable `ifs' are recognized as word delimiters.  The return value
# is the line read.
#:end docstring:

###;;;autoload
fn read \
{
  let (result = `{ sh -c 'read x && echo "$x"' };
       tresult =)
    {
      tresult = $result
      while { ! ~ $* () } \
        {
          if { ~ $#* 1 } \
               { $1 = $tresult } \
             { $1 = $tresult(1) }
          * = $*(2 ...)
          tresult = $tresult(2 ...)
        }
      result $result
    }
}

###;;;autoload
fn-type = whatis

#:docstring unset:
# Usage: unset [var ...]
#
# Unset each variable VAR.
# Note: `unset' cannot affect lexically-scoped variables.
#:end docstring:

###;;;autoload
fn unset \
{
  for (v = $*) $v =
}

###;;;autoload
fn whoami \
{
  * = `{ id >[2] /dev/null | sed -ne 's/.*uid=[0-9]*(//;s/).*//;p' }
  if { ~ $* () '' } \
       { * = `{ local (fn-$0 =) $0 } }
  echo $*
}

provide subr

# subr.es ends here
