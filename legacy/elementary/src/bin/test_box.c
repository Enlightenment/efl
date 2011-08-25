#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
void
test_box_vert(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "box-vert", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Vert");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.0, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, EVAS_HINT_EXPAND, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   evas_object_show(win);
}

static void
_del_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   elm_box_unpack(data, obj);
   evas_object_move(obj, 0, 0);
   evas_object_color_set(obj, 128, 64, 0, 128);
//   evas_object_del(obj);
}

void
test_box_vert2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt;

   win = elm_win_add(NULL, "box-vert2", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Vert 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   elm_box_padding_set(bx, 10, 10);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 4");
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 5");
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}

void
test_box_horiz(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *ic;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "box-horiz", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Horiz");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_icon_file_set(ic, buf, NULL);
   elm_icon_scale_set(ic, 0, 0);
   evas_object_size_hint_align_set(ic, 0.0, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   evas_object_show(win);
}

typedef struct
{
   Eina_List *transitions;
   Evas_Object *box;
   Evas_Object_Box_Layout last_layout;
} Transitions_Data;

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
   elm_box_layout_set(tdata->box, elm_box_layout_transition, layout_data, elm_box_transition_free);
   tdata->last_layout = next_layout;

   tdata->transitions = eina_list_demote_list(tdata->transitions, tdata->transitions);
}

static void
_win_del(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Transitions_Data *tdata = data;
   if (!data) return;
   free(tdata);
}

void
test_box_transition(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt;
   Transitions_Data *tdata;

   win = elm_win_add(NULL, "box-transition", ELM_WIN_BASIC);
   elm_win_title_set(win, "Box Transition");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   elm_win_resize_object_add(win, bx);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);

   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_resize(bt, 100, 100);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_resize(bt, 100, 100);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   evas_object_size_hint_weight_set(bt, 1.0, 1.0);
   evas_object_size_hint_align_set(bt, -1.0, -1.0);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_resize(bt, 100, 100);
   evas_object_show(bt);

   tdata = calloc(1, sizeof(Transitions_Data));
   tdata->box = bx;
   tdata->last_layout = evas_object_box_layout_horizontal;
   tdata->transitions = eina_list_append(tdata->transitions,
         evas_object_box_layout_vertical);
   tdata->transitions = eina_list_append(tdata->transitions,
         evas_object_box_layout_horizontal);
   tdata->transitions = eina_list_append(tdata->transitions,
         evas_object_box_layout_stack);
   tdata->transitions = eina_list_append(tdata->transitions,
         evas_object_box_layout_homogeneous_vertical);
   tdata->transitions = eina_list_append(tdata->transitions,
         evas_object_box_layout_homogeneous_horizontal);
   tdata->transitions = eina_list_append(tdata->transitions,
         evas_object_box_layout_flow_vertical);
   tdata->transitions = eina_list_append(tdata->transitions,
         evas_object_box_layout_flow_horizontal);
   tdata->transitions = eina_list_append(tdata->transitions,
         evas_object_box_layout_stack);

   evas_object_resize(win, 300, 300);
   evas_object_resize(bx, 300, 300);
   evas_object_smart_callback_add(win, "delete,request", _win_del, tdata);
   evas_object_show(win);
   elm_box_layout_set(bx, evas_object_box_layout_horizontal, NULL, NULL);
   _test_box_transition_change(tdata);
}
#endif
