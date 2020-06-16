#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "edje_suite.h"
#include "../efl_check.h"
#include <Ecore_Evas.h>
#include <Efreet.h>
#include <Ecore.h>

static const Efl_Test_Case etc[] = {
  { "Edje", edje_test_edje },
  { "Container", edje_test_container },
  { "Features", edje_test_features },
  { "Signal", edje_test_signal },
  { "Swallow", edje_test_swallow },
  { "Text", edje_test_text },
  { "Edje Text", edje_test_text },
  { NULL, NULL }
};

const char *
test_layout_get(const char *name)
{
   static char filename[PATH_MAX];

   snprintf(filename, PATH_MAX, TESTS_BUILD_DIR"/data/%s", name);

   return filename;
}

static Evas *evas = NULL;

Evas *
_setup_evas(void)
{
   Evas_Engine_Info *einfo;

   evas = evas_new();

   evas_output_method_set(evas, evas_render_method_lookup("buffer"));
   einfo = evas_engine_info_get(evas);
   fail_unless(evas_engine_info_set(evas, einfo));

   evas_output_size_set(evas, 500, 500);
   evas_output_viewport_set(evas, 0, 0, 500, 500);

   return evas;
}

SUITE_INIT(edje)
{
   ck_assert_int_eq(ecore_evas_init(), 1);
   ck_assert_int_eq(edje_init(), 1);
}

SUITE_SHUTDOWN(edje)
{
   if (evas) evas_free(evas);
   evas = NULL;
   ck_assert_int_eq(edje_shutdown(), 0);
   ck_assert_int_eq(ecore_evas_shutdown(), 0);
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
   ecore_app_no_system_modules();
   efreet_cache_disable();

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Edje", etc, SUITE_INIT_FN(edje), SUITE_SHUTDOWN_FN(edje));

   return (failed_count == 0) ? 0 : 255;
}
