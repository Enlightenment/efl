#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore_Evas.h>

#include "ecore_suite.h"

#define WINDOW_HEIGHT 200
#define WINDOW_WIDTH 200

EFL_START_TEST(ecore_test_ecore_evas_associate)
{
   Ecore_Evas *ee;
   Evas *canvas;
   Evas_Object *bg;
   int ret;

   ret = ecore_evas_init();
   fail_if(ret == 0);

   ee = ecore_evas_buffer_new(WINDOW_WIDTH, WINDOW_HEIGHT);
   fail_if(ee == NULL);

   canvas = ecore_evas_get(ee);
   fail_if(ecore_evas_ecore_evas_get(canvas) != ee);

   bg = evas_object_rectangle_add(canvas);
   ret = ecore_evas_object_associate(ee, bg, ECORE_EVAS_OBJECT_ASSOCIATE_BASE);
   fail_if(ret == 0);

   ecore_evas_free(ee);

   ret = ecore_evas_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_evas_cocoa)
{
   int ret;
   Ecore_Evas *ee;
   Ecore_Cocoa_Window *win;

   ret = ecore_evas_init();
   fail_if(ret != 1);

   ret = ecore_evas_engine_type_supported_get(ECORE_EVAS_ENGINE_OPENGL_COCOA);
   if (ret == EINA_TRUE)
     {
        /* Engine supported. Shall not fail... */
        ee = ecore_evas_cocoa_new(NULL, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        fail_if(ee == NULL);
        win = ecore_evas_cocoa_window_get(ee);
        fail_if(win == NULL);
     }
   else
     {
        /* Engine not supported. Shall not succeed... */
        ee = ecore_evas_cocoa_new(NULL, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        fail_if(ee != NULL);
        win = ecore_evas_cocoa_window_get(ee);
        fail_if(win != NULL);
     }

   ecore_evas_free(ee);

   ret = ecore_evas_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

void ecore_test_ecore_evas(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_evas_associate);
   tcase_add_test(tc, ecore_test_ecore_evas_cocoa);
}
