#	Very incomplete ls completion to see how --option= completion works.
#	Not great so far!
#	TODO: enable --opt[TAB] to complete to '--option=', not '--option= '.
#	TODO: some kind of fanciness to enable good short-option support?

fn %complete-ls _ word {
	if {~ $word -*} {
		result $word^<={~~ (
			--all
			--author
			--block-size=
			--color=
		) $word^*}
	} {
		%file-complete {} $word
	}
}
