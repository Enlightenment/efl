#include <unistd.h>

#if defined(__clang__)
# pragma clang diagnostic ignored "-Wunused-parameter"
#elif (__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4
# pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

/* The failsafe clock of the test harness.
 *
 * efl_check.h forks one of these alongside the test cases and treats its
 * exit as "this suite has run out of time": it stops handing out new test
 * cases, says FAILSAFE TIMEOUT REACHED, and names every test case still
 * running -- which is the only thing that says *where* a suite hung.
 *
 * That only works if this expires before meson gives up on the suite. It
 * did not: this slept for 240 seconds and master_timeout in
 * src/tests/meson.build was also 240, so meson's SIGTERM arrived first (or
 * at the same moment) every time, the suite died without a word, and a hang
 * looked like "TIMEOUT killed by signal 15" and nothing else.
 *
 * So meson hands its number down as EFL_TEST_MASTER_TIMEOUT and this stops
 * short of it. The margin has to cover the harness noticing, printing, and
 * getting out.
 */
#ifndef EFL_TEST_MASTER_TIMEOUT
# define EFL_TEST_MASTER_TIMEOUT 240
#endif

#define FAILSAFE_MARGIN 60

int
main(int arc, char *argv[])
{
   unsigned int seconds = EFL_TEST_MASTER_TIMEOUT;

   seconds = (seconds > FAILSAFE_MARGIN * 2) ?
     seconds - FAILSAFE_MARGIN : seconds / 2;

   sleep(seconds);
   return 0;
}
