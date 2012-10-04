#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

void
_sel_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Object_Item *it;

   it = event_info;
   printf("label of selected item is: %s\n", elm_object_item_text_get(it));
}

void
_unsel_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Object_Item *it;
   Evas_Object *fp = data;
   it = elm_flipselector_selected_item_get(fp);
   elm_flipselector_item_selected_set(it, EINA_FALSE);
}

void
_last_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Object_Item *it;
   Evas_Object *fp = data;
   it = elm_flipselector_last_item_get(fp);
   elm_flipselector_item_selected_set(it, EINA_TRUE);
}

void
_third_from_end_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Object_Item *it;
   Evas_Object *fp = data;
   it = elm_flipselector_last_item_get(fp);
   it = elm_flipselector_item_prev_get(it);
   it = elm_flipselector_item_prev_get(it);
   elm_flipselector_item_selected_set(it, EINA_TRUE);
}

void
_underflow_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("underflow!\n");
}

void
_overflow_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   printf("overflow!\n");
}

void
slider_change_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *fl = (Evas_Object *)data;

   double val = elm_slider_value_get(obj);
   elm_flipselector_first_interval_set(fl, val);
}

void
flip_next_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *fp = data;
   elm_flipselector_flip_next(fp);
}

void
flip_prev_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *fp = data;
   elm_flipselector_flip_prev(fp);
}

void
test_flipselector(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[8];
   unsigned int i;
   Evas_Object *win, *bx, *fp, *bt, *bx2, *sl;
   Elm_Object_Item *it, *edje_it = NULL;
   static const char *lbl[] = {
        "Elementary",
        "Evas",
        "Eina",
        "Edje",
        "Eet",
        "Ecore",
        "Efreet",
        "Edbus"
   };

   win = elm_win_util_standard_add("flipselector", "Flip Selector");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fp = elm_flipselector_add(win);
   evas_object_size_hint_weight_set(fp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(fp, "selected", _sel_cb, NULL);
   evas_object_smart_callback_add(fp, "underflowed", _overflow_cb, NULL);
   evas_object_smart_callback_add(fp, "overflowed", _underflow_cb, NULL);
   for (i = 0; i < sizeof(lbl)/sizeof(char *); i++)
     {
        it = elm_flipselector_item_append(fp, lbl[i], NULL, NULL);
        if (i == 3) edje_it = it;
     }
   elm_flipselector_item_selected_set(edje_it, EINA_TRUE);
   elm_box_pack_end(bx, fp);
   evas_object_show(fp);

   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   fp = elm_flipselector_add(win);
   evas_object_smart_callback_add(fp, "underflowed", _overflow_cb, NULL);
   evas_object_smart_callback_add(fp, "overflowed", _underflow_cb, NULL);
   evas_object_size_hint_weight_set(fp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   for (i = 1990; i <= 2099; i++)
     {
        snprintf(buf, 8, "%u", i);
        elm_flipselector_item_append(fp, buf, _sel_cb, NULL);
     }

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Flip Prev");
   evas_object_smart_callback_add(bt, "clicked", flip_prev_cb, fp);

   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx2, fp);
   evas_object_show(fp);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Flip Next");
   evas_object_smart_callback_add(bt, "clicked", flip_next_cb, fp);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Flip Iterval:");
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0, 3.0);
   elm_slider_value_set(sl, 0.85);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);
   evas_object_smart_callback_add(sl, "changed", slider_change_cb, fp);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select Last");
   evas_object_smart_callback_add(bt, "clicked", _last_cb, fp);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select 2097");
   evas_object_smart_callback_add(bt, "clicked", _third_from_end_cb, fp);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Unselect year");
   evas_object_smart_callback_add(bt, "clicked", _unsel_cb, fp);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}
#endif
