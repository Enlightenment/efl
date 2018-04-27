#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_photocam_legacy_type_check)
{
   Evas_Object *win, *photocam;
   const char *type;

   win = win_add(NULL, "photocam", ELM_WIN_BASIC);

   photocam = elm_photocam_add(win);

   type = elm_object_widget_type_get(photocam);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Photocam"));

   type = evas_object_type_get(photocam);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_photocam"));

}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *photocam;
   Efl_Access_Role role;

   win = win_add(NULL, "photocam", ELM_WIN_BASIC);

   photocam = elm_photocam_add(win);
   role = efl_access_object_role_get(photocam);

   ck_assert(role == EFL_ACCESS_ROLE_IMAGE);

}
EFL_END_TEST

EFL_START_TEST (efl_ui_image_zoomable_icon)
{
   Evas_Object *win, *img_zoomable;
   Eina_Bool ok;
   const char *icon_name;

   win = win_add(NULL, "photocam", ELM_WIN_BASIC);

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

}
EFL_END_TEST

void elm_test_photocam(TCase *tc)
{
   tcase_add_test(tc, elm_photocam_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, efl_ui_image_zoomable_icon);
}
