#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

#define ICON_MAX 24

typedef enum
{
  BOX_PACK_POSITION_START,
  BOX_PACK_POSITION_BEFORE,
  BOX_PACK_POSITION_AFTER,
  BOX_PACK_POSITION_END
} Box_Pack_Position;

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   void *box;           /* Use this to get box content     */
};
typedef struct _api_data api_data;

enum _api_state
{
   BOX_PACK_START,
   BOX_PACK_BEFORE,
   BOX_PACK_AFTER,
   BOX_PADDING_SET,
   BOX_ALIGN_SET,
   BOX_HOMOGENEOUS_SET,
   BOX_UNPACK_ALL,
   BOX_CLEAR,
   API_STATE_LAST
};
typedef enum _api_state api_state;

static void
set_api_state(api_data *api)
{
   const Eina_List *items = elm_box_children_get(api->box);
   if (!eina_list_count(items))
     return;

   /* use elm_box_children_get() to get list of children */
   switch(api->state)
     { /* Put all api-changes under switch */
      case BOX_PACK_START:  /* Move last item to beginning */
         elm_box_unpack(api->box, eina_list_data_get(eina_list_last(items)));
         elm_box_pack_start(api->box, eina_list_data_get(eina_list_last(items)));
         break;

      case BOX_PACK_BEFORE:
         if (eina_list_count(items) > 1)
               {  /* Put last item before the one preceding it */
                  elm_box_unpack(api->box, eina_list_data_get(eina_list_last(items)));
                  elm_box_pack_before(api->box,
                        eina_list_data_get(eina_list_last(items)),
                        eina_list_nth(items, eina_list_count(items)-2));
               }
         break;

      case BOX_PACK_AFTER:
         if (eina_list_count(items) > 1)
               {  /* Put item before last to last */
                  elm_box_unpack(api->box, eina_list_nth(items,
                           eina_list_count(items)-2));
                  elm_box_pack_after(api->box,
                        eina_list_nth(items, eina_list_count(items)-2),
                        eina_list_data_get(eina_list_last(items)));
               }
         break;

      case BOX_PADDING_SET:
         elm_box_padding_set(api->box, 30, 15);
         break;

      case BOX_ALIGN_SET:
         elm_box_align_set(api->box, 0.25, 0.75);
         break;

      case BOX_HOMOGENEOUS_SET:
         elm_box_homogeneous_set(api->box, EINA_TRUE);
         break;

      case BOX_UNPACK_ALL:
           {
              Eina_List *l;
              Evas_Object *data;
              elm_box_unpack_all(api->box);
              EINA_LIST_REVERSE_FOREACH(items, l, data)
                 elm_box_pack_end(api->box, data);
           }
         break;

      case BOX_CLEAR:
         elm_box_clear(api->box);
         break;

      case API_STATE_LAST:

         break;
      default:
         return;
     }
}

static void
_api_bt_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{  /* Will add here a SWITCH command containing code to modify test-object */
   /* in accordance a->state value. */
   api_data *a = data;
   char str[128];

   printf("clicked event on API Button: api_state=<%d>\n", a->state);
   set_api_state(a);
   a->state++;
   sprintf(str, "Next API function (%u)", a->state);
   elm_object_text_set(obj, str);
   elm_object_disabled_set(obj, a->state == API_STATE_LAST);
}

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   free(data);
}

void
test_box_vert(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *ic, *bxx, *bt;
   char buf[PATH_MAX];
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("box-vert", "Box Vert");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   bx = elm_box_add(win);
   api->box = bx;
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   elm_box_pack_end(bxx, bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_01.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_02.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.0, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_03.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, EVAS_HINT_EXPAND, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   evas_object_show(win);
}

static void
_del_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   elm_box_unpack(data, obj);
   evas_object_move(obj, 0, 0);
   evas_object_color_set(obj, 128, 64, 0, 128);
   evas_object_del(obj);
}

void
test_box_vert2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt;

   win = elm_win_util_standard_add("box-vert2", "Box Vert 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   elm_box_padding_set(bx, 10, 10);
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
test_box_horiz(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *ic, *bxx, *bt;
   char buf[PATH_MAX];
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("box-horiz", "Box Horiz");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   bx = elm_box_add(win);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   api->box = bx;
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   elm_box_pack_end(bxx, bx);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_01.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.5);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_02.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
   evas_object_size_hint_align_set(ic, 0.5, 0.0);
   elm_box_pack_end(bx, ic);
   evas_object_show(ic);

   ic = elm_icon_add(win);
   snprintf(buf, sizeof(buf), "%s/images/icon_03.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
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
_index_remove(int i)
{
   if (i >= ICON_MAX) return;
   radio_index[i] = EINA_FALSE;
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
   int idx;
   char buf[PATH_MAX];
   Evas_Object *rdg;

   idx = _index_get();
   if (idx == -1) return NULL;

   ic = elm_icon_add(obj);
   snprintf(buf, sizeof(buf), "%s/images/icon_%02d.png",
            elm_app_data_dir_get(), idx);
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   rd = elm_radio_add(obj);
   elm_radio_state_value_set(rd, idx);
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
_pack_start_btn_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *rd;
   if (!data) return;

   rd = _radio_new(data);
   if (!rd) return;
   elm_box_pack_start(data, rd);
}

static void
_pack_before_btn_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
_pack_after_btn_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
_pack_end_btn_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *rd;
   if (!data) return;

   rd = _radio_new(data);
   if (!rd) return;
   elm_box_pack_end(data, rd);
}

static void
_unpack_btn_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
_unpack_all_btn_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
test_box_pack(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
_cb_check_changed(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Eina_Bool homo;
   Evas_Object *box = data;

   homo = elm_check_state_get(obj);
   elm_box_homogeneous_set(box, homo);
}

void
test_box_homo(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
_win_del(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Transitions_Data *tdata = data;
   if (!data) return;
   free(tdata);
}

void
test_box_transition(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt;
   Transitions_Data *tdata;

   win = elm_win_util_standard_add("box-transition", "Box Transition");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_smart_callback_add(bt, "clicked", _del_cb, bx);
   elm_box_pack_end(bx, bt);
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

   evas_object_smart_callback_add(win, "delete,request", _win_del, tdata);
   elm_box_layout_set(bx, evas_object_box_layout_horizontal, NULL, NULL);
   _test_box_transition_change(tdata);

   evas_object_resize(win, 300, 300);
   evas_object_show(win);
}

typedef struct _Box_Align_Data Box_Align_Data;
struct _Box_Align_Data
{
   Evas_Object *hor_box; // target box horizontal
   Evas_Object *vert_box; // target box vertical
   double hor; // horizontal slider
   double vert; // vertical slider
};

static void
_box_align_win_del_cb(void *data, Evas *e EINA_UNUSED,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   free(data);
}

static void
_hor_slider_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Box_Align_Data *bdata = data;

   bdata->hor = elm_slider_value_get(obj);
   printf("box align: %0.2f %0.2f\n", bdata->hor, bdata->vert);
   elm_box_align_set(bdata->hor_box, bdata->hor, bdata->vert);
   elm_box_align_set(bdata->vert_box, bdata->hor, bdata->vert);
}

static void
_vert_slider_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Box_Align_Data *bdata = data;

   bdata->vert = elm_slider_value_get(obj);
   printf("box align: %0.2f %0.2f\n", bdata->hor, bdata->vert);
   elm_box_align_set(bdata->hor_box, bdata->hor, bdata->vert);
   elm_box_align_set(bdata->vert_box, bdata->hor, bdata->vert);
}

void
_description_add(Evas_Object *bx_out, Box_Align_Data *bdata)
{
   Evas_Object *fr, *bx, *lb, *sl, *tb;

   // description
   fr = elm_frame_add(bx_out);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(fr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx_out, fr);
   elm_object_text_set(fr, "Description");
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   lb = elm_label_add(fr);
   elm_object_text_set(lb, "This test shows how elm_box_align_set() works.");
   evas_object_size_hint_align_set(lb, 0.0, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   tb = elm_table_add(bx);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, tb);
   evas_object_show(tb);

   lb = elm_label_add(tb);
   elm_object_text_set(lb, " Horizontal  ");
   elm_table_pack(tb, lb, 0, 0, 1, 1);
   evas_object_show(lb);

   lb = elm_label_add(tb);
   elm_object_text_set(lb, " Vertical  ");
   elm_table_pack(tb, lb, 0, 1, 1, 1);
   evas_object_show(lb);

   sl = elm_slider_add(fr);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.0, 1.0);
   elm_slider_value_set(sl, 0.5);
   bdata->hor = 0.5;
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, sl, 1, 0, 1, 1);
   evas_object_show(sl);
   evas_object_smart_callback_add(sl, "changed", _hor_slider_changed_cb, bdata);

   sl = elm_slider_add(fr);
   elm_slider_unit_format_set(sl, "%1.2f");
   elm_slider_min_max_set(sl, 0.0, 1.0);
   elm_slider_value_set(sl, 0.5);
   bdata->vert = 0.5;
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_table_pack(tb, sl, 1, 1, 1, 1);
   evas_object_show(sl);
   evas_object_smart_callback_add(sl, "changed", _vert_slider_changed_cb, bdata);
}

void
_align_box_add(Evas_Object *bx_out, Box_Align_Data *bdata)
{
   Evas_Object *bx, *sp, *bt;

   // test box - vertical
   bdata->vert_box = bx = elm_box_add(bx_out);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx_out, bx);
   elm_box_align_set(bx, 0.5, 0.5);
   evas_object_show(bx);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   // separator
   sp = elm_separator_add(bx_out);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   elm_box_pack_end(bx_out, sp);
   evas_object_show(sp);

   // test box - horizontal
   bdata->hor_box = bx = elm_box_add(bx_out);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx_out, bx);
   elm_box_align_set(bx, 0.5, 0.5);
   elm_box_horizontal_set(bx, EINA_TRUE);
   evas_object_show(bx);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Button 1");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(bx);
   elm_object_text_set(bt, "Button 2");
   evas_object_size_hint_weight_set(bt, 0.0, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);
}

void
test_box_align(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx_out;
   Box_Align_Data *bdata= (Box_Align_Data *)calloc(1, sizeof(Box_Align_Data));

   win = elm_win_util_standard_add("box-align", "Box Align");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_DEL,
                                  _box_align_win_del_cb, bdata);

   bx_out = elm_box_add(win);
   evas_object_size_hint_weight_set(bx_out, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx_out);
   evas_object_show(bx_out);

   _description_add(bx_out, bdata);
   _align_box_add(bx_out, bdata);

   evas_object_resize(win, 300, 400);
   evas_object_show(win);
}

void
test_box_stack(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
               void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *o;
   char buf[PATH_MAX];

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Efl.Ui.Box_Stack"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_color_set(efl_part(win, "background"), 24, 24, 64, 255);

   /* stacked box, with items in the center-bottom. its default weight makes
    * the window resizable */
   bx = efl_add(EFL_UI_BOX_STACK_CLASS, win,
                efl_pack_align_set(efl_added, 0.5, 1.0));
   efl_content_set(win, bx);

   /* stretched rectangle */
   o = efl_add(EFL_CANVAS_RECTANGLE_CLASS, win,
               efl_gfx_color_set(efl_added, 0, 64, 128, 255));
   efl_pack(bx, o);

   /* rectangle with a max size */
   o = efl_add(EFL_CANVAS_RECTANGLE_CLASS, win,
               efl_gfx_color_set(efl_added, 64, 128, 64, 255),
               efl_gfx_hint_size_max_set(efl_added, EINA_SIZE2D(128, 20)));
   efl_pack(bx, o);

   /* image with a forced min size */
   snprintf(buf, sizeof(buf), "%s/images/logo.png", elm_app_data_dir_get());
   o = efl_add(EFL_UI_IMAGE_CLASS, win,
               efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(64, 64)),
               efl_file_set(efl_added, buf),
               efl_file_load(efl_added));
   efl_pack(bx, o);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 300));
}
