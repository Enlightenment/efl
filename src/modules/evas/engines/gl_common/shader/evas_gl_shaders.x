/* DO NOT MODIFY THIS FILE AS IT IS AUTO-GENERATED */
/* IF IT IS CHANGED PLEASE COMMIT THE CHANGES */

#include "../evas_gl_private.h"

static const char rect_frag_glsl[] =
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
   rect_frag_glsl
};

static const char rect_vert_glsl[] =
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
   rect_vert_glsl
};

static const char rect_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_mask_frag_src =
{
   rect_mask_frag_glsl
};

static const char rect_mask_vert_glsl[] =
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
   rect_mask_vert_glsl
};

static const char rect_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = vec4(1, 1, 1, 1);\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_mask12_frag_src =
{
   rect_mask12_frag_glsl
};

static const char rect_mask12_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_mask12_vert_src =
{
   rect_mask12_vert_glsl
};

static const char rect_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = vec4(1, 1, 1, 1);\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_mask21_frag_src =
{
   rect_mask21_frag_glsl
};

static const char rect_mask21_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_mask21_vert_src =
{
   rect_mask21_vert_glsl
};

static const char rect_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = vec4(1, 1, 1, 1);\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_mask22_frag_src =
{
   rect_mask22_frag_glsl
};

static const char rect_mask22_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_rect_mask22_vert_src =
{
   rect_mask22_vert_glsl
};

static const char font_frag_glsl[] =
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
   font_frag_glsl
};

static const char font_vert_glsl[] =
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
   font_vert_glsl
};

static const char font_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask_frag_src =
{
   font_mask_frag_glsl
};

static const char font_mask_vert_glsl[] =
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
   font_mask_vert_glsl
};

static const char font_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).aaaa;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask12_frag_src =
{
   font_mask12_frag_glsl
};

static const char font_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask12_vert_src =
{
   font_mask12_vert_glsl
};

static const char font_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).aaaa;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask21_frag_src =
{
   font_mask21_frag_glsl
};

static const char font_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask21_vert_src =
{
   font_mask21_vert_glsl
};

static const char font_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).aaaa;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask22_frag_src =
{
   font_mask22_frag_glsl
};

static const char font_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_font_mask22_vert_src =
{
   font_mask22_vert_glsl
};

static const char img_frag_glsl[] =
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
   img_frag_glsl
};

static const char img_vert_glsl[] =
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
   img_vert_glsl
};

static const char img_bgra_frag_glsl[] =
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
   img_bgra_frag_glsl
};

static const char img_bgra_vert_glsl[] =
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
   img_bgra_vert_glsl
};

static const char img_12_frag_glsl[] =
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
   img_12_frag_glsl
};

static const char img_12_vert_glsl[] =
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
   img_12_vert_glsl
};

static const char img_21_frag_glsl[] =
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
   img_21_frag_glsl
};

static const char img_21_vert_glsl[] =
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
   img_21_vert_glsl
};

static const char img_22_frag_glsl[] =
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
   img_22_frag_glsl
};

static const char img_22_vert_glsl[] =
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
   img_22_vert_glsl
};

static const char img_12_bgra_frag_glsl[] =
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
   img_12_bgra_frag_glsl
};

static const char img_12_bgra_vert_glsl[] =
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
   img_12_bgra_vert_glsl
};

static const char img_21_bgra_frag_glsl[] =
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
   img_21_bgra_frag_glsl
};

static const char img_21_bgra_vert_glsl[] =
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
   img_21_bgra_vert_glsl
};

static const char img_22_bgra_frag_glsl[] =
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
   img_22_bgra_frag_glsl
};

static const char img_22_bgra_vert_glsl[] =
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
   img_22_bgra_vert_glsl
};

static const char img_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_frag_src =
{
   img_mask_frag_glsl
};

static const char img_mask_vert_glsl[] =
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
   img_mask_vert_glsl
};

static const char img_bgra_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask_frag_src =
{
   img_bgra_mask_frag_glsl
};

static const char img_bgra_mask_vert_glsl[] =
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
   img_bgra_mask_vert_glsl
};

static const char img_12_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask_frag_src =
{
   img_12_mask_frag_glsl
};

static const char img_12_mask_vert_glsl[] =
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
   img_12_mask_vert_glsl
};

static const char img_21_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask_frag_src =
{
   img_21_mask_frag_glsl
};

static const char img_21_mask_vert_glsl[] =
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
   img_21_mask_vert_glsl
};

static const char img_22_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask_frag_src =
{
   img_22_mask_frag_glsl
};

static const char img_22_mask_vert_glsl[] =
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
   img_22_mask_vert_glsl
};

static const char img_12_bgra_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask_frag_src =
{
   img_12_bgra_mask_frag_glsl
};

static const char img_12_bgra_mask_vert_glsl[] =
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
   img_12_bgra_mask_vert_glsl
};

static const char img_21_bgra_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask_frag_src =
{
   img_21_bgra_mask_frag_glsl
};

static const char img_21_bgra_mask_vert_glsl[] =
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
   img_21_bgra_mask_vert_glsl
};

static const char img_22_bgra_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask_frag_src =
{
   img_22_bgra_mask_frag_glsl
};

static const char img_22_bgra_mask_vert_glsl[] =
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
   img_22_bgra_mask_vert_glsl
};

static const char img_nomul_frag_glsl[] =
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
   img_nomul_frag_glsl
};

static const char img_nomul_vert_glsl[] =
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
   img_nomul_vert_glsl
};

static const char img_bgra_nomul_frag_glsl[] =
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
   img_bgra_nomul_frag_glsl
};

static const char img_bgra_nomul_vert_glsl[] =
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
   img_bgra_nomul_vert_glsl
};

static const char img_12_nomul_frag_glsl[] =
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
   img_12_nomul_frag_glsl
};

static const char img_12_nomul_vert_glsl[] =
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
   img_12_nomul_vert_glsl
};

static const char img_21_nomul_frag_glsl[] =
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
   img_21_nomul_frag_glsl
};

static const char img_21_nomul_vert_glsl[] =
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
   img_21_nomul_vert_glsl
};

static const char img_22_nomul_frag_glsl[] =
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
   img_22_nomul_frag_glsl
};

static const char img_22_nomul_vert_glsl[] =
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
   img_22_nomul_vert_glsl
};

static const char img_12_bgra_nomul_frag_glsl[] =
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
   img_12_bgra_nomul_frag_glsl
};

static const char img_12_bgra_nomul_vert_glsl[] =
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
   img_12_bgra_nomul_vert_glsl
};

static const char img_21_bgra_nomul_frag_glsl[] =
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
   img_21_bgra_nomul_frag_glsl
};

static const char img_21_bgra_nomul_vert_glsl[] =
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
   img_21_bgra_nomul_vert_glsl
};

static const char img_22_bgra_nomul_frag_glsl[] =
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
   img_22_bgra_nomul_frag_glsl
};

static const char img_22_bgra_nomul_vert_glsl[] =
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
   img_22_bgra_nomul_vert_glsl
};

static const char img_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask_nomul_frag_src =
{
   img_mask_nomul_frag_glsl
};

static const char img_mask_nomul_vert_glsl[] =
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
   img_mask_nomul_vert_glsl
};

static const char img_bgra_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask_nomul_frag_src =
{
   img_bgra_mask_nomul_frag_glsl
};

static const char img_bgra_mask_nomul_vert_glsl[] =
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
   img_bgra_mask_nomul_vert_glsl
};

static const char img_12_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask_nomul_frag_src =
{
   img_12_mask_nomul_frag_glsl
};

static const char img_12_mask_nomul_vert_glsl[] =
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
   img_12_mask_nomul_vert_glsl
};

static const char img_21_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask_nomul_frag_src =
{
   img_21_mask_nomul_frag_glsl
};

static const char img_21_mask_nomul_vert_glsl[] =
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
   img_21_mask_nomul_vert_glsl
};

static const char img_22_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask_nomul_frag_src =
{
   img_22_mask_nomul_frag_glsl
};

static const char img_22_mask_nomul_vert_glsl[] =
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
   img_22_mask_nomul_vert_glsl
};

static const char img_12_bgra_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask_nomul_frag_src =
{
   img_12_bgra_mask_nomul_frag_glsl
};

static const char img_12_bgra_mask_nomul_vert_glsl[] =
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
   img_12_bgra_mask_nomul_vert_glsl
};

static const char img_21_bgra_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask_nomul_frag_src =
{
   img_21_bgra_mask_nomul_frag_glsl
};

static const char img_21_bgra_mask_nomul_vert_glsl[] =
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
   img_21_bgra_mask_nomul_vert_glsl
};

static const char img_22_bgra_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask_nomul_frag_src =
{
   img_22_bgra_mask_nomul_frag_glsl
};

static const char img_22_bgra_mask_nomul_vert_glsl[] =
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
   img_22_bgra_mask_nomul_vert_glsl
};

static const char img_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask12_frag_src =
{
   img_mask12_frag_glsl
};

static const char img_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask12_vert_src =
{
   img_mask12_vert_glsl
};

static const char img_bgra_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask12_frag_src =
{
   img_bgra_mask12_frag_glsl
};

static const char img_bgra_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask12_vert_src =
{
   img_bgra_mask12_vert_glsl
};

static const char img_12_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask12_frag_src =
{
   img_12_mask12_frag_glsl
};

static const char img_12_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask12_vert_src =
{
   img_12_mask12_vert_glsl
};

static const char img_21_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask12_frag_src =
{
   img_21_mask12_frag_glsl
};

static const char img_21_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask12_vert_src =
{
   img_21_mask12_vert_glsl
};

static const char img_22_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask12_frag_src =
{
   img_22_mask12_frag_glsl
};

static const char img_22_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
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
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask12_vert_src =
{
   img_22_mask12_vert_glsl
};

static const char img_12_bgra_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask12_frag_src =
{
   img_12_bgra_mask12_frag_glsl
};

static const char img_12_bgra_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask12_vert_src =
{
   img_12_bgra_mask12_vert_glsl
};

static const char img_21_bgra_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask12_frag_src =
{
   img_21_bgra_mask12_frag_glsl
};

static const char img_21_bgra_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask12_vert_src =
{
   img_21_bgra_mask12_vert_glsl
};

static const char img_22_bgra_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask12_frag_src =
{
   img_22_bgra_mask12_frag_glsl
};

static const char img_22_bgra_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
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
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask12_vert_src =
{
   img_22_bgra_mask12_vert_glsl
};

static const char img_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask12_nomul_frag_src =
{
   img_mask12_nomul_frag_glsl
};

static const char img_mask12_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask12_nomul_vert_src =
{
   img_mask12_nomul_vert_glsl
};

static const char img_bgra_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask12_nomul_frag_src =
{
   img_bgra_mask12_nomul_frag_glsl
};

static const char img_bgra_mask12_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask12_nomul_vert_src =
{
   img_bgra_mask12_nomul_vert_glsl
};

static const char img_12_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask12_nomul_frag_src =
{
   img_12_mask12_nomul_frag_glsl
};

static const char img_12_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask12_nomul_vert_src =
{
   img_12_mask12_nomul_vert_glsl
};

static const char img_21_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask12_nomul_frag_src =
{
   img_21_mask12_nomul_frag_glsl
};

static const char img_21_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask12_nomul_vert_src =
{
   img_21_mask12_nomul_vert_glsl
};

static const char img_22_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask12_nomul_frag_src =
{
   img_22_mask12_nomul_frag_glsl
};

static const char img_22_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask12_nomul_vert_src =
{
   img_22_mask12_nomul_vert_glsl
};

static const char img_12_bgra_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask12_nomul_frag_src =
{
   img_12_bgra_mask12_nomul_frag_glsl
};

static const char img_12_bgra_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask12_nomul_vert_src =
{
   img_12_bgra_mask12_nomul_vert_glsl
};

static const char img_21_bgra_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask12_nomul_frag_src =
{
   img_21_bgra_mask12_nomul_frag_glsl
};

static const char img_21_bgra_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask12_nomul_vert_src =
{
   img_21_bgra_mask12_nomul_vert_glsl
};

static const char img_22_bgra_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask12_nomul_frag_src =
{
   img_22_bgra_mask12_nomul_frag_glsl
};

static const char img_22_bgra_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask12_nomul_vert_src =
{
   img_22_bgra_mask12_nomul_vert_glsl
};

static const char img_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask21_frag_src =
{
   img_mask21_frag_glsl
};

static const char img_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask21_vert_src =
{
   img_mask21_vert_glsl
};

static const char img_bgra_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask21_frag_src =
{
   img_bgra_mask21_frag_glsl
};

static const char img_bgra_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask21_vert_src =
{
   img_bgra_mask21_vert_glsl
};

static const char img_12_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask21_frag_src =
{
   img_12_mask21_frag_glsl
};

static const char img_12_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask21_vert_src =
{
   img_12_mask21_vert_glsl
};

static const char img_21_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask21_frag_src =
{
   img_21_mask21_frag_glsl
};

static const char img_21_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask21_vert_src =
{
   img_21_mask21_vert_glsl
};

static const char img_22_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask21_frag_src =
{
   img_22_mask21_frag_glsl
};

static const char img_22_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
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
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask21_vert_src =
{
   img_22_mask21_vert_glsl
};

static const char img_12_bgra_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask21_frag_src =
{
   img_12_bgra_mask21_frag_glsl
};

static const char img_12_bgra_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask21_vert_src =
{
   img_12_bgra_mask21_vert_glsl
};

static const char img_21_bgra_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask21_frag_src =
{
   img_21_bgra_mask21_frag_glsl
};

static const char img_21_bgra_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask21_vert_src =
{
   img_21_bgra_mask21_vert_glsl
};

static const char img_22_bgra_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask21_frag_src =
{
   img_22_bgra_mask21_frag_glsl
};

static const char img_22_bgra_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
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
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask21_vert_src =
{
   img_22_bgra_mask21_vert_glsl
};

static const char img_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask21_nomul_frag_src =
{
   img_mask21_nomul_frag_glsl
};

static const char img_mask21_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask21_nomul_vert_src =
{
   img_mask21_nomul_vert_glsl
};

static const char img_bgra_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask21_nomul_frag_src =
{
   img_bgra_mask21_nomul_frag_glsl
};

static const char img_bgra_mask21_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask21_nomul_vert_src =
{
   img_bgra_mask21_nomul_vert_glsl
};

static const char img_12_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask21_nomul_frag_src =
{
   img_12_mask21_nomul_frag_glsl
};

static const char img_12_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask21_nomul_vert_src =
{
   img_12_mask21_nomul_vert_glsl
};

static const char img_21_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask21_nomul_frag_src =
{
   img_21_mask21_nomul_frag_glsl
};

static const char img_21_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask21_nomul_vert_src =
{
   img_21_mask21_nomul_vert_glsl
};

static const char img_22_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask21_nomul_frag_src =
{
   img_22_mask21_nomul_frag_glsl
};

static const char img_22_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask21_nomul_vert_src =
{
   img_22_mask21_nomul_vert_glsl
};

static const char img_12_bgra_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask21_nomul_frag_src =
{
   img_12_bgra_mask21_nomul_frag_glsl
};

static const char img_12_bgra_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask21_nomul_vert_src =
{
   img_12_bgra_mask21_nomul_vert_glsl
};

static const char img_21_bgra_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask21_nomul_frag_src =
{
   img_21_bgra_mask21_nomul_frag_glsl
};

static const char img_21_bgra_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask21_nomul_vert_src =
{
   img_21_bgra_mask21_nomul_vert_glsl
};

static const char img_22_bgra_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask21_nomul_frag_src =
{
   img_22_bgra_mask21_nomul_frag_glsl
};

static const char img_22_bgra_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask21_nomul_vert_src =
{
   img_22_bgra_mask21_nomul_vert_glsl
};

static const char img_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask22_frag_src =
{
   img_mask22_frag_glsl
};

static const char img_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask22_vert_src =
{
   img_mask22_vert_glsl
};

static const char img_bgra_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask22_frag_src =
{
   img_bgra_mask22_frag_glsl
};

static const char img_bgra_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask22_vert_src =
{
   img_bgra_mask22_vert_glsl
};

static const char img_12_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask22_frag_src =
{
   img_12_mask22_frag_glsl
};

static const char img_12_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask22_vert_src =
{
   img_12_mask22_vert_glsl
};

static const char img_21_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask22_frag_src =
{
   img_21_mask22_frag_glsl
};

static const char img_21_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask22_vert_src =
{
   img_21_mask22_vert_glsl
};

static const char img_22_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask22_frag_src =
{
   img_22_mask22_frag_glsl
};

static const char img_22_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
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
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask22_vert_src =
{
   img_22_mask22_vert_glsl
};

static const char img_12_bgra_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask22_frag_src =
{
   img_12_bgra_mask22_frag_glsl
};

static const char img_12_bgra_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask22_vert_src =
{
   img_12_bgra_mask22_vert_glsl
};

static const char img_21_bgra_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask22_frag_src =
{
   img_21_bgra_mask22_frag_glsl
};

static const char img_21_bgra_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask22_vert_src =
{
   img_21_bgra_mask22_vert_glsl
};

static const char img_22_bgra_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask22_frag_src =
{
   img_22_bgra_mask22_frag_glsl
};

static const char img_22_bgra_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
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
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask22_vert_src =
{
   img_22_bgra_mask22_vert_glsl
};

static const char img_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask22_nomul_frag_src =
{
   img_mask22_nomul_frag_glsl
};

static const char img_mask22_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_mask22_nomul_vert_src =
{
   img_mask22_nomul_vert_glsl
};

static const char img_bgra_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask22_nomul_frag_src =
{
   img_bgra_mask22_nomul_frag_glsl
};

static const char img_bgra_mask22_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_bgra_mask22_nomul_vert_src =
{
   img_bgra_mask22_nomul_vert_glsl
};

static const char img_12_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask22_nomul_frag_src =
{
   img_12_mask22_nomul_frag_glsl
};

static const char img_12_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_mask22_nomul_vert_src =
{
   img_12_mask22_nomul_vert_glsl
};

static const char img_21_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask22_nomul_frag_src =
{
   img_21_mask22_nomul_frag_glsl
};

static const char img_21_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_mask22_nomul_vert_src =
{
   img_21_mask22_nomul_vert_glsl
};

static const char img_22_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask22_nomul_frag_src =
{
   img_22_mask22_nomul_frag_glsl
};

static const char img_22_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_mask22_nomul_vert_src =
{
   img_22_mask22_nomul_vert_glsl
};

static const char img_12_bgra_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask22_nomul_frag_src =
{
   img_12_bgra_mask22_nomul_frag_glsl
};

static const char img_12_bgra_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_12_bgra_mask22_nomul_vert_src =
{
   img_12_bgra_mask22_nomul_vert_glsl
};

static const char img_21_bgra_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask22_nomul_frag_src =
{
   img_21_bgra_mask22_nomul_frag_glsl
};

static const char img_21_bgra_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_21_bgra_mask22_nomul_vert_src =
{
   img_21_bgra_mask22_nomul_vert_glsl
};

static const char img_22_bgra_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask22_nomul_frag_src =
{
   img_22_bgra_mask22_nomul_frag_glsl
};

static const char img_22_bgra_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_img_22_bgra_mask22_nomul_vert_src =
{
   img_22_bgra_mask22_nomul_vert_glsl
};

static const char img_afill_frag_glsl[] =
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
   img_afill_frag_glsl
};

static const char img_afill_vert_glsl[] =
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
   img_afill_vert_glsl
};

static const char img_bgra_afill_frag_glsl[] =
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
   img_bgra_afill_frag_glsl
};

static const char img_bgra_afill_vert_glsl[] =
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
   img_bgra_afill_vert_glsl
};

static const char img_nomul_afill_frag_glsl[] =
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
   img_nomul_afill_frag_glsl
};

static const char img_nomul_afill_vert_glsl[] =
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
   img_nomul_afill_vert_glsl
};

static const char img_bgra_nomul_afill_frag_glsl[] =
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
   img_bgra_nomul_afill_frag_glsl
};

static const char img_bgra_nomul_afill_vert_glsl[] =
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
   img_bgra_nomul_afill_vert_glsl
};

static const char img_12_afill_frag_glsl[] =
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
   img_12_afill_frag_glsl
};

static const char img_12_afill_vert_glsl[] =
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
   img_12_afill_vert_glsl
};

static const char img_21_afill_frag_glsl[] =
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
   img_21_afill_frag_glsl
};

static const char img_21_afill_vert_glsl[] =
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
   img_21_afill_vert_glsl
};

static const char img_22_afill_frag_glsl[] =
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
   img_22_afill_frag_glsl
};

static const char img_22_afill_vert_glsl[] =
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
   img_22_afill_vert_glsl
};

static const char img_12_bgra_afill_frag_glsl[] =
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
   img_12_bgra_afill_frag_glsl
};

static const char img_12_bgra_afill_vert_glsl[] =
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
   img_12_bgra_afill_vert_glsl
};

static const char img_21_bgra_afill_frag_glsl[] =
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
   img_21_bgra_afill_frag_glsl
};

static const char img_21_bgra_afill_vert_glsl[] =
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
   img_21_bgra_afill_vert_glsl
};

static const char img_22_bgra_afill_frag_glsl[] =
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
   img_22_bgra_afill_frag_glsl
};

static const char img_22_bgra_afill_vert_glsl[] =
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
   img_22_bgra_afill_vert_glsl
};

static const char img_12_nomul_afill_frag_glsl[] =
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
   img_12_nomul_afill_frag_glsl
};

static const char img_12_nomul_afill_vert_glsl[] =
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
   img_12_nomul_afill_vert_glsl
};

static const char img_21_nomul_afill_frag_glsl[] =
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
   img_21_nomul_afill_frag_glsl
};

static const char img_21_nomul_afill_vert_glsl[] =
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
   img_21_nomul_afill_vert_glsl
};

static const char img_22_nomul_afill_frag_glsl[] =
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
   img_22_nomul_afill_frag_glsl
};

static const char img_22_nomul_afill_vert_glsl[] =
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
   img_22_nomul_afill_vert_glsl
};

static const char img_12_bgra_nomul_afill_frag_glsl[] =
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
   img_12_bgra_nomul_afill_frag_glsl
};

static const char img_12_bgra_nomul_afill_vert_glsl[] =
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
   img_12_bgra_nomul_afill_vert_glsl
};

static const char img_21_bgra_nomul_afill_frag_glsl[] =
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
   img_21_bgra_nomul_afill_frag_glsl
};

static const char img_21_bgra_nomul_afill_vert_glsl[] =
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
   img_21_bgra_nomul_afill_vert_glsl
};

static const char img_22_bgra_nomul_afill_frag_glsl[] =
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
   img_22_bgra_nomul_afill_frag_glsl
};

static const char img_22_bgra_nomul_afill_vert_glsl[] =
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
   img_22_bgra_nomul_afill_vert_glsl
};

static const char imgnat_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_frag_src =
{
   imgnat_frag_glsl
};

static const char imgnat_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_vert_src =
{
   imgnat_vert_glsl
};

static const char imgnat_bgra_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_frag_src =
{
   imgnat_bgra_frag_glsl
};

static const char imgnat_bgra_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_vert_src =
{
   imgnat_bgra_vert_glsl
};

static const char imgnat_12_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_frag_src =
{
   imgnat_12_frag_glsl
};

static const char imgnat_12_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_vert_src =
{
   imgnat_12_vert_glsl
};

static const char imgnat_21_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_frag_src =
{
   imgnat_21_frag_glsl
};

static const char imgnat_21_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_vert_src =
{
   imgnat_21_vert_glsl
};

static const char imgnat_22_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_frag_src =
{
   imgnat_22_frag_glsl
};

static const char imgnat_22_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_vert_src =
{
   imgnat_22_vert_glsl
};

static const char imgnat_12_bgra_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_frag_src =
{
   imgnat_12_bgra_frag_glsl
};

static const char imgnat_12_bgra_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_vert_src =
{
   imgnat_12_bgra_vert_glsl
};

static const char imgnat_21_bgra_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_frag_src =
{
   imgnat_21_bgra_frag_glsl
};

static const char imgnat_21_bgra_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_vert_src =
{
   imgnat_21_bgra_vert_glsl
};

static const char imgnat_22_bgra_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_frag_src =
{
   imgnat_22_bgra_frag_glsl
};

static const char imgnat_22_bgra_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_vert_src =
{
   imgnat_22_bgra_vert_glsl
};

static const char imgnat_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask_frag_src =
{
   imgnat_mask_frag_glsl
};

static const char imgnat_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_mask_vert_src =
{
   imgnat_mask_vert_glsl
};

static const char imgnat_bgra_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask_frag_src =
{
   imgnat_bgra_mask_frag_glsl
};

static const char imgnat_bgra_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_mask_vert_src =
{
   imgnat_bgra_mask_vert_glsl
};

static const char imgnat_12_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask_frag_src =
{
   imgnat_12_mask_frag_glsl
};

static const char imgnat_12_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_mask_vert_src =
{
   imgnat_12_mask_vert_glsl
};

static const char imgnat_21_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask_frag_src =
{
   imgnat_21_mask_frag_glsl
};

static const char imgnat_21_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_mask_vert_src =
{
   imgnat_21_mask_vert_glsl
};

static const char imgnat_22_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask_frag_src =
{
   imgnat_22_mask_frag_glsl
};

static const char imgnat_22_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_mask_vert_src =
{
   imgnat_22_mask_vert_glsl
};

static const char imgnat_12_bgra_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask_frag_src =
{
   imgnat_12_bgra_mask_frag_glsl
};

static const char imgnat_12_bgra_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_mask_vert_src =
{
   imgnat_12_bgra_mask_vert_glsl
};

static const char imgnat_21_bgra_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask_frag_src =
{
   imgnat_21_bgra_mask_frag_glsl
};

static const char imgnat_21_bgra_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_mask_vert_src =
{
   imgnat_21_bgra_mask_vert_glsl
};

static const char imgnat_22_bgra_mask_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask_frag_src =
{
   imgnat_22_bgra_mask_frag_glsl
};

static const char imgnat_22_bgra_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_mask_vert_src =
{
   imgnat_22_bgra_mask_vert_glsl
};

static const char imgnat_nomul_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_nomul_frag_src =
{
   imgnat_nomul_frag_glsl
};

static const char imgnat_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_nomul_vert_src =
{
   imgnat_nomul_vert_glsl
};

static const char imgnat_bgra_nomul_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_nomul_frag_src =
{
   imgnat_bgra_nomul_frag_glsl
};

static const char imgnat_bgra_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_nomul_vert_src =
{
   imgnat_bgra_nomul_vert_glsl
};

static const char imgnat_12_nomul_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_nomul_frag_src =
{
   imgnat_12_nomul_frag_glsl
};

static const char imgnat_12_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_nomul_vert_src =
{
   imgnat_12_nomul_vert_glsl
};

static const char imgnat_21_nomul_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_nomul_frag_src =
{
   imgnat_21_nomul_frag_glsl
};

static const char imgnat_21_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_nomul_vert_src =
{
   imgnat_21_nomul_vert_glsl
};

static const char imgnat_22_nomul_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_nomul_frag_src =
{
   imgnat_22_nomul_frag_glsl
};

static const char imgnat_22_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_nomul_vert_src =
{
   imgnat_22_nomul_vert_glsl
};

static const char imgnat_12_bgra_nomul_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_nomul_frag_src =
{
   imgnat_12_bgra_nomul_frag_glsl
};

static const char imgnat_12_bgra_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_nomul_vert_src =
{
   imgnat_12_bgra_nomul_vert_glsl
};

static const char imgnat_21_bgra_nomul_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_nomul_frag_src =
{
   imgnat_21_bgra_nomul_frag_glsl
};

static const char imgnat_21_bgra_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_nomul_vert_src =
{
   imgnat_21_bgra_nomul_vert_glsl
};

static const char imgnat_22_bgra_nomul_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_nomul_frag_src =
{
   imgnat_22_bgra_nomul_frag_glsl
};

static const char imgnat_22_bgra_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_nomul_vert_src =
{
   imgnat_22_bgra_nomul_vert_glsl
};

static const char imgnat_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask_nomul_frag_src =
{
   imgnat_mask_nomul_frag_glsl
};

static const char imgnat_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_mask_nomul_vert_src =
{
   imgnat_mask_nomul_vert_glsl
};

static const char imgnat_bgra_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask_nomul_frag_src =
{
   imgnat_bgra_mask_nomul_frag_glsl
};

static const char imgnat_bgra_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_mask_nomul_vert_src =
{
   imgnat_bgra_mask_nomul_vert_glsl
};

static const char imgnat_12_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask_nomul_frag_src =
{
   imgnat_12_mask_nomul_frag_glsl
};

static const char imgnat_12_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_mask_nomul_vert_src =
{
   imgnat_12_mask_nomul_vert_glsl
};

static const char imgnat_21_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask_nomul_frag_src =
{
   imgnat_21_mask_nomul_frag_glsl
};

static const char imgnat_21_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_mask_nomul_vert_src =
{
   imgnat_21_mask_nomul_vert_glsl
};

static const char imgnat_22_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask_nomul_frag_src =
{
   imgnat_22_mask_nomul_frag_glsl
};

static const char imgnat_22_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_mask_nomul_vert_src =
{
   imgnat_22_mask_nomul_vert_glsl
};

static const char imgnat_12_bgra_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask_nomul_frag_src =
{
   imgnat_12_bgra_mask_nomul_frag_glsl
};

static const char imgnat_12_bgra_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_mask_nomul_vert_src =
{
   imgnat_12_bgra_mask_nomul_vert_glsl
};

static const char imgnat_21_bgra_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask_nomul_frag_src =
{
   imgnat_21_bgra_mask_nomul_frag_glsl
};

static const char imgnat_21_bgra_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_mask_nomul_vert_src =
{
   imgnat_21_bgra_mask_nomul_vert_glsl
};

static const char imgnat_22_bgra_mask_nomul_frag_glsl[] =
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
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask_nomul_frag_src =
{
   imgnat_22_bgra_mask_nomul_frag_glsl
};

static const char imgnat_22_bgra_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_mask_nomul_vert_src =
{
   imgnat_22_bgra_mask_nomul_vert_glsl
};

static const char imgnat_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask12_frag_src =
{
   imgnat_mask12_frag_glsl
};

static const char imgnat_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask12_vert_src =
{
   imgnat_mask12_vert_glsl
};

static const char imgnat_bgra_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask12_frag_src =
{
   imgnat_bgra_mask12_frag_glsl
};

static const char imgnat_bgra_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask12_vert_src =
{
   imgnat_bgra_mask12_vert_glsl
};

static const char imgnat_12_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask12_frag_src =
{
   imgnat_12_mask12_frag_glsl
};

static const char imgnat_12_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask12_vert_src =
{
   imgnat_12_mask12_vert_glsl
};

static const char imgnat_21_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask12_frag_src =
{
   imgnat_21_mask12_frag_glsl
};

static const char imgnat_21_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask12_vert_src =
{
   imgnat_21_mask12_vert_glsl
};

static const char imgnat_22_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask12_frag_src =
{
   imgnat_22_mask12_frag_glsl
};

static const char imgnat_22_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
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
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask12_vert_src =
{
   imgnat_22_mask12_vert_glsl
};

static const char imgnat_12_bgra_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask12_frag_src =
{
   imgnat_12_bgra_mask12_frag_glsl
};

static const char imgnat_12_bgra_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask12_vert_src =
{
   imgnat_12_bgra_mask12_vert_glsl
};

static const char imgnat_21_bgra_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask12_frag_src =
{
   imgnat_21_bgra_mask12_frag_glsl
};

static const char imgnat_21_bgra_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask12_vert_src =
{
   imgnat_21_bgra_mask12_vert_glsl
};

static const char imgnat_22_bgra_mask12_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask12_frag_src =
{
   imgnat_22_bgra_mask12_frag_glsl
};

static const char imgnat_22_bgra_mask12_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
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
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask12_vert_src =
{
   imgnat_22_bgra_mask12_vert_glsl
};

static const char imgnat_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask12_nomul_frag_src =
{
   imgnat_mask12_nomul_frag_glsl
};

static const char imgnat_mask12_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask12_nomul_vert_src =
{
   imgnat_mask12_nomul_vert_glsl
};

static const char imgnat_bgra_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask12_nomul_frag_src =
{
   imgnat_bgra_mask12_nomul_frag_glsl
};

static const char imgnat_bgra_mask12_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask12_nomul_vert_src =
{
   imgnat_bgra_mask12_nomul_vert_glsl
};

static const char imgnat_12_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask12_nomul_frag_src =
{
   imgnat_12_mask12_nomul_frag_glsl
};

static const char imgnat_12_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask12_nomul_vert_src =
{
   imgnat_12_mask12_nomul_vert_glsl
};

static const char imgnat_21_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask12_nomul_frag_src =
{
   imgnat_21_mask12_nomul_frag_glsl
};

static const char imgnat_21_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask12_nomul_vert_src =
{
   imgnat_21_mask12_nomul_vert_glsl
};

static const char imgnat_22_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask12_nomul_frag_src =
{
   imgnat_22_mask12_nomul_frag_glsl
};

static const char imgnat_22_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask12_nomul_vert_src =
{
   imgnat_22_mask12_nomul_vert_glsl
};

static const char imgnat_12_bgra_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask12_nomul_frag_src =
{
   imgnat_12_bgra_mask12_nomul_frag_glsl
};

static const char imgnat_12_bgra_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask12_nomul_vert_src =
{
   imgnat_12_bgra_mask12_nomul_vert_glsl
};

static const char imgnat_21_bgra_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask12_nomul_frag_src =
{
   imgnat_21_bgra_mask12_nomul_frag_glsl
};

static const char imgnat_21_bgra_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask12_nomul_vert_src =
{
   imgnat_21_bgra_mask12_nomul_vert_glsl
};

static const char imgnat_22_bgra_mask12_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask12_nomul_frag_src =
{
   imgnat_22_bgra_mask12_nomul_frag_glsl
};

static const char imgnat_22_bgra_mask12_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(0, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2(0, tex_masksample.y);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask12_nomul_vert_src =
{
   imgnat_22_bgra_mask12_nomul_vert_glsl
};

static const char imgnat_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask21_frag_src =
{
   imgnat_mask21_frag_glsl
};

static const char imgnat_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask21_vert_src =
{
   imgnat_mask21_vert_glsl
};

static const char imgnat_bgra_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask21_frag_src =
{
   imgnat_bgra_mask21_frag_glsl
};

static const char imgnat_bgra_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask21_vert_src =
{
   imgnat_bgra_mask21_vert_glsl
};

static const char imgnat_12_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask21_frag_src =
{
   imgnat_12_mask21_frag_glsl
};

static const char imgnat_12_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask21_vert_src =
{
   imgnat_12_mask21_vert_glsl
};

static const char imgnat_21_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask21_frag_src =
{
   imgnat_21_mask21_frag_glsl
};

static const char imgnat_21_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask21_vert_src =
{
   imgnat_21_mask21_vert_glsl
};

static const char imgnat_22_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask21_frag_src =
{
   imgnat_22_mask21_frag_glsl
};

static const char imgnat_22_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
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
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask21_vert_src =
{
   imgnat_22_mask21_vert_glsl
};

static const char imgnat_12_bgra_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask21_frag_src =
{
   imgnat_12_bgra_mask21_frag_glsl
};

static const char imgnat_12_bgra_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask21_vert_src =
{
   imgnat_12_bgra_mask21_vert_glsl
};

static const char imgnat_21_bgra_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask21_frag_src =
{
   imgnat_21_bgra_mask21_frag_glsl
};

static const char imgnat_21_bgra_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask21_vert_src =
{
   imgnat_21_bgra_mask21_vert_glsl
};

static const char imgnat_22_bgra_mask21_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask21_frag_src =
{
   imgnat_22_bgra_mask21_frag_glsl
};

static const char imgnat_22_bgra_mask21_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
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
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask21_vert_src =
{
   imgnat_22_bgra_mask21_vert_glsl
};

static const char imgnat_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask21_nomul_frag_src =
{
   imgnat_mask21_nomul_frag_glsl
};

static const char imgnat_mask21_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask21_nomul_vert_src =
{
   imgnat_mask21_nomul_vert_glsl
};

static const char imgnat_bgra_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask21_nomul_frag_src =
{
   imgnat_bgra_mask21_nomul_frag_glsl
};

static const char imgnat_bgra_mask21_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask21_nomul_vert_src =
{
   imgnat_bgra_mask21_nomul_vert_glsl
};

static const char imgnat_12_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask21_nomul_frag_src =
{
   imgnat_12_mask21_nomul_frag_glsl
};

static const char imgnat_12_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask21_nomul_vert_src =
{
   imgnat_12_mask21_nomul_vert_glsl
};

static const char imgnat_21_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask21_nomul_frag_src =
{
   imgnat_21_mask21_nomul_frag_glsl
};

static const char imgnat_21_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask21_nomul_vert_src =
{
   imgnat_21_mask21_nomul_vert_glsl
};

static const char imgnat_22_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask21_nomul_frag_src =
{
   imgnat_22_mask21_nomul_frag_glsl
};

static const char imgnat_22_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask21_nomul_vert_src =
{
   imgnat_22_mask21_nomul_vert_glsl
};

static const char imgnat_12_bgra_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask21_nomul_frag_src =
{
   imgnat_12_bgra_mask21_nomul_frag_glsl
};

static const char imgnat_12_bgra_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask21_nomul_vert_src =
{
   imgnat_12_bgra_mask21_nomul_vert_glsl
};

static const char imgnat_21_bgra_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask21_nomul_frag_src =
{
   imgnat_21_bgra_mask21_nomul_frag_glsl
};

static const char imgnat_21_bgra_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask21_nomul_vert_src =
{
   imgnat_21_bgra_mask21_nomul_vert_glsl
};

static const char imgnat_22_bgra_mask21_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   ma = (ma00 + ma01) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask21_nomul_frag_src =
{
   imgnat_22_bgra_mask21_nomul_frag_glsl
};

static const char imgnat_22_bgra_mask21_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[2];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, 0);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, 0);\n"
   "   maskdiv_s = 2.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask21_nomul_vert_src =
{
   imgnat_22_bgra_mask21_nomul_vert_glsl
};

static const char imgnat_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask22_frag_src =
{
   imgnat_mask22_frag_glsl
};

static const char imgnat_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask22_vert_src =
{
   imgnat_mask22_vert_glsl
};

static const char imgnat_bgra_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask22_frag_src =
{
   imgnat_bgra_mask22_frag_glsl
};

static const char imgnat_bgra_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask22_vert_src =
{
   imgnat_bgra_mask22_vert_glsl
};

static const char imgnat_12_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask22_frag_src =
{
   imgnat_12_mask22_frag_glsl
};

static const char imgnat_12_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask22_vert_src =
{
   imgnat_12_mask22_vert_glsl
};

static const char imgnat_21_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask22_frag_src =
{
   imgnat_21_mask22_frag_glsl
};

static const char imgnat_21_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask22_vert_src =
{
   imgnat_21_mask22_vert_glsl
};

static const char imgnat_22_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask22_frag_src =
{
   imgnat_22_mask22_frag_glsl
};

static const char imgnat_22_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
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
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask22_vert_src =
{
   imgnat_22_mask22_vert_glsl
};

static const char imgnat_12_bgra_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask22_frag_src =
{
   imgnat_12_bgra_mask22_frag_glsl
};

static const char imgnat_12_bgra_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask22_vert_src =
{
   imgnat_12_bgra_mask22_vert_glsl
};

static const char imgnat_21_bgra_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask22_frag_src =
{
   imgnat_21_bgra_mask22_frag_glsl
};

static const char imgnat_21_bgra_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask22_vert_src =
{
   imgnat_21_bgra_mask22_vert_glsl
};

static const char imgnat_22_bgra_mask22_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask22_frag_src =
{
   imgnat_22_bgra_mask22_frag_glsl
};

static const char imgnat_22_bgra_mask22_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
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
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask22_vert_src =
{
   imgnat_22_bgra_mask22_vert_glsl
};

static const char imgnat_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask22_nomul_frag_src =
{
   imgnat_mask22_nomul_frag_glsl
};

static const char imgnat_mask22_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_mask22_nomul_vert_src =
{
   imgnat_mask22_nomul_vert_glsl
};

static const char imgnat_bgra_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).rgba;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask22_nomul_frag_src =
{
   imgnat_bgra_mask22_nomul_frag_glsl
};

static const char imgnat_bgra_mask22_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_bgra_mask22_nomul_vert_src =
{
   imgnat_bgra_mask22_nomul_vert_glsl
};

static const char imgnat_12_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask22_nomul_frag_src =
{
   imgnat_12_mask22_nomul_frag_glsl
};

static const char imgnat_12_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_mask22_nomul_vert_src =
{
   imgnat_12_mask22_nomul_vert_glsl
};

static const char imgnat_21_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask22_nomul_frag_src =
{
   imgnat_21_mask22_nomul_frag_glsl
};

static const char imgnat_21_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_mask22_nomul_vert_src =
{
   imgnat_21_mask22_nomul_vert_glsl
};

static const char imgnat_22_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).bgra;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).bgra;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).bgra;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).bgra;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask22_nomul_frag_src =
{
   imgnat_22_mask22_nomul_frag_glsl
};

static const char imgnat_22_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_mask22_nomul_vert_src =
{
   imgnat_22_mask22_nomul_vert_glsl
};

static const char imgnat_12_bgra_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask22_nomul_frag_src =
{
   imgnat_12_bgra_mask22_nomul_frag_glsl
};

static const char imgnat_12_bgra_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(0, -tex_sample.y);\n"
   "   tex_s[1] = vec2(0, tex_sample.y);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_12_bgra_mask22_nomul_vert_src =
{
   imgnat_12_bgra_mask22_nomul_vert_glsl
};

static const char imgnat_21_bgra_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   c = (col00 + col01) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask22_nomul_frag_src =
{
   imgnat_21_bgra_mask22_nomul_frag_glsl
};

static const char imgnat_21_bgra_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, 0);\n"
   "   tex_s[1] = vec2( tex_sample.x, 0);\n"
   "   div_s = vec4(2, 2, 2, 2);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_21_bgra_mask22_nomul_vert_src =
{
   imgnat_21_bgra_mask22_nomul_vert_glsl
};

static const char imgnat_22_bgra_mask22_nomul_frag_glsl[] =
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
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   vec4 col00 = texture2D(tex, tex_c + tex_s[0]).rgba;\n"
   "   vec4 col01 = texture2D(tex, tex_c + tex_s[1]).rgba;\n"
   "   vec4 col10 = texture2D(tex, tex_c + tex_s[2]).rgba;\n"
   "   vec4 col11 = texture2D(tex, tex_c + tex_s[3]).rgba;\n"
   "   c = (col00 + col01 + col10 + col11) / div_s;\n"
   "   float ma;\n"
   "   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;\n"
   "   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;\n"
   "   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;\n"
   "   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;\n"
   "   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask22_nomul_frag_src =
{
   imgnat_22_bgra_mask22_nomul_frag_glsl
};

static const char imgnat_22_bgra_mask22_nomul_vert_glsl[] =
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
   "attribute vec2 tex_masksample;\n"
   "varying float maskdiv_s;\n"
   "varying vec2 masktex_s[4];\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);\n"
   "   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);\n"
   "   tex_s[2] = vec2( tex_sample.x, tex_sample.y);\n"
   "   tex_s[3] = vec2(-tex_sample.x, tex_sample.y);\n"
   "   div_s = vec4(4, 4, 4, 4);\n"
   "   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);\n"
   "   masktex_s[2] = vec2( tex_masksample.x, tex_masksample.y);\n"
   "   masktex_s[3] = vec2(-tex_masksample.x, tex_masksample.y);\n"
   "   maskdiv_s = 4.0;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_imgnat_22_bgra_mask22_nomul_vert_src =
{
   imgnat_22_bgra_mask22_nomul_vert_glsl
};

static const char imgnat_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_afill_frag_src =
{
   imgnat_afill_frag_glsl
};

static const char imgnat_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_afill_vert_src =
{
   imgnat_afill_vert_glsl
};

static const char imgnat_bgra_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_afill_frag_src =
{
   imgnat_bgra_afill_frag_glsl
};

static const char imgnat_bgra_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_afill_vert_src =
{
   imgnat_bgra_afill_vert_glsl
};

static const char imgnat_nomul_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_nomul_afill_frag_src =
{
   imgnat_nomul_afill_frag_glsl
};

static const char imgnat_nomul_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_nomul_afill_vert_src =
{
   imgnat_nomul_afill_vert_glsl
};

static const char imgnat_bgra_nomul_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_nomul_afill_frag_src =
{
   imgnat_bgra_nomul_afill_frag_glsl
};

static const char imgnat_bgra_nomul_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_bgra_nomul_afill_vert_src =
{
   imgnat_bgra_nomul_afill_vert_glsl
};

static const char imgnat_12_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_afill_frag_src =
{
   imgnat_12_afill_frag_glsl
};

static const char imgnat_12_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_afill_vert_src =
{
   imgnat_12_afill_vert_glsl
};

static const char imgnat_21_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_afill_frag_src =
{
   imgnat_21_afill_frag_glsl
};

static const char imgnat_21_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_afill_vert_src =
{
   imgnat_21_afill_vert_glsl
};

static const char imgnat_22_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_afill_frag_src =
{
   imgnat_22_afill_frag_glsl
};

static const char imgnat_22_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_afill_vert_src =
{
   imgnat_22_afill_vert_glsl
};

static const char imgnat_12_bgra_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_afill_frag_src =
{
   imgnat_12_bgra_afill_frag_glsl
};

static const char imgnat_12_bgra_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_afill_vert_src =
{
   imgnat_12_bgra_afill_vert_glsl
};

static const char imgnat_21_bgra_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_afill_frag_src =
{
   imgnat_21_bgra_afill_frag_glsl
};

static const char imgnat_21_bgra_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_afill_vert_src =
{
   imgnat_21_bgra_afill_vert_glsl
};

static const char imgnat_22_bgra_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_afill_frag_src =
{
   imgnat_22_bgra_afill_frag_glsl
};

static const char imgnat_22_bgra_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_afill_vert_src =
{
   imgnat_22_bgra_afill_vert_glsl
};

static const char imgnat_12_nomul_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_nomul_afill_frag_src =
{
   imgnat_12_nomul_afill_frag_glsl
};

static const char imgnat_12_nomul_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_nomul_afill_vert_src =
{
   imgnat_12_nomul_afill_vert_glsl
};

static const char imgnat_21_nomul_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_nomul_afill_frag_src =
{
   imgnat_21_nomul_afill_frag_glsl
};

static const char imgnat_21_nomul_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_nomul_afill_vert_src =
{
   imgnat_21_nomul_afill_vert_glsl
};

static const char imgnat_22_nomul_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_nomul_afill_frag_src =
{
   imgnat_22_nomul_afill_frag_glsl
};

static const char imgnat_22_nomul_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_nomul_afill_vert_src =
{
   imgnat_22_nomul_afill_vert_glsl
};

static const char imgnat_12_bgra_nomul_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_nomul_afill_frag_src =
{
   imgnat_12_bgra_nomul_afill_frag_glsl
};

static const char imgnat_12_bgra_nomul_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_12_bgra_nomul_afill_vert_src =
{
   imgnat_12_bgra_nomul_afill_vert_glsl
};

static const char imgnat_21_bgra_nomul_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_nomul_afill_frag_src =
{
   imgnat_21_bgra_nomul_afill_frag_glsl
};

static const char imgnat_21_bgra_nomul_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_21_bgra_nomul_afill_vert_src =
{
   imgnat_21_bgra_nomul_afill_vert_glsl
};

static const char imgnat_22_bgra_nomul_afill_frag_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_nomul_afill_frag_src =
{
   imgnat_22_bgra_nomul_afill_frag_glsl
};

static const char imgnat_22_bgra_nomul_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_imgnat_22_bgra_nomul_afill_vert_src =
{
   imgnat_22_bgra_nomul_afill_vert_glsl
};

static const char rgb_a_pair_frag_glsl[] =
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
   "uniform sampler2D texa;\n"
   "varying vec2 tex_a;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "     * texture2D(texa, tex_a).r\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_frag_src =
{
   rgb_a_pair_frag_glsl
};

static const char rgb_a_pair_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coorda;\n"
   "varying vec2 tex_a;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_a = tex_coorda;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_vert_src =
{
   rgb_a_pair_vert_glsl
};

static const char rgb_a_pair_mask_frag_glsl[] =
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
   "uniform sampler2D texa;\n"
   "varying vec2 tex_a;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "     * texture2D(texa, tex_a).r\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_frag_src =
{
   rgb_a_pair_mask_frag_glsl
};

static const char rgb_a_pair_mask_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec4 color;\n"
   "varying vec4 col;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coorda;\n"
   "varying vec2 tex_a;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   col = color;\n"
   "   tex_c = tex_coord;\n"
   "   tex_a = tex_coorda;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_vert_src =
{
   rgb_a_pair_mask_vert_glsl
};

static const char rgb_a_pair_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texa;\n"
   "varying vec2 tex_a;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * texture2D(texa, tex_a).r\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_nomul_frag_src =
{
   rgb_a_pair_nomul_frag_glsl
};

static const char rgb_a_pair_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coorda;\n"
   "varying vec2 tex_a;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_a = tex_coorda;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_nomul_vert_src =
{
   rgb_a_pair_nomul_vert_glsl
};

static const char rgb_a_pair_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "#else\n"
   "precision mediump float;\n"
   "#endif\n"
   "#endif\n"
   "uniform sampler2D tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texa;\n"
   "varying vec2 tex_a;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "     * texture2D(texa, tex_a).r\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_nomul_frag_src =
{
   rgb_a_pair_mask_nomul_frag_glsl
};

static const char rgb_a_pair_mask_nomul_vert_glsl[] =
   "#ifdef GL_ES\n"
   "precision highp float;\n"
   "#endif\n"
   "attribute vec4 vertex;\n"
   "uniform mat4 mvp;\n"
   "attribute vec2 tex_coord;\n"
   "varying vec2 tex_c;\n"
   "attribute vec2 tex_coorda;\n"
   "varying vec2 tex_a;\n"
   "attribute vec4 mask_coord;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   gl_Position = mvp * vertex;\n"
   "   tex_c = tex_coord;\n"
   "   tex_a = tex_coorda;\n"
   "   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);\n"
   "   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;\n"
   "}\n";
Evas_GL_Program_Source shader_rgb_a_pair_mask_nomul_vert_src =
{
   rgb_a_pair_mask_nomul_vert_glsl
};

static const char tex_external_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "# define SAMPLER_EXTERNAL_OES samplerExternalOES\n"
   "#else\n"
   "# define SAMPLER_EXTERNAL_OES sampler2D\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform SAMPLER_EXTERNAL_OES tex;\n"
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
   tex_external_frag_glsl
};

static const char tex_external_vert_glsl[] =
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
Evas_GL_Program_Source shader_tex_external_vert_src =
{
   tex_external_vert_glsl
};

static const char tex_external_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "# define SAMPLER_EXTERNAL_OES samplerExternalOES\n"
   "#else\n"
   "# define SAMPLER_EXTERNAL_OES sampler2D\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform SAMPLER_EXTERNAL_OES tex;\n"
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
   tex_external_afill_frag_glsl
};

static const char tex_external_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_tex_external_afill_vert_src =
{
   tex_external_afill_vert_glsl
};

static const char tex_external_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "# define SAMPLER_EXTERNAL_OES samplerExternalOES\n"
   "#else\n"
   "# define SAMPLER_EXTERNAL_OES sampler2D\n"
   "#endif\n"
   "uniform SAMPLER_EXTERNAL_OES tex;\n"
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
   tex_external_nomul_frag_glsl
};

static const char tex_external_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_tex_external_nomul_vert_src =
{
   tex_external_nomul_vert_glsl
};

static const char tex_external_nomul_afill_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "# define SAMPLER_EXTERNAL_OES samplerExternalOES\n"
   "#else\n"
   "# define SAMPLER_EXTERNAL_OES sampler2D\n"
   "#endif\n"
   "uniform SAMPLER_EXTERNAL_OES tex;\n"
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
   tex_external_nomul_afill_frag_glsl
};

static const char tex_external_nomul_afill_vert_glsl[] =
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
Evas_GL_Program_Source shader_tex_external_nomul_afill_vert_src =
{
   tex_external_nomul_afill_vert_glsl
};

static const char tex_external_mask_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "# define SAMPLER_EXTERNAL_OES samplerExternalOES\n"
   "#else\n"
   "# define SAMPLER_EXTERNAL_OES sampler2D\n"
   "#endif\n"
   "varying vec4 col;\n"
   "uniform SAMPLER_EXTERNAL_OES tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_mask_frag_src =
{
   tex_external_mask_frag_glsl
};

static const char tex_external_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_tex_external_mask_vert_src =
{
   tex_external_mask_vert_glsl
};

static const char tex_external_mask_nomul_frag_glsl[] =
   "#ifdef GL_ES\n"
   "# extension GL_OES_EGL_image_external : require\n"
   "# ifdef GL_FRAGMENT_PRECISION_HIGH\n"
   "precision highp float;\n"
   "# else\n"
   "precision mediump float;\n"
   "# endif\n"
   "# define SAMPLER_EXTERNAL_OES samplerExternalOES\n"
   "#else\n"
   "# define SAMPLER_EXTERNAL_OES sampler2D\n"
   "#endif\n"
   "uniform SAMPLER_EXTERNAL_OES tex;\n"
   "varying vec2 tex_c;\n"
   "uniform sampler2D texm;\n"
   "varying vec2 tex_m;\n"
   "void main()\n"
   "{\n"
   "   vec4 c;\n"
   "   c = texture2D(tex, tex_c).bgra;\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_tex_external_mask_nomul_frag_src =
{
   tex_external_mask_nomul_frag_glsl
};

static const char tex_external_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_tex_external_mask_nomul_vert_src =
{
   tex_external_mask_nomul_vert_glsl
};

static const char yuv_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_frag_src =
{
   yuv_frag_glsl
};

static const char yuv_vert_glsl[] =
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
   yuv_vert_glsl
};

static const char yuv_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_nomul_frag_src =
{
   yuv_nomul_frag_glsl
};

static const char yuv_nomul_vert_glsl[] =
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
   yuv_nomul_vert_glsl
};

static const char yuv_mask_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_mask_frag_src =
{
   yuv_mask_frag_glsl
};

static const char yuv_mask_vert_glsl[] =
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
   yuv_mask_vert_glsl
};

static const char yuv_mask_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_mask_nomul_frag_src =
{
   yuv_mask_nomul_frag_glsl
};

static const char yuv_mask_nomul_vert_glsl[] =
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
   yuv_mask_nomul_vert_glsl
};

static const char yuy2_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
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
   yuy2_frag_glsl
};

static const char yuy2_vert_glsl[] =
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
   yuy2_vert_glsl
};

static const char yuy2_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
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
   yuy2_nomul_frag_glsl
};

static const char yuy2_nomul_vert_glsl[] =
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
   yuy2_nomul_vert_glsl
};

static const char yuy2_mask_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_mask_frag_src =
{
   yuy2_mask_frag_glsl
};

static const char yuy2_mask_vert_glsl[] =
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
   yuy2_mask_vert_glsl
};

static const char yuy2_mask_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_mask_nomul_frag_src =
{
   yuy2_mask_nomul_frag_glsl
};

static const char yuy2_mask_nomul_vert_glsl[] =
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
   yuy2_mask_nomul_vert_glsl
};

static const char nv12_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
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
   nv12_frag_glsl
};

static const char nv12_vert_glsl[] =
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
   nv12_vert_glsl
};

static const char nv12_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
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
   nv12_nomul_frag_glsl
};

static const char nv12_nomul_vert_glsl[] =
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
   nv12_nomul_vert_glsl
};

static const char nv12_mask_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_mask_frag_src =
{
   nv12_mask_frag_glsl
};

static const char nv12_mask_vert_glsl[] =
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
   nv12_mask_vert_glsl
};

static const char nv12_mask_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.813) + (u * 0.391);\n"
   "   v = v * 1.596;\n"
   "   u = u * 2.018;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_mask_nomul_frag_src =
{
   nv12_mask_nomul_frag_glsl
};

static const char nv12_mask_nomul_vert_glsl[] =
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
   nv12_mask_nomul_vert_glsl
};

static const char yuv_709_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_709_frag_src =
{
   yuv_709_frag_glsl
};

static const char yuv_709_vert_glsl[] =
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
Evas_GL_Program_Source shader_yuv_709_vert_src =
{
   yuv_709_vert_glsl
};

static const char yuv_709_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_709_nomul_frag_src =
{
   yuv_709_nomul_frag_glsl
};

static const char yuv_709_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_yuv_709_nomul_vert_src =
{
   yuv_709_nomul_vert_glsl
};

static const char yuv_709_mask_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_709_mask_frag_src =
{
   yuv_709_mask_frag_glsl
};

static const char yuv_709_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_yuv_709_mask_vert_src =
{
   yuv_709_mask_vert_glsl
};

static const char yuv_709_mask_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).r;\n"
   "   u = texture2D(texu, tex_c2).r;\n"
   "   v = texture2D(texv, tex_c3).r;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuv_709_mask_nomul_frag_src =
{
   yuv_709_mask_nomul_frag_glsl
};

static const char yuv_709_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_yuv_709_mask_nomul_vert_src =
{
   yuv_709_mask_nomul_vert_glsl
};

static const char yuy2_709_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_709_frag_src =
{
   yuy2_709_frag_glsl
};

static const char yuy2_709_vert_glsl[] =
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
Evas_GL_Program_Source shader_yuy2_709_vert_src =
{
   yuy2_709_vert_glsl
};

static const char yuy2_709_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_709_nomul_frag_src =
{
   yuy2_709_nomul_frag_glsl
};

static const char yuy2_709_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_yuy2_709_nomul_vert_src =
{
   yuy2_709_nomul_vert_glsl
};

static const char yuy2_709_mask_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_709_mask_frag_src =
{
   yuy2_709_mask_frag_glsl
};

static const char yuy2_709_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_yuy2_709_mask_vert_src =
{
   yuy2_709_mask_vert_glsl
};

static const char yuy2_709_mask_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_yuy2_709_mask_nomul_frag_src =
{
   yuy2_709_mask_nomul_frag_glsl
};

static const char yuy2_709_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_yuy2_709_mask_nomul_vert_src =
{
   yuy2_709_mask_nomul_vert_glsl
};

static const char nv12_709_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_709_frag_src =
{
   nv12_709_frag_glsl
};

static const char nv12_709_vert_glsl[] =
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
Evas_GL_Program_Source shader_nv12_709_vert_src =
{
   nv12_709_vert_glsl
};

static const char nv12_709_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   gl_FragColor =\n"
   "       c\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_709_nomul_frag_src =
{
   nv12_709_nomul_frag_glsl
};

static const char nv12_709_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_nv12_709_nomul_vert_src =
{
   nv12_709_nomul_vert_glsl
};

static const char nv12_709_mask_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "     * col\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_709_mask_frag_src =
{
   nv12_709_mask_frag_glsl
};

static const char nv12_709_mask_vert_glsl[] =
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
Evas_GL_Program_Source shader_nv12_709_mask_vert_src =
{
   nv12_709_mask_vert_glsl
};

static const char nv12_709_mask_nomul_frag_glsl[] =
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
   "   float r, g, b, y, u, v, vmu;\n"
   "   y = texture2D(tex, tex_c).g;\n"
   "   u = texture2D(texuv, tex_c2).g;\n"
   "   v = texture2D(texuv, tex_c2).a;\n"
   "   u = u - 0.5;\n"
   "   v = v - 0.5;\n"
   "   y = (y - 0.062) * 1.164;\n"
   "   vmu = (v * 0.534) + (u * 0.213);\n"
   "   v = v * 1.793;\n"
   "   u = u * 2.115;\n"
   "   r = y + v;\n"
   "   g = y - vmu;\n"
   "   b = y + u;\n"
   "   c = vec4(r, g, b, 1.0);\n"
   "   float ma;\n"
   "   ma = texture2D(texm, tex_m).a;\n"
   "   gl_FragColor =\n"
   "       c\n"
   "  * ma\n"
   "   ;\n"
   "}\n";
Evas_GL_Program_Source shader_nv12_709_mask_nomul_frag_src =
{
   nv12_709_mask_nomul_frag_glsl
};

static const char nv12_709_mask_nomul_vert_glsl[] =
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
Evas_GL_Program_Source shader_nv12_709_mask_nomul_vert_src =
{
   nv12_709_mask_nomul_vert_glsl
};


static const struct {
   Evas_GL_Shader id;
   Evas_GL_Program_Source *vert;
   Evas_GL_Program_Source *frag;
   const char *name;
   Shader_Type type;
   Shader_Sampling sam;
   Shader_Sampling masksam;
   Eina_Bool bgra : 1;
   Eina_Bool mask : 1;
   Eina_Bool nomul : 1;
   Eina_Bool afill : 1;
} _shaders_source[] = {
   { SHADER_RECT, &(shader_rect_vert_src), &(shader_rect_frag_src), "rect", SHD_RECT, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_RECT_MASK, &(shader_rect_mask_vert_src), &(shader_rect_mask_frag_src), "rect_mask", SHD_RECT, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_RECT_MASK12, &(shader_rect_mask12_vert_src), &(shader_rect_mask12_frag_src), "rect_mask12", SHD_RECT, SHD_SAM11, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_RECT_MASK21, &(shader_rect_mask21_vert_src), &(shader_rect_mask21_frag_src), "rect_mask21", SHD_RECT, SHD_SAM11, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_RECT_MASK22, &(shader_rect_mask22_vert_src), &(shader_rect_mask22_frag_src), "rect_mask22", SHD_RECT, SHD_SAM11, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_FONT, &(shader_font_vert_src), &(shader_font_frag_src), "font", SHD_FONT, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_FONT_MASK, &(shader_font_mask_vert_src), &(shader_font_mask_frag_src), "font_mask", SHD_FONT, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_FONT_MASK12, &(shader_font_mask12_vert_src), &(shader_font_mask12_frag_src), "font_mask12", SHD_FONT, SHD_SAM11, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_FONT_MASK21, &(shader_font_mask21_vert_src), &(shader_font_mask21_frag_src), "font_mask21", SHD_FONT, SHD_SAM11, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_FONT_MASK22, &(shader_font_mask22_vert_src), &(shader_font_mask22_frag_src), "font_mask22", SHD_FONT, SHD_SAM11, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_IMG, &(shader_img_vert_src), &(shader_img_frag_src), "img", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_IMG_BGRA, &(shader_img_bgra_vert_src), &(shader_img_bgra_frag_src), "img_bgra", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 1, 0, 0, 0 },
   { SHADER_IMG_12, &(shader_img_12_vert_src), &(shader_img_12_frag_src), "img_12", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_IMG_21, &(shader_img_21_vert_src), &(shader_img_21_frag_src), "img_21", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_IMG_22, &(shader_img_22_vert_src), &(shader_img_22_frag_src), "img_22", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_IMG_12_BGRA, &(shader_img_12_bgra_vert_src), &(shader_img_12_bgra_frag_src), "img_12_bgra", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 1, 0, 0, 0 },
   { SHADER_IMG_21_BGRA, &(shader_img_21_bgra_vert_src), &(shader_img_21_bgra_frag_src), "img_21_bgra", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 1, 0, 0, 0 },
   { SHADER_IMG_22_BGRA, &(shader_img_22_bgra_vert_src), &(shader_img_22_bgra_frag_src), "img_22_bgra", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 1, 0, 0, 0 },
   { SHADER_IMG_MASK, &(shader_img_mask_vert_src), &(shader_img_mask_frag_src), "img_mask", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_IMG_BGRA_MASK, &(shader_img_bgra_mask_vert_src), &(shader_img_bgra_mask_frag_src), "img_bgra_mask", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 1, 1, 0, 0 },
   { SHADER_IMG_12_MASK, &(shader_img_12_mask_vert_src), &(shader_img_12_mask_frag_src), "img_12_mask", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_IMG_21_MASK, &(shader_img_21_mask_vert_src), &(shader_img_21_mask_frag_src), "img_21_mask", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_IMG_22_MASK, &(shader_img_22_mask_vert_src), &(shader_img_22_mask_frag_src), "img_22_mask", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_IMG_12_BGRA_MASK, &(shader_img_12_bgra_mask_vert_src), &(shader_img_12_bgra_mask_frag_src), "img_12_bgra_mask", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 1, 1, 0, 0 },
   { SHADER_IMG_21_BGRA_MASK, &(shader_img_21_bgra_mask_vert_src), &(shader_img_21_bgra_mask_frag_src), "img_21_bgra_mask", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 1, 1, 0, 0 },
   { SHADER_IMG_22_BGRA_MASK, &(shader_img_22_bgra_mask_vert_src), &(shader_img_22_bgra_mask_frag_src), "img_22_bgra_mask", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 1, 1, 0, 0 },
   { SHADER_IMG_NOMUL, &(shader_img_nomul_vert_src), &(shader_img_nomul_frag_src), "img_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_IMG_BGRA_NOMUL, &(shader_img_bgra_nomul_vert_src), &(shader_img_bgra_nomul_frag_src), "img_bgra_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 1, 0, 1, 0 },
   { SHADER_IMG_12_NOMUL, &(shader_img_12_nomul_vert_src), &(shader_img_12_nomul_frag_src), "img_12_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_IMG_21_NOMUL, &(shader_img_21_nomul_vert_src), &(shader_img_21_nomul_frag_src), "img_21_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_IMG_22_NOMUL, &(shader_img_22_nomul_vert_src), &(shader_img_22_nomul_frag_src), "img_22_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_IMG_12_BGRA_NOMUL, &(shader_img_12_bgra_nomul_vert_src), &(shader_img_12_bgra_nomul_frag_src), "img_12_bgra_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 1, 0, 1, 0 },
   { SHADER_IMG_21_BGRA_NOMUL, &(shader_img_21_bgra_nomul_vert_src), &(shader_img_21_bgra_nomul_frag_src), "img_21_bgra_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 1, 0, 1, 0 },
   { SHADER_IMG_22_BGRA_NOMUL, &(shader_img_22_bgra_nomul_vert_src), &(shader_img_22_bgra_nomul_frag_src), "img_22_bgra_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 1, 0, 1, 0 },
   { SHADER_IMG_MASK_NOMUL, &(shader_img_mask_nomul_vert_src), &(shader_img_mask_nomul_frag_src), "img_mask_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_IMG_BGRA_MASK_NOMUL, &(shader_img_bgra_mask_nomul_vert_src), &(shader_img_bgra_mask_nomul_frag_src), "img_bgra_mask_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 1, 1, 1, 0 },
   { SHADER_IMG_12_MASK_NOMUL, &(shader_img_12_mask_nomul_vert_src), &(shader_img_12_mask_nomul_frag_src), "img_12_mask_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_IMG_21_MASK_NOMUL, &(shader_img_21_mask_nomul_vert_src), &(shader_img_21_mask_nomul_frag_src), "img_21_mask_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_IMG_22_MASK_NOMUL, &(shader_img_22_mask_nomul_vert_src), &(shader_img_22_mask_nomul_frag_src), "img_22_mask_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_IMG_12_BGRA_MASK_NOMUL, &(shader_img_12_bgra_mask_nomul_vert_src), &(shader_img_12_bgra_mask_nomul_frag_src), "img_12_bgra_mask_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 1, 1, 1, 0 },
   { SHADER_IMG_21_BGRA_MASK_NOMUL, &(shader_img_21_bgra_mask_nomul_vert_src), &(shader_img_21_bgra_mask_nomul_frag_src), "img_21_bgra_mask_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 1, 1, 1, 0 },
   { SHADER_IMG_22_BGRA_MASK_NOMUL, &(shader_img_22_bgra_mask_nomul_vert_src), &(shader_img_22_bgra_mask_nomul_frag_src), "img_22_bgra_mask_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 1, 1, 1, 0 },
   { SHADER_IMG_MASK12, &(shader_img_mask12_vert_src), &(shader_img_mask12_frag_src), "img_mask12", SHD_IMAGE, SHD_SAM11, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_IMG_BGRA_MASK12, &(shader_img_bgra_mask12_vert_src), &(shader_img_bgra_mask12_frag_src), "img_bgra_mask12", SHD_IMAGE, SHD_SAM11, SHD_SAM12, 1, 1, 0, 0 },
   { SHADER_IMG_12_MASK12, &(shader_img_12_mask12_vert_src), &(shader_img_12_mask12_frag_src), "img_12_mask12", SHD_IMAGE, SHD_SAM12, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_IMG_21_MASK12, &(shader_img_21_mask12_vert_src), &(shader_img_21_mask12_frag_src), "img_21_mask12", SHD_IMAGE, SHD_SAM21, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_IMG_22_MASK12, &(shader_img_22_mask12_vert_src), &(shader_img_22_mask12_frag_src), "img_22_mask12", SHD_IMAGE, SHD_SAM22, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_IMG_12_BGRA_MASK12, &(shader_img_12_bgra_mask12_vert_src), &(shader_img_12_bgra_mask12_frag_src), "img_12_bgra_mask12", SHD_IMAGE, SHD_SAM12, SHD_SAM12, 1, 1, 0, 0 },
   { SHADER_IMG_21_BGRA_MASK12, &(shader_img_21_bgra_mask12_vert_src), &(shader_img_21_bgra_mask12_frag_src), "img_21_bgra_mask12", SHD_IMAGE, SHD_SAM21, SHD_SAM12, 1, 1, 0, 0 },
   { SHADER_IMG_22_BGRA_MASK12, &(shader_img_22_bgra_mask12_vert_src), &(shader_img_22_bgra_mask12_frag_src), "img_22_bgra_mask12", SHD_IMAGE, SHD_SAM22, SHD_SAM12, 1, 1, 0, 0 },
   { SHADER_IMG_MASK12_NOMUL, &(shader_img_mask12_nomul_vert_src), &(shader_img_mask12_nomul_frag_src), "img_mask12_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM12, 0, 1, 1, 0 },
   { SHADER_IMG_BGRA_MASK12_NOMUL, &(shader_img_bgra_mask12_nomul_vert_src), &(shader_img_bgra_mask12_nomul_frag_src), "img_bgra_mask12_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM12, 1, 1, 1, 0 },
   { SHADER_IMG_12_MASK12_NOMUL, &(shader_img_12_mask12_nomul_vert_src), &(shader_img_12_mask12_nomul_frag_src), "img_12_mask12_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM12, 0, 1, 1, 0 },
   { SHADER_IMG_21_MASK12_NOMUL, &(shader_img_21_mask12_nomul_vert_src), &(shader_img_21_mask12_nomul_frag_src), "img_21_mask12_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM12, 0, 1, 1, 0 },
   { SHADER_IMG_22_MASK12_NOMUL, &(shader_img_22_mask12_nomul_vert_src), &(shader_img_22_mask12_nomul_frag_src), "img_22_mask12_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM12, 0, 1, 1, 0 },
   { SHADER_IMG_12_BGRA_MASK12_NOMUL, &(shader_img_12_bgra_mask12_nomul_vert_src), &(shader_img_12_bgra_mask12_nomul_frag_src), "img_12_bgra_mask12_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM12, 1, 1, 1, 0 },
   { SHADER_IMG_21_BGRA_MASK12_NOMUL, &(shader_img_21_bgra_mask12_nomul_vert_src), &(shader_img_21_bgra_mask12_nomul_frag_src), "img_21_bgra_mask12_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM12, 1, 1, 1, 0 },
   { SHADER_IMG_22_BGRA_MASK12_NOMUL, &(shader_img_22_bgra_mask12_nomul_vert_src), &(shader_img_22_bgra_mask12_nomul_frag_src), "img_22_bgra_mask12_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM12, 1, 1, 1, 0 },
   { SHADER_IMG_MASK21, &(shader_img_mask21_vert_src), &(shader_img_mask21_frag_src), "img_mask21", SHD_IMAGE, SHD_SAM11, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_IMG_BGRA_MASK21, &(shader_img_bgra_mask21_vert_src), &(shader_img_bgra_mask21_frag_src), "img_bgra_mask21", SHD_IMAGE, SHD_SAM11, SHD_SAM21, 1, 1, 0, 0 },
   { SHADER_IMG_12_MASK21, &(shader_img_12_mask21_vert_src), &(shader_img_12_mask21_frag_src), "img_12_mask21", SHD_IMAGE, SHD_SAM12, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_IMG_21_MASK21, &(shader_img_21_mask21_vert_src), &(shader_img_21_mask21_frag_src), "img_21_mask21", SHD_IMAGE, SHD_SAM21, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_IMG_22_MASK21, &(shader_img_22_mask21_vert_src), &(shader_img_22_mask21_frag_src), "img_22_mask21", SHD_IMAGE, SHD_SAM22, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_IMG_12_BGRA_MASK21, &(shader_img_12_bgra_mask21_vert_src), &(shader_img_12_bgra_mask21_frag_src), "img_12_bgra_mask21", SHD_IMAGE, SHD_SAM12, SHD_SAM21, 1, 1, 0, 0 },
   { SHADER_IMG_21_BGRA_MASK21, &(shader_img_21_bgra_mask21_vert_src), &(shader_img_21_bgra_mask21_frag_src), "img_21_bgra_mask21", SHD_IMAGE, SHD_SAM21, SHD_SAM21, 1, 1, 0, 0 },
   { SHADER_IMG_22_BGRA_MASK21, &(shader_img_22_bgra_mask21_vert_src), &(shader_img_22_bgra_mask21_frag_src), "img_22_bgra_mask21", SHD_IMAGE, SHD_SAM22, SHD_SAM21, 1, 1, 0, 0 },
   { SHADER_IMG_MASK21_NOMUL, &(shader_img_mask21_nomul_vert_src), &(shader_img_mask21_nomul_frag_src), "img_mask21_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM21, 0, 1, 1, 0 },
   { SHADER_IMG_BGRA_MASK21_NOMUL, &(shader_img_bgra_mask21_nomul_vert_src), &(shader_img_bgra_mask21_nomul_frag_src), "img_bgra_mask21_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM21, 1, 1, 1, 0 },
   { SHADER_IMG_12_MASK21_NOMUL, &(shader_img_12_mask21_nomul_vert_src), &(shader_img_12_mask21_nomul_frag_src), "img_12_mask21_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM21, 0, 1, 1, 0 },
   { SHADER_IMG_21_MASK21_NOMUL, &(shader_img_21_mask21_nomul_vert_src), &(shader_img_21_mask21_nomul_frag_src), "img_21_mask21_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM21, 0, 1, 1, 0 },
   { SHADER_IMG_22_MASK21_NOMUL, &(shader_img_22_mask21_nomul_vert_src), &(shader_img_22_mask21_nomul_frag_src), "img_22_mask21_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM21, 0, 1, 1, 0 },
   { SHADER_IMG_12_BGRA_MASK21_NOMUL, &(shader_img_12_bgra_mask21_nomul_vert_src), &(shader_img_12_bgra_mask21_nomul_frag_src), "img_12_bgra_mask21_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM21, 1, 1, 1, 0 },
   { SHADER_IMG_21_BGRA_MASK21_NOMUL, &(shader_img_21_bgra_mask21_nomul_vert_src), &(shader_img_21_bgra_mask21_nomul_frag_src), "img_21_bgra_mask21_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM21, 1, 1, 1, 0 },
   { SHADER_IMG_22_BGRA_MASK21_NOMUL, &(shader_img_22_bgra_mask21_nomul_vert_src), &(shader_img_22_bgra_mask21_nomul_frag_src), "img_22_bgra_mask21_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM21, 1, 1, 1, 0 },
   { SHADER_IMG_MASK22, &(shader_img_mask22_vert_src), &(shader_img_mask22_frag_src), "img_mask22", SHD_IMAGE, SHD_SAM11, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_IMG_BGRA_MASK22, &(shader_img_bgra_mask22_vert_src), &(shader_img_bgra_mask22_frag_src), "img_bgra_mask22", SHD_IMAGE, SHD_SAM11, SHD_SAM22, 1, 1, 0, 0 },
   { SHADER_IMG_12_MASK22, &(shader_img_12_mask22_vert_src), &(shader_img_12_mask22_frag_src), "img_12_mask22", SHD_IMAGE, SHD_SAM12, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_IMG_21_MASK22, &(shader_img_21_mask22_vert_src), &(shader_img_21_mask22_frag_src), "img_21_mask22", SHD_IMAGE, SHD_SAM21, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_IMG_22_MASK22, &(shader_img_22_mask22_vert_src), &(shader_img_22_mask22_frag_src), "img_22_mask22", SHD_IMAGE, SHD_SAM22, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_IMG_12_BGRA_MASK22, &(shader_img_12_bgra_mask22_vert_src), &(shader_img_12_bgra_mask22_frag_src), "img_12_bgra_mask22", SHD_IMAGE, SHD_SAM12, SHD_SAM22, 1, 1, 0, 0 },
   { SHADER_IMG_21_BGRA_MASK22, &(shader_img_21_bgra_mask22_vert_src), &(shader_img_21_bgra_mask22_frag_src), "img_21_bgra_mask22", SHD_IMAGE, SHD_SAM21, SHD_SAM22, 1, 1, 0, 0 },
   { SHADER_IMG_22_BGRA_MASK22, &(shader_img_22_bgra_mask22_vert_src), &(shader_img_22_bgra_mask22_frag_src), "img_22_bgra_mask22", SHD_IMAGE, SHD_SAM22, SHD_SAM22, 1, 1, 0, 0 },
   { SHADER_IMG_MASK22_NOMUL, &(shader_img_mask22_nomul_vert_src), &(shader_img_mask22_nomul_frag_src), "img_mask22_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM22, 0, 1, 1, 0 },
   { SHADER_IMG_BGRA_MASK22_NOMUL, &(shader_img_bgra_mask22_nomul_vert_src), &(shader_img_bgra_mask22_nomul_frag_src), "img_bgra_mask22_nomul", SHD_IMAGE, SHD_SAM11, SHD_SAM22, 1, 1, 1, 0 },
   { SHADER_IMG_12_MASK22_NOMUL, &(shader_img_12_mask22_nomul_vert_src), &(shader_img_12_mask22_nomul_frag_src), "img_12_mask22_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM22, 0, 1, 1, 0 },
   { SHADER_IMG_21_MASK22_NOMUL, &(shader_img_21_mask22_nomul_vert_src), &(shader_img_21_mask22_nomul_frag_src), "img_21_mask22_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM22, 0, 1, 1, 0 },
   { SHADER_IMG_22_MASK22_NOMUL, &(shader_img_22_mask22_nomul_vert_src), &(shader_img_22_mask22_nomul_frag_src), "img_22_mask22_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM22, 0, 1, 1, 0 },
   { SHADER_IMG_12_BGRA_MASK22_NOMUL, &(shader_img_12_bgra_mask22_nomul_vert_src), &(shader_img_12_bgra_mask22_nomul_frag_src), "img_12_bgra_mask22_nomul", SHD_IMAGE, SHD_SAM12, SHD_SAM22, 1, 1, 1, 0 },
   { SHADER_IMG_21_BGRA_MASK22_NOMUL, &(shader_img_21_bgra_mask22_nomul_vert_src), &(shader_img_21_bgra_mask22_nomul_frag_src), "img_21_bgra_mask22_nomul", SHD_IMAGE, SHD_SAM21, SHD_SAM22, 1, 1, 1, 0 },
   { SHADER_IMG_22_BGRA_MASK22_NOMUL, &(shader_img_22_bgra_mask22_nomul_vert_src), &(shader_img_22_bgra_mask22_nomul_frag_src), "img_22_bgra_mask22_nomul", SHD_IMAGE, SHD_SAM22, SHD_SAM22, 1, 1, 1, 0 },
   { SHADER_IMG_AFILL, &(shader_img_afill_vert_src), &(shader_img_afill_frag_src), "img_afill", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 0, 0, 0, 1 },
   { SHADER_IMG_BGRA_AFILL, &(shader_img_bgra_afill_vert_src), &(shader_img_bgra_afill_frag_src), "img_bgra_afill", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 1, 0, 0, 1 },
   { SHADER_IMG_NOMUL_AFILL, &(shader_img_nomul_afill_vert_src), &(shader_img_nomul_afill_frag_src), "img_nomul_afill", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 0, 0, 1, 1 },
   { SHADER_IMG_BGRA_NOMUL_AFILL, &(shader_img_bgra_nomul_afill_vert_src), &(shader_img_bgra_nomul_afill_frag_src), "img_bgra_nomul_afill", SHD_IMAGE, SHD_SAM11, SHD_SAM11, 1, 0, 1, 1 },
   { SHADER_IMG_12_AFILL, &(shader_img_12_afill_vert_src), &(shader_img_12_afill_frag_src), "img_12_afill", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 0, 0, 0, 1 },
   { SHADER_IMG_21_AFILL, &(shader_img_21_afill_vert_src), &(shader_img_21_afill_frag_src), "img_21_afill", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 0, 0, 0, 1 },
   { SHADER_IMG_22_AFILL, &(shader_img_22_afill_vert_src), &(shader_img_22_afill_frag_src), "img_22_afill", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 0, 0, 0, 1 },
   { SHADER_IMG_12_BGRA_AFILL, &(shader_img_12_bgra_afill_vert_src), &(shader_img_12_bgra_afill_frag_src), "img_12_bgra_afill", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 1, 0, 0, 1 },
   { SHADER_IMG_21_BGRA_AFILL, &(shader_img_21_bgra_afill_vert_src), &(shader_img_21_bgra_afill_frag_src), "img_21_bgra_afill", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 1, 0, 0, 1 },
   { SHADER_IMG_22_BGRA_AFILL, &(shader_img_22_bgra_afill_vert_src), &(shader_img_22_bgra_afill_frag_src), "img_22_bgra_afill", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 1, 0, 0, 1 },
   { SHADER_IMG_12_NOMUL_AFILL, &(shader_img_12_nomul_afill_vert_src), &(shader_img_12_nomul_afill_frag_src), "img_12_nomul_afill", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 0, 0, 1, 1 },
   { SHADER_IMG_21_NOMUL_AFILL, &(shader_img_21_nomul_afill_vert_src), &(shader_img_21_nomul_afill_frag_src), "img_21_nomul_afill", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 0, 0, 1, 1 },
   { SHADER_IMG_22_NOMUL_AFILL, &(shader_img_22_nomul_afill_vert_src), &(shader_img_22_nomul_afill_frag_src), "img_22_nomul_afill", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 0, 0, 1, 1 },
   { SHADER_IMG_12_BGRA_NOMUL_AFILL, &(shader_img_12_bgra_nomul_afill_vert_src), &(shader_img_12_bgra_nomul_afill_frag_src), "img_12_bgra_nomul_afill", SHD_IMAGE, SHD_SAM12, SHD_SAM11, 1, 0, 1, 1 },
   { SHADER_IMG_21_BGRA_NOMUL_AFILL, &(shader_img_21_bgra_nomul_afill_vert_src), &(shader_img_21_bgra_nomul_afill_frag_src), "img_21_bgra_nomul_afill", SHD_IMAGE, SHD_SAM21, SHD_SAM11, 1, 0, 1, 1 },
   { SHADER_IMG_22_BGRA_NOMUL_AFILL, &(shader_img_22_bgra_nomul_afill_vert_src), &(shader_img_22_bgra_nomul_afill_frag_src), "img_22_bgra_nomul_afill", SHD_IMAGE, SHD_SAM22, SHD_SAM11, 1, 0, 1, 1 },
   { SHADER_IMGNAT, &(shader_imgnat_vert_src), &(shader_imgnat_frag_src), "imgnat", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_IMGNAT_BGRA, &(shader_imgnat_bgra_vert_src), &(shader_imgnat_bgra_frag_src), "imgnat_bgra", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 1, 0, 0, 0 },
   { SHADER_IMGNAT_12, &(shader_imgnat_12_vert_src), &(shader_imgnat_12_frag_src), "imgnat_12", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_IMGNAT_21, &(shader_imgnat_21_vert_src), &(shader_imgnat_21_frag_src), "imgnat_21", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_IMGNAT_22, &(shader_imgnat_22_vert_src), &(shader_imgnat_22_frag_src), "imgnat_22", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_IMGNAT_12_BGRA, &(shader_imgnat_12_bgra_vert_src), &(shader_imgnat_12_bgra_frag_src), "imgnat_12_bgra", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 1, 0, 0, 0 },
   { SHADER_IMGNAT_21_BGRA, &(shader_imgnat_21_bgra_vert_src), &(shader_imgnat_21_bgra_frag_src), "imgnat_21_bgra", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 1, 0, 0, 0 },
   { SHADER_IMGNAT_22_BGRA, &(shader_imgnat_22_bgra_vert_src), &(shader_imgnat_22_bgra_frag_src), "imgnat_22_bgra", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 1, 0, 0, 0 },
   { SHADER_IMGNAT_MASK, &(shader_imgnat_mask_vert_src), &(shader_imgnat_mask_frag_src), "imgnat_mask", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_IMGNAT_BGRA_MASK, &(shader_imgnat_bgra_mask_vert_src), &(shader_imgnat_bgra_mask_frag_src), "imgnat_bgra_mask", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 1, 1, 0, 0 },
   { SHADER_IMGNAT_12_MASK, &(shader_imgnat_12_mask_vert_src), &(shader_imgnat_12_mask_frag_src), "imgnat_12_mask", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_IMGNAT_21_MASK, &(shader_imgnat_21_mask_vert_src), &(shader_imgnat_21_mask_frag_src), "imgnat_21_mask", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_IMGNAT_22_MASK, &(shader_imgnat_22_mask_vert_src), &(shader_imgnat_22_mask_frag_src), "imgnat_22_mask", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_IMGNAT_12_BGRA_MASK, &(shader_imgnat_12_bgra_mask_vert_src), &(shader_imgnat_12_bgra_mask_frag_src), "imgnat_12_bgra_mask", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 1, 1, 0, 0 },
   { SHADER_IMGNAT_21_BGRA_MASK, &(shader_imgnat_21_bgra_mask_vert_src), &(shader_imgnat_21_bgra_mask_frag_src), "imgnat_21_bgra_mask", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 1, 1, 0, 0 },
   { SHADER_IMGNAT_22_BGRA_MASK, &(shader_imgnat_22_bgra_mask_vert_src), &(shader_imgnat_22_bgra_mask_frag_src), "imgnat_22_bgra_mask", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 1, 1, 0, 0 },
   { SHADER_IMGNAT_NOMUL, &(shader_imgnat_nomul_vert_src), &(shader_imgnat_nomul_frag_src), "imgnat_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_IMGNAT_BGRA_NOMUL, &(shader_imgnat_bgra_nomul_vert_src), &(shader_imgnat_bgra_nomul_frag_src), "imgnat_bgra_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 1, 0, 1, 0 },
   { SHADER_IMGNAT_12_NOMUL, &(shader_imgnat_12_nomul_vert_src), &(shader_imgnat_12_nomul_frag_src), "imgnat_12_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_IMGNAT_21_NOMUL, &(shader_imgnat_21_nomul_vert_src), &(shader_imgnat_21_nomul_frag_src), "imgnat_21_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_IMGNAT_22_NOMUL, &(shader_imgnat_22_nomul_vert_src), &(shader_imgnat_22_nomul_frag_src), "imgnat_22_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_IMGNAT_12_BGRA_NOMUL, &(shader_imgnat_12_bgra_nomul_vert_src), &(shader_imgnat_12_bgra_nomul_frag_src), "imgnat_12_bgra_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 1, 0, 1, 0 },
   { SHADER_IMGNAT_21_BGRA_NOMUL, &(shader_imgnat_21_bgra_nomul_vert_src), &(shader_imgnat_21_bgra_nomul_frag_src), "imgnat_21_bgra_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 1, 0, 1, 0 },
   { SHADER_IMGNAT_22_BGRA_NOMUL, &(shader_imgnat_22_bgra_nomul_vert_src), &(shader_imgnat_22_bgra_nomul_frag_src), "imgnat_22_bgra_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 1, 0, 1, 0 },
   { SHADER_IMGNAT_MASK_NOMUL, &(shader_imgnat_mask_nomul_vert_src), &(shader_imgnat_mask_nomul_frag_src), "imgnat_mask_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_IMGNAT_BGRA_MASK_NOMUL, &(shader_imgnat_bgra_mask_nomul_vert_src), &(shader_imgnat_bgra_mask_nomul_frag_src), "imgnat_bgra_mask_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 1, 1, 1, 0 },
   { SHADER_IMGNAT_12_MASK_NOMUL, &(shader_imgnat_12_mask_nomul_vert_src), &(shader_imgnat_12_mask_nomul_frag_src), "imgnat_12_mask_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_IMGNAT_21_MASK_NOMUL, &(shader_imgnat_21_mask_nomul_vert_src), &(shader_imgnat_21_mask_nomul_frag_src), "imgnat_21_mask_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_IMGNAT_22_MASK_NOMUL, &(shader_imgnat_22_mask_nomul_vert_src), &(shader_imgnat_22_mask_nomul_frag_src), "imgnat_22_mask_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_IMGNAT_12_BGRA_MASK_NOMUL, &(shader_imgnat_12_bgra_mask_nomul_vert_src), &(shader_imgnat_12_bgra_mask_nomul_frag_src), "imgnat_12_bgra_mask_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 1, 1, 1, 0 },
   { SHADER_IMGNAT_21_BGRA_MASK_NOMUL, &(shader_imgnat_21_bgra_mask_nomul_vert_src), &(shader_imgnat_21_bgra_mask_nomul_frag_src), "imgnat_21_bgra_mask_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 1, 1, 1, 0 },
   { SHADER_IMGNAT_22_BGRA_MASK_NOMUL, &(shader_imgnat_22_bgra_mask_nomul_vert_src), &(shader_imgnat_22_bgra_mask_nomul_frag_src), "imgnat_22_bgra_mask_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 1, 1, 1, 0 },
   { SHADER_IMGNAT_MASK12, &(shader_imgnat_mask12_vert_src), &(shader_imgnat_mask12_frag_src), "imgnat_mask12", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_IMGNAT_BGRA_MASK12, &(shader_imgnat_bgra_mask12_vert_src), &(shader_imgnat_bgra_mask12_frag_src), "imgnat_bgra_mask12", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM12, 1, 1, 0, 0 },
   { SHADER_IMGNAT_12_MASK12, &(shader_imgnat_12_mask12_vert_src), &(shader_imgnat_12_mask12_frag_src), "imgnat_12_mask12", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_IMGNAT_21_MASK12, &(shader_imgnat_21_mask12_vert_src), &(shader_imgnat_21_mask12_frag_src), "imgnat_21_mask12", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_IMGNAT_22_MASK12, &(shader_imgnat_22_mask12_vert_src), &(shader_imgnat_22_mask12_frag_src), "imgnat_22_mask12", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM12, 0, 1, 0, 0 },
   { SHADER_IMGNAT_12_BGRA_MASK12, &(shader_imgnat_12_bgra_mask12_vert_src), &(shader_imgnat_12_bgra_mask12_frag_src), "imgnat_12_bgra_mask12", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM12, 1, 1, 0, 0 },
   { SHADER_IMGNAT_21_BGRA_MASK12, &(shader_imgnat_21_bgra_mask12_vert_src), &(shader_imgnat_21_bgra_mask12_frag_src), "imgnat_21_bgra_mask12", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM12, 1, 1, 0, 0 },
   { SHADER_IMGNAT_22_BGRA_MASK12, &(shader_imgnat_22_bgra_mask12_vert_src), &(shader_imgnat_22_bgra_mask12_frag_src), "imgnat_22_bgra_mask12", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM12, 1, 1, 0, 0 },
   { SHADER_IMGNAT_MASK12_NOMUL, &(shader_imgnat_mask12_nomul_vert_src), &(shader_imgnat_mask12_nomul_frag_src), "imgnat_mask12_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM12, 0, 1, 1, 0 },
   { SHADER_IMGNAT_BGRA_MASK12_NOMUL, &(shader_imgnat_bgra_mask12_nomul_vert_src), &(shader_imgnat_bgra_mask12_nomul_frag_src), "imgnat_bgra_mask12_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM12, 1, 1, 1, 0 },
   { SHADER_IMGNAT_12_MASK12_NOMUL, &(shader_imgnat_12_mask12_nomul_vert_src), &(shader_imgnat_12_mask12_nomul_frag_src), "imgnat_12_mask12_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM12, 0, 1, 1, 0 },
   { SHADER_IMGNAT_21_MASK12_NOMUL, &(shader_imgnat_21_mask12_nomul_vert_src), &(shader_imgnat_21_mask12_nomul_frag_src), "imgnat_21_mask12_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM12, 0, 1, 1, 0 },
   { SHADER_IMGNAT_22_MASK12_NOMUL, &(shader_imgnat_22_mask12_nomul_vert_src), &(shader_imgnat_22_mask12_nomul_frag_src), "imgnat_22_mask12_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM12, 0, 1, 1, 0 },
   { SHADER_IMGNAT_12_BGRA_MASK12_NOMUL, &(shader_imgnat_12_bgra_mask12_nomul_vert_src), &(shader_imgnat_12_bgra_mask12_nomul_frag_src), "imgnat_12_bgra_mask12_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM12, 1, 1, 1, 0 },
   { SHADER_IMGNAT_21_BGRA_MASK12_NOMUL, &(shader_imgnat_21_bgra_mask12_nomul_vert_src), &(shader_imgnat_21_bgra_mask12_nomul_frag_src), "imgnat_21_bgra_mask12_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM12, 1, 1, 1, 0 },
   { SHADER_IMGNAT_22_BGRA_MASK12_NOMUL, &(shader_imgnat_22_bgra_mask12_nomul_vert_src), &(shader_imgnat_22_bgra_mask12_nomul_frag_src), "imgnat_22_bgra_mask12_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM12, 1, 1, 1, 0 },
   { SHADER_IMGNAT_MASK21, &(shader_imgnat_mask21_vert_src), &(shader_imgnat_mask21_frag_src), "imgnat_mask21", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_IMGNAT_BGRA_MASK21, &(shader_imgnat_bgra_mask21_vert_src), &(shader_imgnat_bgra_mask21_frag_src), "imgnat_bgra_mask21", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM21, 1, 1, 0, 0 },
   { SHADER_IMGNAT_12_MASK21, &(shader_imgnat_12_mask21_vert_src), &(shader_imgnat_12_mask21_frag_src), "imgnat_12_mask21", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_IMGNAT_21_MASK21, &(shader_imgnat_21_mask21_vert_src), &(shader_imgnat_21_mask21_frag_src), "imgnat_21_mask21", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_IMGNAT_22_MASK21, &(shader_imgnat_22_mask21_vert_src), &(shader_imgnat_22_mask21_frag_src), "imgnat_22_mask21", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM21, 0, 1, 0, 0 },
   { SHADER_IMGNAT_12_BGRA_MASK21, &(shader_imgnat_12_bgra_mask21_vert_src), &(shader_imgnat_12_bgra_mask21_frag_src), "imgnat_12_bgra_mask21", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM21, 1, 1, 0, 0 },
   { SHADER_IMGNAT_21_BGRA_MASK21, &(shader_imgnat_21_bgra_mask21_vert_src), &(shader_imgnat_21_bgra_mask21_frag_src), "imgnat_21_bgra_mask21", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM21, 1, 1, 0, 0 },
   { SHADER_IMGNAT_22_BGRA_MASK21, &(shader_imgnat_22_bgra_mask21_vert_src), &(shader_imgnat_22_bgra_mask21_frag_src), "imgnat_22_bgra_mask21", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM21, 1, 1, 0, 0 },
   { SHADER_IMGNAT_MASK21_NOMUL, &(shader_imgnat_mask21_nomul_vert_src), &(shader_imgnat_mask21_nomul_frag_src), "imgnat_mask21_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM21, 0, 1, 1, 0 },
   { SHADER_IMGNAT_BGRA_MASK21_NOMUL, &(shader_imgnat_bgra_mask21_nomul_vert_src), &(shader_imgnat_bgra_mask21_nomul_frag_src), "imgnat_bgra_mask21_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM21, 1, 1, 1, 0 },
   { SHADER_IMGNAT_12_MASK21_NOMUL, &(shader_imgnat_12_mask21_nomul_vert_src), &(shader_imgnat_12_mask21_nomul_frag_src), "imgnat_12_mask21_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM21, 0, 1, 1, 0 },
   { SHADER_IMGNAT_21_MASK21_NOMUL, &(shader_imgnat_21_mask21_nomul_vert_src), &(shader_imgnat_21_mask21_nomul_frag_src), "imgnat_21_mask21_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM21, 0, 1, 1, 0 },
   { SHADER_IMGNAT_22_MASK21_NOMUL, &(shader_imgnat_22_mask21_nomul_vert_src), &(shader_imgnat_22_mask21_nomul_frag_src), "imgnat_22_mask21_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM21, 0, 1, 1, 0 },
   { SHADER_IMGNAT_12_BGRA_MASK21_NOMUL, &(shader_imgnat_12_bgra_mask21_nomul_vert_src), &(shader_imgnat_12_bgra_mask21_nomul_frag_src), "imgnat_12_bgra_mask21_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM21, 1, 1, 1, 0 },
   { SHADER_IMGNAT_21_BGRA_MASK21_NOMUL, &(shader_imgnat_21_bgra_mask21_nomul_vert_src), &(shader_imgnat_21_bgra_mask21_nomul_frag_src), "imgnat_21_bgra_mask21_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM21, 1, 1, 1, 0 },
   { SHADER_IMGNAT_22_BGRA_MASK21_NOMUL, &(shader_imgnat_22_bgra_mask21_nomul_vert_src), &(shader_imgnat_22_bgra_mask21_nomul_frag_src), "imgnat_22_bgra_mask21_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM21, 1, 1, 1, 0 },
   { SHADER_IMGNAT_MASK22, &(shader_imgnat_mask22_vert_src), &(shader_imgnat_mask22_frag_src), "imgnat_mask22", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_IMGNAT_BGRA_MASK22, &(shader_imgnat_bgra_mask22_vert_src), &(shader_imgnat_bgra_mask22_frag_src), "imgnat_bgra_mask22", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM22, 1, 1, 0, 0 },
   { SHADER_IMGNAT_12_MASK22, &(shader_imgnat_12_mask22_vert_src), &(shader_imgnat_12_mask22_frag_src), "imgnat_12_mask22", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_IMGNAT_21_MASK22, &(shader_imgnat_21_mask22_vert_src), &(shader_imgnat_21_mask22_frag_src), "imgnat_21_mask22", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_IMGNAT_22_MASK22, &(shader_imgnat_22_mask22_vert_src), &(shader_imgnat_22_mask22_frag_src), "imgnat_22_mask22", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM22, 0, 1, 0, 0 },
   { SHADER_IMGNAT_12_BGRA_MASK22, &(shader_imgnat_12_bgra_mask22_vert_src), &(shader_imgnat_12_bgra_mask22_frag_src), "imgnat_12_bgra_mask22", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM22, 1, 1, 0, 0 },
   { SHADER_IMGNAT_21_BGRA_MASK22, &(shader_imgnat_21_bgra_mask22_vert_src), &(shader_imgnat_21_bgra_mask22_frag_src), "imgnat_21_bgra_mask22", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM22, 1, 1, 0, 0 },
   { SHADER_IMGNAT_22_BGRA_MASK22, &(shader_imgnat_22_bgra_mask22_vert_src), &(shader_imgnat_22_bgra_mask22_frag_src), "imgnat_22_bgra_mask22", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM22, 1, 1, 0, 0 },
   { SHADER_IMGNAT_MASK22_NOMUL, &(shader_imgnat_mask22_nomul_vert_src), &(shader_imgnat_mask22_nomul_frag_src), "imgnat_mask22_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM22, 0, 1, 1, 0 },
   { SHADER_IMGNAT_BGRA_MASK22_NOMUL, &(shader_imgnat_bgra_mask22_nomul_vert_src), &(shader_imgnat_bgra_mask22_nomul_frag_src), "imgnat_bgra_mask22_nomul", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM22, 1, 1, 1, 0 },
   { SHADER_IMGNAT_12_MASK22_NOMUL, &(shader_imgnat_12_mask22_nomul_vert_src), &(shader_imgnat_12_mask22_nomul_frag_src), "imgnat_12_mask22_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM22, 0, 1, 1, 0 },
   { SHADER_IMGNAT_21_MASK22_NOMUL, &(shader_imgnat_21_mask22_nomul_vert_src), &(shader_imgnat_21_mask22_nomul_frag_src), "imgnat_21_mask22_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM22, 0, 1, 1, 0 },
   { SHADER_IMGNAT_22_MASK22_NOMUL, &(shader_imgnat_22_mask22_nomul_vert_src), &(shader_imgnat_22_mask22_nomul_frag_src), "imgnat_22_mask22_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM22, 0, 1, 1, 0 },
   { SHADER_IMGNAT_12_BGRA_MASK22_NOMUL, &(shader_imgnat_12_bgra_mask22_nomul_vert_src), &(shader_imgnat_12_bgra_mask22_nomul_frag_src), "imgnat_12_bgra_mask22_nomul", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM22, 1, 1, 1, 0 },
   { SHADER_IMGNAT_21_BGRA_MASK22_NOMUL, &(shader_imgnat_21_bgra_mask22_nomul_vert_src), &(shader_imgnat_21_bgra_mask22_nomul_frag_src), "imgnat_21_bgra_mask22_nomul", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM22, 1, 1, 1, 0 },
   { SHADER_IMGNAT_22_BGRA_MASK22_NOMUL, &(shader_imgnat_22_bgra_mask22_nomul_vert_src), &(shader_imgnat_22_bgra_mask22_nomul_frag_src), "imgnat_22_bgra_mask22_nomul", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM22, 1, 1, 1, 0 },
   { SHADER_IMGNAT_AFILL, &(shader_imgnat_afill_vert_src), &(shader_imgnat_afill_frag_src), "imgnat_afill", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 0, 0, 0, 1 },
   { SHADER_IMGNAT_BGRA_AFILL, &(shader_imgnat_bgra_afill_vert_src), &(shader_imgnat_bgra_afill_frag_src), "imgnat_bgra_afill", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 1, 0, 0, 1 },
   { SHADER_IMGNAT_NOMUL_AFILL, &(shader_imgnat_nomul_afill_vert_src), &(shader_imgnat_nomul_afill_frag_src), "imgnat_nomul_afill", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 0, 0, 1, 1 },
   { SHADER_IMGNAT_BGRA_NOMUL_AFILL, &(shader_imgnat_bgra_nomul_afill_vert_src), &(shader_imgnat_bgra_nomul_afill_frag_src), "imgnat_bgra_nomul_afill", SHD_IMAGENATIVE, SHD_SAM11, SHD_SAM11, 1, 0, 1, 1 },
   { SHADER_IMGNAT_12_AFILL, &(shader_imgnat_12_afill_vert_src), &(shader_imgnat_12_afill_frag_src), "imgnat_12_afill", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 0, 0, 0, 1 },
   { SHADER_IMGNAT_21_AFILL, &(shader_imgnat_21_afill_vert_src), &(shader_imgnat_21_afill_frag_src), "imgnat_21_afill", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 0, 0, 0, 1 },
   { SHADER_IMGNAT_22_AFILL, &(shader_imgnat_22_afill_vert_src), &(shader_imgnat_22_afill_frag_src), "imgnat_22_afill", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 0, 0, 0, 1 },
   { SHADER_IMGNAT_12_BGRA_AFILL, &(shader_imgnat_12_bgra_afill_vert_src), &(shader_imgnat_12_bgra_afill_frag_src), "imgnat_12_bgra_afill", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 1, 0, 0, 1 },
   { SHADER_IMGNAT_21_BGRA_AFILL, &(shader_imgnat_21_bgra_afill_vert_src), &(shader_imgnat_21_bgra_afill_frag_src), "imgnat_21_bgra_afill", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 1, 0, 0, 1 },
   { SHADER_IMGNAT_22_BGRA_AFILL, &(shader_imgnat_22_bgra_afill_vert_src), &(shader_imgnat_22_bgra_afill_frag_src), "imgnat_22_bgra_afill", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 1, 0, 0, 1 },
   { SHADER_IMGNAT_12_NOMUL_AFILL, &(shader_imgnat_12_nomul_afill_vert_src), &(shader_imgnat_12_nomul_afill_frag_src), "imgnat_12_nomul_afill", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 0, 0, 1, 1 },
   { SHADER_IMGNAT_21_NOMUL_AFILL, &(shader_imgnat_21_nomul_afill_vert_src), &(shader_imgnat_21_nomul_afill_frag_src), "imgnat_21_nomul_afill", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 0, 0, 1, 1 },
   { SHADER_IMGNAT_22_NOMUL_AFILL, &(shader_imgnat_22_nomul_afill_vert_src), &(shader_imgnat_22_nomul_afill_frag_src), "imgnat_22_nomul_afill", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 0, 0, 1, 1 },
   { SHADER_IMGNAT_12_BGRA_NOMUL_AFILL, &(shader_imgnat_12_bgra_nomul_afill_vert_src), &(shader_imgnat_12_bgra_nomul_afill_frag_src), "imgnat_12_bgra_nomul_afill", SHD_IMAGENATIVE, SHD_SAM12, SHD_SAM11, 1, 0, 1, 1 },
   { SHADER_IMGNAT_21_BGRA_NOMUL_AFILL, &(shader_imgnat_21_bgra_nomul_afill_vert_src), &(shader_imgnat_21_bgra_nomul_afill_frag_src), "imgnat_21_bgra_nomul_afill", SHD_IMAGENATIVE, SHD_SAM21, SHD_SAM11, 1, 0, 1, 1 },
   { SHADER_IMGNAT_22_BGRA_NOMUL_AFILL, &(shader_imgnat_22_bgra_nomul_afill_vert_src), &(shader_imgnat_22_bgra_nomul_afill_frag_src), "imgnat_22_bgra_nomul_afill", SHD_IMAGENATIVE, SHD_SAM22, SHD_SAM11, 1, 0, 1, 1 },
   { SHADER_RGB_A_PAIR, &(shader_rgb_a_pair_vert_src), &(shader_rgb_a_pair_frag_src), "rgb_a_pair", SHD_RGB_A_PAIR, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_RGB_A_PAIR_MASK, &(shader_rgb_a_pair_mask_vert_src), &(shader_rgb_a_pair_mask_frag_src), "rgb_a_pair_mask", SHD_RGB_A_PAIR, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_RGB_A_PAIR_NOMUL, &(shader_rgb_a_pair_nomul_vert_src), &(shader_rgb_a_pair_nomul_frag_src), "rgb_a_pair_nomul", SHD_RGB_A_PAIR, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_RGB_A_PAIR_MASK_NOMUL, &(shader_rgb_a_pair_mask_nomul_vert_src), &(shader_rgb_a_pair_mask_nomul_frag_src), "rgb_a_pair_mask_nomul", SHD_RGB_A_PAIR, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_TEX_EXTERNAL, &(shader_tex_external_vert_src), &(shader_tex_external_frag_src), "tex_external", SHD_TEX_EXTERNAL, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_TEX_EXTERNAL_AFILL, &(shader_tex_external_afill_vert_src), &(shader_tex_external_afill_frag_src), "tex_external_afill", SHD_TEX_EXTERNAL, SHD_SAM11, SHD_SAM11, 0, 0, 0, 1 },
   { SHADER_TEX_EXTERNAL_NOMUL, &(shader_tex_external_nomul_vert_src), &(shader_tex_external_nomul_frag_src), "tex_external_nomul", SHD_TEX_EXTERNAL, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_TEX_EXTERNAL_NOMUL_AFILL, &(shader_tex_external_nomul_afill_vert_src), &(shader_tex_external_nomul_afill_frag_src), "tex_external_nomul_afill", SHD_TEX_EXTERNAL, SHD_SAM11, SHD_SAM11, 0, 0, 1, 1 },
   { SHADER_TEX_EXTERNAL_MASK, &(shader_tex_external_mask_vert_src), &(shader_tex_external_mask_frag_src), "tex_external_mask", SHD_TEX_EXTERNAL, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_TEX_EXTERNAL_MASK_NOMUL, &(shader_tex_external_mask_nomul_vert_src), &(shader_tex_external_mask_nomul_frag_src), "tex_external_mask_nomul", SHD_TEX_EXTERNAL, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_YUV, &(shader_yuv_vert_src), &(shader_yuv_frag_src), "yuv", SHD_YUV, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_YUV_NOMUL, &(shader_yuv_nomul_vert_src), &(shader_yuv_nomul_frag_src), "yuv_nomul", SHD_YUV, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_YUV_MASK, &(shader_yuv_mask_vert_src), &(shader_yuv_mask_frag_src), "yuv_mask", SHD_YUV, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_YUV_MASK_NOMUL, &(shader_yuv_mask_nomul_vert_src), &(shader_yuv_mask_nomul_frag_src), "yuv_mask_nomul", SHD_YUV, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_YUY2, &(shader_yuy2_vert_src), &(shader_yuy2_frag_src), "yuy2", SHD_YUY2, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_YUY2_NOMUL, &(shader_yuy2_nomul_vert_src), &(shader_yuy2_nomul_frag_src), "yuy2_nomul", SHD_YUY2, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_YUY2_MASK, &(shader_yuy2_mask_vert_src), &(shader_yuy2_mask_frag_src), "yuy2_mask", SHD_YUY2, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_YUY2_MASK_NOMUL, &(shader_yuy2_mask_nomul_vert_src), &(shader_yuy2_mask_nomul_frag_src), "yuy2_mask_nomul", SHD_YUY2, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_NV12, &(shader_nv12_vert_src), &(shader_nv12_frag_src), "nv12", SHD_NV12, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_NV12_NOMUL, &(shader_nv12_nomul_vert_src), &(shader_nv12_nomul_frag_src), "nv12_nomul", SHD_NV12, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_NV12_MASK, &(shader_nv12_mask_vert_src), &(shader_nv12_mask_frag_src), "nv12_mask", SHD_NV12, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_NV12_MASK_NOMUL, &(shader_nv12_mask_nomul_vert_src), &(shader_nv12_mask_nomul_frag_src), "nv12_mask_nomul", SHD_NV12, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_YUV_709, &(shader_yuv_709_vert_src), &(shader_yuv_709_frag_src), "yuv_709", SHD_YUV, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_YUV_709_NOMUL, &(shader_yuv_709_nomul_vert_src), &(shader_yuv_709_nomul_frag_src), "yuv_709_nomul", SHD_YUV, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_YUV_709_MASK, &(shader_yuv_709_mask_vert_src), &(shader_yuv_709_mask_frag_src), "yuv_709_mask", SHD_YUV, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_YUV_709_MASK_NOMUL, &(shader_yuv_709_mask_nomul_vert_src), &(shader_yuv_709_mask_nomul_frag_src), "yuv_709_mask_nomul", SHD_YUV, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_YUY2_709, &(shader_yuy2_709_vert_src), &(shader_yuy2_709_frag_src), "yuy2_709", SHD_YUY2, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_YUY2_709_NOMUL, &(shader_yuy2_709_nomul_vert_src), &(shader_yuy2_709_nomul_frag_src), "yuy2_709_nomul", SHD_YUY2, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_YUY2_709_MASK, &(shader_yuy2_709_mask_vert_src), &(shader_yuy2_709_mask_frag_src), "yuy2_709_mask", SHD_YUY2, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_YUY2_709_MASK_NOMUL, &(shader_yuy2_709_mask_nomul_vert_src), &(shader_yuy2_709_mask_nomul_frag_src), "yuy2_709_mask_nomul", SHD_YUY2, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
   { SHADER_NV12_709, &(shader_nv12_709_vert_src), &(shader_nv12_709_frag_src), "nv12_709", SHD_NV12, SHD_SAM11, SHD_SAM11, 0, 0, 0, 0 },
   { SHADER_NV12_709_NOMUL, &(shader_nv12_709_nomul_vert_src), &(shader_nv12_709_nomul_frag_src), "nv12_709_nomul", SHD_NV12, SHD_SAM11, SHD_SAM11, 0, 0, 1, 0 },
   { SHADER_NV12_709_MASK, &(shader_nv12_709_mask_vert_src), &(shader_nv12_709_mask_frag_src), "nv12_709_mask", SHD_NV12, SHD_SAM11, SHD_SAM11, 0, 1, 0, 0 },
   { SHADER_NV12_709_MASK_NOMUL, &(shader_nv12_709_mask_nomul_vert_src), &(shader_nv12_709_mask_nomul_frag_src), "nv12_709_mask_nomul", SHD_NV12, SHD_SAM11, SHD_SAM11, 0, 1, 1, 0 },
};

