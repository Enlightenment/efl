#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_slider_legacy_type_check)
{
   Evas_Object *win, *slider;
   const char *type;

   win = win_add(NULL, "slider", ELM_WIN_BASIC);

   slider = elm_slider_add(win);

   type = elm_object_widget_type_get(slider);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Slider"));

   type = evas_object_type_get(slider);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_slider"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *slider;
   Efl_Access_Role role;

   win = win_add(NULL, "slider", ELM_WIN_BASIC);

   slider = elm_slider_add(win);
   role = efl_access_object_role_get(slider);

   ck_assert(role == EFL_ACCESS_ROLE_SLIDER);

}
EFL_END_TEST

EFL_START_TEST(elm_slider_in_scroller)
{
   Evas_Object *win, *slider, *box, *scroller, *fslider = NULL;
   Evas *e;

   win = win_add(NULL, "slider", ELM_WIN_BASIC);
   e = evas_object_evas_get(win);
   evas_object_size_hint_weight_set(win, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   scroller = elm_scroller_add(win);
   evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(scroller);
   elm_win_resize_object_add(win, scroller);

   box = elm_box_add(scroller);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_object_content_set(scroller, box);
   evas_object_show(box);

   for (int i = 0; i < 20; ++i)
     {
        slider = elm_slider_add(win);
        evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        elm_slider_min_max_set(slider, 0, 100);
        evas_object_show(slider);
        elm_box_pack_end(box, slider);
        if (i == 0)
          fslider = slider;
     }
   elm_scroller_page_bring_in(scroller, 0, 0);
   evas_object_resize(win, 100, 100);
   evas_object_show(win);

   get_me_to_those_events(scroller);

   int x, y, w, h;

   ck_assert_int_eq(elm_object_scroll_hold_get(scroller), 0);
   evas_event_feed_mouse_in(e, 0, NULL);
   evas_object_geometry_get(fslider, &x, &y, &w, &h);
   evas_event_feed_mouse_move(e, x+w/2, y+h/2, 0, NULL);
   //ensure that the scroller is on hold
   ck_assert_int_eq(elm_object_scroll_hold_get(scroller), 1);
   evas_event_feed_mouse_move(e, x+w+w/2, y+h+h/2, 0, NULL);
   ck_assert_int_eq(elm_object_scroll_hold_get(scroller), 0);
}
EFL_END_TEST

static unsigned int event_counter;

static void
slider_changed(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   event_counter++;
   if (event_counter == 1)
     evas_object_smart_callback_del(obj, "changed", slider_changed);
   else if (event_counter == 2)
     evas_object_smart_callback_del(obj, "slider,drag,start", slider_changed);
   else if (event_counter == 3)
     evas_object_smart_callback_del(obj, "slider,drag,stop", slider_changed);
   else if (event_counter == 4)
     ecore_main_loop_quit();
}

EFL_START_TEST(elm_slider_events)
{
   Evas_Object *win, *slider;
   Evas *e;

   win = win_add(NULL, "slider", ELM_WIN_BASIC);
   e = evas_object_evas_get(win);

   slider = elm_slider_add(win);
   evas_object_smart_callback_add(slider, "changed", slider_changed, NULL);
   evas_object_smart_callback_add(slider, "delay,changed", slider_changed, NULL);
   evas_object_smart_callback_add(slider, "slider,drag,start", slider_changed, NULL);
   evas_object_smart_callback_add(slider, "slider,drag,stop", slider_changed, NULL);
   evas_object_show(slider);
   evas_object_show(win);
   evas_object_resize(slider, 400, 100);
   evas_object_resize(win, 400, 100);
   edje_object_message_signal_process(elm_layout_edje_get(slider));
   get_me_to_those_events(slider);


   int x, y, w, h;
   int sx, sy, sw, sh;

   evas_object_geometry_get(elm_object_part_content_get(slider, "elm.swallow.bar"), &x, &y, &w, &h);
   evas_object_geometry_get(slider, &sx, &sy, &sw, &sh);
   evas_event_feed_mouse_in(e, 0, NULL);
   evas_event_feed_mouse_move(e, x + (w / 2), y + (h / 2), 0, NULL);
   evas_event_feed_mouse_down(e, 1, 0, 0, NULL);
   evas_event_feed_mouse_move(e, sx + (sw / 2), sy + (sh / 2), 0, NULL);
   evas_event_feed_mouse_up(e, 1, 0, 0, NULL);
   ecore_main_loop_begin();
   ck_assert_int_eq(event_counter, 4);
}
EFL_END_TEST

EFL_START_TEST(elm_slider_indicator_format_set_get_p)
{
   Evas_Object *win, *slider;
   const char *fmt;

   win = win_add(NULL, "slider", ELM_WIN_BASIC);
   slider = elm_slider_add(win);
   elm_slider_indicator_format_set(slider, "%1.0f");
   fmt = elm_slider_indicator_format_get(slider);

   ck_assert(fmt != NULL);
   ck_assert(!strcmp(fmt, "%1.0f"));
}
EFL_END_TEST

EFL_START_TEST(elm_slider_indicator_format_get_n)
{
   const char *fmt;

   fmt = elm_slider_indicator_format_get(NULL);

   ck_assert(fmt == NULL);
}
EFL_END_TEST

EFL_START_TEST(elm_slider_unit_format_get_n)
{
   const char *fmt;

   fmt = elm_slider_unit_format_get(NULL);

   ck_assert(fmt == NULL);
}
EFL_END_TEST

void elm_test_slider(TCase *tc)
{
   tcase_add_test(tc, elm_slider_legacy_type_check);
   tcase_add_test(tc, elm_slider_in_scroller);
   tcase_add_test(tc, elm_slider_events);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_slider_indicator_format_set_get_p);
   tcase_add_test(tc, elm_slider_indicator_format_get_n);
   tcase_add_test(tc, elm_slider_unit_format_get_n);
}
