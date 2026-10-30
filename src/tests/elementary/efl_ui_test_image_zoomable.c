#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_NOLEGACY_API_SUPPORT

#include <Efl_Ui.h>
#include "efl_ui_suite.h"


EFL_START_TEST(efl_ui_test_image_zoomable_icon)
{
   Evas_Object *win, *img_zoomable;
   Eina_Bool ok;
   const char *icon_name;

   win = win_add(NULL, "photocam", EFL_UI_WIN_TYPE_BASIC);

   img_zoomable = efl_add(EFL_UI_IMAGE_ZOOMABLE_CLASS, win);
   efl_gfx_entity_visible_set(img_zoomable, EINA_TRUE);

   ok = efl_ui_image_icon_set(img_zoomable, "folder");
   ck_assert(ok);
   icon_name = efl_ui_image_icon_get(img_zoomable);
   ck_assert_str_eq(icon_name, "folder");

   ok = efl_ui_image_icon_set(img_zoomable, "None");
   ck_assert(ok == 0);
   icon_name = efl_ui_image_icon_get(img_zoomable);
   ck_assert(icon_name == NULL);

}
EFL_END_TEST

static void
_count_errors(const Eina_Log_Domain *d, Eina_Log_Level level,
              const char *file, const char *fnc, int line,
              const char *fmt, void *data, va_list args)
{
   int *errors = data;

   if (level <= EINA_LOG_LEVEL_ERR)
     (*errors)++;
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
}

EFL_START_TEST(efl_ui_test_image_zoomable_replaced_calc_job)
{
   Evas_Object *win, *img_zoomable;
   int errors = 0, i;

   win = win_add(NULL, "photocam", EFL_UI_WIN_TYPE_BASIC);

   img_zoomable = efl_add(EFL_UI_IMAGE_ZOOMABLE_CLASS, win);
   ck_assert_int_eq(efl_file_set(img_zoomable, ELM_IMAGE_DATA_DIR"/images/logo.png"), 0);
   ck_assert_int_eq(efl_file_load(img_zoomable), 0);
   ck_assert_int_eq(efl_file_set(img_zoomable, ELM_IMAGE_DATA_DIR"/images/bubble.png"), 0);
   ck_assert_int_eq(efl_file_load(img_zoomable), 0);
   efl_file_unload(img_zoomable);
   ck_assert_int_eq(efl_file_load(img_zoomable), 0);

   eina_log_print_cb_set(_count_errors, &errors);
   efl_del(img_zoomable);
   eina_freeq_clear(eina_freeq_main_get());
   for (i = 0; i < 3; i++)
     {
        ecore_main_loop_iterate();
     }
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   ck_assert_int_eq(errors, 0);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_image_zoomable_file_after_icon)
{
   Evas_Object *win, *img_zoomable, *reference;
   Eina_Size2D expected, size;
   int errors = 0;

   win = win_add(NULL, "photocam", EFL_UI_WIN_TYPE_BASIC);

   reference = efl_add(EFL_UI_IMAGE_CLASS, win);
   ck_assert(efl_file_simple_load(reference, ELM_IMAGE_DATA_DIR"/images/logo.png", NULL));
   expected = efl_gfx_view_size_get(reference);
   ck_assert_int_gt(expected.w, 0);

   img_zoomable = efl_add(EFL_UI_IMAGE_ZOOMABLE_CLASS, win);
   ck_assert(efl_ui_image_icon_set(img_zoomable, "folder"));
   ck_assert_int_eq(efl_file_set(img_zoomable, ELM_IMAGE_DATA_DIR"/images/logo.png"), 0);
   ck_assert_int_eq(efl_file_load(img_zoomable), 0);

   size = efl_gfx_view_size_get(img_zoomable);
   ck_assert_int_eq(size.w, expected.w);
   ck_assert_int_eq(size.h, expected.h);

   eina_log_print_cb_set(_count_errors, &errors);
   efl_del(img_zoomable);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   ck_assert_int_eq(errors, 0);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_test_image_zoomable_set_while_constructing)
{
   Evas_Object *win, *img_zoomable;
   Eina_Size2D size;

   win = win_add(NULL, "photocam", EFL_UI_WIN_TYPE_BASIC);

   img_zoomable = efl_add(EFL_UI_IMAGE_ZOOMABLE_CLASS, win,
                          efl_file_set(efl_added, ELM_IMAGE_DATA_DIR"/images/logo.png"));
   ck_assert(efl_file_loaded_get(img_zoomable));
   size = efl_gfx_view_size_get(img_zoomable);
   ck_assert_int_gt(size.w, 0);
   ck_assert_int_gt(size.h, 0);

   img_zoomable = efl_add(EFL_UI_IMAGE_ZOOMABLE_CLASS, win,
                          efl_ui_image_icon_set(efl_added, "folder"));
   ck_assert(efl_file_loaded_get(img_zoomable));
   ck_assert_str_eq(efl_ui_image_icon_get(img_zoomable), "folder");
}
EFL_END_TEST

void efl_ui_test_image_zoomable(TCase *tc)
{
   tcase_add_test(tc, efl_ui_test_image_zoomable_icon);
   tcase_add_test(tc, efl_ui_test_image_zoomable_replaced_calc_job);
   tcase_add_test(tc, efl_ui_test_image_zoomable_file_after_icon);
   tcase_add_test(tc, efl_ui_test_image_zoomable_set_while_constructing);
}
