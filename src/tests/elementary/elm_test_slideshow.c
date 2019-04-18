#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_slideshow_legacy_type_check)
{
   Evas_Object *win, *slideshow;
   const char *type;

   win = win_add(NULL, "slideshow", ELM_WIN_BASIC);

   slideshow = elm_slideshow_add(win);

   type = elm_object_widget_type_get(slideshow);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Slideshow"));

   type = evas_object_type_get(slideshow);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_slideshow"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *slideshow;
   Efl_Access_Role role;

   win = win_add(NULL, "slideshow", ELM_WIN_BASIC);

   slideshow = elm_slideshow_add(win);
   role = efl_access_object_role_get(slideshow);

   ck_assert(role == EFL_ACCESS_ROLE_DOCUMENT_PRESENTATION);

}
EFL_END_TEST

void elm_test_slideshow(TCase *tc)
{
   tcase_add_test(tc, elm_slideshow_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
