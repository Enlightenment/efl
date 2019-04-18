#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_photo_legacy_type_check)
{
   Evas_Object *win, *photo;
   const char *type;

   win = win_add(NULL, "photo", ELM_WIN_BASIC);

   photo = elm_photo_add(win);

   type = elm_object_widget_type_get(photo);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Photo"));

   type = evas_object_type_get(photo);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_photo"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *photo;
   Efl_Access_Role role;

   win = win_add(NULL, "photo", ELM_WIN_BASIC);

   photo = elm_photo_add(win);
   role = efl_access_object_role_get(photo);

   ck_assert(role == EFL_ACCESS_ROLE_IMAGE);

}
EFL_END_TEST

void elm_test_photo(TCase *tc)
{
   tcase_add_test(tc, elm_photo_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
