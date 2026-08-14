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

#include <dlfcn.h>
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

/* ------------------------------------------------------------------ */
/* Pixel-unpack buffer staging                                         */
/* ------------------------------------------------------------------ */

/* Uploading span rows straight from client memory makes the driver swizzle
 * them into the texture's tiled layout on the CPU.  Staging them through a
 * pixel-unpack buffer instead lets the GPU do that as a blit, which measured
 * ~20% off the span upload cost on Gen9/iris.
 *
 * Needs GLES 3 or GL_NV_pixel_buffer_object; where neither is present the
 * client-memory path below is used unchanged. */
#ifndef GL_PIXEL_UNPACK_BUFFER
# define GL_PIXEL_UNPACK_BUFFER 0x88EC
#endif

static GLuint _span_pbo = 0;
static size_t _span_pbo_size = 0;
static int    _span_pbo_ok = -1;   /* -1 unprobed, 0 unusable, 1 usable */

/* Bind the staging buffer and make sure it holds @p need bytes, growing it
 * on a high-water mark.
 *
 * Respecifying the store on every upload orphans it, which avoids waiting on
 * a copy the GPU may still be reading - but it also makes the driver take a
 * fresh buffer object from the kernel each time.  At the sizes a whole pass
 * reaches that showed up as four times the ioctl time.  Allocate once and
 * refill instead; a single upload per pass leaves more than enough slack for
 * the previous blit to have drained. */
static void
_span_pbo_bind(size_t need)
{
   if (!_span_pbo) glGenBuffers(1, &_span_pbo);
   glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _span_pbo);
   if (need > _span_pbo_size)
     {
        glBufferData(GL_PIXEL_UNPACK_BUFFER, (GLsizeiptr)need, NULL,
                     GL_STREAM_DRAW);
        _span_pbo_size = need;
     }
}

static int
_span_pbo_usable(void)
{
   const char *ver, *ext;

   if (_span_pbo_ok >= 0) return _span_pbo_ok;

   _span_pbo_ok = 0;
   ver = (const char *)glGetString(GL_VERSION);
   ext = (const char *)glGetString(GL_EXTENSIONS);

   /* "OpenGL ES 3.x" has it in core; on ES 2 it needs the NV extension.
    * Desktop GL has had it since 2.1. */
   if ((ver && (strstr(ver, "OpenGL ES 3") || !strstr(ver, "OpenGL ES"))) ||
       (ext && strstr(ext, "pixel_buffer_object")))
     _span_pbo_ok = 1;


   if (!_span_pbo_ok)
     INF("span shader: no pixel buffer objects, uploading from client memory");
   return _span_pbo_ok;
}

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
   "         vec4 grad_col;\n"
   "         SPAN_HP float t;\n"
   "         if (gtype == 2) {\n"
   "            /* Plain colour riding in a gradient variant: the four\n"
   "             * gradient-coefficient slots carry it verbatim, so that a\n"
   "             * shape with a gradient fill and a solid stroke needs one\n"
   "             * program and one draw rather than two. */\n"
   "            grad_col = vec4(ga, gb, gc, ramp_v);\n"
   "         } else {\n"
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
   "         grad_col = texture2D(u_grad_ramp_atlas, vec2(t, ramp_v));\n"
   "         }\n"
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
/* Fixed vertex attribute locations                                    */
/* ------------------------------------------------------------------ */

/* Bound with glBindAttribLocation before every link so that all twelve
 * programs agree.  Solid and gradient variants reuse 5 and 6 because a VAO
 * is created per variant and no variant declares both sets.  The widest
 * variant (gradient + mask) uses 13 of the 16 GLES2-guaranteed slots. */
#define SPAN_ATTR_POSITION          0
#define SPAN_ATTR_FBO_FILL_OFF      1
#define SPAN_ATTR_STROKE_OFF_FLAGS  2
#define SPAN_ATTR_X_MIN             3
#define SPAN_ATTR_MUL_COL           4
#define SPAN_ATTR_FILL_COL          5   /* solid variants    */
#define SPAN_ATTR_STROKE_COL        6   /* solid variants    */
#define SPAN_ATTR_F_GRAD_ABC_Y      5   /* gradient variants */
#define SPAN_ATTR_F_GRAD_DEF        6
#define SPAN_ATTR_F_GRAD_RADIAL     7
#define SPAN_ATTR_S_GRAD_ABC_Y      8
#define SPAN_ATTR_S_GRAD_DEF        9
#define SPAN_ATTR_S_GRAD_RADIAL    10
#define SPAN_ATTR_MASK_OFF_SIZE    11
#define SPAN_ATTR_MASK_COMP_INV    12

/* ------------------------------------------------------------------ */
/* Internal shader state                                               */
/* ------------------------------------------------------------------ */

typedef struct
{
   unsigned int program;
   /* Uniform locations — samplers and pool reciprocals only.
    * Per-shape data travels in vertex attributes, whose locations are the
    * fixed SPAN_ATTR_* constants bound before linking. */
   int          loc_fill_spans;
   int          loc_stroke_spans;
   int          loc_inv_tw;
   int          loc_inv_th;
   /* Gradient atlas sampler — valid only in gradient variants (-1 otherwise). */
   int          loc_grad_ramp_atlas;
   /* Mask sampler — valid only in mask variants (-1 otherwise). */
   int          loc_mask_tex;
   /* Sampler uniforms are program state; assign the texture units once. */
   Eina_Bool    samplers_bound;
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
/* Per-draw state: shared VBO, per-variant VAOs, binding cache         */
/* ------------------------------------------------------------------ */

/* One streaming VBO for every span pipe instead of one per pipe entry.
 * A VAO records which buffer each attribute reads from, so sharing a single
 * buffer is what allows the VAO to be per-variant rather than per-pipe. */
static GLuint _span_vbo = 0;

/* One VAO per Span_Variant, or 0 when vertex array objects are unavailable.
 * Binding one replaces the ten glVertexAttribPointer + ten
 * glEnableVertexAttribArray calls the flush used to issue per draw. */
static GLuint _span_vao[SPAN_VARIANT_COUNT];

static void (*_gl_gen_vao)(GLsizei, GLuint *)       = NULL;
static void (*_gl_bind_vao)(GLuint)                 = NULL;
static void (*_gl_del_vao)(GLsizei, const GLuint *) = NULL;
static int  _span_vao_probed = 0;

static void
_span_vao_probe(void)
{
   const char *ext;

   if (_span_vao_probed) return;
   _span_vao_probed = 1;

   ext = (const char *)glGetString(GL_EXTENSIONS);
   if (!ext || (!strstr(ext, "GL_OES_vertex_array_object") &&
                !strstr(ext, "GL_ARB_vertex_array_object")))
     {
        /* GLES 3.0+ and desktop GL 3.0+ have it in core with no extension
         * string in the (non-indexed) list, so fall through to the dlsym
         * probe rather than giving up here. */
     }

   _gl_gen_vao  = dlsym(RTLD_DEFAULT, "glGenVertexArrays");
   _gl_bind_vao = dlsym(RTLD_DEFAULT, "glBindVertexArray");
   _gl_del_vao  = dlsym(RTLD_DEFAULT, "glDeleteVertexArrays");
   if (!_gl_gen_vao || !_gl_bind_vao || !_gl_del_vao)
     {
        _gl_gen_vao  = dlsym(RTLD_DEFAULT, "glGenVertexArraysOES");
        _gl_bind_vao = dlsym(RTLD_DEFAULT, "glBindVertexArrayOES");
        _gl_del_vao  = dlsym(RTLD_DEFAULT, "glDeleteVertexArraysOES");
     }
   if (!_gl_gen_vao || !_gl_bind_vao || !_gl_del_vao)
     {
        _gl_gen_vao = NULL; _gl_bind_vao = NULL; _gl_del_vao = NULL;
        INF("span shader: no vertex array objects, using per-draw attribute setup");
     }
}

/* Describe one attribute: location, component count, byte offset. */
typedef struct { int loc, cnt, off; } Span_Attr_Desc;

/* Fill @p out with the attribute layout of @p variant; returns the count. */
static int
_span_attr_layout(Span_Variant variant, Span_Attr_Desc *out)
{
   int n = 0;
#define A(l, c, o) do { out[n].loc = (l); out[n].cnt = (c); out[n].off = (int)(o); n++; } while (0)
#define COMMON(base) \
   A(SPAN_ATTR_POSITION,         2, (base) + offsetof(Span_Vertex_Common, pos)); \
   A(SPAN_ATTR_FBO_FILL_OFF,     4, (base) + offsetof(Span_Vertex_Common, fbo_fill_off)); \
   A(SPAN_ATTR_STROKE_OFF_FLAGS, 4, (base) + offsetof(Span_Vertex_Common, stroke_off_flags)); \
   A(SPAN_ATTR_X_MIN,            2, (base) + offsetof(Span_Vertex_Common, x_min)); \
   A(SPAN_ATTR_MUL_COL,          4, (base) + offsetof(Span_Vertex_Common, mul_col))

   switch (variant)
     {
      case SPAN_VARIANT_SOLID:
        COMMON(offsetof(Span_Vertex_Solid, c));
        A(SPAN_ATTR_FILL_COL,   4, offsetof(Span_Vertex_Solid, fill_col));
        A(SPAN_ATTR_STROKE_COL, 4, offsetof(Span_Vertex_Solid, stroke_col));
        break;
      case SPAN_VARIANT_SOLID_MASK:
        COMMON(offsetof(Span_Vertex_Solid_Mask, s) + offsetof(Span_Vertex_Solid, c));
        A(SPAN_ATTR_FILL_COL,   4,
          offsetof(Span_Vertex_Solid_Mask, s) + offsetof(Span_Vertex_Solid, fill_col));
        A(SPAN_ATTR_STROKE_COL, 4,
          offsetof(Span_Vertex_Solid_Mask, s) + offsetof(Span_Vertex_Solid, stroke_col));
        A(SPAN_ATTR_MASK_OFF_SIZE, 4, offsetof(Span_Vertex_Solid_Mask, mask_off_size));
        A(SPAN_ATTR_MASK_COMP_INV, 2, offsetof(Span_Vertex_Solid_Mask, mask_comp_inv));
        break;
      case SPAN_VARIANT_GRADIENT:
        COMMON(offsetof(Span_Vertex_Gradient, c));
        A(SPAN_ATTR_F_GRAD_ABC_Y,  4, offsetof(Span_Vertex_Gradient, fill_grad_abc_y));
        A(SPAN_ATTR_F_GRAD_DEF,    4, offsetof(Span_Vertex_Gradient, fill_grad_def));
        A(SPAN_ATTR_F_GRAD_RADIAL, 4, offsetof(Span_Vertex_Gradient, fill_grad_radial));
        A(SPAN_ATTR_S_GRAD_ABC_Y,  4, offsetof(Span_Vertex_Gradient, stroke_grad_abc_y));
        A(SPAN_ATTR_S_GRAD_DEF,    4, offsetof(Span_Vertex_Gradient, stroke_grad_def));
        A(SPAN_ATTR_S_GRAD_RADIAL, 4, offsetof(Span_Vertex_Gradient, stroke_grad_radial));
        break;
      case SPAN_VARIANT_GRADIENT_MASK:
        COMMON(offsetof(Span_Vertex_Gradient_Mask, g) + offsetof(Span_Vertex_Gradient, c));
#define G(f) (offsetof(Span_Vertex_Gradient_Mask, g) + offsetof(Span_Vertex_Gradient, f))
        A(SPAN_ATTR_F_GRAD_ABC_Y,  4, G(fill_grad_abc_y));
        A(SPAN_ATTR_F_GRAD_DEF,    4, G(fill_grad_def));
        A(SPAN_ATTR_F_GRAD_RADIAL, 4, G(fill_grad_radial));
        A(SPAN_ATTR_S_GRAD_ABC_Y,  4, G(stroke_grad_abc_y));
        A(SPAN_ATTR_S_GRAD_DEF,    4, G(stroke_grad_def));
        A(SPAN_ATTR_S_GRAD_RADIAL, 4, G(stroke_grad_radial));
#undef G
        A(SPAN_ATTR_MASK_OFF_SIZE, 4, offsetof(Span_Vertex_Gradient_Mask, mask_off_size));
        A(SPAN_ATTR_MASK_COMP_INV, 2, offsetof(Span_Vertex_Gradient_Mask, mask_comp_inv));
        break;
      default: break;
     }
#undef COMMON
#undef A
   return n;
}

/* Apply @p n attribute descriptors against the currently bound VBO. */
static void
_span_attr_apply(const Span_Attr_Desc *d, int n, GLsizei stride)
{
   int i;
   for (i = 0; i < n; i++)
     {
        glEnableVertexAttribArray((GLuint)d[i].loc);
        glVertexAttribPointer((GLuint)d[i].loc, d[i].cnt, GL_FLOAT, GL_FALSE,
                              stride, (const void *)(uintptr_t)d[i].off);
     }
}

/* Return the VAO for @p variant, creating it on first use.  0 means the
 * caller must fall back to per-draw attribute setup. */
static GLuint
_span_vao_get(Span_Variant variant)
{
   Span_Attr_Desc desc[16];
   int n;

   _span_vao_probe();
   if (!_gl_bind_vao) return 0;
   if (_span_vao[variant]) return _span_vao[variant];

   _gl_gen_vao(1, &_span_vao[variant]);
   if (!_span_vao[variant]) return 0;

   _gl_bind_vao(_span_vao[variant]);
   glBindBuffer(GL_ARRAY_BUFFER, _span_vbo);
   n = _span_attr_layout(variant, desc);
   _span_attr_apply(desc, n, (GLsizei)span_vertex_size(variant));
   _gl_bind_vao(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);

   return _span_vao[variant];
}

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

   /* Bind attribute locations explicitly, before linking, so that every
    * variant places a given semantic at the same index.  Without this the
    * linker is free to assign per-program locations, and a vertex array
    * object — whose state is keyed by location, not by program — could not
    * be shared between the programs that use the same vertex layout.
    *
    * Solid and gradient variants deliberately overlap on 5/6: a VAO is
    * per-variant, and no variant declares both sets. */
   glBindAttribLocation(ss->program, SPAN_ATTR_POSITION,         "a_position");
   glBindAttribLocation(ss->program, SPAN_ATTR_FBO_FILL_OFF,     "a_fbo_fill_off");
   glBindAttribLocation(ss->program, SPAN_ATTR_STROKE_OFF_FLAGS, "a_stroke_off_flags");
   glBindAttribLocation(ss->program, SPAN_ATTR_X_MIN,            "a_x_min");
   glBindAttribLocation(ss->program, SPAN_ATTR_MUL_COL,          "a_mul_col");
   glBindAttribLocation(ss->program, SPAN_ATTR_FILL_COL,         "a_fill_col");
   glBindAttribLocation(ss->program, SPAN_ATTR_STROKE_COL,       "a_stroke_col");
   glBindAttribLocation(ss->program, SPAN_ATTR_F_GRAD_ABC_Y,     "a_fill_grad_abc_y");
   glBindAttribLocation(ss->program, SPAN_ATTR_F_GRAD_DEF,       "a_fill_grad_def");
   glBindAttribLocation(ss->program, SPAN_ATTR_F_GRAD_RADIAL,    "a_fill_grad_radial");
   glBindAttribLocation(ss->program, SPAN_ATTR_S_GRAD_ABC_Y,     "a_stroke_grad_abc_y");
   glBindAttribLocation(ss->program, SPAN_ATTR_S_GRAD_DEF,       "a_stroke_grad_def");
   glBindAttribLocation(ss->program, SPAN_ATTR_S_GRAD_RADIAL,    "a_stroke_grad_radial");
   glBindAttribLocation(ss->program, SPAN_ATTR_MASK_OFF_SIZE,    "a_mask_off_size");
   glBindAttribLocation(ss->program, SPAN_ATTR_MASK_COMP_INV,    "a_mask_comp_inv");

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

   /* Attribute locations are the constants bound above, not queried: an
    * attribute the linker optimised out would report -1, but enabling an
    * unused array is harmless and keeping the index fixed is what lets the
    * per-variant VAO be shared across programs. */

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
            ss->samplers_bound = EINA_FALSE;
         }

   if (_white_mask_tex)
     {
        glDeleteTextures(1, &_white_mask_tex);
        _white_mask_tex = 0;
     }

   if (_gl_del_vao)
     {
        int v;
        for (v = 0; v < SPAN_VARIANT_COUNT; v++)
          if (_span_vao[v]) { _gl_del_vao(1, &_span_vao[v]); _span_vao[v] = 0; }
     }
   if (_span_vbo)
     {
        glDeleteBuffers(1, &_span_vbo);
        _span_vbo = 0;
     }
   if (_span_pbo)
     {
        glDeleteBuffers(1, &_span_pbo);
        _span_pbo = 0;
        _span_pbo_size = 0;
     }
   _span_pbo_ok = -1;


   free(_span_pack_buf);
   _span_pack_buf = NULL;
   _span_pack_sz  = 0;
}

/* ------------------------------------------------------------------ */
/* Public API: texture upload / delete                                 */
/* ------------------------------------------------------------------ */

Span_Page *
span_page_new(void)
{
   return calloc(1, sizeof(Span_Page));
}

void
span_page_free(Span_Page *page, Eina_Bool release_tex)
{
   if (!page) return;
   if (page->evas_tex && release_tex)
     evas_gl_common_texture_free((Evas_GL_Texture *)page->evas_tex, EINA_TRUE);
   free(page);
}

unsigned int
span_page_tex_id(const Span_Page *page)
{
   const Evas_GL_Texture *t = page ? (const Evas_GL_Texture *)page->evas_tex : NULL;
   return (t && t->pt) ? t->pt->texture : 0;
}

void
span_page_pool_size(const Span_Page *page, int *w, int *h)
{
   const Evas_GL_Texture *t = page ? (const Evas_GL_Texture *)page->evas_tex : NULL;

   if (w) *w = (t && t->pt) ? t->pt->w : 1;
   if (h) *h = (t && t->pt) ? t->pt->h : 1;
}

/* Walk every Span_Texture of every collector in the pass.  The two arrays
 * are indexed by shape and either may be shorter, so they are visited in
 * sequence rather than in lockstep. */
#define SPAN_PAGE_FOREACH(fills, nf, strokes, ns, scvar, texvar, body)   \
   do {                                                                 \
      int _a, _c, _t;                                                    \
      for (_a = 0; _a < 2; _a++)                                         \
        {                                                                \
           void **_arr = _a ? (strokes) : (fills);                       \
           int    _n   = _a ? (ns)      : (nf);                          \
           if (!_arr) continue;                                          \
           for (_c = 0; _c < _n; _c++)                                   \
             {                                                           \
                Span_Collector *scvar = (Span_Collector *)_arr[_c];      \
                if (!scvar) continue;                                    \
                for (_t = 0; _t < scvar->texture_count; _t++)            \
                  {                                                      \
                     Span_Texture *texvar = &scvar->textures[_t];        \
                     body                                                \
                  }                                                      \
             }                                                           \
        }                                                                \
   } while (0)

/* Ensure the page texture can hold w x h texels, growing it if not.  The
 * texture is created through the Evas pool so that it lives in the same
 * atlas machinery as every other engine texture. */
static Eina_Bool
_span_page_ensure(Span_Page *page, Evas_Engine_GL_Context *gc, int w, int h)
{
   Evas_GL_Texture *t = (Evas_GL_Texture *)page->evas_tex;
   RGBA_Image  *im;
   Image_Entry *ie;

   /* A texture from a previous context is not ours to free - that context's
    * pool already did - but it must not be used either. */
   if (t && page->gc != gc)
     {
        t = NULL;
        page->evas_tex  = NULL;
        page->gc        = NULL;
        page->w = page->h = 0;
        page->prev_hash = 0;
     }

   if (t && page->w >= w && page->h >= h) return EINA_TRUE;

   /* Grow to at least what is asked, never shrink. */
   if (w < page->w) w = page->w;
   if (h < page->h) h = page->h;

   if (t)
     {
        evas_gl_common_texture_free(t, EINA_TRUE);
        page->evas_tex  = NULL;
        page->prev_hash = 0;
     }

   ie = evas_cache_image_copied_data(evas_common_image_cache_get(), w, h,
                                     NULL, EINA_TRUE, EVAS_COLORSPACE_ARGB8888);
   if (!ie) return EINA_FALSE;
   ie->flags.preload_done = 0;
   im = (RGBA_Image *)ie;
   if (im->image.data) memset(im->image.data, 0, (size_t)w * h * 4);

   t = evas_gl_common_texture_new(gc, im, EINA_FALSE);
   evas_cache_image_drop(ie);
   if (!t) return EINA_FALSE;

   page->evas_tex = t;
   page->gc = gc;
   page->w = w;
   page->h = h;
   return EINA_TRUE;
}

/* One Span_Texture's worth of rows waiting to go into the page. */
typedef struct
{
   Span_Texture *tex;
   int           width;   /* columns the shader will actually read */
   int           rows;
   int           stride;  /* source row stride in bytes */
   int           max_spans;
   int          *counts;
} Span_Page_Entry;

static int
_entry_cmp_width_desc(const void *a, const void *b)
{
   const Span_Page_Entry *x = a, *y = b;

   if (x->width != y->width) return y->width - x->width;
   return 0;
}

Eina_Bool
span_page_upload(void *gc_ptr, Span_Page *page,
                 void **fills, int nfills, void **strokes, int nstrokes)
{
   Evas_Engine_GL_Context *gc = (Evas_Engine_GL_Context *)gc_ptr;
   Evas_GL_Texture *pt;
   Span_Page_Entry  stackbuf[64];
   Span_Page_Entry *ent = stackbuf;
   uint32_t hash = 2166136261u;
   int      n = 0, cap = (int)(sizeof(stackbuf) / sizeof(stackbuf[0]));
   int      i, page_w = 1, total_h = 0, row_at;
   Eina_Bool ok = EINA_FALSE;

   if (!gc || !page) return EINA_FALSE;

   /* Gather every Span_Texture of the pass. */
   SPAN_PAGE_FOREACH(fills, nfills, strokes, nstrokes, sc, tex,
     {
        int wid = sc->actual_max_spans + 1;

        if (wid > sc->max_spans) wid = sc->max_spans;
        if (wid < 1) wid = 1;

        if (n == cap)
          {
             int newcap = cap * 2;
             Span_Page_Entry *grown = (ent == stackbuf)
                ? malloc((size_t)newcap * sizeof(*grown))
                : realloc(ent, (size_t)newcap * sizeof(*grown));
             if (!grown) goto done;
             if (ent == stackbuf) memcpy(grown, stackbuf, sizeof(stackbuf));
             ent = grown;
             cap = newcap;
          }

        ent[n].tex       = tex;
        ent[n].width     = wid;
        ent[n].rows      = sc->height;
        ent[n].stride    = sc->stride;
        ent[n].max_spans = sc->max_spans;
        ent[n].counts    = tex->span_counts;
        n++;

        total_h += sc->height;
        if (wid > page_w) page_w = wid;
     });

   if (!n || total_h <= 0) goto done;

   /* Widest first.  One glTexSubImage2D covers a rectangle, so collectors
    * sharing an upload also share its width - putting a 3-column shape in
    * the same rectangle as a 40-column one would upload thirteen times the
    * rows it needs.  Sorting lets similar widths group together below. */
   qsort(ent, (size_t)n, sizeof(*ent), _entry_cmp_width_desc);

   for (i = 0; i < n; i++)
     {
        hash = hash * 31 + ent[i].tex->rolling_hash;
        hash = hash * 31 + (uint32_t)ent[i].rows;
        hash = hash * 31 + (uint32_t)ent[i].width;
     }

   if (!_span_page_ensure(page, gc, page_w, total_h)) goto done;
   pt = (Evas_GL_Texture *)page->evas_tex;

   /* Hand out page offsets.  This has to happen even when the upload is
    * skipped, because the draw code reads them on every pass. */
   row_at = 0;
   for (i = 0; i < n; i++)
     {
        ent[i].tex->page_x = pt->x;
        ent[i].tex->page_y = pt->y + row_at;
        ent[i].tex->dirty  = EINA_FALSE;
        row_at += ent[i].rows;
     }

   ok = EINA_TRUE;

   /* Identical content and layout to the last pass: the rows are already
    * there.  This is what keeps unchanging shapes free, and they are the
    * common case - a pass re-runs whenever anything on the canvas changes,
    * not only when the vector content does. */
   if (hash == page->prev_hash) goto done;
   page->prev_hash = hash;

   glBindTexture(GL_TEXTURE_2D, pt->pt->texture);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   /* The rows are packed tight below.  Evas' own upload paths leave
    * GL_UNPACK_ROW_LENGTH set to the stride they used, so it has to be
    * cleared or the driver reads these rows with the wrong pitch. */
   glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

   /* Walk the sorted entries in runs whose widths are within a factor of
    * two, and upload one rectangle per run.  Uniform shapes - the common
    * case - collapse to a single call; a mix costs one call per width class
    * instead of one per collector, without inflating the bytes. */
   row_at = 0;
   i = 0;
   while (i < n)
     {
        int      g_start = i, g_w = ent[i].width, g_rows = 0;
        size_t   row_bytes, need;
        uint8_t *packed;
        int      j, at;

        while (i < n && ent[i].width * 2 >= g_w)
          {
             g_rows += ent[i].rows;
             i++;
          }

        row_bytes = (size_t)g_w * 4;
        need      = row_bytes * (size_t)g_rows;

        packed = _span_pack_buf_get(need);
        if (!packed) break;

        at = 0;
        for (j = g_start; j < i; j++)
          {
             Span_Texture *tex = ent[j].tex;
             int y;

             /* Sentinel backstop: the collection phase memsets the tail of
              * each row it touches, but chunked callbacks can leave a row
              * without a clean terminator.  Rows with no spans were already
              * zeroed by span_collector_clear. */
             for (y = 0; y < ent[j].rows; y++)
               {
                  int idx = ent[j].counts[y];
                  if (idx > 0 && idx < ent[j].max_spans)
                    tex->buffer[((size_t)y * ent[j].stride) +
                                ((size_t)idx * 4) + 1] = 0;
               }

             for (y = 0; y < ent[j].rows; y++)
               memcpy(packed + (size_t)(at + y) * row_bytes,
                      tex->buffer + (size_t)y * ent[j].stride,
                      row_bytes);
             at += ent[j].rows;
          }

        if (_span_pbo_usable())
          {
             _span_pbo_bind(need);
             glBufferSubData(GL_PIXEL_UNPACK_BUFFER, 0, (GLsizeiptr)need, packed);
             glTexSubImage2D(GL_TEXTURE_2D, 0, pt->x, pt->y + row_at,
                             g_w, g_rows, pt->pt->format,
                             GL_UNSIGNED_BYTE, (const void *)0);
             glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
          }
        else
          {
             glTexSubImage2D(GL_TEXTURE_2D, 0, pt->x, pt->y + row_at,
                             g_w, g_rows, pt->pt->format,
                             GL_UNSIGNED_BYTE, packed);
          }

        row_at += g_rows;
     }

   if (pt->pt->texture != gc->state.current.cur_tex)
     glBindTexture(gc->state.current.tex_target, gc->state.current.cur_tex);

done:
   if (ent != stackbuf) free(ent);
   return ok;
}

/* ------------------------------------------------------------------ */
/* Span drawing                                                        */
/* ------------------------------------------------------------------ */

/* Which program a quad needs.  A plain colour rides in the gradient variant,
 * so only a genuine gradient on either side selects it. */
static Span_Variant
_span_variant_of(const Span_Pipe_Params *p)
{
   int grad = ((p->fill.tex   && p->fill.type   >= SPAN_FILL_TYPE_GRADIENT_MIN) ||
               (p->stroke.tex && p->stroke.type >= SPAN_FILL_TYPE_GRADIENT_MIN));

   if (grad) return (p->mask_tex != 0) ? SPAN_VARIANT_GRADIENT_MASK
                                       : SPAN_VARIANT_GRADIENT;
   return (p->mask_tex != 0) ? SPAN_VARIANT_SOLID_MASK : SPAN_VARIANT_SOLID;
}

/* Issue one batch of span quads.  Shared by the pipe path and by the direct
 * VG pass, which cannot use a pipe entry because it renders into its own
 * framebuffer. */
static void
_span_draw_batch(Evas_Engine_GL_Context *gc, Span_Variant variant,
                 const void *vdata, size_t vbytes, int nverts,
                 GLuint fill_tex, GLuint stroke_tex, GLuint atlas_tex,
                 GLuint mask_tex, float inv_tw, float inv_th)
{
   GLsizei       stride = (GLsizei)span_vertex_size(variant);
   GLuint        vao;

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

   /* Sampler uniforms are program state and never change: unit 0 is always
    * the fill span texture, 1 the stroke, 2 the gradient ramp atlas, 3 the
    * composite mask.  Assign them once per program rather than on every
    * draw. */
   if (!ss->samplers_bound)
     {
        if (ss->loc_fill_spans      >= 0) glUniform1i(ss->loc_fill_spans, 0);
        if (ss->loc_stroke_spans    >= 0) glUniform1i(ss->loc_stroke_spans, 1);
        if (ss->loc_grad_ramp_atlas >= 0) glUniform1i(ss->loc_grad_ramp_atlas, 2);
        if (ss->loc_mask_tex        >= 0) glUniform1i(ss->loc_mask_tex, 3);
        ss->samplers_bound = EINA_TRUE;
     }

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, fill_tex ? fill_tex : stroke_tex);
   glActiveTexture(GL_TEXTURE1);
   glBindTexture(GL_TEXTURE_2D, stroke_tex ? stroke_tex : fill_tex);
   if (ss->loc_grad_ramp_atlas >= 0 && atlas_tex)
     {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, atlas_tex);
     }
   if (ss->loc_mask_tex >= 0 && mask_tex)
     {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, mask_tex);
     }

   if (ss->loc_inv_tw >= 0) glUniform1f(ss->loc_inv_tw, inv_tw);
   if (ss->loc_inv_th >= 0) glUniform1f(ss->loc_inv_th, inv_th);

   /* One streaming VBO shared by every span pipe.  Re-specifying it with
    * glBufferData orphans the previous storage, so the driver never has to
    * stall on data the GPU may still be reading. */
   if (!_span_vbo) glGenBuffers(1, &_span_vbo);
   vao = _span_vao_get(variant);
   if (vao) _gl_bind_vao(vao);
   glBindBuffer(GL_ARRAY_BUFFER, _span_vbo);

   glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)vbytes, vdata, GL_STREAM_DRAW);

   if (!vao)
     {
        /* No vertex array objects: respecify the layout for every draw.
         * Locations left enabled afterwards are benign — image/font shaders
         * bind their own slots explicitly and never fetch from ours. */
        Span_Attr_Desc desc[16];
        int n = _span_attr_layout(variant, desc);
        _span_attr_apply(desc, n, stride);
     }

   glEnable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
   glDisable(GL_SCISSOR_TEST);

   glDrawArrays(GL_TRIANGLES, 0, nverts);

   /* Leave the default vertex array and no array buffer bound so that the
    * client-side array draws of the image/font pipes are unaffected, and
    * hand back texture unit 0 which those paths bind without selecting. */
   if (vao) _gl_bind_vao(0);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
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


/* ------------------------------------------------------------------ */
/* Direct VG pass                                                      */
/* ------------------------------------------------------------------ */

/* Vertex scratch for the direct pass, grown on demand. */
static void  *_span_pass_buf = NULL;
static size_t _span_pass_sz  = 0;

static void *
_span_pass_buf_get(size_t need)
{
   if (need > _span_pass_sz)
     {
        void *p = realloc(_span_pass_buf, need);
        if (!p) return NULL;
        _span_pass_buf = p;
        _span_pass_sz  = need;
     }
   return _span_pass_buf;
}

/* Restore the framebuffer and viewport the Evas pipe expects, given whatever
 * surface it is currently targeting. */
static void
_span_pass_restore(Evas_Engine_GL_Context *gc)
{
   Evas_GL_Image *s = gc->pipe[0].shader.surface;

   if (!s || s == gc->def_surface)
     {
        glsym_glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if ((gc->rot == 0) || (gc->rot == 180))
          glViewport(0, 0, gc->w, gc->h);
        else
          glViewport(0, 0, gc->h, gc->w);
     }
   else
     {
        glsym_glBindFramebuffer(GL_FRAMEBUFFER, s->tex->pt->fb);
        glViewport(s->tex->x, s->tex->y, s->w, s->h);
     }
}

void
span_pass_draw(Evas_Engine_GL_Context *gc, Evas_GL_Image *target,
               const Span_Pipe_Params *quads, const GLfloat *ndc, int n,
               int clear_x, int clear_y, int clear_w, int clear_h)
{
   int i, run_start;

   if (!gc || !target || !target->tex || !target->tex->pt || n <= 0) return;
   if (!span_shader_init()) return;

   /* Bind directly rather than through evas_gl_common_context_target_surface_set:
    * that flushes the pipe, and the pipe is holding the composite draws of
    * every vector object rendered so far this frame.  Leaving them queued is
    * the point - they can then batch into one draw instead of one each. */
   glsym_glBindFramebuffer(GL_FRAMEBUFFER, target->tex->pt->fb);
   glViewport(target->tex->x, target->tex->y, target->w, target->h);

   glEnable(GL_SCISSOR_TEST);
   glScissor(clear_x, clear_y, clear_w, clear_h);
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   glClear(GL_COLOR_BUFFER_BIT);
   glDisable(GL_SCISSOR_TEST);

   /* Walk the quads in runs that share a program and its bindings, so a pass
    * of many shapes costs one draw per distinct binding rather than one per
    * shape. */
   run_start = 0;
   while (run_start < n)
     {
        Span_Variant variant = _span_variant_of(&quads[run_start]);
        size_t vsize, need;
        void *buf;
        int end = run_start + 1, k;

        while (end < n &&
               _span_variant_of(&quads[end]) == variant &&
               quads[end].fill.tex        == quads[run_start].fill.tex &&
               quads[end].stroke.tex      == quads[run_start].stroke.tex &&
               quads[end].grad_atlas_tex  == quads[run_start].grad_atlas_tex &&
               quads[end].mask_tex        == quads[run_start].mask_tex)
          end++;

        vsize = span_vertex_size(variant);
        need  = vsize * 6 * (size_t)(end - run_start);
        buf   = _span_pass_buf_get(need);
        if (!buf) break;

        for (k = run_start; k < end; k++)
          evas_gl_common_span_fill_vertices((char *)buf + vsize * 6 * (size_t)(k - run_start),
                                            variant, &quads[k], ndc + k * 8);

        _span_draw_batch(gc, variant, buf, need, 6 * (end - run_start),
                         quads[run_start].fill.tex, quads[run_start].stroke.tex,
                         quads[run_start].grad_atlas_tex, quads[run_start].mask_tex,
                         1.0f / (float)quads[run_start].pool_w,
                         1.0f / (float)quads[run_start].pool_h);
        run_start = end;
     }

   _span_pass_restore(gc);
}
