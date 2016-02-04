#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>

#include "evas_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
  { "Evas", evas_test_init },
  { "Object", evas_test_object },
  { "Object Textblock", evas_test_textblock },
  { "Object Text", evas_test_text },
  { "Callbacks", evas_test_callbacks },
  { "Render Engines", evas_test_render_engines },
  { "Filters", evas_test_filters },
  { "Images", evas_test_image_object },
  { "Meshes", evas_test_mesh },
  { "Masking", evas_test_mask },
  { "Evas GL", evas_test_evasgl },
  { "Object Smart", evas_test_object_smart },
  { "Matrix", evas_test_matrix },
  { NULL, NULL }
};

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv("EFL_RUN_IN_TREE=1");

   evas_init();

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Evas", etc);

   evas_shutdown();

   return (failed_count == 0) ? 0 : 255;
}
