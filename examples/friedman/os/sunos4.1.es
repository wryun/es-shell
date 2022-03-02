# sunos4.1.es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-05-18
# Public domain

# $Id: sunos4.1.es,v 1.1 1995/10/04 00:24:18 friedman Exp $

# Commentary:

# In SunOS 4.1, there is inscrutable magic that goes on with dlopen (a
# dynamic linking routine) such that it will fail if a directory with
# libdl.so is referenced via a symlink.  Therefore, this template removes
# symlinks from LD_LIBRARY_PATH.

# Code:

verbose-startup os/$OS.es

# access -1 returns path if found, i.e. return value will be nonzero, hence
# the use of !.
if { ! ~ $LD_LIBRARY_PATH () && ! access -1n perl $path } \
     {
       let (new =)
         {
           for (d = <={ %split ':' $LD_LIBRARY_PATH })
             {
               if { ! access -l $d && access -d $d } \
                    { new = $new $d }
             }
           LD_LIBRARY_PATH = <={ %flatten ':' $new }
         }
     }

verbose-startup

# sunos4.1.es ends here
