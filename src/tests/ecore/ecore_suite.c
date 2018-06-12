#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_suite.h"
#include "../efl_check.h"
#include <Ecore.h>

static const Efl_Test_Case etc[] = {
  { "Ecore", ecore_test_ecore },
  { "idle", ecore_test_ecore_idle },
  { "poller", ecore_test_ecore_idle },
#if HAVE_ECORE_X
  { "Ecore_X", ecore_test_ecore_x },
#endif
  { "Ecore_Imf", ecore_test_ecore_imf },
#if HAVE_ECORE_AUDIO
  { "Ecore Audio", ecore_test_ecore_audio},
#endif
  { "Ecore_Timers", ecore_test_timer },
  { "Ecore_Evas", ecore_test_ecore_evas },
  { "Ecore_Animators", ecore_test_animator },
  { "Eina_Thread_Queue", ecore_test_ecore_thread_eina_thread_queue },
  { "Eina_Thread_Queue", ecore_test_ecore_thread_eina_thread_queue2 },
#if HAVE_ECORE_DRM
  { "Ecore_Drm", ecore_test_ecore_drm },
#endif
#if HAVE_ECORE_FB
  { "Ecore_Fb", ecore_test_ecore_fb },
#endif
  { "Ecore_Input", ecore_test_ecore_input },
  { "Ecore_File", ecore_test_ecore_file },
  { "Ecore_Job", ecore_test_ecore_job },
  { "Ecore_Args", ecore_test_ecore_args },
  { NULL, NULL }
};

static Ecore_Timer *timeout;
int timeout_reached = 0;

static Eina_Bool
timeout_cb()
{
#if CHECK_MINOR_VERSION >= 11
   const char *tcname = tcase_name();

   timeout_reached = 1;

   if (tcname && strstr(tcname, "download"))
     {
        fprintf(stderr, "test timeout reached: download failed, probably network issue. skipping\n");
        ecore_main_loop_quit();
     }
   else
#endif
     ck_abort_msg("test timeout reached!");
   timeout = NULL;
   return EINA_FALSE;
}

SUITE_INIT(ecore)
{
   timeout_reached = 0;
   ck_assert_int_eq(ecore_init(), 1);
   timeout = ecore_timer_add(5.0, timeout_cb, NULL);
   ck_assert_msg(!!timeout, "timeout timer creation failed!");
}

SUITE_SHUTDOWN(ecore)
{
   ecore_timer_del(timeout);
   timeout = NULL;
   ck_assert_int_eq(ecore_shutdown(), 0);
}

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Ecore", etc, SUITE_INIT_FN(ecore), SUITE_SHUTDOWN_FN(ecore));

   return (failed_count == 0) ? 0 : 255;
}
