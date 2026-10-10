#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef BUILD_ENGINE_BUFFER

#include <Evas.h>
#include <Ecore_Evas.h>
#include <string.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

#define TEST_FONT_NAME "DejaVuSansMono"
#define TEST_FONT_SOURCE TESTS_SRC_DIR "/fonts/TestFontMono.eet"

/* Canvas size. The grid sits at (0,0); the rest stays black. */
#define CANVAS_W 200
#define CANVAS_H 200

/* Grid size, in cells */
#define GRID_W 8
#define GRID_H 4

/* Cell size, set by _textgrid_new() for the current test */
static int cw, ch;

#define START_TG_TEST() \
   Ecore_Evas *ee; Evas *e; \
   ee = ecore_evas_buffer_new(CANVAS_W, CANVAS_H); \
   fail_if(!ee); \
   ecore_evas_show(ee); \
   ecore_evas_manual_render_set(ee, EINA_TRUE); \
   e = ecore_evas_get(ee); \
   do {} while (0)

#define END_TG_TEST() do { ecore_evas_free(ee); } while (0)

#define ARGB(a, r, g, b) \
   (((unsigned int)(a) << 24) | ((r) << 16) | ((g) << 8) | (b))

/* Palette colors used by several tests */
#define COL_RED    ARGB(255, 255,   0,   0)
#define COL_BLUE   ARGB(255,   0,   0, 255)
#define COL_GREEN  ARGB(255,  10, 200,  30)
#define COL_FG     ARGB(255, 200,  30,  10)
/* What shows through where nothing is drawn on the buffer canvas:
 * transparent black */
#define COL_CANVAS 0x00000000u

static Evas_Object *
_textgrid_new(Evas *e)
{
   Evas_Object *tg = evas_object_textgrid_add(e);

   fail_if(!tg);
   evas_object_textgrid_font_source_set(tg, TEST_FONT_SOURCE);
   evas_object_textgrid_font_set(tg, TEST_FONT_NAME, 16);
   evas_object_textgrid_size_set(tg, GRID_W, GRID_H);
   evas_object_textgrid_cell_size_get(tg, &cw, &ch);
   fail_if((cw <= 0) || (ch <= 0));
   evas_object_geometry_set(tg, 0, 0, cw * GRID_W, ch * GRID_H);
   evas_object_show(tg);
   return tg;
}

static void
_palette_std_set(Evas_Object *tg, int idx, unsigned int argb)
{
   evas_object_textgrid_palette_set(tg, EVAS_TEXTGRID_PALETTE_STANDARD, idx,
                                    (argb >> 16) & 0xff, (argb >> 8) & 0xff,
                                    argb & 0xff, (argb >> 24) & 0xff);
}

static void
_cells_bg_set(Evas_Object *tg, unsigned char idx)
{
   int x, y;

   for (y = 0; y < GRID_H; y++)
     for (x = 0; x < GRID_W; x++)
       evas_object_textgrid_cellrow_get(tg, y)[x].bg = idx;
   evas_object_textgrid_update_add(tg, 0, 0, GRID_W, GRID_H);
}

static unsigned int *
_snapshot(Ecore_Evas *ee)
{
   unsigned int *data = malloc(CANVAS_W * CANVAS_H * 4);

   fail_if(!data);
   ecore_evas_manual_render(ee);
   memcpy(data, ecore_evas_buffer_pixels_get(ee), CANVAS_W * CANVAS_H * 4);
   return data;
}

static Eina_Bool
_pixels_equal(const unsigned int *a, const unsigned int *b)
{
   int i;

   for (i = 0; i < CANVAS_W * CANVAS_H; i++)
     if (a[i] != b[i]) return EINA_FALSE;
   return EINA_TRUE;
}

/* Center pixel of cell (cx, cy) */
static unsigned int
_pixel(const unsigned int *pix, int cx, int cy)
{
   return pix[(cy * ch + ch / 2) * CANVAS_W + (cx * cw + cw / 2)];
}

static void
_check_pixel(const unsigned int *pix, int cx, int cy, unsigned int expected)
{
   unsigned int got = _pixel(pix, cx, cy);

   if (got != expected)
     ck_abort_msg("pixel (%d,%d) got %#08x expected %#08x", cx, cy, got,
                  expected);
}

EFL_START_TEST(evas_textgrid_test_truecolor_setget)
{
   START_TG_TEST();
   Evas_Object *tg = _textgrid_new(e);
   Evas_Textgrid_Truecolor *row;
   int i;

   fail_if(evas_object_textgrid_truecolor_get(tg) != EINA_FALSE);
   fail_if(evas_object_textgrid_truecolor_cellrow_get(tg, 0) != NULL);

   evas_object_textgrid_truecolor_set(tg, EINA_TRUE);
   fail_if(evas_object_textgrid_truecolor_get(tg) != EINA_TRUE);
   row = evas_object_textgrid_truecolor_cellrow_get(tg, 0);
   fail_if(!row);
   fail_if(evas_object_textgrid_truecolor_cellrow_get(tg, -1) != NULL);
   fail_if(evas_object_textgrid_truecolor_cellrow_get(tg, GRID_H) != NULL);
   /* The buffer starts zeroed: alpha 0 means no color */
   for (i = 0; i < GRID_W; i++)
     fail_if((row[i].fg.a != 0) || (row[i].bg.a != 0));

   evas_object_textgrid_truecolor_set(tg, EINA_FALSE);
   fail_if(evas_object_textgrid_truecolor_get(tg) != EINA_FALSE);
   fail_if(evas_object_textgrid_truecolor_cellrow_get(tg, 0) != NULL);

   END_TG_TEST();
}
EFL_END_TEST

/* Opt-in with no true color flags set must not change any pixel */
EFL_START_TEST(evas_textgrid_test_truecolor_off_pixels_identical)
{
   START_TG_TEST();
   Evas_Object *tg = _textgrid_new(e);
   unsigned int *off, *on;

   _palette_std_set(tg, 0, COL_RED);
   _palette_std_set(tg, 1, COL_BLUE);
   _cells_bg_set(tg, 0);
   {
      int x, y;

      for (y = 0; y < GRID_H; y++)
        for (x = 0; x < GRID_W; x++)
          if (x & 1) evas_object_textgrid_cellrow_get(tg, y)[x].bg = 1;
   }
   evas_object_textgrid_update_add(tg, 0, 0, GRID_W, GRID_H);

   off = _snapshot(ee);

   evas_object_textgrid_truecolor_set(tg, EINA_TRUE);
   on = _snapshot(ee);

   fail_if(!_pixels_equal(off, on));
   _check_pixel(off, 0, 0, COL_RED);
   _check_pixel(off, 1, 0, COL_BLUE);

   free(off);
   free(on);
   END_TG_TEST();
}
EFL_END_TEST

EFL_START_TEST(evas_textgrid_test_truecolor_bg)
{
   START_TG_TEST();
   Evas_Object *tg = _textgrid_new(e);
   Evas_Textgrid_Truecolor *row;
   unsigned int *pix;

   _palette_std_set(tg, 0, COL_RED);
   _cells_bg_set(tg, 0);

   evas_object_textgrid_truecolor_set(tg, EINA_TRUE);
   row = evas_object_textgrid_truecolor_cellrow_get(tg, 1);
   fail_if(!row);
   row[2].bg.r = 10; row[2].bg.g = 200; row[2].bg.b = 30; row[2].bg.a = 255;
   evas_object_textgrid_cellrow_get(tg, 1)[2].bg_truecolor = 1;
   /* alpha 0 means no color: the canvas shows through */
   evas_object_textgrid_cellrow_get(tg, 1)[3].bg_truecolor = 1;
   evas_object_textgrid_update_add(tg, 0, 0, GRID_W, GRID_H);

   pix = _snapshot(ee);
   _check_pixel(pix, 0, 0, COL_RED);
   _check_pixel(pix, 2, 1, COL_GREEN);
   _check_pixel(pix, 3, 1, COL_CANVAS);

   free(pix);
   END_TG_TEST();
}
EFL_END_TEST

/* Foreground via a full block glyph (U+2588, present in the test font) */
EFL_START_TEST(evas_textgrid_test_truecolor_fg)
{
   START_TG_TEST();
   Evas_Object *tg = _textgrid_new(e);
   Evas_Textgrid_Cell *cells;
   Evas_Textgrid_Truecolor *row;
   unsigned int *pix;
   int x, y, n = 0, total = 0;

   evas_object_textgrid_truecolor_set(tg, EINA_TRUE);
   /* All backgrounds transparent (entries are zeroed) */
   for (y = 0; y < GRID_H; y++)
     for (x = 0; x < GRID_W; x++)
       evas_object_textgrid_cellrow_get(tg, y)[x].bg_truecolor = 1;

   cells = evas_object_textgrid_cellrow_get(tg, 1);
   cells[1].codepoint = 0x2588;
   cells[1].fg_truecolor = 1;
   row = evas_object_textgrid_truecolor_cellrow_get(tg, 1);
   fail_if(!row);
   row[1].fg.r = 200; row[1].fg.g = 30; row[1].fg.b = 10; row[1].fg.a = 255;
   evas_object_textgrid_update_add(tg, 0, 0, GRID_W, GRID_H);

   pix = _snapshot(ee);
   /* The glyph is antialiased at its edges: most of the cell, not every
    * pixel, must be exactly the requested color. */
   for (y = 0; y < ch; y++)
     for (x = 0; x < cw; x++)
       {
          unsigned int p = pix[(ch + y) * CANVAS_W + (cw + x)];

          total++;
          if (p == COL_FG) n++;
       }
   if (n <= total / 2)
     printf("textgrid: fg color on %d of %d cell pixels\n", n, total);
   fail_if(n <= total / 2);
   _check_pixel(pix, 2, 1, COL_CANVAS);

   free(pix);
   END_TG_TEST();
}
EFL_END_TEST

/* Bits 8 and 9 of the flags, set while the buffer is disabled, are
 * garbage for old binaries: they must be ignored, with no crash */
EFL_START_TEST(evas_textgrid_test_truecolor_flags_ignored_when_disabled)
{
   START_TG_TEST();
   Evas_Object *tg = _textgrid_new(e);
   unsigned int *pix;
   int x, y;

   _palette_std_set(tg, 0, COL_RED);
   _cells_bg_set(tg, 0);
   for (y = 0; y < GRID_H; y++)
     for (x = 0; x < GRID_W; x++)
       {
          Evas_Textgrid_Cell *cells = evas_object_textgrid_cellrow_get(tg, y);

          cells[x].fg_truecolor = 1;
          cells[x].bg_truecolor = 1;
       }
   evas_object_textgrid_update_add(tg, 0, 0, GRID_W, GRID_H);

   pix = _snapshot(ee);
   _check_pixel(pix, 0, 0, COL_RED);
   _check_pixel(pix, GRID_W - 1, GRID_H - 1, COL_RED);

   free(pix);
   END_TG_TEST();
}
EFL_END_TEST

/* The buffer follows the cell array: freed and zeroed on resize */
EFL_START_TEST(evas_textgrid_test_truecolor_resize_zeroed)
{
   START_TG_TEST();
   Evas_Object *tg = _textgrid_new(e);
   Evas_Textgrid_Truecolor *row;
   int x, i;

   evas_object_textgrid_truecolor_set(tg, EINA_TRUE);
   for (x = 0; x < GRID_W; x++)
     {
        row = evas_object_textgrid_truecolor_cellrow_get(tg, 0);
        row[x].fg.a = 255;
        row[x].bg.a = 255;
     }

   evas_object_textgrid_size_set(tg, 4, 2);
   row = evas_object_textgrid_truecolor_cellrow_get(tg, 0);
   fail_if(!row);
   for (i = 0; i < 4; i++)
     fail_if((row[i].fg.a != 0) || (row[i].bg.a != 0));
   fail_if(evas_object_textgrid_truecolor_cellrow_get(tg, 2) != NULL);

   END_TG_TEST();
}
EFL_END_TEST

/* After disabling, cells fall back to their palette colors */
EFL_START_TEST(evas_textgrid_test_truecolor_disable_fallback)
{
   START_TG_TEST();
   Evas_Object *tg = _textgrid_new(e);
   Evas_Textgrid_Truecolor *row;
   unsigned int *pix;

   _palette_std_set(tg, 0, COL_RED);
   _cells_bg_set(tg, 0);

   evas_object_textgrid_truecolor_set(tg, EINA_TRUE);
   row = evas_object_textgrid_truecolor_cellrow_get(tg, 1);
   fail_if(!row);
   row[1].bg.r = 10; row[1].bg.g = 200; row[1].bg.b = 30; row[1].bg.a = 255;
   evas_object_textgrid_cellrow_get(tg, 1)[1].bg_truecolor = 1;
   evas_object_textgrid_update_add(tg, 0, 0, GRID_W, GRID_H);
   pix = _snapshot(ee);
   _check_pixel(pix, 1, 1, COL_GREEN);
   free(pix);

   evas_object_textgrid_truecolor_set(tg, EINA_FALSE);
   pix = _snapshot(ee);
   _check_pixel(pix, 1, 1, COL_RED);
   free(pix);

   END_TG_TEST();
}
EFL_END_TEST

/* cellrow_set copies a row in and dirties it: no update_add needed */
EFL_START_TEST(evas_textgrid_test_truecolor_cellrow_set)
{
   START_TG_TEST();
   Evas_Object *tg = _textgrid_new(e);
   Evas_Textgrid_Truecolor row[GRID_W];
   Evas_Textgrid_Truecolor *tcs;
   unsigned int *pix;
   int x;

   evas_object_textgrid_truecolor_set(tg, EINA_TRUE);
   for (x = 0; x < GRID_W; x++)
     {
        memset(&row[x], 0, sizeof(row[x]));
        row[x].bg.r = 10; row[x].bg.g = 200; row[x].bg.b = 30;
        row[x].bg.a = 255;
     }
   evas_object_textgrid_truecolor_cellrow_set(tg, 1, row);
   for (x = 0; x < GRID_W; x++)
     evas_object_textgrid_cellrow_get(tg, 1)[x].bg_truecolor = 1;
   evas_object_textgrid_update_add(tg, 0, 0, GRID_W, GRID_H);

   pix = _snapshot(ee);
   _check_pixel(pix, 0, 1, COL_GREEN);
   _check_pixel(pix, GRID_W - 1, 1, COL_GREEN);
   free(pix);

   /* the get pointer reflects what set copied in */
   tcs = evas_object_textgrid_truecolor_cellrow_get(tg, 1);
   fail_if(!tcs);
   fail_if(tcs[0].bg.r != 10 || tcs[0].bg.a != 255);

   END_TG_TEST();
}
EFL_END_TEST

void evas_test_textgrid(TCase *tc)
{
   tcase_add_test(tc, evas_textgrid_test_truecolor_setget);
   tcase_add_test(tc, evas_textgrid_test_truecolor_off_pixels_identical);
   tcase_add_test(tc, evas_textgrid_test_truecolor_bg);
   tcase_add_test(tc, evas_textgrid_test_truecolor_fg);
   tcase_add_test(tc, evas_textgrid_test_truecolor_flags_ignored_when_disabled);
   tcase_add_test(tc, evas_textgrid_test_truecolor_resize_zeroed);
   tcase_add_test(tc, evas_textgrid_test_truecolor_disable_fallback);
   tcase_add_test(tc, evas_textgrid_test_truecolor_cellrow_set);
}

#endif // BUILD_ENGINE_BUFFER
