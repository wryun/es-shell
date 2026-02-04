fn %complete-cd _ word {
	%file-complete @ {access -d -- $*} $word
}

