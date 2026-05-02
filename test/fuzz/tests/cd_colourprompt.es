let (cd = $fn-cd) { fn cd {$cd $*;
  let (c = \001\033; z = \002) {
    wd = `pwd
    prompt = $c[4\;35m$z`{hostname}^$c[0m$z:$c[1\;34m$z$^wd$c[0m$z^'; '
  }}
}
