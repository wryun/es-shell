# type-pred.es --- misc type predicates for symbols
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1992-06-04
# Last modified: 1993-05-20
# Public domain

# Commentary:
# Code:

#:docstring alphabetic?:
# Usage: alphabetic? EXPR
#
# Return true if EXPR consists entirely of alphabetic characters (either
# upper or lower case), false otherwise.
#:end docstring:

###;;;autoload
fn alphabetic? { ! ~ $1 '' *[~a-zA-Z]* }

#:docstring alphanumeric?:
# Usage: alphanumeric? EXPR
#
# Return true if EXPR consists entirely of alphabetic characters (either
# upper or lower case) and/or numeric characters.  Otherwise return false.
#:end docstring:

###;;;autoload
fn alphanumeric? { ! ~ $1 '' *[~a-zA-Z0-9]* }

#:docstring lowercase?:
# Usage: lowercase? STRING
#
# Return true if STRING consists entirely of lower case alphabetic
# characters, otherwise return false.
#:end docstring:

###;;;autoload
fn lowercase? { ! ~ $1 '' *[~a-z]* }

#:docstring numeric?:
# Usage: numeric? EXPR
#
# Return true if EXPR consists entirely of numeric characters, false
# otherwise.
#:end docstring:

###;;;autoload
fn numeric? { ! ~ $1 *[~0-9]* }

#:docstring punctuation?:
# Usage: punctuation? STRING
#
# Return true if STRING consists entirely of punctuation characters---that
# is, any characters in the set
#
#        []{}()<>!*~@#%^=_+-*$&!\`\|;:'\",./?
#
# Return false if any other characters appear in STRING.
#:end docstring:
#TODO: make this work.
#-###;;;autoload
#fn punctuation? { ! ~ $1 *[~]~@'#'%'^'=_+-{}()*$&!'`'|;:''",.<>/?[]* }

#:docstring uppercase?:
# Usage: uppercase? STRING
#
# Return true if STRING consists entirely of upper case alphabetic
# characters, false otherwise.
#:end docstring:

###;;;autoload
fn uppercase? { ! ~ $1 '' *[~A-Z]* }

#:docstring whitespace?:
# Usage: whitespace? STRING
#
# Return true if STRING consists entirely of whitespace characters.
# Whitespace is defined to be spaces, tabs, newlines, or BEL characters
# (C-g).  Return false if STRING contains any other characters.
#:end docstring:

###;;;autoload
fn whitespace? { ! ~ $1 *[~\t\ \007\010\013]* }

provide type-pred

# type-pred.es ends here
