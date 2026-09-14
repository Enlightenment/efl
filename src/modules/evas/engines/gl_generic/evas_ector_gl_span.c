// Span-buffer collector for the Ector GL engine.
//
// Implements lifecycle management and the solid-fill collector callback.
// Gradient and composite callbacks are added in later tasks.

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Include the full private header first so its guard (ECTOR_SOFTWARE_PRIVATE_H_)
// is set before evas_ector_gl_span.h is processed.  This prevents the forward
// declarations in the span header from conflicting with the real definitions.
//
// When SPAN_COLLECTOR_TEST_BUILD is defined (set by the unit-test meson target)
// the private header is not available, so we provide the minimal Span_Data
// definition needed by the solid callback instead.
#ifndef SPAN_COLLECTOR_TEST_BUILD
// Normal engine build: include the evas private headers first (they pull in
// draw.h and other dependencies that ector_software_private.h needs), then
// the ector private header so that its guard ECTOR_SOFTWARE_PRIVATE_H_ is
// set before evas_ector_gl_span.h is processed.
# include "evas_common_private.h"
# include "evas_gl_private.h"
# include "ector_software_private.h"
# include "evas_ector_log_restore.h"
#else
// Test build: the full evas/ector private headers are not available because
// they require generated EFL headers.  Include Eina.h for Eina_Bool and
// EINA_UNUSED.  The minimal Span_Data definition is provided by
// evas_ector_gl_span.h when SPAN_COLLECTOR_TEST_BUILD is defined.
# include <Eina.h>
#endif

#include "evas_ector_gl_span.h"

// ------------------------------------------------------------------
// Internal helpers
// ------------------------------------------------------------------

// Initialize the CPU-side buffers of an already-allocated Span_Texture.
//
// The struct pointed to by @p tex is zeroed and its per-row arrays are
// allocated.  Callers pass a pointer to a slot that already lives in the
// sc->textures array, avoiding a separate heap allocation and copy.
//
// @param tex     Pointer to the Span_Texture slot to initialise.
// @param h       Canvas height - determines buffer row count.
// @param stride  Bytes per row ((max_spans + 1) * 4).
// @param x_min   Inclusive left edge of the x-range this texture covers.
// @param x_max   Inclusive right edge of the x-range this texture covers.
// @return        EINA_TRUE on success, EINA_FALSE on allocation failure.
static Eina_Bool
_span_texture_init(Span_Texture *tex, int h, int stride, int x_min, int x_max)
{
   memset(tex, 0, sizeof(*tex));

   if ((h <= 0) || (h > 16384)) return EINA_FALSE;

   tex->buffer = calloc(h, stride);
   tex->span_counts = calloc(h, sizeof(int));
   tex->last_x_end = calloc(h, sizeof(int));
   if (!tex->buffer || !tex->span_counts || !tex->last_x_end)
     {
        free(tex->buffer);
        free(tex->span_counts);
        free(tex->last_x_end);
        tex->buffer = NULL;
        tex->span_counts = NULL;
        tex->last_x_end = NULL;
        return EINA_FALSE;
     }

   tex->x_min = x_min;
   tex->x_max = x_max;
   return EINA_TRUE;
}

// ------------------------------------------------------------------
// Lifecycle
// ------------------------------------------------------------------

Span_Collector *
span_collector_new(int h, int max_spans, Span_Data_Type type)
{
   Span_Collector *sc;

   // max_spans is stored in 16 bits.
   if ((h <= 0) || (max_spans <= 0) || (max_spans > 0xffff)) return NULL;

   sc = calloc(1, sizeof(Span_Collector));
   if (!sc) return NULL;

   sc->h            = h;
   sc->alloc_h      = h;
   sc->max_spans    = max_spans;
   sc->type      = type;

   // stride: each span entry is 1 RGBA8 texel (4 bytes).
   // One extra entry is reserved for the zero-length sentinel.
   sc->stride = (max_spans + 1) * 4;

   // Start with a single texture covering the full x-range [0, SPAN_TEXTURE_X_MAX_INITIAL].
   sc->textures = malloc(sizeof(Span_Texture));
   if (!sc->textures)
     {
        free(sc);
        return NULL;
     }

   // Initialise the primary texture slot in place - no alloc+copy+free.
   if (!_span_texture_init(&sc->textures[0], h, sc->stride, 0, SPAN_TEXTURE_X_MAX_INITIAL))
     {
        free(sc->textures);
        free(sc);
        return NULL;
     }

   sc->texture_count = 1;
   sc->split_count   = 0;

   // Initialize per-collector transform matrix to identity.
   eina_matrix3_identity(&sc->inv);

   return sc;
}

// Resize a collector for a new active height.
//
// Follows the Evas high-water mark pattern (like pipe buffers and RLE
// spans): buffers grow via realloc when h > alloc_h, but never
// shrink.  When h <= alloc_h, only the active height is updated
// and the existing buffers are reused - no allocation at all.
void
span_collector_resize(Span_Collector *sc, int h)
{
   int i;

   if (!sc || (h <= 0)) return;

   if (sc->h == h) return;  // no change at all

   sc->h = h;

   // When active height changes, the GPU texture dimensions no longer
   // match - mark dirty so the upload path recreates or resizes it.
   for (i = 0; i < sc->texture_count; i++)
     {
        sc->textures[i].dirty = EINA_TRUE;
     }

   // Common case: h fits within existing allocation - no realloc needed.
   if (h <= sc->alloc_h)
     return;

   // Growth needed: realloc all per-row arrays in each texture slot.
   for (i = 0; i < sc->texture_count; i++)
     {
        Span_Texture *tex = &sc->textures[i];
        uint8_t *new_buf;
        int *new_counts, *new_last;

        new_buf = realloc(tex->buffer, (size_t)h * sc->stride);
        new_counts = realloc(tex->span_counts, (size_t)h * sizeof(int));
        new_last = realloc(tex->last_x_end, (size_t)h * sizeof(int));

        if (!new_buf || !new_counts || !new_last)
          {
             // OOM: keep old size, the collector will clip spans to
             // alloc_h via the h field.
             if (new_buf) tex->buffer = new_buf;
             if (new_counts) tex->span_counts = new_counts;
             if (new_last) tex->last_x_end = new_last;
             sc->h = sc->alloc_h;
             return;
          }

        tex->buffer = new_buf;
        tex->span_counts = new_counts;
        tex->last_x_end = new_last;

        // Zero the newly added rows only.
        memset(tex->buffer + ((size_t)sc->alloc_h * sc->stride),
               0, (size_t)(h - sc->alloc_h) * sc->stride);
        memset(tex->span_counts + sc->alloc_h,
               0, (size_t)(h - sc->alloc_h) * sizeof(int));
        memset(tex->last_x_end + sc->alloc_h,
               0, (size_t)(h - sc->alloc_h) * sizeof(int));
     }

   sc->alloc_h = h;
}

void
span_collector_free(Span_Collector *sc)
{
   int i;

   if (!sc) return;

   // No GL resource is owned here: the rows live in the engine's shared
   // span page, which outlives individual collectors.
   for (i = 0; i < sc->texture_count; i++)
     {
        free(sc->textures[i].buffer);
        free(sc->textures[i].span_counts);
        free(sc->textures[i].last_x_end);
     }
   free(sc->textures);
   free(sc);
}

void
span_collector_clear(Span_Collector *sc)
{
   if (!sc) return;

   // Only zero sc->h rows (the active region), not alloc_h.
   // This is safe because:
   // - span_collector_resize zeros newly added rows when growing
   // - _collect_spans_solid memsets the tail of each row (from the last
   //   written entry to max_spans+1) during collection, which writes the
   //   sentinel implicitly for rows that receive spans
   // - for rows that receive NO spans this frame, we zero byte[1] of entry 0
   //   here so the shader sees len=0 and stops immediately (the rest of the
   //   buffer may retain stale data, but the shader never reaches it)
   // - clear is always called after resize, which has already set height
   {
      int i;
      for (i = 0; i < sc->texture_count; i++)
        {
           Span_Texture *tex = &sc->textures[i];
           int           y;

           memset(tex->span_counts, 0, sc->h * sizeof(int));
           memset(tex->last_x_end, 0, sc->h * sizeof(int));

           // Zero byte[1] (len) of entry 0 on every row so that rows
           // which receive no spans this frame have a valid sentinel.
           // _collect_spans_solid memsets the full tail for rows it touches,
           // so this 4-byte-stride write covers only the uncollected rows.
           for (y = 0; y < sc->h; y++)
             {
                tex->buffer[((size_t)y * sc->stride) + 1] = 0;  // byte[1] = len = 0
             }

           tex->dirty = EINA_FALSE;
           tex->rolling_hash = 2166136261u;  // seed
        }
   }

   sc->actual_max_spans = 0;

   // Reset per-shape metadata to prevent stale pointers and type mismatches.
   // gradient_data points to Eo-managed data (Ector_Renderer_Software_Gradient_Data)
   // that may be freed between frames via the Eina free queue.  If not
   // NULLed here, a reused collector from the high-water mark pool
   // would retain a dangling pointer from the previous frame.
   // type must also be reset - a collector previously used for gradient fills
   // retains LinearGradient/RadialGradient, causing eng_ector_end to select
   // the gradient shader with a NULL gradient_data when reused for solid fills.
   sc->type          = Solid;
   sc->gradient_data = NULL;
   sc->color         = 0;
   sc->mask_surface  = NULL;
   sc->comp_method   = 0;

   // Reset row-tail flush state for the new frame.
   sc->flush_prev_y  = -1;
   sc->flush_prev_ti = -1;
}

// ------------------------------------------------------------------
// Internal: span entry write helper
// ------------------------------------------------------------------

// Clamp gap and len to [0, 255] and write a 4-byte span entry to @p dst.
//
// @param dst  4-byte destination in the span texture row buffer.
// @param cov  Coverage value (0-255); caller guarantees this is already in range.
// @param len  Span length; clamped to [0, 255] defensively.
// @param gap  Gap from end of previous span; clamped to [0, 255].
static inline void
_write_span_entry(uint8_t *dst, int cov, int len, int gap)
{
   if (gap < 0) gap = 0;
   if (gap > 255) gap = 255;
   if (len < 0) len = 0;
   if (len > 255) len = 255;
   dst[0] = (uint8_t)cov;
   dst[1] = (uint8_t)len;
   dst[2] = (uint8_t)gap;
   dst[3] = 0;
}

// ------------------------------------------------------------------
// Internal: spatial split helpers
// ------------------------------------------------------------------

// Find the approximate median x-coordinate among spans on row @p y.
//
// The 1-texel gap-encoded format stores [cov, len, gap, reserved] per span.
// Absolute x is reconstructed by walking the spans: x += gap, then x += len.
// The average midpoint (x_start + len/2) is used as the split point.
//
// @param tex            Texture whose row to inspect.
// @param y              Row index.
// @param stride         Bytes per row (sc->stride).
// @param bytes_per_span Bytes per span entry (always 4).
// @return               Average midpoint x, or 0 if the row is empty.
static int
_find_split_x(Span_Texture *tex, int y, int stride, int bytes_per_span)
{
   int count = tex->span_counts[y];
   int sum_x = 0;
   int real_count = 0;
   // First gap is relative to x_min, not 0.
   int abs_x = tex->x_min;
   int i;

   for (i = 0; i < count; i++)
     {
        uint8_t *e = tex->buffer + ((size_t)y * stride) + ((size_t)i * bytes_per_span);
        // Gap-encoded: byte[0]=cov, byte[1]=len, byte[2]=gap, byte[3]=reserved
        int cov = e[0];
        int len = e[1];
        int gap = e[2];
        if (len == 0) break;  // sentinel
        abs_x += gap;
        // Skip gap extender entries (cov==0 && len==1): advance position
        // but do not count them when computing the midpoint average.
        if ((cov == 0) && (len == 1))
          {
             abs_x += len;
             continue;
          }
        sum_x += (abs_x + (len / 2));
        real_count++;
        abs_x += len;
     }
   return (real_count > 0) ? (sum_x / real_count) : 0;
}

// Emit gap extender entries to bridge a gap larger than 255.
//
// Writes extender entries (cov=0, len=1, gap=255) to row buffer starting
// at entry index @p idx.  Reduces *gap_ptr by 256 per extender.
// Returns number of entries written.
//
// @param row_buf   Pointer to the start of the row (buffer + y * stride).
// @param idx       Entry index to start writing at.
// @param max_spans Maximum spans allowed in this row.
// @param stride    Bytes per row (unused here - caller passes row_buf already
//                  offset to the correct row; kept for API symmetry).
// @param gap_ptr   Remaining gap; reduced by 256 per extender written.
// @return          Number of extender entries written.
static int
_emit_gap_extenders(uint8_t *row_buf, int idx, int max_spans,
                    int stride EINA_UNUSED, int *gap_ptr)
{
   int written = 0;

   while ((*gap_ptr > 255) && ((idx + written) < max_spans))
     {
        _write_span_entry(row_buf + ((size_t)(idx + written) * 4),
                          0, 1, 255);
        *gap_ptr -= 256;  // 255 gap + 1 len
        written++;
     }
   return written;
}

// Perform a spatial split of the texture that overflowed on @p overflow_y.
//
// A new Span_Texture is created for spans whose x_start >= split_x, and all
// existing rows are redistributed: spans fully on the right migrate to the
// new texture; straddling spans are split at split_x; left-only spans stay.
//
// Gap extender entries (cov==0, len==1) in the source are skipped - they are
// not redistributed.  New gap extenders are emitted in each destination where
// the recomputed relative gap exceeds 255.
//
// After a successful split the caller must retry the span that triggered the
// overflow (without advancing the spans pointer) so that it routes to the
// correct texture.
//
// @param sc         Active collector.
// @param overflow_y Row index where the overflow was detected.
// @return           EINA_TRUE on success, EINA_FALSE if the split quota is
//                   exhausted or no meaningful split point could be found.
static Eina_Bool
_do_spatial_split(Span_Collector *sc, int overflow_y)
{
   int           bytes_per_span = 4;
   int           old_ti, split_x, old_x_max, i, y;
   Span_Texture *old_tex;
   Span_Texture *new_tex;

   if (sc->split_count >= SPAN_COLLECTOR_MAX_SPLITS)
     return EINA_FALSE;

   // Find which texture holds the overflowing row.
   old_ti = -1;
   for (i = 0; i < sc->texture_count; i++)
     {
        if (sc->textures[i].span_counts[overflow_y] >= sc->max_spans)
          {
             old_ti = i;
             break;
          }
     }
   if (old_ti < 0) return EINA_FALSE;  // no texture actually overflowed
   old_tex = &sc->textures[old_ti];

   split_x = _find_split_x(old_tex, overflow_y, sc->stride, bytes_per_span);

   // Guard against degenerate split points that would produce an empty half.
   if ((split_x <= old_tex->x_min) || (split_x >= old_tex->x_max))
     return EINA_FALSE;

   // Save x_max before realloc potentially moves the textures array.
   old_x_max = old_tex->x_max;

   // Grow the textures array.  Note: realloc may move it, so re-seat
   // old_tex after the realloc.
   {
      Span_Texture *resized = realloc(sc->textures,
                                      (size_t)(sc->texture_count + 1) *
                                      sizeof(Span_Texture));
      if (!resized) return EINA_FALSE;
      sc->textures = resized;
   }

   // Initialise the new right-half texture slot in place.
   // Allocate at alloc_h (high-water mark), not the current active
   // height.  span_collector_clear memsets alloc_h rows on ALL
   // textures when the active height grows back within alloc_h.
   if (!_span_texture_init(&sc->textures[sc->texture_count],
                           sc->alloc_h, sc->stride, split_x, old_x_max))
     {
        // realloc already grew the array; shrink the logical count back down.
        // The uninitialized slot at sc->texture_count is harmless since
        // texture_count is not incremented.
        return EINA_FALSE;
     }
   sc->texture_count++;

   // Re-seat pointers after realloc.
   old_tex = &sc->textures[old_ti];
   new_tex = &sc->textures[sc->texture_count - 1];

   // Redistribute every row.
   //
   // Gap-encoded format: byte[0]=cov, byte[1]=len, byte[2]=gap, byte[3]=0
   // Absolute position reconstructed as:
   //   abs_x starts at old_tex->x_min (first gap is relative to x_min)
   //   abs_x += gap  -> start of span
   //   abs_x += len  -> end of span / start of next gap region
   //
   // Gap extender entries (cov==0 && len==1) bridge gaps > 255 in the
   // source - skip them during redistribution (they are regenerated below
   // wherever the recomputed destination gap still exceeds 255).
   //
   // left_last  tracks the end of the last span written to the left texture;
   //            initialised to old_tex->x_min so that the first left-span
   //            gap is relative to the same origin used during collection.
   // right_last tracks the end of the last span written to the right texture;
   //            initialised to split_x (the x_min of the new right texture).
   for (y = 0; y < sc->h; y++)
     {
        int src_count  = old_tex->span_counts[y];
        int left_idx   = 0;
        int right_idx  = 0;
        int abs_x      = old_tex->x_min;
        int left_last  = old_tex->x_min;
        int right_last = split_x;

        uint8_t *left_row  = old_tex->buffer + ((size_t)y * sc->stride);
        uint8_t *right_row = new_tex->buffer + ((size_t)y * sc->stride);

        for (i = 0; i < src_count; i++)
          {
             uint8_t *src_entry = left_row + ((size_t)i * bytes_per_span);
             int gap = src_entry[2];
             int len = src_entry[1];
             int cov = src_entry[0];
             if (len == 0) break;  // sentinel

             abs_x += gap;  // start of this span in absolute coords

             // Skip source gap extender entries - just advance position.
             if ((cov == 0) && (len == 1))
               {
                  abs_x += len;
                  continue;
               }

             {
                int span_end = abs_x + len;

                if (span_end <= split_x)
                  {
                     // Entirely in the left half - compact in place.
                     int new_gap = abs_x - left_last;
                     left_idx += _emit_gap_extenders(left_row, left_idx,
                                                     sc->max_spans,
                                                     sc->stride, &new_gap);
                     if (left_idx < sc->max_spans)
                       {
                          _write_span_entry(left_row + ((size_t)left_idx * 4),
                                            cov, len, new_gap);
                          left_idx++;
                          left_last = span_end;
                       }
                  }
                else if (abs_x >= split_x)
                  {
                     // Entirely in the right half - move to new texture.
                     int new_gap = abs_x - right_last;
                     right_idx += _emit_gap_extenders(right_row, right_idx,
                                                      sc->max_spans,
                                                      sc->stride, &new_gap);
                     if (right_idx < sc->max_spans)
                       {
                          _write_span_entry(right_row + ((size_t)right_idx * 4),
                                            cov, len, new_gap);
                          right_idx++;
                          right_last = span_end;
                       }
                  }
                else
                  {
                     // Straddles the split point - divide at split_x.
                     int left_len  = split_x - abs_x;
                     int right_len = span_end - split_x;

                     // Left fragment.
                     {
                        int new_gap = abs_x - left_last;
                        left_idx += _emit_gap_extenders(left_row, left_idx,
                                                        sc->max_spans,
                                                        sc->stride, &new_gap);
                        if (left_idx < sc->max_spans)
                          {
                             _write_span_entry(left_row + ((size_t)left_idx * 4),
                                               cov, left_len, new_gap);
                             left_idx++;
                             left_last = split_x;
                          }
                     }

                     // Right fragment starts exactly at split_x -> gap = 0.
                     if (right_idx < sc->max_spans)
                       {
                          _write_span_entry(right_row + ((size_t)right_idx * 4),
                                            cov, right_len, 0);
                          right_idx++;
                          right_last = span_end;
                       }
                  }
             }

             abs_x += len;  // advance past this span
          }

        old_tex->span_counts[y] = left_idx;
        old_tex->last_x_end[y]  = left_last;
        new_tex->span_counts[y] = right_idx;
        new_tex->last_x_end[y]  = right_last;

        // Clear tails of redistributed rows - stale entries beyond the
        // new span_counts have non-zero len bytes from the pre-split data.
        // The collection callback's row-change memset won't cover these
        // since the split happens mid-collection.
        if (left_idx < sc->max_spans)
          memset(left_row + ((size_t)left_idx * 4), 0,
                 (size_t)(sc->max_spans + 1 - left_idx) * 4);
        if (right_idx < sc->max_spans)
          memset(right_row + ((size_t)right_idx * 4), 0,
                 (size_t)(sc->max_spans + 1 - right_idx) * 4);
     }

   // Mark both textures dirty so the next upload path recreates them.
   old_tex->dirty = EINA_TRUE;
   new_tex->dirty = EINA_TRUE;

   // Commit metadata now that redistribution is complete.
   old_tex->x_max = split_x - 1;
   sc->split_points[sc->split_count++] = split_x;

   return EINA_TRUE;
}

// ------------------------------------------------------------------
// Internal: spatial texture lookup
// ------------------------------------------------------------------

// Return the index of the Span_Texture whose x-range contains @p x.
//
// Falls back to texture 0 (the primary) if no slot matches, which can
// happen transiently before spatial splits are fully set up.
static inline int
_find_texture_for_x(Span_Collector *sc, int x)
{
   int i;

   if (sc->texture_count == 1) return 0;

   for (i = 0; i < sc->texture_count; i++)
     {
        if ((x >= sc->textures[i].x_min) && (x <= sc->textures[i].x_max))
          return i;
     }
   return 0;
}

// ------------------------------------------------------------------
// Row-tail sentinel helper
// ------------------------------------------------------------------

// Zero the tail of a span texture row from the last written entry to the
// end of the stride.
//
// This writes the zero-length sentinel implicitly (the entry at
// span_counts[y] has len=0 after zeroing) AND clears any stale data from
// prior frames beyond the current frame's last span.
//
// The memset covers exactly (max_spans + 1 - idx) entries starting at
// index idx - only the unused tail, not the full row.  When idx == 0 the
// entire row is zeroed; when idx == max_spans nothing is done (row full,
// sentinel already provided by the spatial split path).
//
// @param sc  Span collector owning the texture.
// @param ti  Texture index within sc->textures[].
// @param y   Row index.
static inline void
_flush_row_tail(Span_Collector *sc, int ti, int y)
{
   Span_Texture *tex = &sc->textures[ti];
   int           idx = tex->span_counts[y];

   // Only the terminator has to be written, not the whole tail.  Nothing
   // reads past it: the shader's scan breaks at the first zero length, and
   // the split helpers walk exactly span_counts[y] entries.  Clearing the
   // rest was up to 260 bytes of strided memset for every row of every
   // shape - on a 600x300 drawing with five shapes, a few hundred kilobytes
   // a frame to erase bytes no one looks at.
   if (idx < sc->max_spans)
     tex->buffer[((size_t)y * sc->stride) + ((size_t)idx * 4) + 1] = 0;
}

// ------------------------------------------------------------------
// Solid span collector callback
// ------------------------------------------------------------------

// SW_FT_SpanFunc callback for Solid fills.
//
// Packs each span as 1 RGBA8 texel (4 bytes) into the Span_Texture row:
//   byte 0: gap   - distance from end of previous span on this row
//   byte 1: len   - span length (max 255; longer spans are split)
//   byte 2: coverage - AA coverage 0-255
//   byte 3: reserved (zero)
//
// The base color is passed to the shader as a uniform, not per-span.
// A zero-length sentinel (byte[1] = 0) terminates the row; it is written
// implicitly by memset-ing the tail of the row (from span_counts[y] to
// max_spans+1) once per row when y changes.  The buffer is hot in L1-D
// from the span writes, so the memset is nearly free.
//
// Rows that receive no spans have their sentinel guaranteed by
// span_collector_clear, which zeroes byte[1] of entry 0 on all rows.
//
// Spans with y outside [0, height) are silently skipped.
// When a row hits max_spans, a spatial split is attempted (_do_spatial_split).
//
// @p user_data must point to a Span_Data whose span_collector field holds
// the active Span_Collector.
void
_collect_spans_solid(int count, const SW_FT_Span *spans, void *user_data)
{
   Span_Data      *sd  = (Span_Data *)user_data;
   Span_Collector *sc  = (Span_Collector *)sd->span_collector;
   int             ti, idx, y, sx;
   Span_Texture   *tex;
   uint8_t        *entry;

   if (!sc) return;

   // sd->color and sd->mul_col are constant across all spans in one
   // callback invocation - compute the composited base color once.
   // DRAW_MUL4_SYM is defined either via draw.h (engine build) or via
   // evas_ector_gl_span.h (SPAN_COLLECTOR_TEST_BUILD).
   sc->color = DRAW_MUL4_SYM(sd->color, sd->mul_col);

   while (count > 0)
     {
        // Apply the ector surface offset to get canvas-space coords.
        // spans->x/y are local to the shape; offx/offy position the
        // shape within the ector surface (same as _blend_argb).
        y  = spans->y + sd->offy;
        sx = spans->x + sd->offx;

        // Skip spans outside the canvas.
        if ((y < 0) || (y >= sc->h))
          {
             spans++;
             count--;
             continue;
          }

        // FreeType delivers spans in row order (y non-decreasing within a
        // callback).  When y changes, the previous row is complete: memset
        // the tail from the last written entry to the end of the row.
        //
        // This writes the zero-length sentinel implicitly (entry at
        // span_counts[prev_y] has len=0 after zeroing) AND clears any stale
        // data from prior frames beyond the current frame's last span.
        // The buffer is hot in L1-D from the span writes above, so the
        // memset is nearly free.
        //
        // Only fires when y actually changes - not once per span.
        if ((y != sc->flush_prev_y) && (sc->flush_prev_y >= 0) && (sc->flush_prev_ti >= 0))
          _flush_row_tail(sc, sc->flush_prev_ti, sc->flush_prev_y);

        ti  = (sc->texture_count == 1) ? 0 : _find_texture_for_x(sc, sx);
        tex = &sc->textures[ti];
        idx = tex->span_counts[y];

        // When the row is full, attempt a spatial split so that the
        // overflow span can still be routed to a new right-half texture.
        // On split success, retry the current span without advancing - the
        // textures array has been reorganised and _find_texture_for_x will
        // now return a different (less-full) slot.
        // On split failure (quota exhausted or degenerate geometry), drop
        // the span and continue.
        if (idx >= sc->max_spans)
          {
             if (!_do_spatial_split(sc, y))
               {
                  spans++;
                  count--;
               }
             continue;
          }

        {
           // 1-texel (4-byte) packing in BGRA-swapped order so the buffer
           // can be uploaded directly via GL_BGRA without a staging copy.
           //
           // Memory layout: [cov, len, gap, reserved]
           // GL_BGRA interprets: B=cov, G=len, R=gap, A=reserved
           // Shader reads: .r=gap, .g=len, .b=cov - correct.
           //
           // gap = distance from end of previous span on this row.
           // Spans longer than 255 are split into multiple entries.

           // Compute gap relative to x_min so split textures don't
           // overflow the 8-bit gap field.  The shader adds x_min to
           // its sx accumulator to recover absolute coordinates.
           int ref = (tex->last_x_end[y] > tex->x_min)
                   ? tex->last_x_end[y] : tex->x_min;
           int gap = sx - ref;
           int remaining = spans->len;
           unsigned int cov = spans->coverage;
           int cur_x = sx;

           tex->dirty = EINA_TRUE;

           if (gap < 0) gap = 0;

           // When the gap exceeds 255, emit invisible "gap extender"
           // entries (coverage=0, len=1) that advance the shader's x
           // accumulator by 256 per entry without drawing anything.
           // This preserves absolute x positioning for wide VG objects
           // where spans can be hundreds of pixels apart.
           while ((gap > 255) && (idx < sc->max_spans))
             {
                entry = tex->buffer + ((size_t)y * sc->stride) + ((size_t)idx * 4);
                entry[0] = 0;              // cov = 0 -> invisible
                entry[1] = 1;              // len = 1 -> advances x by 1
                entry[2] = 255;            // gap = 255 -> advances x by 255
                entry[3] = 0;
                tex->rolling_hash = (tex->rolling_hash * 31) + *((const uint32_t *)entry);
                gap -= 256;                // 255 gap + 1 len = 256 pixels
                idx++;
             }

           while ((remaining > 0) && (idx < sc->max_spans))
             {
                int chunk = (remaining > 255) ? 255 : remaining;
                int g = (cur_x == sx) ? gap : 0;
                if (g > 255) g = 255;

                // Compose once, then hash the value rather than reading
                // back the bytes just stored - that read waits on the
                // stores in the innermost loop of the collector.
                {
                   uint32_t v = (uint32_t)cov
                              | ((uint32_t)chunk << 8)
                              | ((uint32_t)g     << 16);

                   entry = tex->buffer + ((size_t)y * sc->stride) + ((size_t)idx * 4);
                   entry[0] = (uint8_t)cov;    // byte0 -> B in BGRA
                   entry[1] = (uint8_t)chunk;  // byte1 -> G in BGRA
                   entry[2] = (uint8_t)g;      // byte2 -> R in BGRA
                   entry[3] = 0;               // byte3 -> A in BGRA
                   tex->rolling_hash = (tex->rolling_hash * 31) + v;
                }

                cur_x += chunk;
                remaining -= chunk;
                idx++;
             }

           tex->last_x_end[y] = sx + spans->len;
        }

        tex->span_counts[y] = idx;
        if (idx > sc->actual_max_spans)
          sc->actual_max_spans = idx;

        sc->flush_prev_y  = y;
        sc->flush_prev_ti = ti;

        spans++;
        count--;
     }

   // Flush the tail of the last row after the loop ends.
   // The row-change path above fires only when y changes, so the final
   // row (or the only row when the shape spans a single scanline) is
   // handled here.
   if ((sc->flush_prev_y >= 0) && (sc->flush_prev_ti >= 0))
     _flush_row_tail(sc, sc->flush_prev_ti, sc->flush_prev_y);
}

// ------------------------------------------------------------------
// Gradient span collector callback
// ------------------------------------------------------------------

// SW_FT_SpanFunc callback for LinearGradient and RadialGradient fills.
//
// Thin wrapper around _collect_spans_solid(): the span buffer format is
// identical (1 texel per span: gap, len, coverage, reserved).  Gradient
// colors are NOT stored per-span; instead the fragment shader computes
// them per-pixel using:
//   - a 1024x1 RGBA8 gradient ramp texture (uploaded by eng_ector_end)
//   - three float coefficients (a, b, c) where t = a*px + b*py + c
//     with px/py being FBO-space gl_FragCoord values.
//
// This function:
//   1. Sets sc->type to the gradient fill type (Linear or Radial).
//   2. Captures sc->gradient_data (the Ector_Renderer_Software_Gradient_Data*).
//   3. Captures sc->grad_offx/offy (ector surface origin in canvas space).
//   4. Delegates all span packing to _collect_spans_solid().
void
_collect_spans_gradient(int count, const SW_FT_Span *spans, void *user_data)
{
   Span_Data      *sd  = (Span_Data *)user_data;
   Span_Collector *sc  = (Span_Collector *)sd->span_collector;

   if (!sc) return;

    // Record the gradient type so eng_ector_end() knows to use the gradient
    // shader and to upload the ramp texture.
   sc->type          = sd->type;
   sc->gradient_data = sd->gradient;  // Ector_Renderer_Software_Gradient_Data*
   sc->grad_offx     = sd->offx;
   sc->grad_offy     = sd->offy;

    // Capture the per-shape inverse transform matrix.
    sc->inv = sd->inv;

   // Delegate: span buffer format is identical to solid fills.
   _collect_spans_solid(count, spans, user_data);
}

// ------------------------------------------------------------------
// Composite span collector callback and support query
// ------------------------------------------------------------------

// Return EINA_TRUE if the span-buffer GL path can handle @p comp_method
// natively via the fragment shader.
//
// Supported methods (shader handles the blend via branchless mix()):
//   MATTE_ALPHA         - result *= mask_a              (u_mask_inv = 0.0)
//   MATTE_ALPHA_INVERSE - result *= (1 - mask_a)        (u_mask_inv = 1.0)
//   MASK_INTERSECT      - result *= mask_a              (u_mask_inv = 0.0)
//   MASK_SUBSTRACT      - result *= (1 - mask_a)        (u_mask_inv = 1.0)
//
// INTERSECT uses the same shader math as MATTE_ALPHA; SUBSTRACT uses
// the same as MATTE_ALPHA_INVERSE.  The semantic difference (how
// multiple overlapping mask shapes interact) is handled by the mask
// FBO rendering phase, not the source shape shader.
//
// Additive / difference modes (u_mask_op selects the math):
//   MASK_ADD             - result.a = min(result.a + mask_a, 1.0) (u_mask_op = 1)
//   MASK_DIFFERENCE      - result *= abs(result.a - mask_a)       (u_mask_op = 2)
//
// All 6 composite methods are GPU-accelerated.  NONE falls through.
//
// Under SPAN_COLLECTOR_TEST_BUILD the real EFL enum values are not
// available, so the function always returns EINA_FALSE to avoid pulling
// in generated headers from the test binary.
Eina_Bool
span_collector_supports_composite(Efl_Gfx_Vg_Composite_Method comp_method)
{
#ifndef SPAN_COLLECTOR_TEST_BUILD
   switch (comp_method)
     {
      case EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA:
      case EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA_INVERSE:
      case EFL_GFX_VG_COMPOSITE_METHOD_MASK_ADD:
      case EFL_GFX_VG_COMPOSITE_METHOD_MASK_SUBSTRACT:
      case EFL_GFX_VG_COMPOSITE_METHOD_MASK_INTERSECT:
      case EFL_GFX_VG_COMPOSITE_METHOD_MASK_DIFFERENCE:
        return EINA_TRUE;
      default:
        return EINA_FALSE;
     }
#else
   (void)comp_method;
   return EINA_FALSE;
#endif
}

// SW_FT_SpanFunc callback for composite fills.
//
// Delegates span packing to either _collect_spans_gradient() or
// _collect_spans_solid() depending on sc->type.  The actual composite
// math (matte alpha, mask add/sub) happens in the fragment shader at
// draw time (Task 6), not here.  The engine binds both the shape's span
// texture and the composite source's span texture; the shader applies the
// operator per-pixel.
//
// When the composite source cannot be represented as a span buffer (e.g.,
// a nested composite whose result is only available as a rasterised FBO),
// eng_ector_end() detects this and falls back to the FBO pixel-buffer path
// before calling this function.
//
// @p user_data must point to a Span_Data whose span_collector field holds
// the active Span_Collector.
void
_collect_spans_composite(int count, const SW_FT_Span *spans, void *user_data)
{
   Span_Data      *sd = (Span_Data *)user_data;
   Span_Collector *sc = (Span_Collector *)sd->span_collector;

   if (!sc) return;

   // Set inv for both gradient and solid paths. The gradient callback
   // also sets it (redundant for that case), but _collect_spans_solid
   // does not, so we must set it here unconditionally.
   sc->inv = sd->inv;

   if ((sc->type == LinearGradient) || (sc->type == RadialGradient))
     _collect_spans_gradient(count, spans, user_data);
   else
     _collect_spans_solid(count, spans, user_data);
}

// ------------------------------------------------------------------
// GL shader / texture stubs (test build only)
// ------------------------------------------------------------------

// The real implementations live in evas_ector_gl_span_shader.c, which is
// compiled as part of the gl_generic engine module.  The stubs below are
// compiled ONLY when SPAN_COLLECTOR_TEST_BUILD is defined so that the
// unit-test binary (which does not link against GL) still links.
#ifdef SPAN_COLLECTOR_TEST_BUILD

Eina_Bool
span_shader_init(void)
{
   return EINA_FALSE;
}

void
span_shader_shutdown(void)
{
}

#endif // SPAN_COLLECTOR_TEST_BUILD
