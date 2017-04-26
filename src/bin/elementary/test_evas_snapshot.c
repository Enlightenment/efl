#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

#define GRID_SIZE 10

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

static inline void
_efl_key_int_set(Eo *obj, const char *key, int val)
{
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_INT);
   eina_value_set(v, val);
   efl_key_value_set(obj, key, v);
}

static inline int
_efl_key_int_get(Eo *obj, const char *key)
{
   Eina_Value *v = efl_key_value_get(obj, key);
   int val;

   if (!eina_value_get(v, &val)) return 0;
   return val;
}

static inline Eo *
_image_create(Eo *win, const char *path)
{
   return efl_add(EFL_UI_IMAGE_CLASS, win,
                  efl_file_set(efl_added, path, NULL),
                  efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
                  efl_gfx_size_hint_weight_set(efl_added, 1.0, 1.0),
                  efl_gfx_size_hint_min_set(efl_added, 20, 20),
                  efl_gfx_visible_set(efl_added, 1));
}

static void
_anim_toggle(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eina_Iterator *it;
   Eo *win = data;
   Eo *grid, *o;

   grid = efl_key_wref_get(win, "grid");
   it = efl_content_iterate(grid);
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

   efl_event_callback_del(ev->object, EFL_CANVAS_EVENT_RENDER_POST, _render_post, data);
   efl_file_save(snap, eina_slstr_printf("%s/snap-efl.png", eina_environment_tmp_get()), NULL, NULL);
}

static void
_save_image(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *snap;
   int w, h;

   // Save is available only during render_post
   snap = efl_key_wref_get(win, "snap");
   efl_event_callback_add(win, EFL_CANVAS_EVENT_RENDER_POST, _render_post, snap);

   // Force a render in order to ensure post_render is called. EO API provides
   // no way to do manual render, so we add a damage to the snapshot object.
   // This is a special case handled by snapshot for the purpose of taking
   // screenshots like this. This is useful only if the button click has no
   // animation on screen and there is no spinning wheel either.
   efl_gfx_size_get(snap, &w, &h);
   efl_gfx_buffer_update_add(snap, 0, 0, w, h);
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
_map_do(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *snap = data;
   int x, y, w, h;

   // Prevent recursive infinite loop :(
   static int here = 0;
   if (here) return;
   here = 1;

   efl_gfx_map_reset(snap);
   efl_gfx_geometry_get(snap, &x, &y, &w, &h);
   efl_gfx_map_zoom(snap, 0.8, 0.8, x + w/2., y + h/2.);
   efl_gfx_map_rotate(snap, 45., x + w/2., y + h/2.);
   efl_gfx_map_enable_set(snap, EINA_TRUE);

   here = 0;
}

static void
_toggle_map(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;
   Eo *snap;

   snap = efl_key_wref_get(win, "snap");
   if (!_efl_key_int_get(snap, "map"))
     {
        _efl_key_int_set(snap, "map", 1);
        _map_do(snap, NULL);
        efl_event_callback_add(snap, EFL_GFX_EVENT_RESIZE, _map_do, snap);
        efl_event_callback_add(snap, EFL_GFX_EVENT_MOVE, _map_do, snap);
     }
   else
     {
        _efl_key_int_set(snap, "map", 0);
        efl_event_callback_del(snap, EFL_GFX_EVENT_RESIZE, _map_do, snap);
        efl_event_callback_del(snap, EFL_GFX_EVENT_MOVE, _map_do, snap);
        efl_gfx_map_enable_set(snap, EINA_FALSE);
     }
}

void
test_evas_snapshot(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *grid, *o, *snap, *box;
   const char *path;

   win = efl_add(EFL_UI_WIN_STANDARD_CLASS, NULL,
                 efl_text_set(efl_added, "Evas Snapshot"),
                 efl_ui_win_autodel_set(efl_added, 1));

   grid = efl_add(EFL_UI_GRID_CLASS, win,
                  efl_pack_padding_set(efl_added, 5, 5, 1),
                  efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
                  efl_gfx_size_hint_weight_set(efl_added, 1.0, 1.0));
   efl_content_set(win, grid);

   // Objects below the snapshot
   for (int c = 0; c < GRID_SIZE; c++)
     for (int r = 0; r < GRID_SIZE; r++)
       {
          int id = rnd[((r * GRID_SIZE) + c) % EINA_C_ARRAY_LENGTH(rnd)]
                % EINA_C_ARRAY_LENGTH(images);

          path = eina_slstr_printf("%s/images/%s", elm_app_data_dir_get(), images[id]);
          o = _image_create(win, path);
          efl_pack_grid(grid, o, r, c, 1, 1);
       }
   _anim_toggle(win, NULL);

   // Snapshot
   snap = efl_add(EFL_CANVAS_SNAPSHOT_CLASS, win,
                  efl_gfx_filter_program_set(efl_added, filter, "filter"),
                  efl_gfx_filter_data_set(efl_added, "radius", "tonumber(5)", EINA_TRUE),
                  efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
                  efl_gfx_size_hint_weight_set(efl_added, 1.0, 1.0),
                  efl_pack_grid(grid, efl_added, 1, 1, GRID_SIZE - 2, GRID_SIZE - 2),
                  efl_gfx_visible_set(efl_added, 1));

   // Objects above snapshot
   // 1. Opaque rect, not changing
   o = evas_object_rectangle_add(win);
   evas_object_color_set(o, 32, 32, 96, 255);
   evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   efl_pack_grid(grid, o, 2, GRID_SIZE / 2, GRID_SIZE - 4, GRID_SIZE / 2 - 2);
   evas_object_show(o);

   // 2. Non-opaque animated object
   o = elm_progressbar_add(win);
   elm_object_style_set(o, "wheel");
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_align_set(o, 0.5, 0.5);
   elm_progressbar_pulse_set(o, EINA_TRUE);
   elm_progressbar_pulse(o, EINA_TRUE);
   evas_object_show(o);
   efl_pack_grid(grid, o, 0, 0, GRID_SIZE, GRID_SIZE);

   // Controls
   efl_add(ELM_SLIDER_CLASS, win,
           elm_object_text_set(efl_added, "Blur radius: "),
           elm_slider_min_max_set(efl_added, 0, 64),
           elm_slider_value_set(efl_added, 5),
           elm_slider_unit_format_set(efl_added, "%.0f px"),
           efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
           efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack_grid(grid, efl_added, 0, GRID_SIZE, GRID_SIZE, 1),
           efl_event_callback_add(efl_added, ELM_SLIDER_EVENT_CHANGED, _radius_set, win),
           efl_gfx_visible_set(efl_added, 1));

   box = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
                 efl_pack_grid(grid, efl_added, 0, GRID_SIZE + 1, GRID_SIZE, 1),
                 efl_gfx_visible_set(efl_added, 1));

   efl_add(ELM_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Toggle animation"),
           efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
           efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _anim_toggle, win),
           efl_gfx_visible_set(efl_added, 1));

   efl_add(ELM_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Save to file"),
           efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
           efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _save_image, win),
           efl_gfx_visible_set(efl_added, 1));

   efl_add(ELM_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Map"),
           efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
           efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _toggle_map, win),
           efl_gfx_visible_set(efl_added, 1));

   efl_add(ELM_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Close"),
           efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
           efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack(box, efl_added),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _close_do, win),
           efl_gfx_visible_set(efl_added, 1));

   efl_key_wref_set(win, "snap", snap);
   efl_key_wref_set(win, "grid", grid);
   efl_gfx_size_set(win, 400, 400);
   efl_gfx_visible_set(win, 1);


}
