#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
test_ui_box_stack(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *o;
   char buf[PATH_MAX];

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "Efl.Ui.Box_Stack"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   efl_gfx_color_set(efl_part(win, "background"), 24, 24, 64, 255);

   bx = efl_add(EFL_UI_BOX_STACK_CLASS, win);
   efl_content_set(win, bx);

   /* stretched rectangle */
   o = efl_add(EFL_CANVAS_RECTANGLE_CLASS, win,
               efl_gfx_color_set(efl_added, 0, 64, 128, 255));
   efl_pack(bx, o);

   /* rectangle with a max size */
   o = efl_add(EFL_CANVAS_RECTANGLE_CLASS, win,
               efl_gfx_color_set(efl_added, 64, 128, 64, 255),
               efl_gfx_hint_size_max_set(efl_added, EINA_SIZE2D(128, 20)),
               efl_gfx_hint_align_set(efl_added, 0.5, 1.0));
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
