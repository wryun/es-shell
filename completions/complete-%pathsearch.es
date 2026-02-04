fn %complete-%pathsearch _ word {
	fn %completion-to-file f {
		catch @ e {result $f} {
			# Like %pathsearch, but don't filter file types.
			access -n $f -1e $path
		}
	}
	let (files = ()) {
		for (p = $path)
		for (w = $p/$word^*)
		if {access -x -- $w} {
			files = $files <={~~ $w $p/*}
		}
		result $files
	}
}
