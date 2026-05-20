# vscode-integration.es -- commands for operating within VS Code.
#
# Absolutely no warranty on this.
#
# Relevant vscode docs at the time of writing: https://github.com/microsoft/vscode/blob/main/src/vs/platform/terminal/common/xterm/shellIntegrationAddon.ts

fn %osc code msg {
	result \e]^$code^';'^$^msg^\a
}

fn osc code msg {
	echo -n <={%osc $code $msg}
}

fn-vsc-escape = map @ str {
	result <={%flatten '' <={map @ c {
		match $c (
		'\' {result '\\'}
		\a  {result '\x07'}
		\t  {result '\x09'}
		\n  {result '\x0a'}
		\e  {result '\x1b'}
		';' {result '\x3b'}
		*   {result $c}
		)
	} <={%fsplit '' $str}}}
}

let (
	# readline-friendly %osc
	fn %osc code msg {
		result \1\e]^$code^';'^$^msg^\a\2
	}
)
let (
	start	= <={%osc 633 A}
	end	= <={%osc 633 B}
	pr = $fn-%prompt
)
fn %prompt {
	$pr $*
	# TODO: osc 633 F and G?
	osc 633 'P;ContinuationPrompt='^<={vsc-escape $prompt(2)}
	prompt = $start^$prompt^$end  # note cross-product concatenation
}

let (wh = $fn-%write-history)
fn %write-history cmd {
	# TODO: nonce support?
	osc 633 'E;'^<={vsc-escape $cmd}
	$wh $cmd
}

noexport = $noexport $VSCODE_NONCE

# tweak on the status.es canonical extension
let (loop = $fn-%interactive-loop)
fn %interactive-loop {
	let (
		d = $fn-%dispatch
		fn truthy value {
			if {result $value} {true} {false}
		}
	)
	local (
		noexport = $noexport status
		status = <=true
		fn-%dispatch = $&noreturn @ {
			catch @ e rest {
				osc 633 D';'^<={truthy $rest}
				throw $e $rest
			} {
				osc 633 C
				status = <={$d $*}
				osc 633 D';'^<={truthy $status}
				result $status
			}
		}
	) $loop $*
}

let (cd = $fn-cd)
fn cd {
	let (r = <={$cd $*}) {
		osc 633 'P;Cwd='^<={vsc-escape <={%flatten \n `` \n pwd}}
		result $r
	}
}

VSCODE_SHELL_INTEGRATION = 1
osc 633 'P;HasRichCommandDetection=True'
