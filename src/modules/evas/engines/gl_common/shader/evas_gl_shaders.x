/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED
 * See: modules/evas/engines/gl_common/shader/gen_shaders.sh */

#include "../evas_gl_private.h"

/* Source: modules/evas/engines/gl_common/shader/font_frag.shd */
static const char const font_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(tex, tex_c.xy).aaaa * col;\n"
   "}\n";
Evas_GL_Program_Source shader_font_frag_src =
{
   font_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/font_vert.shd */
static const char const font_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
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

/* Source: modules/evas/engines/gl_common/shader/font_mask_frag.shd */
static const char const font_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "uniform sampler2D texm;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(tex, tex_c.xy).aaaa * texture2D(texm, tex_m.xy).aaaa * col;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask_frag_src =
{
   font_mask_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/font_mask_vert.shd */
static const char const font_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_coordm;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_m = tex_coordm;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask_vert_src =
{
   font_mask_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_12_bgra_frag.shd */
static const char const img_12_bgra_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   gl_FragColor = ((col00 + col01) / div_s) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_frag_src =
{
   img_12_bgra_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_12_bgra_nomul_frag.shd */
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
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   gl_FragColor = (col00 + col01) / div_s;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_nomul_frag_src =
{
   img_12_bgra_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_12_bgra_nomul_vert.shd */
static const char const img_12_bgra_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0,  tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_nomul_vert_src =
{
   img_12_bgra_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_12_bgra_vert.shd */
static const char const img_12_bgra_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0,  tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_vert_src =
{
   img_12_bgra_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_12_frag.shd */
static const char const img_12_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   gl_FragColor = ((col00 + col01) / div_s) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_frag_src =
{
   img_12_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_12_nomul_frag.shd */
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
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   gl_FragColor = (col00 + col01) / div_s;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_nomul_frag_src =
{
   img_12_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_12_nomul_vert.shd */
static const char const img_12_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0,  tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_nomul_vert_src =
{
   img_12_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_12_vert.shd */
static const char const img_12_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0,  tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_vert_src =
{
   img_12_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_21_bgra_frag.shd */
static const char const img_21_bgra_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   gl_FragColor = ((col00 + col01) / div_s) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_frag_src =
{
   img_21_bgra_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_21_bgra_nomul_frag.shd */
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
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   gl_FragColor = (col00 + col01) / div_s;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_nomul_frag_src =
{
   img_21_bgra_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_21_bgra_nomul_vert.shd */
static const char const img_21_bgra_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
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

/* Source: modules/evas/engines/gl_common/shader/img_21_bgra_vert.shd */
static const char const img_21_bgra_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
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

/* Source: modules/evas/engines/gl_common/shader/img_21_frag.shd */
static const char const img_21_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   gl_FragColor = ((col00 + col01) / div_s) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_frag_src =
{
   img_21_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_21_nomul_frag.shd */
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
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   gl_FragColor = (col00 + col01) / div_s;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_nomul_frag_src =
{
   img_21_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_21_nomul_vert.shd */
static const char const img_21_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
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

/* Source: modules/evas/engines/gl_common/shader/img_21_vert.shd */
static const char const img_21_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
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

/* Source: modules/evas/engines/gl_common/shader/img_22_bgra_frag.shd */
static const char const img_22_bgra_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]);\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]);\n"
   "   gl_FragColor = ((col00 + col01 + col10 + col11) / div_s) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_frag_src =
{
   img_22_bgra_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_22_bgra_nomul_frag.shd */
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
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]);\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]);\n"
   "   gl_FragColor = (col00 + col01 + col10 + col11) / div_s;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_nomul_frag_src =
{
   img_22_bgra_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_22_bgra_nomul_vert.shd */
static const char const img_22_bgra_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x,  tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x,  tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_nomul_vert_src =
{
   img_22_bgra_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_22_bgra_vert.shd */
static const char const img_22_bgra_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x,  tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x,  tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_vert_src =
{
   img_22_bgra_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_22_frag.shd */
static const char const img_22_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   gl_FragColor = ((col00 + col01 + col10 + col11) / div_s) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_frag_src =
{
   img_22_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_22_nomul_frag.shd */
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
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   gl_FragColor = (col00 + col01 + col10 + col11) / div_s;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_nomul_frag_src =
{
   img_22_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_22_nomul_vert.shd */
static const char const img_22_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x,  tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x,  tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_nomul_vert_src =
{
   img_22_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_22_vert.shd */
static const char const img_22_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x,  tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x,  tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_vert_src =
{
   img_22_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_bgra_frag.shd */
static const char const img_bgra_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(tex, tex_c.xy) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_frag_src =
{
   img_bgra_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_bgra_nomul_frag.shd */
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
   "   gl_FragColor = texture2D(tex, tex_c.xy);\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_nomul_frag_src =
{
   img_bgra_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_bgra_nomul_vert.shd */
static const char const img_bgra_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "uniform mat4 mvp;\n"
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

/* Source: modules/evas/engines/gl_common/shader/img_bgra_vert.shd */
static const char const img_bgra_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
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

/* Source: modules/evas/engines/gl_common/shader/img_frag.shd */
static const char const img_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(tex, tex_c.xy).bgra * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_frag_src =
{
   img_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_nomul_frag.shd */
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
   "   gl_FragColor = texture2D(tex, tex_c.xy).bgra;\n"
   "}\n";
Evas_GL_Program_Source shader_img_nomul_frag_src =
{
   img_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_nomul_vert.shd */
static const char const img_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "uniform mat4 mvp;\n"
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

/* Source: modules/evas/engines/gl_common/shader/img_vert.shd */
static const char const img_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
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

/* Source: modules/evas/engines/gl_common/shader/nv12_frag.shd */
static const char const nv12_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex, texuv;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_cuv;\n"
   "void main()\n"
   "{\n"
   "  float y,u,v,vmu,r,g,b;\n"
   "  y=texture2D(tex,tex_c).g;\n"
   "  u=texture2D(texuv,tex_cuv).g;\n"
   "  v=texture2D(texuv,tex_cuv).a;\n"
   "  u=u-0.5;\n"
   "  v=v-0.5;\n"
   "  vmu=v*0.813+u*0.391;\n"
   "  u=u*2.018;\n"
   "  v=v*1.596;\n"
   "  y=(y-0.062)*1.164;\n"
   "  r=y+v;\n"
   "  g=y-vmu;\n"
   "  b=y+u;\n"
   "  gl_FragColor=vec4(r,g,b,1.0) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_frag_src =
{
   nv12_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/nv12_nomul_frag.shd */
static const char const nv12_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex, texuv;\n"
   "varying vec2 tex_c, tex_cuv;\n"
   "void main()\n"
   "{\n"
   "  float y,u,v,vmu,r,g,b;\n"
   "  y=texture2D(tex,tex_c).g;\n"
   "  u=texture2D(texuv,tex_cuv).g;\n"
   "  v=texture2D(texuv,tex_cuv).a;\n"
   "  u=u-0.5;\n"
   "  v=v-0.5;\n"
   "  vmu=v*0.813+u*0.391;\n"
   "  u=u*2.018;\n"
   "  v=v*1.596;\n"
   "  y=(y-0.062)*1.164;\n"
   "  r=y+v;\n"
   "  g=y-vmu;\n"
   "  b=y+u;\n"
   "  gl_FragColor=vec4(r,g,b,1.0);\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_nomul_frag_src =
{
   nv12_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/nv12_nomul_vert.shd */
static const char const nv12_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord, tex_coord2;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c, tex_cuv;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_cuv = tex_coord2 * 0.5;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_nomul_vert_src =
{
   nv12_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/nv12_vert.shd */
static const char const nv12_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord, tex_coord2;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_cuv;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_cuv = tex_coord2 * 0.5;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_vert_src =
{
   nv12_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/rect_frag.shd */
static const char const rect_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = col;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_frag_src =
{
   rect_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/rect_vert.shd */
static const char const rect_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "uniform mat4 mvp;\n"
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

/* Source: modules/evas/engines/gl_common/shader/rgb_a_pair_frag.shd */
static const char const rgb_a_pair_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "uniform sampler2D texa;\n"
   "varying vec4 col;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_a;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor.rgb = texture2D(tex, coord_c.xy).rgb * col.rgb * texture2D(texa, coord_a).g;\n"
   "   gl_FragColor.a = col.a * texture2D(texa, coord_a).g;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_frag_src =
{
   rgb_a_pair_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/rgb_a_pair_nomul_frag.shd */
static const char const rgb_a_pair_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "uniform sampler2D texa;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_a;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor.rgb = texture2D(tex, coord_c.xy).rgb * texture2D(texa, coord_a).g;\n"
   "   gl_FragColor.a   = texture2D(texa, coord_a).g;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_nomul_frag_src =
{
   rgb_a_pair_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/rgb_a_pair_nomul_vert.shd */
static const char const rgb_a_pair_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_coorda;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_a;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   coord_c = tex_coord;\n"
   "   coord_a = tex_coorda;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_nomul_vert_src =
{
   rgb_a_pair_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/rgb_a_pair_vert.shd */
static const char const rgb_a_pair_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_coorda;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_a;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   coord_c = tex_coord;\n"
   "   coord_a = tex_coorda;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_vert_src =
{
   rgb_a_pair_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_12_frag.shd */
static const char const tex_12_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   gl_FragColor = ((col00 + col01) / div_s) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_12_frag_src =
{
   tex_12_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_12_vert.shd */
static const char const tex_12_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0,  tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_12_vert_src =
{
   tex_12_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_12_nomul_frag.shd */
static const char const tex_12_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   gl_FragColor = (col00 + col01) / div_s;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_12_nomul_frag_src =
{
   tex_12_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_12_nomul_vert.shd */
static const char const tex_12_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0,  tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_12_nomul_vert_src =
{
   tex_12_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_21_frag.shd */
static const char const tex_21_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   gl_FragColor = ((col00 + col01) / div_s) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_21_frag_src =
{
   tex_21_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_21_vert.shd */
static const char const tex_21_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_21_vert_src =
{
   tex_21_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_21_nomul_frag.shd */
static const char const tex_21_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   gl_FragColor = (col00 + col01) / div_s;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_21_nomul_frag_src =
{
   tex_21_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_21_nomul_vert.shd */
static const char const tex_21_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_21_nomul_vert_src =
{
   tex_21_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_22_frag.shd */
static const char const tex_22_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]);\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]);\n"
   "   gl_FragColor = ((col00 + col01 + col10 + col11) / div_s) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_22_frag_src =
{
   tex_22_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_22_vert.shd */
static const char const tex_22_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x,  tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x,  tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_22_vert_src =
{
   tex_22_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_22_nomul_frag.shd */
static const char const tex_22_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]);\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]);\n"
   "   gl_FragColor = (col00 + col01 + col10 + col11) / div_s;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_22_nomul_frag_src =
{
   tex_22_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_22_nomul_vert.shd */
static const char const tex_22_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x,  tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x,  tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_22_nomul_vert_src =
{
   tex_22_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_frag.shd */
static const char const tex_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(tex, tex_c.xy) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_frag_src =
{
   tex_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_vert.shd */
static const char const tex_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_vert_src =
{
   tex_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_nomul_frag.shd */
static const char const tex_nomul_frag_glsl[] =
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
   "   gl_FragColor = texture2D(tex, tex_c.xy);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_nomul_frag_src =
{
   tex_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_nomul_vert.shd */
static const char const tex_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_nomul_vert_src =
{
   tex_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_afill_frag.shd */
static const char const tex_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   vec4 c = texture2D(tex, tex_c.xy);\n"
   "   gl_FragColor = vec4(c.r, c.g, c.b, 1) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_afill_frag_src =
{
   tex_afill_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_afill_vert.shd */
static const char const tex_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_afill_vert_src =
{
   tex_afill_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_22_afill_frag.shd */
static const char const tex_22_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]);\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]);\n"
   "   vec4 c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor = vec4(c.r, c.g, c.b, 1) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_22_afill_frag_src =
{
   tex_22_afill_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_22_afill_vert.shd */
static const char const tex_22_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x,  tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x,  tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_22_afill_vert_src =
{
   tex_22_afill_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_21_afill_frag.shd */
static const char const tex_21_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 c = (col00 + col01) / div_s;\n"
   "   gl_FragColor = vec4(c.r, c.g, c.b, 1) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_21_afill_frag_src =
{
   tex_21_afill_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_21_afill_vert.shd */
static const char const tex_21_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_21_afill_vert_src =
{
   tex_21_afill_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_12_afill_frag.shd */
static const char const tex_12_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 c = (col00 + col01) / div_s;\n"
   "   gl_FragColor = vec4(c.r, c.g, c.b, 1) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_12_afill_frag_src =
{
   tex_12_afill_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_12_afill_vert.shd */
static const char const tex_12_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0,  tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_12_afill_vert_src =
{
   tex_12_afill_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_22_nomul_afill_frag.shd */
static const char const tex_22_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]);\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]);\n"
   "   vec4 c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   gl_FragColor = vec4(c.r, c.g, c.b, 1);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_22_nomul_afill_frag_src =
{
   tex_22_nomul_afill_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_22_nomul_afill_vert.shd */
static const char const tex_22_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[4];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x,  tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x,  tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_22_nomul_afill_vert_src =
{
   tex_22_nomul_afill_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_21_nomul_afill_frag.shd */
static const char const tex_21_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 c = (col00 + col01) / div_s;\n"
   "   gl_FragColor = vec4(c.r, c.g, c.b, 1);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_21_nomul_afill_frag_src =
{
   tex_21_nomul_afill_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_21_nomul_afill_vert.shd */
static const char const tex_21_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_21_nomul_afill_vert_src =
{
   tex_21_nomul_afill_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_12_nomul_afill_frag.shd */
static const char const tex_12_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]);\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]);\n"
   "   vec4 c = (col00 + col01) / div_s;\n"
   "   gl_FragColor = vec4(c.r, c.g, c.b, 1);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_12_nomul_afill_frag_src =
{
   tex_12_nomul_afill_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_12_nomul_afill_vert.shd */
static const char const tex_12_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_sample;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "varying vec2 tex_s[2];\n"
   "varying vec4 div_s;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0,  tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_12_nomul_afill_vert_src =
{
   tex_12_nomul_afill_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_nomul_afill_frag.shd */
static const char const tex_nomul_afill_frag_glsl[] =
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
   "   vec4 c = texture2D(tex, tex_c.xy);\n"
   "   gl_FragColor = vec4(c.r, c.g, c.b, 1);\n"
   "}\n";
Evas_GL_Program_Source shader_tex_nomul_afill_frag_src =
{
   tex_nomul_afill_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/tex_nomul_afill_vert.shd */
static const char const tex_nomul_afill_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_nomul_afill_vert_src =
{
   tex_nomul_afill_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuv_frag.shd */
static const char const yuv_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex, texu, texv;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_c2, tex_c3;\n"
   "void main()\n"
   "{\n"
   "   float r, g, b, y, u, v;\n"
   "   y = texture2D(tex, tex_c.xy).r;\n"
   "   u = texture2D(texu, tex_c2.xy).r;\n"
   "   v = texture2D(texv, tex_c3.xy).r;\n"
   "   y = (y - 0.0625) * 1.164;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   r = y + (1.402   * v);\n"
   "   g = y - (0.34414 * u) - (0.71414 * v);\n"
   "   b = y + (1.772   * u);\n"
   "   gl_FragColor = vec4(r, g, b, 1.0) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_frag_src =
{
   yuv_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuv_nomul_frag.shd */
static const char const yuv_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex, texu, texv;\n"
   "varying vec2 tex_c, tex_c2, tex_c3;\n"
   "void main()\n"
   "{\n"
   "   float r, g, b, y, u, v;\n"
   "   y = texture2D(tex, tex_c.xy).r;\n"
   "   u = texture2D(texu, tex_c2.xy).r;\n"
   "   v = texture2D(texv, tex_c3.xy).r;\n"
   "   y = (y - 0.0625) * 1.164;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   r = y + (1.402   * v);\n"
   "   g = y - (0.34414 * u) - (0.71414 * v);\n"
   "   b = y + (1.772   * u);\n"
   "   gl_FragColor = vec4(r, g, b, 1.0);\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_nomul_frag_src =
{
   yuv_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuv_nomul_vert.shd */
static const char const yuv_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord, tex_coord2, tex_coord3;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c, tex_c2, tex_c3;\n"
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

/* Source: modules/evas/engines/gl_common/shader/yuv_vert.shd */
static const char const yuv_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord, tex_coord2, tex_coord3;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_c2, tex_c3;\n"
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

/* Source: modules/evas/engines/gl_common/shader/yuy2_frag.shd */
static const char const yuy2_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex, texuv;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_cuv;\n"
   "void main()\n"
   "{\n"
   "  float y,u,v,vmu,r,g,b;\n"
   "  y=texture2D(tex,tex_c).r;\n"
   "  u=texture2D(texuv,tex_cuv).g;\n"
   "  v=texture2D(texuv,tex_cuv).a;\n"
   "  u=u-0.5;\n"
   "  v=v-0.5;\n"
   "  vmu=v*0.813+u*0.391;\n"
   "  u=u*2.018;\n"
   "  v=v*1.596;\n"
   "  r=y+v;\n"
   "  g=y-vmu;\n"
   "  b=y+u;\n"
   "  gl_FragColor=vec4(r,g,b,1.0) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_frag_src =
{
   yuy2_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuy2_nomul_frag.shd */
static const char const yuy2_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex, texuv;\n"
   "varying vec2 tex_c, tex_cuv;\n"
   "void main()\n"
   "{\n"
   "  float y,u,v,vmu,r,g,b;\n"
   "  y=texture2D(tex,tex_c).r;\n"
   "  u=texture2D(texuv,tex_cuv).g;\n"
   "  v=texture2D(texuv,tex_cuv).a;\n"
   "  u=u-0.5;\n"
   "  v=v-0.5;\n"
   "  vmu=v*0.813+u*0.391;\n"
   "  u=u*2.018;\n"
   "  v=v*1.596;\n"
   "  y=(y-0.062)*1.164;\n"
   "  r=y+v;\n"
   "  g=y-vmu;\n"
   "  b=y+u;\n"
   "  gl_FragColor=vec4(r,g,b,1.0);\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_nomul_frag_src =
{
   yuy2_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuy2_nomul_vert.shd */
static const char const yuy2_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord, tex_coord2;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 tex_c, tex_cuv;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_cuv = vec2(tex_coord2.x * 0.5, tex_coord2.y);\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_nomul_vert_src =
{
   yuy2_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuy2_vert.shd */
static const char const yuy2_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord, tex_coord2;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_cuv;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_cuv = vec2(tex_coord2.x * 0.5, tex_coord2.y);\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_vert_src =
{
   yuy2_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_mask_frag.shd */
static const char const img_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "uniform sampler2D texm;\n"
   "varying vec4 col;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(texm, coord_m.xy).a * texture2D(tex, coord_c.xy).bgra * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_frag_src =
{
   img_mask_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_mask_vert.shd */
static const char const img_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_coordm;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   coord_c = tex_coord;\n"
   "   coord_m = tex_coordm;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_vert_src =
{
   img_mask_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_mask_nomul_frag.shd */
static const char const img_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(tex, coord_c.xy) * texture2D(texm, coord_m).a;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_nomul_frag_src =
{
   img_mask_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_mask_nomul_vert.shd */
static const char const img_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_coordm;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   coord_c = tex_coord;\n"
   "   coord_m = tex_coordm;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_nomul_vert_src =
{
   img_mask_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_mask_bgra_frag.shd */
static const char const img_mask_bgra_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "uniform sampler2D texm;\n"
   "varying vec4 col;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(texm, coord_m.xy).a * texture2D(tex, coord_c.xy) * col;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_bgra_frag_src =
{
   img_mask_bgra_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_mask_bgra_vert.shd */
static const char const img_mask_bgra_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_coordm;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   coord_c = tex_coord;\n"
   "   coord_m = tex_coordm;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_bgra_vert_src =
{
   img_mask_bgra_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_mask_bgra_nomul_frag.shd */
static const char const img_mask_bgra_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor = texture2D(texm, coord_m.xy).a * texture2D(tex, coord_c.xy);\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_bgra_nomul_frag_src =
{
   img_mask_bgra_nomul_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/img_mask_bgra_nomul_vert.shd */
static const char const img_mask_bgra_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_coordm;\n"
   "uniform mat4 mvp;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   coord_c = tex_coord;\n"
   "   coord_m = tex_coordm;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_bgra_nomul_vert_src =
{
   img_mask_bgra_nomul_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuv_mask_frag.shd */
static const char const yuv_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex, texu, texv, texm;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_c2, tex_c3, tex_m;\n"
   "void main()\n"
   "{\n"
   "   float r, g, b, y, u, v;\n"
   "   y = texture2D(tex, tex_c.xy).r;\n"
   "   u = texture2D(texu, tex_c2.xy).r;\n"
   "   v = texture2D(texv, tex_c3.xy).r;\n"
   "   y = (y - 0.0625) * 1.164;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   r = y + (1.402   * v);\n"
   "   g = y - (0.34414 * u) - (0.71414 * v);\n"
   "   b = y + (1.772   * u);\n"
   "   gl_FragColor = vec4(r, g, b, 1.0) * texture2D(texm, tex_m.xy).a * col;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_mask_frag_src =
{
   yuv_mask_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuv_mask_vert.shd */
static const char const yuv_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord, tex_coord2, tex_coord3, tex_coordm;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_c2, tex_c3, tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_c2 = tex_coord2;\n"
   "   tex_c3 = tex_coord3;\n"
   "   tex_m = tex_coordm;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_mask_vert_src =
{
   yuv_mask_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/nv12_mask_frag.shd */
static const char const nv12_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex, texuv, texm;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_cuv, tex_m;\n"
   "void main()\n"
   "{\n"
   "  float y,u,v,vmu,r,g,b;\n"
   "  y=texture2D(tex,tex_c).g;\n"
   "  u=texture2D(texuv,tex_cuv).g;\n"
   "  v=texture2D(texuv,tex_cuv).a;\n"
   "  u=u-0.5;\n"
   "  v=v-0.5;\n"
   "  vmu=v*0.813+u*0.391;\n"
   "  u=u*2.018;\n"
   "  v=v*1.596;\n"
   "  y=(y-0.062)*1.164;\n"
   "  r=y+v;\n"
   "  g=y-vmu;\n"
   "  b=y+u;\n"
   "  gl_FragColor = vec4(r,g,b,1.0) * texture2D(tex, tex_m.xy).a * col;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_mask_frag_src =
{
   nv12_mask_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/nv12_mask_vert.shd */
static const char const nv12_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord, tex_coord2, tex_coordm;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_cuv, tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_cuv = tex_coord2 * 0.5;\n"
   "   tex_m = tex_coordm;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_mask_vert_src =
{
   nv12_mask_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuy2_mask_frag.shd */
static const char const yuy2_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex, texuv, texm;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_cuv, tex_m;\n"
   "void main()\n"
   "{\n"
   "  float y,u,v,vmu,r,g,b;\n"
   "  y=texture2D(tex,tex_c).r;\n"
   "  u=texture2D(texuv,tex_cuv).g;\n"
   "  v=texture2D(texuv,tex_cuv).a;\n"
   "  u=u-0.5;\n"
   "  v=v-0.5;\n"
   "  vmu=v*0.813+u*0.391;\n"
   "  u=u*2.018;\n"
   "  v=v*1.596;\n"
   "  r=y+v;\n"
   "  g=y-vmu;\n"
   "  b=y+u;\n"
   "  gl_FragColor = vec4(r,g,b,1.0) * texture2D(texm, tex_m.xy).a * col;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_mask_frag_src =
{
   yuy2_mask_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/yuy2_mask_vert.shd */
static const char const yuy2_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord, tex_coord2, tex_coordm;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 tex_c, tex_cuv, tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_cuv = vec2(tex_coord2.x * 0.5, tex_coord2.y);\n"
   "   tex_m = tex_coordm;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_mask_vert_src =
{
   yuy2_mask_vert_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/rgb_a_pair_mask_frag.shd */
static const char const rgb_a_pair_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "uniform sampler2D texa;\n"
   "uniform sampler2D texm;\n"
   "varying vec4 col;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_a;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_FragColor.rgb = texture2D(tex, coord_c.xy).rgb * col.rgb * texture2D(texa, coord_a).g * texture2D(texm, coord_m.xy).a;\n"
   "   gl_FragColor.a = col.a * texture2D(texa, coord_a).g * texture2D(texm, coord_m.xy).a;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_frag_src =
{
   rgb_a_pair_mask_frag_glsl,
   NULL, 0
};

/* Source: modules/evas/engines/gl_common/shader/rgb_a_pair_mask_vert.shd */
static const char const rgb_a_pair_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "attribute vec4 color;\n"
   "attribute vec2 tex_coord;\n"
   "attribute vec2 tex_coorda;\n"
   "attribute vec2 tex_coordm;\n"
   "uniform mat4 mvp;\n"
   "varying vec4 col;\n"
   "varying vec2 coord_c;\n"
   "varying vec2 coord_a;\n"
   "varying vec2 coord_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   coord_c = tex_coord;\n"
   "   coord_a = tex_coorda;\n"
   "   coord_m = tex_coordm;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_vert_src =
{
   rgb_a_pair_mask_vert_glsl,
   NULL, 0
};

static const struct {
   Evas_GL_Shader id;
   Evas_GL_Program_Source *vert;
   Evas_GL_Program_Source *frag;
   const char *name;
} _shaders_source[] = {
   { SHADER_FONT, &(shader_font_vert_src), &(shader_font_frag_src), "font" },
   { SHADER_FONT_MASK, &(shader_font_mask_vert_src), &(shader_font_mask_frag_src), "font_mask" },
   { SHADER_IMG_12_BGRA_NOMUL, &(shader_img_12_bgra_nomul_vert_src), &(shader_img_12_bgra_nomul_frag_src), "img_12_bgra_nomul" },
   { SHADER_IMG_12_BGRA, &(shader_img_12_bgra_vert_src), &(shader_img_12_bgra_frag_src), "img_12_bgra" },
   { SHADER_IMG_12_NOMUL, &(shader_img_12_nomul_vert_src), &(shader_img_12_nomul_frag_src), "img_12_nomul" },
   { SHADER_IMG_12, &(shader_img_12_vert_src), &(shader_img_12_frag_src), "img_12" },
   { SHADER_IMG_21_BGRA_NOMUL, &(shader_img_21_bgra_nomul_vert_src), &(shader_img_21_bgra_nomul_frag_src), "img_21_bgra_nomul" },
   { SHADER_IMG_21_BGRA, &(shader_img_21_bgra_vert_src), &(shader_img_21_bgra_frag_src), "img_21_bgra" },
   { SHADER_IMG_21_NOMUL, &(shader_img_21_nomul_vert_src), &(shader_img_21_nomul_frag_src), "img_21_nomul" },
   { SHADER_IMG_21, &(shader_img_21_vert_src), &(shader_img_21_frag_src), "img_21" },
   { SHADER_IMG_22_BGRA_NOMUL, &(shader_img_22_bgra_nomul_vert_src), &(shader_img_22_bgra_nomul_frag_src), "img_22_bgra_nomul" },
   { SHADER_IMG_22_BGRA, &(shader_img_22_bgra_vert_src), &(shader_img_22_bgra_frag_src), "img_22_bgra" },
   { SHADER_IMG_22_NOMUL, &(shader_img_22_nomul_vert_src), &(shader_img_22_nomul_frag_src), "img_22_nomul" },
   { SHADER_IMG_22, &(shader_img_22_vert_src), &(shader_img_22_frag_src), "img_22" },
   { SHADER_IMG_BGRA_NOMUL, &(shader_img_bgra_nomul_vert_src), &(shader_img_bgra_nomul_frag_src), "img_bgra_nomul" },
   { SHADER_IMG_BGRA, &(shader_img_bgra_vert_src), &(shader_img_bgra_frag_src), "img_bgra" },
   { SHADER_IMG_NOMUL, &(shader_img_nomul_vert_src), &(shader_img_nomul_frag_src), "img_nomul" },
   { SHADER_IMG, &(shader_img_vert_src), &(shader_img_frag_src), "img" },
   { SHADER_NV12_NOMUL, &(shader_nv12_nomul_vert_src), &(shader_nv12_nomul_frag_src), "nv12_nomul" },
   { SHADER_NV12, &(shader_nv12_vert_src), &(shader_nv12_frag_src), "nv12" },
   { SHADER_RECT, &(shader_rect_vert_src), &(shader_rect_frag_src), "rect" },
   { SHADER_RGB_A_PAIR_NOMUL, &(shader_rgb_a_pair_nomul_vert_src), &(shader_rgb_a_pair_nomul_frag_src), "rgb_a_pair_nomul" },
   { SHADER_RGB_A_PAIR, &(shader_rgb_a_pair_vert_src), &(shader_rgb_a_pair_frag_src), "rgb_a_pair" },
   { SHADER_TEX_12, &(shader_tex_12_vert_src), &(shader_tex_12_frag_src), "tex_12" },
   { SHADER_TEX_12_NOMUL, &(shader_tex_12_nomul_vert_src), &(shader_tex_12_nomul_frag_src), "tex_12_nomul" },
   { SHADER_TEX_21, &(shader_tex_21_vert_src), &(shader_tex_21_frag_src), "tex_21" },
   { SHADER_TEX_21_NOMUL, &(shader_tex_21_nomul_vert_src), &(shader_tex_21_nomul_frag_src), "tex_21_nomul" },
   { SHADER_TEX_22, &(shader_tex_22_vert_src), &(shader_tex_22_frag_src), "tex_22" },
   { SHADER_TEX_22_NOMUL, &(shader_tex_22_nomul_vert_src), &(shader_tex_22_nomul_frag_src), "tex_22_nomul" },
   { SHADER_TEX, &(shader_tex_vert_src), &(shader_tex_frag_src), "tex" },
   { SHADER_TEX_NOMUL, &(shader_tex_nomul_vert_src), &(shader_tex_nomul_frag_src), "tex_nomul" },
   { SHADER_TEX_AFILL, &(shader_tex_afill_vert_src), &(shader_tex_afill_frag_src), "tex_afill" },
   { SHADER_TEX_22_AFILL, &(shader_tex_22_afill_vert_src), &(shader_tex_22_afill_frag_src), "tex_22_afill" },
   { SHADER_TEX_21_AFILL, &(shader_tex_21_afill_vert_src), &(shader_tex_21_afill_frag_src), "tex_21_afill" },
   { SHADER_TEX_12_AFILL, &(shader_tex_12_afill_vert_src), &(shader_tex_12_afill_frag_src), "tex_12_afill" },
   { SHADER_TEX_22_NOMUL_AFILL, &(shader_tex_22_nomul_afill_vert_src), &(shader_tex_22_nomul_afill_frag_src), "tex_22_nomul_afill" },
   { SHADER_TEX_21_NOMUL_AFILL, &(shader_tex_21_nomul_afill_vert_src), &(shader_tex_21_nomul_afill_frag_src), "tex_21_nomul_afill" },
   { SHADER_TEX_12_NOMUL_AFILL, &(shader_tex_12_nomul_afill_vert_src), &(shader_tex_12_nomul_afill_frag_src), "tex_12_nomul_afill" },
   { SHADER_TEX_NOMUL_AFILL, &(shader_tex_nomul_afill_vert_src), &(shader_tex_nomul_afill_frag_src), "tex_nomul_afill" },
   { SHADER_YUV_NOMUL, &(shader_yuv_nomul_vert_src), &(shader_yuv_nomul_frag_src), "yuv_nomul" },
   { SHADER_YUV, &(shader_yuv_vert_src), &(shader_yuv_frag_src), "yuv" },
   { SHADER_YUY2_NOMUL, &(shader_yuy2_nomul_vert_src), &(shader_yuy2_nomul_frag_src), "yuy2_nomul" },
   { SHADER_YUY2, &(shader_yuy2_vert_src), &(shader_yuy2_frag_src), "yuy2" },
   { SHADER_IMG_MASK, &(shader_img_mask_vert_src), &(shader_img_mask_frag_src), "img_mask" },
   { SHADER_IMG_MASK_NOMUL, &(shader_img_mask_nomul_vert_src), &(shader_img_mask_nomul_frag_src), "img_mask_nomul" },
   { SHADER_IMG_MASK_BGRA, &(shader_img_mask_bgra_vert_src), &(shader_img_mask_bgra_frag_src), "img_mask_bgra" },
   { SHADER_IMG_MASK_BGRA_NOMUL, &(shader_img_mask_bgra_nomul_vert_src), &(shader_img_mask_bgra_nomul_frag_src), "img_mask_bgra_nomul" },
   { SHADER_YUV_MASK, &(shader_yuv_mask_vert_src), &(shader_yuv_mask_frag_src), "yuv_mask" },
   { SHADER_NV12_MASK, &(shader_nv12_mask_vert_src), &(shader_nv12_mask_frag_src), "nv12_mask" },
   { SHADER_YUY2_MASK, &(shader_yuy2_mask_vert_src), &(shader_yuy2_mask_frag_src), "yuy2_mask" },
   { SHADER_RGB_A_PAIR_MASK, &(shader_rgb_a_pair_mask_vert_src), &(shader_rgb_a_pair_mask_frag_src), "rgb_a_pair_mask" },
};

