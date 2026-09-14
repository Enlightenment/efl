#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef SPAN_GRAD_ATLAS_TEST_BUILD
// Normal engine build: include the full evas GL private headers so that
// GL functions (glGenTextures etc.) are available.
# include "evas_gl_private.h"
// eina_crc() is a static inline in eina_crc.h; the full Eina headers are
// already reachable via evas_gl_private.h -> evas_private.h -> Eina.h.
# include <eina_crc.h>
#else
// Test build: no real GL context.  Pull in Eina.h for basic types.
// GL type stubs (GLuint) are provided by evas_ector_gl_grad_atlas.h
// when SPAN_GRAD_ATLAS_TEST_BUILD is defined.
# include <Eina.h>
# include <eina_crc.h>
// Stub out all GL functions used by the implementation - in test mode
// _ensure_gl short-circuits before any GL call, and _upload_row skips
// the GL path, so these are never reached.
# define glGenTextures(n, ids)        ((void)0)
# define glDeleteTextures(n, ids)     ((void)0)
# define glBindTexture(t, id)         ((void)0)
# define glTexImage2D(...)            ((void)0)
# define glTexSubImage2D(...)         ((void)0)
# define glTexParameteri(...)         ((void)0)
# define GL_TEXTURE_2D                0x0DE1
# define GL_RGBA                      0x1908
# define GL_UNSIGNED_BYTE             0x1401
# define GL_LINEAR                    0x2601
# define GL_TEXTURE_MIN_FILTER        0x2801
# define GL_TEXTURE_MAG_FILTER        0x2800
# define GL_CLAMP_TO_EDGE             0x812F
# define GL_TEXTURE_WRAP_S            0x2802
# define GL_TEXTURE_WRAP_T            0x2803
#endif

#include "evas_ector_gl_grad_atlas.h"

// CRC32 over 4096 bytes using eina_crc() (SSE4.2-accelerated when
// available - measured ~2.6x faster than FNV-1a on 4 KB ramps).
// Cache lookup uses byte-compare on hit to defend against collisions.
uint32_t
span_grad_atlas_hash(const uint8_t *bytes)
{
   return (uint32_t)eina_crc((const char *)bytes,
                             SPAN_GRAD_ATLAS_ROW_BYTES,
                             0xffffffffU,
                             EINA_TRUE);
}

Span_Grad_Atlas *
span_grad_atlas_new(void)
{
   Span_Grad_Atlas *a = calloc(1, sizeof(*a));
   if (!a) return NULL;

   a->cpu_mirror = malloc((size_t)SPAN_GRAD_ATLAS_H * SPAN_GRAD_ATLAS_ROW_BYTES);
   if (!a->cpu_mirror)
     {
        free(a);
        return NULL;
     }
   // GL allocation is deferred to first lookup so this is callable
   // outside an active GL context (e.g. unit tests).  See _ensure_gl().
   return a;
}

void
span_grad_atlas_free(Span_Grad_Atlas *a)
{
   if (!a) return;
   if (a->tex) glDeleteTextures(1, &a->tex);
   free(a->cpu_mirror);
   free(a);
}

void
span_grad_atlas_frame_begin(Span_Grad_Atlas *a)
{
   if (!a) return;
   a->current_frame++;
}

// Lazily create the GL texture on first use.  Returns 1 on success, 0 on
// failure (caller marks atlas disabled).
static int
_ensure_gl(Span_Grad_Atlas *a)
{
#ifdef SPAN_GRAD_ATLAS_TEST_BUILD
   if (a->test_skip_gl) return 1;
#endif
   if (a->tex) return 1;
   if (a->disabled) return 0;

   GLuint t = 0;
   glGenTextures(1, &t);
   if (!t) { a->disabled = 1; return 0; }
   glBindTexture(GL_TEXTURE_2D, t);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                SPAN_GRAD_ATLAS_W, SPAN_GRAD_ATLAS_H, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
   a->tex = t;
   return 1;
}

void
span_grad_atlas_flush_cb_set(Span_Grad_Atlas *a,
                             void (*cb)(void *data), void *data)
{
   if (!a) return;
   a->flush_cb   = cb;
   a->flush_data = data;
}

// Find row by (grad_id, version) - O(64). Returns row idx or -1.
static int
_find_identity(Span_Grad_Atlas *a, void *grad_id, uint32_t version)
{
   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     if (a->rows[i].occupied &&
         (a->rows[i].grad_id == grad_id) &&
         (a->rows[i].version == version))
       return i;
   return -1;
}

// Find row whose hash matches AND whose CPU mirror byte-equals bytes.
// Returns row idx or -1.  Distinguishes true hits from hash collisions.
static int
_find_by_content(Span_Grad_Atlas *a, uint32_t hash, const uint8_t *bytes)
{
   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     {
        if (!a->rows[i].occupied) continue;
        if (a->rows[i].hash != hash) continue;
        if (!memcmp(a->cpu_mirror + ((size_t)i * SPAN_GRAD_ATLAS_ROW_BYTES),
                    bytes, SPAN_GRAD_ATLAS_ROW_BYTES))
          return i;
     }
   return -1;
}

// Free row, else evict the least recently used row that is NOT already in
// use by the current pass.
//
// Rows stamped with current_frame are referenced by draws that have been
// recorded but not yet submitted - span pushes are batched and flushed at
// the end of the pass, and grad_ramp_y travels per-vertex.  Overwriting such
// a row makes the earlier shape sample the newer ramp.  When every row is
// pinned, drain the pending draws first; after that the rows are free to
// reuse.
//
// This also repairs a degenerate LRU: last_used is a per-pass counter, so
// once the atlas fills within one pass every row compares equal and the
// strict < below never beats index 0, meaning the pass evicted its own
// earliest row every time.
static int
_alloc_row(Span_Grad_Atlas *a)
{
   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     if (!a->rows[i].occupied) return i;

   int      best     = -1;
   uint32_t best_age = 0;
   for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
     {
        if (a->rows[i].last_used == a->current_frame) continue; // pinned
        if ((best < 0) || (a->rows[i].last_used < best_age))
          { best = i; best_age = a->rows[i].last_used; }
     }
   if (best >= 0) return best;

   // Every row is pinned by this pass.  Drain the pending draws, then no
   // unflushed draw references any row any more - age every pinned row by
   // one so ordinary LRU resumes for the rest of this pass instead of
   // flushing again on the very next miss (which would turn a heavy-gradient
   // pass into one pipe flush per gradient).
   if (a->flush_cb)
     {
        a->flush_cb(a->flush_data);
        for (int i = 0; i < SPAN_GRAD_ATLAS_H; i++)
          if (a->rows[i].last_used == a->current_frame)
            // current_frame is uint32_t; guard against underflow when this
            // branch is reached before the first frame_begin() (current_frame
            // == 0).  Without the guard every row's age would wrap to
            // UINT32_MAX and never be beaten again, flattening the LRU for
            // the atlas's lifetime.
            a->rows[i].last_used = a->current_frame ? (a->current_frame - 1) : 0;

        best     = 0;
        best_age = a->rows[0].last_used;
        for (int i = 1; i < SPAN_GRAD_ATLAS_H; i++)
          if (a->rows[i].last_used < best_age)
            { best = i; best_age = a->rows[i].last_used; }
        return best;
     }

   // No callback registered: nothing was drained, so no row is actually
   // safe to reuse.  There is no way to make progress otherwise (the
   // gradient path has no failure mode for "atlas full mid-pass"), so
   // return row 0 anyway - this is the same degenerate-but-non-crashing
   // fallback the atlas has always had for an unrecoverable situation.
   return 0;
}

// Upload bytes to row idx via glTexSubImage2D and copy to mirror.
//
// @p bytes points at native uint32 ARGB content (same layout as
// gd->color_table).  The ARGB->RGBA byte-swap for the GL upload is
// done here on a stack staging buffer so that callers never need a
// separate rearrangement pass - the cpu_mirror stores the native
// layout too, keeping hash/memcmp consistent.
static void
_upload_row(Span_Grad_Atlas *a, int row, const uint8_t *bytes)
{
#ifdef SPAN_GRAD_ATLAS_TEST_BUILD
   if (a->test_skip_gl) goto mirror_only;
#endif
   if (a->tex)
     {
        // Rearrange ARGB native->RGBA for GL only at upload time.
        uint32_t staging[SPAN_GRAD_ATLAS_W];
        const uint32_t *src = (const uint32_t *)bytes;
        for (int j = 0; j < SPAN_GRAD_ATLAS_W; j++)
          {
             uint32_t c = src[j];
             uint8_t *p = (uint8_t *)&staging[j];
             p[0] = (c >> 16) & 0xFF; // R
             p[1] = (c >>  8) & 0xFF; // G
             p[2] =  c        & 0xFF; // B
             p[3] = (c >> 24) & 0xFF; // A
          }
        glBindTexture(GL_TEXTURE_2D, a->tex);
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        0, row, SPAN_GRAD_ATLAS_W, 1,
                        GL_RGBA, GL_UNSIGNED_BYTE, staging);
     }
#ifdef SPAN_GRAD_ATLAS_TEST_BUILD
mirror_only:
#endif
   memcpy(a->cpu_mirror + ((size_t)row * SPAN_GRAD_ATLAS_ROW_BYTES),
          bytes, SPAN_GRAD_ATLAS_ROW_BYTES);
}

int
span_grad_atlas_lookup(Span_Grad_Atlas *a, void *grad_id,
                       uint32_t version, const uint8_t *bytes)
{
   if (!a || a->disabled) return -1;

   // Identity fast path.
   int row = _find_identity(a, grad_id, version);
   if (row >= 0)
     {
        a->rows[row].last_used = a->current_frame;
        return row;
     }

   // Hash + byte-compare path.
   // version == span_grad_atlas_hash(bytes) by contract: callers compute it
   // via span_grad_atlas_hash() before calling lookup, so recomputing here
   // is redundant.  Use version directly.
   uint32_t h = version;
   row = _find_by_content(a, h, bytes);
   if (row >= 0)
     {
        a->rows[row].grad_id   = grad_id;
        a->rows[row].version   = version;
        a->rows[row].last_used = a->current_frame;
        return row;
     }

   // Miss - allocate or evict, upload.
   if (!_ensure_gl(a)) return -1;
   row = _alloc_row(a);
   _upload_row(a, row, bytes);
   a->rows[row].hash      = h;
   a->rows[row].version   = version;
   a->rows[row].grad_id   = grad_id;
   a->rows[row].last_used = a->current_frame;
   a->rows[row].occupied  = 1;
   return row;
}

#ifdef SPAN_GRAD_ATLAS_TEST_BUILD
void
span_grad_atlas_test_enable(Span_Grad_Atlas *a)
{
   if (!a) return;
   a->test_skip_gl = 1;
}
#endif
