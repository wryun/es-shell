# test/fuzz.es - fuzz testing harness for es using afl-fuzz

# This is very experimental and is most likely to be useful right now as
# documentation in the form of a shell script, rather than something actually
# runnable.

# Prerequisites:
#  - Running on a computer that you're willing to get thrashed (like a VM in the
#    cloud).  Fuzzing a shell is kind of like giving your bus driver a seizure:
#    you better be prepared for some consequences.
#  - afl is installed.
#  - you have an es repo ready to run `make` and you are in the base directory
#    of that repo.

# TODO
#  - Make this a real script that can really be run
#  - Parallelization
#  - Argv fuzzing

# Step 1: Build the shell with afl instrumentation.
# Disable LOCAL_GETENV, as the overridden setenv() breaks when using afl right now.
# The other defines are to increase sensitivity to memory issues.

make clean
local (AFL_HARDEN = 1) make CC=/usr/bin/afl-clang-fast CFLAGS='-ggdb -DLOCAL_GETENV=0 -DGCPROTECT=1 -DREF_ASSERTIONS=1'

# Step 2: Run afl-fuzz.  This will take a long time.
# test/fuzz/dict.txt gives the fuzzer a shortcut to significant tokens.

local (AFL_NO_ARITH = 1)
afl-fuzz -i ./test/fuzz/tests -o ./test/fuzz/findings -x ./test/fuzz/dict.txt ./es
