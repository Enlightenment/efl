// gcc -o efl_ui_grid_test_resize efl_ui_grid_test_resize.c `pkg-config --cflags --libs elementary`

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
   Eo *sizw,*sizh;
   Eo *padw, *padh;
   Eo *alnw, *alnh;
   Eina_Size2D size;
   struct {
     double w;
     double h;
   } pad;
   struct {
     double w;
     double h;
   } align;
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
slider_changed(void *data, const Efl_Event *ev)
{
   Grid_Event_Data *gd = data;
   if (!gd) return;
   if (ev->object == gd->sizw)
     gd->size.w = (int) efl_ui_range_value_get(ev->object);
   if (ev->object  == gd->sizh)
     gd->size.h = (int) efl_ui_range_value_get(ev->object);
   if (ev->object  == gd->padw)
     gd->pad.w = efl_ui_range_value_get(ev->object);
   if (ev->object  == gd->padh)
     gd->pad.h = efl_ui_range_value_get(ev->object);
   if (ev->object  == gd->alnw)
     gd->align.w = efl_ui_range_value_get(ev->object);
   if (ev->object  == gd->alnh)
     gd->align.h = efl_ui_range_value_get(ev->object);
}

static void
_btn_clicked(void *data, const Efl_Event *ev)
{
   Grid_Event_Data *gd = data;
   if (!gd) return;

   printf("LSH %d %d %.2lf %.2lf %.2lf %.2lf\n", gd->size.w, gd->size.h, gd->pad.w, gd->pad.h, gd->align.w, gd->align.h);
   efl_ui_grid_item_size_set(gd->grid, gd->size);
   efl_pack_padding_set(gd->grid, gd->pad.w, gd->pad.h, EINA_TRUE);
   efl_pack_align_set(gd->grid, gd->align.w, gd->align.h);
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
   Eo *box, *bx, *bbx, *bbbx, *bbbbx;
   Eo *txt, *sldw, *sldh, *btn;
   Eo *grid, *gitem;
   Grid_Event_Data *gd = calloc(sizeof(Grid_Event_Data *), 1);

   win =  efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
      efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
      efl_text_set(efl_added, "Efl.Ui.Grid Resize"),
      efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   box = EoGenerate(EFL_UI_BOX_CLASS, win, EFL_UI_DIR_VERTICAL);
   elm_win_resize_object_add(win, box);

   // TEST#1 : Create Grid
   gd->grid = grid = EoGenerate(EFL_UI_GRID_CLASS, box, EFL_UI_DIR_VERTICAL);

   // TEST#2 : Set Item Default Size
   gd->size.w = 100;
   gd->size.h = 120;
   gd->pad.w = 1.0;
   gd->pad.h = 1.0;
   gd->align.w = 0.5;
   gd->align.h = 0.5;
   efl_ui_grid_item_size_set(grid, gd->size); // 4X4
   efl_gfx_size_hint_weight_set(grid, EFL_GFX_SIZE_HINT_EXPAND, 0.7);
   efl_pack_padding_set(grid, gd->pad.w, gd->pad.h, EINA_TRUE);
   efl_pack_align_set(grid, gd->align.w, gd->align.h);

   efl_event_callback_add(grid, EFL_EVENT_DEL, _grid_del, gd);
   efl_pack_end(box, grid);

   for (i = 0; i < itemmax; i++)
   {
      int r = 0, g = 0, b = 0;
      // TEST#3 : Create Grid Item
      gitem = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
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

   bx = EoGenerate(EFL_UI_BOX_CLASS, box, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_size_hint_weight_set(bx, EFL_GFX_SIZE_HINT_EXPAND, 0.3);
   efl_pack_padding_set(bx, 10.0, 10.0, EINA_TRUE);
   efl_pack_end(box, bx);

   bbx = EoGenerate(EFL_UI_BOX_CLASS, bx, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_size_hint_weight_set(bbx, EFL_GFX_SIZE_HINT_EXPAND, 0.25);

   bbbx = EoGenerate(EFL_UI_BOX_CLASS, bbx, EFL_UI_DIR_VERTICAL);
   txt = efl_add(EFL_UI_TEXT_CLASS, bbbx,
                 efl_text_set(efl_added, "Size [W, H]"),
                 efl_text_interactive_editable_set(efl_added, EINA_FALSE));
   efl_gfx_size_hint_weight_set(txt, EFL_GFX_SIZE_HINT_EXPAND, 0.1);
   efl_gfx_size_hint_align_set(txt, 0.5, 0.5);

   efl_pack_end(bbbx, txt);

   bbbbx = EoGenerate(EFL_UI_BOX_CLASS, bbbx, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_size_hint_weight_set(bbbbx, EFL_GFX_SIZE_HINT_EXPAND, 0.9);

   gd->sizw = sldw = EoGenerate(EFL_UI_SLIDER_CLASS, bbbbx, EFL_UI_DIR_DOWN);
   efl_gfx_size_hint_min_set(sldw, EINA_SIZE2D(0, 35));
   efl_ui_range_min_max_set(sldw, 50.0, 200.0);
   efl_ui_range_step_set(sldw, 10.0);
   efl_ui_range_value_set(sldw, gd->size.w);
   efl_event_callback_add(sldw, EFL_UI_SLIDER_EVENT_CHANGED, slider_changed, gd);
   efl_pack_end(bbbbx, sldw);

   gd->sizh = sldh = EoGenerate(EFL_UI_SLIDER_CLASS, bbbbx, EFL_UI_DIR_DOWN);
   efl_text_set(sldh, "Size Height");
   efl_gfx_size_hint_min_set(sldh, EINA_SIZE2D(0, 35));
   efl_ui_range_min_max_set(sldh, 50.0, 200.0);
   efl_ui_range_step_set(sldh, 10.0);
   efl_ui_range_value_set(sldh, gd->size.h);
   efl_event_callback_add(sldh, EFL_UI_SLIDER_EVENT_CHANGED, slider_changed, gd);
   efl_pack_end(bbbbx, sldh);

   efl_pack_end(bbbx, bbbbx);
   efl_pack_end(bbx, bbbx);
   efl_pack_end(bx, bbx);

   bbx = EoGenerate(EFL_UI_BOX_CLASS, bx, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_size_hint_weight_set(bbx, EFL_GFX_SIZE_HINT_EXPAND, 0.25);

   bbbx = EoGenerate(EFL_UI_BOX_CLASS, bbx, EFL_UI_DIR_VERTICAL);
   txt = efl_add(EFL_UI_TEXT_CLASS, bbbx,
                 efl_text_set(efl_added, "Pad [W, H]"),
                 efl_text_interactive_editable_set(efl_added, EINA_FALSE));
   efl_gfx_size_hint_weight_set(txt, EFL_GFX_SIZE_HINT_EXPAND, 0.1);
   efl_gfx_size_hint_align_set(txt, 0.5, 0.5);

   efl_pack_end(bbbx, txt);

   bbbbx = EoGenerate(EFL_UI_BOX_CLASS, bbbx, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_size_hint_weight_set(bbbbx, EFL_GFX_SIZE_HINT_EXPAND, 0.9);

   gd->padw = sldw = EoGenerate(EFL_UI_SLIDER_CLASS, bbbbx, EFL_UI_DIR_DOWN);
   efl_text_set(sldw, "Pad Width");
   efl_gfx_size_hint_min_set(sldw, EINA_SIZE2D(0, 35));
   efl_ui_range_min_max_set(sldw, 0.0, 10.0);
   efl_ui_range_step_set(sldw, 1.0);
   efl_ui_range_value_set(sldw, gd->pad.w);
   efl_event_callback_add(sldw, EFL_UI_SLIDER_EVENT_CHANGED, slider_changed, gd);
   efl_pack_end(bbbbx, sldw);

   gd->padh = sldh = EoGenerate(EFL_UI_SLIDER_CLASS, bbbbx, EFL_UI_DIR_DOWN);
   efl_text_set(sldh, "Pad Height");
   efl_gfx_size_hint_min_set(sldh, EINA_SIZE2D(0, 35));
   efl_ui_range_min_max_set(sldh, 0.0, 10.0);
   efl_ui_range_step_set(sldh, 1.0);
   efl_ui_range_value_set(sldh, gd->pad.h);
   efl_event_callback_add(sldh, EFL_UI_SLIDER_EVENT_CHANGED, slider_changed, gd);
   efl_pack_end(bbbbx, sldh);

   efl_pack_end(bbbx, bbbbx);
   efl_pack_end(bbx, bbbx);
   efl_pack_end(bx, bbx);

   bbx = EoGenerate(EFL_UI_BOX_CLASS, bx, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_size_hint_weight_set(bbx, EFL_GFX_SIZE_HINT_EXPAND, 0.25);

   bbbx = EoGenerate(EFL_UI_BOX_CLASS, bbx, EFL_UI_DIR_VERTICAL);
   txt = efl_add(EFL_UI_TEXT_CLASS, bbbx,
                 efl_text_set(efl_added, "Align [W, H]"),
                 efl_text_interactive_editable_set(efl_added, EINA_FALSE));
   efl_gfx_size_hint_weight_set(txt, EFL_GFX_SIZE_HINT_EXPAND, 0.1);
   efl_gfx_size_hint_align_set(txt, 0.5, 0.5);

   efl_pack_end(bbbx, txt);

   bbbbx = EoGenerate(EFL_UI_BOX_CLASS, bbbx, EFL_UI_DIR_HORIZONTAL);
   efl_gfx_size_hint_weight_set(bbbbx, EFL_GFX_SIZE_HINT_EXPAND, 0.9);

   gd->alnw = sldw = EoGenerate(EFL_UI_SLIDER_CLASS, bbbbx, EFL_UI_DIR_DOWN);
   efl_text_set(sldw, "Align Width");
   efl_gfx_size_hint_min_set(sldw, EINA_SIZE2D(0, 35));
   efl_ui_range_min_max_set(sldw, 0.0, 1.0);
   efl_ui_range_value_set(sldw, gd->align.w);
   efl_event_callback_add(sldw, EFL_UI_SLIDER_EVENT_CHANGED, slider_changed, gd);
   efl_pack_end(bbbbx, sldw);

   gd->alnh = sldh = EoGenerate(EFL_UI_SLIDER_CLASS, bbbbx, EFL_UI_DIR_DOWN);
   efl_text_set(sldh, "Align Height");
   efl_gfx_size_hint_min_set(sldh, EINA_SIZE2D(0, 35));
   efl_ui_range_min_max_set(sldh, 0.0, 1.0);
   efl_ui_range_value_set(sldh, gd->align.h);
   efl_event_callback_add(sldh, EFL_UI_SLIDER_EVENT_CHANGED, slider_changed, gd);
   efl_pack_end(bbbbx, sldh);

   efl_pack_end(bbbx, bbbbx);
   efl_pack_end(bbx, bbbx);
   efl_pack_end(bx, bbx);

   btn = EoGenerate(EFL_UI_BUTTON_CLASS, bx, EFL_UI_DIR_DEFAULT);
   efl_gfx_size_hint_weight_set(btn, EFL_GFX_SIZE_HINT_EXPAND, 0.25);
   efl_text_set(btn, "Set Grid");
   efl_gfx_size_hint_min_set(btn, EINA_SIZE2D(50, 50));
   // TEST#4 : Unpack Item
   efl_event_callback_add(btn, EFL_UI_EVENT_CLICKED, _btn_clicked, gd);
   efl_pack_end(bx, btn);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(417, 600));

   elm_run();
   return 0;
}
ELM_MAIN()
