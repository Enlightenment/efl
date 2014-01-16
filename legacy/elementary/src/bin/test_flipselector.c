#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
_sel_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *it;

   it = event_info;
   printf("label of selected item is: %s\n", elm_object_item_text_get(it));
}

void
_unsel_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it;
   Evas_Object *fp = data;
   it = elm_flipselector_selected_item_get(fp);
   elm_flipselector_item_selected_set(it, EINA_FALSE);
}

void
_last_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it;
   Evas_Object *fp = data;
   it = elm_flipselector_last_item_get(fp);
   elm_flipselector_item_selected_set(it, EINA_TRUE);
}

void
_third_from_end_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Elm_Object_Item *it;
   Evas_Object *fp = data;
   it = elm_flipselector_last_item_get(fp);
   it = elm_flipselector_item_prev_get(it);
   it = elm_flipselector_item_prev_get(it);
   elm_flipselector_item_selected_set(it, EINA_TRUE);
}

void
_underflow_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("underflow!\n");
}

void
_overflow_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("overflow!\n");
}

void
slider_change_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *fl = (Evas_Object *)data;

   double val = elm_slider_value_get(obj);
   elm_flipselector_first_interval_set(fl, val);
}

void
flip_next_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fp = data;
   elm_flipselector_flip_next(fp);
}

void
flip_prev_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *fp = data;
   elm_flipselector_flip_prev(fp);
}

void
test_flipselector(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   char buf[8];
   unsigned int i;
   Evas_Object *win, *bx, *fp, *fpd, *bt, *bx2, *sl;
   Elm_Object_Item *it;
   static const char *lbl[] = {
        "Elementary",
        "Evas",
        "Eina",
        "Edje",
        "Eet",
        "Ecore",
        "Efreet",
        "Eldbus"
   };

   win = elm_win_util_standard_add("flipselector", "Flip Selector");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   fp = elm_flipselector_add(bx);
   evas_object_size_hint_weight_set(fp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(fp, "selected", _sel_cb, NULL);
   evas_object_smart_callback_add(fp, "underflowed", _overflow_cb, NULL);
   evas_object_smart_callback_add(fp, "overflowed", _underflow_cb, NULL);
   for (i = 0; i < sizeof(lbl)/sizeof(char *); i++)
     {
        it = elm_flipselector_item_append(fp, lbl[i], NULL, NULL);
        if (i == 3)
          elm_flipselector_item_selected_set(it, EINA_TRUE);
     }
   elm_box_pack_end(bx, fp);
   evas_object_show(fp);

   bx2 = elm_box_add(bx);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   fp = elm_flipselector_add(bx2);
   evas_object_smart_callback_add(fp, "underflowed", _overflow_cb, NULL);
   evas_object_smart_callback_add(fp, "overflowed", _underflow_cb, NULL);
   evas_object_size_hint_weight_set(fp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   for (i = 1990; i <= 2099; i++)
     {
        snprintf(buf, 8, "%u", i);
        elm_flipselector_item_append(fp, buf, _sel_cb, NULL);
     }

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Flip Prev");
   evas_object_smart_callback_add(bt, "clicked", flip_prev_cb, fp);

   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx2, fp);
   evas_object_show(fp);

   bt = elm_button_add(bx2);
   elm_object_text_set(bt, "Flip Next");
   evas_object_smart_callback_add(bt, "clicked", flip_next_cb, fp);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   fpd = elm_flipselector_add(bx);
   evas_object_size_hint_weight_set(fpd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_flipselector_item_append(fpd, "disabled", NULL, NULL);
   elm_object_disabled_set(fpd, EINA_TRUE);
   elm_box_pack_end(bx, fpd);
   evas_object_show(fpd);

   sl = elm_slider_add(bx);
   elm_object_text_set(sl, "Flip Interval:");
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0, 3.0);
   elm_slider_value_set(sl, 0.85);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);
   evas_object_smart_callback_add(sl, "changed", slider_change_cb, fp);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Select Last");
   evas_object_smart_callback_add(bt, "clicked", _last_cb, fp);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Select 2097");
   evas_object_smart_callback_add(bt, "clicked", _third_from_end_cb, fp);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Unselect year");
   evas_object_smart_callback_add(bt, "clicked", _unsel_cb, fp);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}
