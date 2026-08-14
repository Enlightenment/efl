/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Span-lookup GLSL shaders for the Ector GL engine.
 *
 * Provides shader compilation, texture upload, and draw functions for the
 * span-buffer GL rendering path.  These functions are the real implementations
 * of the stubs declared in evas_ector_gl_span.h; the stubs in
 * evas_ector_gl_span.c are compiled only under SPAN_COLLECTOR_TEST_BUILD.
 *
 * GL function access: evas_gl_private.h -> evas_gl_common.h pulls in
 * <GLES2/gl2.h> (when GL_GLES) or the desktop GL headers, providing all
 * glCreateShader/glGenTextures/etc. symbols directly.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "evas_common_private.h"
#include "evas_gl_private.h"
#include "evas_ector_log_restore.h"

#include "evas_ector_gl_span.h"
#include "evas_ector_gl_grad_atlas.h"

/* Scratch buffer for the no-EXT_unpack_subimage upload path: span rows are
 * copied here tightly packed so one glTexSubImage2D covers the whole
 * sub-rect.  Grown on demand, never shrunk; freed in span_shader_shutdown. */
static uint8_t *_span_pack_buf = NULL;
static size_t   _span_pack_sz  = 0;

static uint8_t *
_span_pack_buf_get(size_t need)
{
   if (need > _span_pack_sz)
     {
        uint8_t *p = realloc(_span_pack_buf, need);
        if (!p) return NULL;
        _span_pack_buf = p;
        _span_pack_sz  = need;
     }
   return _span_pack_buf;
}

/* ------------------------------------------------------------------ */
/* GLSL shader source strings                                          */
/* ------------------------------------------------------------------ */

/* ------------------------------------------------------------------ */
/* Varying declaration blocks (VS writes, FS reads)                    */
/* ------------------------------------------------------------------ */

/* Common varyings present in all variants. */
static const char _glsl_varyings_common[] =
   "varying SPAN_HP vec2 v_fbo_off;\n"
   "varying SPAN_HP vec2 v_fill_off;\n"
   "varying SPAN_HP vec2 v_stroke_off;\n"
   "varying SPAN_HP float v_max_spans;\n"
   "varying SPAN_HP float v_has_flags;\n"  /* (has_fill | has_stroke<<1) as float */
   "varying SPAN_HP float v_fill_x_min;\n"
   "varying SPAN_HP float v_stroke_x_min;\n"
   "varying mediump vec4 v_mul_col;\n";

static const char _glsl_varyings_solid[] =
   "varying mediump vec4 v_fill_col;\n"
   "varying mediump vec4 v_stroke_col;\n";

static const char _glsl_varyings_gradient[] =
   "varying SPAN_HP vec4 v_fill_grad_abc_y;\n"
   "varying SPAN_HP vec4 v_fill_grad_def;\n"
   "varying SPAN_HP vec4 v_fill_grad_radial;\n"
   "varying SPAN_HP vec4 v_stroke_grad_abc_y;\n"
   "varying SPAN_HP vec4 v_stroke_grad_def;\n"
   "varying SPAN_HP vec4 v_stroke_grad_radial;\n";

static const char _glsl_varyings_mask[] =
   "varying SPAN_HP vec4 v_mask_off_size;\n"
   "varying mediump vec2 v_mask_comp_inv;\n";

/* ------------------------------------------------------------------ */
/* Attribute declaration blocks (per-vertex data → VS input)          */
/* ------------------------------------------------------------------ */

static const char _glsl_attributes_common[] =
   "attribute highp vec2  a_position;\n"
   "attribute highp vec4  a_fbo_fill_off;\n"
   "attribute highp vec4  a_stroke_off_flags;\n"
   "attribute highp vec2  a_x_min;\n"
   "attribute mediump vec4 a_mul_col;\n";

static const char _glsl_attributes_solid[] =
   "attribute mediump vec4 a_fill_col;\n"
   "attribute mediump vec4 a_stroke_col;\n";

static const char _glsl_attributes_gradient[] =
   "attribute highp vec4 a_fill_grad_abc_y;\n"
   "attribute highp vec4 a_fill_grad_def;\n"
   "attribute highp vec4 a_fill_grad_radial;\n"
   "attribute highp vec4 a_stroke_grad_abc_y;\n"
   "attribute highp vec4 a_stroke_grad_def;\n"
   "attribute highp vec4 a_stroke_grad_radial;\n";

static const char _glsl_attributes_mask[] =
   "attribute highp vec4 a_mask_off_size;\n"
   "attribute mediump vec2 a_mask_comp_inv;\n";

/* ------------------------------------------------------------------ */
/* Vertex shader main bodies — one per variant family                  */
/* ------------------------------------------------------------------ */

/* Solid, no mask. */
static const char _glsl_vs_main_solid[] =
   "void main() {\n"
   "   gl_Position    = vec4(a_position, 0.0, 1.0);\n"
   "   v_fbo_off      = a_fbo_fill_off.xy;\n"
   "   v_fill_off     = a_fbo_fill_off.zw;\n"
   "   v_stroke_off   = a_stroke_off_flags.xy;\n"
   "   v_max_spans    = a_stroke_off_flags.z;\n"
   "   v_has_flags    = a_stroke_off_flags.w;\n"
   "   v_fill_x_min   = a_x_min.x;\n"
   "   v_stroke_x_min = a_x_min.y;\n"
   "   v_mul_col      = a_mul_col;\n"
   "   v_fill_col     = a_fill_col;\n"
   "   v_stroke_col   = a_stroke_col;\n"
   "}\n";

/* Solid with composite mask.
 * Decodes comp_method (slot 0) → mask_op (0=multiply,1=add,2=difference)
 * so the FS can keep its existing mop < 0.5 / mop < 1.5 logic.
 * Slot 1 (mask_inv) is already pre-decoded by _span_fill_vertices. */
static const char _glsl_vs_main_solid_mask[] =
   "void main() {\n"
   "   gl_Position    = vec4(a_position, 0.0, 1.0);\n"
   "   v_fbo_off      = a_fbo_fill_off.xy;\n"
   "   v_fill_off     = a_fbo_fill_off.zw;\n"
   "   v_stroke_off   = a_stroke_off_flags.xy;\n"
   "   v_max_spans    = a_stroke_off_flags.z;\n"
   "   v_has_flags    = a_stroke_off_flags.w;\n"
   "   v_fill_x_min   = a_x_min.x;\n"
   "   v_stroke_x_min = a_x_min.y;\n"
   "   v_mul_col      = a_mul_col;\n"
   "   v_fill_col     = a_fill_col;\n"
   "   v_stroke_col   = a_stroke_col;\n"
   "   v_mask_off_size = a_mask_off_size;\n"
   "   /* Decode comp_method in slot 0 to mask_op (0=multiply,1=add,2=difference).\n"
   "    * Slot 1 carries pre-decoded mask_inv (0=normal, 1=invert).\n"
   "    * comp_method 3 = ADD, 6 = DIFFERENCE, all others = MULTIPLY.\n"
   "    * Use tight range [2.5,3.5) so method 4 falls through to mop=0.0. */\n"
   "   highp float cm = a_mask_comp_inv.x;\n"
   "   highp float mop;\n"
   "   if (cm > 5.5)                    mop = 2.0;\n"  /* comp_method == 6: MASK_DIFFERENCE */
   "   else if (cm > 2.5 && cm < 3.5)  mop = 1.0;\n"  /* comp_method == 3 only: MASK_ADD */
   "   else                             mop = 0.0;\n"  /* methods 1,2,4,5: MULTIPLY */
   "   v_mask_comp_inv = vec2(mop, a_mask_comp_inv.y);\n"
   "}\n";

/* Gradient, no mask. */
static const char _glsl_vs_main_gradient[] =
   "void main() {\n"
   "   gl_Position    = vec4(a_position, 0.0, 1.0);\n"
   "   v_fbo_off      = a_fbo_fill_off.xy;\n"
   "   v_fill_off     = a_fbo_fill_off.zw;\n"
   "   v_stroke_off   = a_stroke_off_flags.xy;\n"
   "   v_max_spans    = a_stroke_off_flags.z;\n"
   "   v_has_flags    = a_stroke_off_flags.w;\n"
   "   v_fill_x_min   = a_x_min.x;\n"
   "   v_stroke_x_min = a_x_min.y;\n"
   "   v_mul_col      = a_mul_col;\n"
   "   v_fill_grad_abc_y    = a_fill_grad_abc_y;\n"
   "   v_fill_grad_def      = a_fill_grad_def;\n"
   "   v_fill_grad_radial   = a_fill_grad_radial;\n"
   "   v_stroke_grad_abc_y  = a_stroke_grad_abc_y;\n"
   "   v_stroke_grad_def    = a_stroke_grad_def;\n"
   "   v_stroke_grad_radial = a_stroke_grad_radial;\n"
   "}\n";

/* Gradient with composite mask. */
static const char _glsl_vs_main_gradient_mask[] =
   "void main() {\n"
   "   gl_Position    = vec4(a_position, 0.0, 1.0);\n"
   "   v_fbo_off      = a_fbo_fill_off.xy;\n"
   "   v_fill_off     = a_fbo_fill_off.zw;\n"
   "   v_stroke_off   = a_stroke_off_flags.xy;\n"
   "   v_max_spans    = a_stroke_off_flags.z;\n"
   "   v_has_flags    = a_stroke_off_flags.w;\n"
   "   v_fill_x_min   = a_x_min.x;\n"
   "   v_stroke_x_min = a_x_min.y;\n"
   "   v_mul_col      = a_mul_col;\n"
   "   v_fill_grad_abc_y    = a_fill_grad_abc_y;\n"
   "   v_fill_grad_def      = a_fill_grad_def;\n"
   "   v_fill_grad_radial   = a_fill_grad_radial;\n"
   "   v_stroke_grad_abc_y  = a_stroke_grad_abc_y;\n"
   "   v_stroke_grad_def    = a_stroke_grad_def;\n"
   "   v_stroke_grad_radial = a_stroke_grad_radial;\n"
   "   v_mask_off_size      = a_mask_off_size;\n"
   "   /* comp_method 3 = ADD, 6 = DIFFERENCE, all others = MULTIPLY.\n"
   "    * Tight range [2.5,3.5) excludes method 4 from the ADD branch. */\n"
   "   highp float cm = a_mask_comp_inv.x;\n"
   "   highp float mop;\n"
   "   if (cm > 5.5)                    mop = 2.0;\n"  /* comp_method == 6: MASK_DIFFERENCE */
   "   else if (cm > 2.5 && cm < 3.5)  mop = 1.0;\n"  /* comp_method == 3 only: MASK_ADD */
   "   else                             mop = 0.0;\n"  /* methods 1,2,4,5: MULTIPLY */
   "   v_mask_comp_inv = vec2(mop, a_mask_comp_inv.y);\n"
   "}\n";

/* ------------------------------------------------------------------ */
/* Shared GLSL fragment shader source fragments                        */
/* ------------------------------------------------------------------ */

/* Fragment-stage highp is optional in GLSL ES 1.00.  glGetShaderPrecisionFormat
 * reports range == 0 and precision == 0 when a format is unsupported, which is
 * how we detect it.  The result is injected as the SPAN_HP macro into *both*
 * stages, because a varying's precision qualifier has to agree across them. */
static int _span_fs_highp = -1;   /* -1 unprobed, 0 unsupported, 1 supported */

static int
_span_fragment_highp_supported(void)
{
   GLint range[2] = { 0, 0 };
   GLint precision = 0;

   if (_span_fs_highp >= 0) return _span_fs_highp;

   glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT,
                              range, &precision);
   _span_fs_highp = (range[0] != 0 || range[1] != 0 || precision != 0) ? 1 : 0;
   if (!_span_fs_highp)
     INF("span shader: fragment highp unsupported, falling back to mediump");
   return _span_fs_highp;
}

static const char _glsl_hp_highp[]   = "#define SPAN_HP highp\n";
static const char _glsl_hp_mediump[] = "#define SPAN_HP mediump\n";

/* Widest variant (gradient + mask) resource usage, counted from the shader
 * source blocks: 5 common + 6 gradient + 2 mask attributes, and roughly 11
 * packed varying vectors.  GLES2 guarantees only 8 of each. */
#define SPAN_WIDE_MAX_ATTRIBS   13
#define SPAN_WIDE_MAX_VARYINGS  11

typedef enum {
   SPAN_TIER_AUTO = 0,
   SPAN_TIER_WIDE = 1,
   SPAN_TIER_OFF  = 2
} Span_Tier;

static int _span_tier_resolved = -1;  /* -1 unresolved, else Span_Tier */

static Span_Tier
_span_tier_get(void)
{
   const char *env;
   GLint attribs = 0, varyings = 0;

   if (_span_tier_resolved >= 0) return (Span_Tier)_span_tier_resolved;

   env = getenv("EVAS_GL_SPAN_TIER");
   if (env)
     {
        if (!strcmp(env, "wide"))      { _span_tier_resolved = SPAN_TIER_WIDE; goto done; }
        if (!strcmp(env, "off"))       { _span_tier_resolved = SPAN_TIER_OFF;  goto done; }
        if (!strcmp(env, "compact"))
          {
             ERR("EVAS_GL_SPAN_TIER=compact is not implemented yet "
                 "(Stage 2); falling back to auto");
          }
        else if (strcmp(env, "auto"))
          {
             ERR("EVAS_GL_SPAN_TIER: unknown value '%s'; falling back to auto", env);
          }
     }

   glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,  &attribs);
#ifdef GL_MAX_VARYING_VECTORS
   glGetIntegerv(GL_MAX_VARYING_VECTORS, &varyings);
#else
   glGetIntegerv(GL_MAX_VARYING_FLOATS, &varyings);
   varyings /= 4;
#endif

   if (attribs == 0 && varyings == 0)
     {
        /* Both queries came back 0: this means glGetIntegerv failed (e.g. no
         * current GL context yet), not that the device genuinely reports 0
         * attributes/varyings.  Do not memoize — leave _span_tier_resolved
         * unresolved so the next call (once a context is current) retries. */
        INF("span tier query returned 0/0; assuming no current GL context, "
            "will retry on next call");
        return SPAN_TIER_OFF;
     }

   if (attribs  < SPAN_WIDE_MAX_ATTRIBS ||
       varyings < SPAN_WIDE_MAX_VARYINGS)
     {
        INF("span path disabled: device reports %d vertex attributes and %d "
            "varying vectors, the span shaders need %d and %d",
            (int)attribs, (int)varyings,
            SPAN_WIDE_MAX_ATTRIBS, SPAN_WIDE_MAX_VARYINGS);
        _span_tier_resolved = SPAN_TIER_OFF;
     }
   else
     _span_tier_resolved = SPAN_TIER_WIDE;

done:
   return (Span_Tier)_span_tier_resolved;
}

/* Default precision: mediump for register pressure on tilers (V3D 4.2 / Mali).
 * Locals and uniforms that need fp32 (gradient coefficients, gl_FragCoord
 * arithmetic) are explicitly qualified highp at their declaration site. */
static const char _glsl_precision[] =
   "precision mediump float;\n";

/* Which span-data bindings the variant declares uniforms for.  The
 * shader source is assembled from a binding-specific uniforms block so
 * that the GLSL compiler never sees a sampler the variant does not use,
 * removing the need for the "* 0.0" sampler-keep workaround. */
typedef enum {
   SPAN_BIND_FILL_AND_STROKE = 0,  /* fill texture + stroke texture     */
   SPAN_BIND_FILL_ONLY       = 1,  /* fill texture only                 */
   SPAN_BIND_STROKE_ONLY     = 2,  /* stroke texture only               */
   SPAN_BIND_COUNT
} Span_Bind_Set;

/* Always present, regardless of bindings.
 * #define MAX_SPANS must match SPAN_COLLECTOR_DEFAULT_MAX_SPANS (64).
 * Per-shape data (mul_col, fbo_offset, max_spans) are now varyings;
 * only the shared pool reciprocals and atlas sampler remain as uniforms.
 * u_mask_tex is also a uniform (sampler); only present in mask variants. */
static const char _glsl_uniforms_shared[] =
   "uniform SPAN_HP float u_inv_tw;\n"
   "uniform SPAN_HP float u_inv_th;\n"
   "uniform sampler2D u_grad_ramp_atlas;\n"
   "#define MAX_SPANS 64\n";

/* Fill-and-stroke binding set: both span samplers.
 *
 * Per-shape data (offsets, x_min, has_fill/has_stroke) moved to varyings.
 * Samplers still declared as uniforms; the has_fill/has_stroke values are
 * decoded from v_has_flags in main() — no per-binding defines needed. */
static const char _glsl_uniforms_bind_fs[] =
   "uniform sampler2D u_fill_spans;\n"
   "uniform sampler2D u_stroke_spans;\n";

/* Fill-only binding set: fill sampler only.
 *
 * u_stroke_spans is also declared so that identifiers in dead branches
 * resolve on strict GLSL ES 2.0 front-ends (V3D 4.2).  The has_s integer
 * (decoded from v_has_flags) will be 0 for fill-only shapes, making the
 * stroke branch statically dead after constant folding. */
static const char _glsl_uniforms_bind_f[] =
   "uniform sampler2D u_fill_spans;\n"
   "uniform sampler2D u_stroke_spans;\n";

/* Stroke-only binding set: stroke sampler only.
 *
 * u_fill_spans is also declared for dead-branch resolution (see fill-only
 * comment above).  has_f decoded from v_has_flags will be 0. */
static const char _glsl_uniforms_bind_s[] =
   "uniform sampler2D u_stroke_spans;\n"
   "uniform sampler2D u_fill_spans;\n";


/* Composite mask uniforms (present only in *_mask variants).
 * Only the sampler remains; size, offset, op, inv moved to varyings. */
static const char _glsl_uniforms_mask[] =
   "uniform sampler2D u_mask_tex;\n";

/* scan_spans() — shared by solid and solid_mask shaders.
 *
 * Each span entry is 1 texel (4 bytes) in the span texture:
 *   byte0 (B): coverage — AA coverage 0-255
 *   byte1 (G): len      — span length (max 255; longer spans are split)
 *   byte2 (R): gap      — distance from end of previous span on this row
 *   byte3 (A): reserved — zero
 */
static const char _glsl_scan_spans[] =
   "\n"
   "/* Scan one span texture row, accumulating coverage-weighted base_col\n"
   " * via premultiplied-alpha src-over into result. */\n"
   "vec4 scan_spans(sampler2D tex, SPAN_HP vec2 off, vec4 base_col, SPAN_HP float px,\n"
   "                SPAN_HP float fy, SPAN_HP float inv_tw, int max_s, int x_min, vec4 res) {\n"
   "   int sx = x_min;\n"
   "   for (int i = 0; i < MAX_SPANS; i++) {\n"
   "      if (i >= max_s) break;\n"
   "      SPAN_HP float fx = (off.x + float(i) + 0.5) * inv_tw;\n"
   "      vec4 t = texture2D(tex, vec2(fx, fy));\n"
   "      int gap = int(t.r * 255.0 + 0.5);\n"
   "      int len = int(t.g * 255.0 + 0.5);\n"
   "      float cov = t.b;\n"
   "      if (len == 0) break;\n"
   "      sx += gap;\n"
   "      if (int(px) >= sx && int(px) < sx + len) {\n"
   "         vec4 col = base_col * cov;\n"
   "         res.rgb = col.rgb + res.rgb * (1.0 - col.a);\n"
   "         res.a   = col.a  + res.a   * (1.0 - col.a);\n"
   "      }\n"
   "      sx += len;\n"
   "   }\n"
   "   return res;\n"
   "}\n";

/* grad_spread() — shared by gradient and gradient_mask shaders.
 *
 * Spread modes:
 *   spread == 0 (PAD):     t = clamp(t, 0.0, 1.0)
 *   spread == 1 (REFLECT): t = 1.0 - abs(fract(t*0.5)*2.0 - 1.0)
 *   spread == 2 (REPEAT):  t = fract(t)
 */
static const char _glsl_grad_spread[] =
   "\n"
   "/* Apply gradient spread mode to t in [-inf, +inf] -> [0, 1].\n"
   " * Parameter and return type are SPAN_HP to preserve precision for\n"
   " * REFLECT/REPEAT on surfaces wider than ~1024 px (mediump fract\n"
   " * loses mantissa bits once t > 1024). */\n"
   "SPAN_HP float grad_spread(SPAN_HP float t, int spread) {\n"
   "   if (spread == 1) {\n"
   "      /* REFLECT: mirror at 0 and 1; arithmetic stays SPAN_HP */\n"
   "      t = 1.0 - abs(fract(t * 0.5) * 2.0 - 1.0);\n"
   "   } else if (spread == 2) {\n"
   "      /* REPEAT; fract in SPAN_HP */\n"
   "      t = fract(t);\n"
   "   } else {\n"
   "      /* PAD (default) */\n"
   "      t = clamp(t, 0.0, 1.0);\n"
   "   }\n"
   "   return t;\n"
   "}\n";

/* scan_gradient_spans() — shared by gradient and gradient_mask shaders.
 *
 * Span buffer format identical to the solid shader (gap, len, coverage).
 * On hit, computes gradient parameter t per-pixel (linear or radial) and
 * samples the gradient ramp texture.
 */
static const char _glsl_scan_gradient_spans[] =
   "\n"
   "/* Scan one gradient span texture row.  On hit, compute t per-pixel\n"
   " * (linear or radial) and sample the gradient ramp atlas at ramp_v,\n"
   " * then src-over blend into result. */\n"
   "vec4 scan_gradient_spans(sampler2D span_tex, SPAN_HP vec2 off,\n"
   "                         SPAN_HP float ramp_v,\n"
   "                         SPAN_HP float ga, SPAN_HP float gb, SPAN_HP float gc,\n"
   "                         int gspread, int gtype,\n"
   "                         SPAN_HP float gd, SPAN_HP float ge, SPAN_HP float gf,\n"
   "                         SPAN_HP float gra, SPAN_HP float grdx, SPAN_HP float grdy,\n"
   "                         SPAN_HP float px, SPAN_HP float py,\n"
   "                         SPAN_HP float fy, SPAN_HP float inv_tw, int max_s, int x_min, vec4 res) {\n"
   "   int sx = x_min;\n"
   "   for (int i = 0; i < MAX_SPANS; i++) {\n"
   "      if (i >= max_s) break;\n"
   "      SPAN_HP float fx = (off.x + float(i) + 0.5) * inv_tw;\n"
   "      vec4 s = texture2D(span_tex, vec2(fx, fy));\n"
   "      int gap = int(s.r * 255.0 + 0.5);\n"
   "      int len = int(s.g * 255.0 + 0.5);\n"
   "      float cov = s.b;\n"
   "      if (len == 0) break;\n"
   "      sx += gap;\n"
   "      if (int(px) >= sx && int(px) < sx + len) {\n"
   "         SPAN_HP float t;\n"
   "         if (gtype == 1) {\n"
   "            /* Radial gradient: quadratic solve in gradient space */\n"
   "            SPAN_HP float rx = ga * px + gb * py + gc;\n"
   "            SPAN_HP float ry = gd * px + ge * py + gf;\n"
   "            SPAN_HP float b_val = 2.0 * (rx * grdx + ry * grdy);\n"
   "            /* gra = inv2a = 0.5/a, precomputed on CPU to avoid\n"
   "             * per-fragment division.  Pre-scale b and det like\n"
   "             * the software forward-differencing path does. */\n"
   "            SPAN_HP float b_s = b_val * gra;\n"
   "            SPAN_HP float det = b_s * b_s + (rx * rx + ry * ry) * 2.0 * gra;\n"
   "            t = sqrt(max(det, 0.0)) - b_s;\n"
   "         } else {\n"
   "            /* Linear gradient: affine dot product.\n"
   "             * All operands (ga, gb, gc, px, py) and destination t\n"
   "             * are SPAN_HP; result lands in SPAN_HP t. */\n"
   "            t = ga * px + gb * py + gc;\n"
   "         }\n"
   "         t = grad_spread(t, gspread);\n"
   "         vec4 grad_col = texture2D(u_grad_ramp_atlas, vec2(t, ramp_v));\n"
   "         vec4 col = grad_col * cov;\n"
   "         res.rgb = col.rgb + res.rgb * (1.0 - col.a);\n"
   "         res.a   = col.a  + res.a   * (1.0 - col.a);\n"
   "      }\n"
   "      sx += len;\n"
   "   }\n"
   "   return res;\n"
   "}\n";

/* main() body for solid shaders: px/py setup, fill/stroke dispatch.
 * Everything up to but not including gl_FragColor.
 * Reads per-shape data from varyings (not uniforms). */
static const char _glsl_main_solid_body[] =
   "\n"
   "void main() {\n"
   "   int max_s  = int(v_max_spans + 0.5);\n"
   "   int has_f  = int(mod(v_has_flags + 0.5, 2.0));\n"
   "   int has_s  = int(v_has_flags + 0.5) / 2;\n"
   "   int fill_xm = int(v_fill_x_min + 0.5);\n"
   "   int stk_xm  = int(v_stroke_x_min + 0.5);\n"
   "   SPAN_HP float px = gl_FragCoord.x - v_fbo_off.x;\n"
   "   SPAN_HP float py = gl_FragCoord.y - v_fbo_off.y;\n"
   "   vec4 result = vec4(0.0);\n"
   "\n"
   "   if (has_f == 1) {\n"
   "      SPAN_HP float fy = (v_fill_off.y + py) * u_inv_th;\n"
   "      result = scan_spans(u_fill_spans, v_fill_off, v_fill_col,\n"
   "                          px, fy, u_inv_tw, max_s, fill_xm, result);\n"
   "   }\n"
   "   if (has_s == 1) {\n"
   "      SPAN_HP float fy = (v_stroke_off.y + py) * u_inv_th;\n"
   "      result = scan_spans(u_stroke_spans, v_stroke_off, v_stroke_col,\n"
   "                          px, fy, u_inv_tw, max_s, stk_xm, result);\n"
   "   }\n";

/* main() body for gradient shaders.
 * Reads per-shape data from varyings (not uniforms). */
static const char _glsl_main_gradient_body[] =
   "\n"
   "void main() {\n"
   "   int max_s   = int(v_max_spans + 0.5);\n"
   "   int has_f   = int(mod(v_has_flags + 0.5, 2.0));\n"
   "   int has_s   = int(v_has_flags + 0.5) / 2;\n"
   "   int fill_xm = int(v_fill_x_min + 0.5);\n"
   "   int stk_xm  = int(v_stroke_x_min + 0.5);\n"
   "   /* Gradient type/spread packed in .w: type in fill_grad_def.w, spread in fill_grad_radial.w */\n"
   "   int fill_gtype   = int(v_fill_grad_def.w + 0.5);\n"
   "   int fill_gspread = int(v_fill_grad_radial.w + 0.5);\n"
   "   int stk_gtype    = int(v_stroke_grad_def.w + 0.5);\n"
   "   int stk_gspread  = int(v_stroke_grad_radial.w + 0.5);\n"
   "   SPAN_HP float px = gl_FragCoord.x - v_fbo_off.x;\n"
   "   SPAN_HP float py = gl_FragCoord.y - v_fbo_off.y;\n"
   "   vec4 result = vec4(0.0);\n"
   "\n"
   "   if (has_f == 1) {\n"
   "      SPAN_HP float fy = (v_fill_off.y + py) * u_inv_th;\n"
   "      result = scan_gradient_spans(\n"
   "                  u_fill_spans, v_fill_off,\n"
   "                  v_fill_grad_abc_y.w,\n"
   "                  v_fill_grad_abc_y.x, v_fill_grad_abc_y.y, v_fill_grad_abc_y.z,\n"
   "                  fill_gspread, fill_gtype,\n"
   "                  v_fill_grad_def.x, v_fill_grad_def.y, v_fill_grad_def.z,\n"
   "                  v_fill_grad_radial.x, v_fill_grad_radial.y, v_fill_grad_radial.z,\n"
   "                  px, py, fy, u_inv_tw, max_s, fill_xm, result);\n"
   "   }\n"
   "   if (has_s == 1) {\n"
   "      SPAN_HP float fy = (v_stroke_off.y + py) * u_inv_th;\n"
   "      result = scan_gradient_spans(\n"
   "                  u_stroke_spans, v_stroke_off,\n"
   "                  v_stroke_grad_abc_y.w,\n"
   "                  v_stroke_grad_abc_y.x, v_stroke_grad_abc_y.y, v_stroke_grad_abc_y.z,\n"
   "                  stk_gspread, stk_gtype,\n"
   "                  v_stroke_grad_def.x, v_stroke_grad_def.y, v_stroke_grad_def.z,\n"
   "                  v_stroke_grad_radial.x, v_stroke_grad_radial.y, v_stroke_grad_radial.z,\n"
   "                  px, py, fy, u_inv_tw, max_s, stk_xm, result);\n"
   "   }\n";

/* Common main() ending: multiply by color and close.
 * Uses v_mul_col (varying) instead of u_mul_col (uniform). */
static const char _glsl_main_end[] =
   "   gl_FragColor = result * v_mul_col;\n"
   "}\n";

/* Mask epilogue: sample the composite mask texture and apply it.
 * v_mask_comp_inv.x: decoded mask_op (0=multiply, 1=add, 2=difference)
 *   — decoded in the VS from raw comp_method to save per-fragment work.
 * v_mask_comp_inv.y: mask_inv (0=normal, 1=invert, multiply path only). */
static const char _glsl_mask_epilogue[] =
   "\n"
   "   vec2 mask_uv = vec2((px + v_mask_off_size.x + 0.5) / v_mask_off_size.z,\n"
   "                       (py + v_mask_off_size.y + 0.5) / v_mask_off_size.w);\n"
   "   float mask_a = texture2D(u_mask_tex, mask_uv).a;\n"
   "   float mop = v_mask_comp_inv.x;\n"
   "   float mvi = v_mask_comp_inv.y;\n"
   "   if (mop < 0.5)\n"
   "      result *= mix(mask_a, 1.0 - mask_a, mvi);\n"
   "   else if (mop < 1.5)\n"
   "      result = vec4(result.rgb, min(result.a + mask_a, 1.0));\n"
   "   else\n"
   "      result *= abs(result.a - mask_a);\n";

/* ------------------------------------------------------------------ */
/* Per-binding-set uniform block selectors                             */
/* ------------------------------------------------------------------ */

static const char *
_uniforms_bind_for(Span_Bind_Set bind)
{
   switch (bind)
     {
      case SPAN_BIND_FILL_AND_STROKE: return _glsl_uniforms_bind_fs;
      case SPAN_BIND_FILL_ONLY:       return _glsl_uniforms_bind_f;
      case SPAN_BIND_STROKE_ONLY:     return _glsl_uniforms_bind_s;
      default:                        return _glsl_uniforms_bind_fs;
     }
}

/* Build fragment-shader source parts for the (kind, bind, mask) combination.
 * kind: 0=solid, 1=gradient; bind: Span_Bind_Set; mask: 0 or 1.
 *
 * Task 4: per-shape uniform blocks replaced by varying declarations.
 * The bind-specific uniform blocks still declare the span samplers and
 * the fill/stroke binding-set defines (#define u_has_stroke 0 etc.) so
 * dead-branch resolution works on strict GLSL ES 2.0 (V3D 4.2).
 *
 * Returns a heap-allocated array of static string pointers; caller
 * must free() the array (not the strings). */
static const char **
_span_shader_parts_build(int kind, Span_Bind_Set bind, int mask, int *out_count)
{
   const char *parts[20];
   int n = 0;

   parts[n++] = _span_fragment_highp_supported() ? _glsl_hp_highp : _glsl_hp_mediump;
   parts[n++] = _glsl_precision;
   /* Shared uniforms: pool reciprocals + atlas sampler. */
   parts[n++] = _glsl_uniforms_shared;
   /* Binding-set: span samplers + u_has_fill/u_has_stroke defines. */
   parts[n++] = _uniforms_bind_for(bind);
   /* Mask sampler uniform (only in mask variants). */
   if (mask) parts[n++] = _glsl_uniforms_mask;
   /* Varyings: varying declarations read by this FS. */
   parts[n++] = _glsl_varyings_common;
   if (kind == 0) parts[n++] = _glsl_varyings_solid;
   else           parts[n++] = _glsl_varyings_gradient;
   if (mask)      parts[n++] = _glsl_varyings_mask;

   if (kind == 1) parts[n++] = _glsl_grad_spread;
   parts[n++] = (kind == 0) ? _glsl_scan_spans : _glsl_scan_gradient_spans;
   parts[n++] = (kind == 0) ? _glsl_main_solid_body : _glsl_main_gradient_body;
   if (mask) parts[n++] = _glsl_mask_epilogue;
   parts[n++] = _glsl_main_end;

   {
      const char **out = malloc(sizeof(*out) * (size_t)n);
      if (!out) { *out_count = 0; return NULL; }
      memcpy(out, parts, sizeof(*out) * (size_t)n);
      *out_count = n;
      return out;
   }
}

/* Build vertex-shader source parts for the (kind, mask) combination.
 * Returns a heap-allocated array of static string pointers; caller
 * must free() the array (not the strings). */
static const char **
_span_vs_parts_build(int kind, int mask, int *out_count)
{
   const char *parts[13];
   int n = 0;

   parts[n++] = _span_fragment_highp_supported() ? _glsl_hp_highp : _glsl_hp_mediump;
   parts[n++] = _glsl_precision;
   /* Attribute declarations. */
   parts[n++] = _glsl_attributes_common;
   if (kind == 0) parts[n++] = _glsl_attributes_solid;
   else           parts[n++] = _glsl_attributes_gradient;
   if (mask)      parts[n++] = _glsl_attributes_mask;
   /* Varying declarations (same set as FS). */
   parts[n++] = _glsl_varyings_common;
   if (kind == 0) parts[n++] = _glsl_varyings_solid;
   else           parts[n++] = _glsl_varyings_gradient;
   if (mask)      parts[n++] = _glsl_varyings_mask;
   /* VS main body. */
   if (kind == 0)
     parts[n++] = mask ? _glsl_vs_main_solid_mask    : _glsl_vs_main_solid;
   else
     parts[n++] = mask ? _glsl_vs_main_gradient_mask : _glsl_vs_main_gradient;

   {
      const char **out = malloc(sizeof(*out) * (size_t)n);
      if (!out) { *out_count = 0; return NULL; }
      memcpy(out, parts, sizeof(*out) * (size_t)n);
      *out_count = n;
      return out;
   }
}

/* ------------------------------------------------------------------ */
/* Internal shader state                                               */
/* ------------------------------------------------------------------ */

typedef struct
{
   unsigned int program;
   /* Uniform locations — samplers and pool reciprocals only.
    * Per-shape data is now in vertex attributes (attr_* below). */
   int          loc_fill_spans;
   int          loc_stroke_spans;
   int          loc_inv_tw;
   int          loc_inv_th;
   /* Gradient atlas sampler — valid only in gradient variants (-1 otherwise). */
   int          loc_grad_ramp_atlas;
   /* Mask sampler — valid only in mask variants (-1 otherwise). */
   int          loc_mask_tex;
   /* Attribute locations — queried after glLinkProgram.
    * -1 for attributes absent in this variant; BIND_ATTR skips them. */
   int          attr_position;
   int          attr_fbo_fill_off;
   int          attr_stroke_off_flags;
   int          attr_x_min;
   int          attr_mul_col;
   int          attr_fill_col;          /* solid variants only; -1 in gradient */
   int          attr_stroke_col;        /* solid variants only; -1 in gradient */
   int          attr_fill_grad_abc_y;   /* gradient variants only */
   int          attr_fill_grad_def;
   int          attr_fill_grad_radial;
   int          attr_stroke_grad_abc_y;
   int          attr_stroke_grad_def;
   int          attr_stroke_grad_radial;
   int          attr_mask_off_size;     /* mask variants only */
   int          attr_mask_comp_inv;     /* mask variants only */
} Span_Shader;

/* [kind][bind][mask] — kind 0=solid 1=gradient, bind in Span_Bind_Set, mask 0/1.
 * 12 variants total: eliminates unused sampler declarations per binding set. */
static Span_Shader _span_shaders[2][SPAN_BIND_COUNT][2];

static Span_Shader *
_span_shader_pick(int kind, Span_Bind_Set bind, int mask)
{
   return &_span_shaders[kind][(int)bind][mask ? 1 : 0];
}

/* Shader init memo.  A failed compile must never be retried: without this
 * the whole 12-program build is re-attempted on every eng_ector_end, which
 * on a device that cannot compile them costs a full compile plus an
 * unbounded ERR log flood every frame. */
typedef enum {
   SPAN_SHADER_UNTRIED = 0,
   SPAN_SHADER_OK      = 1,
   SPAN_SHADER_FAILED  = 2
} Span_Shader_State;

static Span_Shader_State _span_shader_state = SPAN_SHADER_UNTRIED;

/* 1x1 white texture — kept for potential fallback use; not bound during
 * normal rendering (non-mask shaders have no mask sampler at all). */
static GLuint _white_mask_tex = 0;

/* ------------------------------------------------------------------ */
/* Shader compilation helpers                                          */
/* ------------------------------------------------------------------ */

/**
 * Compile a single shader stage from an array of source fragments.
 *
 * @param type    GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
 * @param parts   Array of null-terminated GLSL source strings.
 * @param count   Number of strings in @p parts.
 * @return        GL shader object name, or 0 on failure.
 */
static unsigned int
_compile_shader_parts(unsigned int type, const char **parts, int count)
{
   unsigned int shd;
   int          ok = 0;

   shd = glCreateShader(type);
   if (!shd) return 0;

   glShaderSource(shd, count, parts, NULL);
   glCompileShader(shd);
   glGetShaderiv(shd, GL_COMPILE_STATUS, &ok);
   if (!ok)
     {
        char log[512];
        glGetShaderInfoLog(shd, sizeof(log), NULL, log);
        ERR("Span shader compile error: %s", log);
        glDeleteShader(shd);
        return 0;
     }
   return shd;
}

/**
 * Compile and link a span shader program from FS + VS source fragment arrays.
 *
 * Idempotent: returns EINA_TRUE immediately if the program is already
 * compiled (ss->program != 0).
 *
 * @param ss          Shader state to populate.
 * @param vert_parts  Array of vertex shader GLSL source strings.
 * @param vert_count  Number of strings in @p vert_parts.
 * @param frag_parts  Array of fragment shader GLSL source strings.
 * @param frag_count  Number of strings in @p frag_parts.
 * @return            EINA_TRUE on success, EINA_FALSE on compile/link error.
 */
static Eina_Bool
_link_program(Span_Shader *ss,
              const char **vert_parts, int vert_count,
              const char **frag_parts, int frag_count)
{
   unsigned int vs, fs;
   int          ok = 0;

   if (ss->program) return EINA_TRUE; /* already compiled */

   vs = _compile_shader_parts(GL_VERTEX_SHADER,   vert_parts, vert_count);
   fs = _compile_shader_parts(GL_FRAGMENT_SHADER, frag_parts, frag_count);
   if (!vs || !fs)
     {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return EINA_FALSE;
     }

   ss->program = glCreateProgram();
   glAttachShader(ss->program, vs);
   glAttachShader(ss->program, fs);
   glLinkProgram(ss->program);
   glGetProgramiv(ss->program, GL_LINK_STATUS, &ok);

   /* Shaders are detached/deleted after linking regardless of outcome. */
   glDetachShader(ss->program, vs);
   glDetachShader(ss->program, fs);
   glDeleteShader(vs);
   glDeleteShader(fs);

   if (!ok)
     {
        char log[512];
        glGetProgramInfoLog(ss->program, sizeof(log), NULL, log);
        ERR("Span shader link error: %s", log);
        glDeleteProgram(ss->program);
        ss->program = 0;
        return EINA_FALSE;
     }

   /* Uniform locations — only samplers and pool reciprocals remain. */
   ss->loc_fill_spans     = glGetUniformLocation(ss->program, "u_fill_spans");
   ss->loc_stroke_spans   = glGetUniformLocation(ss->program, "u_stroke_spans");
   ss->loc_inv_tw         = glGetUniformLocation(ss->program, "u_inv_tw");
   ss->loc_inv_th         = glGetUniformLocation(ss->program, "u_inv_th");
   /* Gradient atlas — location -1 in solid shaders (safe no-op). */
   ss->loc_grad_ramp_atlas = glGetUniformLocation(ss->program, "u_grad_ramp_atlas");
   /* Mask sampler — location -1 in non-mask shaders (safe no-op). */
   ss->loc_mask_tex       = glGetUniformLocation(ss->program, "u_mask_tex");

   /* Attribute locations — determined after link.
    * -1 returned for attributes not in this variant. */
   ss->attr_position          = glGetAttribLocation(ss->program, "a_position");
   ss->attr_fbo_fill_off      = glGetAttribLocation(ss->program, "a_fbo_fill_off");
   ss->attr_stroke_off_flags  = glGetAttribLocation(ss->program, "a_stroke_off_flags");
   ss->attr_x_min             = glGetAttribLocation(ss->program, "a_x_min");
   ss->attr_mul_col           = glGetAttribLocation(ss->program, "a_mul_col");
   ss->attr_fill_col          = glGetAttribLocation(ss->program, "a_fill_col");
   ss->attr_stroke_col        = glGetAttribLocation(ss->program, "a_stroke_col");
   ss->attr_fill_grad_abc_y   = glGetAttribLocation(ss->program, "a_fill_grad_abc_y");
   ss->attr_fill_grad_def     = glGetAttribLocation(ss->program, "a_fill_grad_def");
   ss->attr_fill_grad_radial  = glGetAttribLocation(ss->program, "a_fill_grad_radial");
   ss->attr_stroke_grad_abc_y = glGetAttribLocation(ss->program, "a_stroke_grad_abc_y");
   ss->attr_stroke_grad_def   = glGetAttribLocation(ss->program, "a_stroke_grad_def");
   ss->attr_stroke_grad_radial= glGetAttribLocation(ss->program, "a_stroke_grad_radial");
   ss->attr_mask_off_size     = glGetAttribLocation(ss->program, "a_mask_off_size");
   ss->attr_mask_comp_inv     = glGetAttribLocation(ss->program, "a_mask_comp_inv");

   return EINA_TRUE;
}

/* ------------------------------------------------------------------ */
/* Debug: pool texture readback at pipeline checkpoints                */
/* ------------------------------------------------------------------ */

/* Define SPAN_DEBUG_PROBES at compile time to enable readback diagnostics. */
#ifdef SPAN_DEBUG_PROBES
void
span_debug_readback(const char *label, GLuint tex_id, int px_x, int px_y)
{
   static int fire_count = 0;
   GLuint     tmp_fbo = 0;
   GLint      prev_fbo = 0;
   uint8_t    px[4] = {0};
   GLenum     status;

   if (fire_count >= 30) return;
   fire_count++;

   (void)glGetError();
   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prev_fbo);
   glGenFramebuffers(1, &tmp_fbo);
   glBindFramebuffer(GL_FRAMEBUFFER, tmp_fbo);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, tex_id, 0);
   status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (status == GL_FRAMEBUFFER_COMPLETE)
     {
        glReadPixels(px_x, px_y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
        fprintf(stderr, "SPAN_PROBE [%s]: tex=%u at (%d,%d) -> (%d,%d,%d,%d)\n",
                label, tex_id, px_x, px_y,
                px[0], px[1], px[2], px[3]);
     }
   glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)prev_fbo);
   glDeleteFramebuffers(1, &tmp_fbo);
}
#endif /* SPAN_DEBUG_PROBES */

/* ------------------------------------------------------------------ */
/* Public API: shader init / shutdown                                  */
/* ------------------------------------------------------------------ */

Eina_Bool
span_shader_init(void)
{
   static const char *kind_name[2] = { "solid", "gradient" };
   static const char *bind_name[SPAN_BIND_COUNT] = {
      "fill+stroke", "fill-only", "stroke-only"
   };
   int kind, b, mask;

   if (_span_shader_state == SPAN_SHADER_OK)     return EINA_TRUE;
   if (_span_shader_state == SPAN_SHADER_FAILED) return EINA_FALSE;

   if (_span_tier_get() == SPAN_TIER_OFF)
     {
        _span_shader_state = SPAN_SHADER_FAILED;
        return EINA_FALSE;
     }

   for (kind = 0; kind < 2; kind++)
     {
        for (b = 0; b < (int)SPAN_BIND_COUNT; b++)
          {
             for (mask = 0; mask < 2; mask++)
               {
                  Span_Shader *ss = &_span_shaders[kind][b][mask];
                  const char **fs_parts, **vs_parts;
                  int fn, vn;

                  fs_parts = _span_shader_parts_build(kind, (Span_Bind_Set)b, mask, &fn);
                  vs_parts = _span_vs_parts_build(kind, mask, &vn);
                  if (!fs_parts || !vs_parts)
                    {
                       free(fs_parts);
                       free(vs_parts);
                       ERR("span shader parts alloc failed (%s %s %s)",
                           kind_name[kind], bind_name[b], mask ? "mask" : "no-mask");
                       _span_shader_state = SPAN_SHADER_FAILED;
                       return EINA_FALSE;
                    }
                  if (!_link_program(ss, vs_parts, vn, fs_parts, fn))
                    {
                       free(fs_parts);
                       free(vs_parts);
                       ERR("span shader link failed (%s %s %s)",
                           kind_name[kind], bind_name[b], mask ? "mask" : "no-mask");
                       _span_shader_state = SPAN_SHADER_FAILED;
                       return EINA_FALSE;
                    }
                  free(fs_parts);
                  free(vs_parts);
               }
          }
     }

   /* Create 1x1 white texture — kept as a potential no-mask fallback. */
   if (!_white_mask_tex)
     {
        uint8_t white[4] = { 255, 255, 255, 255 };
        glGenTextures(1, &_white_mask_tex);
        glBindTexture(GL_TEXTURE_2D, _white_mask_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, white);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
     }

   _span_shader_state = SPAN_SHADER_OK;
   return EINA_TRUE;
}

Eina_Bool
span_path_usable(void)
{
   if (_span_tier_get() == SPAN_TIER_OFF) return EINA_FALSE;
   return span_shader_init();
}

void
span_shader_shutdown(void)
{
   int kind, b, mask;

   _span_shader_state = SPAN_SHADER_UNTRIED;
   _span_tier_resolved = -1;
   _span_fs_highp = -1;

   for (kind = 0; kind < 2; kind++)
     for (b = 0; b < (int)SPAN_BIND_COUNT; b++)
       for (mask = 0; mask < 2; mask++)
         {
            Span_Shader *ss = &_span_shaders[kind][b][mask];
            if (ss->program)
              {
                 glDeleteProgram(ss->program);
                 ss->program = 0;
              }
         }

   if (_white_mask_tex)
     {
        glDeleteTextures(1, &_white_mask_tex);
        _white_mask_tex = 0;
     }

   free(_span_pack_buf);
   _span_pack_buf = NULL;
   _span_pack_sz  = 0;
}

/* ------------------------------------------------------------------ */
/* Public API: texture upload / delete                                 */
/* ------------------------------------------------------------------ */

void
span_collector_upload_textures(Span_Collector *sc, void *gc_ptr)
{
   Evas_Engine_GL_Context *gc = (Evas_Engine_GL_Context *)gc_ptr;
   int i, tex_width;

   if (!sc || !gc) return;

   tex_width = sc->max_spans;

   /* Upload only the columns the shader will actually read.  The scan loop
    * is bounded by actual_max_spans, so entries past that are never sampled
    * and uploading the full max_spans width is pure bandwidth waste (64
    * columns instead of the 2-15 a typical shape needs).  The texture stays
    * max_spans wide so nothing has to be reallocated when the span count
    * fluctuates between frames. */
   int up_width = sc->actual_max_spans + 1;
   if (up_width > sc->max_spans) up_width = sc->max_spans;
   if (up_width < 1) up_width = 1;

   for (i = 0; i < sc->texture_count; i++)
     {
        Span_Texture    *tex = &sc->textures[i];
        Evas_GL_Texture *evas_t;

        /* Keep the GPU texture across height changes.  It is allocated at
         * the high-water alloc_height, so a shorter active height simply
         * leaves unused rows at the bottom that the shader never samples
         * (px/py are clamped to the surface, and py < sc->height).  Only a
         * genuine growth past the allocation forces a recreate.
         *
         * Freeing and recreating on every height change was extremely
         * expensive: it takes the evas_gl_common_texture_new() path, which
         * reallocates an atlas slot and re-uploads the whole rect plus its
         * border rows.  For content that resizes every frame that was
         * ~2200 glTexSubImage2D calls and ~10 MB of upload per frame. */
        if (tex->evas_tex)
          {
             Evas_GL_Texture *existing = (Evas_GL_Texture *)tex->evas_tex;
             if (existing->h < (unsigned int)sc->height)
               {
                  evas_gl_common_texture_free(existing, EINA_TRUE);
                  tex->evas_tex  = NULL;
                  tex->prev_hash = 0;
               }
          }

        /* Skip upload if span data hasn't changed since last frame.
         * Use a fast hash of the buffer to detect identical content even
         * when dirty is set (spans re-collected with same values). */
        if (tex->evas_tex)
          {
             if (!tex->dirty)
               continue;

             {
                uint32_t hash = tex->rolling_hash;
                if (hash == tex->prev_hash)
                  {
                     tex->dirty = EINA_FALSE;
                     continue;
                  }
                tex->prev_hash = hash;
             }
          }

        /* Write sentinels for rows that have spans.
         *
         * The collection phase (_flush_row_tail) memsets the tail of each
         * row it touches, but edge cases in chunked callbacks can leave
         * rows without a clean sentinel.  This per-row single-byte write
         * is the correctness backstop: it ensures byte[1] (len) at the
         * span_counts[y] position is 0 for every active row.
         *
         * For rows with NO spans, span_collector_clear already zeroed
         * byte[1] of entry 0, so only rows with idx > 0 need attention. */
        {
           int y;
           for (y = 0; y < sc->height; y++)
             {
                int idx = tex->span_counts[y];
                if (idx > 0 && idx < sc->max_spans)
                  {
                     uint8_t *sentinel = tex->buffer +
                                         ((size_t)y * sc->stride) +
                                         ((size_t)idx * 4);
                     sentinel[1] = 0;
                  }
             }
        }

        /* First frame: create the Evas texture via the standard path.
         * Subsequent dirty frames: update in-place via glTexSubImage2D.
         *
         * The span buffer is already in BGRA-swapped byte order (written
         * that way by _collect_spans_solid), so we can upload directly
         * without a staging copy. */
        if (!tex->evas_tex)
          {
             /* Initial creation via Evas texture pool.  Allocate at the
              * high-water alloc_height so later frames with a different
              * active height can reuse this texture in place. */
             int row_bytes = tex_width * 4;
             int tex_h = sc->alloc_height;
             RGBA_Image *im;
             Image_Entry *ie;

             if (tex_h < sc->height) tex_h = sc->height;
             ie = evas_cache_image_copied_data(evas_common_image_cache_get(),
                                               tex_width, tex_h,
                                               NULL, EINA_TRUE,
                                               EVAS_COLORSPACE_ARGB8888);
             if (!ie) continue;
             ie->flags.preload_done = 0;
             im = (RGBA_Image *)ie;

             /* Copy pre-swapped buffer into the RGBA_Image.  Rows past the
              * active height are never sampled but are zeroed so the texture
              * never carries uninitialised memory. */
             {
                int row;
                uint8_t *dst = (uint8_t *)im->image.data;
                for (row = 0; row < sc->height; row++)
                  {
                     memcpy(dst, tex->buffer + (size_t)row * sc->stride,
                            row_bytes);
                     dst += row_bytes;
                  }
                if (tex_h > sc->height)
                  memset(dst, 0, (size_t)(tex_h - sc->height) * row_bytes);
             }

             evas_t = evas_gl_common_texture_new(gc, im, EINA_FALSE);
             if (evas_t)
               tex->evas_tex = evas_t;
             evas_cache_image_drop(ie);
          }
        else
          {
             /* Update existing texture in-place via glTexSubImage2D —
              * avoids creating a new texture object and pool allocation
              * each frame. */
             evas_t = (Evas_GL_Texture *)tex->evas_tex;

             glBindTexture(GL_TEXTURE_2D, evas_t->pt->texture);
             glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

             /* Upload the entire span buffer in one call when the driver
              * supports GL_UNPACK_ROW_LENGTH (handles stride != tex width).
              * Otherwise fall back to row-by-row upload. */
             if (gc->shared->info.unpack_row_length)
               {
                  /* stride is in bytes; GL_UNPACK_ROW_LENGTH is in pixels. */
                  int stride_pixels = sc->stride / 4;
                  glPixelStorei(GL_UNPACK_ROW_LENGTH, stride_pixels);
                  glTexSubImage2D(GL_TEXTURE_2D, 0,
                                  evas_t->x, evas_t->y,
                                  up_width, sc->height,
                                  evas_t->pt->format,
                                  GL_UNSIGNED_BYTE, tex->buffer);
                  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
               }
             else
               {
                  /* No GL_UNPACK_ROW_LENGTH: the span buffer's stride
                   * ((max_spans + 1) * 4) is wider than the uploaded rect, so
                   * the rows cannot be handed to GL as-is.  Pack them into a
                   * contiguous scratch buffer and upload once, rather than
                   * issuing one call per scanline per shape per frame. */
                  size_t   row_bytes = (size_t)up_width * 4;
                  size_t   need      = row_bytes * (size_t)sc->height;
                  uint8_t *packed    = _span_pack_buf_get(need);

                  if (packed)
                    {
                       int row;
                       for (row = 0; row < sc->height; row++)
                         memcpy(packed + (size_t)row * row_bytes,
                                tex->buffer + (size_t)row * sc->stride,
                                row_bytes);
                       glTexSubImage2D(GL_TEXTURE_2D, 0,
                                       evas_t->x, evas_t->y,
                                       up_width, sc->height,
                                       evas_t->pt->format,
                                       GL_UNSIGNED_BYTE, packed);
                    }
                  else
                    {
                       int row;
                       for (row = 0; row < sc->height; row++)
                         glTexSubImage2D(GL_TEXTURE_2D, 0,
                                         evas_t->x, evas_t->y + row,
                                         up_width, 1,
                                         evas_t->pt->format,
                                         GL_UNSIGNED_BYTE,
                                         tex->buffer + (size_t)row * sc->stride);
                    }
               }

             /* Restore Evas texture binding state. */
             if (evas_t->pt->texture != gc->state.current.cur_tex)
               glBindTexture(gc->state.current.tex_target,
                             gc->state.current.cur_tex);
          }

        tex->dirty = EINA_FALSE;
     }
}

void
span_collector_delete_textures(Span_Collector *sc)
{
   int i;

   if (!sc) return;

   for (i = 0; i < sc->texture_count; i++)
     {
        if (sc->textures[i].evas_tex)
          {
             evas_gl_common_texture_free(
                (Evas_GL_Texture *)sc->textures[i].evas_tex, EINA_TRUE);
             sc->textures[i].evas_tex = NULL;
          }
     }

}

/* ------------------------------------------------------------------ */
/* Evas pipe integration: span_shader_pipe_flush                       */
/* ------------------------------------------------------------------ */

void
span_shader_pipe_flush(Evas_Engine_GL_Context *gc, int pipe_idx)
{
   Span_Variant  variant   = gc->pipe[pipe_idx].array.span_variant;
   void         *vdata     = gc->pipe[pipe_idx].array.span_vertex_data;
   int           nverts    = gc->pipe[pipe_idx].array.num;
   GLsizei       stride    = (GLsizei)span_vertex_size(variant);
   GLuint        fill_tex  = gc->pipe[pipe_idx].shader.span_fill_tex;
   GLuint        stroke_tex= gc->pipe[pipe_idx].shader.span_stroke_tex;
   GLuint        atlas_tex = gc->pipe[pipe_idx].shader.span_grad_atlas_tex;
   GLuint        mask_tex  = gc->pipe[pipe_idx].shader.span_mask_tex;
   float         inv_tw    = gc->pipe[pipe_idx].shader.span_inv_tw;
   float         inv_th    = gc->pipe[pipe_idx].shader.span_inv_th;

   /* Determine kind (0=solid, 1=gradient) and bind set from variant + textures. */
   int kind     = (variant == SPAN_VARIANT_GRADIENT ||
                   variant == SPAN_VARIANT_GRADIENT_MASK) ? 1 : 0;
   int has_mask = (variant == SPAN_VARIANT_SOLID_MASK ||
                   variant == SPAN_VARIANT_GRADIENT_MASK) ? 1 : 0;
   Span_Bind_Set bind;
   if (fill_tex && stroke_tex) bind = SPAN_BIND_FILL_AND_STROKE;
   else if (fill_tex)          bind = SPAN_BIND_FILL_ONLY;
   else                        bind = SPAN_BIND_STROKE_ONLY;

   Span_Shader *ss = _span_shader_pick(kind, bind, has_mask);

   if (!vdata || nverts == 0) return;

   /* Ensure all 12 shader programs are compiled.  Checking the specific
    * variant matters: span_shader_init() aborts at the first failing
    * variant, so [0][0][0] linking says nothing about the one we are about
    * to bind, and glUseProgram(0) yields GL_INVALID_OPERATION. */
   if (!span_shader_init()) return;
   if (!ss->program) return;

   glUseProgram(ss->program);

   /* Uniforms — sampler bindings + pool reciprocals. */
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, fill_tex ? fill_tex : stroke_tex);
   glUniform1i(ss->loc_fill_spans, 0);

   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, stroke_tex ? stroke_tex : fill_tex);
   glUniform1i(ss->loc_stroke_spans, 1);

   if (ss->loc_inv_tw >= 0) glUniform1f(ss->loc_inv_tw, inv_tw);
   if (ss->loc_inv_th >= 0) glUniform1f(ss->loc_inv_th, inv_th);

   if (ss->loc_grad_ramp_atlas >= 0 && atlas_tex)
     {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, atlas_tex);
        glUniform1i(ss->loc_grad_ramp_atlas, 2);
     }

   if (ss->loc_mask_tex >= 0 && mask_tex)
     {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mask_tex);
        glUniform1i(ss->loc_mask_tex, 3);
     }

   /* VBO upload: upload span vertex data once per flush as GL_STREAM_DRAW.
    * Using a VBO avoids the per-draw driver scratch-buffer allocation that
    * client-side vertex arrays require, recovering the performance lost by
    * the attribute-batching refactor on non-batching draws.
    *
    * Lazy allocation: glGenBuffers fires on first use; the context teardown
    * already calls glDeleteBuffers for array.buffer (unconditionally for span
    * pipes via the span_vertex_data cleanup block). */
   if (!gc->pipe[pipe_idx].array.buffer)
     glGenBuffers(1, &gc->pipe[pipe_idx].array.buffer);
   glBindBuffer(GL_ARRAY_BUFFER, gc->pipe[pipe_idx].array.buffer);
   glBufferData(GL_ARRAY_BUFFER,
                (GLsizeiptr)gc->pipe[pipe_idx].array.span_vertex_data_used,
                vdata,
                GL_STREAM_DRAW);

   /* Attribute pointer setup.
    * BIND_ATTR(loc, components, byte_offset_into_vbo) enables and binds each
    * attribute using VBO byte offsets (not CPU pointers).  The VBO is bound
    * above; GL interprets the last argument as an offset when a buffer is
    * bound to GL_ARRAY_BUFFER.  Locations left enabled after draw are benign —
    * image/font shaders bind their own slots (SHAD_VERTEX/SHAD_COLOR)
    * explicitly before drawing, and never fetch from span-specific locations. */
#define BIND_ATTR(loc, cnt, off) \
   do { \
      if ((loc) >= 0) { \
         glEnableVertexAttribArray((GLuint)(loc)); \
         glVertexAttribPointer((GLuint)(loc), (cnt), GL_FLOAT, GL_FALSE, \
                               stride, (const void *)(uintptr_t)(off)); \
      } \
   } while (0)

   /* Common fields — all variants share Span_Vertex_Common at offset 0. */
   BIND_ATTR(ss->attr_position,         2, offsetof(Span_Vertex_Solid, c) + offsetof(Span_Vertex_Common, pos));
   BIND_ATTR(ss->attr_fbo_fill_off,     4, offsetof(Span_Vertex_Solid, c) + offsetof(Span_Vertex_Common, fbo_fill_off));
   BIND_ATTR(ss->attr_stroke_off_flags, 4, offsetof(Span_Vertex_Solid, c) + offsetof(Span_Vertex_Common, stroke_off_flags));
   BIND_ATTR(ss->attr_x_min,            2, offsetof(Span_Vertex_Solid, c) + offsetof(Span_Vertex_Common, x_min));
   BIND_ATTR(ss->attr_mul_col,          4, offsetof(Span_Vertex_Solid, c) + offsetof(Span_Vertex_Common, mul_col));

   switch (variant)
     {
      case SPAN_VARIANT_SOLID:
        BIND_ATTR(ss->attr_fill_col,   4, offsetof(Span_Vertex_Solid, fill_col));
        BIND_ATTR(ss->attr_stroke_col, 4, offsetof(Span_Vertex_Solid, stroke_col));
        break;
      case SPAN_VARIANT_SOLID_MASK:
        BIND_ATTR(ss->attr_fill_col,      4,
                  offsetof(Span_Vertex_Solid_Mask, s) + offsetof(Span_Vertex_Solid, fill_col));
        BIND_ATTR(ss->attr_stroke_col,    4,
                  offsetof(Span_Vertex_Solid_Mask, s) + offsetof(Span_Vertex_Solid, stroke_col));
        BIND_ATTR(ss->attr_mask_off_size, 4, offsetof(Span_Vertex_Solid_Mask, mask_off_size));
        BIND_ATTR(ss->attr_mask_comp_inv, 2, offsetof(Span_Vertex_Solid_Mask, mask_comp_inv));
        break;
      case SPAN_VARIANT_GRADIENT:
        BIND_ATTR(ss->attr_fill_grad_abc_y,    4, offsetof(Span_Vertex_Gradient, fill_grad_abc_y));
        BIND_ATTR(ss->attr_fill_grad_def,      4, offsetof(Span_Vertex_Gradient, fill_grad_def));
        BIND_ATTR(ss->attr_fill_grad_radial,   4, offsetof(Span_Vertex_Gradient, fill_grad_radial));
        BIND_ATTR(ss->attr_stroke_grad_abc_y,  4, offsetof(Span_Vertex_Gradient, stroke_grad_abc_y));
        BIND_ATTR(ss->attr_stroke_grad_def,    4, offsetof(Span_Vertex_Gradient, stroke_grad_def));
        BIND_ATTR(ss->attr_stroke_grad_radial, 4, offsetof(Span_Vertex_Gradient, stroke_grad_radial));
        break;
      case SPAN_VARIANT_GRADIENT_MASK:
        BIND_ATTR(ss->attr_fill_grad_abc_y,    4,
                  offsetof(Span_Vertex_Gradient_Mask, g) + offsetof(Span_Vertex_Gradient, fill_grad_abc_y));
        BIND_ATTR(ss->attr_fill_grad_def,      4,
                  offsetof(Span_Vertex_Gradient_Mask, g) + offsetof(Span_Vertex_Gradient, fill_grad_def));
        BIND_ATTR(ss->attr_fill_grad_radial,   4,
                  offsetof(Span_Vertex_Gradient_Mask, g) + offsetof(Span_Vertex_Gradient, fill_grad_radial));
        BIND_ATTR(ss->attr_stroke_grad_abc_y,  4,
                  offsetof(Span_Vertex_Gradient_Mask, g) + offsetof(Span_Vertex_Gradient, stroke_grad_abc_y));
        BIND_ATTR(ss->attr_stroke_grad_def,    4,
                  offsetof(Span_Vertex_Gradient_Mask, g) + offsetof(Span_Vertex_Gradient, stroke_grad_def));
        BIND_ATTR(ss->attr_stroke_grad_radial, 4,
                  offsetof(Span_Vertex_Gradient_Mask, g) + offsetof(Span_Vertex_Gradient, stroke_grad_radial));
        BIND_ATTR(ss->attr_mask_off_size, 4, offsetof(Span_Vertex_Gradient_Mask, mask_off_size));
        BIND_ATTR(ss->attr_mask_comp_inv, 2, offsetof(Span_Vertex_Gradient_Mask, mask_comp_inv));
        break;
      default: break;
     }
#undef BIND_ATTR

   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_SCISSOR_TEST);

   glDrawArrays(GL_TRIANGLES, 0, nverts);

   /* Unbind span VBO so subsequent client-side array draws (image/font)
    * are not accidentally interpreted as VBO-offset draws. */
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   /* Restore active texture unit. */
   glActiveTexture(GL_TEXTURE0);

   /* Invalidate Evas GL state cache fields touched by the span shader. */
   gc->state.current.prog       = NULL;
   gc->state.current.cur_tex    = 0;
   gc->state.current.cur_texu   = 0;
   gc->state.current.render_op  = -1;
   gc->state.current.blend      = -1;
   gc->state.current.clip       = 0;
   gc->state.current.cx         = 0;
   gc->state.current.cy         = 0;
   gc->state.current.cw         = 0;
   gc->state.current.ch         = 0;
}
