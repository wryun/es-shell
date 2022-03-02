# env.es --- define environment variables
# Author: Noah Friedman <friedman@splode.com>
# Created: 1993-05-18
# Public domain

# $Id: env.es,v 1.12 2010/03/04 09:38:03 friedman Exp $

# Commentary:
# Code:

# First set a temporary path so we can set some needed variables, then load
# functions we need during initialization time.
path = $sinit/bin $path

local (d = $sinit/share/paths;

     fn-verified-dirlist = @ \
     {
       do-if-exist $1 \
         { path-list-verify 'path-list:name-exists-as-directory?' $1 }
     }

     fn-verified-filelist = @ \
     {
       do-if-exist $1 \
         { path-list-verify 'path-list:name-exists-as-file?' $1 }
     }

     fn-flatten = @ \
     {
       let (s = <={ %flatten $* })
         {
           if { ~ $s 0 } \
                { s = () }
           result $s
         }
     }

     fn-defvar-dirlist = @ var file \
     { defvar $var <={ $fn-verified-dirlist $file } }

     fn-defvar-filelist = @ var file \
     { defvar $var <={ $fn-verified-filelist $file } }

     fn-defvar-flat-dirlist = @ var file \
     { defvar $var <={ $fn-flatten : <={ $fn-verified-dirlist $file } } }

     fn-defvar-flat-filelist = @ var file \
     { defvar $var <={ $fn-flatten : <={ $fn-verified-filelist $file } } }

     fn-defvar-cmd = @ var package cmd \
     {
       if { ~ $$var () } \
            {
              if { ! ~ $package '' } \
                   { require $package }
              $var = `{ $cmd }
            }
     }
    )
  {
    # Useful to have set before initializing real path.
    $fn-defvar-cmd SINIT_MACHTYPE '' hosttype
    defvar OS <={ let (l = <={ %split - $SINIT_MACHTYPE }) result $l(3) }

    $fn-defvar-cmd HOSTNAME '' hostname-fqdn

    require path-list
    do-if-exist $d/path    { path   = <={ $fn-verified-dirlist $d/path   } }
    do-if-exist $d/cdpath  { cdpath = <={ $fn-verified-dirlist $d/cdpath } }

    # $shellname is used in setting fpath by $sinit/share/paths/fpath
    local (shellname = es)
      do-if-exist $d/fpath { fpath  = <={ $fn-verified-dirlist $d/fpath  } }

    $fn-defvar-flat-dirlist  LD_LIBRARY_PATH $d/ld_library_path
    $fn-defvar-flat-dirlist  LD_RUN_PATH     $d/ld_library_path
    $fn-defvar-flat-filelist MAILCAPS        $d/mailcaps
    $fn-defvar-flat-dirlist  MANPATH         $d/manpath
    #$fn-defvar-flat-dirlist TEXFONTS        $d/texfonts
    #$fn-defvar-flat-dirlist TEXFORMATS      $d/texformats
    #$fn-defvar-flat-dirlist TEXINPUTS       $d/texinputs
    #$fn-defvar-flat-dirlist TEXPOOL         $d/texpool

    # Used by path-list.es library.
    $fn-defvar-filelist      termcapfiles    $d/termcapfiles
    $fn-defvar-dirlist       terminfodirs    $d/terminfodirs
  }


defvar EDITOR          ed
defvar HOSTALIASES     $sinit/share/hostaliases
defvar LESS            '-adeiqs -h10 -P--Less--?pB(%pB\%).'
defvar MAILRC          $sinit/share/mailrc
defvar MORE            '-s'
defvar NCFTPDIR        $sinit/share/ncftp
defvar SCREENRC        $sinit/share/screenrc

set-USER

# Set TERM if necessary.
if { ~ $TERM xterm } \
     { EMACS = } \
   { ~ $EMACS t } \
     { TERM = emacs }

if { ~ $TERM '' su unknown dialup network dumb plugboard } \
     {
       echo
       setterm
     }

# used by some GNU utils for making backups
defvar VERSION_CONTROL numbered
defvar XINITRC         $sinit/share/xinitrc

# Some variables for common nonprintable characters.
e = \033  # ESC
g = \007  # BEL
t = \011  # TAB

source-local-es-init-file env

# env.es ends here
