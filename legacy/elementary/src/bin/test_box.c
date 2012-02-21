#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

#define ICON_MAX 24

typedef enum
{
  BOX_PACK_POSITION_START,
  BOX_PACK_POSITION_BEFORE,
  BOX_PACK_POSITION_AFTER,
  BOX_PACK_POSITION_END
} Box_Pack_Position;

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

static Eina_Bool radio_index[ICON_MAX];

static int
_index_get(void)
{
   int i;
   for (i = 0; i < ICON_MAX; i++)
     {
        if (!radio_index[i])
          {
             radio_index[i] = EINA_TRUE;
             return i;
          }

     }
   return -1;
}

static void
_index_remove(int index)
{
   if (index >= ICON_MAX) return;
   radio_index[index] = EINA_FALSE;
}

static void
_index_clear()
{
   int i;
   for (i = 0; i < ICON_MAX; i++)
     radio_index[i] = EINA_FALSE;
}

static Evas_Object *
_radio_new(Evas_Object *obj)
{
   Evas_Object *ic, *rd;
   int index;
   char buf[PATH_MAX];
   Evas_Object *rdg;

   index = _index_get();
   if (index == -1) return NULL;

   ic = elm_icon_add(obj);
   snprintf(buf, sizeof(buf), "%s/images/icon_%02d.png",
            elm_app_data_dir_get(), index);
   elm_icon_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   rd = elm_radio_add(obj);
   elm_radio_state_value_set(rd, index);
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rd, EVAS_HINT_FILL, 0.5);
   elm_object_part_content_set(rd, "icon", ic);

   rdg = evas_object_data_get(obj, "radio-group");
   if (rdg) elm_radio_group_add(rd, rdg);

   evas_object_show(rd);
   evas_object_show(ic);

   return rd;
}

static void
_pack_start_btn_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *rd;
   if (!data) return;

   rd = _radio_new(data);
   if (!rd) return;
   elm_box_pack_start(data, rd);
}

static void
_pack_before_btn_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *rd, *selected_rd, *rdg;
   if (!data) return;

   rdg = evas_object_data_get(data, "radio-group");
   if (!rdg) return;

   selected_rd = elm_radio_selected_object_get(rdg);
   if (!selected_rd || (selected_rd == rdg)) return;

   rd = _radio_new(data);
   if (!rd) return;
   elm_box_pack_before(data, rd, selected_rd);
}

static void
_pack_after_btn_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *rd, *selected_rd, *rdg;
   if (!data) return;

   rdg = evas_object_data_get(data, "radio-group");
   if (!rdg) return;

   selected_rd = elm_radio_selected_object_get(rdg);
   if (!selected_rd || (selected_rd == rdg)) return;

   rd = _radio_new(data);
   if (!rd) return;
   elm_box_pack_after(data, rd, selected_rd);
}

static void
_pack_end_btn_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *rd;
   if (!data) return;

   rd = _radio_new(data);
   if (!rd) return;
   elm_box_pack_end(data, rd);
}

static void
_unpack_btn_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *rdg, *selected_rd;
   if (!data) return;

   rdg = evas_object_data_get(data, "radio-group");
   if (!rdg) return;

   selected_rd = elm_radio_selected_object_get(rdg);
   if (selected_rd == rdg) return;

   _index_remove(elm_radio_value_get(selected_rd));
   elm_box_unpack(data, selected_rd);
   evas_object_del(selected_rd);
   elm_radio_value_set(rdg, -1);
}

static void
_unpack_all_btn_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *radio, *rdg;
   Eina_List *rl, *l;
   if (!data) return;

   rdg = evas_object_data_get(data, "radio-group");
   if (!rdg) return;

   _index_clear();
   rl = elm_box_children_get(data);
   EINA_LIST_FOREACH(rl, l, radio)
     {
        if (radio != rdg)
          evas_object_del(radio);
     }
   elm_box_unpack_all(data);
   elm_radio_value_set(rdg, -1);
}

void
test_box_pack(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *sc, *bt;
   Evas_Object *box, *lbox, *rbox;
   Evas_Object *rd, *rdg = NULL;
   int i;

   win = elm_win_util_standard_add("box-pack", "Box Pack");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_TRUE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   // left side
   lbox = elm_box_add(win);
   evas_object_size_hint_weight_set(lbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(lbox, EVAS_HINT_FILL, EVAS_HINT_FILL);

   sc = elm_scroller_add(win);
   elm_scroller_bounce_set(sc, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set(sc, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sc, EVAS_HINT_FILL, EVAS_HINT_FILL);

   rdg = elm_radio_add(win); /* a radio for easy group handling */
   elm_radio_state_value_set(rdg, -1);
   evas_object_data_set(lbox, "radio-group", rdg);

   for (i = 0; i < 3; i++)
     {
        rd = _radio_new(lbox);
        elm_box_pack_end(lbox, rd);
     }

   elm_object_content_set(sc, lbox);
   evas_object_show(lbox);
   elm_box_pack_end(box, sc);
   evas_object_show(sc);

   // right side
   rbox = elm_box_add(win);
   evas_object_size_hint_weight_set(rbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(rbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(rbox);
   elm_box_pack_end(box, rbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pack Start");
   evas_object_show(bt);
   elm_box_pack_end(rbox, bt);
   evas_object_smart_callback_add(bt, "clicked", _pack_start_btn_cb, lbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pack Before");
   evas_object_show(bt);
   elm_box_pack_end(rbox, bt);
   evas_object_smart_callback_add(bt, "clicked", _pack_before_btn_cb, lbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pack After");
   evas_object_show(bt);
   elm_box_pack_end(rbox, bt);
   evas_object_smart_callback_add(bt, "clicked", _pack_after_btn_cb, lbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Pack End");
   evas_object_show(bt);
   elm_box_pack_end(rbox, bt);
   evas_object_smart_callback_add(bt, "clicked", _pack_end_btn_cb, lbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Unpack");
   evas_object_show(bt);
   elm_box_pack_end(rbox, bt);
   evas_object_smart_callback_add(bt, "clicked", _unpack_btn_cb, lbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Unpack All");
   evas_object_show(bt);
   elm_box_pack_end(rbox, bt);
   evas_object_smart_callback_add(bt, "clicked", _unpack_all_btn_cb, lbox);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

static void
_cb_check_changed(void *data, Evas_Object *obj, void *event __UNUSED__)
{
   Eina_Bool homo;
   Evas_Object *box = data;

   homo = elm_check_state_get(obj);
   elm_box_homogeneous_set(box, homo);
}

void
test_box_homo(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win;
   Evas_Object *box, *o_bg;
   Evas_Object *rd;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("box-homogeneous", "Box Homogeneous");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   o_bg = elm_bg_add(win);
   snprintf(buf, sizeof(buf), "%s/images/twofish.jpg", elm_app_data_dir_get());
   elm_bg_file_set(o_bg, buf, NULL);
   evas_object_size_hint_weight_set(o_bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(o_bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, o_bg);

   rd = elm_check_add(win);
   elm_object_text_set(rd, "Homogeneous");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_check_changed, box);
   elm_box_pack_end(box, rd);
   evas_object_show(rd);
   evas_object_show(o_bg);

   evas_object_resize(win, 320, 320);
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
