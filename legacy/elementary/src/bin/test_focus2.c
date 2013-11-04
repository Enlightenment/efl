#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>


static void
_focus_in(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info)
{
   const char *type = evas_object_type_get(event_info);
   if ((type) && (!strcmp(type, "elm_widget")))
     type = elm_object_widget_type_get(event_info);
   printf("Evas_Object focus in: %p %s\n", event_info, type);
}

static void
_focus_out(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info)
{
   const char *type = evas_object_type_get(event_info);
   if ((type) && (!strcmp(type, "elm_widget")))
     type = elm_object_widget_type_get(event_info);
   printf("Evas_Object focus out: %p %s\n", event_info, type);
}

static void
_focus_obj(void *data, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *newfocus = data;
   const char *type = evas_object_type_get(newfocus);
   if ((type) && (!strcmp(type, "elm_widget")))
     type = elm_object_widget_type_get(newfocus);
   printf("elm_object_focus_set(%p, EINA_TRUE) %s\n", newfocus, type);
   elm_object_focus_set(newfocus, EINA_TRUE);
}

static void
_focus_layout_part(void *data, Evas_Object *o EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *ed = elm_layout_edje_get(data);

   Evas_Object *newfocus = (Evas_Object *)edje_object_part_object_get(ed, "sky");
   const char *type = evas_object_type_get(newfocus);
   printf("evas_object_focus_set(%p, EINA_TRUE) %s\n", newfocus, type);
   evas_object_focus_set(newfocus, EINA_TRUE);;
}


void
test_focus2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *ly, *bt, *en, *en1, *bt1, *bt2;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("focus2", "Focus 2");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);

   evas_event_callback_add
     (evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
      _focus_in, NULL);
   evas_event_callback_add
     (evas_object_evas_get(win), EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
      _focus_out, NULL);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

#define PARENT bx /* this is broken, but should work */
//#define PARENT win

   en = elm_entry_add(PARENT);
   elm_entry_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_scroller_policy_set(en, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_object_text_set(en, "Entry that should get focus");
   elm_entry_single_line_set(en, EINA_TRUE);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to entry");
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
   elm_object_part_content_set(ly, "element1", bt);

   en1 = elm_entry_add(ly);
   elm_entry_scrollable_set(en1, EINA_TRUE);
   evas_object_size_hint_weight_set(en1, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en1, EVAS_HINT_FILL, 0.5);
   elm_scroller_policy_set(en1, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_object_text_set(en1, "Scrolled Entry that should get focus");
   elm_entry_single_line_set(en1, EINA_TRUE);
   elm_object_part_content_set(ly, "element2", en1);

   bt = elm_button_add(ly);
   elm_object_text_set(bt, "Button 2");
   elm_object_part_content_set(ly, "element3", bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout");
   evas_object_smart_callback_add(bt, "clicked", _focus_obj, ly);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout part");
   evas_object_smart_callback_add(bt, "clicked", _focus_layout_part, ly);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(PARENT);
   elm_object_text_set(bt, "Give focus to layout 'Button 1'");
   evas_object_smart_callback_add(bt, "clicked", _focus_obj, bt1);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt2 = elm_button_add(PARENT);
   elm_object_text_set(bt2, "Give focus to layout 'Entry'");
   evas_object_smart_callback_add(bt2, "clicked", _focus_obj, en);
   evas_object_size_hint_weight_set(bt2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt2, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt2);
   evas_object_show(bt2);

   elm_object_focus_next_object_set(bt2, en, ELM_FOCUS_DOWN);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
