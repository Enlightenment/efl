#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static void
_focus_in(void *data __UNUSED__, Evas *e __UNUSED__, void *event_info)
{
   const char *type = evas_object_type_get(event_info);
   if ((type) && (!strcmp(type, "elm_widget")))
     type = elm_object_widget_type_get(event_info);
   printf("Evas_Object focus in: %p %s\n", event_info, type);
}

static void
_focus_out(void *data __UNUSED__, Evas *e __UNUSED__, void *event_info)
{
   const char *type = evas_object_type_get(event_info);
   if ((type) && (!strcmp(type, "elm_widget")))
     type = elm_object_widget_type_get(event_info);
   printf("Evas_Object focus out: %p %s\n", event_info, type);
}

static void
_focus_obj(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *newfocus = data;
   const char *type = evas_object_type_get(newfocus);
   if ((type) && (!strcmp(type, "elm_widget")))
     type = elm_object_widget_type_get(newfocus);
   printf("elm_object_focus_set(%p, EINA_TRUE) %s\n", newfocus, type);
   elm_object_focus_set(newfocus, EINA_TRUE);
}

static void
_focus_layout_part(void *data, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *ed = elm_layout_edje_get(data);

   Evas_Object *newfocus = (Evas_Object *)edje_object_part_object_get(ed, "sky");
   const char *type = evas_object_type_get(newfocus);
   printf("evas_object_focus_set(%p, 1) %s\n", newfocus, type);
   evas_object_focus_set(newfocus, EINA_TRUE);;
}


void
test_focus2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ly, *bt, *en, *bt1;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "focus2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Focus2");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   evas_event_callback_add
     (evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
      _focus_in, NULL);
   evas_event_callback_add
     (evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
      _focus_out, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

#define PARENT bx /* this is broken, but should work */
//#define PARENT win

   en = elm_entry_add(PARENT);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_entry_set(en, "Scrolled Entry that should get focus");
   elm_entry_single_line_set(en, 1);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to scrolled entry");
   evas_object_smart_callback_add(bt, "clicked", _focus_obj, en);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   ly = elm_layout_add(PARENT);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "layout");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, ly);
   evas_object_show(ly);

   bt1 = bt = elm_button_add(ly);
   elm_object_text_set(bt, "Button 1");
   elm_layout_content_set(ly, "element1", bt);

   en = elm_entry_add(ly);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_scrollbar_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_entry_entry_set(en, "Scrolled Entry that should get focus");
   elm_entry_single_line_set(en, 1);
   elm_layout_content_set(ly, "element2", en);

   bt = elm_button_add(ly);
   elm_object_text_set(bt, "Button 2");
   elm_layout_content_set(ly, "element3", bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout");
   evas_object_smart_callback_add(bt, "clicked", _focus_obj, ly);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout part");
   evas_object_smart_callback_add(bt, "clicked", _focus_layout_part, ly);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout 'Button 1'");
   evas_object_smart_callback_add(bt, "clicked", _focus_obj, bt1);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout 'Entry'");
   evas_object_smart_callback_add(bt, "clicked", _focus_obj, en);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
#endif
