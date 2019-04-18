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

void efl_ui_test_image_zoomable(TCase *tc)
{
   tcase_add_test(tc, efl_ui_test_image_zoomable_icon);
}
