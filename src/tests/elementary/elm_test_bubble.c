#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_bubble_legacy_type_check)
{
   Evas_Object *win, *bubble;
   const char *type;

   win = win_add(NULL, "bubble", ELM_WIN_BASIC);

   bubble = elm_bubble_add(win);

   type = elm_object_widget_type_get(bubble);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Bubble"));

   type = evas_object_type_get(bubble);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_bubble"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *bubble;
   Efl_Access_Role role;

   win = win_add(NULL, "bubble", ELM_WIN_BASIC);

   bubble = elm_bubble_add(win);
   role = efl_access_object_role_get(bubble);

   ck_assert(role == EFL_ACCESS_ROLE_FILLER);

}
EFL_END_TEST

EFL_START_TEST(elm_bubble_test_callbacks)
{
   Evas_Object *win, *bb, *ic, *ct;
   int called = 0;

   win = win_add(NULL, "bubble", ELM_WIN_BASIC);

   ic = elm_icon_add(win);
   ck_assert(elm_image_file_set(ic, ELM_IMAGE_DATA_DIR "/images/logo_small.png", NULL));
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);

   bb = elm_bubble_add(win);
   elm_object_text_set(bb, "Message 1");
   elm_object_part_text_set(bb, "info", "Corner: bottom_right");
   elm_object_part_content_set(bb, "icon", ic);
   elm_bubble_pos_set(bb, ELM_BUBBLE_POS_BOTTOM_RIGHT);
   evas_object_smart_callback_add(bb, "clicked", event_callback_single_call_int_data, &called);
   evas_object_show(ic);
   evas_object_size_hint_weight_set(bb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bb, EVAS_HINT_FILL, EVAS_HINT_FILL);

   ct = elm_label_add(win);
   elm_object_text_set(ct,
                       "\"The future of the art: R or G or B?\",  by Rusty");
   elm_object_content_set(bb, ct);

   evas_object_resize(bb, 240, 100);
   evas_object_resize(win, 240, 100);
   evas_object_show(bb);
   evas_object_show(win);

   get_me_to_those_events(bb);
   click_object(ct);
   ck_assert_int_eq(called, 1);
   called = 0;
   click_part(bb, "elm.info");
   ck_assert_int_eq(called, 1);
}
EFL_END_TEST

void elm_test_bubble(TCase *tc)
{
   tcase_add_test(tc, elm_bubble_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_bubble_test_callbacks);
}

