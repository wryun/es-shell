# tests/match.es -- verify the match command works

# This is migrated from trip.es, but the match section is big enough that it
# deserves its own file.

test 'match/if equivalence' {
	for (subjstr = ( '()' 'foo' )) {
		subj = <={eval result $subjstr}
		for (
			exp = (
				<={}
				<={if {~ $subj}{result CMD}}
				<={if {~ $subj}{result CMD1}{result CMD2}}
				<={if {~ $subj *}{result CMD1}{result CMD2}}
			)
			rc = (
				<={match $subj ()}
				<={match $subj (() {result CMD})}
				<={match $subj (
					() {result CMD1}; * {result CMD2}
				)}
				<={match $subj ( * {result CMD1}
					* {result CMD2};)}
			)
		) {
			assert {~ $exp $rc} 'match1 '^$subjstr^' -- '^$rc^' matches '^$exp
		}
	}
}

test 'subjects' {
	let (
		subjects = (
			# ??zz -- wildcards can be used in patterns to match subjects
			'(fizz buzz)'
			# [1-9] -- cases are evaluated in order of appearance,
			#          and [1-9] comes before ??zz
			'(buzz 4 fizz 2 1)'
			# a* c* -- 'case patt1 patt2' matches like '~ $subj patt1 patt2'
			'(he ate it all)'
			'(bravo charlie)'
			# list.o -- wildcards are expanded in subjects
			'l*.o'
			# *.o -- wildcards are not expanded in patterns
			'nonexistent.o'
			# * -- catch-all for subjects that did not match any preceding patterns
			#
			# 'case *' should be last in every switch, but ensuring that would make
			# parsing more complicated and adding a 'default' keyword would just be
			# one more keyword to break existing scripts.
			'(20 fizzy ''think up different'' match.c)'
		)
		if-block = '
		if {~ $subj list.o} {
			result list
		} {~ $subj *.o} {
			result object
		} {~ $subj [1-9]} {
			result digit
		} {~ $subj ??zz} {
			result fizz/buzz
		} {~ $subj a* c*} {
			result AC_OUTPUT
		} {
			result other
		}'
		match-block = 'match $subj (
			list.o     {result list}
			*.o        {result object}
			[1-9]      {result digit}
			??zz       {result fizz/buzz}
			(a* c*)    {result AC_OUTPUT}
			*          {result other}
		)'
	)
	for (subjstr = $subjects) {
		let (
			subj = <={eval result $subjstr}
			exp = <={eval $if-block}
			rc = <={eval $match-block}
		)
			assert {~ $exp $rc} 'match2 '^$subjstr^' -- '^$rc^' matches '^$exp
	}
}

# The following ensures that the body of a case does not require
# braces and that 'match' has no special handling for 'break'.
test 'error handling' {
	let (stderr = `{mktemp match-stderr.XXXXXX})
	unwind-protect {
		$es -c 'match () (* break)' >[2] $stderr
		assert {~ `^{cat $stderr} *'uncaught exception'*}
	} {
		rm -f $stderr
	}
}
