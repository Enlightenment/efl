#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_popup_legacy_type_check)
{
   Evas_Object *win, *popup;
   const char *type;

   win = win_add(NULL, "popup", ELM_WIN_BASIC);

   popup = elm_popup_add(win);

   type = elm_object_widget_type_get(popup);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Popup"));

   type = evas_object_type_get(popup);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_popup"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *popup;
   Efl_Access_Role role;

   win = win_add(NULL, "popup", ELM_WIN_BASIC);

   popup = elm_popup_add(win);
   role = efl_access_object_role_get(popup);

   ck_assert(role == EFL_ACCESS_ROLE_DIALOG);

}
EFL_END_TEST

static void
_focus_cb(void *data, Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   *((Eina_Bool*)data) = EINA_TRUE;
}


EFL_START_TEST(elm_popup_focus_get)
{
   Evas_Object *win, *popup;
   Eina_Bool focused = EINA_FALSE;

   win = win_add(NULL, "popup", ELM_WIN_BASIC);

   popup = elm_popup_add(win);
   elm_popup_scrollable_set(popup, EINA_TRUE);
   elm_object_text_set(popup, "This Popup has content area and "
                       "timeout value is 3 seconds");
   elm_popup_timeout_set(popup, 3.0);
   evas_object_smart_callback_add(popup, "focused", _focus_cb, &focused);

   // popup show should be called after adding all the contents and the buttons
   // of popup to set the focus into popup's contents correctly.
   evas_object_show(popup);
   ck_assert(focused);
}
EFL_END_TEST

EFL_START_TEST(elm_popup_text_set)
{
   Evas_Object *win, *popup;
   const char *popup_text = "hello world";

   win = win_add(NULL, "popup", ELM_WIN_BASIC);

   popup = elm_popup_add(win);

   ck_assert(elm_layout_text_set(popup, NULL, popup_text));
   ck_assert_str_eq(elm_object_text_get(popup), popup_text);
}
EFL_END_TEST

void elm_test_popup(TCase *tc)
{
   tcase_add_test(tc, elm_popup_focus_get);
   tcase_add_test(tc, elm_popup_legacy_type_check);
   tcase_add_test(tc, elm_popup_text_set);
   tcase_add_test(tc, elm_atspi_role_get);
}
