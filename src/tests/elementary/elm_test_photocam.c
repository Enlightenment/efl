#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *photocam;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "photocam", ELM_WIN_BASIC);

   photocam = elm_photocam_add(win);
   role = elm_interface_atspi_accessible_role_get(photocam);

   ck_assert(role == ELM_ATSPI_ROLE_IMAGE);

   elm_shutdown();
}
END_TEST

START_TEST (efl_ui_image_zoomable_icon)
{
   Evas_Object *win, *img_zoomable;
   Eina_Bool ok;
   const char *icon_name;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "photocam", ELM_WIN_BASIC);

   img_zoomable = efl_add(EFL_UI_IMAGE_ZOOMABLE_CLASS, win);
   evas_object_show(img_zoomable);

   ok = efl_ui_image_icon_set(img_zoomable, "folder");
   ck_assert(ok);
   icon_name = efl_ui_image_icon_get(img_zoomable);
   ck_assert_str_eq(icon_name, "folder");

   ok = efl_ui_image_icon_set(img_zoomable, "None");
   ck_assert(ok == 0);
   icon_name = efl_ui_image_icon_get(img_zoomable);
   ck_assert(icon_name == NULL);

   elm_shutdown();
}
END_TEST

void elm_test_photocam(TCase *tc)
{
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, efl_ui_image_zoomable_icon);
}
