/**
 * Simple Elementary's <b>mapbuf widget</b> example, illustrating its API.
 *
 * See stdout/stderr for output. Compile with:
 *
 * @verbatim
 * gcc -g mapbuf_example.c -o mapbuf_example `pkg-config --cflags --libs elementary`
 * @endverbatim
 */

#include <Elementary.h>

static void
_enabled_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *mb = data;
   elm_mapbuf_enabled_set(mb, elm_check_state_get(obj));
}

static void
_alpha_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *mb = data;
   elm_mapbuf_alpha_set(mb, elm_check_state_get(obj));
}

static void
_smooth_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *mb = data;
   elm_mapbuf_smooth_set(mb, elm_check_state_get(obj));
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Evas_Object *win, *bx, *hbx, *tg, *ck, *mb, *tb, *ic;
   unsigned int i, j;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   win = elm_win_util_standard_add("mapbuf", "Mapbuf Example");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
             elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);
             evas_object_show(ic);
             elm_table_pack(tb, ic, i, j, 1, 1);
          }
     }

   elm_object_content_set(mb, tb);
   evas_object_show(mb);

   hbx = elm_box_add(win);
   elm_box_horizontal_set(hbx, EINA_TRUE);
   evas_object_size_hint_weight_set(hbx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, hbx);
   evas_object_show(hbx);

   tg = elm_check_add(win);
   elm_object_style_set(tg, "toggle");
   elm_object_part_text_set(tg, "on", "Enabled");
   elm_object_part_text_set(tg, "off", "Disabled");
   elm_object_text_set(tg, "Map");
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
