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
   elm_panes_content_left_min_size_set(panes, 100);
   elm_panes_content_left_size_set(panes, 0.7);
   elm_win_resize_object_add(win, panes);
   evas_object_show(panes);

   evas_object_smart_callback_add(panes, "clicked", _clicked, panes);
   evas_object_smart_callback_add(panes, "clicked,double", _clicked_double, &vbar_size);

   evas_object_smart_callback_add(panes, "press", _press, panes);
   evas_object_smart_callback_add(panes, "unpress", _unpress, panes);

   // add left button
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left - min size 100 - size 70%");
   evas_object_show(bt);
   elm_object_part_content_set(panes, "left", bt);

   // add panes
   panes_h = elm_panes_add(win);
   elm_panes_horizontal_set(panes_h, EINA_TRUE);
   elm_panes_content_right_min_size_set(panes_h, 100);
   elm_panes_content_right_size_set(panes_h, 0.3);
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
   elm_object_text_set(bt, "Down - min size 100 size 30%");
   evas_object_show(bt);
   elm_object_part_content_set(panes_h, "bottom", bt);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

void
test_panes_minsize(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *panes, *panes_h;

   win = efl_add(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Panes"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE)
                );

   panes = efl_add(EFL_UI_PANES_CLASS, win,
                   efl_content_set(win, efl_added),
                   efl_ui_panes_split_ratio_set(efl_added, 0.7)
                  );

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Left - user set min size(110,110)"),
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(110, 110)),
           efl_content_set(efl_part(panes, "first"), efl_added)
          );

   panes_h = efl_add(EFL_UI_PANES_CLASS, win,
                     efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                     efl_content_set(efl_part(panes, "second"), efl_added)
                    );
   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Up - user set min size(10,0)"),
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(10, 0)),
           efl_content_set(efl_part(panes_h, "first"), efl_added)
          );
   efl_ui_panes_part_hint_min_allow_set(efl_part(panes_h, "first"), EINA_TRUE);

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Down - min size 50 40"),
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(50, 40)),
           efl_content_set(efl_part(panes_h, "second"), efl_added)
          );

   efl_gfx_size_set(win, EINA_SIZE2D(320,  400));
}

