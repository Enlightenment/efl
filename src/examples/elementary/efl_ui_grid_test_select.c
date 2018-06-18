// gcc -o efl_ui_grid_test_select efl_ui_grid_test_select.c `pkg-config --cflags --libs elementary`

#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#else
#define EFL_BETA_API_SUPPORT 1
#define EFL_EO_API_SUPPORT 1
#endif

#include <Elementary.h>
#include <Efl.h>
#include <stdio.h>

#define IMAX 100;

typedef struct _RGB_Color {
      int r;
      int g;
      int b;
} RGB_Color;

Eo *
EoGenerate(const Efl_Class *klass, Eo *parent, Efl_Ui_Dir dir)
{
   Eo* obj = efl_add(klass, parent);
   if (dir != EFL_UI_DIR_DEFAULT) efl_ui_direction_set(obj, dir);
   efl_gfx_size_hint_weight_set(obj, EFL_GFX_SIZE_HINT_EXPAND, EFL_GFX_SIZE_HINT_EXPAND);
   efl_gfx_size_hint_align_set(obj, EFL_GFX_SIZE_HINT_FILL, EFL_GFX_SIZE_HINT_FILL);
   return obj;
}

static RGB_Color
_get_color(int index)
{
   RGB_Color color;

   switch (index % 5)
    {
      case 0:
         color.r = 255;
         color.g = color.b = 0;
         break;
      case 1:
         color.g = 255;
         color.r = color.b = 0;
         break;
      case 2:
         color.b = 255;
         color.r = color.g = 0;
         break;
      case 3:
         color.r = color.g = color.b = 255;
         break;
      case 4:
         color.r = color.g = color.b = 0;
         break;
    }
   return color;
}

static void
_radio_changed(void *data, const Efl_Event *ev)
{
   Eo *grid = data;
   efl_ui_select_mode_set(grid, efl_ui_nstate_value_get(ev->object));
}

static void
_grid_selected(void *data, const Efl_Event *ev)
{
  Eo *grid = ev->object;
  Eo *item = ev->info;

  char buf[256];

  switch(efl_ui_select_mode_get(grid))
   {
     case EFL_UI_SELECT_SINGLE:
        snprintf(buf, sizeof(buf), "SINGLE");
        break;
     case EFL_UI_SELECT_SINGLE_ALWAYS:
        snprintf(buf, sizeof(buf), "SINGLE_ALWAYS");
        break;
     case EFL_UI_SELECT_MULTI:
        snprintf(buf, sizeof(buf), "MULTI");
        break;
     case EFL_UI_SELECT_NONE: // Warn: It never reachable
     default:
        snprintf(buf, sizeof(buf), "NONE");
        break;
   }

  if (item == efl_ui_grid_last_selected_item_get(grid))
    printf("Grid[Mode:%s] Item[%p:%d] Selected\n", buf, item, efl_pack_index_get(grid, item));
  else
    printf("Grid[Mode:%s] Item[%p:%d] is Unmatched\n", buf, item, efl_pack_index_get(grid, item));
}

static void
_grid_unselected(void *data, const Efl_Event *ev)
{
  Eo *grid = ev->object;
  Eo *item = ev->info;
  char buf[256];

  switch(efl_ui_select_mode_get(grid))
   {
     case EFL_UI_SELECT_SINGLE:
        snprintf(buf, sizeof(buf), "SINGLE");
        break;
     case EFL_UI_SELECT_SINGLE_ALWAYS:
        snprintf(buf, sizeof(buf), "SINGLE_ALWAYS");
        break;
     case EFL_UI_SELECT_MULTI:
        snprintf(buf, sizeof(buf), "MULTI");
        break;
     case EFL_UI_SELECT_NONE:
     default:
        snprintf(buf, sizeof(buf), "NONE");
        break;
   }

  printf("Grid[Mode:%s] Item[%p:%d] Unselected\n", buf, item, efl_pack_index_get(grid, item));
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   int itemmax = IMAX;
   int i = 0;
   if (argv[1]) itemmax = atoi(argv[1]);
   Eo *win, *box, *bbx, *txt, *rd, *rdg;
   Eo *grid, *gitem;
   char buf[255];

   win =  efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
      efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
      efl_text_set(efl_added, "Efl.Ui.Grid Unpack"),
      efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   box = EoGenerate(EFL_UI_BOX_CLASS, win, EFL_UI_DIR_VERTICAL);
   elm_win_resize_object_add(win, box);

   // TEST#1 : Create Grid
   grid = EoGenerate(EFL_UI_GRID_CLASS, box, EFL_UI_DIR_VERTICAL);
   efl_gfx_size_hint_weight_set(grid, EFL_GFX_SIZE_HINT_EXPAND, 0.8);
   // TEST#2 : Set Item Default Size
   efl_ui_grid_item_size_set(grid, EINA_SIZE2D(100, 100)); // 4X4
   efl_event_callback_add(grid, EFL_UI_EVENT_SELECTED, _grid_selected, NULL);
   efl_event_callback_add(grid, EFL_UI_EVENT_UNSELECTED, _grid_unselected, NULL);
   efl_pack_end(box, grid);

   for (i = 0; i < itemmax; i++)
   {
      int r = 0, g = 0, b = 0;
      // TEST#3 : Create Grid Item
      gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
      Eo *rect = evas_object_rectangle_add(evas_object_evas_get(win));
      RGB_Color clr = _get_color(i);
      efl_gfx_color_set(rect, clr.r, clr.g, clr.b, 255);
      efl_content_set(gitem, rect);
      snprintf(buf, sizeof(buf), "index %d", i);
      efl_text_set(gitem, buf);
      // TEST#4 : Pack end
      efl_pack_end(grid, gitem);
   }

   bbx = EoGenerate(EFL_UI_BOX_CLASS, win, EFL_UI_DIR_VERTICAL);
   efl_gfx_size_hint_weight_set(bbx, EFL_GFX_SIZE_HINT_EXPAND, 0.2);

   txt = EoGenerate(EFL_UI_TEXT_CLASS, bbx, EFL_UI_DIR_DEFAULT);
   efl_text_set(txt, "Select Mode");
   efl_pack_end(bbx, txt);

   rdg = rd = EoGenerate(EFL_UI_RADIO_CLASS, bbx, EFL_UI_DIR_DEFAULT);
   efl_ui_radio_state_value_set(rd, EFL_UI_SELECT_SINGLE);
   efl_text_set(rd, "SELECT_SINGLE");
   efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, _radio_changed, grid);
   efl_pack_end(bbx, rd);

   rd = EoGenerate(EFL_UI_RADIO_CLASS, box, EFL_UI_DIR_DEFAULT);
   efl_ui_radio_group_add(rd, rdg);
   efl_ui_radio_state_value_set(rd, EFL_UI_SELECT_SINGLE_ALWAYS);
   efl_text_set(rd, "SELECT_SINGLE_ALWAYS");
   efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, _radio_changed, grid);
   efl_pack_end(bbx, rd);

   rd = EoGenerate(EFL_UI_RADIO_CLASS, box, EFL_UI_DIR_DEFAULT);
   efl_ui_radio_group_add(rd, rdg);
   efl_ui_radio_state_value_set(rd, EFL_UI_SELECT_MULTI);
   efl_text_set(rd, "SELECT_MULTI");
   efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, _radio_changed, grid);
   efl_pack_end(bbx, rd);

   rd = EoGenerate(EFL_UI_RADIO_CLASS, box, EFL_UI_DIR_DEFAULT);
   efl_ui_radio_group_add(rd, rdg);
   efl_ui_radio_state_value_set(rd, EFL_UI_SELECT_NONE);
   efl_text_set(rd, "SELECT_NONE");
   efl_event_callback_add(rd, EFL_UI_RADIO_EVENT_CHANGED, _radio_changed, grid);
   efl_pack_end(bbx, rd);

   efl_ui_nstate_value_set(rdg, efl_ui_select_mode_get(grid));

   efl_pack_end(box, bbx);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(417, 600));

   elm_run();
   return 0;
}
ELM_MAIN()
