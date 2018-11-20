// gcc -o efl_ui_grid_example_1 efl_ui_grid_example_1.c `pkg-config --cflags --libs elementary`

#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#else
#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1
#endif

#include <Elementary.h>
#include <Efl.h>
#include <stdio.h>

#define IMAX 500;

typedef struct _Grid_Event_Data {
   Eo *grid;
   Eo *item;
} Grid_Event_Data;

Eo *
EoGenerate(const Efl_Class *klass, Eo *parent, Efl_Ui_Dir dir)
{
   Eo* obj = efl_add(klass, parent);
   if (dir != EFL_UI_DIR_DEFAULT) efl_ui_direction_set(obj, dir);
   efl_gfx_size_hint_weight_set(obj, EFL_GFX_SIZE_HINT_EXPAND, EFL_GFX_SIZE_HINT_EXPAND);
   efl_gfx_size_hint_align_set(obj, EFL_GFX_SIZE_HINT_FILL, EFL_GFX_SIZE_HINT_FILL);
   return obj;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   int itemmax = IMAX;
   int i = 0;
   if (argv[1]) itemmax = atoi(argv[1]);
   Eo *win, *box, *bbx, *upbtn, *allbtn, *clrbtn;
   Eo *grid, *gitem;
   Grid_Event_Data *gd = calloc(sizeof(Grid_Event_Data *), 1);

   win =  efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
      efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
      efl_text_set(efl_added, "Efl.Ui.Grid"),
      efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   box = EoGenerate(EFL_UI_BOX_CLASS, win, EFL_UI_DIR_VERTICAL);
   elm_win_resize_object_add(win, box);

   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, box);
   efl_gfx_size_hint_weight_set(btn, 0.3, 0.3);
   efl_gfx_size_hint_align_set(btn, EFL_GFX_SIZE_HINT_FILL, EFL_GFX_SIZE_HINT_FILL);
   efl_text_set(btn, "BUTTON");
   efl_pack_end(box, btn);

   Eina_Bool horiz = 0;
   // TEST#1 : Create Grid
   gd->grid = grid = EoGenerate(EFL_UI_GRID_CLASS, box, (horiz ? EFL_UI_DIR_HORIZONTAL : EFL_UI_DIR_VERTICAL));
   efl_ui_grid_item_size_set(grid, EINA_SIZE2D(100, 120)); // 4X4
   efl_pack_padding_set(grid, 5.0, 5.0, EINA_TRUE);
   efl_pack_align_set(grid, 0.5, 0.5);
   efl_pack_end(box, grid);
   
   // TEST#2 : Set Item Default Size

   Efl_Ui_Grid_Item *target = NULL;

   for (i = 0; i < itemmax; i++)
   {
      int r = 0, g = 0, b = 0;
      // TEST#3 : Create Grid Item
      gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
	  if (i == 300) target = gitem;
      if (i == 0) gd->item = gitem;
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
      // TEST#4 : Pack end
      efl_pack_end(grid, gitem);
   }

   efl_gfx_entity_size_set(win, EINA_SIZE2D(417, 600));

   elm_run();
   return 0;
}
ELM_MAIN()
