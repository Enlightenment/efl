#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

typedef struct
{
   Eina_List *transitions;
   Evas_Object *box;
   Evas_Object_Box_Layout last_layout;
} Transitions_Data;

static void
_add_cb(void *data, Evas_Object *obj __UNUSED__, void *ev __UNUSED__)
{
   Evas_Object *btn;
   Eina_List *children;
   Transitions_Data *tdata = data;

   btn = elm_button_add(tdata->box);
   elm_object_text_set(btn, "I do nothing");
   children = (Eina_List *)elm_box_children_get(tdata->box);
   if (children)
     {
        elm_box_pack_after(tdata->box, btn, (Evas_Object *)children->data);
        eina_list_free(children);
     }
   else
     elm_box_pack_end(tdata->box, btn);
   evas_object_show(btn);
}

static void
_clear_cb(void *data, Evas_Object *obj __UNUSED__, void *ev __UNUSED__)
{
   Transitions_Data *tdata = data;
   elm_box_clear(tdata->box);
}

static void
_unpack_cb(void *data, Evas_Object *obj, void *ev __UNUSED__)
{
   Transitions_Data *tdata = data;
   elm_box_unpack(tdata->box, obj);
   evas_object_move(obj, 0, 50);
   evas_object_color_set(obj, 128, 64, 0, 128);
}

static void
_test_box_transition_change(void *data)
{
   Transitions_Data *tdata = data;
   Elm_Box_Transition *layout_data;
   Evas_Object_Box_Layout next_layout;

   if (!data) return;
   next_layout = eina_list_data_get(tdata->transitions);
   layout_data = elm_box_transition_new(2.0, tdata->last_layout,
                                        NULL, NULL, next_layout, NULL, NULL,
                                        _test_box_transition_change, tdata);
   elm_box_layout_set(tdata->box, elm_box_layout_transition, layout_data,
                      elm_box_transition_free);
   tdata->last_layout = next_layout;

   tdata->transitions = eina_list_demote_list(tdata->transitions,
                                              tdata->transitions);
}

int
elm_main(int argc __UNUSED__, char *argv[] __UNUSED__)
{
   Evas_Object *win, *bg, *bigbox, *bx, *bt;
   static Transitions_Data tdata = {
        .transitions = NULL,
        .box = NULL,
        .last_layout = NULL
   };

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_add(NULL, "box-transition", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Transition");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_resize(win, 300, 320);
   evas_object_show(win);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bigbox = elm_box_add(win);
   evas_object_size_hint_weight_set(bigbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bigbox);
   evas_object_show(bigbox);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_box_pack_end(bigbox, bx);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Add");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _add_cb, &tdata);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clear");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
   evas_object_smart_callback_add(bt, "clicked", _clear_cb, &tdata);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bigbox, bx);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   evas_object_smart_callback_add(bt, "clicked", _unpack_cb, &tdata);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, 1.0, 0.5);
   evas_object_smart_callback_add(bt, "clicked", _unpack_cb, &tdata);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   evas_object_smart_callback_add(bt, "clicked", _unpack_cb, &tdata);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   tdata.box = bx;
   tdata.last_layout = evas_object_box_layout_horizontal;
   tdata.transitions = eina_list_append(tdata.transitions,
         evas_object_box_layout_vertical);
   tdata.transitions = eina_list_append(tdata.transitions,
         evas_object_box_layout_horizontal);
   tdata.transitions = eina_list_append(tdata.transitions,
         evas_object_box_layout_stack);
   tdata.transitions = eina_list_append(tdata.transitions,
         evas_object_box_layout_homogeneous_vertical);
   tdata.transitions = eina_list_append(tdata.transitions,
         evas_object_box_layout_homogeneous_horizontal);
   tdata.transitions = eina_list_append(tdata.transitions,
         evas_object_box_layout_flow_vertical);
   tdata.transitions = eina_list_append(tdata.transitions,
         evas_object_box_layout_flow_horizontal);
   tdata.transitions = eina_list_append(tdata.transitions,
         evas_object_box_layout_stack);

   elm_box_layout_set(bx, evas_object_box_layout_horizontal, NULL, NULL);
   _test_box_transition_change(&tdata);

   elm_run();

   return 0;
}
ELM_MAIN();
