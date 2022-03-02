# list.es --- `cons' data type plus list operations
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-11-01
# Last modified: 1993-12-31
# Public domain

# Commentary:

# This library implements and preserves the semantics of most of the
# procedures documented in chapter 7 (lists) of the MIT Scheme Reference
# Manual, Edition 1.1.1 for MIT Scheme 7.1.3.

# Code:

require plist
require equal

###;;;autoload
fn cons   car cdr \
{
  result { result @ { result $$1 } @ s v { $s = $v } }
}

###;;;autoload
fn pair? { ~ $1 '%closure(car='*';cdr='*'{'* }

###;;;autoload
fn-cons? = 'pair?'

###;;;autoload
fn null? { ~ $1 () }


# Equality predicates

# Install type-specific predicates on the standard equality plists.
put 'eq?'    'pair?' 'pair-eq?'
put 'eqv?'   'pair?' 'pair-eqv?'
put 'equal?' 'pair?' 'pair-equal?'

# It's ironic that eq? is more expensive for pairs than equal?, but the `~'
# pattern matcher in es isn't otherwise clever enough.
###;;;autoload
fn pair-eq? \
{
  let (old-car = <={ car $1 }
       eq =)
    {
      unwind-protect \
          {
            set-car! $1 $old-car fnord
            eq = <={ pair-equal? $1 $2 }
          } \
        {
          set-car! $1 $old-car
        }
      result $eq
    }
}

###;;;autoload
fn-pair-eqv? = 'pair-equal?'

###;;;autoload
fn pair-equal? { ~ $1 $2 }


# pair accessing interface

let (fn-cons-op = @ caller cons method op arg \
     {
       if { null? $cons } \
            { result () } \
          { pair? $cons } \
            {
              cons = <={ $cons }
              $cons($method) $op $arg 
            } \
          { throw error $0 $caller: 'pair?,' $cons }
     })
  {
###;;;autoload
fn car { $fn-cons-op $0 $1 1 $0 }

###;;;autoload
fn cdr { $fn-cons-op $0 $1 1 $0 }

###;;;autoload
fn set-car! { $fn-cons-op $0 $1 2 car $*(2 ...) }

###;;;autoload
fn set-cdr! { $fn-cons-op $0 $1 2 cdr $*(2 ...) }
  }

###;;;autoload
fn general-car-cdr   object path \
{
  let (new-path =)
    {
      path = <={ %fsplit '' $path }
      if { ! ~ $path(1) a d } \
           { path = $path(2 ...) }

      for (i = $path)
        new-path = $i $new-path
      path = $new-path

      if { ! ~ $path(1) a d } \
           { path = $path(2 ...) }
    }

  let (a = car; d = cdr; op = $path(1))
    while { ~ $op a d } \
      {
        object = <={ $$op $object }
        path = $path(2 ...)
        op = $path(1)
      }

  result $object
}

###;;;autoload
fn caar   { general-car-cdr $1 $0 }

###;;;autoload
fn cadr   { general-car-cdr $1 $0 }

###;;;autoload
fn cdar   { general-car-cdr $1 $0 }

###;;;autoload
fn cddr   { general-car-cdr $1 $0 }


###;;;autoload
fn caaar  { general-car-cdr $1 $0 }

###;;;autoload
fn caadr  { general-car-cdr $1 $0 }

###;;;autoload
fn cadar  { general-car-cdr $1 $0 }

###;;;autoload
fn caddr  { general-car-cdr $1 $0 }

###;;;autoload
fn cdaar  { general-car-cdr $1 $0 }

###;;;autoload
fn cdadr  { general-car-cdr $1 $0 }

###;;;autoload
fn cddar  { general-car-cdr $1 $0 }

###;;;autoload
fn cdddr  { general-car-cdr $1 $0 }


###;;;autoload
fn caaaar { general-car-cdr $1 $0 }

###;;;autoload
fn caaadr { general-car-cdr $1 $0 }

###;;;autoload
fn caadar { general-car-cdr $1 $0 }

###;;;autoload
fn caaddr { general-car-cdr $1 $0 }

###;;;autoload
fn cadaar { general-car-cdr $1 $0 }

###;;;autoload
fn cadadr { general-car-cdr $1 $0 }

###;;;autoload
fn caddar { general-car-cdr $1 $0 }

###;;;autoload
fn cadddr { general-car-cdr $1 $0 }

###;;;autoload
fn cdaaar { general-car-cdr $1 $0 }

###;;;autoload
fn cdaadr { general-car-cdr $1 $0 }

###;;;autoload
fn cdadar { general-car-cdr $1 $0 }

###;;;autoload
fn cdaddr { general-car-cdr $1 $0 }

###;;;autoload
fn cddaar { general-car-cdr $1 $0 }

###;;;autoload
fn cddadr { general-car-cdr $1 $0 }

###;;;autoload
fn cdddar { general-car-cdr $1 $0 }

###;;;autoload
fn cddddr { general-car-cdr $1 $0 }


# TODO: make this recognize circular lists (right now it loops forever)
# Circular lists are not "lists" by the Scheme definition, so if this
# condition ever becomes detectable `list?' should return false for it.
###;;;autoload
fn list? \
{
  if { ~ $* 0 } \
       { * = 1 }

  while { ! ~ $* 0 1 } \
    {
      if { null? $* } \
           { * = 0 } \
         { pair? $* } \
           { * = <={ cdr $* } } \
         { * = 1 }
    }

  result $*
}

###;;;autoload
fn make-list  k init \
{
  let (count =; new-list =; p =)
    {
      while { ! ~ $#count $k } \
        {
          count = $count ''

          p = <={cons}
          set-car! $p $init
          set-cdr! $p $new-list

          new-list = $p
        }
      result $new-list
    }
}

###;;;autoload
fn list \
{
  # This algorithm is O(2N).
  let (revlist =; result =)
    {
      for (i = $*) 
        revlist = $i $revlist
      for (i = $revlist)
        result = <={ cons $i $result }
      result $result
    }
}

###;;;autoload
fn cons* \
{
  let (revlist =; result =)
    {
      for (i = $*) 
        revlist = $i $revlist

      result = <={ cons $revlist(2) $revlist(1) }
      revlist = $revlist(3 ...)

      for (i = $revlist)
        result = <={ cons $i $result }

      result $result
    }
}

###;;;autoload
fn list-copy { %flatten ' ' $* }

###;;;autoload
fn-tree-copy = list-copy

###;;;autoload
fn list-length \
{
  let (i =)
    {
      while { pair? $1 } \
        {
          * = <={cdr $1}
          i = $i ''
        }
      result $#i
    }
}

###;;;autoload
fn-length = list-length

###;;;autoload
fn sublist     list start end \
{
  list = <={ list-copy $list }
  let (count =
       ptr =)
    {
      while { ! ~ $#count $start } \
        { 
          count = $count '' 
          list = <={ cdr $list }
        }

      ptr = $list
      while { ! ~ $#count $end } \
        { 
          count = $count ''
          ptr = <={ cdr $ptr }
        }
      if { pair? $ptr } \
           { set-cdr! $ptr }

      result $list
    }
}

###;;;autoload
fn list-head     list k \
{
  sublist $list 0 $k
}

###;;;autoload
fn list-tail     list k \
{
  let (count =)
    while { ! ~ $#count $k } \
      {
        count = $count ''
        list = <={ cdr $list }
      }
  result $list
}

###;;;autoload
fn list-ref     list k \
{
  car <={ list-tail $list $k }
}

###;;;autoload
fn first   { list-ref $1 0 }

###;;;autoload
fn second  { list-ref $1 1 }

###;;;autoload
fn third   { list-ref $1 2 }

###;;;autoload
fn fourth  { list-ref $1 3 }

###;;;autoload
fn fifth   { list-ref $1 4 }

###;;;autoload
fn sixth   { list-ref $1 5 }

###;;;autoload
fn seventh { list-ref $1 6 }

###;;;autoload
fn eighth  { list-ref $1 7 }

###;;;autoload
fn ninth   { list-ref $1 8 }

###;;;autoload
fn tenth   { list-ref $1 9 }


# Misc pair operations

###;;;autoload
fn last-pair \
{
  let (p = $1; q =)
    {
      while { pair? $p } \
        {
          q = $p
          p = <={ cdr $p }
        }
      result $q
    }
}

###;;;autoload
fn except-last-pair { $0^! <={ list-copy $1 } }

###;;;autoload
fn except-last-pair! \
{
  let (p = $1
       q = <={ cdr $1 }
       r =)
    {
      r = <={ cdr $q }
      while { pair? $r } \
        {
          p = $q
          q = <={ cdr $q }
          r = <={ cdr $q }
        }
      set-cdr! $p
    }
  result $1
}


# List appending operations

###;;;autoload
fn append \
{
  let (lists =)
    {
      # Last list is not copied
      while { ! ~ $#* 0 1 } \
        {
          lists = $lists <={ list-copy $1 }
          * = $*(2 ...)
        }
      $0^! $lists $1
    }
}

###;;;autoload
fn append! \
{
  let (result = $1)
    {
      while { ! ~ $#* 1 0 } \
        {
          set-cdr! <={ last-pair $1 } $2
          * = $*(2 ...)
        }
      result $result
    }
}


# List reversing operations.

###;;;autoload
fn reverse { $0^! <={ list-copy $* } }

###;;;autoload
fn reverse!    list \
{
  let (p = $list
       q = <={ cdr $list }
       r =)
    {
      set-cdr! $p
      while {} \
        {
          r = <={ cdr $q }
          set-cdr! $q $p
          p = $q
          if { ~ $r () } \
               { break } \
             { q = $r }
        }
      result $q
    }
}


# list/vector operations

###;;;autoload
fn 'list->vector'    list \
{
  let (result =)
    {
      while { ! null? $list } \
        {
          if { pair? $list } \
               {
                 result = $result <={ car $list }
                 list = <={ cdr $list }
               } \
             {
               result = $result $list
               list =
             }
        }
      result $result
    }
}

###;;;autoload
'fn-vector->list' = list

###;;;autoload
fn 'subvector->list' \
{
  let (vector = $*(... <={ %count $*(3 ...) })
       start  = $*(<={ %count $*(2 ...) })
       end    = $*($#*))
    {
      list $vector($start ... $end)
    }
}


# list/string operations

###;;;autoload
fn 'list->string' { %flatten '' <={ 'list->vector' $* } }

###;;;autoload
fn 'string->list' { list <={ %fsplit '' $* } }

###;;;autoload
fn 'substring->list'    string start end \
{
  string = <={ %fsplit '' $string }
  list $string($start ... $end)
}


# list filters

###;;;autoload
fn delete-member-procedure    deletor predicate \
{
  result @ elt list \
    {
      <={$deletor @ { $predicate $* $elt } } $list
    }
}

###;;;autoload
fn list-deletor      predicate \
{
  result @ list \
    { 
      list-predicate-delete! <={ list-copy $list } $predicate 
    }
}

###;;;autoload
fn list-deletor!     predicate \
{
  result @ list \
    { 
      list-predicate-delete! $list $predicate
    }
}

###;;;autoload
fn list-predicate-delete!    list predicate \
{
  while { $predicate <={ car $list } } \
    { list = <={ cdr $list } }

  let (head = $list
       p = $list
       q = <={ cdr $list })
    {
      while { ! null? $q } \
        {
          if { $predicate <={ car $q } } \
               { 
                 set-cdr! $p <={ cdr $q } 
                 q = <={ cdr $p }
               } \
             {
               p = $q
               q = <={ cdr $p }
             }
        }
      result $head
    }
}

###;;;autoload
fn-delete  = <={ delete-member-procedure list-deletor equal? }

###;;;autoload
fn-delq    = <={ delete-member-procedure list-deletor eq? }

###;;;autoload
fn-delv    = delete

###;;;autoload
fn-delete! = <={ delete-member-procedure list-deletor! equal? }

###;;;autoload
fn-delq!   = <={ delete-member-procedure list-deletor! eq? }

###;;;autoload
fn-delv!   = delete!

###;;;autoload
fn list-transform-positive \
{
}

###;;;autoload
fn list-transform-negative \
{
}


# List mapping operations

###;;;autoload
fn map \
{
  let (fn = $1
       arglist = $*(2 ...)
       carlist =
       cdrlist =
       result =
       maplist =)
    {
      while { ! ~ $arglist () } \
        {
          carlist =
          cdrlist =
          for (a = $arglist)
            {
              carlist = $carlist <={car $a}
              cdrlist = $cdrlist <={cdr $a}
            }
          arglist = $cdrlist
          maplist = <={ cons '' $maplist }
          result = <={$fn $carlist}
          set-car! $maplist $result
        }
      result $maplist
    }
}

###;;;autoload
fn for-each \
{
  let (fn = $1
       arglist = $*(2 ...)
       carlist =
       cdrlist =)
    {
      while { ! ~ $arglist () } \
        {
          carlist =
          cdrlist =
          for (a = $arglist)
            {
              carlist = $carlist <={car $a}
              cdrlist = $cdrlist <={cdr $a}
            }
          arglist = $cdrlist
          $fn $carlist
        }
    }
}


###;;;autoload
fn list-sort \
{
  throw error $0 $0: list sorting not yet implemented.
}


provide list

# list.es ends here
