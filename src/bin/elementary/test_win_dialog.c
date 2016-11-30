#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_bt_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *dia, *lb;
   Evas_Object *parent = data;

   dia = elm_win_util_dialog_add(parent, "window-dia", "A Dialog Window");
   elm_win_autodel_set(dia, EINA_TRUE);

   lb = elm_label_add(dia);
   elm_object_text_set(lb, "This is a Dialog Window");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(dia, lb);
   evas_object_show(lb);

   evas_object_resize(dia, 200, 150);
   evas_object_show(dia);
}

static void
_bt2_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Canvas_Object *dia, *lb, *parent = data;

   dia = efl_add(EFL_UI_WIN_STANDARD_CLASS, parent,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_DIALOG_BASIC),
                 efl_ui_win_name_set(efl_added, "window-dia-2"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE),
                 efl_text_set(efl_added, "A Fixed Size Dialog"));

   // Notes:
   // - All objects have 0 weight, this makes the window not resizable.
   // - With EO APIs, efl_gfx_size_hint_min_set() is safe to call as EFL will
   //   only set the restricted min size (and combine them for calculation).

   lb = efl_add(EFL_UI_TEXT_CLASS, dia,
                efl_text_set(efl_added, "This is a non-resizable dialog."),
                efl_gfx_size_hint_min_set(efl_added, 300, 150),
                efl_gfx_size_hint_max_set(efl_added, 300, 150),
                efl_gfx_size_hint_weight_set(efl_added, 0, 0));

   // Swallow in the label as the default content, this will make it visible.
   efl_content_set(dia, lb);

   efl_gfx_visible_set(dia, EINA_TRUE);
}

static void
_size_update(void *data, const Efl_Event *ev)
{
   int w, h, W, H, wc, hc, ww, hh;
   char buf[2048];
   Eo *dia = ev->object;
   Eo *lbl = data;

   efl_gfx_size_hint_combined_min_get(dia, &wc, &hc);
   efl_gfx_size_hint_min_get(dia, &w, &h);
   efl_gfx_size_hint_max_get(dia, &W, &H);
   efl_gfx_size_get(dia, &ww, &hh);

   sprintf(buf, "This is a dialog with min/max size<br>"
           "Min size: %dx%d (requested) %dx%d (effective)<br>"
           "Max size: %dx%d<br>"
           "Current size: %dx%d", w, h, wc, hc, W, H, ww, hh);
   //efl_text_set(lbl, buf);
   elm_object_text_set(lbl, buf);
}

static void
_bt3_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Efl_Canvas_Object *dia, *lb, *parent = data;

   dia = efl_add(EFL_UI_WIN_STANDARD_CLASS, parent,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_DIALOG_BASIC),
                 efl_ui_win_name_set(efl_added, "window-dia-3"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE),
                 efl_text_set(efl_added, "Min/Max Size Dialog"));

   //lb = efl_add(EFL_UI_TEXT_CLASS, dia);,
   lb = elm_label_add(dia);
   efl_event_callback_add(dia, EFL_GFX_EVENT_CHANGE_SIZE_HINTS, _size_update, lb);
   efl_event_callback_add(dia, EFL_GFX_EVENT_RESIZE, _size_update, lb);
   elm_object_text_set(lb, "This is a Dialog Window");
   efl_gfx_size_hint_weight_set(lb, 1.0, 1.0);

   // Swallow in the label as the default content, this will make it visible.
   efl_content_set(dia, lb);

   // Set min & max size (app-side)
   efl_gfx_size_hint_min_set(dia, ELM_SCALE_SIZE(0), ELM_SCALE_SIZE(100));
   efl_gfx_size_hint_max_set(dia, ELM_SCALE_SIZE(800), ELM_SCALE_SIZE(600));

   efl_gfx_visible_set(dia, EINA_TRUE);
}

void
test_win_dialog(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *bt;

   win = elm_win_util_standard_add("window-dia", "A Standard Window");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Create a new dialog");
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked_cb, win);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Create a new unresizable dialog");
   evas_object_smart_callback_add(bt, "clicked", _bt2_clicked_cb, win);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Create a new dialog with min/max sizes");
   evas_object_smart_callback_add(bt, "clicked", _bt3_clicked_cb, win);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
