/**
 * Simple Elementary's <b>mapbuf widget</b> example, illustrating its API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g `pkg-config --cflags --libs elementary` mapbuf_example.c -o mapbuf_example
 * @endverbatim
 */

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__
#endif

static void
_on_done(void *data __UNUSED__,
         Evas_Object *obj __UNUSED__,
         void *event_info __UNUSED__)
{
   elm_exit();
}

static void
_enabled_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *mb = data;
   elm_mapbuf_enabled_set(mb, elm_toggle_state_get(obj));
}

static void
_alpha_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *mb = data;
   elm_mapbuf_alpha_set(mb, elm_check_state_get(obj));
}

static void
_smooth_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *mb = data;
   elm_mapbuf_smooth_set(mb, elm_check_state_get(obj));
}

EAPI_MAIN int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *hbx, *tg, *ck, *mb, *tb, *ic;
   unsigned int i, j;

   win = elm_win_add(NULL, "mapbuf", ELM_WIN_BASIC);
   elm_win_title_set(win, "Mapbuf Example");
   evas_object_smart_callback_add(win, "delete,request", _on_done, NULL);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   mb = elm_mapbuf_add(win);
   evas_object_size_hint_weight_set(mb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(mb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, mb);

   tb = elm_table_add(win);
   evas_object_size_hint_weight_set(tb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(tb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(tb);

   for (i = 0; i < 8; i++)
     {
        for (j = 0; j < 8; j++)
          {
             ic = elm_icon_add(win);
             elm_icon_standard_set(ic, "home");
             elm_icon_scale_set(ic, EINA_FALSE, EINA_FALSE);
             evas_object_show(ic);
             elm_table_pack(tb, ic, i, j, 1, 1);
          }
     }

   elm_mapbuf_content_set(mb, tb);
   evas_object_show(mb);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   tg = elm_toggle_add(win);
   elm_object_text_set(tg, "Map");
   elm_toggle_states_labels_set(tg, "Enabled", "Disabled");
   evas_object_smart_callback_add(tg, "changed", _enabled_cb, mb);
   elm_box_pack_end(hbx, tg);
   evas_object_show(tg);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Alpha");
   elm_check_state_set(ck, EINA_TRUE);
   evas_object_smart_callback_add(ck, "changed", _alpha_cb, mb);
   elm_box_pack_end(hbx, ck);
   evas_object_show(ck);

   ck = elm_check_add(win);
   elm_object_text_set(ck, "Smooth");
   elm_check_state_set(ck, EINA_TRUE);
   evas_object_smart_callback_add(ck, "changed", _smooth_cb, mb);
   elm_box_pack_end(hbx, ck);
   evas_object_show(ck);

   evas_object_resize(win, 240, 320);
   evas_object_show(win);

   elm_run();
   return 0;
}
ELM_MAIN()
