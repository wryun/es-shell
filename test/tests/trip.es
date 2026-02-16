# tests/trip.es -- migration of the classic trip.es to the new test framework.

test 'lexical analysis' {
	let (tmp = `{mktemp trip-nul.XXXXXX})
	unwind-protect {
		./testrun 0 > $tmp
		let ((status output) = <={$&backquote \n {$es $tmp >[2=1]}}) {
			assert {~ $output *'null character ignored'*} 'null character produces warning'
			assert {~ $status 6} 'null character does not disturb behavior'
		}
	} {
		rm -f $tmp
	}

	let (wtmp = `{mktemp long-word.XXXXXX}; qtmp = `{mktemp long-qword.XXXXXX})
	unwind-protect {
		echo here_is_a_really_long_word.It_has_got_to_be_longer_than_2048_characters_for_the_lexical_analyzers_buffer_to_overflow_but_that_should_not_be_too_difficult_to_do.Let_me_start_writing_some_Lewis_Carroll.The_sun_was_shining_on_the_sea,Shining_with_all_his_might:He_did_his_very_best_to_make_The_billows_smooth_and_bright-And_this_was_odd,because_it_was_The_middle_of_the_night.The_moon_was_shining_sulkily,Because_she_thought_the_sunHad_got_no_business_to_be_there_After_the_day_was_done-Its_very_rude_of_him,_she_said,To_come_and_spoil_the_fun.The_sea_was_wet_as_wet_could_be,The_sands_were_dry_as_dry.You_could_not_see_a_cloud,because_No_cloud_was_in_the_sky:No_birds_were_flying_overhead-There_were_no_birds_to_fly.The_Walrus_and_the_Carpenter_Were_walking_close_at_hand,They_wept_like_anything_to_see_Such_quantities_of_sand:If_this_were_only_cleared_away,_They_said,it_would_be_grand,If_seven_maids_with_seven_mops_Swept_it_for_half_a_year,Do_you_suppose,_the_Walrus_said,That_they_could_get_it_clear?I_doubt_it,_said_the_Carpenter,And_shed_a_bitter_tear.O_Oysters,come_and_walk_with_us,_The_Walrus_did_beseech.A_pleasant_walk,a_pleasant_talk,Along_the_briny_beach:We_cannot_do_with_more_than_four,To_give_a_hand_to_each.The_eldest_Oyster_looked_at_him,But_never_a_word_he_said:The_eldest_Oyster_winked_his_eye,And_shook_his_heavy_head-Meaning_to_say_he_did_not_choose_To_leave_the_oyster-bed.But_four_young_Oysters_hurried_up,All_eager_for_the_treat:Their_coats_were_brushed,their_faces_washed,Their_shoes_were_clean_and_neat-And_this_was_odd,because,you_know,They_hadnt_any_feet.Four_other_Oysters_followed_them,And_yet_another_four,And_thick_and_fast_they_came_at_last,And_more,and_more,and_more-All_hopping_through_the_frothy_waves,And_scrambling_to_the_shore.The_Walrus_and_the_Carpenter_Walked_on_a_mile_or_so,And_then_they_rested_on_a_rock_Conveniently_low:And_all_the_little_Oysters_stood_And_waited_in_a_row.The_time_has_come,_the_Walrus_said,To_talk_of_many_things:Of_shoes-and_ships-and_sealing-wax-Of_cabbages-and_kings-And_why_the_sea_is_boiling_hot-And_whether_pigs_have_wings.But_wait_a_bit,_the_Oysters_cried,Before_we_have_our_chat,For_some_of_us_are_out_of_breath,And_all_of_us_are_fat,No_hurry,_said_the_Carpenter.They_thanked_him_much_for_that.A_loaf_of_bread,_the_Walrus_said,Is_what_we_chiefly_need:Pepper_and_vinegar_besides_Are_very_good_indeed-Now_if_youre_ready,Oysters_dear,We_can_begin_to_feed.But_not_on_us,_the_Oysters_cried,Turning_a_little_blue.After_such_kindness,that_would_be_A_dismal_thing_to_do,The_night_is_fine,_the_Walrus_said.Do_you_admire_the_view?It_was_so_kind_of_you_to_come,_And_you_are_very_nice,The_Carpenter_said_nothing_but_Cut_us_another_slice:I_wish_you_were_not_quite_so_deaf-Ive_had_to_ask_you_twice,It_seems_a_shame,_the_Walrus_said,To_play_them_such_a_trick,After_weve_brought_them_out_so_far,And_made_them_trot_so_quick,The_Carpenter_said_nothing_but_The_butters_spread_too_thick,I_weep_for_you,_the_Walrus_said:_I_deeply_sympathize.With_sobs_and_tears_he_sorted_out_Those_of_the_largest_size,Holding_his_pocket-handkerchief_Before_his_streaming_eyes.O_Oysters,_said_the_Carpenter,Youve_had_a_pleasant_run,Shall_we_be_trotting_home_again?_But_answer_came_there_none-And_this_was_scarcely_odd,because_Theyd_eaten_every_one. > $wtmp

		echo 'here_is_a_really_long_word.It_has_got_to_be_longer_than_2048_characters_for_the_lexical_analyzers_buffer_to_overflow_but_that_should_not_be_too_difficult_to_do.Let_me_start_writing_some_Lewis_Carroll.The_sun_was_shining_on_the_sea,Shining_with_all_his_might:He_did_his_very_best_to_make_The_billows_smooth_and_bright-And_this_was_odd,because_it_was_The_middle_of_the_night.The_moon_was_shining_sulkily,Because_she_thought_the_sunHad_got_no_business_to_be_there_After_the_day_was_done-Its_very_rude_of_him,_she_said,To_come_and_spoil_the_fun.The_sea_was_wet_as_wet_could_be,The_sands_were_dry_as_dry.You_could_not_see_a_cloud,because_No_cloud_was_in_the_sky:No_birds_were_flying_overhead-There_were_no_birds_to_fly.The_Walrus_and_the_Carpenter_Were_walking_close_at_hand,They_wept_like_anything_to_see_Such_quantities_of_sand:If_this_were_only_cleared_away,_They_said,it_would_be_grand,If_seven_maids_with_seven_mops_Swept_it_for_half_a_year,Do_you_suppose,_the_Walrus_said,That_they_could_get_it_clear?I_doubt_it,_said_the_Carpenter,And_shed_a_bitter_tear.O_Oysters,come_and_walk_with_us,_The_Walrus_did_beseech.A_pleasant_walk,a_pleasant_talk,Along_the_briny_beach:We_cannot_do_with_more_than_four,To_give_a_hand_to_each.The_eldest_Oyster_looked_at_him,But_never_a_word_he_said:The_eldest_Oyster_winked_his_eye,And_shook_his_heavy_head-Meaning_to_say_he_did_not_choose_To_leave_the_oyster-bed.But_four_young_Oysters_hurried_up,All_eager_for_the_treat:Their_coats_were_brushed,their_faces_washed,Their_shoes_were_clean_and_neat-And_this_was_odd,because,you_know,They_hadnt_any_feet.Four_other_Oysters_followed_them,And_yet_another_four,And_thick_and_fast_they_came_at_last,And_more,and_more,and_more-All_hopping_through_the_frothy_waves,And_scrambling_to_the_shore.The_Walrus_and_the_Carpenter_Walked_on_a_mile_or_so,And_then_they_rested_on_a_rock_Conveniently_low:And_all_the_little_Oysters_stood_And_waited_in_a_row.The_time_has_come,_the_Walrus_said,To_talk_of_many_things:Of_shoes-and_ships-and_sealing-wax-Of_cabbages-and_kings-And_why_the_sea_is_boiling_hot-And_whether_pigs_have_wings.But_wait_a_bit,_the_Oysters_cried,Before_we_have_our_chat,For_some_of_us_are_out_of_breath,And_all_of_us_are_fat,No_hurry,_said_the_Carpenter.They_thanked_him_much_for_that.A_loaf_of_bread,_the_Walrus_said,Is_what_we_chiefly_need:Pepper_and_vinegar_besides_Are_very_good_indeed-Now_if_youre_ready,Oysters_dear,We_can_begin_to_feed.But_not_on_us,_the_Oysters_cried,Turning_a_little_blue.After_such_kindness,that_would_be_A_dismal_thing_to_do,The_night_is_fine,_the_Walrus_said.Do_you_admire_the_view?It_was_so_kind_of_you_to_come,_And_you_are_very_nice,The_Carpenter_said_nothing_but_Cut_us_another_slice:I_wish_you_were_not_quite_so_deaf-Ive_had_to_ask_you_twice,It_seems_a_shame,_the_Walrus_said,To_play_them_such_a_trick,After_weve_brought_them_out_so_far,And_made_them_trot_so_quick,The_Carpenter_said_nothing_but_The_butters_spread_too_thick,I_weep_for_you,_the_Walrus_said:_I_deeply_sympathize.With_sobs_and_tears_he_sorted_out_Those_of_the_largest_size,Holding_his_pocket-handkerchief_Before_his_streaming_eyes.O_Oysters,_said_the_Carpenter,Youve_had_a_pleasant_run,Shall_we_be_trotting_home_again?_But_answer_came_there_none-And_this_was_scarcely_odd,because_Theyd_eaten_every_one.' > $qtmp

		assert {~ `` () {cat $wtmp} `` '' {cat $qtmp}} \
			long string and long word are identical
		let (x = `{wc -c $wtmp})
			assert {~ $x(1) 3281} long word is 3281 bytes
		let (x = `{wc -c $qtmp})
			assert {~ $x(1) 3281} long quote is 3281 bytes
	} {
		rm -f $wtmp $qtmp
	}

	local (ifs = \n) {
		assert {~ `{echo h\
i} 'h i'} backslash-newline to space conversion
		assert {~ `{echo $es\\es} $es^\\es} backslash after variable name terminates variable name scan
		assert {~ `{echo $es\
es} $es^' es'} backslash-newline after variable name space conversion
		assert {~ `{echo h\\i} 'h\i'} backslash in the middle of word
		assert {~ `{echo h \\ i} 'h \ i'} free-standing backslash
	}

	assert {$es -c '# eof in comment'} eof in comment exits with zero status
	assert {$es -c '{} $00'} 00 can be used as a variable
}

test 'tokenizer errors' {
	local (ifs = '') {
		assert {~ `{$es -c 'echo hi |[2'	>[2=1]} *'expected ''='' or '']'' after digit'*}
		assert {~ `{$es -c 'echo hi |[92='	>[2=1]} *'expected digit or '']'' after ''='''*}
		assert {~ `{$es -c 'echo hi |[a]'	>[2=1]} *'expected digit after ''['''*}
		assert {~ `{$es -c 'echo hi |[2-]'	>[2=1]} *'expected ''='' or '']'' after digit'*}
		assert {~ `{$es -c 'echo hi |[2=99a]'	>[2=1]} *'expected '']'' after digit'*}
		assert {~ `{$es -c 'echo hi |[2=a99]'	>[2=1]} *'expected digit or '']'' after ''='''*}
		assert {~ `{$es -c 'echo ''hi'		>[2=1]} *'eof in quoted string'*}
	}
}

test 'blow the input stack' {
	assert {~ hi `{
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval eval eval \
		eval eval eval eval eval eval eval eval eval eval eval {echo hi}
	}}
}

test 'umask' {
	let (tmp = `{mktemp umask-file.XXXXXX})
	unwind-protect {
		rm -f $tmp
		umask 0
		> $tmp
		let (x = `{ls -l $tmp})
			assert {~ $x(1) '-rw-rw-rw-'*} umask 0 produces correct result

		rm -f $tmp
		umask 027
		> $tmp
		let (y = `{ls -l $tmp})
			assert {~ $y(1) '-rw-r-----'*} umask 027 produces correct file

		assert {~ `umask 027 0027} fail umask reports correct value
	} {
		rm -f $tmp
	}

	let (exception = ()) {
		catch @ e {exception = $e} {umask bad}
		assert {~ $exception *'bad umask'*} '`umask bad` throws "bad umask" exception'
		exception = ()
		catch @ e {exception = $e} {umask -027}
		assert {~ $exception *'bad umask'*}'`umask -027` throws "bad umask" exception'
		exception = ()
		catch @ e {exception = $e} {umask 999999}
		assert {~ $exception *'bad umask'*} '`umask 999999` throws "bad umask" exception'
	}

	assert {~ `umask 027 0027} bad umask does not change umask value
}

test 'redirections' {
	echo foo > foo > bar
	let (x = `{wc -c foo})
		assert {~ $x(1) 0} double redirection creates correct empty file
	let (y = `{wc -c bar})
		assert {~ $y(1) 4} double redirection creates right sized non-empty file
	rm -f foo bar

	echo -n >1 >[2]2 >[1=2] foo
	assert {~ `` '' {cat 1} ()} dup created non-empty empty file: `` '' {cat 1}
	assert {~ `` '' {cat 2} foo} dup put wrong contents in file : `` '' {cat 2}
	rm -f 1 2

	assert {~ `` \n {$es -c 'cat >[0=]' >[2=1]} *'cat:'*'Bad file '*}
	assert {~ `` \n {$es -c 'cat >(1 2 3)' >[2=1]} *'too many'*}
	assert {~ `` \n {$es -c 'cat >()' >[2=1]} *'null'*}
}

test 'exceptions' {
	assert {~ `` '' {
		let (x = a b c d e f g)
		catch @ e {
			echo caught $e
			if {!~ $#x 0} {
				x = $x(2 ...)
				throw retry
			}
			echo never succeeded
		} {
			echo trying ...
			eval '@'
			echo succeeded -- something''''s wrong
		}
	} \
'trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
trying ...
caught error $&parse {@}:1: syntax error
never succeeded
'
	}
}

test 'signals in exception catchers' {
	local (signals = sigint) {
		let (
			was-blocked = false
			thrown = ()
		) {
			catch @ {
				thrown = $*
			} {
				catch @ e {
					kill -INT $pid
					was-blocked = true
				} {
					throw exception
				}
			}
			assert $was-blocked signal is blocked during catcher
			assert {~ $thrown(1) signal} signal exception during catcher is thrown
		}
		let (thrown = ()) {
			catch @ {
				thrown = $*
			} {
				catch @ e {kill -INT $pid} {throw exception}
			}
			assert {~ $thrown(1) signal} second signal is caught
		}
		let (thrown = ()) {
			catch @ {
				thrown = $*
			} {
				catch @ e {kill -INT $pid; throw exception} {throw exception}
			}
			assert {~ $thrown(1) signal} signal exception has precedence within catcher
		}
	}
}

test 'heredocs and herestrings' {
	let (bigfile = `{mktemp big-file.XXXXXX})
	unwind-protect {
		od $es | sed 5q > $bigfile
		let (
			abc = (this is a)
			x = ()
			result = 'this is a heredoc
this is an heredoc
'
		) {
			assert {~ `` '' {<<[5] EOF cat <[0=5]} $result} 'unquoted heredoc'
$abc heredoc$x
$abc^n $x^here$x^doc
EOF
		}
		assert {~ `` \n cat '	'} 'quoted heredoc' << ' '
	
 
		<<<[9] `` '' {cat $bigfile} \
		{
			assert {~ `` '' {cat <[0=9]} `` '' cat} 'large herestrings'
		} < $bigfile
	} {
		rm -f $bigfile
	}

	assert {~ `{cat<<eof
$$
eof
	} '$'} 'quoting ''$'' in heredoc'

	assert {~ `` \n {$es -c 'cat<<eof' >[2=1]} *'pending'*} 'incomplete heredoc 1'
	assert {~ `` \n {$es -c 'cat<<eof'\n >[2=1]} *'incomplete'*} 'incomplete heredoc 2'
	assert {~ `` \n {$es -c 'cat<<eof'\n\$ >[2=1]} *'incomplete'*} 'incomplete heredoc 3'

	assert {~ `` \n {$es -c 'cat<<()' >[2=1]} *'not a single literal word'*} 'bad heredoc marker 1'
	assert {~ `` \n {$es -c 'cat<<(eof eof)' >[2=1]} *'not a single literal word'*} 'bad heredoc marker 2'
	assert {~ `` \n {$es -c 'cat<<'''\n''''\n >[2=1]} *'contains a newline'*} 'bad heredoc marker 3'
}

test 'tilde matching' {
	assert {$es -c '~ 0 1 `{}`{}`{}`{}`{} 0'}
	assert {$es -c '! ~ `{} 1 `{}`{}`{}`{}`{} 0'}
	assert {$es -c '~~ 0 1 `{}`{}`{}`{}`{}`{}`{}`{}`{} 0'}
}

test 'flat command expansion' {
	let (x = `^{echo some random phrase that should be identified as a single string})
		assert {~ $#x 1}
	let (x = `^{echo simple test with concatenation}^' '^`^{echo another random phrase})
		assert {~ $#x 1}
	let (x = ``^ abc {echo -n abchello})
		assert {~ $x 'hello'}

	assert {~ `` \n {$es -c '``^{true}' >[2=1]} *'syntax error'*}
	assert {~ `` \n {$es -c '`^^{true}' >[2=1]} *'syntax error'*}
}

test 'equal sign in command arguments' {
	assert {$es -c 'echo foo=bar' > /dev/null} '''='' in argument does not cause error'
	assert {~ `^{echo foo=bar} 'foo=bar'} '''='' is automatically concatenated with adjacent strings'
	assert {$es -c 'echo foo = bar' > /dev/null} '''='' as standalone argument does not cause error'
	assert {~ `^{echo foo = bar} 'foo = bar'} '''='' is not automatically concatenated with non-adjacent strings'
	assert {~ `` \n {$es -c 'foo^= = 384; echo $foo'} *'= 384'*}
	assert {~ `` \n {$es -c 'echo =foo; echo $echo'} *'foo'*}
}

test 'exit with signal codes' {
	assert {~ <={$es -c 'signals = sigterm; kill -TERM $pid' >[2] /dev/null} sigterm} \
		'die with a signal code'
	assert {~ <={$es -c 'signals = sigchld; kill -CHLD $pid' >[2] /dev/null} 1} \
		'die normally with an ignored signal'
	assert {~ <={$es -c 'signals = -sigterm; throw signal sigterm' >[2] /dev/null} sigterm} \
		'die from a thrown signal even if we would ignore it externally'
}

test 'lexical $0' {
	local (0 = es) {
		assert {~ `{echo echo $0} es}
		assert {if {~ $0 es} {true} {false}}
		assert {let (fn if {$&if $*}) {if {~ $0 es} {true} {false}}}
		assert {let (fn-if = $&noreturn @ {$&if $*}) {
			if {~ $0 es} {true} {false}
		}}
		assert {~ <={true && result $0} es}

		let (fn x {result $0 <={$*}})
		let (result = <={x {result $0}})
		assert {~ $result(1) x && ~ $result(2) es}

		let (fn x {$*})
		let (fn-doit = x @ {result $0})
		assert {~ <=doit doit}
	}
}

test 'binary $0' {
	local (path = .)
		assert {~ `{testrun a} 'testrun'} '$0 from hacked path is ok'
	local (fn %pathsearch bin {result ./testrun a})
		assert {~ `testrun 'testrun'} '$0 from hacked pathsearch is ok'
	let (fn-testrun = ./testrun)
		assert {~ `{testrun a} 'testrun'} '$0 from function is ok'
}

test 'backslash' {
	assert {~ `` \n {echo h\
i} 'h i'}
	assert {~ `` \n {echo $es\es} $es^\es}
	assert {~ `` \n {echo $es\
es} $es^' es'}
	assert {~ `` \n {echo h\\i} 'h\i'}
	assert {~ `` \n {echo h \\ i} 'h \ i'}
}
