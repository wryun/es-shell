# sun.es
# Author: Noah Friedman <friedman@splode.com
# Created: 1993-05-18
# Public domain

# $Id: sun.es,v 1.2 2010/03/04 09:44:36 friedman Exp $

# Commentary:
# Code:

# Make terminal black-on-white
fn-bow = echo -n \033^'[p'

# Make terminal white-on-black
fn-wob = echo -n \033^'[q'

# echoing a C-l (form feed) clears the screen on suns
fn-cls = echo -n \f

# Function to fix screwy hiking on the console, which can cause the screen
# to be scrolled halfway even if only one line of scrolling is needed.
# Echoing this sequence on the console seems to make it sane again.
fn-fixhike = echo -n \033^'[1r'

#LINES = 34
#COLUMNS = 80
#stty erase '^?' rows $LINES columns $COLUMNS >[2] /dev/null

# sun.es ends here
