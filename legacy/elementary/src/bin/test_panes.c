#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static Elm_Genlist_Item_Class itc;

static void
_press(void *data, Evas_Object *obj, void *event_info)
{
    printf("press\n");
}

static void
_unpress(void *data, Evas_Object *obj, void *event_info)
{
    printf("unpress\n");
}

static void
_clicked(void *data, Evas_Object *obj, void *event_info)
{
    printf("clicked\n");
}

void
test_panes(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *panes, *bt;
   char buf[PATH_MAX];
   Evas_Object *list;

   win = elm_win_add(NULL, "panes", ELM_WIN_BASIC);
   elm_win_title_set(win, "Panes");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   panes = elm_panes_add(win);
   elm_win_resize_object_add(win, panes);
   evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(panes, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(panes);

   evas_object_smart_callback_add(panes, "clicked", _clicked, panes);
   evas_object_smart_callback_add(panes, "press", _press, panes);
   evas_object_smart_callback_add(panes, "unpress", _unpress, panes);


   bt = elm_button_add(win);
   elm_button_label_set(bt, "Left");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_show(bt);
   elm_panes_content_left_set(panes, bt);

   bt = elm_button_add(win);
   elm_button_label_set(bt, "Right");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bt);
   elm_panes_content_right_set(panes, bt);


   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}


#endif
