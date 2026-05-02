# settor.es --- define various useful settors
# Author: Noah Friedman <friedman@splode.com>
# Created: 1993-05-18
# Last modified: 1993-09-25
# Public domain

# Commentary:
# Code:

# Keep VISUAL and EDITOR in sync, since it's hard to guess which variable a
# given program will actually use anyway.
set-VISUAL = @ { local (set-EDITOR =) EDITOR = $*; result $* }
set-EDITOR = @ { local (set-VISUAL =) VISUAL = $*; result $* }

# Keep LOGNAME (SysV) and USER (BSD) in sync.
set-LOGNAME = @ { local (set-USER =)    USER = $*;    result $* }
set-USER    = @ { local (set-LOGNAME =) LOGNAME = $*; result $* }

# Keep LPDEST (SVR4) and PRINTER (BSD) in sync.
set-PRINTER = @ { local (set-LPDEST  =) LPDEST  = $*; result $* }
set-LPDEST  = @ { local (set-PRINTER =) PRINTER = $*; result $* }

# When value of TERM changes, save old value in $OLDTERM.  This allows
# subshells to know if they should source term files, etc. upon startup.
# OLDTERM shouldn't be set by anything else.
set-TERM    = @ { local (set-OLDTERM =) OLDTERM = $TERM; result $* }
set-OLDTERM = @ { throw error $0 attempt to modify read-only variable OLDTERM }

set-HOSTNAME = @ \
{
  local (fields = <={%split . $* };
         set-HOSTNICK =)
    HOSTNICK = $fields(1)
  result $*
}

source-local-es-init-file settor

# settor.es ends here
