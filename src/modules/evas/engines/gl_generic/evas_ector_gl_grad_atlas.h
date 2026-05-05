/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef EVAS_ECTOR_GL_GRAD_ATLAS_H
#define EVAS_ECTOR_GL_GRAD_ATLAS_H

#include <stdint.h>

/* In the normal engine build GLuint is provided by the system GL headers
 * pulled in by evas_gl_private.h (the .c file includes that before this
 * header).  In a unit-test build (SPAN_GRAD_ATLAS_TEST_BUILD) no GL headers
 * are on the path, so provide a minimal stub typedef. */
#ifdef SPAN_GRAD_ATLAS_TEST_BUILD
# ifndef SPAN_GRAD_ATLAS_GL_STUBS_DEFINED
#  define SPAN_GRAD_ATLAS_GL_STUBS_DEFINED
typedef unsigned int GLuint;
# endif
#endif

/* Gradient ramp atlas — fixed 1024×64 RGBA8 texture pool.
 *
 * Each row holds one resolved 1024-texel ramp.  Lookup is by
 *   (Efl_Vg_Gradient*, version)  — fast identity match
 * or by content hash with byte-compare fallback for collisions.
 *
 * The CPU-side mirror (256 KB) lives only for hash-collision
 * verification — it is never read back from the GPU.
 *
 * Single-threaded; no locking.
 */

#define SPAN_GRAD_ATLAS_W           1024
#define SPAN_GRAD_ATLAS_H             64
#define SPAN_GRAD_ATLAS_ROW_BYTES   (SPAN_GRAD_ATLAS_W * 4)

typedef struct _Span_Grad_Atlas_Row
{
   uint32_t hash;       /* hash of 4096-byte ramp */
   uint32_t version;    /* last_uploaded_version, piggybacks gradient counter */
   void    *grad_id;    /* gradient pointer for fast-path identity match */
   uint32_t last_used;  /* render-frame counter for LRU */
   int      occupied;   /* 0 = free row, 1 = occupied */
} Span_Grad_Atlas_Row;

typedef struct _Span_Grad_Atlas Span_Grad_Atlas;

struct _Span_Grad_Atlas
{
   GLuint               tex;                  /* GL texture handle, 0 until allocated */
   uint8_t             *cpu_mirror;           /* SPAN_GRAD_ATLAS_H * SPAN_GRAD_ATLAS_ROW_BYTES = 256 KB */
   Span_Grad_Atlas_Row  rows[SPAN_GRAD_ATLAS_H];
   uint32_t             current_frame;        /* monotonic, incremented per render pass */
   int                  disabled;             /* 1 if alloc failed; gradient path skips */
#ifdef SPAN_GRAD_ATLAS_TEST_BUILD
   int                  test_skip_gl;         /* bypass GL; uploads are no-ops */
#endif
};

/* Allocate the atlas.  Returns NULL on failure (caller falls back). */
Span_Grad_Atlas *span_grad_atlas_new(void);

/* Free GL resources and CPU mirror. */
void span_grad_atlas_free(Span_Grad_Atlas *a);

/* Begin a new render pass — bumps the LRU frame counter. */
void span_grad_atlas_frame_begin(Span_Grad_Atlas *a);

/* Hash 4096 bytes of ramp content.  Public so tests can reach it. */
uint32_t span_grad_atlas_hash(const uint8_t *bytes);

/* Look up or insert a ramp.  Returns row index 0..63 on success, -1 on
 * failure (atlas disabled).  On insert/refresh, uploads via
 * glTexSubImage2D and copies to the CPU mirror.
 *
 * @param a        atlas
 * @param grad_id  gradient identity (Efl_Vg_Gradient* or equivalent)
 * @param version  current version counter for this gradient
 * @param bytes    pointer to 4096 bytes of resolved RGBA8 ramp content
 */
int span_grad_atlas_lookup(Span_Grad_Atlas *a, void *grad_id,
                           uint32_t version, const uint8_t *bytes);

/* Convert a row index to the normalized texture-V coordinate to use
 * as an attribute: (row + 0.5) / SPAN_GRAD_ATLAS_H. */
static inline float
span_grad_atlas_row_to_v(int row)
{
   return ((float)row + 0.5f) / (float)SPAN_GRAD_ATLAS_H;
}

#ifdef SPAN_GRAD_ATLAS_TEST_BUILD
/* Test-only: bypass GL allocation; uploads are no-ops, lookup logic
 * still runs against the CPU mirror. */
void span_grad_atlas_test_enable(Span_Grad_Atlas *a);
#endif

#endif
