#ifndef EFL_CHECK_H
#define EFL_CHECK_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* getenv */
#include <stdio.h> /* fprintf, fputs */
#include <string.h> /* strcmp */
#include <unistd.h> /* execvp */
#include <errno.h> /* errno */
#include <sys/time.h>

#ifdef HAVE_FORK
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#include <signal.h>
#include <Eina.h>
#endif

#ifndef EINA_UNUSED

#ifdef __GNUC__

# if __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
#  define EINA_UNUSED __attribute__ ((__unused__))
# else
#  define EINA_UNUSED
# endif
#else
#  define EINA_UNUSED
#endif

#endif

#ifdef HAVE_GETTIMEOFDAY
# if CHECK_MINOR_VERSION >= 11
#  define ENABLE_TIMING_INFO
# endif
#endif

#define DISABLE_ABORT_ON_CRITICAL_START \
   do { \
      int ___val = eina_log_abort_on_critical_get(); \
      eina_log_abort_on_critical_set(0)

#define DISABLE_ABORT_ON_CRITICAL_END \
      eina_log_abort_on_critical_set(___val); \
   } while (0)

#define EXPECT_ERROR_START \
  do { \
      DISABLE_ABORT_ON_CRITICAL_START; \
      Eina_Bool expect_error_start = EINA_FALSE; \
      do { \
        eina_log_print_cb_set(_efl_test_expect_error, &expect_error_start); \
      } while(0)


#define EXPECT_ERROR_END \
    ck_assert_int_eq(expect_error_start, EINA_TRUE); \
    DISABLE_ABORT_ON_CRITICAL_END; \
  } while(0)

typedef struct _Efl_Test_Case Efl_Test_Case;
struct _Efl_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static int timeout_pid = 0;

EINA_UNUSED static void
_efl_test_expect_error(const Eina_Log_Domain *d EINA_UNUSED, Eina_Log_Level level, const char *file EINA_UNUSED, const char *fnc, int line EINA_UNUSED, const char *fmt EINA_UNUSED, void *data, va_list args EINA_UNUSED)
{
   Eina_Bool *error = (Eina_Bool*) data;
   if (level == EINA_LOG_LEVEL_ERR) *error = EINA_TRUE;

   printf("EXPECTED ERROR %s\n", fnc);
}

static void
_efl_tests_list(const Efl_Test_Case *etc)
{
   const Efl_Test_Case *itr = etc;
      fputs("Available Test Cases:\n", stderr);
   for (; itr->test_case; itr++)
      fprintf(stderr, "\t%s\n", itr->test_case);
}

EINA_UNUSED static int
_efl_test_option_disp(int argc, char **argv, const Efl_Test_Case *etc)
{
   int i;

   for (i = 1; i < argc; i++)
     {
        if ((strcmp(argv[i], "-h") == 0) ||
            (strcmp(argv[i], "--help") == 0))
          {
             fprintf(stderr, "Usage: %s [options] [test_case1 .. [test_caseN]]\n",
                     argv[0]);
             fprintf(stderr, " -l\t--list\t\tList all tests case.\n");
             fprintf(stderr, " \t--valgrind\tRun the tests under valgrind.\n");
             fprintf(stderr, "\nNote that CK_RUN_CASE=test_case does also filter which tests are run\n");
             return 0;
          }
        else if ((strcmp(argv[i], "-l") == 0) ||
                 (strcmp(argv[i], "--list") == 0))
          {
             _efl_tests_list(etc);
             return 0;
          }
        else if (strcmp(argv[i], "--valgrind") == 0)
          {
             const char *nav[argc + 3];
             int j, k;

             nav[0] = "valgrind";
             nav[1] = "--trace-children=yes";
             for (j = 0, k = 2; j < argc; j++)
               {
                  if (i == j) continue ; // Remove --valgrind
                  nav[k++] = argv[j];
               }

             nav[k] = NULL;
             execvp("valgrind", (char**) nav);
             fprintf(stderr, "Failed to execute valgrind due to '%s'\n", strerror(errno));
             return 0;
          }
     }

   return 1;
}

static int
_efl_test_use(int argc, const char **argv, const char *test_case)
{
   if (argc < 1)
     return 1;

   for (; argc > 0; argc--, argv++)
     if (strcasecmp(test_case, *argv) == 0)
       return 1;
   return 0;
}

static int
_efl_test_fork_has(SRunner *sr)
{
   if (srunner_fork_status(sr) == CK_FORK)
     return 1;
   else if (srunner_fork_status(sr) == CK_NOFORK)
     return 0;
   else if (srunner_fork_status(sr) == CK_FORK_GETENV)
     {
        char *res;

        res = getenv("CK_FORK");
        if (res && (strcmp(res, "no") == 0))
          return 0;
        else
          return 1;
     }

   /* should never get there */
   return 0;
}

#ifdef ENABLE_TIMING_INFO
EINA_UNUSED static double _timing_start_time;

static int
_timing_enabled(void)
{
   const char *lc = getenv("TIMING_ENABLED");
   return !!lc;
}

static double
_timing_time_get(void)
{
   struct timeval timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

EINA_UNUSED static void
_timing_start(void)
{
   if (_timing_enabled())
     _timing_start_time = _timing_time_get();
}

EINA_UNUSED static void
_timing_end(void)
{
   double diff;
   static int thres_set = 0;
   static double thres = 0;

   if (!_timing_enabled()) return;
   if (!thres_set)
     {
        const char *env = getenv("TIME_DIFF_THRESHOLD");

        if (env)
          {
             thres = strtod(env, NULL);
             if (thres > 0)
               thres_set = 1;
          }
        if (!thres_set)
          thres = 0.0001;
        thres_set = 1;
     }
   diff = _timing_time_get() - _timing_start_time;

   if (diff > thres)
     printf("TIME %s: %.5g\n", tcase_name(), diff);
}

# define EFL_START_TEST(TEST_NAME) \
  START_TEST(TEST_NAME) \
  { \
  _timing_start();

# define EFL_END_TEST \
  _timing_end(); \
  } \
  END_TEST

#else
# define EFL_START_TEST(TEST_NAME) START_TEST(TEST_NAME)
# define EFL_END_TEST END_TEST

#endif

static int
_efl_suite_run_end(SRunner *sr, const char *name)
{
   int failed_count;

   if (name)
     {
        char *n = strdup(name);
        char buf[4096], *p;

        for (p = n; p[0]; p++)
          {
             switch (p[0])
               {
                case ' ':
                case '/':
                case '\\':
                case ';':
                  p[0] = '_';
                  break;
               }
          }
        snprintf(buf, sizeof(buf), TESTS_BUILD_DIR "/check-results-%s.xml", n);
        srunner_set_xml(sr, buf);
     }
   else
     srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);
   return failed_count;
}

#ifdef HAVE_FORK
static EINA_UNUSED Eina_Hash *fork_map;

EINA_UNUSED static int
_efl_suite_wait_on_fork(int *num_forks, Eina_Bool *timeout)
{
   int status = 0, ret, pid;
   pid = waitpid(0, &status, 0);
   if (WIFEXITED(status))
     ret = WEXITSTATUS(status);
   else
     ret = 1;
   if (pid == timeout_pid)
     *timeout = EINA_TRUE;
   else
     {
        eina_hash_del_by_key(fork_map, &pid);
        (*num_forks)--;
     }
   return ret;
}
#endif

EINA_UNUSED static int
_efl_suite_build_and_run(int argc, const char **argv, const char *suite_name, const Efl_Test_Case *etc, SFun init, SFun shutdown)
{
   Suite *s;
   SRunner *sr;
   TCase *tc;
   int i, failed_count = 0;
   int do_fork;
   int num_forks = 0;
   int can_fork = 0;
#ifdef HAVE_FORK
   Eina_Bool timeout_reached = EINA_FALSE;
#endif
#ifdef ENABLE_TIMING_INFO
   double tstart = 0.0, tcstart = 0.0;
   int timing = _timing_enabled();

   if (timing)
     tcstart = tstart = _timing_time_get();
#endif
   fflush(stdout);
   s = suite_create(suite_name);
   sr = srunner_create(s);
   do_fork = _efl_test_fork_has(sr);
   if (do_fork)
     can_fork = !!etc[1].test_case /* can't parallelize 1 test */;

   for (i = 0; etc[i].test_case; ++i)
     {
        int pid = 0;

        if (!_efl_test_use(argc, argv, etc[i].test_case))
           continue;
#ifdef HAVE_FORK
        if (do_fork && can_fork)
          {
             if (!timeout_pid)
               {
                  timeout_pid = fork();
                  if (!timeout_pid)
                    execl("/bin/sh", "/bin/sh", "-c", PACKAGE_BUILD_DIR "/src/tests/timeout", (char *)NULL);
               }
             if (num_forks == eina_cpu_count())
               failed_count += _efl_suite_wait_on_fork(&num_forks, &timeout_reached);
             if (timeout_reached) break;
             pid = fork();
             if (pid > 0)
               {
                  if (!fork_map) fork_map = eina_hash_int32_new(NULL);
                  eina_hash_add(fork_map, &pid, etc[i].test_case);
                  num_forks++;
#ifdef ENABLE_TIMING_INFO
                  if (timing)
                    tcstart = _timing_time_get();
#endif
                  continue;
               }
          }
#endif

        tc = tcase_create(etc[i].test_case);
        if (init || shutdown)
          tcase_add_checked_fixture(tc, init, shutdown);

        if (do_fork)
          tcase_set_timeout(tc, 0);

        etc[i].build(tc);
        suite_add_tcase(s, tc);
#ifdef HAVE_FORK
        if (do_fork && (!pid) && can_fork)
          {
             failed_count = _efl_suite_run_end(sr, etc[i].test_case);
             if (failed_count > 255)
               failed_count = 255;
#ifdef ENABLE_TIMING_INFO
             if (timing)
               printf("TC TIME %s: %.5g\n", etc[i].test_case, _timing_time_get() - tcstart);
#endif
             exit(failed_count);
          }
#endif
     }

#ifdef HAVE_FORK
   if (num_forks && (!timeout_reached))
     {
        do
          {
             failed_count += _efl_suite_wait_on_fork(&num_forks, &timeout_reached);
          } while (num_forks && (!timeout_reached));
        if (timeout_reached)
          {
             Eina_Iterator *it;
             const char *testname;
             it = eina_hash_iterator_data_new(fork_map);
             timeout_pid = 0;
             printf("FAILSAFE TIMEOUT REACHED!\n");
             fflush(stdout);
             EINA_ITERATOR_FOREACH(it, testname)
               printf("STILL RUNNING: %s\n", testname);
             fflush(stdout);
             eina_iterator_free(it);
             failed_count++;
          }
     }
   else
#endif
     failed_count = _efl_suite_run_end(sr, NULL);

#ifdef HAVE_FORK
   if (timeout_pid)
     {
        kill(timeout_pid, SIGKILL);
        timeout_pid = 0;
     }
   eina_hash_free(fork_map);
   fork_map = NULL;
#endif

#ifdef ENABLE_TIMING_INFO
   if (timing)
     {
        printf("SUITE TIME(%u) %s: %.5g\n", getpid(), suite_name, _timing_time_get() - tstart);
        fflush(stdout);
     }
#endif
   return failed_count;
}

#define SUITE_INIT(NAME) static void _##NAME##_suite_init(void)
#define SUITE_INIT_FN(NAME) _##NAME##_suite_init
#define SUITE_SHUTDOWN(NAME) static void _##NAME##_suite_shutdown(void)
#define SUITE_SHUTDOWN_FN(NAME) _##NAME##_suite_shutdown

#endif
