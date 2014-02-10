#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

typedef struct _App_Data App_Data;

struct _App_Data
{
   Eina_Bool    wm_rot_supported;
   Eina_List   *chs;
   int          available_rots[4];
   Evas_Object *lb;
   Evas_Object *rdg;
};

static void
_bt_available_rots_set(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = data;
   App_Data *ad = evas_object_data_get(win, "ad");
   Evas_Object *o;
   Eina_List *l;
   const char *str;
   unsigned int i = 0;

   if (!ad->wm_rot_supported) return;

   EINA_LIST_FOREACH(ad->chs, l, o)
     {
        if (!elm_check_state_get(o)) continue;
        str = elm_object_text_get(o);
        if (!str) continue;
        ad->available_rots[i] = atoi(str);
        i++;
     }

   elm_win_wm_rotation_available_rotations_set
     (win, ad->available_rots, i);
}

static void
_bt_preferred_rot_set(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win = (Evas_Object *)(data);
   App_Data *ad = evas_object_data_get(win, "ad");

   if (!ad->wm_rot_supported) return;

   Evas_Object *rd = elm_radio_selected_object_get(ad->rdg);
   if (rd)
     {
        const char *str = elm_object_text_get(rd);
        int rot = 0;

        if (!strcmp(str, "Unset"))
          rot = -1;
        else
          rot = atoi(str);

        elm_win_wm_rotation_preferred_rotation_set(win, rot);
     }
}

static void
_win_wm_rotation_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *win = data;
   App_Data *ad = evas_object_data_get(win, "ad");
   int rot = elm_win_rotation_get(win);
   char buf[32];

   if (!ad->wm_rot_supported) return;

   eina_convert_itoa(rot, buf);
   elm_object_text_set(ad->lb, eina_stringshare_add(buf));
}

static void
_win_del_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   App_Data *ad = evas_object_data_get(obj, "ad");
   Evas_Object *o;

   if (ad->wm_rot_supported)
     {
        EINA_LIST_FREE(ad->chs, o)
          evas_object_data_del(o, "rotation");
     }

   free(ad);
}

void
test_win_wm_rotation(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   App_Data *ad;
   Evas_Object *win, *bx, *bx2, *lb, *ch, *bt, *en, *rd, *rdg = NULL;
   int i;
   char buf[32];

   if (!(ad = calloc(1, sizeof(App_Data)))) return;

   win = elm_win_util_standard_add("wmrotation", "WMRotation");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_data_set(win, "ad", ad);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "<b>Window manager rotation test</b>");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   ad->wm_rot_supported = elm_win_wm_rotation_supported_get(win);
   if (ad->wm_rot_supported)
     {
        int rots[] = { 0, 90, 270 };
        elm_win_wm_rotation_available_rotations_set(win, rots, (sizeof(rots) / sizeof(int)));
        elm_win_wm_rotation_preferred_rotation_set(win, 90);

        bx2 = elm_box_add(win);
        evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.0);
        elm_box_align_set(bx2, 0.0, 0.5);
        elm_box_horizontal_set(bx2, EINA_TRUE);
        elm_box_pack_end(bx, bx2);
        evas_object_show(bx2);

        for (i = 0; i < 4; i++)
          {
             ch = elm_check_add(win);
             eina_convert_itoa((i * 90), buf);
             elm_object_text_set(ch, eina_stringshare_add(buf));
             evas_object_size_hint_weight_set(ch, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             elm_box_pack_end(bx2, ch);
             evas_object_show(ch);

             if (i != 2) elm_check_state_set(ch, EINA_TRUE);

             ad->chs = eina_list_append(ad->chs, ch);
          }

        bt = elm_button_add(win);
        elm_object_text_set(bt, "Available rotations");
        evas_object_smart_callback_add(bt, "clicked", _bt_available_rots_set, win);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);

        bx2 = elm_box_add(win);
        evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.0);
        elm_box_align_set(bx2, 0.0, 0.5);
        elm_box_horizontal_set(bx2, EINA_TRUE);
        elm_box_pack_end(bx, bx2);
        evas_object_show(bx2);

        for (i = 0; i < 5; i++)
          {
             rd = elm_radio_add(win);
             if (!rdg) rdg = rd;
             elm_radio_state_value_set(rd, i);
             elm_radio_group_add(rd, rdg);

             if (i == 0)
               elm_object_text_set(rd, "Unset");
             else
               {
                  eina_convert_itoa(((i - 1) * 90), buf);
                  elm_object_text_set(rd, eina_stringshare_add(buf));
               }

             evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
             elm_box_pack_end(bx2, rd);
             evas_object_show(rd);
          }

        elm_radio_value_set(rdg, 2);
        ad->rdg = rdg;

        bt = elm_button_add(win);
        elm_object_text_set(bt, "Preferred rotation");
        evas_object_smart_callback_add(bt, "clicked", _bt_preferred_rot_set, win);
        elm_box_pack_end(bx, bt);
        evas_object_show(bt);

        evas_object_smart_callback_add(win, "wm,rotation,changed", _win_wm_rotation_changed_cb, win);
     }
   else
     printf("Window manager doesn't support rotation\n");

   en = elm_entry_add(win);
   elm_entry_single_line_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "N/A");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);
   ad->lb = lb;

   evas_object_smart_callback_add(win, "delete,request", _win_del_cb, NULL);

   evas_object_resize(win, 480, 400);
   evas_object_show(bx);
   evas_object_show(win);
}
