// gcc -o efl_ui_grid_test_pack efl_ui_grid_test_pack.c `pkg-config --cflags --libs elementary`

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
_pack_end_btn_clicked(void *data, const Efl_Event *ev)
{
   Eo *grid = data;
   int count;
   RGB_Color color;

   Eo *gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
   Eo *rect = evas_object_rectangle_add(evas_object_evas_get(grid));
   count = efl_content_count(grid);

   color = _get_color(count);

   efl_gfx_color_set(rect, color.r, color.g, color.b, 255);
   efl_content_set(gitem, rect);
   efl_text_set(gitem, "end");
   // TEST#1 : Pack end
   efl_pack_end(grid, gitem);
}

static void
_pack_begin_btn_clicked(void *data, const Efl_Event *ev)
{
   Eo *grid = data;
   int count;
   RGB_Color color;

   Eo *gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
   Eo *rect = evas_object_rectangle_add(evas_object_evas_get(grid));
   count = efl_content_count(grid);

   color = _get_color(count);

   efl_gfx_color_set(rect, color.r, color.g, color.b, 255);
   efl_content_set(gitem, rect);
   efl_text_set(gitem, "begin");
   // TEST#2 : Pack end
   efl_pack_begin(grid, gitem);
}

static void
_pack_before_btn_clicked(void *data, const Efl_Event *ev)
{
   Eo *grid = data;
   int count;
   RGB_Color color;

   Eo *gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
   Eo *before = efl_pack_content_get(grid, 10);
   Eo *rect = evas_object_rectangle_add(evas_object_evas_get(grid));
   count = efl_content_count(grid);

   color = _get_color(count);

   efl_gfx_color_set(rect, color.r, color.g, color.b, 255);
   efl_content_set(gitem, rect);
   efl_text_set(gitem, "before");

   // TEST#3 : Pack end
   efl_pack_before(grid, gitem, before);
}

static void
_pack_after_btn_clicked(void *data, const Efl_Event *ev)
{
   Eo *grid = data;
   int count;
   RGB_Color color;

   Eo *gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
   Eo *after = efl_pack_content_get(grid, 10);
   Eo *rect = evas_object_rectangle_add(evas_object_evas_get(grid));
   count = efl_content_count(grid);

   color = _get_color(count);

   efl_gfx_color_set(rect, color.r, color.g, color.b, 255);
   efl_content_set(gitem, rect);
   efl_text_set(gitem, "after");

   // TEST#3 : Pack end
   efl_pack_after(grid, gitem, after);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   int itemmax = IMAX;
   int i = 0;
   if (argv[1]) itemmax = atoi(argv[1]);
   Eo *win, *box, *bbx;
   Eo *endbtn, *begbtn, *befbtn, *aftbtn;
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
   // TEST#2 : Set Item Default Size
   efl_ui_grid_item_size_set(grid, EINA_SIZE2D(100, 100)); // 4X4
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

   bbx = EoGenerate(EFL_UI_BOX_CLASS, box, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_size_hint_weight_set(bbx, EFL_GFX_SIZE_HINT_EXPAND, 0.05);
   efl_pack_end(box, bbx);

   endbtn = EoGenerate(EFL_UI_BUTTON_CLASS, bbx, EFL_UI_DIR_DEFAULT);
   efl_text_set(endbtn, "Pack End");
   efl_gfx_size_hint_min_set(endbtn, EINA_SIZE2D(100, 25));
   // TEST#4 : Unpack Item
   efl_event_callback_add(endbtn, EFL_UI_EVENT_CLICKED, _pack_end_btn_clicked, grid);
   efl_pack_end(bbx, endbtn);

   begbtn = EoGenerate(EFL_UI_BUTTON_CLASS, bbx, EFL_UI_DIR_DEFAULT);
   efl_text_set(begbtn, "Pack Begin");
   efl_gfx_size_hint_min_set(begbtn, EINA_SIZE2D(100, 25));
   // TEST#4 : Unpack Item
   efl_event_callback_add(begbtn, EFL_UI_EVENT_CLICKED, _pack_begin_btn_clicked, grid);
   efl_pack_end(bbx, begbtn);

   befbtn = EoGenerate(EFL_UI_BUTTON_CLASS, bbx, EFL_UI_DIR_DEFAULT);
   efl_text_set(befbtn, "Pack Before");
   efl_gfx_size_hint_min_set(befbtn, EINA_SIZE2D(100, 25));
   // TEST#4 : Unpack All
   efl_event_callback_add(befbtn, EFL_UI_EVENT_CLICKED, _pack_before_btn_clicked, grid);
   efl_pack_end(bbx, befbtn);

   aftbtn = EoGenerate(EFL_UI_BUTTON_CLASS, bbx, EFL_UI_DIR_DEFAULT);
   efl_text_set(aftbtn, "Pack After");
   efl_gfx_size_hint_min_set(aftbtn, EINA_SIZE2D(100, 25));
   // TEST#4 : Unpack All
   efl_event_callback_add(aftbtn, EFL_UI_EVENT_CLICKED, _pack_after_btn_clicked, grid);
   efl_pack_end(bbx, aftbtn);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(417, 600));

   elm_run();
   return 0;
}
ELM_MAIN()
