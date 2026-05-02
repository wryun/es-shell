# vt100.es --- terminal-specific code for vt100 class terminals
# Author: Noah Friedman <friedman@splode.com>
# Created: 1993-05-18
# Public domain

# $Id: vt100.es,v 1.2 2010/03/04 09:44:36 friedman Exp $

# Change terminal to black-on-white
fn-bow = echo -n \033^'[?5l'

# Change terminal to white-on-black
fn-wob = echo -n \033^'[?5h'

#LINES = 24
#COLUMNS = 80
#stty erase '^?' rows $LINES columns $COLUMNS >[2] /dev/null

# vt100.es ends here
