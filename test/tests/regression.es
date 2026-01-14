# regression.es -- regression tests for previous bugs

# these may be redundant with other tests, but what's the harm in having a
# little redundancy?

# These tests are based on bug reports and PRs on github.
# TODO not all tagged bugs have been covered. Some are hard to repro.
test 'regressions' {
	# https://github.com/wryun/es-shell/issues/5
	assert {~ `` \n {$&a. >[2=1]} 'invalid primitive name: $&a^.'}

	# https://github.com/wryun/es-shell/issues/7
	# slow and busy, but we're testing for a race condition
	assert {!{{
		for (i = `{awk 'BEGIN {for (i=1;i<=100;i++)print i}'})
			$es -c 'signals = sigterm; kill $pid'
	} |[2] grep child}}

	# https://github.com/wryun/es-shell/issues/8
	let (fd = <=%newfd) {
		catch @ {} {
			%open $fd /dev/null {
				throw blah
			}
		}
		let (exception = ()) {
			catch @ e {exception = $e} {%dup 1 $fd {}}
			assert {!~ $exception ()}
		}
	}

	# https://github.com/wryun/es-shell/issues/57
	assert {~ `` \n {let (n = '%closure (x = y) echo $x') {$n}} y}

	# https://github.com/wryun/es-shell/issues/63
	assert {~ `` \n {() echo success} success}

	# https://github.com/wryun/es-shell/issues/68
	assert {$es -c {catch @ {} {'%closure ()'}}}

	# https://github.com/wryun/es-shell/issues/78
	# this one requires GCDEBUG=1
	assert {$es '-cresult'}

	# https://github.com/wryun/es-shell/issues/85
	assert {$es -c {
		set-foo = result
		catch @ {} {
			local (foo = !) {
				throw whatever
			}
		}
	}}

	# https://github.com/wryun/es-shell/issues/87
	assert {$es -c {
		catch @ {} {
			local (var = !) {
				set-var = {throw something}
			}
		}
	}}

	# https://github.com/wryun/es-shell/issues/93
	assert {./testrun 0 | {catch @ {} {%read}}}

	# https://github.com/wryun/es-shell/issues/99
	assert {$es -c {catch @ {} {echo >[1=]}}}

	# https://github.com/wryun/es-shell/issues/104
	assert {$es -c 'eval {}'}

	# https://github.com/wryun/es-shell/issues/150
	assert {!$es >[2] /dev/null << EOF
%read << EOM
hello
EOM
wait
EOF
	}

	# https://github.com/wryun/es-shell/issues/180
	assert {!$es -ec 'if {false} {true} {false; true}'}

	# https://github.com/wryun/es-shell/issues/191
	# this one actually would hang before the fix on systems with
	# unsigned chars
	assert {!$es -c 'echo hi |[2' >[2] /dev/null}

	# https://github.com/wryun/es-shell/issues/199
	assert {~ `` \n {echo 'fn-%write-history = $&collect'^\n^'cat << eof' | $es -i >[2=1]} *'incomplete here document'*}

	# https://github.com/wryun/es-shell/issues/206
	assert {~ `` \n {$es -c 'let (a=<=true) echo $a'} <=true} 'concatenated assignment+call syntax works'

	# https://github.com/wryun/es-shell/issues/235
	assert {$es -c 'catch @ {} {%pathsearch %pnothingthatreallyexists}'} '%-like strings don''t break %pathsearch'

	# https://github.com/wryun/es-shell/issues/246
	let (x = \e^';'^\e^';'^\e^';')
	local (fn ok {true})
	assert {$es -c ok}

	# https://github.com/wryun/es-shell/pull/248
	local (fn %exec-failure {})
	assert {~ `{%run notarealbinary >[2=1]} 'notarealbinary'*}
}

# These tests are based on notes in the CHANGES file from the pre-git days.
test 'old regressions' {
	# TODO variable export/import deserves much more testing
	local ('x=y' = very good stuff)
	assert {~ `` \n {$es -c 'echo $''x=y'''} 'very good stuff'} \
		'''='' in exported variable names works'

	local (x = y) {
		assert {~ `` \n {env | grep '^x='} 'x=y'}
		local (x = '')
			assert {~ `` \n {env | grep '^x='} 'x='}
		local (x = ())
			assert {~ `` \n {env | grep '^x='} ()}
	}

	let (exception = ()) {
		catch @ e {exception = $e} {
			for (a = <={break; result b}) {
				assert {false} 'break in binder escapes for loop'
			}
		}
		assert {~ $exception(1) break} 'for does not catch break in binder'
	}

	let (a = b) {
		{a = c} >[1=2]
		assert {~ $a c}
		{a = d} >[1=]
		assert {~ $a d}
		{a = e} < /dev/null
		assert {~ $a e}
	}

	assert {!~ '-' [a-z]}
	assert {~ q [a-z]}
	assert {~ '-' [a'-'z]}
	assert {!~ q [a'-'z]}
	assert {~ '-' [-az]}
	assert {~ '-' [az-]}
}
