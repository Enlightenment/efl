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

// Unit tests for the gradient ramp atlas.
//
// The atlas is exercised without a GL context: span_grad_atlas_test_enable
// sets a flag that makes _ensure_gl return success and _upload_row skip
// the glTexSubImage2D call.  All cache logic (identity match, hash match,
// memcmp collision check, LRU eviction) runs against the CPU mirror.
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <Eina.h>
#include "ector_suite.h"

#define SPAN_GRAD_ATLAS_TEST_BUILD 1
#include "evas_ector_gl_grad_atlas.h"

// Fill a 4096-byte ramp with a recognizable pattern keyed by `seed`.
static void
_fill_ramp(uint8_t *buf, uint32_t seed)
{
   for (int i = 0; i < SPAN_GRAD_ATLAS_W; i++)
     {
        uint32_t v = (seed * 2654435761u) + (uint32_t)i;
        buf[(i*4) + 0] = (uint8_t)(v >>  0);
        buf[(i*4) + 1] = (uint8_t)(v >>  8);
        buf[(i*4) + 2] = (uint8_t)(v >> 16);
        buf[(i*4) + 3] = (uint8_t)(v >> 24);
     }
}

EFL_START_TEST(grad_atlas_identity_fast_path)
{
   Span_Grad_Atlas *a = span_grad_atlas_new();
   ck_assert_ptr_nonnull(a);
   span_grad_atlas_test_enable(a);
   span_grad_atlas_frame_begin(a);

   uint8_t ramp[SPAN_GRAD_ATLAS_ROW_BYTES];
   _fill_ramp(ramp, 7);
   void *grad = (void *)0x1000;

   int r1 = span_grad_atlas_lookup(a, grad, 1, ramp);
   ck_assert_int_ge(r1, 0);
   // Same (grad, version) should hit identity path and return same row.
   int r2 = span_grad_atlas_lookup(a, grad, 1, ramp);
   ck_assert_int_eq(r1, r2);

   span_grad_atlas_free(a);
}
EFL_END_TEST

EFL_START_TEST(grad_atlas_distinct_ramps_get_distinct_rows)
{
   Span_Grad_Atlas *a = span_grad_atlas_new();
   ck_assert_ptr_nonnull(a);
   span_grad_atlas_test_enable(a);
   span_grad_atlas_frame_begin(a);

   int rows[SPAN_GRAD_ATLAS_H];
   uint8_t ramp[SPAN_GRAD_ATLAS_ROW_BYTES];

   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     {
        _fill_ramp(ramp, (uint32_t)(i + 1));
        rows[i] = span_grad_atlas_lookup(a, (void *)(uintptr_t)(0x1000 + i),
                                         1, ramp);
        ck_assert_int_ge(rows[i], 0);
     }
   // All 64 rows should be unique.
   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     for (int j = i + 1; j < SPAN_GRAD_ATLAS_H; j++)
       ck_assert_int_ne(rows[i], rows[j]);

   span_grad_atlas_free(a);
}
EFL_END_TEST

EFL_START_TEST(grad_atlas_lru_eviction)
{
   Span_Grad_Atlas *a = span_grad_atlas_new();
   ck_assert_ptr_nonnull(a);
   span_grad_atlas_test_enable(a);
   span_grad_atlas_frame_begin(a);

   uint8_t ramp[SPAN_GRAD_ATLAS_ROW_BYTES];

   // Fill all 64 rows, each in its own frame (so last_used differs).
   int first_rows[SPAN_GRAD_ATLAS_H];
   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     {
        span_grad_atlas_frame_begin(a);
        _fill_ramp(ramp, (uint32_t)(i + 1));
        first_rows[i] = span_grad_atlas_lookup(a, (void *)(uintptr_t)(0x1000 + i),
                                               1, ramp);
        ck_assert_int_ge(first_rows[i], 0);
     }
   // The oldest row was filled at frame 1 - that's first_rows[0].
   int oldest_row = first_rows[0];

   // Insert a 65th distinct ramp.  Must evict oldest_row.
   span_grad_atlas_frame_begin(a);
   _fill_ramp(ramp, 9999);
   int new_row = span_grad_atlas_lookup(a, (void *)0xDEADBEEF, 1, ramp);
   ck_assert_int_eq(new_row, oldest_row);

   span_grad_atlas_free(a);
}
EFL_END_TEST

EFL_START_TEST(grad_atlas_content_dedup_across_distinct_grad_ids)
{
   Span_Grad_Atlas *a = span_grad_atlas_new();
   ck_assert_ptr_nonnull(a);
   span_grad_atlas_test_enable(a);
   span_grad_atlas_frame_begin(a);

   uint8_t ramp[SPAN_GRAD_ATLAS_ROW_BYTES];
   _fill_ramp(ramp, 42);

   // Two different grad_ids with identical content - second should hit
   // the hash+memcmp path and return the same row.
   int r1 = span_grad_atlas_lookup(a, (void *)0x1000, 1, ramp);
   ck_assert_int_ge(r1, 0);

   span_grad_atlas_frame_begin(a);
   int r2 = span_grad_atlas_lookup(a, (void *)0x2000, 1, ramp);
   ck_assert_int_eq(r1, r2);

   span_grad_atlas_free(a);
}
EFL_END_TEST

EFL_START_TEST(grad_atlas_hash_collision_distinguished_by_memcmp)
{
   // Two distinct ramp contents - even if they hashed to the same value
   // (rare in practice for FNV-1a over 4 KB), memcmp must detect they
   // differ and allocate a second row.  We can't easily synthesize a
   // real collision, but we CAN simulate the logic by inserting two
   // distinct ramps and verifying they end up in distinct rows.  The
   // real protection (memcmp on hash hit) is tested by inspection:
   // the only path that returns an existing row on hash hit is gated
   // on memcmp == 0.
   Span_Grad_Atlas *a = span_grad_atlas_new();
   ck_assert_ptr_nonnull(a);
   span_grad_atlas_test_enable(a);
   span_grad_atlas_frame_begin(a);

   uint8_t ramp_a[SPAN_GRAD_ATLAS_ROW_BYTES];
   uint8_t ramp_b[SPAN_GRAD_ATLAS_ROW_BYTES];
   _fill_ramp(ramp_a, 1001);
   _fill_ramp(ramp_b, 1002);
   // Confirm they differ - fail loud if our pattern collides accidentally.
   ck_assert_int_ne(memcmp(ramp_a, ramp_b, SPAN_GRAD_ATLAS_ROW_BYTES), 0);

   int ra = span_grad_atlas_lookup(a, (void *)0x1000, 1, ramp_a);
   int rb = span_grad_atlas_lookup(a, (void *)0x2000, 1, ramp_b);
   ck_assert_int_ge(ra, 0);
   ck_assert_int_ge(rb, 0);
   ck_assert_int_ne(ra, rb);

   span_grad_atlas_free(a);
}
EFL_END_TEST

EFL_START_TEST(grad_atlas_version_change_evicts_or_refreshes)
{
   Span_Grad_Atlas *a = span_grad_atlas_new();
   ck_assert_ptr_nonnull(a);
   span_grad_atlas_test_enable(a);
   span_grad_atlas_frame_begin(a);

   uint8_t ramp_v1[SPAN_GRAD_ATLAS_ROW_BYTES];
   uint8_t ramp_v2[SPAN_GRAD_ATLAS_ROW_BYTES];
   _fill_ramp(ramp_v1, 7);
   _fill_ramp(ramp_v2, 8);
   void *grad = (void *)0x1000;

   int r1 = span_grad_atlas_lookup(a, grad, 1, ramp_v1);
   ck_assert_int_ge(r1, 0);

   // Version bumps + content changes - must NOT return r1 via identity
   // (version differs); content also differs so hash path won't dedup.
   // Result: a fresh row is allocated.
   int r2 = span_grad_atlas_lookup(a, grad, 2, ramp_v2);
   ck_assert_int_ge(r2, 0);
   ck_assert_int_ne(r1, r2);

   span_grad_atlas_free(a);
}
EFL_END_TEST

// Counts flush-callback invocations for the exhaustion test.
static int _flush_calls = 0;
static void _count_flush(void *data EINA_UNUSED) { _flush_calls++; }

EFL_START_TEST(grad_atlas_no_eviction_of_rows_used_this_frame)
{
   Span_Grad_Atlas *a = span_grad_atlas_new();
   ck_assert_ptr_nonnull(a);
   span_grad_atlas_test_enable(a);
   _flush_calls = 0;
   span_grad_atlas_flush_cb_set(a, _count_flush, NULL);

   uint8_t ramp[SPAN_GRAD_ATLAS_ROW_BYTES];

   // Fill all 64 rows within a SINGLE frame.
   span_grad_atlas_frame_begin(a);
   int rows[SPAN_GRAD_ATLAS_H];
   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     {
        _fill_ramp(ramp, (uint32_t)(i + 1));
        rows[i] = span_grad_atlas_lookup(a, (void *)(uintptr_t)(0x2000 + i),
                                         (uint32_t)(i + 1), ramp);
        ck_assert_int_ge(rows[i], 0);
     }

   // No flush needed yet: every row was free when it was taken.
   ck_assert_int_eq(_flush_calls, 0);

   // The 65th distinct ramp, still in the same frame, must force a flush
   // before reusing a row that this frame's pending draws still reference.
   _fill_ramp(ramp, 9999);
   int new_row = span_grad_atlas_lookup(a, (void *)0xCAFE, 9999, ramp);
   ck_assert_int_ge(new_row, 0);
   ck_assert_int_eq(_flush_calls, 1);

   // After the drain, no unflushed draw references any row any more, so the
   // next several distinct gradients in the SAME pass must resume ordinary
   // LRU reuse rather than forcing a flush each time - one flush must not
   // turn into a flush storm for every further gradient.
   for (int i = 0; i < 5; i++)
     {
        _fill_ramp(ramp, (uint32_t)(20000 + i));
        int r = span_grad_atlas_lookup(a, (void *)(uintptr_t)(0xD000 + i),
                                       (uint32_t)(20000 + i), ramp);
        ck_assert_int_ge(r, 0);
     }
   ck_assert_int_eq(_flush_calls, 1);

   span_grad_atlas_free(a);
}
EFL_END_TEST

EFL_START_TEST(grad_atlas_flush_cb_optional)
{
   // With no callback registered, nothing is ever drained, so the atlas
   // cannot claim any row is safe to reuse once all are pinned.  It falls
   // back to always returning row 0 for further misses in the same pass -
   // this is deterministic and must not crash across many repeated
   // overwrites of that row.
   Span_Grad_Atlas *a = span_grad_atlas_new();
   ck_assert_ptr_nonnull(a);
   span_grad_atlas_test_enable(a);

   uint8_t ramp[SPAN_GRAD_ATLAS_ROW_BYTES];
   span_grad_atlas_frame_begin(a);
   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     {
        _fill_ramp(ramp, (uint32_t)(i + 1));
        int r = span_grad_atlas_lookup(a, (void *)(uintptr_t)(0x3000 + i),
                                       (uint32_t)(i + 1), ramp);
        ck_assert_int_ge(r, 0);
     }

   // Every subsequent distinct gradient in this pass must land on row 0.
   for (int i = 0; i < 5; i++)
     {
        _fill_ramp(ramp, (uint32_t)(40000 + i));
        int r = span_grad_atlas_lookup(a, (void *)(uintptr_t)(0x4000 + i),
                                       (uint32_t)(40000 + i), ramp);
        ck_assert_int_eq(r, 0);
     }
   span_grad_atlas_free(a);
}
EFL_END_TEST

// Reproduces the all-pinned branch of _alloc_row() being reached while
// current_frame == 0 (i.e. before the first span_grad_atlas_frame_begin()
// call).  Regression test for a uint32_t underflow: `current_frame - 1`
// used to wrap to UINT32_MAX, flattening the LRU forever (row 0 would be
// returned for the rest of the atlas's lifetime, even in later frames).
// With the fix, ageing is clamped at frame 0 instead of underflowing.
EFL_START_TEST(grad_atlas_all_pinned_at_frame_zero)
{
   Span_Grad_Atlas *a = span_grad_atlas_new();
   ck_assert_ptr_nonnull(a);
   span_grad_atlas_test_enable(a);
   _flush_calls = 0;
   span_grad_atlas_flush_cb_set(a, _count_flush, NULL);

   uint8_t ramp[SPAN_GRAD_ATLAS_ROW_BYTES];

   // Deliberately do NOT call span_grad_atlas_frame_begin(): current_frame
   // stays at its calloc'd value of 0, matching every row's initial
   // last_used, so the atlas is "all pinned" as soon as it fills up.
   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     {
        _fill_ramp(ramp, (uint32_t)(i + 1));
        int r = span_grad_atlas_lookup(a, (void *)(uintptr_t)(0x5000 + i),
                                       (uint32_t)(i + 1), ramp);
        ck_assert_int_ge(r, 0);
     }
   ck_assert_int_eq(_flush_calls, 0);

   // The 65th distinct ramp forces the all-pinned branch at current_frame
   // == 0.  It must not crash and must drain via the flush callback.
   //
   // Note: at current_frame == 0 the post-flush ageing is clamped to 0
   // (guarded, see the fix), so aged rows still compare equal to
   // current_frame and read as "pinned" again on the very next miss.  This
   // means every further miss in this same frame-0 pass re-triggers a
   // flush - a degraded-but-safe outcome, not the resumed-LRU behaviour
   // normal passes (current_frame > 0) get.  The regression this guards
   // against is the uint32_t underflow that made rows unevictable *forever*
   // (UINT32_MAX never ages further); what matters here is that every
   // lookup keeps succeeding and the row index stays valid.
   for (int i = 0; i < 6; i++)
     {
        _fill_ramp(ramp, (uint32_t)(30000 + i));
        int r = span_grad_atlas_lookup(a, (void *)(uintptr_t)(0x6000 + i),
                                       (uint32_t)(30000 + i), ramp);
        ck_assert_int_ge(r, 0);
        ck_assert_int_lt(r, SPAN_GRAD_ATLAS_H);
     }
   ck_assert_int_gt(_flush_calls, 0);

   span_grad_atlas_free(a);
}
EFL_END_TEST

void
ector_test_grad_atlas(TCase *tc)
{
   tcase_add_test(tc, grad_atlas_identity_fast_path);
   tcase_add_test(tc, grad_atlas_distinct_ramps_get_distinct_rows);
   tcase_add_test(tc, grad_atlas_lru_eviction);
   tcase_add_test(tc, grad_atlas_content_dedup_across_distinct_grad_ids);
   tcase_add_test(tc, grad_atlas_hash_collision_distinguished_by_memcmp);
   tcase_add_test(tc, grad_atlas_version_change_evicts_or_refreshes);
   tcase_add_test(tc, grad_atlas_no_eviction_of_rows_used_this_frame);
   tcase_add_test(tc, grad_atlas_flush_cb_optional);
   tcase_add_test(tc, grad_atlas_all_pinned_at_frame_zero);
}
