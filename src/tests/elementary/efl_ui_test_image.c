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

   win = win_add(NULL, "image", EFL_UI_WIN_BASIC);

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

void efl_ui_test_image(TCase *tc)
{
   tcase_add_test(tc, efl_ui_image_test_icon);
}
