/* General-purpose fragment shader for all operations in Evas.
 * This file can either be used directly by evas at runtime to
 * generate its shaders with the appropriate #defines, or passed
 * through cpp first (in which case the precision must be manually added).
 */

FRAGMENT_SHADER

#define M_PI   3.141592653589793238462643383279502884
#define M_PI_2 1.570796326794896619231321691639751442

#ifndef SHD_NOMUL
varying vec4 col;
#endif

#ifdef SHD_EXTERNAL
uniform SAMPLER_EXTERNAL_OES tex;
varying vec2 tex_c;
#elif defined(SHD_TEX)
uniform sampler2D tex;
varying vec2 tex_c;
#endif

#if defined(SHD_NV12) || defined(SHD_YUY2)
uniform sampler2D texuv;
varying vec2 tex_c2;
#endif

#if defined(SHD_YUV)
uniform sampler2D texu;
uniform sampler2D texv;
varying vec2 tex_c2;
varying vec2 tex_c3;
#endif

#ifdef SHD_TEXA
uniform sampler2D texa;
varying vec2 tex_a;
#endif

#if defined(SHD_SAM12) || defined(SHD_SAM21) || defined(SHD_SAM22)
varying vec4 div_s;
# if defined(SHD_SAM12) || defined(SHD_SAM21)
varying vec2 tex_s[2];
# else
varying vec2 tex_s[4];
# endif
#endif

#ifdef SHD_MASK
uniform sampler2D texm;
varying vec2 tex_m;
# if defined(SHD_MASKSAM12) || defined(SHD_MASKSAM21)
varying float maskdiv_s;
varying vec2 masktex_s[2];
# elif defined(SHD_MASKSAM22)
varying float maskdiv_s;
varying vec2 masktex_s[4];
# endif
#endif

#ifdef SHD_ALPHA
# define SWZ aaaa
#else
# ifndef SHD_BGRA
#  if defined(SHD_IMG) && defined(SHD_BIGENDIAN)
#   define SWZ gbar
#  else
#   define SWZ bgra
#endif
# else
#  if defined(SHD_IMG) && defined(SHD_BIGENDIAN)
#   define SWZ grab
#  else
#   define SWZ rgba
#  endif
# endif
#endif

#ifdef SHD_FILTER_DISPLACE
uniform sampler2D tex_filter;
varying vec2 displace_vector;
varying vec2 displace_min;
varying vec2 displace_max;
#endif

#ifdef SHD_FILTER_CURVE
uniform sampler2D tex_filter;
#endif

#ifdef SHD_FILTER_BLUR
uniform sampler2D tex_filter;
uniform int blur_count;
uniform float blur_texlen;
uniform float blur_div;
#endif

// ----------------------------------------------------------------------------

#ifndef SHD_FILTER_BLUR
void main()
{
#if defined(SHD_EXTERNAL) || defined(SHD_TEX)
   vec2 coord = tex_c;
#endif

#else // SHD_FILTER_BLUR

vec4 fetch_pixel(float ox, float oy)
{
   vec2 coord = tex_c + vec2(ox, oy);

#endif // SHD_FILTER_BLUR

   vec4 c;

#ifdef SHD_FILTER_DISPLACE
   vec2 dxy = (texture2D(tex_filter, tex_c).rg - vec2(0.5, 0.5)) * displace_vector;
   float fa = texture2D(tex_filter, tex_c).a;
   coord = clamp(tex_c + dxy, displace_min, displace_max);
#endif

#if defined(SHD_YUV) || defined(SHD_NV12) || defined(SHD_YUY2)
   float r, g, b, y, u, v, vmu;
# if defined(SHD_YUV)
   y = texture2D(tex, tex_c).r;
   u = texture2D(texu, tex_c2).r;
   v = texture2D(texv, tex_c3).r;
# elif defined(SHD_NV12) || defined(SHD_YUY2)
   y = texture2D(tex, tex_c).g;
   u = texture2D(texuv, tex_c2).g;
   v = texture2D(texuv, tex_c2).a;
# endif
// center u and v around 0 for uv and y (with 128/255 for u + v, 16/255 for y)
   u = u - 0.5;
   v = v - 0.5;

# if defined (SHD_YUV_709)
// 709 yuv colorspace for hd content
   y = (y - 0.062) * 1.164;
   vmu = (v * 0.534) + (u * 0.213);
   v = v * 1.793;
   u = u * 2.115;
# else
// 601 colorspace constants (older yuv content)
   y = (y - 0.062) * 1.164;
   vmu = (v * 0.813) + (u * 0.391);
   v = v * 1.596;
   u = u * 2.018;
# endif
// common yuv
   r = y + v;
   g = y - vmu;
   b = y + u;
   c = vec4(r, g, b, 1.0);

#elif defined(SHD_SAM12) || defined(SHD_SAM21)
   vec4 col00 = texture2D(tex, coord + tex_s[0]).SWZ;
   vec4 col01 = texture2D(tex, coord + tex_s[1]).SWZ;
   c = (col00 + col01) / div_s;

#elif defined(SHD_SAM22)
   vec4 col00 = texture2D(tex, coord + tex_s[0]).SWZ;
   vec4 col01 = texture2D(tex, coord + tex_s[1]).SWZ;
   vec4 col10 = texture2D(tex, coord + tex_s[2]).SWZ;
   vec4 col11 = texture2D(tex, coord + tex_s[3]).SWZ;
   c = (col00 + col01 + col10 + col11) / div_s;

#elif defined(SHD_TEX) || defined(SHD_EXTERNAL)
   c = texture2D(tex, coord).SWZ;

#else
   c = vec4(1, 1, 1, 1);
#endif

#ifdef SHD_MASK
# ifndef SHD_MASK_COLOR
   // Classic mask: alpha only
   float ma;
#  if defined(SHD_MASKSAM12) || defined(SHD_MASKSAM21)
   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;
   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;
   ma = (ma00 + ma01) / maskdiv_s;
#  elif defined(SHD_MASKSAM22)
   float ma00 = texture2D(texm, tex_m + masktex_s[0]).a;
   float ma01 = texture2D(texm, tex_m + masktex_s[1]).a;
   float ma10 = texture2D(texm, tex_m + masktex_s[2]).a;
   float ma11 = texture2D(texm, tex_m + masktex_s[3]).a;
   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;
#  else
   ma = texture2D(texm, tex_m).a;
#  endif
# else
   // Full color mask
   vec4 ma;
#  if defined(SHD_MASKSAM12) || defined(SHD_MASKSAM21)
   vec4 ma00 = texture2D(texm, tex_m + masktex_s[0]);
   vec4 ma01 = texture2D(texm, tex_m + masktex_s[1]);
   ma = (ma00 + ma01) / maskdiv_s;
#  elif defined(SHD_MASKSAM22)
   vec4 ma00 = texture2D(texm, tex_m + masktex_s[0]);
   vec4 ma01 = texture2D(texm, tex_m + masktex_s[1]);
   vec4 ma10 = texture2D(texm, tex_m + masktex_s[2]);
   vec4 ma11 = texture2D(texm, tex_m + masktex_s[3]);
   ma = (ma00 + ma01 + ma10 + ma11) / maskdiv_s;
#  else
   ma = texture2D(texm, tex_m);
#  endif
# endif
#endif

#ifdef SHD_AFILL
   c.a = 1.0;
#endif

#ifdef SHD_TEXA
   c *= texture2D(texa, tex_a).r;
#endif

#if defined(SHD_FILTER_CURVE)
   float old_alpha = max(c.a, 0.00001);
   float new_alpha = texture2D(tex_filter, vec2(old_alpha, 0.0)).a;
   c = vec4(texture2D(tex_filter, vec2(c.r / old_alpha, 0.0)).r * new_alpha,
            texture2D(tex_filter, vec2(c.g / old_alpha, 0.0)).g * new_alpha,
            texture2D(tex_filter, vec2(c.b / old_alpha, 0.0)).b * new_alpha,
            new_alpha);
#endif

#ifdef SHD_ALPHA_ONLY
   c = vec4(c.a, c.a, c.a, c.a);
#endif

#ifdef SHD_FILTER_GRAYSCALE
   c.r = 0.3 * c.r + 0.59 * c.g + 0.11 * c.b;
   c.g = c.r;
   c.b = c.r;
#endif

#ifdef SHD_FILTER_INVERSE_COLOR
   c.rgb = c.a - c.rgba;
#endif

#ifndef SHD_FILTER_BLUR

   gl_FragColor =
       c
#ifndef SHD_NOMUL
     * col
#endif
#ifdef SHD_MASK
     * ma
#endif
#ifdef SHD_FILTER_DISPLACE
     * fa
#endif
   ;
}

#else // SHD_FILTER_BLUR

   return c;
}

#ifndef SHD_FILTER_DIR_Y
# define FETCH_PIXEL(x) fetch_pixel((x), 0.0)
#else
# define FETCH_PIXEL(x) fetch_pixel(0.0, (x))
#endif

float weight_get(float u, float count, float index)
{
   vec4 val = texture2D(tex_filter, vec2(u / count, index)).bgra;
   return val.a * 255.0 + val.r + val.g / 256.0 + val.b / 65536.0;
}

float offset_get(float u, float count, float index)
{
   // val.a is always 0 here ~ discard
   vec4 val = texture2D(tex_filter, vec2(u / count, index)).bgra;
   return val.r + val.g / 256.0 + val.b / 65536.0;
}

void main()
{
   float weight, offset, count;
   vec4 acc, px;
   int k;

   count = float(blur_count);


   // Center pixel, offset is 0.0
   weight = weight_get(0.0, count, 0.0);
   px = FETCH_PIXEL(0.0);
   acc = px * weight;

   // Left & right pixels
   for (k = 1; k <= blur_count; k++)
   {
      float u = float(k);

      weight = weight_get(u, count, 0.0);
      offset = offset_get(u, count, 1.0);

      // Left
      vec4 px1 = FETCH_PIXEL(-((offset + (2.0 * u) - 1.0)) / blur_texlen);

      // Right
      vec4 px2 = FETCH_PIXEL((offset + (2.0 * u) - 1.0) / blur_texlen);

      acc += (px1 + px2) * weight;
   }

#ifndef SHD_NOMUL
   gl_FragColor = (acc / blur_div) * col;
#else
   gl_FragColor = (acc / blur_div);
#endif
}

#endif // SHD_FILTER_BLUR
