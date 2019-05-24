#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

#define TABLE_SIZE 10

static const char *images[] = { "plant_01.jpg", "rock_01.jpg",
                                "rock_02.jpg", "sky_01.jpg", "sky_04.jpg",
                                "wood_01.jpg", "fire.gif" };

static const int rnd[] = {
   3, 86, 81, 36, 38, 8, 65, 53, 93, 40, 71, 74, 90, 95, 88, 4, 96, 73, 14, 52,
   83, 77, 51, 13, 79, 15, 59, 99, 100, 31, 27, 24, 97, 34, 28, 58, 84, 92, 18,
   50, 22, 62, 12, 7, 17, 56, 2, 47, 75, 85, 42, 89, 80, 57, 44, 49, 5, 45, 54,
   68, 76, 70, 11, 69, 64, 30, 55, 29, 39, 60, 82, 63, 94, 43, 19, 26, 35, 21,
   67, 91, 46, 72, 33, 87, 10, 9, 41, 66, 37, 32, 78, 20, 23, 48, 6, 1, 25, 16,
   61, 98,
};

static const char *filter =
      "padding_set {0}"
      "fill { color = 'red' }"
      "blur { radius }"
      "print ('Evaluating filter: ' .. input.width .. 'x' .. input.height)"
      ;

static inline Eo *
_image_create(Eo *win, const char *path)
{
   return efl_add(EFL_UI_IMAGE_CLASS, win,
                  efl_file_set(efl_added, path),
                  efl_file_load(efl_added),
                  efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(20, 20)));
}

static void
_anim_toggle(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Iterator *it;
   Eo *win = data;
   Eo *table, *o;

   table = efl_key_wref_get(win, "table");
   it = efl_content_iterate(table);
   EINA_ITERATOR_FOREACH(it, o)
     {
        if (efl_isa(o, EFL_PLAYER_INTERFACE) && efl_player_playable_get(o))
          efl_player_play_set(o, !efl_player_play_get(o));
     }
   eina_iterator_free(it);
}

static void
_render_post(void *data, const Efl_Event *ev)
{
   Eo *snap = data;

   efl_event_callback_del(ev->object, EFL_CANVAS_SCENE_EVENT_RENDER_POST, _render_post, data);
   efl_file_save(snap, eina_slstr_printf("%s/snap-efl.png", eina_environment_tmp_get()), NULL, NULL);
}

static void
_save_image(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *snap;

   // Save is available only during render_post
   snap = efl_key_wref_get(win, "snap");
   efl_event_callback_add(win, EFL_CANVAS_SCENE_EVENT_RENDER_POST, _render_post, snap);

   // Force a render in order to ensure post_render is called. EO API provides
   // no way to do manual render, so we add a damage to the snapshot object.
   // This is a special case handled by snapshot for the purpose of taking
   // screenshots like this. This is useful only if the button click has no
   // animation on screen and there is no spinning wheel either.
   efl_gfx_buffer_update_add(snap, NULL);
}

static void
_radius_set(void *data, const Efl_Event *ev)
{
   char buf[128];
   Eo *win = data;
   Eo *snap;

   snap = efl_key_wref_get(win, "snap");
   sprintf(buf, "tonumber(%d)", (int) elm_slider_value_get(ev->object));
   efl_gfx_filter_data_set(snap, "radius", buf, EINA_TRUE);
}

static void
_close_do(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;

   efl_del(win);
}

static void
_toggle_map(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *snap;

   snap = efl_key_wref_get(win, "snap");
   if (!efl_gfx_mapping_has(snap))
     {
        efl_gfx_mapping_zoom(snap, 0.8, 0.8, NULL, 0.5, 0.5);
        efl_gfx_mapping_rotate(snap, 20.0, NULL, 0.5, 0.5);
     }
   else efl_gfx_mapping_reset(snap);
}

void
test_evas_snapshot(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *table, *o, *snap, *box;
   const char *path;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Evas Snapshot"),
                 efl_ui_win_autodel_set(efl_added, 1));

   table = efl_add(EFL_UI_TABLE_CLASS, win,
                  efl_gfx_arrangement_content_padding_set(efl_added, 5, 5, 1));
   efl_content_set(win, table);

   // Objects below the snapshot
   for (int c = 0; c < TABLE_SIZE; c++)
     for (int r = 0; r < TABLE_SIZE; r++)
       {
          int id = rnd[((r * TABLE_SIZE) + c) % EINA_C_ARRAY_LENGTH(rnd)]
                % EINA_C_ARRAY_LENGTH(images);

          path = eina_slstr_printf("%s/images/%s", elm_app_data_dir_get(), images[id]);
          o = _image_create(win, path);
          efl_pack_table(table, o, r, c, 1, 1);
       }
   _anim_toggle(win, NULL);

   // Snapshot
   snap = efl_add(EFL_CANVAS_SNAPSHOT_CLASS, win,
                  efl_gfx_filter_program_set(efl_added, filter, "filter"),
                  efl_gfx_filter_data_set(efl_added, "radius", "tonumber(5)", EINA_TRUE),
                  efl_pack_table(table, efl_added, 1, 1, TABLE_SIZE - 2, TABLE_SIZE - 2));

   // Objects above snapshot
   // 1. Opaque rect, not changing
   o = evas_object_rectangle_add(win);
   evas_object_color_set(o, 32, 32, 96, 255);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_pack_table(table, o, 2, TABLE_SIZE / 2, TABLE_SIZE - 4, TABLE_SIZE / 2 - 2);
   evas_object_show(o);

   // 2. Non-opaque animated object
   o = elm_progressbar_add(win);
   elm_object_style_set(o, "wheel");
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_align_set(o, 0.5, 0.5);
   elm_progressbar_pulse_set(o, EINA_TRUE);
   elm_progressbar_pulse(o, EINA_TRUE);
   evas_object_show(o);
   efl_pack_table(table, o, 0, 0, TABLE_SIZE, TABLE_SIZE);

   // Controls
   efl_add(EFL_UI_SLIDER_CLASS, win,
           elm_object_text_set(efl_added, "Blur radius: "),
           elm_slider_min_max_set(efl_added, 0, 64),
           elm_slider_value_set(efl_added, 5),
           elm_slider_unit_format_set(efl_added, "%.0f px"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack_table(table, efl_added, 0, TABLE_SIZE, TABLE_SIZE, 1),
           efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, _radius_set, win));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                 efl_gfx_hint_fill_set(efl_added, EINA_TRUE, EINA_TRUE),
                 efl_pack_table(table, efl_added, 0, TABLE_SIZE + 1, TABLE_SIZE, 1));

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Toggle animation"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _anim_toggle, win));

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Save to file"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _save_image, win));

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Map"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _toggle_map, win));

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Close"),
           efl_gfx_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _close_do, win));

   efl_key_wref_set(win, "snap", snap);
   efl_key_wref_set(win, "table", table);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(400,  400));
}
