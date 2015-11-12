/* General-purpose vertex shader for all operations in Evas.
 * This file can either be used directly by evas at runtime to
 * generate its shaders with the appropriate #defines, or passed
 * through cpp first (in which case the precision must be manually added).
 */

VERTEX_SHADER

attribute vec4 vertex;
uniform mat4 mvp;

/* All except nomul */
#ifndef SHD_NOMUL
attribute vec4 color;
varying vec4 col;
#endif

/* All images & fonts */
#if defined(SHD_TEX) || defined(SHD_EXTERNAL)
attribute vec2 tex_coord;
varying vec2 tex_c;
#endif

/* NV12, YUY2 */
#if defined(SHD_NV12) || defined(SHD_YUY2) || defined(SHD_YUV)
attribute vec2 tex_coord2;
varying vec2 tex_c2;
#endif

/* YUV */
#ifdef SHD_YUV
attribute vec2 tex_coord3;
varying vec2 tex_c3;
#endif

/* RGB+A */
#ifdef SHD_TEXA
attribute vec2 tex_coorda;
varying vec2 tex_a;
#endif

/* Sampling */
#if defined(SHD_SAM12) || defined(SHD_SAM21) || defined(SHD_SAM22)
attribute vec2 tex_sample;
varying vec4 div_s;
# if defined(SHD_SAM12) || defined(SHD_SAM21)
varying vec2 tex_s[2];
# else
varying vec2 tex_s[4];
# endif
#endif

/* Masking */
#ifdef SHD_MASK
attribute vec4 mask_coord;
varying vec2 tex_m;
# if defined(SHD_MASKSAM12) || defined(SHD_MASKSAM21)
attribute vec2 tex_masksample;
varying float maskdiv_s;
varying vec2 masktex_s[2];
# elif defined(SHD_MASKSAM22)
attribute vec2 tex_masksample;
varying float maskdiv_s;
varying vec2 masktex_s[4];
# endif
#endif


void main()
{
   gl_Position = mvp * vertex;

#ifndef SHD_NOMUL
   col = color;
#endif

#if defined(SHD_TEX) || defined(SHD_EXTERNAL)
   tex_c = tex_coord;
#endif

#ifdef SHD_NV12
   tex_c2 = tex_coord2 * 0.5;
#endif

#ifdef SHD_YUY2
   tex_c2 = vec2(tex_coord2.x * 0.5, tex_coord2.y);
#endif

#ifdef SHD_YUV
   tex_c2 = tex_coord2;
   tex_c3 = tex_coord3;
#endif

#ifdef SHD_TEXA
   tex_a = tex_coorda;
#endif

#if defined(SHD_SAM12)
   tex_s[0] = vec2(0, -tex_sample.y);
   tex_s[1] = vec2(0,  tex_sample.y);
   div_s = vec4(2, 2, 2, 2);
#elif defined(SHD_SAM21)
   tex_s[0] = vec2(-tex_sample.x, 0);
   tex_s[1] = vec2( tex_sample.x, 0);
   div_s = vec4(2, 2, 2, 2);
#elif defined(SHD_SAM22)
   tex_s[0] = vec2(-tex_sample.x, -tex_sample.y);
   tex_s[1] = vec2( tex_sample.x, -tex_sample.y);
   tex_s[2] = vec2( tex_sample.x,  tex_sample.y);
   tex_s[3] = vec2(-tex_sample.x,  tex_sample.y);
   div_s = vec4(4, 4, 4, 4);
#endif

#if defined(SHD_MASKSAM12)
   masktex_s[0] = vec2(0, -tex_masksample.y);
   masktex_s[1] = vec2(0,  tex_masksample.y);
   maskdiv_s = 2.0;
#elif defined(SHD_MASKSAM21)
   masktex_s[0] = vec2(-tex_masksample.x, 0);
   masktex_s[1] = vec2( tex_masksample.x, 0);
   maskdiv_s = 2.0;
#elif defined(SHD_MASKSAM22)
   masktex_s[0] = vec2(-tex_masksample.x, -tex_masksample.y);
   masktex_s[1] = vec2( tex_masksample.x, -tex_masksample.y);
   masktex_s[2] = vec2( tex_masksample.x,  tex_masksample.y);
   masktex_s[3] = vec2(-tex_masksample.x,  tex_masksample.y);
   maskdiv_s = 4.0;
#endif

#ifdef SHD_MASK
   // mask_coord.w contains the Y-invert flag
   // position on screen in [0..1] range of current pixel
   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);
   tex_m = mask_Position.xy * abs(mask_coord.zw) + mask_coord.xy;
#endif
}

