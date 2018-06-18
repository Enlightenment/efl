// gcc -o efl_ui_grid_test_scroll efl_ui_grid_test_scroll.c `pkg-config --cflags --libs elementary`

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

typedef struct _Grid_Event_Data {
   Eo *grid;
   Efl_Ui_Grid_Item *alignit;
   Eina_Bool anim;
   double align;
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
_check_changed(void *data, const Efl_Event *ev)
{
   Grid_Event_Data *gd = data;
   if (!gd) return;
   gd->anim = efl_ui_nstate_value_get(ev->object);
}

static void
_slider_changed(void *data, const Efl_Event *ev)
{
   Grid_Event_Data *gd = data;
   if (!gd) return;
   gd->align = efl_ui_range_value_get(ev->object);
}

static void
_scroll_btn_clicked(void *data, const Efl_Event *ev)
{
   Grid_Event_Data *gd = data;
   if (!gd) return;

   efl_ui_grid_item_scroll(gd->grid, gd->alignit, gd->anim);
}

static void
_scroll_align_btn_clicked(void *data, const Efl_Event *ev)
{
   Grid_Event_Data *gd = data;
   if (!gd) return;

   efl_ui_grid_item_scroll_align(gd->grid, gd->alignit, gd->align, gd->anim);
}

static void
_grid_del(void *data, const Efl_Event *ev)
{
   Grid_Event_Data *gd = data;
   if (!gd) return;

   efl_event_callback_del(ev->object, EFL_EVENT_DEL, _grid_del, gd);

   //FIXME it occurs critical crash!
   //free(gd);
   printf("FREED!!!\n");

}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   int itemmax = IMAX;
   int i = 0;
   if (argv[1]) itemmax = atoi(argv[1]);
   Eo *win;
   Eo *box, *bx, *ck, *sld, *btn;
   Eo *grid, *gitem;
   Grid_Event_Data *gd = calloc(sizeof(Grid_Event_Data *), 1);
   gd->align = 0.5;
   gd->anim = EINA_FALSE;
   char buf[255];

   win =  efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
      efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
      efl_text_set(efl_added, "Efl.Ui.Grid Scroll"),
      efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   box = EoGenerate(EFL_UI_BOX_CLASS, win, EFL_UI_DIR_VERTICAL);
   elm_win_resize_object_add(win, box);

   // TEST#1 : Create Grid
   gd->grid = grid = EoGenerate(EFL_UI_GRID_CLASS, box, EFL_UI_DIR_VERTICAL);

   // TEST#2 : Set Item Default Size
   efl_ui_grid_item_size_set(grid, EINA_SIZE2D(100, 100)); // 4X4
   efl_gfx_size_hint_weight_set(grid, EFL_GFX_SIZE_HINT_EXPAND, 0.7);
   efl_pack_padding_set(grid, 1.0, 1.0, EINA_TRUE);
   efl_pack_align_set(grid, 0.5, 0.5);

   efl_event_callback_add(grid, EFL_EVENT_DEL, _grid_del, gd);
   efl_pack_end(box, grid);

   for (i = 0; i < itemmax; i++)
   {
      int r = 0, g = 0, b = 0;
      // TEST#3 : Create Grid Item
      gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
      if (i == 50) gd->alignit = gitem;
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
      snprintf(buf, sizeof(buf), "index %d", i);
      efl_text_set(gitem, buf);
      // TEST#4 : Pack end
      efl_pack_end(grid, gitem);
   }

   bx = EoGenerate(EFL_UI_BOX_CLASS, box, EFL_UI_DIR_VERTICAL);
   efl_gfx_size_hint_weight_set(bx, EFL_GFX_SIZE_HINT_EXPAND, 0.3);
   efl_pack_padding_set(bx, 10.0, 10.0, EINA_TRUE);
   efl_pack_end(box, bx);

   ck = EoGenerate(EFL_UI_CHECK_CLASS, bx, EFL_UI_DIR_LEFT);
   efl_gfx_size_hint_weight_set(ck, EFL_GFX_SIZE_HINT_EXPAND, 0.25);
   efl_text_set(ck, "Animation");
   efl_ui_nstate_value_set(ck, EINA_FALSE);
   efl_event_callback_add(ck, EFL_UI_CHECK_EVENT_CHANGED, _check_changed, gd);
   efl_pack_end(bx, ck);

   sld = EoGenerate(EFL_UI_SLIDER_CLASS, bx, EFL_UI_DIR_LEFT);
   efl_gfx_size_hint_weight_set(sld, EFL_GFX_SIZE_HINT_EXPAND, 0.25);
   efl_gfx_size_hint_min_set(sld, EINA_SIZE2D(35, 0));
   efl_ui_range_min_max_set(sld, 0.0, 1.0);
   efl_ui_range_value_set(sld, gd->align);
   efl_event_callback_add(sld, EFL_UI_SLIDER_EVENT_CHANGED, _slider_changed, gd);
   efl_pack_end(bx, sld);

   btn = EoGenerate(EFL_UI_BUTTON_CLASS, bx, EFL_UI_DIR_DEFAULT);
   efl_gfx_size_hint_weight_set(btn, EFL_GFX_SIZE_HINT_EXPAND, 0.25);
   efl_text_set(btn, "Scroll");
   efl_gfx_size_hint_min_set(btn, EINA_SIZE2D(50, 50));
   // TEST#4 : Unpack Item
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _scroll_btn_clicked, gd);
   efl_pack_end(bx, btn);

   btn = EoGenerate(EFL_UI_BUTTON_CLASS, bx, EFL_UI_DIR_DEFAULT);
   efl_gfx_size_hint_weight_set(btn, EFL_GFX_SIZE_HINT_EXPAND, 0.25);
   efl_text_set(btn, "Scroll Align");
   efl_gfx_size_hint_min_set(btn, EINA_SIZE2D(50, 50));
   // TEST#4 : Unpack Item
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _scroll_align_btn_clicked, gd);
   efl_pack_end(bx, btn);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(417, 600));

   elm_run();
   return 0;
}
ELM_MAIN()
