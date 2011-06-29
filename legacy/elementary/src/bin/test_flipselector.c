#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

void
_sel_cb(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Flipselector_Item *it;

   it = event_info;
   printf("label of selected item is: %s\n", elm_flipselector_item_label_get(it));
}

void
_unsel_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Flipselector_Item *it;
   Evas_Object *fp = data;
   it = elm_flipselector_selected_item_get(fp);
   elm_flipselector_item_selected_set(it, EINA_FALSE);
}

void
_third_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Flipselector_Item *it;
   Evas_Object *fp = data;
   it = elm_flipselector_first_item_get(fp);
   it = elm_flipselector_item_next_get(it);
   it = elm_flipselector_item_next_get(it);
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
test_flipselector(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   char buf[8];
   unsigned int i;
   Evas_Object *win, *bg, *bx, *fp, *bt;
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

   win = elm_win_add(NULL, "flipselector", ELM_WIN_BASIC);
   elm_win_title_set(win, "Flip Selector");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

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
     elm_flipselector_item_append(fp, lbl[i], NULL, NULL);
   elm_box_pack_end(bx, fp);
   evas_object_show(fp);

   fp = elm_flipselector_add(win);
   evas_object_smart_callback_add(fp, "underflowed", _overflow_cb, NULL);
   evas_object_smart_callback_add(fp, "overflowed", _underflow_cb, NULL);
   evas_object_size_hint_weight_set(fp, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   for (i = 2099; i >= 1990; i--)
     {
	snprintf(buf, 8, "%d", i);
	elm_flipselector_item_append(fp, buf, _sel_cb, NULL);
     }
   elm_box_pack_end(bx, fp);
   evas_object_show(fp);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Select 2097");
   evas_object_smart_callback_add(bt, "clicked", _third_cb, fp);
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
