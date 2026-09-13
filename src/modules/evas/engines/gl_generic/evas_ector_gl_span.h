// Span-buffer collector for the Ector GL engine.
//
// The SW_FT_Raster software rasterizer is kept untouched.  Three collector
// callbacks replace the pixel-blending callbacks (_blend_argb,
// _blend_gradient) and pack SW_FT_Span arrays into a row-major byte buffer
// inside one or more Span_Texture slots.  eng_ector_begin()/eng_ector_end()
// then upload the buffer as a GL texture and draw a quad with a span-lookup
// fragment shader.
//
// GL types (GLuint, etc.) are deliberately avoided here so that this header
// can be included from non-GL translation units.  tex_id is declared as
// unsigned int; the GL upload code casts where needed.

#ifndef EVAS_ECTOR_GL_SPAN_H_
#define EVAS_ECTOR_GL_SPAN_H_

// This header requires sw_ft_raster.h on the include path
// (src/static_libs/freetype, ensured by the gl_generic meson.build).
//
// ector_software_private.h is NOT included here because it transitively
// pulls in generated EFL headers that are only available on the full
// engine build path.  The .c implementation includes it directly.
//
// Span_Data and Span_Data_Type are forward-declared below; callers that
// need the full struct definition must include ector_software_private.h
// themselves before including this header.
#include "sw_ft_raster.h"

// ------------------------------------------------------------------
// Forward declarations for ector_software_private.h types
// ------------------------------------------------------------------

// If ector_software_private.h has already been included its include guard
// ECTOR_SOFTWARE_PRIVATE_H_ is defined, so these declarations are skipped.
// Otherwise we provide the minimum needed for this header's API surface.
#ifndef ECTOR_SOFTWARE_PRIVATE_H_

// Span_Data_Type: canonical definition lives in ector_software_private.h.
// This copy is only compiled in test builds (SPAN_COLLECTOR_TEST_BUILD) where
// the full ector private headers are unavailable, so the include guard above is
// never set and we need the enum here to satisfy the span-collector API.
typedef enum _Span_Data_Type
{
   None           = 0,
   Solid          = 1,
   LinearGradient = 2,
   RadialGradient = 3,
} Span_Data_Type;

// Span_Data struct: full definition when building in test mode so test code
// can declare stack instances and drive _collect_spans_solid directly.
// Must mirror the fields that _collect_spans_solid actually reads:
//   offx, offy     — ector surface offset (zero in unit tests)
//   mul_col        — multiplicative tint; set to 0xFFFFFFFF for identity
//   color (union)  — premultiplied ARGB base color for Solid fills
//   type           — fill type (Solid / LinearGradient / RadialGradient)
//   span_collector — pointer to the active Span_Collector
// In normal engine builds the full definition comes from ector_software_private.h.
# ifdef SPAN_COLLECTOR_TEST_BUILD
struct _Span_Data
{
   void    *raster_buffer;  // unused in tests; keeps struct layout sane
   void    *blend;          // unused in tests
   void    *unclipped_blend; // unused in tests
   int      offx, offy;     // ector surface offset — set to 0 in tests
   void    *clip;           // unused in tests
   int      type;           // Span_Data_Type cast — Solid=1, etc.
   uint32_t mul_col;        // multiplicative tint; 0xFFFFFFFF = identity
   int      op;             // render op — unused in tests
   union {
      uint32_t color;       // premultiplied ARGB for Solid fills
      void    *gradient;    // gradient pointer — unused in span tests
      void    *buffer;      // unused in tests
   };
   void    *span_collector; // active Span_Collector *
   Eina_Matrix3 inv;        // inverse transform matrix — identity in tests
};

// DRAW_MUL4_SYM: symmetric 8-bit channel multiply used to composite mul_col
// onto the base color.  In test mode we provide the same formula as draw.h
// so that _collect_spans_solid can call it without pulling in draw.h.
#  define DRAW_MUL4_SYM(x, y) \
    ( ((((((x) >> 16) & 0xff00) * (((y) >> 16) & 0xff00)) + 0xff0000) & 0xff000000) + \
      ((((((x) >> 8) & 0xff00) * (((y) >> 16) & 0xff)) + 0xff00) & 0xff0000) + \
      ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff0000) >> 16) & 0xff00) + \
      (((((x) & 0xff) * ((y) & 0xff)) + 0xff) >> 8) )

typedef int Efl_Gfx_Vg_Composite_Method;
# endif // SPAN_COLLECTOR_TEST_BUILD
typedef struct _Span_Data Span_Data;

#endif // !ECTOR_SOFTWARE_PRIVATE_H_

// ------------------------------------------------------------------
// Constants
// ------------------------------------------------------------------

// Default maximum number of spans packed per row in a single texture.
#define SPAN_COLLECTOR_DEFAULT_MAX_SPANS 64

// Initial x_max value for the primary Span_Texture created in
// span_collector_new().  Covers the full expected canvas width before any
// spatial splits have occurred.
#define SPAN_TEXTURE_X_MAX_INITIAL 65535

// Maximum number of spatial splits.  When the rasteriser produces spans
// wider than a single texture can accommodate, the x-range is divided into
// at most this many non-overlapping sub-ranges, each backed by its own
// Span_Texture.
#define SPAN_COLLECTOR_MAX_SPLITS        8

// ------------------------------------------------------------------
// Forward declarations
// ------------------------------------------------------------------

typedef struct _Span_Texture   Span_Texture;
typedef struct _Span_Collector Span_Collector;

// ------------------------------------------------------------------
// Span_Texture
// ------------------------------------------------------------------

// One GPU-uploadable span buffer covering a horizontal sub-range of the
// canvas.  The buffer is row-major: row y starts at buffer + y * stride.
// Each row holds up to max_spans packed span entries followed by a
// zero-length sentinel.
//
// Each entry is 1 texel (4 bytes) for all fill types (Solid, LinearGradient,
// RadialGradient):
//   byte 0 (BGRA B): coverage — AA coverage 0-255
//   byte 1 (BGRA G): len      — span length (max 255; longer spans are split)
//   byte 2 (BGRA R): gap      — distance from end of previous span on this row
//   byte 3 (BGRA A): reserved — zero
//
// The buffer is stored in BGRA-swapped byte order for direct upload.
// The shader reads: .r=gap, .g=len, .b=coverage.
//
// For Solid fills: the base color is passed as a shader uniform.
// For LinearGradient / RadialGradient fills: the gradient ramp texture
// (1024×1 RGBA8) and per-pixel t-computation coefficients are passed as
// shader uniforms.  No per-span color data is stored in the span buffer.
//
// The rows do not get a GPU texture of their own.  Every Span_Texture of
// every collector in one render pass is packed into a single shared page
// (Span_Page below) and uploaded in one call, because the span upload cost
// on a tiled GPU is dominated by the number of glTexSubImage2D calls rather
// than by the bytes they carry.  page_x/page_y are this texture's texel
// offset inside that page, and are what the shader receives as the fill or
// stroke offset.  They are only valid after span_page_upload().
struct _Span_Texture
{
   uint8_t      *buffer;      // row-major span buffer, height * stride bytes
   int          *span_counts; // per-row count of packed spans, height ints
   int          *last_x_end;  // per-row: x coord where last span ended (for gap calc)
   Eina_Bool     dirty;       // EINA_TRUE if span data changed since last upload
   uint32_t      prev_hash;   // hash of buffer content at last upload, for change detection
   uint32_t      rolling_hash;  // accumulated during span collection
   int           x_min;       // inclusive left edge of the x-range covered
   int           x_max;       // inclusive right edge of the x-range covered
   int           page_x;      // texel offset of these rows inside the shared page
   int           page_y;
};

// ------------------------------------------------------------------
// Span_Collector
// ------------------------------------------------------------------

// Aggregates span data produced by the SW_FT_Raster into one or more
// Span_Texture buffers ready for GPU upload.
//
// A single Span_Collector is created per eng_ector_begin() call and freed
// (or cleared and reused) at eng_ector_end().
struct _Span_Collector
{
   Span_Texture  *textures;      // dynamic array of texture slots
   int            texture_count; // number of active entries in textures[]

   // X-coordinates at which the canvas is split into separate textures.
   // split_points[i] is the x_min of texture i+1 (i.e., the exclusive
   // right boundary of texture i).  split_count is the number of splits
   // recorded so far, up to SPAN_COLLECTOR_MAX_SPLITS.
   int            split_points[SPAN_COLLECTOR_MAX_SPLITS];
   int            split_count;

   int            max_spans;       // max spans per row per texture (default 32)
   int            h;               // active height this frame (VG object height)
   int            alloc_h;         // allocated buffer height (high-water mark, never shrinks)
   int            stride;          // bytes per row = (max_spans + 1) * 4
                                   // The +1 reserves a dedicated sentinel slot.
   int            actual_max_spans; // max span_counts[y] seen during collection this frame

   // Row-tail flush state — tracked across multiple _collect_spans_solid
   // invocations (e.g., when _span_fill_clipRect calls the callback in
   // chunks).  Reset in span_collector_clear.
   int            flush_prev_y;    // last row flushed (-1 = none)
   int            flush_prev_ti;   // texture index of last flushed row

   // Fill parameters captured at span_collector_new() time or during collection
   Span_Data_Type type;            // Solid, LinearGradient, or RadialGradient
   uint32_t       color;           // premultiplied ARGB (0xAARRGGBB) for Solid fills

   // Gradient data pointer set during _collect_spans_gradient().
   // Points into the active Ector_Renderer_Software_Gradient_Data for this
   // draw call.  Only valid between eng_ector_begin() and eng_ector_end().
   // Used by eng_ector_end() to upload the color ramp and compute t-coefficients.
   void          *gradient_data;   // Ector_Renderer_Software_Gradient_Data* or NULL

   // Ector surface offset captured during _collect_spans_gradient().
   // These are the x/y values passed to ector_surface_reference_point_set().
   // Needed to fold the local→canvas translation into the t-coefficients.
   int            grad_offx;
   int            grad_offy;

   // Per-shape inverse transform matrix captured during gradient/composite collection.
   // Copy of sd->inv from the current shape being rasterized.  Used by
   // eng_ector_end() to compute gradient t-coefficients with the correct transform.
   Eina_Matrix3   inv;

   // Composite/mask parameters set by _collect_spans_composite() when a shape
   // has an active composite mask.  Both default to 0/NULL for non-masked shapes.
   void          *mask_surface;   // Evas_GL_Image* for the mask FBO (NULL = no mask)
   int            comp_method;    // Efl_Gfx_Vg_Composite_Method (0 = NONE)

};

// ------------------------------------------------------------------
// Lifecycle
// ------------------------------------------------------------------

// Allocate and initialise a new Span_Collector.
//
// @param h          Canvas height in pixels.  Determines per-texture buffer
//                   allocation.
// @param max_spans  Maximum spans packed per row.  Pass
//                   SPAN_COLLECTOR_DEFAULT_MAX_SPANS unless you have a
//                   measured reason to use a different value.
// @param type       Fill type; controls which collector
//                   callback is active.
// @return           Newly allocated collector, or NULL on allocation failure.
Span_Collector *span_collector_new(int h, int max_spans, Span_Data_Type type);

// Free all resources owned by @p sc including texture buffers and any
// GL textures that have been uploaded.  The pointer itself is freed; @p sc
// must not be used after this call.
void span_collector_free(Span_Collector *sc);

// Prepare @p sc for a new frame.
//
// Resets span_counts and last_x_end arrays to zero and clears the dirty
// flag on all textures.  Uploaded textures are kept alive for potential
// reuse via the content-hash check in upload_textures().
void span_collector_clear(Span_Collector *sc);

// Resize a collector for a new active height.
//
// Follows the Evas high-water mark pattern: buffers grow via realloc when
// @p h exceeds alloc_h, but never shrink.  When @p h is within
// alloc_h, only the active height is updated — no allocation.
void span_collector_resize(Span_Collector *sc, int h);

// ------------------------------------------------------------------
// Collector callbacks (SW_FT_SpanFunc signature)
// ------------------------------------------------------------------

// SW_FT_SpanFunc callback for Solid fills.
//
// Packs each span as 2 texels into the appropriate Span_Texture row.
// @p user_data must point to the Span_Data whose span_collector field
// references the active Span_Collector.
void _collect_spans_solid(int count, const SW_FT_Span *spans, void *user_data);

// SW_FT_SpanFunc callback for LinearGradient and RadialGradient fills.
//
// Thin wrapper around _collect_spans_solid(): packs spans in the same
// 1-texel format (gap, len, coverage, reserved).  The gradient shader
// computes per-pixel colors from a 1024×1 ramp texture and per-frame
// t-computation coefficients passed as uniforms — no per-span color data
// is written into the span buffer.
//
// Sets sc->type = sd->type and captures sc->gradient_data and
// sc->grad_offx/offy so that eng_ector_end() can upload the ramp and
// compute the t-coefficients.
//
// @p user_data must point to the Span_Data whose span_collector field
// references the active Span_Collector.
void _collect_spans_gradient(int count, const SW_FT_Span *spans, void *user_data);

// SW_FT_SpanFunc callback for composite (mask/clip) passes.
//
// Used when a shape is drawn with an Efl_Gfx_Vg_Composite_Method other
// than the default.  The composite mask coverage is packed alongside the
// base color so the fragment shader can apply the operator.
// @p user_data must point to the Span_Data whose span_collector field
// references the active Span_Collector.
void _collect_spans_composite(int count, const SW_FT_Span *spans, void *user_data);

// ------------------------------------------------------------------
// Composite support query
// ------------------------------------------------------------------

// Return EINA_TRUE if the span-buffer GL path supports the given composite
// method natively.  When EINA_FALSE is returned the engine should fall back
// to the pixel-buffer upload path for this operation.
//
// @param comp_method  The composite method to test.
// @return             EINA_TRUE if supported, EINA_FALSE otherwise.
Eina_Bool span_collector_supports_composite(Efl_Gfx_Vg_Composite_Method comp_method);

// ------------------------------------------------------------------
// Texture management
// ------------------------------------------------------------------

// One GPU texture holding the span rows of every collector in a render pass.
//
// Owned by the engine and reused across passes, following the same
// high-water discipline as the CPU-side buffers: it grows when a pass needs
// more room and never shrinks.  Because it is shared, a pass overwrites the
// previous pass's rows, which is why eng_ector_begin() drains queued draws
// before collecting again.
typedef struct _Span_Page
{
   void     *evas_tex;   // Evas_GL_Texture *; NULL until first upload
   void     *gc;         // the Evas_Engine_GL_Context evas_tex belongs to
   int       w, h;       // logical size currently allocated
   uint32_t  prev_hash;  // combined hash of the last uploaded pass
} Span_Page;

// Allocate an empty page.  No GL resource is taken until first upload.
Span_Page *span_page_new(void);

// Free @p page.
//
// @p release_tex says whether the GL texture may still be touched.  It comes
// from the context's texture pool, so once that context is gone the pool has
// already freed it and releasing it again is a use-after-free.  Pass
// EINA_FALSE when tearing down after the context has been destroyed.
//
// Must be called from the GL thread.
void span_page_free(Span_Page *page, Eina_Bool release_tex);

// Pack the span rows of every collector in @p fills and @p strokes into
// @p page and upload them with a single glTexSubImage2D.
//
// Each collector's Span_Texture gets its page_x/page_y assigned.  When the
// combined content and layout match the previous pass the upload is skipped,
// so unchanging shapes cost nothing.
//
// Must be called from the GL thread (i.e., inside eng_ector_end()).
//
// @param gc_ptr  Evas_Engine_GL_Context *.
// @return EINA_TRUE when the page holds valid rows and page_x/page_y are set.
Eina_Bool span_page_upload(void *gc_ptr, Span_Page *page,
                           void **fills, int nfills,
                           void **strokes, int nstrokes);

// GL texture name backing @p page, or 0 if it has none yet.
unsigned int span_page_tex_id(const Span_Page *page);

// Pool dimensions of the texture backing @p page (for the shader's 1/w, 1/h).
void span_page_pool_size(const Span_Page *page, int *w, int *h);

// ------------------------------------------------------------------
// Span-lookup shader
// ------------------------------------------------------------------

// Compile and link the span-lookup GLSL programs (solid + gradient).
//
// Uses file-scope static shader state.  Safe to call multiple times;
// subsequent calls are no-ops if the programs are already compiled.
// Must be called from the GL thread.
//
// @return EINA_TRUE on success, EINA_FALSE if compilation fails.
Eina_Bool span_shader_init(void);

// EINA_TRUE when the span rendering path may be used: the tier setting
// permits it and the shaders linked.  When EINA_FALSE the caller must not
// install span collectors.
Eina_Bool span_path_usable(void);

// Delete the GL programs and reset internal shader state.
// Must be called from the GL thread.
void span_shader_shutdown(void);

// span_pass_draw() uses Evas_Engine_GL_Context which is only
// available when evas_gl_common.h has been included before this header.
// Guard the declaration so non-GL translation units (e.g., unit tests)
// can still include this header without the full GL context definition.
#ifdef EVAS_GL_COMMON_H

// Render one VG object's span quads straight into @p target's framebuffer.
//
// Deliberately does not go through a pipe entry.  A pipe entry is flushed
// with whatever surface the pipe is targeting, so putting the VG pass there
// forced evas_gl_common_context_target_surface_set() around every vector
// object - and that flushes, which meant each object's composite quad was
// drawn on its own instead of batching with the rest of the canvas.
//
// Clears (@p clear_x, @p clear_y, @p clear_w, @p clear_h) in framebuffer
// coordinates first, then draws @p n quads, grouping consecutive ones that
// share a program and bindings into single draws.  The pipe's framebuffer
// and viewport are restored before returning; its queued contents are left
// untouched.
void span_pass_draw(Evas_Engine_GL_Context *gc, Evas_GL_Image *target,
                    const Span_Pipe_Params *quads, const GLfloat *ndc, int n,
                    int clear_x, int clear_y, int clear_w, int clear_h);

// Debug helper: read a single pixel from a GL texture via a temp FBO.
// Logs the RGBA values with a caller-supplied label.  Throttled to avoid
// log flooding (max 30 calls).  Only available when compiled with
// -DSPAN_DEBUG_PROBES=1.
//
// @param label   Human-readable checkpoint name (e.g. "after_upload").
// @param tex_id  GL texture name to read from.
// @param px_x    X coordinate within the texture.
// @param px_y    Y coordinate within the texture.
#ifdef SPAN_DEBUG_PROBES
void span_debug_readback(const char *label, unsigned int tex_id,
                         int px_x, int px_y);
#endif // SPAN_DEBUG_PROBES
#endif // EVAS_GL_COMMON_H

// ------------------------------------------------------------------
// Per-variant interleaved vertex structs (Task 3)
// ------------------------------------------------------------------

// Per-variant vertex types, SPAN_PIPE_MAX_QUADS, SPAN_FILL_TYPE_GRADIENT_MIN.
// Single source of truth shared with gl_common (no sw_ft_raster.h dependency).
#include "../gl_common/evas_ector_gl_span_types.h"

#endif // EVAS_ECTOR_GL_SPAN_H_
