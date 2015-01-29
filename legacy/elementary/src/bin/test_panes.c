#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_press(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("press\n");
}

static void
_unpress(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   printf("unpress, size : %f\n", elm_panes_content_left_size_get(obj));
}

static void
_clicked(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   printf("clicked\n");
}

static void
_clicked_double(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   double *size = data;
   double tmp_size = 0.0;

   tmp_size = elm_panes_content_left_size_get(obj);
   printf("clicked double\n");
   if (tmp_size > 0)
     {
        elm_panes_content_left_size_set(obj, 0.0);
        *size = tmp_size;
     }
   else
     elm_panes_content_left_size_set(obj, *size);
}

void
test_panes(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *panes, *panes_h, *bt;
   static double vbar_size = 0.0;
   static double hbar_size = 0.0;

   win = elm_win_add(NULL, "panes", ELM_WIN_BASIC);
   elm_win_title_set(win, "Panes");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   evas_object_show(bg);

   panes = elm_panes_add(win);
   evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, panes);
   evas_object_show(panes);

   evas_object_smart_callback_add(panes, "clicked", _clicked, panes);
   evas_object_smart_callback_add(panes, "clicked,double", _clicked_double, &vbar_size);

   evas_object_smart_callback_add(panes, "press", _press, panes);
   evas_object_smart_callback_add(panes, "unpress", _unpress, panes);

   // add left button
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left");
   evas_object_show(bt);
   elm_object_part_content_set(panes, "left", bt);

   // add panes
   panes_h = elm_panes_add(win);
   elm_panes_horizontal_set(panes_h, EINA_TRUE);
   evas_object_show(panes_h);

   evas_object_smart_callback_add(panes_h, "clicked", _clicked, panes_h);
   evas_object_smart_callback_add(panes_h, "clicked,double", _clicked_double, &hbar_size);

   evas_object_smart_callback_add(panes_h, "press", _press, panes_h);
   evas_object_smart_callback_add(panes_h, "unpress", _unpress, panes_h);
   elm_object_part_content_set(panes, "right", panes_h);

   // add up button
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Up");
   evas_object_show(bt);
   elm_object_part_content_set(panes_h, "top", bt);

   // add down button
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Down");
   evas_object_show(bt);
   elm_object_part_content_set(panes_h, "bottom", bt);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}
