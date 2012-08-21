# cd [ -[N] | dir ]
#
# This replaces the builtin version of cd, absorbing all of its features
# and adding the ability to jump back to the Nth previous directory. The
# number of previous directories that cd remembers may be adjusted by
# incrementing cd-stack-depth.  The size of the stack is actually
# cd-stack-depth + 1 (if cd-stack-depth is not set, then the stack becomes
# unabounded).

cd-stack-depth = 3
let ( cd-stack = .; cwd = `pwd ) {
   fn cd dir { 
      if {~ $#dir 0} {
         if {! ~ $#home 1} {
            throw error cd <={
               if {~ $#home 0} {
                  result 'cd: $home not set'
               } {
                  result 'cd: $home contains more than one word'
               }
            }
         }
         dir = $home
      } {~ $#dir 1} {
         if { ~ $dir -* } {
            let (index = <={%split - $dir}) {
               if {~ $#index 0} {
                  index = 1
               } { ! ~ $index [0-9]* } {
                  throw error cd 'cd: invalid argument'
               }
               dir = $cd-stack($index)
               echo $dir >[1=2]
            }
            if { ~ $#dir 0 } {
               throw error cd 'cd: stack not that deep'
            }
         } {! %is-absolute $dir} {
            let (old = $dir) {
               dir = <={%cdpathsearch $dir}
               if {! ~ $dir $old} {
                  echo $dir >[1=2]
               }
            }
         }
      } {
         throw error cd 'usage: cd [-[N]|[directory]]'
      }

      if {$&cd $dir} {
         cd-stack = ($cwd $cd-stack( 1 ... $cd-stack-depth ))
         cwd = $dir
      }
   }
}
