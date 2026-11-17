#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Efl_Core.h>

#include "ecore_private.h"
#include "ecore_evas_private.h"

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

static Eina_Value
_verify_and_exit(void *data, const Eina_Value value, const Eina_Future *dead_future EINA_UNUSED)
{
   ck_assert_ptr_eq(eina_value_type_get(&value), EINA_VALUE_TYPE_CONTENT);
   Eina_Content *content = eina_value_to_content(&value);
   Eina_Content *reference = data;

   ck_assert_int_eq(eina_content_data_get(content).len, eina_content_data_get(reference).len);
   ck_assert_str_eq(eina_content_data_get(content).mem, eina_content_data_get(reference).mem);
   ck_assert_str_eq(eina_content_type_get(content), eina_content_type_get(reference));

   efl_loop_quit(efl_main_loop_get(), eina_value_int_init(0));

   return EINA_VALUE_EMPTY;
}

EFL_START_TEST(ecore_test_ecore_evas_fallback_selection)
{
   Ecore_Evas *ee;
   ecore_evas_init();

   ee = ecore_evas_buffer_new(WINDOW_WIDTH, WINDOW_HEIGHT);
   fail_if(ee == NULL);

   for (int i = 0; i < ECORE_EVAS_SELECTION_BUFFER_LAST; ++i)
     {
        ck_assert_int_eq(ecore_evas_selection_exists(ee, 0, i), EINA_FALSE);
     }
   Eina_Content *content = eina_content_new((Eina_Slice)EINA_SLICE_STR_FULL("asdf"), "text/plain");
   Eina_Content *ref = eina_content_new((Eina_Slice)EINA_SLICE_STR_FULL("asdf"), "text/plain");
   ecore_evas_selection_set(ee, 0, ECORE_EVAS_SELECTION_BUFFER_SELECTION_BUFFER, content);

   const char *types[] = {eina_stringshare_add("text/plain")};

   Eina_Future *f = ecore_evas_selection_get(ee, 0, ECORE_EVAS_SELECTION_BUFFER_SELECTION_BUFFER, EINA_C_ARRAY_ITERATOR_NEW(types));
   ck_assert_ptr_ne(f, NULL);
   eina_future_then(f, _verify_and_exit, ref);
   efl_task_run(efl_main_loop_get());
   ecore_evas_shutdown();
}
EFL_END_TEST

// Resizing a buffer canvas while an asynchronous render is in flight must not
// free the output buffer the draw thread is still writing to.
EFL_START_TEST(ecore_test_ecore_evas_buffer_resize_async)
{
   Ecore_Evas *ee;
   Evas *canvas;
   int i, j;

   fail_if(ecore_evas_init() == 0);

   ee = ecore_evas_buffer_new(400, 400);
   fail_if(ee == NULL);
   ecore_evas_show(ee);
   canvas = ecore_evas_get(ee);

   for (j = 0; j < 16; j++)
     {
        Evas_Object *r = evas_object_rectangle_add(canvas);

        evas_object_color_set(r, 20 * j, 255 - 10 * j, 128, 255);
        evas_object_move(r, j * 4, j * 4);
        evas_object_resize(r, 400, 400);
        evas_object_show(r);
     }

   for (i = 0; i < 20; i++)
     {
        int s = 400 + ((i & 1) ? 37 : 0);

        // Queue an asynchronous render, then resize before it completes.
        ecore_evas_render(ee);
        ecore_evas_resize(ee, s, s);
        ecore_evas_render_wait(ee);
        ecore_main_loop_iterate();
     }

   ecore_evas_free(ee);
   fail_if(ecore_evas_shutdown() != 0);
}
EFL_END_TEST

void ecore_test_ecore_evas(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_evas_associate);
   tcase_add_test(tc, ecore_test_ecore_evas_cocoa);
   tcase_add_test(tc, ecore_test_ecore_evas_fallback_selection);
   tcase_add_test(tc, ecore_test_ecore_evas_buffer_resize_async);
}
