# tests/wait.es -- verify behaviors around backgrounding and wait are correct

test 'exit status' {
	let (pid = <={$&background {result 3}}) {
		let (status = <={wait $pid >[2] /dev/null})
			assert {~ $status 3}
	}

	let (pid = <={$&background {./testrun s}}) {
		kill -TERM $pid
		let (status = <={wait $pid >[2] /dev/null})
			assert {~ $status sigterm}
	}

	let (pid = <={$&background {./testrun s}}) {
		kill -QUIT $pid
		# TODO: clean up core file?
		let (status = <={wait $pid >[2] /dev/null})
			assert {~ $status sigquit+core}
	}
}

test 'wait is precise' {
	let (pid = <={$&background {result 99}}) {
		assert {~ <=%apids $pid}
		assert {~ <=%apids $pid} 'apids is stable'
		fork {}
		assert {~ <=%apids $pid} 'waiting is precise'
		assert {~ <={wait $pid} 99} 'exit status is available'
	}
}

test 'setpgid' {
	let (pid = <={$&background {./testrun s}}) {
		assert {ps -o pid | grep $pid > /dev/null} 'background process appears in ps'
		kill $pid
		wait $pid >[2] /dev/null
		assert {!{ps -o pid | grep $pid}}
	}
}
