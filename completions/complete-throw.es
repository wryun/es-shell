fn %complete-throw prefix word {
	let (cmd = <={%split ' '\t $prefix}) {
		if {~ $#cmd 0} {
			return $word^<={~~ (break eof error retry return signal) $word^*}
		}
		match $cmd(1) (
		(break eof retry return)	{result ()}	# no good guesses :/
		error	{
			if {~ $#cmd 1} {
				%whatis-complete $word
			} {
				result ()
			}
		}
		signal {
			# The shell should be able to give us this list...
			result $word^<={~~ (
				sigabrt
				sigalrm
				sigbus
				sigchld
				sigcont
				sigfpe
				sighup
				sigill
				sigint
				sigkill
				sigpipe
				sigpoll
				sigprof
				sigquit
				sigsegv
				sigstop
				sigtstp
				sigsys
				sigterm
				sigtrap
				sigttin
				sigttou
				sigurg
				sigusr1
				sigusr2
				sigvtalrm
				sigxcpu
				sigxfsz
				sigwinch
			) $word^*}
		}
		*	{result ()}	# Not sure :/
		)
	}
}
