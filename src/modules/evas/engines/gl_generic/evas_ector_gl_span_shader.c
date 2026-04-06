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

#include <stdint.h>
#include <stdlib.h>

#include "evas_common_private.h"
#include "evas_gl_private.h"
#include "evas_ector_log_restore.h"

#include "evas_ector_gl_span.h"

/* ------------------------------------------------------------------ */
/* GLSL shader source strings                                          */
/* ------------------------------------------------------------------ */

/* Vertex shader — shared between solid and gradient programs.
 * Positions are passed as NDC; no matrix transform needed. */
static const char _span_vertex_glsl[] =
   "attribute vec4 a_position;\n"
   "void main() {\n"
   "   gl_Position = a_position;\n"
   "}\n";

/* --- Solid fragment shader ---
 *
 * Each span entry is 1 texel (4 bytes) in the span texture:
 *   byte0 (B): coverage — AA coverage 0-255
 *   byte1 (G): len      — span length (max 255; longer spans are split)
 *   byte2 (R): gap      — distance from end of previous span on this row
 *   byte3 (A): reserved — zero
 *
 * The shader iterates over up to u_max_spans span entries on the current
 * scanline (identified by gl_FragCoord.y), checks whether the current pixel
 * falls inside each span's x range, and accumulates a coverage-weighted
 * premultiplied-alpha result.  A zero-length sentinel terminates the search.
 *
 * u_inv_tw / u_inv_th are 1/pool_w and 1/pool_h (pool-space reciprocals).
 * u_offset.x / u_offset.y are the texel offsets of the span sub-region
 * within the pool.  The shader adds these to convert from logical
 * span-buffer coordinates to pool UV coordinates.
 * #define MAX_SPANS must match SPAN_COLLECTOR_DEFAULT_MAX_SPANS (64).
 */
static const char _span_solid_fragment_glsl[] =
   "precision highp float;\n"
   "uniform sampler2D u_fill_spans;\n"
   "uniform sampler2D u_stroke_spans;\n"
   "uniform float u_inv_tw;\n"
   "uniform float u_inv_th;\n"
   "uniform int   u_max_spans;\n"
   "uniform vec4  u_mul_col;\n"
   "uniform vec2  u_fill_offset;\n"
   "uniform vec2  u_stroke_offset;\n"
   "uniform vec2  u_fbo_offset;\n"
   "uniform vec4  u_fill_col;\n"
   "uniform vec4  u_stroke_col;\n"
   "uniform int   u_has_fill;\n"
   "uniform int   u_has_stroke;\n"
   "uniform int   u_fill_x_min;\n"
   "uniform int   u_stroke_x_min;\n"
   "#define MAX_SPANS 64\n"
   "\n"
   "/* Scan one span texture row, accumulating coverage-weighted base_col\n"
   " * via premultiplied-alpha src-over into result. */\n"
   "vec4 scan_spans(sampler2D tex, vec2 off, vec4 base_col, float px,\n"
   "                float fy, float inv_tw, int max_s, int x_min, vec4 res) {\n"
   "   int sx = x_min;\n"
   "   for (int i = 0; i < MAX_SPANS; i++) {\n"
   "      if (i >= max_s) break;\n"
   "      float fx = (off.x + float(i) + 0.5) * inv_tw;\n"
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
   "}\n"
   "\n"
   "void main() {\n"
   "   float px = gl_FragCoord.x - u_fbo_offset.x;\n"
   "   float py = gl_FragCoord.y - u_fbo_offset.y;\n"
   "   vec4 result = vec4(0.0);\n"
   "\n"
   "   if (u_has_fill == 1) {\n"
   "      float fy = (u_fill_offset.y + py) * u_inv_th;\n"
   "      result = scan_spans(u_fill_spans, u_fill_offset, u_fill_col,\n"
   "                          px, fy, u_inv_tw, u_max_spans, u_fill_x_min, result);\n"
   "   }\n"
   "   if (u_has_stroke == 1) {\n"
   "      float fy = (u_stroke_offset.y + py) * u_inv_th;\n"
   "      result = scan_spans(u_stroke_spans, u_stroke_offset, u_stroke_col,\n"
   "                          px, fy, u_inv_tw, u_max_spans, u_stroke_x_min, result);\n"
   "   }\n"
   "\n"
   "   gl_FragColor = result * u_mul_col;\n"
   "}\n";

/* --- Solid mask fragment shader ---
 *
 * Identical to the solid shader but with unconditional composite mask sampling.
 * Used when span_mask_tex != 0.  No u_has_mask / u_comp_method uniforms —
 * the mask alpha is always multiplied into the result.
 */
static const char _span_solid_mask_fragment_glsl[] =
   "precision highp float;\n"
   "uniform sampler2D u_fill_spans;\n"
   "uniform sampler2D u_stroke_spans;\n"
   "uniform float u_inv_tw;\n"
   "uniform float u_inv_th;\n"
   "uniform int   u_max_spans;\n"
   "uniform vec4  u_mul_col;\n"
   "uniform vec2  u_fill_offset;\n"
   "uniform vec2  u_stroke_offset;\n"
   "uniform vec2  u_fbo_offset;\n"
   "uniform vec4  u_fill_col;\n"
   "uniform vec4  u_stroke_col;\n"
   "uniform int   u_has_fill;\n"
   "uniform int   u_has_stroke;\n"
   "uniform int   u_fill_x_min;\n"
   "uniform int   u_stroke_x_min;\n"
   "uniform sampler2D u_mask_tex;\n"
   "uniform vec2  u_mask_size;\n"
   "uniform vec2  u_mask_offset;\n"
   "uniform float u_mask_inv;\n"
   "uniform float u_mask_op;\n"
   "#define MAX_SPANS 64\n"
   "\n"
   "vec4 scan_spans(sampler2D tex, vec2 off, vec4 base_col, float px,\n"
   "                float fy, float inv_tw, int max_s, int x_min, vec4 res) {\n"
   "   int sx = x_min;\n"
   "   for (int i = 0; i < MAX_SPANS; i++) {\n"
   "      if (i >= max_s) break;\n"
   "      float fx = (off.x + float(i) + 0.5) * inv_tw;\n"
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
   "}\n"
   "\n"
   "void main() {\n"
   "   float px = gl_FragCoord.x - u_fbo_offset.x;\n"
   "   float py = gl_FragCoord.y - u_fbo_offset.y;\n"
   "   vec4 result = vec4(0.0);\n"
   "\n"
   "   if (u_has_fill == 1) {\n"
   "      float fy = (u_fill_offset.y + py) * u_inv_th;\n"
   "      result = scan_spans(u_fill_spans, u_fill_offset, u_fill_col,\n"
   "                          px, fy, u_inv_tw, u_max_spans, u_fill_x_min, result);\n"
   "   }\n"
   "   if (u_has_stroke == 1) {\n"
   "      float fy = (u_stroke_offset.y + py) * u_inv_th;\n"
   "      result = scan_spans(u_stroke_spans, u_stroke_offset, u_stroke_col,\n"
   "                          px, fy, u_inv_tw, u_max_spans, u_stroke_x_min, result);\n"
   "   }\n"
   "\n"
   "   /* Force unconditional sampler references (driver workaround). */\n"
   "   result += (texture2D(u_fill_spans, vec2(0.0)) +\n"
   "             texture2D(u_stroke_spans, vec2(0.0))) * 0.0;\n"
   "\n"
   "   vec2 mask_uv = vec2((px + u_mask_offset.x + 0.5) / u_mask_size.x,\n"
   "                       (py + u_mask_offset.y + 0.5) / u_mask_size.y);\n"
   "   float mask_a = texture2D(u_mask_tex, mask_uv).a;\n"
   "   /* u_mask_op: 0=multiply, 1=add, 2=difference\n"
   "    * u_mask_inv: 0=normal, 1=invert (for multiply path) */\n"
   "   if (u_mask_op < 0.5)\n"
   "      result *= mix(mask_a, 1.0 - mask_a, u_mask_inv);\n"
   "   else if (u_mask_op < 1.5)\n"
   "      result = vec4(result.rgb, min(result.a + mask_a, 1.0));\n"
   "   else\n"
   "      result *= abs(result.a - mask_a);\n"
   "\n"
   "   gl_FragColor = result * u_mul_col;\n"
   "}\n";

/* --- Gradient fragment shader ---
 *
 * Per-pixel gradient evaluation using a 1024×1 RGBA8 ramp texture.
 *
 * Span buffer format: identical to the solid shader — 1 texel per span
 * (gap, len, coverage).  The shader uses the same scan_spans() helper as
 * the solid shader to find which span covers the current pixel.  On hit it
 * computes the gradient parameter t per-pixel instead of using a fixed color:
 *
 *   t = u_grad_a * gl_FragCoord.x + u_grad_b * gl_FragCoord.y + u_grad_c
 *
 * The three coefficients encode the full inverse-transform + gradient
 * direction in a single dot-product, pre-computed on the CPU in eng_ector_end.
 *
 * Spread modes:
 *   u_grad_spread == 0 (PAD):     t = clamp(t, 0.0, 1.0)
 *   u_grad_spread == 1 (REFLECT): t = 1.0 - abs(fract(t*0.5)*2.0 - 1.0)
 *   u_grad_spread == 2 (REPEAT):  t = fract(t)
 *
 * The ramp texture is on unit 2 (units 0 and 1 are fill/stroke span textures).
 *
 * For fill and stroke each channel carries independent gradient parameters
 * (u_fill_grad_* vs u_stroke_grad_*) so mixed gradient+gradient shapes
 * render correctly with different gradients per channel.
 */
static const char _span_gradient_fragment_glsl[] =
   "precision highp float;\n"
   "uniform sampler2D u_fill_spans;\n"
   "uniform sampler2D u_stroke_spans;\n"
   "uniform float u_inv_tw;\n"
   "uniform float u_inv_th;\n"
   "uniform int   u_max_spans;\n"
   "uniform vec4  u_mul_col;\n"
   "uniform vec2  u_fill_offset;\n"
   "uniform vec2  u_stroke_offset;\n"
   "uniform vec2  u_fbo_offset;\n"
   "uniform int   u_has_fill;\n"
   "uniform int   u_has_stroke;\n"
   "uniform int   u_fill_x_min;\n"
   "uniform int   u_stroke_x_min;\n"
   "uniform sampler2D u_fill_grad_ramp;\n"
   "uniform float u_fill_grad_a;\n"
   "uniform float u_fill_grad_b;\n"
   "uniform float u_fill_grad_c;\n"
   "uniform int   u_fill_grad_spread;\n"
   "uniform sampler2D u_stroke_grad_ramp;\n"
   "uniform float u_stroke_grad_a;\n"
   "uniform float u_stroke_grad_b;\n"
   "uniform float u_stroke_grad_c;\n"
   "uniform int   u_stroke_grad_spread;\n"
   "uniform int   u_fill_grad_type;\n"
   "uniform float u_fill_grad_d;\n"
   "uniform float u_fill_grad_e;\n"
   "uniform float u_fill_grad_f;\n"
   "uniform float u_fill_grad_ra;\n"
   "uniform float u_fill_grad_rdx;\n"
   "uniform float u_fill_grad_rdy;\n"
   "uniform int   u_stroke_grad_type;\n"
   "uniform float u_stroke_grad_d;\n"
   "uniform float u_stroke_grad_e;\n"
   "uniform float u_stroke_grad_f;\n"
   "uniform float u_stroke_grad_ra;\n"
   "uniform float u_stroke_grad_rdx;\n"
   "uniform float u_stroke_grad_rdy;\n"
   "#define MAX_SPANS 64\n"
   "\n"
   "/* Apply gradient spread mode to t in [−∞, +∞] → [0, 1]. */\n"
   "float grad_spread(float t, int spread) {\n"
   "   if (spread == 1) {\n"
   "      /* REFLECT: mirror at 0 and 1 */\n"
   "      t = 1.0 - abs(fract(t * 0.5) * 2.0 - 1.0);\n"
   "   } else if (spread == 2) {\n"
   "      /* REPEAT */\n"
   "      t = fract(t);\n"
   "   } else {\n"
   "      /* PAD (default) */\n"
   "      t = clamp(t, 0.0, 1.0);\n"
   "   }\n"
   "   return t;\n"
   "}\n"
   "\n"
   "/* Scan one gradient span texture row.  On hit, compute t per-pixel\n"
   " * (linear or radial) and sample the gradient ramp, then src-over\n"
   " * blend into result. */\n"
   "vec4 scan_gradient_spans(sampler2D span_tex, vec2 off,\n"
   "                         sampler2D ramp, float ga, float gb, float gc,\n"
   "                         int gspread, int gtype,\n"
   "                         float gd, float ge, float gf,\n"
   "                         float gra, float grdx, float grdy,\n"
   "                         float px, float py,\n"
   "                         float fy, float inv_tw, int max_s, int x_min, vec4 res) {\n"
   "   int sx = x_min;\n"
   "   for (int i = 0; i < MAX_SPANS; i++) {\n"
   "      if (i >= max_s) break;\n"
   "      float fx = (off.x + float(i) + 0.5) * inv_tw;\n"
   "      vec4 s = texture2D(span_tex, vec2(fx, fy));\n"
   "      int gap = int(s.r * 255.0 + 0.5);\n"
   "      int len = int(s.g * 255.0 + 0.5);\n"
   "      float cov = s.b;\n"
   "      if (len == 0) break;\n"
   "      sx += gap;\n"
   "      if (int(px) >= sx && int(px) < sx + len) {\n"
   "         float t;\n"
   "         if (gtype == 1) {\n"
   "            /* Radial gradient: quadratic solve in gradient space */\n"
   "            float rx = ga * px + gb * py + gc;\n"
   "            float ry = gd * px + ge * py + gf;\n"
   "            float b_val = 2.0 * (rx * grdx + ry * grdy);\n"
   "            /* gra = inv2a = 0.5/a, precomputed on CPU to avoid\n"
   "             * per-fragment division.  Pre-scale b and det like\n"
   "             * the software forward-differencing path does. */\n"
   "            float b_s = b_val * gra;\n"
   "            float det = b_s * b_s + (rx * rx + ry * ry) * 2.0 * gra;\n"
   "            t = sqrt(max(det, 0.0)) - b_s;\n"
   "         } else {\n"
   "            /* Linear gradient: affine dot product */\n"
   "            t = ga * px + gb * py + gc;\n"
   "         }\n"
   "         t = grad_spread(t, gspread);\n"
   "         vec4 grad_col = texture2D(ramp, vec2(t, 0.5));\n"
   "         vec4 col = grad_col * cov;\n"
   "         res.rgb = col.rgb + res.rgb * (1.0 - col.a);\n"
   "         res.a   = col.a  + res.a   * (1.0 - col.a);\n"
   "      }\n"
   "      sx += len;\n"
   "   }\n"
   "   return res;\n"
   "}\n"
   "\n"
   "void main() {\n"
   "   float px = gl_FragCoord.x - u_fbo_offset.x;\n"
   "   float py = gl_FragCoord.y - u_fbo_offset.y;\n"
   "   vec4 result = vec4(0.0);\n"
   "\n"
   "   if (u_has_fill == 1) {\n"
   "      float fy = (u_fill_offset.y + py) * u_inv_th;\n"
   "      result = scan_gradient_spans(\n"
   "                  u_fill_spans, u_fill_offset,\n"
   "                  u_fill_grad_ramp,\n"
   "                  u_fill_grad_a, u_fill_grad_b, u_fill_grad_c,\n"
   "                  u_fill_grad_spread, u_fill_grad_type,\n"
   "                  u_fill_grad_d, u_fill_grad_e, u_fill_grad_f,\n"
   "                  u_fill_grad_ra, u_fill_grad_rdx, u_fill_grad_rdy,\n"
   "                  px, py, fy, u_inv_tw, u_max_spans, u_fill_x_min, result);\n"
   "   }\n"
   "   if (u_has_stroke == 1) {\n"
   "      float fy = (u_stroke_offset.y + py) * u_inv_th;\n"
   "      result = scan_gradient_spans(\n"
   "                  u_stroke_spans, u_stroke_offset,\n"
   "                  u_stroke_grad_ramp,\n"
   "                  u_stroke_grad_a, u_stroke_grad_b, u_stroke_grad_c,\n"
   "                  u_stroke_grad_spread, u_stroke_grad_type,\n"
   "                  u_stroke_grad_d, u_stroke_grad_e, u_stroke_grad_f,\n"
   "                  u_stroke_grad_ra, u_stroke_grad_rdx, u_stroke_grad_rdy,\n"
   "                  px, py, fy, u_inv_tw, u_max_spans, u_stroke_x_min, result);\n"
   "   }\n"
   "\n"
   "   gl_FragColor = result * u_mul_col;\n"
   "}\n";

/* --- Gradient mask fragment shader ---
 *
 * Identical to the gradient shader but with unconditional composite mask sampling.
 * Used when span_mask_tex != 0.  No u_has_mask / u_comp_method uniforms —
 * the mask alpha is always multiplied into the result.
 */
static const char _span_gradient_mask_fragment_glsl[] =
   "precision highp float;\n"
   "uniform sampler2D u_fill_spans;\n"
   "uniform sampler2D u_stroke_spans;\n"
   "uniform float u_inv_tw;\n"
   "uniform float u_inv_th;\n"
   "uniform int   u_max_spans;\n"
   "uniform vec4  u_mul_col;\n"
   "uniform vec2  u_fill_offset;\n"
   "uniform vec2  u_stroke_offset;\n"
   "uniform vec2  u_fbo_offset;\n"
   "uniform int   u_has_fill;\n"
   "uniform int   u_has_stroke;\n"
   "uniform int   u_fill_x_min;\n"
   "uniform int   u_stroke_x_min;\n"
   "uniform sampler2D u_mask_tex;\n"
   "uniform vec2  u_mask_size;\n"
   "uniform vec2  u_mask_offset;\n"
   "uniform float u_mask_inv;\n"
   "uniform float u_mask_op;\n"
   "uniform sampler2D u_fill_grad_ramp;\n"
   "uniform float u_fill_grad_a;\n"
   "uniform float u_fill_grad_b;\n"
   "uniform float u_fill_grad_c;\n"
   "uniform int   u_fill_grad_spread;\n"
   "uniform sampler2D u_stroke_grad_ramp;\n"
   "uniform float u_stroke_grad_a;\n"
   "uniform float u_stroke_grad_b;\n"
   "uniform float u_stroke_grad_c;\n"
   "uniform int   u_stroke_grad_spread;\n"
   "uniform int   u_fill_grad_type;\n"
   "uniform float u_fill_grad_d;\n"
   "uniform float u_fill_grad_e;\n"
   "uniform float u_fill_grad_f;\n"
   "uniform float u_fill_grad_ra;\n"
   "uniform float u_fill_grad_rdx;\n"
   "uniform float u_fill_grad_rdy;\n"
   "uniform int   u_stroke_grad_type;\n"
   "uniform float u_stroke_grad_d;\n"
   "uniform float u_stroke_grad_e;\n"
   "uniform float u_stroke_grad_f;\n"
   "uniform float u_stroke_grad_ra;\n"
   "uniform float u_stroke_grad_rdx;\n"
   "uniform float u_stroke_grad_rdy;\n"
   "#define MAX_SPANS 64\n"
   "\n"
   "float grad_spread(float t, int spread) {\n"
   "   if (spread == 1) {\n"
   "      t = 1.0 - abs(fract(t * 0.5) * 2.0 - 1.0);\n"
   "   } else if (spread == 2) {\n"
   "      t = fract(t);\n"
   "   } else {\n"
   "      t = clamp(t, 0.0, 1.0);\n"
   "   }\n"
   "   return t;\n"
   "}\n"
   "\n"
   "vec4 scan_gradient_spans(sampler2D span_tex, vec2 off,\n"
   "                         sampler2D ramp, float ga, float gb, float gc,\n"
   "                         int gspread, int gtype,\n"
   "                         float gd, float ge, float gf,\n"
   "                         float gra, float grdx, float grdy,\n"
   "                         float px, float py,\n"
   "                         float fy, float inv_tw, int max_s, int x_min, vec4 res) {\n"
   "   int sx = x_min;\n"
   "   for (int i = 0; i < MAX_SPANS; i++) {\n"
   "      if (i >= max_s) break;\n"
   "      float fx = (off.x + float(i) + 0.5) * inv_tw;\n"
   "      vec4 s = texture2D(span_tex, vec2(fx, fy));\n"
   "      int gap = int(s.r * 255.0 + 0.5);\n"
   "      int len = int(s.g * 255.0 + 0.5);\n"
   "      float cov = s.b;\n"
   "      if (len == 0) break;\n"
   "      sx += gap;\n"
   "      if (int(px) >= sx && int(px) < sx + len) {\n"
   "         float t;\n"
   "         if (gtype == 1) {\n"
   "            float rx = ga * px + gb * py + gc;\n"
   "            float ry = gd * px + ge * py + gf;\n"
   "            float b_val = 2.0 * (rx * grdx + ry * grdy);\n"
   "            float b_s = b_val * gra;\n"
   "            float det = b_s * b_s + (rx * rx + ry * ry) * 2.0 * gra;\n"
   "            t = sqrt(max(det, 0.0)) - b_s;\n"
   "         } else {\n"
   "            t = ga * px + gb * py + gc;\n"
   "         }\n"
   "         t = grad_spread(t, gspread);\n"
   "         vec4 grad_col = texture2D(ramp, vec2(t, 0.5));\n"
   "         vec4 col = grad_col * cov;\n"
   "         res.rgb = col.rgb + res.rgb * (1.0 - col.a);\n"
   "         res.a   = col.a  + res.a   * (1.0 - col.a);\n"
   "      }\n"
   "      sx += len;\n"
   "   }\n"
   "   return res;\n"
   "}\n"
   "\n"
   "void main() {\n"
   "   float px = gl_FragCoord.x - u_fbo_offset.x;\n"
   "   float py = gl_FragCoord.y - u_fbo_offset.y;\n"
   "   vec4 result = vec4(0.0);\n"
   "\n"
   "   if (u_has_fill == 1) {\n"
   "      float fy = (u_fill_offset.y + py) * u_inv_th;\n"
   "      result = scan_gradient_spans(\n"
   "                  u_fill_spans, u_fill_offset,\n"
   "                  u_fill_grad_ramp,\n"
   "                  u_fill_grad_a, u_fill_grad_b, u_fill_grad_c,\n"
   "                  u_fill_grad_spread, u_fill_grad_type,\n"
   "                  u_fill_grad_d, u_fill_grad_e, u_fill_grad_f,\n"
   "                  u_fill_grad_ra, u_fill_grad_rdx, u_fill_grad_rdy,\n"
   "                  px, py, fy, u_inv_tw, u_max_spans, u_fill_x_min, result);\n"
   "   }\n"
   "   if (u_has_stroke == 1) {\n"
   "      float fy = (u_stroke_offset.y + py) * u_inv_th;\n"
   "      result = scan_gradient_spans(\n"
   "                  u_stroke_spans, u_stroke_offset,\n"
   "                  u_stroke_grad_ramp,\n"
   "                  u_stroke_grad_a, u_stroke_grad_b, u_stroke_grad_c,\n"
   "                  u_stroke_grad_spread, u_stroke_grad_type,\n"
   "                  u_stroke_grad_d, u_stroke_grad_e, u_stroke_grad_f,\n"
   "                  u_stroke_grad_ra, u_stroke_grad_rdx, u_stroke_grad_rdy,\n"
   "                  px, py, fy, u_inv_tw, u_max_spans, u_stroke_x_min, result);\n"
   "   }\n"
   "\n"
   "   /* Force all sampler references unconditionally to prevent the GLSL\n"
   "    * compiler from stripping samplers that only appear inside if-branches.\n"
   "    * The 0.0 * texture2D() terms contribute nothing to the result. */\n"
   "   vec4 _keep_fill   = texture2D(u_fill_spans, vec2(0.0));\n"
   "   vec4 _keep_stroke = texture2D(u_stroke_spans, vec2(0.0));\n"
   "   vec4 _keep_framp  = texture2D(u_fill_grad_ramp, vec2(0.0));\n"
   "   vec4 _keep_sramp  = texture2D(u_stroke_grad_ramp, vec2(0.0));\n"
   "   result += (_keep_fill + _keep_stroke + _keep_framp + _keep_sramp) * 0.0;\n"
   "\n"
   "   vec2 mask_uv = vec2((px + u_mask_offset.x + 0.5) / u_mask_size.x,\n"
   "                       (py + u_mask_offset.y + 0.5) / u_mask_size.y);\n"
   "   float mask_a = texture2D(u_mask_tex, mask_uv).a;\n"
   "   /* u_mask_op: 0=multiply, 1=add, 2=difference\n"
   "    * u_mask_inv: 0=normal, 1=invert (for multiply path) */\n"
   "   if (u_mask_op < 0.5)\n"
   "      result *= mix(mask_a, 1.0 - mask_a, u_mask_inv);\n"
   "   else if (u_mask_op < 1.5)\n"
   "      result = vec4(result.rgb, min(result.a + mask_a, 1.0));\n"
   "   else\n"
   "      result *= abs(result.a - mask_a);\n"
   "   gl_FragColor = result * u_mul_col;\n"
   "}\n";

/* ------------------------------------------------------------------ */
/* Internal shader state                                               */
/* ------------------------------------------------------------------ */

typedef struct
{
   unsigned int program;
   int          loc_fill_spans;
   int          loc_stroke_spans;
   int          loc_inv_tw;
   int          loc_inv_th;
   int          loc_max_spans;
   int          loc_mul_col;
   int          loc_fill_offset;
   int          loc_stroke_offset;
   int          loc_fill_col;
   int          loc_stroke_col;
   int          loc_has_fill;
   int          loc_has_stroke;
   int          loc_fbo_offset;
   int          loc_fill_x_min;
   int          loc_stroke_x_min;
   /* Gradient-only uniforms (location -1 in solid shader → safe no-op) */
   int          loc_fill_grad_ramp;
   int          loc_fill_grad_a;
   int          loc_fill_grad_b;
   int          loc_fill_grad_c;
   int          loc_fill_grad_spread;
   int          loc_stroke_grad_ramp;
   int          loc_stroke_grad_a;
   int          loc_stroke_grad_b;
   int          loc_stroke_grad_c;
   int          loc_stroke_grad_spread;
   int          loc_fill_grad_type;
   int          loc_fill_grad_d;
   int          loc_fill_grad_e;
   int          loc_fill_grad_f;
   int          loc_fill_grad_ra;
   int          loc_fill_grad_rdx;
   int          loc_fill_grad_rdy;
   int          loc_stroke_grad_type;
   int          loc_stroke_grad_d;
   int          loc_stroke_grad_e;
   int          loc_stroke_grad_f;
   int          loc_stroke_grad_ra;
   int          loc_stroke_grad_rdx;
   int          loc_stroke_grad_rdy;
   /* Mask uniform locations — valid only in *_mask_shader variants.
    * glGetUniformLocation returns -1 for non-mask shaders; glUniform on
    * location -1 is a GL no-op per spec (safe to call unconditionally). */
   int          loc_mask_tex;
   int          loc_mask_size;
   int          loc_mask_offset;
   int          loc_mask_inv;  /* 0.0 = normal, 1.0 = invert (multiply path) */
   int          loc_mask_op;   /* 0.0 = multiply, 1.0 = add, 2.0 = difference */
} Span_Shader;

static Span_Shader _solid_shader         = { 0 };
static Span_Shader _gradient_shader      = { 0 };
static Span_Shader _solid_mask_shader    = { 0 };
static Span_Shader _gradient_mask_shader = { 0 };

/* 1x1 white texture — kept for potential fallback use; not bound during
 * normal rendering (non-mask shaders have no mask sampler at all). */
static GLuint _white_mask_tex = 0;

/* ------------------------------------------------------------------ */
/* Shader compilation helpers                                          */
/* ------------------------------------------------------------------ */

/**
 * Compile a single shader stage.
 *
 * @param type  GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
 * @param src   Null-terminated GLSL source string.
 * @return      GL shader object name, or 0 on failure.
 */
static unsigned int
_compile_shader(unsigned int type, const char *src)
{
   unsigned int shd;
   int          ok = 0;

   shd = glCreateShader(type);
   if (!shd) return 0;

   glShaderSource(shd, 1, &src, NULL);
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
 * Compile and link a span shader program.
 *
 * Idempotent: returns EINA_TRUE immediately if the program is already
 * compiled (ss->program != 0).
 *
 * @param ss        Shader state to populate.
 * @param frag_src  Fragment shader GLSL source.
 * @return          EINA_TRUE on success, EINA_FALSE on compile/link error.
 */
static Eina_Bool
_link_program(Span_Shader *ss, const char *frag_src)
{
   unsigned int vs, fs;
   int          ok = 0;

   if (ss->program) return EINA_TRUE; /* already compiled */

   vs = _compile_shader(GL_VERTEX_SHADER, _span_vertex_glsl);
   fs = _compile_shader(GL_FRAGMENT_SHADER, frag_src);
   if (!vs || !fs)
     {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return EINA_FALSE;
     }

   ss->program = glCreateProgram();
   glAttachShader(ss->program, vs);
   glAttachShader(ss->program, fs);
   glBindAttribLocation(ss->program, 0, "a_position");
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

   ss->loc_fill_spans   = glGetUniformLocation(ss->program, "u_fill_spans");
   ss->loc_stroke_spans = glGetUniformLocation(ss->program, "u_stroke_spans");
   ss->loc_inv_tw       = glGetUniformLocation(ss->program, "u_inv_tw");
   ss->loc_inv_th       = glGetUniformLocation(ss->program, "u_inv_th");
   ss->loc_max_spans    = glGetUniformLocation(ss->program, "u_max_spans");
   ss->loc_mul_col      = glGetUniformLocation(ss->program, "u_mul_col");
   ss->loc_fill_offset  = glGetUniformLocation(ss->program, "u_fill_offset");
   ss->loc_stroke_offset = glGetUniformLocation(ss->program, "u_stroke_offset");
   ss->loc_fill_col     = glGetUniformLocation(ss->program, "u_fill_col");
   ss->loc_stroke_col   = glGetUniformLocation(ss->program, "u_stroke_col");
   ss->loc_has_fill     = glGetUniformLocation(ss->program, "u_has_fill");
   ss->loc_has_stroke   = glGetUniformLocation(ss->program, "u_has_stroke");
   ss->loc_fbo_offset     = glGetUniformLocation(ss->program, "u_fbo_offset");
   ss->loc_fill_x_min    = glGetUniformLocation(ss->program, "u_fill_x_min");
   ss->loc_stroke_x_min  = glGetUniformLocation(ss->program, "u_stroke_x_min");
   /* Gradient uniforms — location -1 in solid shader (safe no-op). */
   ss->loc_fill_grad_ramp    = glGetUniformLocation(ss->program, "u_fill_grad_ramp");
   ss->loc_fill_grad_a       = glGetUniformLocation(ss->program, "u_fill_grad_a");
   ss->loc_fill_grad_b       = glGetUniformLocation(ss->program, "u_fill_grad_b");
   ss->loc_fill_grad_c       = glGetUniformLocation(ss->program, "u_fill_grad_c");
   ss->loc_fill_grad_spread  = glGetUniformLocation(ss->program, "u_fill_grad_spread");
   ss->loc_stroke_grad_ramp   = glGetUniformLocation(ss->program, "u_stroke_grad_ramp");
   ss->loc_stroke_grad_a      = glGetUniformLocation(ss->program, "u_stroke_grad_a");
   ss->loc_stroke_grad_b      = glGetUniformLocation(ss->program, "u_stroke_grad_b");
   ss->loc_stroke_grad_c      = glGetUniformLocation(ss->program, "u_stroke_grad_c");
   ss->loc_stroke_grad_spread = glGetUniformLocation(ss->program, "u_stroke_grad_spread");
   ss->loc_fill_grad_type   = glGetUniformLocation(ss->program, "u_fill_grad_type");
   ss->loc_fill_grad_d      = glGetUniformLocation(ss->program, "u_fill_grad_d");
   ss->loc_fill_grad_e      = glGetUniformLocation(ss->program, "u_fill_grad_e");
   ss->loc_fill_grad_f      = glGetUniformLocation(ss->program, "u_fill_grad_f");
   ss->loc_fill_grad_ra     = glGetUniformLocation(ss->program, "u_fill_grad_ra");
   ss->loc_fill_grad_rdx    = glGetUniformLocation(ss->program, "u_fill_grad_rdx");
   ss->loc_fill_grad_rdy    = glGetUniformLocation(ss->program, "u_fill_grad_rdy");
   ss->loc_stroke_grad_type = glGetUniformLocation(ss->program, "u_stroke_grad_type");
   ss->loc_stroke_grad_d    = glGetUniformLocation(ss->program, "u_stroke_grad_d");
   ss->loc_stroke_grad_e    = glGetUniformLocation(ss->program, "u_stroke_grad_e");
   ss->loc_stroke_grad_f    = glGetUniformLocation(ss->program, "u_stroke_grad_f");
   ss->loc_stroke_grad_ra   = glGetUniformLocation(ss->program, "u_stroke_grad_ra");
   ss->loc_stroke_grad_rdx  = glGetUniformLocation(ss->program, "u_stroke_grad_rdx");
   ss->loc_stroke_grad_rdy  = glGetUniformLocation(ss->program, "u_stroke_grad_rdy");

   /* Mask uniforms — present only in *_mask_shader variants; returns -1
    * for non-mask shaders (safe no-op when passed to glUniform). */
   ss->loc_mask_tex    = glGetUniformLocation(ss->program, "u_mask_tex");
   ss->loc_mask_size   = glGetUniformLocation(ss->program, "u_mask_size");
   ss->loc_mask_offset = glGetUniformLocation(ss->program, "u_mask_offset");
   ss->loc_mask_inv    = glGetUniformLocation(ss->program, "u_mask_inv");
   ss->loc_mask_op     = glGetUniformLocation(ss->program, "u_mask_op");

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
   if (!_link_program(&_solid_shader, _span_solid_fragment_glsl))
     {
        ERR("span solid shader link failed");
        return EINA_FALSE;
     }
   if (!_link_program(&_gradient_shader, _span_gradient_fragment_glsl))
     {
        ERR("span gradient shader link failed");
        return EINA_FALSE;
     }
   if (!_link_program(&_solid_mask_shader, _span_solid_mask_fragment_glsl))
     {
        ERR("span solid mask shader link failed");
        return EINA_FALSE;
     }
   if (!_link_program(&_gradient_mask_shader, _span_gradient_mask_fragment_glsl))
     {
        ERR("span gradient mask shader link failed");
        return EINA_FALSE;
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

   return EINA_TRUE;
}

void
span_shader_shutdown(void)
{
   if (_solid_shader.program)
     {
        glDeleteProgram(_solid_shader.program);
        _solid_shader.program = 0;
     }
   if (_gradient_shader.program)
     {
        glDeleteProgram(_gradient_shader.program);
        _gradient_shader.program = 0;
     }
   if (_solid_mask_shader.program)
     {
        glDeleteProgram(_solid_mask_shader.program);
        _solid_mask_shader.program = 0;
     }
   if (_gradient_mask_shader.program)
     {
        glDeleteProgram(_gradient_mask_shader.program);
        _gradient_mask_shader.program = 0;
     }
   if (_white_mask_tex)
     {
        glDeleteTextures(1, &_white_mask_tex);
        _white_mask_tex = 0;
     }
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

   for (i = 0; i < sc->texture_count; i++)
     {
        Span_Texture    *tex = &sc->textures[i];
        Evas_GL_Texture *evas_t;
        int              y;

        /* Recreate the GPU texture if the active height changed
         * (the sub-region within the pool was allocated for a different height). */
        if (tex->evas_tex)
          {
             Evas_GL_Texture *existing = (Evas_GL_Texture *)tex->evas_tex;
             if (existing->h != (unsigned int)sc->height)
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

             /* Quick content hash: only hash the active span entries per row
              * (span_counts[row] + 1 for the sentinel) rather than the full
              * max_spans allocation.  Uses a polynomial hash (h = h*31 + v)
              * seeded with the FNV-1a offset basis to avoid the degenerate
              * all-zero seed producing hash=0 on empty buffers. */
             {
                uint32_t hash = 2166136261u;  /* FNV-1a offset basis as seed */
                int row;
                for (row = 0; row < sc->height; row++)
                  {
                     const uint32_t *p = (const uint32_t *)(tex->buffer +
                                                            (size_t)row * sc->stride);
                     int active = tex->span_counts[row] + 1;  /* +1 for sentinel */
                     int w;
                     if (active > sc->max_spans) active = sc->max_spans;
                     for (w = 0; w < active; w++)
                       hash = hash * 31 + p[w];
                  }
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
         * All fill types (Solid and gradient) use 1-texel-per-span BGRA layout:
         * byte[1] holds len (the G channel).  Setting byte[1] = 0 writes a
         * zero-length sentinel that terminates the shader's span scan loop. */
        for (y = 0; y < sc->height; y++)
          {
             int idx = tex->span_counts[y];
             if (idx < sc->max_spans)
               {
                  int     bps      = 4;
                  uint8_t *sentinel = tex->buffer +
                                      ((size_t)y * sc->stride) +
                                      ((size_t)idx * bps);
                  sentinel[1] = 0;  /* byte[1] = len (BGRA G channel) = 0 */
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
             /* Initial creation via Evas texture pool. */
             int row_bytes = tex_width * 4;
             RGBA_Image *im;
             Image_Entry *ie;

             ie = evas_cache_image_copied_data(evas_common_image_cache_get(),
                                               tex_width, sc->height,
                                               NULL, EINA_TRUE,
                                               EVAS_COLORSPACE_ARGB8888);
             if (!ie) continue;
             ie->flags.preload_done = 0;
             im = (RGBA_Image *)ie;

             /* Copy pre-swapped buffer into the RGBA_Image. */
             {
                int row;
                uint8_t *dst = (uint8_t *)im->image.data;
                for (row = 0; row < sc->height; row++)
                  {
                     memcpy(dst, tex->buffer + (size_t)row * sc->stride,
                            row_bytes);
                     dst += row_bytes;
                  }
             }

             evas_t = evas_gl_common_texture_new(gc, im, EINA_FALSE);
             if (evas_t)
               tex->evas_tex = evas_t;
             evas_cache_image_drop(ie);
          }
        else
          {
             /* Update existing texture in-place. Upload the full span
              * buffer via glTexSubImage2D — avoids creating a new
              * texture object and pool allocation each frame. */
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
                                  tex_width, sc->height,
                                  evas_t->pt->format,
                                  GL_UNSIGNED_BYTE, tex->buffer);
                  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
               }
             else
               {
                  int row;
                  for (row = 0; row < sc->height; row++)
                    {
                       const uint8_t *src = tex->buffer + (size_t)row * sc->stride;
                       glTexSubImage2D(GL_TEXTURE_2D, 0,
                                       evas_t->x, evas_t->y + row,
                                       tex_width, 1,
                                       evas_t->pt->format,
                                       GL_UNSIGNED_BYTE, src);
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

   /* Free the gradient ramp texture if one was created. */
   if (sc->grad_ramp_tex)
     {
        GLuint t = (GLuint)sc->grad_ramp_tex;
        glDeleteTextures(1, &t);
        sc->grad_ramp_tex = 0;
     }
}

/* ------------------------------------------------------------------ */
/* Public API: draw                                                    */
/* ------------------------------------------------------------------ */

/**
 * Draw a textured quad for every Span_Texture in @p sc using the
 * appropriate span-lookup shader (solid or gradient, based on sc->type).
 *
 * GL state saved and restored: current program, blend enable.
 * Blend function is set to (GL_ONE, GL_ONE_MINUS_SRC_ALPHA) for
 * premultiplied-alpha compositing.
 *
 * After return the caller must call evas_gl_common_context_flush() to
 * invalidate the Evas GL state cache.
 */
/* span_shader_draw() removed — all rendering goes through the Evas pipe
 * path via span_shader_pipe_flush(). */

/* ------------------------------------------------------------------ */
/* Evas pipe integration: span_shader_pipe_flush                       */
/* ------------------------------------------------------------------ */

/**
 * Flush one SHD_SPAN pipe entry from shader_array_flush().
 *
 * Reads the canvas-space vertex quad from gc->pipe[pipe_idx].array.vertex
 * and the span shader parameters from gc->pipe[pipe_idx].shader.span_*.
 * Converts vertex coordinates to NDC and issues a single glDrawArrays call
 * using the solid span-lookup shader.
 *
 * @param gc       GL context.
 * @param pipe_idx Index of the pipe being flushed.
 * @param gw       Viewport width in pixels (for NDC conversion).
 * @param gh       Viewport height in pixels (for NDC conversion).
 *
 * This function is the strong override of the weak stub in evas_gl_context.c.
 * It is only linked into engine modules that include evas_ector_gl_span_shader.c
 * (currently gl_generic and its sub-engines).
 */
/**
 * Bind shader uniforms and issue one glDrawArrays call for a span pipe entry.
 *
 * Precondition: glVertexAttribPointer(SHAD_VERTEX, ...) has already been
 * called by the parent span_shader_pipe_flush() with the NDC vertex data.
 *
 * @param ss         Shader to use (solid or gradient).
 * @param gc         GL context.
 * @param pipe_idx   Pipe index.
 * @param nverts     Vertex count (passed to glDrawArrays).
 * @param mul_col    Premultiplied multiply color.
 * @param fill_tex   Fill texture GL name (0 = no fill this pass).
 * @param stroke_tex Stroke texture GL name (0 = no stroke this pass).
 */
static void
_span_draw_pass(Span_Shader *ss,
                Evas_Engine_GL_Context *gc,
                int pipe_idx,
                int nverts,
                int max_spans,
                uint32_t mul_col,
                GLuint fill_tex,
                GLuint stroke_tex)
{
   float inv_tw   = gc->pipe[pipe_idx].shader.span_inv_tw;
   float inv_th   = gc->pipe[pipe_idx].shader.span_inv_th;
   float r, g, b, a;

   a = (float)((mul_col >> 24) & 0xFF) / 255.0f;
   r = (float)((mul_col >> 16) & 0xFF) / 255.0f;
   g = (float)((mul_col >>  8) & 0xFF) / 255.0f;
   b = (float)( mul_col        & 0xFF) / 255.0f;

   glUseProgram(ss->program);

   glUniform1f(ss->loc_inv_tw,    inv_tw);
   glUniform1f(ss->loc_inv_th,    inv_th);
   glUniform1i(ss->loc_max_spans, max_spans);
   glUniform4f(ss->loc_mul_col,   r, g, b, a);

   /* Fill on texture unit 0 */
   {
      int has_fill = (fill_tex != 0) ? 1 : 0;
      glUniform1i(ss->loc_has_fill, has_fill);
      if (has_fill)
        {
           uint32_t fc = gc->pipe[pipe_idx].shader.span_fill_col;
           glUniform2f(ss->loc_fill_offset,
                       gc->pipe[pipe_idx].shader.span_fill_off_tx,
                       gc->pipe[pipe_idx].shader.span_fill_off_ty);
           /* loc_fill_col is -1 in the gradient shader (uniform absent) —
            * glUniform on location -1 is a GL no-op per spec. */
           glUniform4f(ss->loc_fill_col,
                       (float)((fc >> 16) & 0xFF) / 255.0f,
                       (float)((fc >>  8) & 0xFF) / 255.0f,
                       (float)( fc        & 0xFF) / 255.0f,
                       (float)((fc >> 24) & 0xFF) / 255.0f);
           glActiveTexture(GL_TEXTURE0);
           glBindTexture(GL_TEXTURE_2D, fill_tex);
        }
      glUniform1i(ss->loc_fill_spans, 0);
   }

   /* Stroke on texture unit 1 */
   {
      int has_stroke = (stroke_tex != 0) ? 1 : 0;
      glUniform1i(ss->loc_has_stroke, has_stroke);
      if (has_stroke)
        {
           uint32_t s_col = gc->pipe[pipe_idx].shader.span_stroke_col;
           glUniform2f(ss->loc_stroke_offset,
                       gc->pipe[pipe_idx].shader.span_stroke_off_tx,
                       gc->pipe[pipe_idx].shader.span_stroke_off_ty);
           glUniform4f(ss->loc_stroke_col,
                       (float)((s_col >> 16) & 0xFF) / 255.0f,
                       (float)((s_col >>  8) & 0xFF) / 255.0f,
                       (float)( s_col        & 0xFF) / 255.0f,
                       (float)((s_col >> 24) & 0xFF) / 255.0f);
           glActiveTexture(GL_TEXTURE1);
           glBindTexture(GL_TEXTURE_2D, stroke_tex);
        }
      glUniform1i(ss->loc_stroke_spans, 1);
   }

   /* FBO atlas offset: converts gl_FragCoord from atlas-space to VG-local
    * coordinates.  Zero for dedicated FBOs (pre-atlas path). */
   glUniform2f(ss->loc_fbo_offset,
               gc->pipe[pipe_idx].shader.span_fbo_off_x,
               gc->pipe[pipe_idx].shader.span_fbo_off_y);

   /* Spatial split x_min: each split texture covers a sub-range of x.
    * The shader starts its span accumulator at x_min instead of 0. */
   glUniform1i(ss->loc_fill_x_min,   gc->pipe[pipe_idx].shader.span_fill_x_min);
   glUniform1i(ss->loc_stroke_x_min, gc->pipe[pipe_idx].shader.span_stroke_x_min);

   /* Gradient parameters — units 2 and 3 for fill and stroke ramp textures.
    * Only set these uniforms when using a gradient shader; on the solid fast
    * path all gradient uniform locations are -1 and glUniform on -1 is a GL
    * no-op per spec, but the calls are still dispatched through the driver.
    * Skipping them entirely saves ~22 glUniform calls per solid draw. */
   if (ss == &_gradient_shader || ss == &_gradient_mask_shader)
     {
        GLuint fill_ramp   = gc->pipe[pipe_idx].shader.span_fill_grad_ramp;
        GLuint stroke_ramp = gc->pipe[pipe_idx].shader.span_stroke_grad_ramp;

        glUniform1f(ss->loc_fill_grad_a,       gc->pipe[pipe_idx].shader.span_fill_grad_a);
        glUniform1f(ss->loc_fill_grad_b,       gc->pipe[pipe_idx].shader.span_fill_grad_b);
        glUniform1f(ss->loc_fill_grad_c,       gc->pipe[pipe_idx].shader.span_fill_grad_c);
        glUniform1i(ss->loc_fill_grad_spread,  gc->pipe[pipe_idx].shader.span_fill_grad_spread);
        glUniform1i(ss->loc_fill_grad_ramp,    2);  /* texture unit 2 */
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, fill_ramp ? fill_ramp : 0);

        glUniform1f(ss->loc_stroke_grad_a,      gc->pipe[pipe_idx].shader.span_stroke_grad_a);
        glUniform1f(ss->loc_stroke_grad_b,      gc->pipe[pipe_idx].shader.span_stroke_grad_b);
        glUniform1f(ss->loc_stroke_grad_c,      gc->pipe[pipe_idx].shader.span_stroke_grad_c);
        glUniform1i(ss->loc_stroke_grad_spread, gc->pipe[pipe_idx].shader.span_stroke_grad_spread);
        glUniform1i(ss->loc_stroke_grad_ramp,   3);  /* texture unit 3 */
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, stroke_ramp ? stroke_ramp : 0);

        glUniform1i(ss->loc_fill_grad_type,   gc->pipe[pipe_idx].shader.span_fill_grad_type);
        glUniform1f(ss->loc_fill_grad_d,      gc->pipe[pipe_idx].shader.span_fill_grad_d);
        glUniform1f(ss->loc_fill_grad_e,      gc->pipe[pipe_idx].shader.span_fill_grad_e);
        glUniform1f(ss->loc_fill_grad_f,      gc->pipe[pipe_idx].shader.span_fill_grad_f);
        glUniform1f(ss->loc_fill_grad_ra,     gc->pipe[pipe_idx].shader.span_fill_grad_ra);
        glUniform1f(ss->loc_fill_grad_rdx,    gc->pipe[pipe_idx].shader.span_fill_grad_rdx);
        glUniform1f(ss->loc_fill_grad_rdy,    gc->pipe[pipe_idx].shader.span_fill_grad_rdy);

        glUniform1i(ss->loc_stroke_grad_type, gc->pipe[pipe_idx].shader.span_stroke_grad_type);
        glUniform1f(ss->loc_stroke_grad_d,    gc->pipe[pipe_idx].shader.span_stroke_grad_d);
        glUniform1f(ss->loc_stroke_grad_e,    gc->pipe[pipe_idx].shader.span_stroke_grad_e);
        glUniform1f(ss->loc_stroke_grad_f,    gc->pipe[pipe_idx].shader.span_stroke_grad_f);
        glUniform1f(ss->loc_stroke_grad_ra,   gc->pipe[pipe_idx].shader.span_stroke_grad_ra);
        glUniform1f(ss->loc_stroke_grad_rdx,  gc->pipe[pipe_idx].shader.span_stroke_grad_rdx);
        glUniform1f(ss->loc_stroke_grad_rdy,  gc->pipe[pipe_idx].shader.span_stroke_grad_rdy);

        /* Restore active texture to unit 0 (Evas convention). */
        glActiveTexture(GL_TEXTURE0);
     }

   /* Composite mask (texture unit 4) — only for mask shader variants.
    * Non-mask shaders have no u_mask_tex uniform at all; skip binding
    * to avoid unnecessary texture unit state changes. */
   if (ss == &_solid_mask_shader || ss == &_gradient_mask_shader)
     {
        GLuint mask_tex = gc->pipe[pipe_idx].shader.span_mask_tex;

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, mask_tex);
        glUniform1i(ss->loc_mask_tex, 4);
        glUniform2f(ss->loc_mask_size,
                    gc->pipe[pipe_idx].shader.span_mask_w,
                    gc->pipe[pipe_idx].shader.span_mask_h);
        glUniform2f(ss->loc_mask_offset,
                    gc->pipe[pipe_idx].shader.span_mask_off_x,
                    gc->pipe[pipe_idx].shader.span_mask_off_y);
        /* EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA         = 1 → mask_inv 0.0
         * EFL_GFX_VG_COMPOSITE_METHOD_MATTE_ALPHA_INVERSE = 2 → mask_inv 1.0
         * Using the integer value directly since this file does not pull in
         * the Efl_Gfx_Vg header; see efl_gfx_types.eot for the mapping. */
        {
           float mask_inv = 0.0f;
           float mask_op  = 0.0f;
           int   comp_method = gc->pipe[pipe_idx].shader.span_comp_method;
           /* Enum values from efl_gfx_types.eot:
            *   1 = MATTE_ALPHA         → inv=0 op=0 (result *= mask_a)
            *   2 = MATTE_ALPHA_INVERSE → inv=1 op=0 (result *= 1-mask_a)
            *   3 = MASK_ADD            → inv=0 op=1 (result.a += mask_a)
            *   4 = MASK_SUBSTRACT      → inv=1 op=0 (result *= 1-mask_a)
            *   5 = MASK_INTERSECT      → inv=0 op=0 (result *= mask_a)
            *   6 = MASK_DIFFERENCE     → inv=0 op=2 (result *= |a - mask_a|) */
           if (comp_method == 2 || comp_method == 4)
             mask_inv = 1.0f;
           if (comp_method == 3)
             mask_op = 1.0f;
           else if (comp_method == 6)
             mask_op = 2.0f;
           glUniform1f(ss->loc_mask_inv, mask_inv);
           glUniform1f(ss->loc_mask_op, mask_op);
        }
        glActiveTexture(GL_TEXTURE0);
     }

   glDrawArrays(GL_TRIANGLES, 0, nverts);
}

void
span_shader_pipe_flush(Evas_Engine_GL_Context *gc, int pipe_idx, int gw, int gh)
{
   int      fill_type   = gc->pipe[pipe_idx].shader.span_fill_type;
   int      stroke_type = gc->pipe[pipe_idx].shader.span_stroke_type;
   GLuint   fill_tex    = gc->pipe[pipe_idx].shader.span_fill_tex;
   GLuint   stroke_tex  = gc->pipe[pipe_idx].shader.span_stroke_tex;
   int      n           = gc->pipe[pipe_idx].array.num;
   GLfloat *vsrc        = gc->pipe[pipe_idx].array.vertex;
   uint32_t mul_col     = gc->pipe[pipe_idx].shader.span_mul_col;
   int      max_spans   = gc->pipe[pipe_idx].shader.span_max_spans;
   int      vi, nverts  = n;
   /* 18 vertices = max 3 merged quads (6 verts each).  In practice
    * span_push creates one quad per push, but the pipe merge logic
    * can accumulate up to 3 quads in a single pipe entry. */
   GLfloat  ndc[18 * 2];
   GLfloat *dst;

   /* Ensure all four shader programs are compiled. */
   if (!_solid_shader.program || !_gradient_shader.program ||
       !_solid_mask_shader.program || !_gradient_mask_shader.program)
     {
        if (!span_shader_init())
          return;
     }

   if (nverts > 18) nverts = 18;

   /* Convert canvas-space (x, y, z) vertices to NDC (x, y) vec2.
    * No Y-flip: canvas y=0 maps to NDC -1 (GL bottom) for FBO rendering. */
   dst = ndc;
   for (vi = 0; vi < nverts; vi++)
     {
        float px = vsrc[vi * 3 + 0];
        float py = vsrc[vi * 3 + 1];
        dst[vi * 2 + 0] = (px / (float)gw) * 2.0f - 1.0f;
        dst[vi * 2 + 1] = (py / (float)gh) * 2.0f - 1.0f;
     }

   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_SCISSOR_TEST);

   glEnableVertexAttribArray(SHAD_VERTEX);
   glVertexAttribPointer(SHAD_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, ndc);

   /* Determine if fill and stroke use the same shader family.
    *
    * Option A: both solid, or both gradient — single draw call.
    * Option B: mixed (e.g., gradient fill + solid stroke) — two draw calls,
    *   one per shader.  This is rare (most VG shapes have matching fill/stroke
    *   types) but must be handled correctly.
    *
    * "Is gradient" means LinearGradient or RadialGradient.
    *
    * Mask awareness: when span_mask_tex is set, select the *_mask_shader
    * variant so that the mask alpha is applied unconditionally inside the
    * fragment shader.  Non-mask shaders have no mask sampler at all. */
   {
      int fill_is_grad   = fill_tex   &&
                           (fill_type == LinearGradient || fill_type == RadialGradient);
      int stroke_is_grad = stroke_tex &&
                           (stroke_type == LinearGradient || stroke_type == RadialGradient);
      int fill_is_solid   = fill_tex   && !fill_is_grad;
      int stroke_is_solid = stroke_tex && !stroke_is_grad;
      int has_mask        = (gc->pipe[pipe_idx].shader.span_mask_tex != 0);

      int same_family = (!fill_tex || !stroke_tex) ||
                        (fill_is_grad == stroke_is_grad);

      if (same_family)
        {
           /* Single draw call: choose shader by fill type and mask presence. */
           Span_Shader *ss;

           if (fill_is_grad || stroke_is_grad)
             ss = has_mask ? &_gradient_mask_shader : &_gradient_shader;
           else
             ss = has_mask ? &_solid_mask_shader    : &_solid_shader;

           _span_draw_pass(ss, gc, pipe_idx, nverts, max_spans,
                           mul_col, fill_tex, stroke_tex);
        }
      else
        {
           /* Mixed types: draw fill and stroke separately.
            * max_spans is a safe upper bound for both; sentinels
            * terminate the shader loop at the actual entry count.
            * The mask (if any) is applied on every pass — this is correct
            * because each pass draws the same quad geometry and the mask
            * covers the same pixel region. */
           if (fill_is_solid)
             _span_draw_pass(has_mask ? &_solid_mask_shader    : &_solid_shader,
                             gc, pipe_idx, nverts, max_spans, mul_col, fill_tex, 0);
           if (fill_is_grad)
             _span_draw_pass(has_mask ? &_gradient_mask_shader : &_gradient_shader,
                             gc, pipe_idx, nverts, max_spans, mul_col, fill_tex, 0);

           if (stroke_is_solid)
             _span_draw_pass(has_mask ? &_solid_mask_shader    : &_solid_shader,
                             gc, pipe_idx, nverts, max_spans, mul_col, 0, stroke_tex);
           if (stroke_is_grad)
             _span_draw_pass(has_mask ? &_gradient_mask_shader : &_gradient_shader,
                             gc, pipe_idx, nverts, max_spans, mul_col, 0, stroke_tex);
        }
   }

   /* Invalidate only the Evas GL state cache fields that the span shader
    * actually touched: program, textures (units 0-4), blend, and render_op.
    * Leave clip, smooth, and anti_alias alone. */
   gc->state.current.prog       = NULL;
   gc->state.current.cur_tex    = 0;
   gc->state.current.cur_texu   = 0;
   gc->state.current.render_op  = -1;
   gc->state.current.blend      = -1;
}
