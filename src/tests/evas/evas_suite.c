#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>
#include <Ecore_Evas.h>

#include "evas_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "Evas", evas_test_init },
  { "Evas Focus", evas_test_focus },
  { "Evas New", evas_test_new },
  { "Object", evas_test_object },
  { "Object Textblock", evas_test_textblock },
  { "Object Text", evas_test_text },
  { "Callbacks", evas_test_callbacks },
  { "Render Engines", evas_test_render_engines },
  { "Filters", evas_test_filters },
  { "Images", evas_test_image_object },
  { "Images", evas_test_image_object2 },
  { "Masking", evas_test_mask },
  { "Evas GL", evas_test_evasgl },
  { "Object Smart", evas_test_object_smart },
  { "Events", evas_test_events },
  { "Efl Canvas Animation", efl_test_canvas_animation },
  { "Map", evas_test_map },
  { NULL, NULL }
};

SUITE_INIT(evas)
{
   ck_assert_int_eq(evas_init(), 1);
   ck_assert_int_eq(ecore_evas_init(), 1);
}

SUITE_SHUTDOWN(evas)
{
   ck_assert_int_eq(ecore_evas_shutdown(), 0);
   ck_assert_int_eq(evas_shutdown(), 0);
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
                                           "Evas", etc, SUITE_INIT_FN(evas), SUITE_SHUTDOWN_FN(evas));

   return (failed_count == 0) ? 0 : 255;
}
