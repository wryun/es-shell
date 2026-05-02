#
# number.es, 26-May-93 Noah Friedman <friedman@prep.ai.mit.edu>
# Last modified 18-Feb-94
#
# Public domain.
#

#:docstring number:
# Usage: number [number]
#
# Converts decimal integers to english notation.  Spaces and commas are
# optional.  Numbers 67 digits and larger will overflow this script.
#
# E.g: number 99,000,000,000,000,454
#      => ninety-nine quadrillion four hundred fifty-four
#
#:end docstring:

fn number \
{
  if { ~ $* *[~0-9,.]* } \
       { throw error $0 $0: invalid character in argument. }
     { ~ $* *.* } \
       { throw error $0 $0: fractions not supported '(yet).' }

  # Strips excess spaces and commas, and puts each digit into a separate
  # slot in the array.
  * = <={ %fsplit '' <={%flatten '' <={%fsplit ', ' $^* } } }

  let (ones = one two three four five six seven eight nine;
       tens = ten twenty thirty forty fifty sixty seventy eighty ninety;
       teens = eleven twelve (thir four fif six seven eigh nine)^teen;
       bignum = (thousand 
                 (m b tr quadr quint sext sept oct non 
                  ('' un duo tre quattuoro quin sex septen octo novem)^dec
                  vigint)^illion );
       a = $*
       bignum-ref = ;
       val100 =; val10 =; val1 =;
       result =)
    {
      while { ! ~ $#a 0 1 2 3 } \
        { 
          a = $a(4 ...) 
          bignum-ref = $bignum-ref ''
        }

      if { ~ $#a 1 } \
           { * = 0 0 $* } \
         { ~ $#a 2 } \
           { * = 0 $* }

      while { ! ~ $* () } \
        {
          val100 =; val10 =; val1 =;
          if { ! ~ $1 0 } { val100 = $ones($1) hundred }
          if { ! ~ $2 0 } { val10 = $tens($2) }
          if { ! ~ $3 0 } \
               { 
                 if { ~ $val10 ten } \
                      { val10 = ; val1 = $teens($3) } \
                    { val1 = $ones($3) } 
               }

          result = $result $val100 
          if { ~ $val10 *ty && ! ~ $val1 () } \
               { result = $result $^val10^-^$val1 } \
             { result = $result $val10 $val1 }
          if { ! { ~ $bignum-ref () || ~ $1$2$3 000 } } \
               { 
                 result = $result $bignum($#bignum-ref) 
               }
          bignum-ref = $bignum-ref(2 ...)
          * = $*(4 ...)
        }
      result $result
    }
}

# eof
