#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static double size = 0.0;

static void
_press(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    printf("press\n");
}

static void
_unpress(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
    printf("unpress, size : %f\n", elm_panes_content_left_size_get(obj));
}

static void
_clicked(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    printf("clicked\n");
}


static void
_clicked_double(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
    printf("clicked double\n");
    if(elm_panes_content_left_size_get(obj) > 0)
    {
        size = elm_panes_content_left_size_get(obj);
        elm_panes_content_left_size_set(obj, 0.0);
    }
    else
        elm_panes_content_left_size_set(obj, size);
}


void
test_panes(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *panes, *panes_h, *bt;

   win = elm_win_add(NULL, "panes", ELM_WIN_BASIC);
   elm_win_title_set(win, "Panes");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   evas_object_show(bg);

   panes = elm_panes_add(win);
   elm_win_resize_object_add(win, panes);
   evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panes);

   evas_object_smart_callback_add(panes, "clicked", _clicked, panes);
   evas_object_smart_callback_add(panes, "clicked,double", _clicked_double, panes);

   evas_object_smart_callback_add(panes, "press", _press, panes);
   evas_object_smart_callback_add(panes, "unpress", _unpress, panes);


   //
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_show(bt);
   elm_panes_content_left_set(panes, bt);
   //

   //
   panes_h = elm_panes_add(win);
   elm_panes_horizontal_set(panes_h, EINA_TRUE);
   evas_object_size_hint_weight_set(panes_h, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panes_h, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panes_h);

   evas_object_smart_callback_add(panes_h, "clicked", _clicked, panes_h);
   evas_object_smart_callback_add(panes_h, "clicked,double", _clicked_double, panes_h);

   evas_object_smart_callback_add(panes_h, "press", _press, panes_h);
   evas_object_smart_callback_add(panes_h, "unpress", _unpress, panes_h);
   elm_panes_content_right_set(panes, panes_h);
   //

   //
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Up");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);
   elm_panes_content_left_set(panes_h, bt);
   //

   //
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Down");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);
   elm_panes_content_right_set(panes_h, bt);
   //


   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}


#endif
