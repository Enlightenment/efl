#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

typedef struct _Simple_Test_Widget
{
   Evas_Object* (*constructor)(Evas_Object *win);
   const char *name;
} Simple_Test_Widget;

static Evas_Object*
_custom_clock(Evas_Object *win)
{
   Evas_Object *o = elm_clock_add(win);
   elm_clock_edit_set(o, EINA_TRUE);
   return o;
}

static void
_realized(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   static int i = 0;

   i++;
   //we have two items - each of them are getting realized unrealized and again realized.
   if (i == 2)
     ecore_main_loop_quit();
}

static Evas_Object*
_content_get(void *data EINA_UNUSED, Evas_Object *obj, const char *part EINA_UNUSED)
{
   return elm_button_add(obj);
}

static Evas_Object*
_custom_gengrid(Evas_Object *win)
{
   Evas_Object *o;
   Elm_Object_Item *it;
   Elm_Gengrid_Item_Class *itc;

   o = elm_gengrid_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);

   itc = elm_gengrid_item_class_new();
   itc->func.content_get = _content_get;

   it = elm_gengrid_item_append(o, itc, NULL, NULL, NULL);
   evas_object_smart_callback_add(o, "realized", _realized, it);
   evas_object_size_hint_min_set(o, 200, 200);

   elm_object_item_focus_set(it, EINA_TRUE);
   return o;
}

static Evas_Object*
_custom_genlist(Evas_Object *win)
{
   Evas_Object *o;
   Elm_Object_Item *it;
   Elm_Genlist_Item_Class *itc;

   o = elm_genlist_add(win);
   evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_genlist_mode_set(o, ELM_LIST_COMPRESS);

   itc = elm_genlist_item_class_new();
   itc->func.content_get = _content_get;

   it = elm_genlist_item_append(o, itc, NULL, NULL, 0, NULL, NULL);
   evas_object_smart_callback_add(o, "realized", _realized, it);
   evas_object_size_hint_min_set(o, 200, 200);

   elm_object_item_focus_set(it, EINA_TRUE);
   return o;
}

static Evas_Object*
_custom_list(Evas_Object *win)
{
   Evas_Object *o = elm_list_add(win);
   elm_list_item_append(o, "test", NULL, NULL, NULL, NULL);
   return o;
}

static Evas_Object*
_custom_progressbar(Evas_Object *win)
{
   Evas_Object *o = elm_progressbar_add(win);
   efl_ui_widget_focus_allow_set(o, EINA_TRUE);
   return o;
}

static Evas_Object*
_custom_colorselector_add(Evas_Object *win)
{
   Evas_Object *o = elm_colorselector_add(win);
   Elm_Object_Item *item = elm_colorselector_palette_color_add(o, 0, 0, 0, 255);
   elm_colorselector_palette_color_add(o, 255, 255, 255, 255);
   elm_colorselector_palette_item_selected_set(item, EINA_TRUE);
   return o;
}

static const Simple_Test_Widget simple_widgets[] = {
  {elm_button_add, "button"},
  {elm_check_add, "check"},
  {elm_radio_add, "radio"},
  {elm_diskselector_add, "diskselector"},
  {elm_entry_add, "entry"},
  {elm_flipselector_add, "flipselector"},
  {elm_video_add, "video"},
  {elm_spinner_add, "spinner"},
  {elm_multibuttonentry_add, "mbe"},
  {elm_fileselector_add, "fileselector"},
  {elm_fileselector_button_add, "fileselector_button"},
  {elm_fileselector_entry_add, "fileselector_entry"},
  {_custom_clock, "clock"},
  {elm_toolbar_add, "toolbar"},
  {elm_gengrid_add, "gengrid1"},
  {elm_genlist_add, "genlist1"},
  {_custom_gengrid, "gengrid2"},
  {_custom_genlist, "genlist2"},
  {elm_list_add, "list1"},
  {_custom_list, "list2"},
  {elm_scroller_add, "scroller"},
  {elm_photocam_add, "photocam"},
  {_custom_progressbar, "progressbar"},
  {elm_web_add, "web"},
  {elm_colorselector_add, "colorselector"},
  {_custom_colorselector_add, "colorselector2"},
  {elm_slideshow_add, "slideshow"},
  {elm_map_add, "map"},
  {NULL, NULL},
};

static void
_eventing_test(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_Bool *val = data;

   *val = EINA_TRUE;
}

EFL_START_TEST(elm_test_widget_focus_simple_widget)
{
   Evas_Object *win, *box, *resettor, *o;
   Eina_Hash *map;

   map = eina_hash_string_small_new(NULL);
   win = win_add_focused(NULL, "focus test", ELM_WIN_BASIC);

   box = elm_box_add(win);
   elm_win_resize_object_add(win, box);
   evas_object_show(win);
   evas_object_show(box);

   resettor = o = elm_button_add(win);
   elm_box_pack_end(box, o);
   evas_object_show(o);
   elm_object_focus_set(o, EINA_TRUE);

   for (int i = 0; simple_widgets[i].name; ++i)
     {
        o = simple_widgets[i].constructor(win);
        elm_box_pack_end(box, o);
        evas_object_show(o);

        eina_hash_add(map, &simple_widgets[i].name, o);
      }
   evas_object_resize(win, 200, 200);

   //I have no idea why this is needed - but if this here is not done,
   // then elements that are part of a layout will NOT be shown even if
   // the window and layout is shown
   evas_object_hide(win);
   evas_object_show(win);

   ecore_main_loop_begin();

   for (int i = 0; simple_widgets[i].name; ++i)
      {
         Eina_Bool flag_focused = EINA_FALSE, flag_unfocused = EINA_FALSE;

         printf("Testing %s\n", simple_widgets[i].name);

         o = eina_hash_find(map, &simple_widgets[i].name);
         evas_object_smart_callback_add(o, "focused", _eventing_test, &flag_focused);
         evas_object_smart_callback_add(o, "unfocused", _eventing_test, &flag_unfocused);
         elm_object_focus_set(o, EINA_TRUE);

         ck_assert_int_eq(flag_focused, EINA_TRUE);
         ck_assert_int_eq(flag_unfocused, EINA_FALSE);
         ck_assert_int_eq(elm_object_focus_get(o), EINA_TRUE);
         ck_assert_int_eq(elm_object_focus_get(resettor), EINA_FALSE);

         elm_object_focus_set(resettor, EINA_TRUE);
         ck_assert_int_eq(flag_focused, EINA_TRUE);
         ck_assert_int_eq(flag_unfocused, EINA_TRUE);
         ck_assert_int_eq(elm_object_focus_get(resettor), EINA_TRUE);
         ck_assert_int_eq(elm_object_focus_get(o), EINA_FALSE);
      }

   eina_hash_free(map);
}
EFL_END_TEST


void elm_test_widget_focus(TCase *tc)
{
   tcase_add_test(tc, elm_test_widget_focus_simple_widget);
}
