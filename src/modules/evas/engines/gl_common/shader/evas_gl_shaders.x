/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED
 * See: ./modules/evas/engines/gl_common/shader/gen_shaders.sh */

#include "../evas_gl_private.h"

static const char const rect_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = vec4(1, 1, 1, 1);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_frag_src =
{
   rect_frag_glsl,
   NULL, 0
};

static const char const rect_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_vert_src =
{
   rect_vert_glsl,
   NULL, 0
};

static const char const rect_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = vec4(1, 1, 1, 1);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_mask_frag_src =
{
   rect_mask_frag_glsl,
   NULL, 0
};

static const char const rect_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_mask_vert_src =
{
   rect_mask_vert_glsl,
   NULL, 0
};

static const char const font_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).aaaa;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_font_frag_src =
{
   font_frag_glsl,
   NULL, 0
};

static const char const font_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_font_vert_src =
{
   font_vert_glsl,
   NULL, 0
};

static const char const font_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).aaaa;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask_frag_src =
{
   font_mask_frag_glsl,
   NULL, 0
};

static const char const font_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask_vert_src =
{
   font_mask_vert_glsl,
   NULL, 0
};

static const char const img_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_frag_src =
{
   img_frag_glsl,
   NULL, 0
};

static const char const img_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_img_vert_src =
{
   img_vert_glsl,
   NULL, 0
};

static const char const img_bgra_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_frag_src =
{
   img_bgra_frag_glsl,
   NULL, 0
};

static const char const img_bgra_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_vert_src =
{
   img_bgra_vert_glsl,
   NULL, 0
};

static const char const img_12_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_frag_src =
{
   img_12_frag_glsl,
   NULL, 0
};

static const char const img_12_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_vert_src =
{
   img_12_vert_glsl,
   NULL, 0
};

static const char const img_21_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_frag_src =
{
   img_21_frag_glsl,
   NULL, 0
};

static const char const img_21_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_vert_src =
{
   img_21_vert_glsl,
   NULL, 0
};

static const char const img_22_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_frag_src =
{
   img_22_frag_glsl,
   NULL, 0
};

static const char const img_22_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_vert_src =
{
   img_22_vert_glsl,
   NULL, 0
};

static const char const img_12_bgra_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_frag_src =
{
   img_12_bgra_frag_glsl,
   NULL, 0
};

static const char const img_12_bgra_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_vert_src =
{
   img_12_bgra_vert_glsl,
   NULL, 0
};

static const char const img_21_bgra_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_frag_src =
{
   img_21_bgra_frag_glsl,
   NULL, 0
};

static const char const img_21_bgra_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_vert_src =
{
   img_21_bgra_vert_glsl,
   NULL, 0
};

static const char const img_22_bgra_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_frag_src =
{
   img_22_bgra_frag_glsl,
   NULL, 0
};

static const char const img_22_bgra_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_vert_src =
{
   img_22_bgra_vert_glsl,
   NULL, 0
};

static const char const img_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_frag_src =
{
   img_mask_frag_glsl,
   NULL, 0
};

static const char const img_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_vert_src =
{
   img_mask_vert_glsl,
   NULL, 0
};

static const char const img_bgra_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask_frag_src =
{
   img_bgra_mask_frag_glsl,
   NULL, 0
};

static const char const img_bgra_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask_vert_src =
{
   img_bgra_mask_vert_glsl,
   NULL, 0
};

static const char const img_12_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask_frag_src =
{
   img_12_mask_frag_glsl,
   NULL, 0
};

static const char const img_12_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask_vert_src =
{
   img_12_mask_vert_glsl,
   NULL, 0
};

static const char const img_21_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask_frag_src =
{
   img_21_mask_frag_glsl,
   NULL, 0
};

static const char const img_21_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask_vert_src =
{
   img_21_mask_vert_glsl,
   NULL, 0
};

static const char const img_22_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask_frag_src =
{
   img_22_mask_frag_glsl,
   NULL, 0
};

static const char const img_22_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask_vert_src =
{
   img_22_mask_vert_glsl,
   NULL, 0
};

static const char const img_12_bgra_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask_frag_src =
{
   img_12_bgra_mask_frag_glsl,
   NULL, 0
};

static const char const img_12_bgra_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask_vert_src =
{
   img_12_bgra_mask_vert_glsl,
   NULL, 0
};

static const char const img_21_bgra_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask_frag_src =
{
   img_21_bgra_mask_frag_glsl,
   NULL, 0
};

static const char const img_21_bgra_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask_vert_src =
{
   img_21_bgra_mask_vert_glsl,
   NULL, 0
};

static const char const img_22_bgra_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask_frag_src =
{
   img_22_bgra_mask_frag_glsl,
   NULL, 0
};

static const char const img_22_bgra_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask_vert_src =
{
   img_22_bgra_mask_vert_glsl,
   NULL, 0
};

static const char const img_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_nomul_frag_src =
{
   img_nomul_frag_glsl,
   NULL, 0
};

static const char const img_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_img_nomul_vert_src =
{
   img_nomul_vert_glsl,
   NULL, 0
};

static const char const img_bgra_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_nomul_frag_src =
{
   img_bgra_nomul_frag_glsl,
   NULL, 0
};

static const char const img_bgra_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_nomul_vert_src =
{
   img_bgra_nomul_vert_glsl,
   NULL, 0
};

static const char const img_12_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_nomul_frag_src =
{
   img_12_nomul_frag_glsl,
   NULL, 0
};

static const char const img_12_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_nomul_vert_src =
{
   img_12_nomul_vert_glsl,
   NULL, 0
};

static const char const img_21_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_nomul_frag_src =
{
   img_21_nomul_frag_glsl,
   NULL, 0
};

static const char const img_21_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_nomul_vert_src =
{
   img_21_nomul_vert_glsl,
   NULL, 0
};

static const char const img_22_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_nomul_frag_src =
{
   img_22_nomul_frag_glsl,
   NULL, 0
};

static const char const img_22_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_nomul_vert_src =
{
   img_22_nomul_vert_glsl,
   NULL, 0
};

static const char const img_12_bgra_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_nomul_frag_src =
{
   img_12_bgra_nomul_frag_glsl,
   NULL, 0
};

static const char const img_12_bgra_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_nomul_vert_src =
{
   img_12_bgra_nomul_vert_glsl,
   NULL, 0
};

static const char const img_21_bgra_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_nomul_frag_src =
{
   img_21_bgra_nomul_frag_glsl,
   NULL, 0
};

static const char const img_21_bgra_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_nomul_vert_src =
{
   img_21_bgra_nomul_vert_glsl,
   NULL, 0
};

static const char const img_22_bgra_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_nomul_frag_src =
{
   img_22_bgra_nomul_frag_glsl,
   NULL, 0
};

static const char const img_22_bgra_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_nomul_vert_src =
{
   img_22_bgra_nomul_vert_glsl,
   NULL, 0
};

static const char const img_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_nomul_frag_src =
{
   img_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const img_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_nomul_vert_src =
{
   img_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const img_bgra_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask_nomul_frag_src =
{
   img_bgra_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const img_bgra_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask_nomul_vert_src =
{
   img_bgra_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const img_12_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask_nomul_frag_src =
{
   img_12_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const img_12_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask_nomul_vert_src =
{
   img_12_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const img_21_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask_nomul_frag_src =
{
   img_21_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const img_21_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask_nomul_vert_src =
{
   img_21_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const img_22_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask_nomul_frag_src =
{
   img_22_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const img_22_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask_nomul_vert_src =
{
   img_22_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const img_12_bgra_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask_nomul_frag_src =
{
   img_12_bgra_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const img_12_bgra_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask_nomul_vert_src =
{
   img_12_bgra_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const img_21_bgra_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask_nomul_frag_src =
{
   img_21_bgra_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const img_21_bgra_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask_nomul_vert_src =
{
   img_21_bgra_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const img_22_bgra_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask_nomul_frag_src =
{
   img_22_bgra_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const img_22_bgra_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask_nomul_vert_src =
{
   img_22_bgra_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const img_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_afill_frag_src =
{
   img_afill_frag_glsl,
   NULL, 0
};

static const char const img_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_img_afill_vert_src =
{
   img_afill_vert_glsl,
   NULL, 0
};

static const char const img_bgra_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_afill_frag_src =
{
   img_bgra_afill_frag_glsl,
   NULL, 0
};

static const char const img_bgra_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_afill_vert_src =
{
   img_bgra_afill_vert_glsl,
   NULL, 0
};

static const char const img_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_nomul_afill_frag_src =
{
   img_nomul_afill_frag_glsl,
   NULL, 0
};

static const char const img_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_img_nomul_afill_vert_src =
{
   img_nomul_afill_vert_glsl,
   NULL, 0
};

static const char const img_bgra_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_nomul_afill_frag_src =
{
   img_bgra_nomul_afill_frag_glsl,
   NULL, 0
};

static const char const img_bgra_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_nomul_afill_vert_src =
{
   img_bgra_nomul_afill_vert_glsl,
   NULL, 0
};

static const char const img_12_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_afill_frag_src =
{
   img_12_afill_frag_glsl,
   NULL, 0
};

static const char const img_12_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_afill_vert_src =
{
   img_12_afill_vert_glsl,
   NULL, 0
};

static const char const img_21_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_afill_frag_src =
{
   img_21_afill_frag_glsl,
   NULL, 0
};

static const char const img_21_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_afill_vert_src =
{
   img_21_afill_vert_glsl,
   NULL, 0
};

static const char const img_22_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_afill_frag_src =
{
   img_22_afill_frag_glsl,
   NULL, 0
};

static const char const img_22_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_afill_vert_src =
{
   img_22_afill_vert_glsl,
   NULL, 0
};

static const char const img_12_bgra_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_afill_frag_src =
{
   img_12_bgra_afill_frag_glsl,
   NULL, 0
};

static const char const img_12_bgra_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_afill_vert_src =
{
   img_12_bgra_afill_vert_glsl,
   NULL, 0
};

static const char const img_21_bgra_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_afill_frag_src =
{
   img_21_bgra_afill_frag_glsl,
   NULL, 0
};

static const char const img_21_bgra_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_afill_vert_src =
{
   img_21_bgra_afill_vert_glsl,
   NULL, 0
};

static const char const img_22_bgra_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_afill_frag_src =
{
   img_22_bgra_afill_frag_glsl,
   NULL, 0
};

static const char const img_22_bgra_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_afill_vert_src =
{
   img_22_bgra_afill_vert_glsl,
   NULL, 0
};

static const char const img_12_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_nomul_afill_frag_src =
{
   img_12_nomul_afill_frag_glsl,
   NULL, 0
};

static const char const img_12_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_nomul_afill_vert_src =
{
   img_12_nomul_afill_vert_glsl,
   NULL, 0
};

static const char const img_21_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_nomul_afill_frag_src =
{
   img_21_nomul_afill_frag_glsl,
   NULL, 0
};

static const char const img_21_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_nomul_afill_vert_src =
{
   img_21_nomul_afill_vert_glsl,
   NULL, 0
};

static const char const img_22_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_nomul_afill_frag_src =
{
   img_22_nomul_afill_frag_glsl,
   NULL, 0
};

static const char const img_22_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_nomul_afill_vert_src =
{
   img_22_nomul_afill_vert_glsl,
   NULL, 0
};

static const char const img_12_bgra_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_nomul_afill_frag_src =
{
   img_12_bgra_nomul_afill_frag_glsl,
   NULL, 0
};

static const char const img_12_bgra_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_nomul_afill_vert_src =
{
   img_12_bgra_nomul_afill_vert_glsl,
   NULL, 0
};

static const char const img_21_bgra_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_nomul_afill_frag_src =
{
   img_21_bgra_nomul_afill_frag_glsl,
   NULL, 0
};

static const char const img_21_bgra_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_nomul_afill_vert_src =
{
   img_21_bgra_nomul_afill_vert_glsl,
   NULL, 0
};

static const char const img_22_bgra_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_nomul_afill_frag_src =
{
   img_22_bgra_nomul_afill_frag_glsl,
   NULL, 0
};

static const char const img_22_bgra_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_sample;\n"
   "varying vec4 div_s;\n"
   "varying vec2 tex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_nomul_afill_vert_src =
{
   img_22_bgra_nomul_afill_vert_glsl,
   NULL, 0
};

static const char const rgb_a_pair_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D texa;\n"
   "varying vec2 tex_a;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = vec4(1, 1, 1, 1);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texa, tex_a).r\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_frag_src =
{
   rgb_a_pair_frag_glsl,
   NULL, 0
};

static const char const rgb_a_pair_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coorda;\n"
   "varying vec2 tex_a;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_a = tex_coorda;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_vert_src =
{
   rgb_a_pair_vert_glsl,
   NULL, 0
};

static const char const rgb_a_pair_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D texa;\n"
   "varying vec2 tex_a;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = vec4(1, 1, 1, 1);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "     * texture2D(texa, tex_a).r\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_frag_src =
{
   rgb_a_pair_mask_frag_glsl,
   NULL, 0
};

static const char const rgb_a_pair_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coorda;\n"
   "varying vec2 tex_a;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_a = tex_coorda;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_vert_src =
{
   rgb_a_pair_mask_vert_glsl,
   NULL, 0
};

static const char const rgb_a_pair_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D texa;\n"
   "varying vec2 tex_a;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = vec4(1, 1, 1, 1);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texa, tex_a).r\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_nomul_frag_src =
{
   rgb_a_pair_nomul_frag_glsl,
   NULL, 0
};

static const char const rgb_a_pair_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coorda;\n"
   "varying vec2 tex_a;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_a = tex_coorda;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_nomul_vert_src =
{
   rgb_a_pair_nomul_vert_glsl,
   NULL, 0
};

static const char const rgb_a_pair_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D texa;\n"
   "varying vec2 tex_a;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = vec4(1, 1, 1, 1);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "     * texture2D(texa, tex_a).r\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_nomul_frag_src =
{
   rgb_a_pair_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const rgb_a_pair_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coorda;\n"
   "varying vec2 tex_a;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_a = tex_coorda;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_nomul_vert_src =
{
   rgb_a_pair_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const tex_external_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "uniform samplerExternalOES tex;\n"
   "#else\n"
   "uniform sampler2D tex;\n"
   "#endif\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_frag_src =
{
   tex_external_frag_glsl,
   NULL, 0
};

static const char const tex_external_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_vert_src =
{
   tex_external_vert_glsl,
   NULL, 0
};

static const char const tex_external_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "uniform samplerExternalOES tex;\n"
   "#else\n"
   "uniform sampler2D tex;\n"
   "#endif\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_afill_frag_src =
{
   tex_external_afill_frag_glsl,
   NULL, 0
};

static const char const tex_external_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_afill_vert_src =
{
   tex_external_afill_vert_glsl,
   NULL, 0
};

static const char const tex_external_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "uniform samplerExternalOES tex;\n"
   "#else\n"
   "uniform sampler2D tex;\n"
   "#endif\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_nomul_frag_src =
{
   tex_external_nomul_frag_glsl,
   NULL, 0
};

static const char const tex_external_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_nomul_vert_src =
{
   tex_external_nomul_vert_glsl,
   NULL, 0
};

static const char const tex_external_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "uniform samplerExternalOES tex;\n"
   "#else\n"
   "uniform sampler2D tex;\n"
   "#endif\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "   gl_FragColor.a = 1.0;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_nomul_afill_frag_src =
{
   tex_external_nomul_afill_frag_glsl,
   NULL, 0
};

static const char const tex_external_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_nomul_afill_vert_src =
{
   tex_external_nomul_afill_vert_glsl,
   NULL, 0
};

static const char const tex_external_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "uniform samplerExternalOES tex;\n"
   "#else\n"
   "uniform sampler2D tex;\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_mask_frag_src =
{
   tex_external_mask_frag_glsl,
   NULL, 0
};

static const char const tex_external_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_mask_vert_src =
{
   tex_external_mask_vert_glsl,
   NULL, 0
};

static const char const tex_external_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "uniform samplerExternalOES tex;\n"
   "#else\n"
   "uniform sampler2D tex;\n"
   "#endif\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_mask_nomul_frag_src =
{
   tex_external_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const tex_external_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_mask_nomul_vert_src =
{
   tex_external_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const yuv_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texu;\n"
   "uniform sampler2D texv;\n"
   "varying vec2 tex_c2;\n"
   "varying vec2 tex_c3;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float r, g, b, y, u, v;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   y = (y - 0.0625) * 1.164;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   r = y + (1.402 * v);\n"
   "   g = y - (0.34414 * u) - (0.71414 * v);\n"
   "   b = y + (1.772 * u);\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_frag_src =
{
   yuv_frag_glsl,
   NULL, 0
};

static const char const yuv_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "attribute vec2 tex_coord3;\n"
   "varying vec2 tex_c3;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = tex_coord2;\n"
   "   tex_c3 = tex_coord3;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_vert_src =
{
   yuv_vert_glsl,
   NULL, 0
};

static const char const yuv_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texu;\n"
   "uniform sampler2D texv;\n"
   "varying vec2 tex_c2;\n"
   "varying vec2 tex_c3;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float r, g, b, y, u, v;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   y = (y - 0.0625) * 1.164;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   r = y + (1.402 * v);\n"
   "   g = y - (0.34414 * u) - (0.71414 * v);\n"
   "   b = y + (1.772 * u);\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_nomul_frag_src =
{
   yuv_nomul_frag_glsl,
   NULL, 0
};

static const char const yuv_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "attribute vec2 tex_coord3;\n"
   "varying vec2 tex_c3;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = tex_coord2;\n"
   "   tex_c3 = tex_coord3;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_nomul_vert_src =
{
   yuv_nomul_vert_glsl,
   NULL, 0
};

static const char const yuv_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texu;\n"
   "uniform sampler2D texv;\n"
   "varying vec2 tex_c2;\n"
   "varying vec2 tex_c3;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float r, g, b, y, u, v;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   y = (y - 0.0625) * 1.164;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   r = y + (1.402 * v);\n"
   "   g = y - (0.34414 * u) - (0.71414 * v);\n"
   "   b = y + (1.772 * u);\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_mask_frag_src =
{
   yuv_mask_frag_glsl,
   NULL, 0
};

static const char const yuv_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "attribute vec2 tex_coord3;\n"
   "varying vec2 tex_c3;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = tex_coord2;\n"
   "   tex_c3 = tex_coord3;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_mask_vert_src =
{
   yuv_mask_vert_glsl,
   NULL, 0
};

static const char const yuv_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texu;\n"
   "uniform sampler2D texv;\n"
   "varying vec2 tex_c2;\n"
   "varying vec2 tex_c3;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float r, g, b, y, u, v;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   y = (y - 0.0625) * 1.164;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   r = y + (1.402 * v);\n"
   "   g = y - (0.34414 * u) - (0.71414 * v);\n"
   "   b = y + (1.772 * u);\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_mask_nomul_frag_src =
{
   yuv_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const yuv_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "attribute vec2 tex_coord3;\n"
   "varying vec2 tex_c3;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = tex_coord2;\n"
   "   tex_c3 = tex_coord3;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_mask_nomul_vert_src =
{
   yuv_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const yuy2_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texuv;\n"
   "varying vec2 tex_c2;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float y, u, v, vmu, r, g, b;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   vmu = v * 0.813 + u * 0.391;\n"
   "   u = u * 2.018;\n"
   "   v = v * 1.596;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_frag_src =
{
   yuy2_frag_glsl,
   NULL, 0
};

static const char const yuy2_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = vec2(tex_coord2.x * 0.5, tex_coord2.y);\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_vert_src =
{
   yuy2_vert_glsl,
   NULL, 0
};

static const char const yuy2_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texuv;\n"
   "varying vec2 tex_c2;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float y, u, v, vmu, r, g, b;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   vmu = v * 0.813 + u * 0.391;\n"
   "   u = u * 2.018;\n"
   "   v = v * 1.596;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_nomul_frag_src =
{
   yuy2_nomul_frag_glsl,
   NULL, 0
};

static const char const yuy2_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = vec2(tex_coord2.x * 0.5, tex_coord2.y);\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_nomul_vert_src =
{
   yuy2_nomul_vert_glsl,
   NULL, 0
};

static const char const yuy2_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texuv;\n"
   "varying vec2 tex_c2;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float y, u, v, vmu, r, g, b;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   vmu = v * 0.813 + u * 0.391;\n"
   "   u = u * 2.018;\n"
   "   v = v * 1.596;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_mask_frag_src =
{
   yuy2_mask_frag_glsl,
   NULL, 0
};

static const char const yuy2_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = vec2(tex_coord2.x * 0.5, tex_coord2.y);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_mask_vert_src =
{
   yuy2_mask_vert_glsl,
   NULL, 0
};

static const char const yuy2_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texuv;\n"
   "varying vec2 tex_c2;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float y, u, v, vmu, r, g, b;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   vmu = v * 0.813 + u * 0.391;\n"
   "   u = u * 2.018;\n"
   "   v = v * 1.596;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_mask_nomul_frag_src =
{
   yuy2_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const yuy2_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = vec2(tex_coord2.x * 0.5, tex_coord2.y);\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_mask_nomul_vert_src =
{
   yuy2_mask_nomul_vert_glsl,
   NULL, 0
};

static const char const nv12_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texuv;\n"
   "varying vec2 tex_c2;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float y, u, v, vmu, r, g, b;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   vmu = v * 0.813 + u * 0.391;\n"
   "   u = u * 2.018;\n"
   "   v = v * 1.596;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_frag_src =
{
   nv12_frag_glsl,
   NULL, 0
};

static const char const nv12_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = tex_coord2 * 0.5;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_vert_src =
{
   nv12_vert_glsl,
   NULL, 0
};

static const char const nv12_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texuv;\n"
   "varying vec2 tex_c2;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float y, u, v, vmu, r, g, b;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   vmu = v * 0.813 + u * 0.391;\n"
   "   u = u * 2.018;\n"
   "   v = v * 1.596;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_nomul_frag_src =
{
   nv12_nomul_frag_glsl,
   NULL, 0
};

static const char const nv12_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = tex_coord2 * 0.5;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_nomul_vert_src =
{
   nv12_nomul_vert_glsl,
   NULL, 0
};

static const char const nv12_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texuv;\n"
   "varying vec2 tex_c2;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float y, u, v, vmu, r, g, b;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   vmu = v * 0.813 + u * 0.391;\n"
   "   u = u * 2.018;\n"
   "   v = v * 1.596;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_mask_frag_src =
{
   nv12_mask_frag_glsl,
   NULL, 0
};

static const char const nv12_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = tex_coord2 * 0.5;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_mask_vert_src =
{
   nv12_mask_vert_glsl,
   NULL, 0
};

static const char const nv12_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texuv;\n"
   "varying vec2 tex_c2;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   float y, u, v, vmu, r, g, b;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   vmu = v * 0.813 + u * 0.391;\n"
   "   u = u * 2.018;\n"
   "   v = v * 1.596;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texm, tex_m).a\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_mask_nomul_frag_src =
{
   nv12_mask_nomul_frag_glsl,
   NULL, 0
};

static const char const nv12_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coord2;\n"
   "varying vec2 tex_c2;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = tex_coord2 * 0.5;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_mask_nomul_vert_src =
{
   nv12_mask_nomul_vert_glsl,
   NULL, 0
};


typedef enum _Shader_Sampling Shader_Sampling;
enum _Shader_Sampling {
   SHADER_SAM11,
   SHADER_SAM12,
   SHADER_SAM21,
   SHADER_SAM22
};

static const struct {
   Evas_GL_Shader id;
   Evas_GL_Program_Source *vert;
   Evas_GL_Program_Source *frag;
   const char *name;
   unsigned char type; // RTYPE
   Shader_Sampling sam;
   Eina_Bool bgra : 1;
   Eina_Bool mask : 1;
   Eina_Bool nomul : 1;
   Eina_Bool afill : 1;
} _shaders_source[] = {
   { SHADER_RECT, &(shader_rect_vert_src), &(shader_rect_frag_src), "rect", RTYPE_RECT, SHADER_SAM11, 0, 0, 0, 0 },
   { SHADER_RECT_MASK, &(shader_rect_mask_vert_src), &(shader_rect_mask_frag_src), "rect_mask", RTYPE_RECT, SHADER_SAM11, 0, 1, 0, 0 },
   { SHADER_FONT, &(shader_font_vert_src), &(shader_font_frag_src), "font", RTYPE_FONT, SHADER_SAM11, 0, 0, 0, 0 },
   { SHADER_FONT_MASK, &(shader_font_mask_vert_src), &(shader_font_mask_frag_src), "font_mask", RTYPE_FONT, SHADER_SAM11, 0, 1, 0, 0 },
   { SHADER_IMG, &(shader_img_vert_src), &(shader_img_frag_src), "img", RTYPE_IMAGE, SHADER_SAM11, 0, 0, 0, 0 },
   { SHADER_IMG_BGRA, &(shader_img_bgra_vert_src), &(shader_img_bgra_frag_src), "img_bgra", RTYPE_IMAGE, SHADER_SAM11, 1, 0, 0, 0 },
   { SHADER_IMG_12, &(shader_img_12_vert_src), &(shader_img_12_frag_src), "img_12", RTYPE_IMAGE, SHADER_SAM12, 0, 0, 0, 0 },
   { SHADER_IMG_21, &(shader_img_21_vert_src), &(shader_img_21_frag_src), "img_21", RTYPE_IMAGE, SHADER_SAM21, 0, 0, 0, 0 },
   { SHADER_IMG_22, &(shader_img_22_vert_src), &(shader_img_22_frag_src), "img_22", RTYPE_IMAGE, SHADER_SAM22, 0, 0, 0, 0 },
   { SHADER_IMG_12_BGRA, &(shader_img_12_bgra_vert_src), &(shader_img_12_bgra_frag_src), "img_12_bgra", RTYPE_IMAGE, SHADER_SAM12, 1, 0, 0, 0 },
   { SHADER_IMG_21_BGRA, &(shader_img_21_bgra_vert_src), &(shader_img_21_bgra_frag_src), "img_21_bgra", RTYPE_IMAGE, SHADER_SAM21, 1, 0, 0, 0 },
   { SHADER_IMG_22_BGRA, &(shader_img_22_bgra_vert_src), &(shader_img_22_bgra_frag_src), "img_22_bgra", RTYPE_IMAGE, SHADER_SAM22, 1, 0, 0, 0 },
   { SHADER_IMG_MASK, &(shader_img_mask_vert_src), &(shader_img_mask_frag_src), "img_mask", RTYPE_IMAGE, SHADER_SAM11, 0, 1, 0, 0 },
   { SHADER_IMG_BGRA_MASK, &(shader_img_bgra_mask_vert_src), &(shader_img_bgra_mask_frag_src), "img_bgra_mask", RTYPE_IMAGE, SHADER_SAM11, 1, 1, 0, 0 },
   { SHADER_IMG_12_MASK, &(shader_img_12_mask_vert_src), &(shader_img_12_mask_frag_src), "img_12_mask", RTYPE_IMAGE, SHADER_SAM12, 0, 1, 0, 0 },
   { SHADER_IMG_21_MASK, &(shader_img_21_mask_vert_src), &(shader_img_21_mask_frag_src), "img_21_mask", RTYPE_IMAGE, SHADER_SAM21, 0, 1, 0, 0 },
   { SHADER_IMG_22_MASK, &(shader_img_22_mask_vert_src), &(shader_img_22_mask_frag_src), "img_22_mask", RTYPE_IMAGE, SHADER_SAM22, 0, 1, 0, 0 },
   { SHADER_IMG_12_BGRA_MASK, &(shader_img_12_bgra_mask_vert_src), &(shader_img_12_bgra_mask_frag_src), "img_12_bgra_mask", RTYPE_IMAGE, SHADER_SAM12, 1, 1, 0, 0 },
   { SHADER_IMG_21_BGRA_MASK, &(shader_img_21_bgra_mask_vert_src), &(shader_img_21_bgra_mask_frag_src), "img_21_bgra_mask", RTYPE_IMAGE, SHADER_SAM21, 1, 1, 0, 0 },
   { SHADER_IMG_22_BGRA_MASK, &(shader_img_22_bgra_mask_vert_src), &(shader_img_22_bgra_mask_frag_src), "img_22_bgra_mask", RTYPE_IMAGE, SHADER_SAM22, 1, 1, 0, 0 },
   { SHADER_IMG_NOMUL, &(shader_img_nomul_vert_src), &(shader_img_nomul_frag_src), "img_nomul", RTYPE_IMAGE, SHADER_SAM11, 0, 0, 1, 0 },
   { SHADER_IMG_BGRA_NOMUL, &(shader_img_bgra_nomul_vert_src), &(shader_img_bgra_nomul_frag_src), "img_bgra_nomul", RTYPE_IMAGE, SHADER_SAM11, 1, 0, 1, 0 },
   { SHADER_IMG_12_NOMUL, &(shader_img_12_nomul_vert_src), &(shader_img_12_nomul_frag_src), "img_12_nomul", RTYPE_IMAGE, SHADER_SAM12, 0, 0, 1, 0 },
   { SHADER_IMG_21_NOMUL, &(shader_img_21_nomul_vert_src), &(shader_img_21_nomul_frag_src), "img_21_nomul", RTYPE_IMAGE, SHADER_SAM21, 0, 0, 1, 0 },
   { SHADER_IMG_22_NOMUL, &(shader_img_22_nomul_vert_src), &(shader_img_22_nomul_frag_src), "img_22_nomul", RTYPE_IMAGE, SHADER_SAM22, 0, 0, 1, 0 },
   { SHADER_IMG_12_BGRA_NOMUL, &(shader_img_12_bgra_nomul_vert_src), &(shader_img_12_bgra_nomul_frag_src), "img_12_bgra_nomul", RTYPE_IMAGE, SHADER_SAM12, 1, 0, 1, 0 },
   { SHADER_IMG_21_BGRA_NOMUL, &(shader_img_21_bgra_nomul_vert_src), &(shader_img_21_bgra_nomul_frag_src), "img_21_bgra_nomul", RTYPE_IMAGE, SHADER_SAM21, 1, 0, 1, 0 },
   { SHADER_IMG_22_BGRA_NOMUL, &(shader_img_22_bgra_nomul_vert_src), &(shader_img_22_bgra_nomul_frag_src), "img_22_bgra_nomul", RTYPE_IMAGE, SHADER_SAM22, 1, 0, 1, 0 },
   { SHADER_IMG_MASK_NOMUL, &(shader_img_mask_nomul_vert_src), &(shader_img_mask_nomul_frag_src), "img_mask_nomul", RTYPE_IMAGE, SHADER_SAM11, 0, 1, 1, 0 },
   { SHADER_IMG_BGRA_MASK_NOMUL, &(shader_img_bgra_mask_nomul_vert_src), &(shader_img_bgra_mask_nomul_frag_src), "img_bgra_mask_nomul", RTYPE_IMAGE, SHADER_SAM11, 1, 1, 1, 0 },
   { SHADER_IMG_12_MASK_NOMUL, &(shader_img_12_mask_nomul_vert_src), &(shader_img_12_mask_nomul_frag_src), "img_12_mask_nomul", RTYPE_IMAGE, SHADER_SAM12, 0, 1, 1, 0 },
   { SHADER_IMG_21_MASK_NOMUL, &(shader_img_21_mask_nomul_vert_src), &(shader_img_21_mask_nomul_frag_src), "img_21_mask_nomul", RTYPE_IMAGE, SHADER_SAM21, 0, 1, 1, 0 },
   { SHADER_IMG_22_MASK_NOMUL, &(shader_img_22_mask_nomul_vert_src), &(shader_img_22_mask_nomul_frag_src), "img_22_mask_nomul", RTYPE_IMAGE, SHADER_SAM22, 0, 1, 1, 0 },
   { SHADER_IMG_12_BGRA_MASK_NOMUL, &(shader_img_12_bgra_mask_nomul_vert_src), &(shader_img_12_bgra_mask_nomul_frag_src), "img_12_bgra_mask_nomul", RTYPE_IMAGE, SHADER_SAM12, 1, 1, 1, 0 },
   { SHADER_IMG_21_BGRA_MASK_NOMUL, &(shader_img_21_bgra_mask_nomul_vert_src), &(shader_img_21_bgra_mask_nomul_frag_src), "img_21_bgra_mask_nomul", RTYPE_IMAGE, SHADER_SAM21, 1, 1, 1, 0 },
   { SHADER_IMG_22_BGRA_MASK_NOMUL, &(shader_img_22_bgra_mask_nomul_vert_src), &(shader_img_22_bgra_mask_nomul_frag_src), "img_22_bgra_mask_nomul", RTYPE_IMAGE, SHADER_SAM22, 1, 1, 1, 0 },
   { SHADER_IMG_AFILL, &(shader_img_afill_vert_src), &(shader_img_afill_frag_src), "img_afill", RTYPE_IMAGE, SHADER_SAM11, 0, 0, 0, 1 },
   { SHADER_IMG_BGRA_AFILL, &(shader_img_bgra_afill_vert_src), &(shader_img_bgra_afill_frag_src), "img_bgra_afill", RTYPE_IMAGE, SHADER_SAM11, 1, 0, 0, 1 },
   { SHADER_IMG_NOMUL_AFILL, &(shader_img_nomul_afill_vert_src), &(shader_img_nomul_afill_frag_src), "img_nomul_afill", RTYPE_IMAGE, SHADER_SAM11, 0, 0, 1, 1 },
   { SHADER_IMG_BGRA_NOMUL_AFILL, &(shader_img_bgra_nomul_afill_vert_src), &(shader_img_bgra_nomul_afill_frag_src), "img_bgra_nomul_afill", RTYPE_IMAGE, SHADER_SAM11, 1, 0, 1, 1 },
   { SHADER_IMG_12_AFILL, &(shader_img_12_afill_vert_src), &(shader_img_12_afill_frag_src), "img_12_afill", RTYPE_IMAGE, SHADER_SAM12, 0, 0, 0, 1 },
   { SHADER_IMG_21_AFILL, &(shader_img_21_afill_vert_src), &(shader_img_21_afill_frag_src), "img_21_afill", RTYPE_IMAGE, SHADER_SAM21, 0, 0, 0, 1 },
   { SHADER_IMG_22_AFILL, &(shader_img_22_afill_vert_src), &(shader_img_22_afill_frag_src), "img_22_afill", RTYPE_IMAGE, SHADER_SAM22, 0, 0, 0, 1 },
   { SHADER_IMG_12_BGRA_AFILL, &(shader_img_12_bgra_afill_vert_src), &(shader_img_12_bgra_afill_frag_src), "img_12_bgra_afill", RTYPE_IMAGE, SHADER_SAM12, 1, 0, 0, 1 },
   { SHADER_IMG_21_BGRA_AFILL, &(shader_img_21_bgra_afill_vert_src), &(shader_img_21_bgra_afill_frag_src), "img_21_bgra_afill", RTYPE_IMAGE, SHADER_SAM21, 1, 0, 0, 1 },
   { SHADER_IMG_22_BGRA_AFILL, &(shader_img_22_bgra_afill_vert_src), &(shader_img_22_bgra_afill_frag_src), "img_22_bgra_afill", RTYPE_IMAGE, SHADER_SAM22, 1, 0, 0, 1 },
   { SHADER_IMG_12_NOMUL_AFILL, &(shader_img_12_nomul_afill_vert_src), &(shader_img_12_nomul_afill_frag_src), "img_12_nomul_afill", RTYPE_IMAGE, SHADER_SAM12, 0, 0, 1, 1 },
   { SHADER_IMG_21_NOMUL_AFILL, &(shader_img_21_nomul_afill_vert_src), &(shader_img_21_nomul_afill_frag_src), "img_21_nomul_afill", RTYPE_IMAGE, SHADER_SAM21, 0, 0, 1, 1 },
   { SHADER_IMG_22_NOMUL_AFILL, &(shader_img_22_nomul_afill_vert_src), &(shader_img_22_nomul_afill_frag_src), "img_22_nomul_afill", RTYPE_IMAGE, SHADER_SAM22, 0, 0, 1, 1 },
   { SHADER_IMG_12_BGRA_NOMUL_AFILL, &(shader_img_12_bgra_nomul_afill_vert_src), &(shader_img_12_bgra_nomul_afill_frag_src), "img_12_bgra_nomul_afill", RTYPE_IMAGE, SHADER_SAM12, 1, 0, 1, 1 },
   { SHADER_IMG_21_BGRA_NOMUL_AFILL, &(shader_img_21_bgra_nomul_afill_vert_src), &(shader_img_21_bgra_nomul_afill_frag_src), "img_21_bgra_nomul_afill", RTYPE_IMAGE, SHADER_SAM21, 1, 0, 1, 1 },
   { SHADER_IMG_22_BGRA_NOMUL_AFILL, &(shader_img_22_bgra_nomul_afill_vert_src), &(shader_img_22_bgra_nomul_afill_frag_src), "img_22_bgra_nomul_afill", RTYPE_IMAGE, SHADER_SAM22, 1, 0, 1, 1 },
   { SHADER_RGB_A_PAIR, &(shader_rgb_a_pair_vert_src), &(shader_rgb_a_pair_frag_src), "rgb_a_pair", RTYPE_RGB_A_PAIR, SHADER_SAM11, 0, 0, 0, 0 },
   { SHADER_RGB_A_PAIR_MASK, &(shader_rgb_a_pair_mask_vert_src), &(shader_rgb_a_pair_mask_frag_src), "rgb_a_pair_mask", RTYPE_RGB_A_PAIR, SHADER_SAM11, 0, 1, 0, 0 },
   { SHADER_RGB_A_PAIR_NOMUL, &(shader_rgb_a_pair_nomul_vert_src), &(shader_rgb_a_pair_nomul_frag_src), "rgb_a_pair_nomul", RTYPE_RGB_A_PAIR, SHADER_SAM11, 0, 0, 1, 0 },
   { SHADER_RGB_A_PAIR_MASK_NOMUL, &(shader_rgb_a_pair_mask_nomul_vert_src), &(shader_rgb_a_pair_mask_nomul_frag_src), "rgb_a_pair_mask_nomul", RTYPE_RGB_A_PAIR, SHADER_SAM11, 0, 1, 1, 0 },
   { SHADER_TEX_EXTERNAL, &(shader_tex_external_vert_src), &(shader_tex_external_frag_src), "tex_external", RTYPE_TEX_EXTERNAL, SHADER_SAM11, 0, 0, 0, 0 },
   { SHADER_TEX_EXTERNAL_AFILL, &(shader_tex_external_afill_vert_src), &(shader_tex_external_afill_frag_src), "tex_external_afill", RTYPE_TEX_EXTERNAL, SHADER_SAM11, 0, 0, 0, 1 },
   { SHADER_TEX_EXTERNAL_NOMUL, &(shader_tex_external_nomul_vert_src), &(shader_tex_external_nomul_frag_src), "tex_external_nomul", RTYPE_TEX_EXTERNAL, SHADER_SAM11, 0, 0, 1, 0 },
   { SHADER_TEX_EXTERNAL_NOMUL_AFILL, &(shader_tex_external_nomul_afill_vert_src), &(shader_tex_external_nomul_afill_frag_src), "tex_external_nomul_afill", RTYPE_TEX_EXTERNAL, SHADER_SAM11, 0, 0, 1, 1 },
   { SHADER_TEX_EXTERNAL_MASK, &(shader_tex_external_mask_vert_src), &(shader_tex_external_mask_frag_src), "tex_external_mask", RTYPE_TEX_EXTERNAL, SHADER_SAM11, 0, 1, 0, 0 },
   { SHADER_TEX_EXTERNAL_MASK_NOMUL, &(shader_tex_external_mask_nomul_vert_src), &(shader_tex_external_mask_nomul_frag_src), "tex_external_mask_nomul", RTYPE_TEX_EXTERNAL, SHADER_SAM11, 0, 1, 1, 0 },
   { SHADER_YUV, &(shader_yuv_vert_src), &(shader_yuv_frag_src), "yuv", RTYPE_YUV, SHADER_SAM11, 0, 0, 0, 0 },
   { SHADER_YUV_NOMUL, &(shader_yuv_nomul_vert_src), &(shader_yuv_nomul_frag_src), "yuv_nomul", RTYPE_YUV, SHADER_SAM11, 0, 0, 1, 0 },
   { SHADER_YUV_MASK, &(shader_yuv_mask_vert_src), &(shader_yuv_mask_frag_src), "yuv_mask", RTYPE_YUV, SHADER_SAM11, 0, 1, 0, 0 },
   { SHADER_YUV_MASK_NOMUL, &(shader_yuv_mask_nomul_vert_src), &(shader_yuv_mask_nomul_frag_src), "yuv_mask_nomul", RTYPE_YUV, SHADER_SAM11, 0, 1, 1, 0 },
   { SHADER_YUY2, &(shader_yuy2_vert_src), &(shader_yuy2_frag_src), "yuy2", RTYPE_YUY2, SHADER_SAM11, 0, 0, 0, 0 },
   { SHADER_YUY2_NOMUL, &(shader_yuy2_nomul_vert_src), &(shader_yuy2_nomul_frag_src), "yuy2_nomul", RTYPE_YUY2, SHADER_SAM11, 0, 0, 1, 0 },
   { SHADER_YUY2_MASK, &(shader_yuy2_mask_vert_src), &(shader_yuy2_mask_frag_src), "yuy2_mask", RTYPE_YUY2, SHADER_SAM11, 0, 1, 0, 0 },
   { SHADER_YUY2_MASK_NOMUL, &(shader_yuy2_mask_nomul_vert_src), &(shader_yuy2_mask_nomul_frag_src), "yuy2_mask_nomul", RTYPE_YUY2, SHADER_SAM11, 0, 1, 1, 0 },
   { SHADER_NV12, &(shader_nv12_vert_src), &(shader_nv12_frag_src), "nv12", RTYPE_NV12, SHADER_SAM11, 0, 0, 0, 0 },
   { SHADER_NV12_NOMUL, &(shader_nv12_nomul_vert_src), &(shader_nv12_nomul_frag_src), "nv12_nomul", RTYPE_NV12, SHADER_SAM11, 0, 0, 1, 0 },
   { SHADER_NV12_MASK, &(shader_nv12_mask_vert_src), &(shader_nv12_mask_frag_src), "nv12_mask", RTYPE_NV12, SHADER_SAM11, 0, 1, 0, 0 },
   { SHADER_NV12_MASK_NOMUL, &(shader_nv12_mask_nomul_vert_src), &(shader_nv12_mask_nomul_frag_src), "nv12_mask_nomul", RTYPE_NV12, SHADER_SAM11, 0, 1, 1, 0 },
};

