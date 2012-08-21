# vi:set shiftwidth=2 textwidth=75:
# adventure.es
# Adventure shell.
# Copyright (c) Darren Bane 1997
# All rights reserved
#
# Last edit:	21/04/1986		D A Gwyn
# SCCS ID:	@(#)adventure.es	1.5
# $Id: adventure.es <dbane@shannon.tellabs.com> 05/09/1997 $
#
# Change log:
#
#   Initials  Name         email
#   --------------------------------------------------
#   db        Darren Bane  <dbane@shannon.tellabs.com>
#
# Initials  Date        Description
# -----------------------------------
# db        05/09/1997  Ported to es.
#
# usage: adventure.es
#
# CLI in the style of an early-80s text adventure game.

OPATH = $PATH

fn ask {
  echo -n $^*^'[y/n] '
  ans = <={%read}

  if {~ $ans y* Y*} {
    return 0
  } {
    return 1
  }
}
  
if {~ $#PAGER 0} {
  CAT = more
} {
  CAT = $PAGER
}

fn instructions {
  cat << EOF

                  Instructions for the Adventure shell

  Welcome to the Adventure shell!  In this exploration of the UNIX file
  system, I will act as your eyes and hands.  As you move around, I will
  describe whatever is visible and will carry out your commands.  The
  general form of a command is
          Verb Object Extra_stuff.
  Most commands pay no attention to the "Extra_stuff", and many do not
  need an "Object".  A typical command is
          get all
  which picks up all files in the current "room" (directory).  You can
  find out what you are carrying by typing the command
          inventory
  The command "help" results in a full description of all commands that I
  understand.  To quit the Adventure shell, type
          quit

  There are UNIX monsters lurking in the background.  These are also
  known as "commands with arguments".

  Good luck!
EOF
}

fn help {
  echo I understand the following commands (synonyms in parentheses):
  echo

  echo change OBJECT to NEW_NAME       changes the name of the object
  echo clone OBJECT as NEW_NAME        duplicates the object
  echo drop OBJECTS                    leaves the objects in the room
  echo enter (go) PASSAGE              takes the labeled passage
  echo examine OBJECTS                 describes the objects in detail
  echo feed OBJECT to MONSTER          stuffs the object into a UNIX monster
  echo get (take) OBJECTS              picks up the specified objects
  echo gripe (bug)                     report a problem with the Adventure shell
  echo help                            prints this summary
  echo inventory (i)                   tells what you are carrying
  echo kill (destroy) OBJECTS          destroys the objects
  echo look (l)                        describes the room, including hidden objects
  echo open (read) OBJECT              shows the contents of an object
  echo quit (exit)                     leaves the Adventure shell
  echo resurrect OBJECTS               attempts to restore dead objects
  echo steal OBJECT from MONSTER       obtains the object from a UNIX monster
  echo throw OBJECT at daemon          feeds the object to the printer daemon
  echo up                              takes the overhead passage
  echo wake MONSTER                    awakens a UNIX monster
  echo where (w)                       tells you where you are
  echo xyzzy                           moves you to your home
}
  
MAINT = dbane@shannon.tellabs.com

PATH = /usr/ucb:/bin:/usr/bin:/usr/local/bin:.

noexport = $noexport OPATH ans MAINT LIM KNAP kn wiz cha prev room exs obs \
    hexs hobs f i verb obj x fn-ask fn-instructions fn-help fn-print fn-delete

local (signals = $signals sigint sigquit) {
  catch @ e {
    echo Ouch!
    throw $e
  } {
    fn print { echo $* | tr ' ' '\012' | pr -5 -t -w75 -l`{expr \( $#* + 4 \) / 5} }
    fn delete elem list {
      let (result = ) {
        for (i = $list) {
          if {! ~ $elem $i} {
            result = $result $i
          }
        }
        result $result
      }
    }

    cd
    LIM = .limbo			# $HOME/$LIM contains "destroyed" objects
    mkdir $LIM >/dev/null >[2=1]
    KNAP = .knapsack			# $HOME/$KNAP contains objects being "carried"
    if {! access -d $KNAP} {
      if {mkdir $KNAP >/dev/null >[2=1]} {
        echo 'You found a discarded empty knapsack.'
      } {
        echo 'You have no knapsack to carry things in.'
	exit 1
      }
    } {
      echo 'One moment while I peek in your old knapsack...'
    }

    kn = `{ls -a $KNAP|sed -e '/^\.$/d' -e '/^\.\.$/d'}

    if {ask 'Welcome to the Adventure shell!  Do you need instructions?'} {
      instructions
      echo -n 'Type a newline to continue: '
      %read
    }

    wiz = false
    cha = false
    prev = $LIM
    forever {
      room=`{pwd}
      if {! ~ $room $prev} {
        if {~ $room $HOME} {
	  echo 'You are in your own home.'
	} {
          echo You have entered $room.
	}
	exs =
	obs =
	hexs =
	hobs =
	f = false
	for (i = `{ls -a}) {
	  if {~ $i . ..} {
          } {~ $i .*} {
            if {access -f $i} {
	      hobs = $hobs $i
	    } {access -d $i} {
	      hexs = $hexs $i
	    } {
              f=true
	    }
	  } {
	    if {access -f $i} {
	      obs = $obs $i
	    } {access -d $i} {
	      exs = $exs $i
	    } {
              f=true
	    }
	  }
	}
	if {! ~ $#obs 0} {
	  echo 'This room contains:'
	  print $obs
	} {
          echo 'The room looks empty.'
	}
	if {! ~ $#exs 0} {
	  echo 'There are exits labeled:'
	  print $exs
	  echo 'as well as a passage overhead.'
	} {
          echo 'There is a passage overhead.'
	}
	if {~ $f true} {
	  echo 'There are shadowy figures in the corner.'
	}
	prev = $room
      }

      echo -n '-advsh> '			# prompt
      local (line = ) {
        line = <={%read}
        if {~ $#line 0} {
          verb = quit	# EOF
        } {
          (verb obj x) = `{echo $line}
        }
      }

      if {~ $verb change} {
      	if {! ~ $#obj 0} {
	  if {~ $obj $obs $hobs} {
            if {~ $x(1) to} {
              local (* = $x) {
		if {! ~ $#2 0} {
		  if {access -f $2} {
		    echo You must destroy $2 first.
		  }
		  if {mv $obj $2 >/dev/null >[2=1]} {
		    echo The $obj shimmers and turns into $2.
		    obs = <={delete $obj $2 $obs}
		  } {
		    echo There is a cloud of smoke but the $obj is unchanged.
		  }
		} {
		  echo 'To what?'
		}
              }
	    } {
	      echo Change $obj to what?
	    }
	  } {
            if {~ $obj $kn} {
              echo 'You must drop it first.'
	    } {
              echo I see no $obj here.
            }
	  }
        } {
          echo 'Change what?'
        }
      } {~ $verb clone} {
        if {! ~ $#obj 0} {
          if {~ $obj $obs $hobs} {
	    if {! access -r $obj} {
              echo The $obj does not wish to be cloned.
            } {
              if {~ $x(1) as} {
                local (* = $x) {
		  if {! ~ $#2 0} {
		    if {access -f $2} {
		      echo You must destroy $2 first.
		    } {
		      if {cp $obj $2 >/dev/null >[2=1]} {
			echo Poof!  When the smoke clears, you see the new $2.
			obs = $obs $2
		      } {
			echo 'You hear a dull thud but no clone appears.'
		      }
		    }
		  } {
		    echo 'As what?'
		  }
                }
	      } {
                echo Clone $obj as what?
	      }
	    }
          } {
            if {~ $obj $kn} {
              echo 'You must drop it first.'
            } {
	      echo I see no $obj here.
            }
          }
        } {
          echo 'Clone what?'
        }
      } {~ $verb drop} {
        if {! ~ $#obj 0} {
	  for (it = $obj $x) {
            if {~ $it $kn} {
              if {access -w $it} {
	        echo You must destroy $it first.
              } {
                if {mv $HOME/$KNAP/$it $it >/dev/null >[2=1]} {
		  echo $it: dropped.
		  kn = <={delete $it $kn}
		  obs = $it $obs
                } {
	          echo The $it is caught in your knapsack.
		}
	      }
	    } {
              echo You\'re not carrying the $it!
	    }
	  }
	} {
          echo 'Drop what?'
	}
      } {~ $verb enter go} {
        if {! ~ $#obj 0} {
	  if {! ~ $obj up} {
	    if {~ $obj $exs $hexs} {
              if {access -x $obj} {
                if {cd $obj} {
		  echo 'You squeeze through the passage.'
                } {
		  echo You can\'t go that direction.
	        }
	      } {
                echo 'An invisible force blocks your way.'
	      }
	    } {
              echo 'I see no such passage.'
	    }
	  } {
            if {cd ..} {
              echo 'You struggle upwards.'
            } {
              echo You can\'t reach that high.
	    }
	  }
        } {
          echo 'Which passage?'
        }
      } {~ $verb examine} {
        if {! ~ $#obj 0} {
	  if {~ $obj all} {
	    $obj = $obs $exs
            x =
	  }
	  for (it = $obj $x) {
	    if {~ $it $obs $hobs $exs $hexs} {
              echo Upon close inspection of the $it, you see:
	      if {! ls -ld $it >[2]/dev/null} {
	        echo -- when you look directly at the $it, it vanishes.
	      }
	    } {
              if {~ $it $kn} {
	        echo 'You must drop it first.'
	      } {
                echo I see no $it here.
	      }
	    }
	  }
	} {
          echo 'Examine what?'
        }
      } {~ $verb feed} {
        if {! ~ $#obj 0} {
	  if {~ $obj $obs $hobs} {
	    if {~ $x(1) to} {
              local (* = $x) {
		if {! ~ $#2 0} {
		  * = $*(2 ...)
		  local (PATH = $OPATH) {
		    if {$* <$obj >[2]/dev/null} {
		      echo The $1 monster devours your $obj.
		      if {/bin/rm -f $obj >/dev/null >[2=1]} {
			obs = <={delete $obj $obs}
		      } {
			echo 'But he spits it back up.'
		      }
		    } {
		      echo The $1 monster holds his nose in disdain.
		    }
                  }
	        } {
	          echo 'To what?'
	        }
              }
	    } {
              echo Feed $obj to what?
	    }
	  } {
            if {~ $obj $kn} {
              echo 'You must drop it first.'
	    } {
              echo I see no $obj here.
            }
	  }
        } {
          echo 'Feed what?'
        }
      } {~ $verb get take} {
        if {! ~ $#obj 0} {
          if {~ $obj all} {
            obj = $obs
            x =
          }
          for (it = $obj $x) {
            if {~ $it $obs $hobs} {
              if {~ $it $kn} {
		echo 'You already have one.'
              } {
                if {mv $it $HOME/$KNAP/$it >/dev/null >[2=1]} {
                  echo $it: taken.
		  kn = $it $kn
		  obs = <={delete $it $obs}
                } {
		  echo The $it is too heavy.
		}
	      }
            } {
              echo I see no $it here.
	    }
	  }
	} {
          echo 'Get what?'
        }
      } {~ $verb gripe bug} {
        echo Please describe the problem and your situation at the time it failed.\nEnd the bug report with a line containing just a Ctrl-D.
	mailx -s 'adventure.es bug' $MAINT
	echo 'Thank you!'
      } {~ $verb help} {
      	help
      } {~ $verb inventory i} {
        if {! ~ $#kn 0} {
	  echo 'Your knapsack contains:'
	  print $kn
	} {
          echo 'You are poverty-stricken.'
	}
      } {~ $verb kill destroy} {
        if {! ~ $#obj 0} {
	  if {~ $obj all} {
	    x=
	    if {ask Do you really want to attempt to $verb them all?} {
	      obj = $obs
	      echo 'Chicken!'
	      obj=
	    }
	  }
          for (it = $obj $x) {
	    if {~ $it $obs $hobs} {
	      if {mv $it $HOME/$LIM </dev/null >[1=0] >[2=0]} {
		if {~ $verb kill} {
		  echo The $it cannot defend himself; he dies.
                } {
		  echo You have destroyed the $it; it vanishes.
		}
		obs = <={delete $it $obs}
              } {~ $verb kill} {
                echo Your feeble blows are no match for the $it.
              } {
		echo The $it is indestructible.
	      }
	    } {~ $it $kn} {
	      echo You must drop the $it first.
	      found=false
	    } {
              echo I see no $it here.
	    }
          } {
            echo 'Kill what?'
          }
        }
      } {~ $verb look l} {
        obs = $obs $hobs
        hobs=
	if {! ~ $#obs 0} {
	  echo 'The room contains:'
	  print $obs
	} {
          echo 'The room is empty.'
	}
	exs = $exs $hexs
        hexs=
	if {! ~ $#exs 0} {
	  echo 'There are exits plainly labeled:'
	  print $exs
	  echo 'and a passage directly overhead.'
	} {
          echo 'The only exit is directly overhead.'
        }
      } {~ $verb magic} {
        if {~ $obj mode} {
	  if {~ $cha true} {
	    echo 'You had your chance and you blew it.'
	  } {
            if {ask 'Are you a wizard?'} {
	      echo -n 'Prove it!  Say the magic word: '
	      obj = <={%read}
              if {~ $obj armadillo} {
	        echo 'Yes, master!!'
	        wiz=true
	      } {
        	echo Homie says: I don\'t think so
	        cha=true
	      }
	    } {
              echo I didn\'t think so.
	    }
	  }
	} {
          echo 'Nice try.'
	}
      } {~ $verb open read} {
        if {! ~ $#obj 0} {
          if {~ $obj $obs $hobs} {
            if {access -r $obj} {
              if {test -s $obj} {
	        echo Opening the $obj reveals:
		if {! $CAT < $obj} {
		  echo '-- oops, you lost the contents!'
		}
	      } {
                echo There is nothing inside the $obj.
	      }
	    } {
              echo You do not have the proper tools to open the $obj.
	    }
	  } {
            if {~ $obj $kn} {
	      echo 'You must drop it first.'
	      found=false
	    } {
              echo I see no $obj here.
	    }
	  }
	} {
          echo 'Open what?'
        }
      } {~ $verb quit exit} {
        if {ask 'Do you really want to quit now?'} {
	  if {! ~ $#kn 0} {
	    echo 'The contents of your knapsack will still be there next time.'
	  }
	  /bin/rm -rf $HOME/$LIM
	  echo 'See you later!'
	  exit 0
	}
      } {~ $verb resurrect} {
        if {! ~ $#obj 0} {
	  for (it = $obj $x) {
	    if {~ $it $obs $hobs} {
	      echo The $it is already alive and well.
	    } {
              if {mv $HOME/$LIM/$it $it </dev/null >[1=0] >[2=0]} {
	        echo The $it staggers to his feet.
		obs = $it $obs
		echo There are sparks but no $it appears.
	      }
	    }
	  }
	} {
          echo 'Resurrect what?'
        }
      } {~ $verb steal} {
        if {! ~ $#obj 0} {
	  if {~ $obj $obs $hobs} {
	    echo 'There is already one here.'
	  } {
            if {~ $x(1) from} {
              local (* = $x) {
		if {! ~ $#2 0} {
		  * = $*(2 ...)
		  local (PATH = $OPATH) {
		    if {$* >$obj >[2]/dev/null} {
		      echo The $1 monster drops the $obj.
		      obs = $obj $obs
		    } {
		      echo The $1 monster runs away as you approach.
		      /bin/rm -f $obj >/dev/null >[2=1]
		    }
		  } {
		    echo 'From what?'
		  }
		} {
		  echo Steal $obj from what?
		}
              }
	    }
	  }
	} {
          echo 'Steal what?'
        }
      } {~ $verb throw} {
        if {! ~ $#obj 0} {
          if {~ $obj $obs $hobs} {
            if {~ $x(1) at} {
              if {~ $x(2) daemon}  {
                if {lpr -r $obj} {
	          echo The daemon catches the $obj, turns it into paper,\nand leaves it in the basket.
		  obs = <={delete $obj $obs}
		} {
                  echo The daemon is nowhere to be found.
		}
	      } {
		echo 'At what?'
	      }
	    } {
	      echo Throw $obj at what?
	    }
          } {
            if {~ $obj $kn} {
	      echo 'It is in your knapsack.'
              found=false
            } {
              echo I see no $obj here.
            }
          }
        } {
          echo 'Throw what?'
        }
      } {~ $verb u up} {
        if {cd ..} {
	  echo 'You pull yourself up a level.'
	} {
          echo You can\'t reach that high.
        }
      } {~ $verb wake} {
        if {! ~ $#obj 0} {
	  echo You awaken the $obj monster:
	  local (PATH = $OPATH) {
            $obj $x
          }
	  echo 'The monster slithers back into the darkness.'
	} {
          echo 'Wake what?'
        }
      } {~ $verb w where} {
        echo You are in $room.
      } {~ $verb xyzzy} {
        if {cd} {
	  echo 'A strange feeling comes over you.'
        } {
	  echo 'Your spell fizzles out.'
	}
      } {
        if {! ~ $#verb 0} {
	  if {~ $wiz true} {
	    local (PATH = $OPATH) {
              $verb $obj $x
	    }
          } {
            echo I don\'t know how to "$verb".
	    echo 'Type "help" for assistance.'
	  }
	} {
          echo 'Say something!'
	}
      }
    }
  }
}
