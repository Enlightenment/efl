#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore_Evas.h>

#include "ecore_suite.h"

#define WINDOW_HEIGHT 200
#define WINDOW_WIDTH 200

START_TEST(ecore_test_ecore_evas_associate)
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
END_TEST

void ecore_test_ecore_evas(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_evas_associate);
}
