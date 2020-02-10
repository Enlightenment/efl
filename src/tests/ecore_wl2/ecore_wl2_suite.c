#include "ecore_wl2_suite.h"

static const Efl_Test_Case etc[] =
{
#if HAVE_ECORE_WL2
   { "Ecore_Wl2", ecore_wl2_test_init },
   { "Display", ecore_wl2_test_display },
   { "Window", ecore_wl2_test_window },
   { "Input", ecore_wl2_test_input },
   { "Output", ecore_wl2_test_output },
#endif
   { NULL, NULL }
};

SUITE_INIT(ecore_wl2)
{
   ck_assert_int_eq(ecore_wl2_init(), 1);
}

SUITE_SHUTDOWN(ecore_wl2)
{
   ck_assert_int_eq(ecore_wl2_shutdown(), 0);
}

int
main(int argc, char **argv)
{
   int count;

   if (!_efl_test_option_disp(argc, argv, etc)) return 0;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   count =
     _efl_suite_build_and_run(argc - 1,
                              (const char **)argv + 1, "Ecore_Wl2", etc,
                              SUITE_INIT_FN(ecore_wl2),
                              SUITE_SHUTDOWN_FN(ecore_wl2));

   return (count == 0) ? 0 : 255;
}
