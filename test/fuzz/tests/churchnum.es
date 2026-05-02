# churchnum.es --- example of church numerals in es
# Author: Harald Hanche-Olsen <hanche@ams.sunysb.edu>
# Maintainer: Noah Friedman <friedman@prep.ai.mit.edu>
# Created: 1993-11-07
# Last modified: 1993-11-07

# Commentary:

# An sample implementation of Church numerals using lambda calculus in es.
# TODO: Get explicit permission from author to redistribute this freely.

# Code:

fn Compose f g {result @ x {$f <={$g $x}}}
fn Succ n {result @ f {Compose <={$n $f} $f}}
fn Plus m n {<={$m Succ} $n}
fn Prod m n {result @ f {$m <={$n $f}}}
fn Power m n {$m $n}
One  =  $&result

# Sample usage:
#
# ; Two  =<={Succ $One}
# ; Three=<={Succ $Two}
# ; Nine =<={Prod $Three $Three}
# ; Twelve=<={Plus $Three $Nine}
# ; Twenty-Seven=<={Power $Three $Three}
# ; dots=@ x {result . $x}
# ; for (i = Three Nine Twelve Twenty-Seven) echo <={<={$$i $dots} $i}} 
# . . . Three
# . . . . . . . . . Nine
# . . . . . . . . . . . . Twelve
# . . . . . . . . . . . . . . . . . . . . . . . . . . . Twenty-Seven

provide churchnum

# churchnum.es ends here
