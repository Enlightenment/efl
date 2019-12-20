// gcc -o efl_ui_grid_example_1 efl_ui_grid_example_1.c `pkg-config --cflags --libs elementary`

#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#else
#define EFL_BETA_API_SUPPORT 1
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include <Efl.h>
#include <stdio.h>

#define IMAX 500;

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   int i = 0, itemmax = IMAX;
   Efl_Ui_Win *win;
   Efl_Ui_Box *box;
   Efl_Ui_Grid *grid;
   Efl_Ui_Item *gitem;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Grid"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   box = efl_add(EFL_UI_BOX_CLASS, win);
   efl_gfx_hint_weight_set(box, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   elm_win_resize_object_add(win, box);

   grid = efl_add(EFL_UI_GRID_CLASS, box);
   efl_gfx_hint_weight_set(grid, EFL_GFX_HINT_EXPAND, EFL_GFX_HINT_EXPAND);
   efl_pack_end(box, grid);

   for (i = 0; i < itemmax; i++)
     {
        int r = 0, g = 0, b = 0;
        gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
        efl_gfx_hint_size_min_set(gitem, EINA_SIZE2D(100, 120));

        Eo *rect = evas_object_rectangle_add(evas_object_evas_get(win));
        switch (i % 5)
          {
           case 0:
             r = 255;
             break;

           case 1:
             g = 255;
             break;

           case 2:
             b = 255;
             break;

           case 3:
             r = g = b = 255;
             break;

           case 4:
             r = g = b = 0;
             break;
          }
        efl_gfx_color_set(rect, r, g, b, 255);
        efl_content_set(gitem, rect);
        efl_pack_end(grid, gitem);
     }

   efl_gfx_entity_size_set(win, EINA_SIZE2D(417, 600));

   elm_run();
   return 0;
}

ELM_MAIN()
