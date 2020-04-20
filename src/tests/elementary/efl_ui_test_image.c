#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"


EFL_START_TEST(efl_ui_image_test_icon)
{
   Eo *win, *image;
   Eina_Bool ok;
   const char *icon_name;

   win = win_add(NULL, "image", EFL_UI_WIN_TYPE_BASIC);

   image = efl_add(EFL_UI_IMAGE_CLASS, win);
   efl_gfx_entity_visible_set(image, EINA_TRUE);

   ok = efl_ui_image_icon_set(image, "folder");
   ck_assert(ok);
   icon_name = efl_ui_image_icon_get(image);
   ck_assert_str_eq(icon_name, "folder");

   ok = efl_ui_image_icon_set(image, "None");
   ck_assert(ok == 0);
   icon_name = efl_ui_image_icon_get(image);
   ck_assert(icon_name == NULL);
}
EFL_END_TEST

extern Eo *elm_image_object_get(Eo*);
EFL_START_TEST(efl_ui_image_test_scale_method)
{
   Eo *win, *image;
   Eina_Size2D sz;

   win = win_add(NULL, "image", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   image = efl_add(EFL_UI_IMAGE_CLASS, win,
      efl_file_set(efl_added, ELM_IMAGE_DATA_DIR"/images/logo.png"),
      efl_gfx_arrangement_content_align_set(efl_added, 0.5, 0.0),
      efl_gfx_image_scale_method_set(efl_added, EFL_GFX_IMAGE_SCALE_METHOD_FIT_WIDTH)
   );
   efl_gfx_entity_size_set(image, EINA_SIZE2D(100, 100));
   get_me_to_those_events(win);
   sz = efl_gfx_entity_size_get(image);
   ck_assert_int_eq(sz.w, 100);
   ck_assert_int_eq(sz.h, 100);
   /* legacy operation on eo object: very illegal */
   sz = efl_gfx_entity_size_get(elm_image_object_get(image));
   ck_assert_int_eq(sz.w, 100);
   ck_assert_int_eq(sz.h, 100);
}
EFL_END_TEST

void efl_ui_test_image(TCase *tc)
{
   tcase_add_test(tc, efl_ui_image_test_icon);
   tcase_add_test(tc, efl_ui_image_test_scale_method);
}
