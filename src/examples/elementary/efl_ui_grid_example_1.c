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

#define IMAX 5;

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


static void
_unpack_btn_clicked(void *data, const Efl_Event *ev)
{
   Grid_Event_Data *gd = data;
   efl_pack_unpack(gd->grid, gd->item);
}


static void
_unpackall_btn_clicked(void *data, const Efl_Event *ev)
{
   Eo *grid = data;
   efl_pack_unpack_all(grid);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   int itemmax = IMAX;
   int i = 0;
   if (argv[1]) itemmax = atoi(argv[1]);
   Eo *win, *box, *upbtn, *allbtn;
   Eo *grid, *gitem;
   Grid_Event_Data *gd = calloc(sizeof(Grid_Event_Data *), 1);

   win =  efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
      efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
      efl_text_set(efl_added, "Efl.Ui.Grid"),
      efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   box = EoGenerate(EFL_UI_BOX_CLASS, win, EFL_UI_DIR_VERTICAL);
   elm_win_resize_object_add(win, box);

   // TEST#1 : Create Grid
   gd->grid = grid = EoGenerate(EFL_UI_GRID_CLASS, box, EFL_UI_DIR_DEFAULT);
   efl_ui_grid_item_size_set(grid, EINA_SIZE2D(100, 100)); // 4X4
   efl_pack_end(box, grid);
   // TEST#2 : Set Item Default Size

   itemmax = 5;
   for (i = 0; i < itemmax; i++)
   {
      int r = 0, g = 0, b = 0;
      // TEST#3 : Create Grid Item
      gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
      if (i == 0) gd->item = gitem;
      Eo *rect = evas_object_rectangle_add(evas_object_evas_get(win));
      switch (i % 4)
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
      }
      efl_gfx_color_set(rect, r, g, b, 255);
      efl_content_set(gitem, rect);
      // TEST#4 : Pack end
      efl_pack_end(grid, gitem);
   }

   upbtn = EoGenerate(EFL_UI_BUTTON_CLASS, box, EFL_UI_DIR_DEFAULT);
   efl_text_set(upbtn, "Unpack Item");
   efl_gfx_size_hint_align_set(upbtn, 0.5, 0.5);
   efl_gfx_size_hint_min_set(upbtn, EINA_SIZE2D(200, 25));
   // TEST#4 : Unpack Item
   efl_event_callback_add(upbtn, EFL_UI_EVENT_CLICKED, _unpack_btn_clicked, gd);
   efl_pack_end(box, upbtn);

   allbtn = EoGenerate(EFL_UI_BUTTON_CLASS, box, EFL_UI_DIR_DEFAULT);
   efl_text_set(allbtn, "Unpack All");
   efl_gfx_size_hint_align_set(allbtn, 0.5, 0.5);
   efl_gfx_size_hint_min_set(allbtn, EINA_SIZE2D(200, 25));
   // TEST#4 : Unpack All
   efl_event_callback_add(allbtn, EFL_UI_EVENT_CLICKED, _unpack_btn_clicked, grid);
   efl_pack_end(box, allbtn);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(402, 402));

   elm_run();
   return 0;
}
ELM_MAIN()
