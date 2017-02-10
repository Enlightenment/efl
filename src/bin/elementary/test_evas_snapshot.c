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
      "fill { color = '#0123' }"
      "blur { 5 }"
      "print ('Evaluating filter: ' .. input.width .. 'x' .. input.height)"
      ;

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
        if (efl_player_playable_get(o))
          efl_player_play_set(o, !efl_player_play_get(o));
     }
   eina_iterator_free(it);
}

static void
_close(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Eo *win = data;

   efl_del(win);
}

void
test_evas_snapshot(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *grid, *o, *snap;
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
          if (efl_player_playable_get(o))
            efl_player_play_set(o, 1);
       }

   // Snapshot
   snap = efl_add(EFL_CANVAS_SNAPSHOT_CLASS, win,
                  efl_gfx_filter_program_set(efl_added, filter, "filter"),
                  efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
                  efl_gfx_size_hint_weight_set(efl_added, 1.0, 1.0),
                  efl_pack_grid(grid, efl_added, 1, 1, GRID_SIZE - 2, GRID_SIZE - 2),
                  efl_gfx_visible_set(efl_added, 1));


   // Objects above snapshot
   o = elm_progressbar_add(win);
   elm_object_style_set(o, "wheel");
   evas_object_size_hint_weight_set(o, 0.0, 0.0);
   evas_object_size_hint_align_set(o, 0.5, 0.5);
   elm_progressbar_pulse_set(o, EINA_TRUE);
   elm_progressbar_pulse(o, EINA_TRUE);
   evas_object_show(o);
   efl_pack_grid(grid, o, 0, 0, GRID_SIZE, GRID_SIZE);

   // Controls
   efl_add(ELM_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Toggle animation"),
           efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
           efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack_grid(grid, efl_added, 0, GRID_SIZE, (GRID_SIZE + 1) / 2, 1),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _anim_toggle, win),
           efl_gfx_visible_set(efl_added, 1));

   efl_add(ELM_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Close"),
           efl_gfx_size_hint_align_set(efl_added, -1.0, -1.0),
           efl_gfx_size_hint_weight_set(efl_added, 1.0, 0.0),
           efl_pack_grid(grid, efl_added, (GRID_SIZE + 1) / 2, GRID_SIZE, (GRID_SIZE + 1) / 2, 1),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _close, win),
           efl_gfx_visible_set(efl_added, 1));

   efl_key_wref_set(win, "snap", snap);
   efl_key_wref_set(win, "grid", grid);
   efl_gfx_size_set(win, 400, 400);
   efl_gfx_visible_set(win, 1);


}
