#! /bin/sh

# mtests.sh : the moronic test suite suited for C projects

# TODO
#  - support test specific subdir test_xxx.d/*.[ch]
#  - integrate with autofoos (SRC_D BUILD_D CC CFLAGS ...)

SCRIPT_DIR=${0%/*}
SCRIPT_FILE=${0##*/}

RESET="\033[0m"
RED="\033[0;31m"
GREEN="\033[0;32m"
BROWN="\033[0;33m"
PURPLE="\033[0;35m"

# arguments
ABORT=0
QUIET=0
TESTS=""
SRC_D="src"
BUILD_D="build"

# env vars
CC="${CC:-"clang"}"
CFLAGS="${CFLAGS:-"-O0 -ggdb -W -Wall -Wextra -Wshadow"}"

# input files
MAIN_C="main.c"

# output files
TEST_D="/tmp/__mtests"

# local vars
LD=""
INCLUDE=""
TEST_N=0
PASS_N=0

function fatal
{
   echo -e "${RED}FATAL${RESET} $1"
   exit 1
}

function say
{
   [ $QUIET -eq 1 ] || echo -e "$1"
}

function sayn
{
   [ $QUIET -eq 1 ] || echo -en "$1"
}

function say_anyway
{
   if [ $QUIET -eq 1 ]
   then
      echo -e "$1"  | sed s'/^ \+//'
   else
      echo -e "$1"
   fi
}

while [ $# -ge 1 ]; do
   case "$1" in
      -s|--src)
         shift
         [ $# -lt 1 ] && fatal "option -s is missing directory argument"
         SRC_D=$1
         shift
         ;;
      -b|--build)
         shift
         [ $# -lt 1 ] && fatal "option -b is missing directory argument"
         BUILD_D=$1
         shift
         ;;
      -a|--abort)
         shift
         ABORT=1
         ;;
      -q|--quiet)
         shift
         QUIET=1
         ;;
      -h|--help)
         echo "Usage: $SCRIPT_FILE [options]"
         echo
         echo "Options:"
         echo "    -b, --abort            Abort on test failure"
         echo "    -q, --quiet            Only output failed tests"
         echo "    -s, --src directory    Directory to search for tests into"
         echo "    -b, --build directory  Directory to search for built files into"
         echo "    -h, --help             This message."
         exit 0
         ;;
      *)
         TESTS="$TESTS $1"
         shift
         ;;
   esac
done

[ -d "$SRC_D" -a -r "$SRC_D" ] || fatal "$SRC_D is not a valid directory"
[ -d "$BUILD_D" -a -r "$BUILD_D" ] || fatal "$BUILD_D is not a valid directory"
rm -fr $TEST_D 2 > /dev/null
mkdir -p $TEST_D || fatal "cannot create $TEST_D directory"

function load_main
{
   INCLUDE=""
   for include in $(cat "$dir/$MAIN_C" | sed -n 's/#include\ \+"\(.*\)"/\1/p')
   do
      F=$(find $SRC_D -name $include)
      if [ -z "$F" ]
      then
         F=$(find $BUILD_D -name $include)
         [ ! -z "$F" ] || fatal "can't find $include in $SRC_D or $BUILD_D"
      fi
      INCLUDE="$INCLUDE -I${F%/*}"
   done
   LDF=""
   LDP=""
   for ld in $(cat "$dir/$MAIN_C" | sed -n 's/.*ld\ *:\ *\(.*\)$/\1/p')
   do
      F=$(find $BUILD_D -name $ld)
      [ ! -z "$F" ] || fatal "can't find $ld in $BUILD_D"
      LDP="$LDP -L${F%/*}"
      lib=${F##*/lib}
      lib=${lib%.so*}
      LDF="$LDF -l$lib"
   done
}

function check_dir
{
   for file in $MAIN_C
   do
      f="$dir/$file"
      if [ ! -f "$f" -o ! -r "$f" ]
      then
         say "    $f missing"
         say  "  leave"
         return 1
      fi
   done
   return 0
}

function enter_dir
{
   say "  enter ${PURPLE}$dir${RESET}"
   check_dir || return 1
   load_main
   return 0
}

function run_test
{
   TEST=$(basename $test_c .c)
   TEST_C="$TEST.c"
   TEST_O="$TEST_D/$TEST.o"
   sayn "    ${BROWN}run ${PURPLE}${test_c##*/}${RESET} "
   $CC $dir/$MAIN_C -o $TEST_O -DTESTC=$TEST_C -DCALL=$TEST -DFUNC="void $TEST(void)" $CFLAGS $INCLUDE $LDP $LDF || fatal " compilation of $test_c failed"
   TEST_N=$((TEST_N + 1))
   $TEST_O && rm $TEST_O && say "${GREEN}PASS${RESET}" && PASS_N=$((PASS_N + 1)) && return
   say "${RED}FAIL${RESET}"
   say_anyway "        $test_c"
   [ $ABORT -ne 1 ] && return
   say "        see $TEST_O"
   exit 1
}

function report
{
   [ $QUIET -eq 1 ] && exit 0
   say "\n$PASS_N/$TEST_N tests passed"
   FAIL_N=$(ls -1 $TEST_D | wc -l)
   [ $FAIL_N -gt 0 ] && say "see $TEST_O"
   exit 0
}

for test_c in $TESTS
do
   if [ ! -f $test_c ]
   then
      say "$test_c does not exists"
      continue
   fi
   dir=${test_c%/*}
   enter_dir || continue
   run_test
   say "  leave"
done

[ ! -z "$TESTS" ] && report

say "search for tests into $SRC_D"
for dir in $(find $SRC_D -type d -name tests)
do
   enter_dir || continue
   for test_c in $(find $dir -name test_*.c | sort)
   do
      run_test
   done
   say "  leave"
done
say "done"

report
