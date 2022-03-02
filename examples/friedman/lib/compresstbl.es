# compresstbl.es --- compression method handler using compress.es
# Author: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1994-03-31
# Last modified: 1994-03-31
# Public domain

# Commentary:
# Code:

require compress

# table type extension compress uncompress
fn compresstbl:add-method! \
{
  compress:add-method! $compress-method-table $*
}

fn compresstbl:get-method \
{
  compress:get-method $compress-method-table $*
}

fn compresstbl:remove-method! \
{
  compress:remove-method! $compress-method-table $*
}

###

compress-method-table = <={ compress:create-method-object }
for (method = (
       { result compact   .C compact  uncompact            }
       { result compress  .Z compress @{compress -d -c $*} }
       { result gzip     .gz gzip     @{gzip     -d -c $*} }
       { result pack      .z pack     unpack               }
       { result yabba     .Y @{let (f=`{basename $1 .Y}) {yabba < $1 > $f}} \
                                      @{unyabba < $1 > $1^.Y}}
       { result unknown   '' cat      cat                  })
    )
  {
    compresstbl:add-method! <={ $method }
  }

provide compresstbl

# compress.es ends here
