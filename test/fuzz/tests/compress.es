# compress.es --- table-driven compression method handler
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1994-01-16
# Last modified: 1994-03-31
# Public domain

# Commentary:
# Code:

###;;;autoload
fn compress:create-method-object    method-list \
{
  method-list =
  result @ msg parms \
    {
      if { ~ $msg value } \
           { result $method-list } \
         { ~ $msg set! } \
           { method-list = $parms } \
         {
           throw error compress.es invalid message to compress-method object, '`'^$msg^''''
         }
    }
}

###;;;autoload
fn compress:method-object? \
{
  ~ $1 '%closure(method-list='*')@ msg parms{'*
}

###;;;autoload
fn compress:add-method!    table type extension compress uncompress \
{
  if { ~ () <={ compress:get-method $table type $type } } \
    {
      $table set! <={ $table value } \
                  { result $type $extension $compress $uncompress }
    }
}

###;;;autoload
fn compress:get-method  table key value \
{
  let (type       = 1;
       extension  = 2;
       compress   = 3;
       uncompress = 4;
       result =;)
    {
      for (method = <={ $table value })
        {
          method = <={ $method }
          if { ~ $method($$key) $value } \
               { 
                 result = $method
                 break
               }
        }
      result $result
    }
}

###;;;autoload
fn compress:remove-method!   table method \
{
  if { ! ~ () <={ compress:get-method $table type $method } } \
       {
         let (method-list = <={ $table value }
              new-list =
              n =)
           {
             for (m = $method-list)
               {
                 n = <={ $m }
                 if { ! ~ $n(1) $method } \
                      { new-list = $new-list $m }
               }
             $table set! $new-list
           }
       }
}

provide compress

# compress.es ends here
