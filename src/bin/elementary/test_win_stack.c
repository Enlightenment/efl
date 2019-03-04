#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

// Stack API is still beta (and EO only)
#define EFL_UI_WIN_BETA
#include <Elementary.h>

static int level = 0;
static Evas_Object *popto_win = NULL;

static void _bt_popto(void *data, Evas_Object *obj, void *event_info);
static void _bt_pressed(void *data, Evas_Object *obj, void *event_info);

static Evas_Object *
_win_new(Evas_Object *stack_top, const char *title)
{
   Evas_Object *bg, *bx, *bt, *lb, *win;

   if (level >= 3)
     win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                   efl_ui_win_name_set(efl_added, "window-stack"),
                   efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_NAVIFRAME_BASIC),
                   efl_text_set(efl_added, title),
                   efl_ui_win_autodel_set(efl_added, EINA_TRUE));
   else
     win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                   efl_ui_win_name_set(efl_added, "window-stack"),
                   efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_DIALOG_BASIC),
                   efl_text_set(efl_added, title),
                   efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   if (level == 3) popto_win = win;

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Press below to push another window on the stack");
   evas_object_size_hint_weight_set(lb, 1.0, 1.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   if (level > 7)
     {
        bt = elm_button_add(win);
        elm_object_text_set(bt, "Pop to level 3");
        evas_object_smart_callback_add(bt, "clicked", _bt_popto, NULL);
        evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);
     }

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Push");
   evas_object_smart_callback_add(bt, "clicked", _bt_pressed, stack_top);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 280, 400);
   return win;
}

static void
_bt_popto(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   efl_ui_win_stack_pop_to(popto_win);
}

static void
_bt_pressed(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win;
   char buf[100];

   level++;
   snprintf(buf, sizeof(buf), "Level %i", level);
   win = _win_new(data, buf);
   efl_ui_win_stack_master_id_set(win, efl_ui_win_stack_id_get(data));
}

static void
_del()
{
   level = 0;
   popto_win = NULL;
}

void
test_win_stack(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *bg, *bx, *bt, *lb, *win;

   win = elm_win_add(NULL, "window-stack", ELM_WIN_BASIC);
   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL, _del, NULL);
   efl_ui_win_stack_base_set(win, EINA_TRUE);
   elm_win_title_set(win, "Window Stack");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "Press below to push another window on the stack");
   evas_object_size_hint_weight_set(lb, 1.0, 1.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Push");
   evas_object_smart_callback_add(bt, "clicked", _bt_pressed, win);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
