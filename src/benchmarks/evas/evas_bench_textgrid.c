/*
 * Textgrid rendering benchmark.
 *
 * Renders full-screen grids where every cell is rewritten each frame,
 * like a terminal redrawing all of its output.  Two cases: palette
 * colors (the classic 8/16/256 color path) and per-cell true colors.
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>

#include "Evas.h"
#include "Evas_Engine_Buffer.h"
#include "evas_bench.h"

#define TEST_FONT_NAME "DejaVuSansMono"
#define TEST_FONT_SOURCE TESTS_SRC_DIR"/fonts/TestFontMono.eet"

#define CANVAS_W 1200
#define CANVAS_H 720

#define GRID_W 200
#define GRID_H 60

static Evas *
_setup_evas(void)
{
   Evas *evas;
   Evas_Engine_Info_Buffer *einfo;

   evas = evas_new();

   evas_output_method_set(evas, evas_render_method_lookup("buffer"));
   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(evas);

   einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
   einfo->info.dest_buffer = malloc(sizeof(int) * CANVAS_W * CANVAS_H);
   einfo->info.dest_buffer_row_bytes = CANVAS_W * sizeof(int);

   evas_engine_info_set(evas, (Evas_Engine_Info *)einfo);

   evas_output_size_set(evas, CANVAS_W, CANVAS_H);
   evas_output_viewport_set(evas, 0, 0, CANVAS_W, CANVAS_H);

   return evas;
}

static Evas_Object *
_textgrid_new(Evas *e)
{
   Evas_Object *tg = evas_object_textgrid_add(e);
   int cw = 0, ch = 0, i;

   evas_object_textgrid_font_source_set(tg, TEST_FONT_SOURCE);
   evas_object_textgrid_font_set(tg, TEST_FONT_NAME, 10);
   evas_object_textgrid_size_set(tg, GRID_W, GRID_H);
   evas_object_textgrid_cell_size_get(tg, &cw, &ch);
   if ((cw <= 0) || (ch <= 0) ||
       (cw * GRID_W > CANVAS_W) || (ch * GRID_H > CANVAS_H))
     {
        fprintf(stderr, "textgrid bench: bad cell size %dx%d\n", cw, ch);
        evas_object_del(tg);
        return NULL;
     }
   evas_object_geometry_set(tg, 0, 0, cw * GRID_W, ch * GRID_H);
   evas_object_show(tg);

   for (i = 0; i < 256; i++)
     evas_object_textgrid_palette_set(tg, EVAS_TEXTGRID_PALETTE_STANDARD, i,
                                      i, 255 - i, (i * 37) & 255, 255);

   return tg;
}

/* Fill every cell with fresh colors, then render one full redraw */
static void
_frame(Evas *e, Evas_Object *tg, Eina_Bool truecolor, int f)
{
   Eina_List *updates;
   int x, y;

   for (y = 0; y < GRID_H; y++)
     {
        Evas_Textgrid_Cell *cells = evas_object_textgrid_cellrow_get(tg, y);
        Evas_Textgrid_Truecolor *tcs = truecolor ?
           evas_object_textgrid_truecolor_cellrow_get(tg, y) : NULL;

        for (x = 0; x < GRID_W; x++)
          {
             int v = (x * 31 + y * 17 + f * 7);

             cells[x].codepoint = 'A' + (v % 26);
             cells[x].fg = v & 255;
             cells[x].bg = (v * 13) & 255;
             cells[x].bold = (x / 16) & 1;
             if (tcs)
               {
                  tcs[x].bg.r = v & 255;
                  tcs[x].bg.g = (v * 13) & 255;
                  tcs[x].bg.b = (v * 29) & 255;
                  tcs[x].bg.a = 255;
                  tcs[x].fg.r = 255 - (v & 255);
                  tcs[x].fg.g = 255 - ((v * 13) & 255);
                  tcs[x].fg.b = 255 - ((v * 29) & 255);
                  tcs[x].fg.a = 255;
               }
          }
     }
   evas_object_textgrid_update_add(tg, 0, 0, GRID_W, GRID_H);

   updates = evas_render_updates(e);
   evas_render_updates_free(updates);
}

static void
_bench_textgrid(Evas *e, Evas_Object *tg, Eina_Bool truecolor, int request)
{
   int f;

   if (truecolor)
     evas_object_textgrid_truecolor_set(tg, EINA_TRUE);

   for (f = 0; f < request; f++)
     _frame(e, tg, truecolor, f);

   if (truecolor)
     evas_object_textgrid_truecolor_set(tg, EINA_FALSE);
}

static void
evas_bench_textgrid_palette(int request)
{
   Evas *e = _setup_evas();
   Evas_Object *tg = _textgrid_new(e);

   if (tg) _bench_textgrid(e, tg, EINA_FALSE, request);

   evas_object_del(tg);
   evas_free(e);
}

static void
evas_bench_textgrid_truecolor(int request)
{
   Evas *e = _setup_evas();
   Evas_Object *tg = _textgrid_new(e);

   if (tg) _bench_textgrid(e, tg, EINA_TRUE, request);

   evas_object_del(tg);
   evas_free(e);
}

void evas_bench_textgrid(Eina_Benchmark *bench)
{
   /* 5 sweep points: 1500 frames total per case */
   eina_benchmark_register(bench, "palette-render",
                           EINA_BENCHMARK(evas_bench_textgrid_palette),
                           100, 500, 100);
   eina_benchmark_register(bench, "truecolor-render",
                           EINA_BENCHMARK(evas_bench_textgrid_truecolor),
                           100, 500, 100);
}
