# prompt.es --- fancy interactive prompt features
# Author: Noah Friedman <friedman@splode.com>
# Created: 1993-04-25
# Public domain

# $Id: prompt.es,v 1.2 2000/12/18 11:17:38 friedman Exp $

# Commentary:

# This provides features intended for interactive use.

# Code:

require hook

# Compress $home component of pwd into `~'
###;;;autoload
fn pwd-pretty \
{
  let (pwd = `pwd; pwdl = ; homel =)
    {
      if { ~ $pwd $home } \
           { result '~' } \
         { ! ~ $pwd $home^* } \
           { result $pwd } \
         {
           pwdl = <={ %split '/' $pwd }
           homel = <={ %split '/' '.' $home }
           pwdl = $pwdl($#homel ...)
           %flatten '/' '~' $pwdl
         }
    }
}

###;;;autoload
fn-prompt-set-cnp-long = \
{
  fn prompt-set \
  {
    prompt =
    if { ! ~ $USER () } \
         {
           prompt = $prompt 'user = '^$USER
         }
    if { ! ~ $HOSTNAME () } \
         {
           prompt = $prompt 'host = '^$HOSTNAME
         }
    if { ! ~ $es-SHLVL () } \
         {
           prompt = $prompt 'shlvl = '^$#es-SHLVL
         }
    prompt = '# '^<={ %flatten \t $prompt }
    prompt = $prompt^\n^'# cwd = '^<={pwd-pretty}^\n'; '

    # No continuation prompt in cnp mode.  This gives the interpreter a more
    # lisp listener--like feel.
    #prompt = $prompt '>'
    result ()
  }
  prompt-set
}

###;;;autoload
fn-prompt-set-cnp-short = \
{
  fn prompt-set \
  {
    prompt = '# cwd = '^<={pwd-pretty}^\n^'; '

    # No continuation prompt in cnp mode.  This gives the interpreter a more
    # lisp listener--like feel.
    #prompt = $prompt '>'
    result ()
  }
  prompt-set
}

###;;;autoload
fn-prompt-set-long = \
{
  fn prompt-set \
  {
    prompt = ''
    if { ! ~ $USER () }      { prompt = $prompt$USER }
    if { ! ~ $HOSTNICK () }  { prompt = $prompt'@'$HOSTNICK }
    if { ! ~ $es-SHLVL () }  { prompt = $prompt'['$#es-SHLVL']' }
    if { ! ~ $prompt '' () } { prompt = $prompt': ' }
    prompt = $prompt^<={pwd-pretty}^' ; '
    # Add secondary prompt
    prompt = $prompt '> '
    result ()
  }
  prompt-set
}

###;;;autoload
fn-prompt-set-short = \
{
  fn prompt-set \
  {
    prompt = (
               <={pwd-pretty}^' ; '
               '> '
             )
    result ()
  }
  prompt-set
}

add-hook cd-hook     prompt-set
add-hook popd-hook   prompt-set
add-hook pushd-hook  prompt-set
add-hook repl-interactive-start-hook prompt-set

# Initialize
prompt-set-short

provide prompt

# prompt.es ends here
