# path-list.es --- functions for parsing path files
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1992-05-11
# Public domain

# $Id: path-list.es,v 1.2 1994/11/01 21:10:18 friedman Exp $

# Commentary:
# Code:

#:docstring path-list:
# Usage: path-list [file1] {files...}
#
# Parse {FILE1 FILES...}, which is assumed to contain a list of names
# separated by whitespace (tabs, spaces, or newlines), and construct a list
# consisting of those names.  Comments in the file (lines beginning
# with `#') are ignored, but names and comments cannot exist on the
# same line.
#:end docstring:

###;;;autoload
fn path-list \
{
  if { ~ $#* 0 } \
       { throw error $0 usage: $0 [file1] '{...}' }

  eval result `{ sed -e '/^[	 ]*[#].*/d' $* }
}    

#:docstring path-list-verify:
# Usage: path-list-verify [predicate] [file1] {files...}
# 
# Like path-list, but don't return names that don't satisfy predicate.
# Some standard predicates available include:
#     path-list:name-exists?
#     path-list:name-exists-as-file?
#     path-list:name-exists-as-directory?
#:end docstring:

###;;;autoload
fn path-list-verify  predicate names \
{
  let (newlist =)
    {
      for (dir = <={ path-list $names })
       if { $predicate $dir } \
            { newlist = $newlist $dir }

      result $newlist
    }
}    

fn path-list:name-exists?              { access    $1 }
fn path-list:name-exists-as-file?      { access -f $1 }
fn path-list:name-exists-as-directory? { access -d $1 }

provide path-list

# path-list.es ends here
