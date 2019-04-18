#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_ctxpopup_legacy_type_check)
{
   Evas_Object *win, *ctxpopup;
   const char *type;

   win = win_add(NULL, "ctxpopup", ELM_WIN_BASIC);

   ctxpopup = elm_ctxpopup_add(win);

   type = elm_object_widget_type_get(ctxpopup);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Ctxpopup"));

   type = evas_object_type_get(ctxpopup);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_ctxpopup"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *ctxpopup;
   Efl_Access_Role role;

   win = win_add(NULL, "icon", ELM_WIN_BASIC);

   ctxpopup = elm_ctxpopup_add(win);
   role = efl_access_object_role_get(ctxpopup);

   ck_assert(role == EFL_ACCESS_ROLE_POPUP_MENU);

}
EFL_END_TEST

void elm_test_ctxpopup(TCase *tc)
{
   tcase_add_test(tc, elm_ctxpopup_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
