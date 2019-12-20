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

static void
_geometry_update(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Eina_Rectangle *geom = event_info;
   int *set = data;
   if ((geom->w >= 150) && (geom->h >= 150))
     *set = 1;
   else
     *set = 0;
}

EFL_START_TEST(elm_ctxpopup_test_sizing)
{
   Eo *win, *bt, *ctx;
   int pass = 0;

   win = win_add();
   bt = elm_button_add(win);
   ctx = elm_ctxpopup_add(win);
   evas_object_smart_callback_add(ctx, "geometry,update", _geometry_update, &pass);
   elm_object_text_set(bt, "test");
   evas_object_size_hint_min_set(bt, 150, 150);
   elm_object_content_set(ctx, bt);
   evas_object_show(win);

   evas_object_resize(bt, 200, 200);
   evas_object_resize(win, 200, 200);

   evas_object_show(ctx);
   get_me_to_those_events(win);

   ck_assert_int_eq(pass, 1);
}
EFL_END_TEST

void elm_test_ctxpopup(TCase *tc)
{
   tcase_add_test(tc, elm_ctxpopup_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_ctxpopup_test_sizing);
}
