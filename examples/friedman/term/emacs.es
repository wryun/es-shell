# emacs.es
# Author: Noah Friedman <friedman@splode.com>
# Created: 1991-12-11
# Public domain

# $Id: emacs.es,v 1.5 2010/03/04 09:44:36 friedman Exp $

# Commentary:
# Code:

PAGER = cat
EDITOR = ed

# Unset ls pager function, which under normal conditions would pipe ls
# through a pager.  That's not necessary under emacs since we can already
# scroll through the buffer.
fn-ls-page = ls

# emacs.es ends here
