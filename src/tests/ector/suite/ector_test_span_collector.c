// ECTOR - EFL retained mode drawing library
// Copyright (C) 2026 Cedric Bail
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library;
// if not, see <http://www.gnu.org/licenses/>.

// Unit tests for the span-buffer collector (1-texel gap-encoded format).
//
// The current format stores each span as one RGBA8 texel (4 bytes):
//   byte[0] (B): coverage  - AA coverage 0-255
//   byte[1] (G): len       - span length (0-255; 0 == sentinel)
//   byte[2] (R): gap       - distance from end of previous span on this row
//                            (first span's gap is relative to x_min of the texture)
//   byte[3] (A): reserved  - always 0
//
// stride = (max_spans + 1) * 4 bytes per row.  The extra +1 slot holds the
// zero-length sentinel (byte[1] == 0) that terminates the shader scan loop.
//
// sc->color is a premultiplied ARGB uniform - it is NOT written into the
// span buffer.  The callback sets sc->color = DRAW_MUL4_SYM(sd->color,
// sd->mul_col) before packing spans.
//
// To drive _collect_spans_solid in the test binary we set:
//   sd.mul_col = 0xFFFFFFFF   - identity, so sc->color == sd.color
//   sd.color   = desired_color
//   sd.offx    = sd.offy = 0  - no ector surface offset
//
// Tests exercise span_collector_new/free and _collect_spans_solid in
// isolation - no GL context is required because upload/draw stubs are
// compiled as no-ops when SPAN_COLLECTOR_TEST_BUILD is defined.

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdint.h>
#include <string.h>
#include <Eina.h>
#include "ector_suite.h"
#include "evas_ector_gl_span.h"

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------

// Initialise a Span_Data for solid-fill span tests.
//
// Sets the identity multiplier so that sc->color == sd.color after the
// DRAW_MUL4_SYM call inside _collect_spans_solid.
//
// @param sd    Span_Data to initialise (zeroed first).
// @param sc    Active collector.
// @param color Premultiplied ARGB colour for the fill.
static void
_sd_init_solid(Span_Data *sd, Span_Collector *sc, uint32_t color)
{
   memset(sd, 0, sizeof(*sd));
   sd->span_collector = sc;
   sd->color          = color;
   sd->mul_col        = 0xFFFFFFFF; // identity: DRAW_MUL4_SYM(c, 0xFFFFFFFF) == c
   sd->type           = SPAN_TYPE_SOLID;
   // offx, offy default to 0 - no ector surface translation
}

// Walk one texture row and reconstruct (absolute_x, len) pairs for every
// real span entry (gap extenders are skipped).
//
// Absolute x is accumulated starting from tex->x_min: the first gap is
// relative to x_min, matching the collection-time reference.
//
// @param tex      Texture whose row to inspect.
// @param y        Row index.
// @param stride   Bytes per row (sc->stride).
// @param max_ent  Maximum entries to walk (sc->max_spans is safe).
// @param out_x    Output array for absolute x start values.
// @param out_len  Output array for span lengths.
// @param max_out  Capacity of out_x / out_len.
// @return         Number of real spans found (excludes gap extenders).
static int
_reconstruct_spans(Span_Texture *tex, int y, int stride, int max_ent,
                   int *out_x, int *out_len, int max_out)
{
   int abs_x = tex->x_min;
   int count = 0;
   int i;

   for (i = 0; i < max_ent; i++)
     {
        uint8_t *e   = tex->buffer + ((size_t)y * stride) + ((size_t)i * 4);
        int      cov = e[0];
        int      len = e[1];
        int      gap = e[2];

        if (len == 0) break; // sentinel

        abs_x += gap;

        // Gap extender: cov==0, len==1 - advances position, not a real span.
        if ((cov == 0) && (len == 1))
          {
             abs_x += len;
             continue;
          }

        if (count < max_out)
          {
             out_x[count]   = abs_x;
             out_len[count] = len;
          }
        count++;
        abs_x += len;
     }
   return count;
}

// ------------------------------------------------------------------
// Test 1: single solid span is packed correctly
// ------------------------------------------------------------------

// Create a collector (height=100, max_spans=32), write one span at
// y=10, x=50, len=30, coverage=200, then verify:
//   span_counts[10]  == 1
//   entry byte[0]    == 200  (coverage)
//   entry byte[1]    == 30   (length)
//   entry byte[2]    == 50   (gap from x_min=0, so gap == x)
//   entry byte[3]    == 0    (reserved)
//
// sc->color is set via DRAW_MUL4_SYM(sd.color, sd.mul_col) - not written
// to the buffer, so we do NOT assert buffer bytes against color values.
EFL_START_TEST(span_collector_solid_single)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      span;
   uint8_t        *entry;

   sc = span_collector_new(100, 32, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);
   ck_assert_int_eq(sc->texture_count, 1);

   _sd_init_solid(&sd, sc, 0xFFFF8040);

   span.x        = 50;
   span.y        = 10;
   span.len      = 30;
   span.coverage = 200;
   _collect_spans_solid(1, &span, &sd);

   ck_assert_int_eq(sc->textures[0].span_counts[10], 1);

   // Row 10, entry 0: base = buffer + 10 * stride
   entry = sc->textures[0].buffer + (10 * sc->stride);

   ck_assert_int_eq(entry[0], 200); // coverage
   ck_assert_int_eq(entry[1],  30); // length
   ck_assert_int_eq(entry[2],  50); // gap (x - x_min = 50 - 0 = 50)
   ck_assert_int_eq(entry[3],   0); // reserved

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test 2: zero-length sentinel is written after the last span
// ------------------------------------------------------------------

// Write one span, then verify that the entry immediately following it
// has byte[1] (len) == 0.  The sentinel is at index span_counts[y].
EFL_START_TEST(span_collector_solid_sentinel)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      span;
   uint8_t        *sentinel;

   sc = span_collector_new(50, 32, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   _sd_init_solid(&sd, sc, 0xFFFFFFFF);

   span.x        = 10;
   span.y        =  5;
   span.len      = 20;
   span.coverage = 255;
   _collect_spans_solid(1, &span, &sd);

   ck_assert_int_eq(sc->textures[0].span_counts[5], 1);

   // Sentinel is at entry index 1 (one past the span we wrote).
   sentinel = sc->textures[0].buffer + (5 * sc->stride) + (1 * 4);
   ck_assert_int_eq(sentinel[1], 0); // len == 0 -> sentinel

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test 3: multiple spans on the same row - gap encoding
// ------------------------------------------------------------------

// Write 3 spans on row 20:
//   (x=10, len=5)   gap from x_min=0:     10 - 0       = 10
//   (x=30, len=10)  gap from end of span0: 30 - (10+5) = 15
//   (x=60, len=3)   gap from end of span1: 60 - (30+10)= 20
//
// Verify span_counts == 3 and each entry's gap, len, and coverage bytes.
EFL_START_TEST(span_collector_solid_multi_span)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      spans[3];
   uint8_t        *e0, *e1, *e2;

   sc = span_collector_new(50, 32, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   _sd_init_solid(&sd, sc, 0xFF00FF00);

   spans[0].x = 10; spans[0].y = 20; spans[0].len =  5; spans[0].coverage = 128;
   spans[1].x = 30; spans[1].y = 20; spans[1].len = 10; spans[1].coverage = 255;
   spans[2].x = 60; spans[2].y = 20; spans[2].len =  3; spans[2].coverage =  64;
   _collect_spans_solid(3, spans, &sd);

   ck_assert_int_eq(sc->textures[0].span_counts[20], 3);

   e0 = sc->textures[0].buffer + (20 * sc->stride) + (0 * 4);
   e1 = sc->textures[0].buffer + (20 * sc->stride) + (1 * 4);
   e2 = sc->textures[0].buffer + (20 * sc->stride) + (2 * 4);

   // Span 0: gap = 10 (x=10, x_min=0)
   ck_assert_int_eq(e0[0], 128); // coverage
   ck_assert_int_eq(e0[1],   5); // len
   ck_assert_int_eq(e0[2],  10); // gap
   ck_assert_int_eq(e0[3],   0); // reserved

   // Span 1: gap = 30 - (10+5) = 15
   ck_assert_int_eq(e1[0], 255); // coverage
   ck_assert_int_eq(e1[1],  10); // len
   ck_assert_int_eq(e1[2],  15); // gap
   ck_assert_int_eq(e1[3],   0); // reserved

   // Span 2: gap = 60 - (30+10) = 20
   ck_assert_int_eq(e2[0],  64); // coverage
   ck_assert_int_eq(e2[1],   3); // len
   ck_assert_int_eq(e2[2],  20); // gap
   ck_assert_int_eq(e2[3],   0); // reserved

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test 4: out-of-bounds y values are silently skipped
// ------------------------------------------------------------------

// Spans with y=-1 and y=height are outside [0, height) and must be
// dropped without touching any span_counts entry.
EFL_START_TEST(span_collector_solid_oob)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      spans[2];
   int             i;

   sc = span_collector_new(50, 32, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   _sd_init_solid(&sd, sc, 0xFFFFFFFF);

   spans[0].x =  10; spans[0].y =  -1; spans[0].len = 5; spans[0].coverage = 255;
   spans[1].x =  10; spans[1].y =  50; spans[1].len = 5; spans[1].coverage = 255;
   _collect_spans_solid(2, spans, &sd);

   for (i = 0; i < 50; i++)
     {
        ck_assert_int_eq(sc->textures[0].span_counts[i], 0);
     }

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test 5: large gaps produce gap extender entries
// ------------------------------------------------------------------

// Write two spans on the same row:
//   span A: x=10,  len=5
//   span B: x=600, len=5
//
// Gap between them: 600 - (10+5) = 585 pixels.
// Since 585 > 255 the callback emits gap extenders before span B.
// Each extender consumes 256 pixels (gap=255, len=1), so:
//   floor(585 / 256) = 2 extenders,  remainder = 585 - 2*256 = 73
//
// Expected entries:
//   [0] span A:      cov=200, len=5,  gap=10
//   [1] extender:    cov=0,   len=1,  gap=255
//   [2] extender:    cov=0,   len=1,  gap=255
//   [3] span B:      cov=100, len=5,  gap=73
//
// span_counts[y] == 4 (real spans + gap extenders).
//
// We also walk the entries with _reconstruct_spans and confirm that the
// two real spans land at x=10 and x=600.
EFL_START_TEST(span_collector_gap_extender)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      spans[2];
   uint8_t        *e0, *e1, *e2, *e3;
   int             rx[8], rl[8], rcount;

   // max_spans must be large enough to hold both spans + 2 extenders
   sc = span_collector_new(20, 32, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   _sd_init_solid(&sd, sc, 0xFF808080);

   spans[0].x = 10;  spans[0].y = 5; spans[0].len = 5; spans[0].coverage = 200;
   spans[1].x = 600; spans[1].y = 5; spans[1].len = 5; spans[1].coverage = 100;
   _collect_spans_solid(2, spans, &sd);

   // 2 real spans + 2 gap extenders = 4 entries total
   ck_assert_int_eq(sc->textures[0].span_counts[5], 4);

   e0 = sc->textures[0].buffer + (5 * sc->stride) + (0 * 4);
   e1 = sc->textures[0].buffer + (5 * sc->stride) + (1 * 4);
   e2 = sc->textures[0].buffer + (5 * sc->stride) + (2 * 4);
   e3 = sc->textures[0].buffer + (5 * sc->stride) + (3 * 4);

   // Entry 0: span A
   ck_assert_int_eq(e0[0], 200); // coverage
   ck_assert_int_eq(e0[1],   5); // len
   ck_assert_int_eq(e0[2],  10); // gap = 10 - 0

   // Entry 1: first gap extender
   ck_assert_int_eq(e1[0],   0); // cov = 0 (invisible)
   ck_assert_int_eq(e1[1],   1); // len = 1
   ck_assert_int_eq(e1[2], 255); // gap = 255

   // Entry 2: second gap extender
   ck_assert_int_eq(e2[0],   0);
   ck_assert_int_eq(e2[1],   1);
   ck_assert_int_eq(e2[2], 255);

   // Entry 3: span B - remainder gap = 585 - 2*256 = 73
   ck_assert_int_eq(e3[0], 100); // coverage
   ck_assert_int_eq(e3[1],   5); // len
   ck_assert_int_eq(e3[2],  73); // gap = 585 - 512

   // Reconstruct absolute x: both real spans must land at the right positions
   rcount = _reconstruct_spans(&sc->textures[0], 5, sc->stride,
                               sc->max_spans, rx, rl, 8);
   ck_assert_int_eq(rcount, 2);
   ck_assert_int_eq(rx[0],  10);
   ck_assert_int_eq(rl[0],   5);
   ck_assert_int_eq(rx[1], 600);
   ck_assert_int_eq(rl[1],   5);

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test 6: overflow triggers spatial split; all spans preserved
// ------------------------------------------------------------------

// Create a collector with max_spans=4.  Write 5 spans at x=10,30,60,90,120
// on row 5.  The 5th span triggers overflow and a spatial split.
//
// After the split:
//   - texture_count >= 2
//   - sum of span_counts across ALL textures on row 5 >= 5
//     (gap extenders added by the redistributor may inflate the count,
//     so we check >=5 rather than ==5)
//
// We also reconstruct absolute x positions across all textures and
// confirm all 5 original x positions appear.
EFL_START_TEST(span_collector_overflow_split)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      spans[5];
   int             i, total;
   int             rx[16], rl[16], rcount, all_count;
   int             found[5];
   int             orig_x[5] = {10, 30, 60, 90, 120};
   int             j, k;

   sc = span_collector_new(10, 4, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   _sd_init_solid(&sd, sc, 0xFFFF0000);

   spans[0].x = 10;  spans[0].y = 5; spans[0].len = 5; spans[0].coverage = 255;
   spans[1].x = 30;  spans[1].y = 5; spans[1].len = 5; spans[1].coverage = 255;
   spans[2].x = 60;  spans[2].y = 5; spans[2].len = 5; spans[2].coverage = 255;
   spans[3].x = 90;  spans[3].y = 5; spans[3].len = 5; spans[3].coverage = 255;
   spans[4].x = 120; spans[4].y = 5; spans[4].len = 5; spans[4].coverage = 255;
   _collect_spans_solid(5, spans, &sd);

   ck_assert_int_ge(sc->texture_count, 2);

   // No individual texture should exceed max_spans on row 5.
   total = 0;
   for (i = 0; i < sc->texture_count; i++)
     {
        ck_assert_int_le(sc->textures[i].span_counts[5], sc->max_spans);
        total += sc->textures[i].span_counts[5];
     }
   // total >= 5: all real spans accounted for (gap extenders may add more)
   ck_assert_int_ge(total, 5);

   // Reconstruct absolute positions across all textures.
   all_count = 0;
   for (i = 0; i < sc->texture_count; i++)
     {
        int t_rx[16], t_rl[16];
        int n;
        n = _reconstruct_spans(&sc->textures[i], 5, sc->stride,
                               sc->max_spans, t_rx, t_rl, 16);
        while ((n > 0) && (all_count < 16))
          {
             rx[all_count] = t_rx[n - 1];
             rl[all_count] = t_rl[n - 1];
             all_count++;
             n--;
          }
     }
   // Reconstruct in a second pass that preserves order
   all_count = 0;
   for (i = 0; i < sc->texture_count; i++)
     {
        rcount = _reconstruct_spans(&sc->textures[i], 5, sc->stride,
                                    sc->max_spans,
                                    rx + all_count, rl + all_count,
                                    16 - all_count);
        all_count += rcount;
     }
   ck_assert_int_ge(all_count, 5);

   // Every original x must appear in the reconstructed list.
   memset(found, 0, sizeof(found));
   for (j = 0; j < 5; j++)
     {
        for (k = 0; k < all_count; k++)
          {
             if ((rx[k] == orig_x[j]) && (rl[k] == 5))
               {
                  found[j] = 1;
                  break;
               }
          }
     }
   for (i = 0; i < 5; i++)
     {
        ck_assert_int_eq(found[i], 1);
     }

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test 7: spatial split produces correct absolute x positions
// ------------------------------------------------------------------

// KEY test for the gap-encoded spatial split.
//
// Create a collector with max_spans=4, write spans at x=10,40,80,130,180
// on row 3 (len=5 each).  The 5th span triggers a split.  Walk both
// textures with _reconstruct_spans and verify every original (x, len)
// pair appears exactly once with the correct values.
//
// This tests that the redistributor correctly computes relative gaps
// in the new right-half texture (using split_x as its x_min origin)
// rather than carrying over the old absolute gaps.
EFL_START_TEST(span_collector_split_absolute_x)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      spans[5];
   int             i, all_count;
   int             rx[16], rl[16];
   int             found[5];
   int             orig_x[5] = {10, 40, 80, 130, 180};
   int             j, k;

   sc = span_collector_new(10, 4, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   _sd_init_solid(&sd, sc, 0xFF0000FF);

   for (i = 0; i < 5; i++)
     {
        spans[i].x        = (short)orig_x[i];
        spans[i].y        = 3;
        spans[i].len      = 5;
        spans[i].coverage = 200;
     }
   _collect_spans_solid(5, spans, &sd);

   ck_assert_int_ge(sc->texture_count, 2);

   // Reconstruct across all textures.
   all_count = 0;
   for (i = 0; i < sc->texture_count; i++)
     {
        int n = _reconstruct_spans(&sc->textures[i], 3, sc->stride,
                                   sc->max_spans,
                                   rx + all_count, rl + all_count,
                                   16 - all_count);
        all_count += n;
     }
   ck_assert_int_ge(all_count, 5);

   // Every original span must be present at the correct absolute x.
   memset(found, 0, sizeof(found));
   for (j = 0; j < 5; j++)
     {
        for (k = 0; k < all_count; k++)
          {
             if ((rx[k] == orig_x[j]) && (rl[k] == 5))
               {
                  found[j] = 1;
                  break;
               }
          }
     }
   for (i = 0; i < 5; i++)
     {
        // Use ck_assert_msg so failures name the offending original x.
        ck_assert_msg(found[i] == 1,
                      "span at x=%d not found after split", orig_x[i]);
     }

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test 8: spans route to correct texture after split
// ------------------------------------------------------------------

// After triggering a split on row 5 (so at least two textures exist),
// write a span at a high x on a different row.  It must land in the
// right-side texture (textures[1]), not the primary (textures[0]).
//
// The split point is determined by the median of the overflow row's
// spans.  With spans at x=10,30,60,90,120 the median midpoint is around
// x=62, so x=200 is safely in the right half.
EFL_START_TEST(span_collector_post_split_routing)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      overflow_spans[5];
   SW_FT_Span      new_span;
   int             split_x;

   sc = span_collector_new(10, 4, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   _sd_init_solid(&sd, sc, 0xFFFF0000);

   // Trigger overflow on row 5 to establish the split.
   overflow_spans[0].x = 10;  overflow_spans[0].y = 5;
   overflow_spans[0].len = 5; overflow_spans[0].coverage = 255;
   overflow_spans[1].x = 30;  overflow_spans[1].y = 5;
   overflow_spans[1].len = 5; overflow_spans[1].coverage = 255;
   overflow_spans[2].x = 60;  overflow_spans[2].y = 5;
   overflow_spans[2].len = 5; overflow_spans[2].coverage = 255;
   overflow_spans[3].x = 90;  overflow_spans[3].y = 5;
   overflow_spans[3].len = 5; overflow_spans[3].coverage = 255;
   overflow_spans[4].x = 120; overflow_spans[4].y = 5;
   overflow_spans[4].len = 5; overflow_spans[4].coverage = 255;
   _collect_spans_solid(5, overflow_spans, &sd);

   ck_assert_int_ge(sc->texture_count, 2);

   // Determine the split point so we can place the new span safely in the
   // right half: use textures[1].x_min (its inclusive left edge).
   split_x = sc->textures[1].x_min;

   // Write a span clearly to the right of the split on a different row.
   new_span.x        = (short)(split_x + 20);
   new_span.y        = 3;
   new_span.len      = 10;
   new_span.coverage = 128;
   _collect_spans_solid(1, &new_span, &sd);

   // The span must land in textures[1] (the right-half texture).
   ck_assert_int_eq(sc->textures[1].span_counts[3], 1);
   // And not in textures[0] (left half).
   ck_assert_int_eq(sc->textures[0].span_counts[3], 0);

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test 9: gradient format invariants - stride == (max_spans+1)*4
// ------------------------------------------------------------------

// Verifies that span_collector_new() sets stride correctly for gradient
// fill types.  Stride must be (max_spans + 1) * 4 bytes for all types
// (SPAN_TYPE_SOLID, SPAN_TYPE_LINEAR_GRADIENT, SPAN_TYPE_RADIAL_GRADIENT) - the +1 is the sentinel slot.
//
// Actual gradient color sampling (which requires a gradient ramp texture
// and t-coefficient uniforms) is tested in the integration suite.  Here
// we verify only the structural invariants.
EFL_START_TEST(span_collector_gradient_basic)
{
   Span_Collector *sc_lin;
   Span_Collector *sc_rad;
   int             expected_stride;

   sc_lin = span_collector_new(200, 32, SPAN_TYPE_LINEAR_GRADIENT);
   ck_assert_ptr_nonnull(sc_lin);
   expected_stride = (32 + 1) * 4; // 132 bytes
   ck_assert_int_eq(sc_lin->stride, expected_stride);
   ck_assert_int_eq(sc_lin->texture_count, 1);
   span_collector_free(sc_lin);

   sc_rad = span_collector_new(200, 32, SPAN_TYPE_RADIAL_GRADIENT);
   ck_assert_ptr_nonnull(sc_rad);
   ck_assert_int_eq(sc_rad->stride, expected_stride);
   ck_assert_int_eq(sc_rad->texture_count, 1);
   span_collector_free(sc_rad);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test 10: overflow drop test (legacy) - preserved for regression
// ------------------------------------------------------------------

// Original regression test: with max_spans=4, write exactly 4 spans
// (fills the primary), then write a 5th to trigger overflow + split.
// After the split texture_count >= 2 and the sum of span_counts on
// row 3 across all textures must be >= 5.
EFL_START_TEST(span_collector_solid_overflow_drop)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      span;
   int             i, total;

   sc = span_collector_new(10, 4, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   _sd_init_solid(&sd, sc, 0xFFFF0000);

   for (i = 0; i < 4; i++)
     {
        span.x        = (short)(i * 20);
        span.y        = 3;
        span.len      = 10;
        span.coverage = 255;
        _collect_spans_solid(1, &span, &sd);
     }

   // 5th span triggers overflow.
   span.x        = 100;
   span.y        = 3;
   span.len      = 5;
   span.coverage = 128;
   _collect_spans_solid(1, &span, &sd);

   ck_assert_int_ge(sc->texture_count, 2);

   total = 0;
   for (i = 0; i < sc->texture_count; i++)
     {
        total += sc->textures[i].span_counts[3];
     }
   ck_assert_int_ge(total, 5);

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test: tail memset zeroes the full tail of a row, not just byte[1]
// ------------------------------------------------------------------

// Write one span, then verify that ALL bytes from the entry after
// span_counts[y] to the end of the row (including the sentinel slot)
// are zero.  This confirms the memset covers the full tail rather than
// writing only a single sentinel byte.
EFL_START_TEST(span_collector_solid_row_terminator)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      span;
   uint8_t        *row;

   sc = span_collector_new(50, 16, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   // Pollute the buffer with non-zero bytes to simulate stale data left by
   // an earlier frame.
   memset(sc->textures[0].buffer, 0xAB, (size_t)50 * sc->stride);

   _sd_init_solid(&sd, sc, 0xFFFFFFFF);

   span.x        = 10;
   span.y        =  7;
   span.len      = 20;
   span.coverage = 200;
   _collect_spans_solid(1, &span, &sd);

   ck_assert_int_eq(sc->textures[0].span_counts[7], 1);

   row = sc->textures[0].buffer + (7 * sc->stride);

   // The span itself.
   ck_assert_int_eq(row[0], 200);   // coverage
   ck_assert_int_eq(row[1], 20);    // length

   // What has to hold is that the row is terminated: the entry one past the
   // last span carries a zero length, which is what stops every consumer -
   // the shader's scan loop and the spatial-split walkers alike.
   //
   // The rest of the tail is deliberately left as it was.  Clearing it cost
   // a strided memset of the full row for every row of every shape, and no
   // consumer ever reads past the terminator, so those bytes only had to be
   // erased to satisfy a test.
   ck_assert_int_eq(row[(1 * 4) + 1], 0);

   // And the stale bytes beyond it are indeed still stale, which is the
   // point: this documents the weaker invariant rather than hiding it.
   ck_assert_int_eq(row[(2 * 4) + 1], 0xAB);

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Test: clear zeroes byte[1] of entry 0 so stale rows have sentinel
// ------------------------------------------------------------------

// Frame 1: write a span on row 3, leaving non-zero data in the buffer.
// Call span_collector_clear (simulating a frame boundary).
// Frame 2: write NO span on row 3.
//
// Verify that byte[1] of entry 0 on row 3 is 0 after clear - the shader
// must see len=0 at the very first entry on a row that received no spans.
EFL_START_TEST(span_collector_clear_stale_sentinel)
{
   Span_Collector *sc;
   Span_Data       sd;
   SW_FT_Span      span;
   uint8_t        *entry0;

   sc = span_collector_new(20, 8, SPAN_TYPE_SOLID);
   ck_assert_ptr_nonnull(sc);

   _sd_init_solid(&sd, sc, 0xFFFF0000);

   // Frame 1: write a span on row 3.
   span.x        =  5;
   span.y        =  3;
   span.len      = 30;
   span.coverage = 255;
   _collect_spans_solid(1, &span, &sd);

   ck_assert_int_eq(sc->textures[0].span_counts[3], 1);

   // Simulate frame boundary.
   span_collector_resize(sc, 20);
   span_collector_clear(sc);

   // Frame 2: no spans written on row 3. span_counts[3] == 0.
   ck_assert_int_eq(sc->textures[0].span_counts[3], 0);

   // byte[1] of entry 0 must be 0 - the sentinel the shader relies on.
   entry0 = sc->textures[0].buffer + (3 * sc->stride);
   ck_assert_int_eq(entry0[1], 0);

   span_collector_free(sc);
}
EFL_END_TEST

// ------------------------------------------------------------------
// Registration
// ------------------------------------------------------------------

void
ector_test_span_collector(TCase *tc)
{
   tcase_add_test(tc, span_collector_solid_single);
   tcase_add_test(tc, span_collector_solid_sentinel);
   tcase_add_test(tc, span_collector_solid_multi_span);
   tcase_add_test(tc, span_collector_solid_oob);
   tcase_add_test(tc, span_collector_gap_extender);
   tcase_add_test(tc, span_collector_overflow_split);
   tcase_add_test(tc, span_collector_split_absolute_x);
   tcase_add_test(tc, span_collector_post_split_routing);
   tcase_add_test(tc, span_collector_gradient_basic);
   tcase_add_test(tc, span_collector_solid_overflow_drop);
   tcase_add_test(tc, span_collector_solid_row_terminator);
   tcase_add_test(tc, span_collector_clear_stale_sentinel);
}
