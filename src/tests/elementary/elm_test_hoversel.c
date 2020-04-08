#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

#include "elm_priv.h"
#include "elm_widget_hoversel.h"

EFL_START_TEST(elm_hoversel_legacy_type_check)
{
   Evas_Object *win, *hoversel;
   const char *type;

   win = win_add(NULL, "hoversel", ELM_WIN_BASIC);

   hoversel = elm_hoversel_add(win);

   type = elm_object_widget_type_get(hoversel);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Hoversel"));

   type = evas_object_type_get(hoversel);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_hoversel"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *hoversel;
   Efl_Access_Role role;

   win = win_add(NULL, "hoversel", ELM_WIN_BASIC);

   hoversel = elm_hoversel_add(win);
   role = efl_access_object_role_get(hoversel);

   ck_assert(role == EFL_ACCESS_ROLE_PUSH_BUTTON);

}
EFL_END_TEST

EFL_START_TEST(elm_test_hoversel_behavior)
{
   Eo *hoversel, *win = win_add();
   unsigned int i;
   const char *callbacks[] =
   {
      "expanded",
      "clicked",
      "selected",
      "item,focused",
      "item,unfocused",
      "dismissed",
   };
   int count[EINA_C_ARRAY_LENGTH(callbacks)] = {0};

   evas_object_resize(win, 500, 500);
   hoversel = elm_hoversel_add(win);
   evas_object_geometry_set(hoversel, 25, 25, 50, 50);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Vertical");
   elm_hoversel_item_add(hoversel, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 4 - Long Label Here", "close", ELM_ICON_STANDARD, NULL, NULL);

   evas_object_show(win);
   evas_object_show(hoversel);
   elm_object_focus_set(hoversel, EINA_TRUE);
   for (i = 0; i < EINA_C_ARRAY_LENGTH(count); i++)
     {
        evas_object_smart_callback_add(hoversel, callbacks[i],
                                      (void*)event_callback_single_call_int_data, &count[i]);
     }
   evas_object_smart_callback_add(hoversel, "dismissed",
                                      (void*)event_callback_that_quits_the_main_loop_when_called, NULL);
   get_me_to_those_events(win);
   assert_object_size_eq(hoversel, 50, 50);
   click_object(hoversel);
   get_me_to_those_events(win);

   /* expanded */
   ck_assert_int_eq(count[0], 1);
   wait_timer(0.6); // from default theme
   ecore_main_loop_iterate();

   ELM_HOVERSEL_DATA_GET(hoversel, sd);
   click_object(eina_list_data_get(elm_box_children_get(sd->bx)));
   get_me_to_those_events(win);

   /* clicked */
   ck_assert_int_eq(count[1], 1);
   /* selected */
   ck_assert_int_eq(count[2], 1);
   /* item,focused */
   ck_assert_int_eq(count[3], 1);
   /* item,focused */
   ck_assert_int_eq(count[4], 1);
   ecore_main_loop_begin();

   /* dismissed */
   ck_assert_int_eq(count[5], 1);
}
EFL_END_TEST

EFL_START_TEST(elm_test_hoversel_position)
{
   Eo *hoversel, *win = win_add();

   evas_object_resize(win, 500, 500);
   hoversel = elm_hoversel_add(win);
   evas_object_geometry_set(hoversel, 450, 450, 50, 50);
   elm_hoversel_hover_parent_set(hoversel, win);
   elm_object_text_set(hoversel, "Vertical");
   elm_hoversel_item_add(hoversel, "Item 1", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 2", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 3", NULL, ELM_ICON_NONE, NULL, NULL);
   elm_hoversel_item_add(hoversel, "Item 4 - Long Label Here", "close", ELM_ICON_STANDARD, NULL, NULL);

   evas_object_show(win);
   evas_object_show(hoversel);
   elm_object_focus_set(hoversel, EINA_TRUE);

   get_me_to_those_events(win);
   assert_object_size_eq(hoversel, 50, 50);
   click_object(hoversel);
   get_me_to_those_events(win);

   wait_timer(0.6); // from default theme
   ecore_main_loop_iterate();

   ELM_HOVERSEL_DATA_GET(hoversel, sd);
   {
      int x, y, w, h;
      Eo *item = eina_list_data_get(elm_box_children_get(sd->bx));
      evas_object_geometry_get(item, &x, &y, &w, &h);

      /* verify that all buttons are in-canvas */
      ck_assert_int_le(x + w, 500);
      ck_assert_int_le(y + h, 500);
   }

   evas_object_del(hoversel);
}
EFL_END_TEST

void elm_test_hoversel(TCase *tc)
{
   tcase_add_test(tc, elm_hoversel_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_test_hoversel_behavior);
   tcase_add_test(tc, elm_test_hoversel_position);
}
