// Shared type definitions for the span-buffer GL batching path.
//
// This header is the single source of truth for:
//   - Span_Variant enum
//   - Span_Vertex_* interleaved vertex structs
//   - span_vertex_size() helper
//   - SPAN_PIPE_MAX_QUADS and SPAN_FILL_TYPE_GRADIENT_MIN macros
//
// Deliberately has NO dependency on sw_ft_raster.h, GL headers, or any
// EFL private header so it can be included from both gl_common (which
// cannot reach gl_generic's static-lib includes) and gl_generic.
//
// GLfloat is resolved via the GL headers that both callers include before
// reaching this header.  In test builds that lack GL headers, define
// GLfloat as float before including.

#ifndef EVAS_ECTOR_GL_SPAN_TYPES_H
#define EVAS_ECTOR_GL_SPAN_TYPES_H

# ifndef GL_FLOAT
// Provide GLfloat for standalone/test include contexts that don't include
// full GL headers before this header.  Real GL builds define GL_FLOAT
// (and therefore GLfloat) via their GL headers.
typedef float GLfloat;
# endif

// ---------------------------------------------------------------------------
// Fill-type threshold: Span_Data_Type values >= this are gradient types.
// Span_Data_Type: 1=Solid, 2=LinearGradient, 3=RadialGradient.
// evas_gl_context.c cannot include evas_ector_gl_span.h (sw_ft_raster.h
// dependency), so it tests fill.type against this macro instead of the enum.
// ---------------------------------------------------------------------------
#define SPAN_FILL_TYPE_GRADIENT_MIN 2

// ---------------------------------------------------------------------------
// Gradient type carried in the .w of a side's grad_def attribute:
//   0 linear, 1 radial, 2 solid.
//
// "Solid" is how a plain colour rides in a gradient variant.  A shape with a
// gradient fill and a solid stroke would otherwise need two programs and so
// two draw calls; encoding the solid side as a degenerate gradient lets one
// draw cover both.  Such a side puts its premultiplied colour in the four
// components of grad_abc_y - the slots a linear gradient uses for its
// coefficients and ramp row - and the shader takes it verbatim instead of
// sampling the ramp atlas.
// ---------------------------------------------------------------------------
#define SPAN_GRAD_TYPE_LINEAR 0
#define SPAN_GRAD_TYPE_RADIAL 1
#define SPAN_GRAD_TYPE_SOLID  2

// ---------------------------------------------------------------------------
// Span_Variant — selects the interleaved vertex layout for a given draw call.
//
// SOLID         — no gradient, no mask
// SOLID_MASK    — no gradient, composite mask present
// GRADIENT      — gradient fill or stroke, no mask
// GRADIENT_MASK — gradient fill or stroke, composite mask present
// ---------------------------------------------------------------------------
typedef enum
{
   SPAN_VARIANT_SOLID         = 0,
   SPAN_VARIANT_SOLID_MASK    = 1,
   SPAN_VARIANT_GRADIENT      = 2,
   SPAN_VARIANT_GRADIENT_MASK = 3,
   SPAN_VARIANT_COUNT
} Span_Variant;

// ---------------------------------------------------------------------------
// Per-variant interleaved vertex structs for the span attribute-batching path.
//
// All sizes are in GLfloat units (4 bytes each).  Byte totals are shown in
// the trailing comments.  Layout must stay in sync with the vertex shader
// attribute offsets documented in:
//   docs/superpowers/specs/2026-05-05-span-gl-attribute-batching-design.md
//
// mask_comp_inv[2]:
//   [0] — raw comp_method (Efl_Gfx_Vg_Composite_Method enum value, cast to GLfloat)
//   [1] — mask_inv flag (1.0 if the mask alpha should be inverted, 0.0 otherwise)
// ---------------------------------------------------------------------------

// Common fields shared by all four variants. 16f = 64 B
typedef struct
{
   GLfloat pos[2];
   GLfloat fbo_fill_off[4];
   GLfloat stroke_off_flags[4];
   GLfloat x_min[2];
   GLfloat mul_col[4];
} Span_Vertex_Common;

// Solid fill, no mask. 24f = 96 B
typedef struct
{
   Span_Vertex_Common c;
   GLfloat            fill_col[4];
   GLfloat            stroke_col[4];
} Span_Vertex_Solid;

// Solid fill with composite mask. 30f = 120 B
typedef struct
{
   Span_Vertex_Solid s;
   GLfloat           mask_off_size[4];
   GLfloat           mask_comp_inv[2];
} Span_Vertex_Solid_Mask;

// Gradient fill or stroke, no mask. 40f = 160 B
typedef struct
{
   Span_Vertex_Common c;
   GLfloat            fill_grad_abc_y[4];
   GLfloat            fill_grad_def[4];
   GLfloat            fill_grad_radial[4];
   GLfloat            stroke_grad_abc_y[4];
   GLfloat            stroke_grad_def[4];
   GLfloat            stroke_grad_radial[4];
} Span_Vertex_Gradient;

// Gradient fill or stroke with composite mask. 46f = 184 B
typedef struct
{
   Span_Vertex_Gradient g;
   GLfloat              mask_off_size[4];
   GLfloat              mask_comp_inv[2];
} Span_Vertex_Gradient_Mask;

// Return the per-vertex byte size for the given variant.
static inline size_t
span_vertex_size(Span_Variant v)
{
   switch (v)
     {
      case SPAN_VARIANT_SOLID:         return sizeof(Span_Vertex_Solid);
      case SPAN_VARIANT_SOLID_MASK:    return sizeof(Span_Vertex_Solid_Mask);
      case SPAN_VARIANT_GRADIENT:      return sizeof(Span_Vertex_Gradient);
      case SPAN_VARIANT_GRADIENT_MASK: return sizeof(Span_Vertex_Gradient_Mask);
      default:                         return sizeof(Span_Vertex_Solid);
     }
}

// Maximum number of quads per pipe entry in the span attribute-batching path.
#define SPAN_PIPE_MAX_QUADS 1024

#endif // EVAS_ECTOR_GL_SPAN_TYPES_H
