#ifndef EVAS_COMMON_H
#define EVAS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "Evas.h"
#include "config.h"

/*****************************************************************************/

#include "evas_options.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <ctype.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifdef _WIN32_WCE
#include <windows.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SIZES_H

#ifdef __GNUC__
# if __GNUC__ >= 4
// BROKEN in gcc 4 on amd64
//#  pragma GCC visibility push(hidden)
# endif
#endif

/*****************************************************************************/

/* use exact rects for updates not tiles */
/* #define RECTUPDATE */
#define TILESIZE 8

#ifdef BUILD_SMALL_DITHER_MASK
# define DM_TABLE     _evas_dither_44
# define DM_SIZE      4
# define DM_BITS      4
# define DM_DIV       16
# define USE_DITHER_44 1
# define DM_MSK       (DM_SIZE - 1)
# define DM_SHF(_b)   (DM_BITS - (8 - _b))
#else
# define DM_TABLE     _evas_dither_128128
# define DM_SIZE      128
# define DM_BITS      6
# define DM_DIV       64
# define USE_DITHER_128128 1
# define DM_MSK       (DM_SIZE - 1)
# define DM_SHF(_b)   (DM_BITS - (8 - _b))
#endif

/*****************************************************************************/

/*****************************************************************************/

#ifndef _WIN32_WCE
typedef unsigned long long            DATA64;
#else
typedef unsigned __int64	      DATA64;
#define strdup _strdup
#define snprintf _snprintf
#define rewind(f) fseek(f,0,SEEK_SET)
#endif

typedef unsigned int                  DATA32;
typedef unsigned short                DATA16;
typedef unsigned char                 DATA8;

typedef struct _Evas_Object_List      Evas_Object_List;

typedef struct _Evas_Hash_El          Evas_Hash_El;

typedef struct _RGBA_Image            RGBA_Image;
typedef struct _RGBA_Surface          RGBA_Surface;
typedef struct _RGBA_Image_Span       RGBA_Image_Span;
typedef struct _RGBA_Draw_Context     RGBA_Draw_Context;
typedef struct _RGBA_Gradient         RGBA_Gradient;
typedef struct _RGBA_Gradient_Color   RGBA_Gradient_Color;
typedef struct _RGBA_Gradient_Type    RGBA_Gradient_Type;
typedef struct _RGBA_Polygon_Point    RGBA_Polygon_Point;
typedef struct _RGBA_Font             RGBA_Font;
typedef struct _RGBA_Font_Int         RGBA_Font_Int;
typedef struct _RGBA_Font_Source      RGBA_Font_Source;
typedef struct _RGBA_Font_Glyph       RGBA_Font_Glyph;

typedef struct _Cutout_Rect           Cutout_Rect;

typedef struct _Convert_Pal           Convert_Pal;

typedef struct _Tilebuf               Tilebuf;
typedef struct _Tilebuf_Tile          Tilebuf_Tile;
typedef struct _Tilebuf_Rect          Tilebuf_Rect;
/*
typedef struct _Regionbuf             Regionbuf;
typedef struct _Regionspan            Regionspan;
*/
typedef void (*Gfx_Func_Blend_Src_Dst) (DATA32 *src, DATA32 *dst, int len);
typedef void (*Gfx_Func_Blend_Color_Dst) (DATA32 src, DATA32 *dst, int len);
typedef void (*Gfx_Func_Blend_Src_Cmod_Dst) (DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod);
typedef void (*Gfx_Func_Blend_Src_Mul_Dst) (DATA32 *src, DATA32 *dst, int len, DATA32 mul_color);
typedef void (*Gfx_Func_Blend_Src_Alpha_Mul_Dst) (DATA8 *src, DATA32 *dst, int len, DATA32 col);

typedef void (*Gfx_Func_Convert) (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

typedef void (*Gfx_Func_Gradient_Span)(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy,
                                       void *geom_data);

/*****************************************************************************/

typedef enum _RGBA_Image_Flags
{
   RGBA_IMAGE_NOTHING     = (0),
   RGBA_IMAGE_HAS_ALPHA   = (1 << 0),
   RGBA_IMAGE_IS_DIRTY    = (1 << 1),
   RGBA_IMAGE_INDEXED     = (1 << 2),
   RGBA_IMAGE_ALPHA_ONLY  = (1 << 3),
   RGBA_IMAGE_ALPHA_TILES = (1 << 4)
} RGBA_Image_Flags;

typedef enum _Convert_Pal_Mode
{
   PAL_MODE_NONE,
   PAL_MODE_MONO,
   PAL_MODE_GRAY4,
   PAL_MODE_GRAY16,
   PAL_MODE_GRAY64,
   PAL_MODE_GRAY256,
   PAL_MODE_RGB111,
   PAL_MODE_RGB121,
   PAL_MODE_RGB221,
   PAL_MODE_RGB222,
   PAL_MODE_RGB232,
   PAL_MODE_RGB666,
   PAL_MODE_RGB332,
   PAL_MODE_LAST
} Convert_Pal_Mode;

typedef enum _CPU_Features
{
   CPU_FEATURE_C       = 0,
   CPU_FEATURE_MMX     = (1 << 0),
   CPU_FEATURE_MMX2    = (1 << 1),
   CPU_FEATURE_SSE     = (1 << 2),
   CPU_FEATURE_ALTIVEC = (1 << 3),
   CPU_FEATURE_VIS     = (1 << 4),
   CPU_FEATURE_VIS2    = (1 << 5)
} CPU_Features;

typedef enum _Font_Hint_Flags
{
   FONT_NO_HINT,
   FONT_AUTO_HINT,
   FONT_BYTECODE_HINT
} Font_Hint_Flags;

/*****************************************************************************/

struct _Evas_Object_List
{
   Evas_Object_List  *next, *prev;
   Evas_Object_List  *last;
};

struct _Evas_Hash
{
   int               population;
   Evas_Object_List *buckets[256];
};

struct _Evas_Hash_El
{
   Evas_Object_List  _list_data;
   char             *key;
   void             *data;
};

struct _RGBA_Draw_Context
{
   struct {
      char   use : 1;
      DATA8  r[256], g[256], b[256], a[256];
   } mod;
   struct {
      char   use : 1;
      DATA32 col;
   } mul;
   struct {
      DATA32 col;
   } col;
   struct {
      char   use : 1;
      int    x, y, w, h;
   } clip;
   struct {
     Cutout_Rect *rects;
   } cutout;
   struct {
      struct {
	 void *(*gl_new)  (void *data, RGBA_Font_Glyph *fg);
	 void  (*gl_free) (void *ext_dat);
	 void  (*gl_draw) (void *data, void *dest, void *context, RGBA_Font_Glyph *fg, int x, int y);
      } func;
      void *data;
   } font_ext;
   struct {
      int color_space;
   } interpolation;
   unsigned char  anti_alias : 1;
};

struct _RGBA_Surface
{
   int                w, h;
   DATA32            *data;
   char               no_free : 1;
   RGBA_Image        *im;
};

struct _RGBA_Image
{
   Evas_Object_List     _list_data;
   RGBA_Surface        *image;
   RGBA_Image_Flags     flags;
   struct
     {
	void           *loader;
//	int             format;
	char           *file;
	char           *real_file;
	char           *key;
	char           *comment;
     } info;
   int                  references;
//   DATA64               timestamp;
   void                *extended_info;
};

struct _RGBA_Gradient_Color
{
   Evas_Object_List  _list_data;
   int               r, g, b, a;
   int               dist;
};

struct _RGBA_Gradient
{
   Evas_Object_List *colors;
   int               ncolors;
   int               len;

   unsigned char     has_alpha : 1;
   struct
     {
	int            x, y, w, h;
     } fill;
   struct
     {
	char          *name;
	char          *params;
	RGBA_Gradient_Type *geometer;
     } type;
   struct
     {
	DATA32        *data;
	int            len;
	unsigned char  has_alpha : 1;
     } map;
};

struct _RGBA_Gradient_Type
{
   char                   *name;
   void                   *geom_data;
   void                    (*setup_geom)(RGBA_Gradient *gr, int spread);
   int                     (*has_alpha)(RGBA_Gradient *gr, int spread);
   int                     (*get_map_len)(RGBA_Gradient *gr, int spread);
   Gfx_Func_Gradient_Span  (*get_span_func)(RGBA_Gradient *gr, int spread, unsigned char aa);
};

struct _RGBA_Polygon_Point
{
   Evas_Object_List  _list_data;
   int               x, y;
};

struct _RGBA_Font
{
   Evas_List *fonts;
   Font_Hint_Flags hinting;
};

struct _RGBA_Font_Int
{
   Evas_Object_List  _list_data;

   RGBA_Font_Source *src;

   int               size;
   int               real_size;

   struct {
      FT_Size       size;
   } ft;

   Evas_Hash       *glyphs;

   int              usage;
   Font_Hint_Flags hinting;

   int              references;
};

struct _RGBA_Font_Source
{
   Evas_Object_List  _list_data;

   char             *name;
   char             *file;

   void             *data;
   int               data_size;

   int               current_size;

   struct {
      int           orig_upem;
      FT_Face       face;
   } ft;

   int              references;
};

struct _RGBA_Font_Glyph
{
   FT_Glyph        glyph;
   FT_BitmapGlyph  glyph_out;
   void           *ext_dat;
   void           (*ext_dat_free) (void *ext_dat);
};

struct _Tilebuf
{
   int outbuf_w;
   int outbuf_h;

   struct {
      int           w, h;
   } tile_size;

#ifdef RECTUPDATE
   Regionbuf *rb;
#else
   struct {
      int           w, h;
      Tilebuf_Tile *tiles;
   } tiles;
#endif
};

struct _Tilebuf_Tile
{
   unsigned char redraw : 1;
/* FIXME: need these flags later - but not now */
/*
   int done   : 1;
   int edge   : 1;
   int from   : 1;

   struct {
      int dx, dy;
   } vector;
 */
};

struct _Tilebuf_Rect
{
   Evas_Object_List  _list_data;
   int               x, y, w, h;
};
/*
struct _Regionbuf
{
   int w, h;
   Regionspan **spans;
};

struct _Regionspan
{
   Evas_Object_List  _list_data;
   int x1, x2;
};
*/
struct _Cutout_Rect
{
   Evas_Object_List  _list_data;
   int               x, y, w, h;
};

struct _Convert_Pal
{
   int               references;
   int               count;
   Convert_Pal_Mode  colors;
   DATA8            *lookup;
   void             *data;
};

/*****************************************************************************/
#include "evas_macros.h"

#define CONVERT_LOOP_START_ROT_0() \
   src_ptr = src; \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {
#define CONVERT_LOOP_END_ROT_0() \
             dst_ptr++; \
             src_ptr++; \
          } \
        src_ptr += src_jump; \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP_START_ROT_270() \
   src_ptr = src + ((w - 1) * (h + src_jump)); \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {
#define CONVERT_LOOP_END_ROT_270() \
             dst_ptr++; \
             src_ptr -= (h + src_jump); \
          } \
        src_ptr = src + ((w - 1) * (h + src_jump)) + (y + 1); \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP_START_ROT_90() \
   src_ptr = src + (h - 1); \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {
#define CONVERT_LOOP_END_ROT_90() \
             dst_ptr++; \
             src_ptr += (h + src_jump); \
          } \
        src_ptr = src + (h - 1) - y - 1; \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP2_START_ROT_0() \
   src_ptr = src; \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {
#define CONVERT_LOOP2_INC_ROT_0() \
src_ptr++; \
x++;
#define CONVERT_LOOP2_END_ROT_0() \
             dst_ptr+=2; \
             src_ptr++; \
          } \
        src_ptr += src_jump; \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP2_START_ROT_270() \
   src_ptr = src + ((w - 1) * (h + src_jump)); \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {
#define CONVERT_LOOP2_INC_ROT_270() \
src_ptr -= (h + src_jump); \
x++;
#define CONVERT_LOOP2_END_ROT_270() \
             dst_ptr+=2; \
             src_ptr -= (h + src_jump); \
          } \
        src_ptr = src + ((w - 1) * (h + src_jump)) + (y + 1); \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP2_START_ROT_90() \
   src_ptr = src + (h - 1); \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {
#define CONVERT_LOOP2_INC_ROT_90() \
src_ptr += (h + src_jump); \
x++;
#define CONVERT_LOOP2_END_ROT_90() \
             dst_ptr+=2; \
             src_ptr += (h + src_jump); \
          } \
        src_ptr = src + (h - 1) - y - 1; \
        dst_ptr += dst_jump; \
     }

#ifndef WORDS_BIGENDIAN
/* x86 */
#define A_VAL(p) ((DATA8 *)(p))[3]
#define R_VAL(p) ((DATA8 *)(p))[2]
#define G_VAL(p) ((DATA8 *)(p))[1]
#define B_VAL(p) ((DATA8 *)(p))[0]
#define AR_VAL(p) ((DATA16 *)(p)[1])
#define GB_VAL(p) ((DATA16 *)(p)[0])
#else
/* ppc */
#define A_VAL(p) ((DATA8 *)(p))[0]
#define R_VAL(p) ((DATA8 *)(p))[1]
#define G_VAL(p) ((DATA8 *)(p))[2]
#define B_VAL(p) ((DATA8 *)(p))[3]
#define AR_VAL(p) ((DATA16 *)(p)[0])
#define GB_VAL(p) ((DATA16 *)(p)[1])
#endif

#define RGB_JOIN(r,g,b) \
        (((r) << 16) + ((g) << 8) + (b))

#define ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

/* thanks to some chats with Mirek Fidler... the new blender code for C
 * blender fallbacks is faster. benchmarks (for the blending code only):
 *
 * OLD BLENDER:
 * MMX: 0.606
 * C:   1.026
 *
 * NEW BLENDER:
 * MMX: 0.549
 * C:   0.716
 */

/*
#define BLEND_ADST_ALPHA_SETUP(a, tmp)
#define BLEND_ADST_COLOR(a, nc, c, cc, tmp)           \
{                                                     \
  (tmp) = ((c) - (cc)) * (a);                         \
  (nc) = (cc) + (((tmp) + ((tmp) >> 8) + 0x80) >> 8); \
}
*/

#define BLEND_ALPHA_SETUP(a, tmp) \
{ \
   (tmp) = ((a) + ((a) >> 7)); \
}
#define BLEND_COLOR(a, nc, c, cc, tmp)                \
{ \
   (nc) = ((((tmp) * ((c) - (cc))) >> 8) + (cc)); \
}

#define BLEND_ADST_ALPHA_SETUP BLEND_ALPHA_SETUP
#define BLEND_ADST_COLOR BLEND_COLOR


#define PIXEL_SOLID_ALPHA 0xff000000

#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))
#define RECTS_CLIP_TO_RECT(_x, _y, _w, _h, _cx, _cy, _cw, _ch) \
{ \
   if (RECTS_INTERSECT(_x, _y, _w, _h, _cx, _cy, _cw, _ch)) \
     { \
	if (_x < (_cx)) \
	  { \
	     _w += _x - (_cx); \
	     _x = (_cx); \
	     if (_w < 0) _w = 0; \
	  } \
	if ((_x + _w) > ((_cx) + (_cw))) \
	  _w = (_cx) + (_cw) - _x; \
	if (_y < (_cy)) \
	  { \
	     _h += _y - (_cy); \
	     _y = (_cy); \
	     if (_h < 0) _h = 0; \
	  } \
	if ((_y + _h) > ((_cy) + (_ch))) \
	  _h = (_cy) + (_ch) - _y; \
     } \
   else \
     { \
	_w = 0; _h = 0; \
     } \
}

#define _EVAS_TEXTURE_REFLECT     0
#define _EVAS_TEXTURE_REPEAT      1
#define _EVAS_TEXTURE_RESTRICT    2

#define _EVAS_COLOR_SPACE_ARGB    0
#define _EVAS_COLOR_SPACE_AHSV    1

/*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void *evas_object_list_append           (void *in_list, void *in_item);
void *evas_object_list_prepend          (void *in_list, void *in_item);
void *evas_object_list_append_relative  (void *in_list, void *in_item, void *in_relative);
void *evas_object_list_prepend_relative (void *in_list, void *in_item, void *in_relative);
void *evas_object_list_remove           (void *in_list, void *in_item);
void *evas_object_list_find             (void *in_list, void *in_item);

/****/
void evas_common_init                                   (void);
void evas_common_shutdown                               (void);

void evas_common_cpu_init                               (void);

int  evas_common_cpu_have_cpuid                         (void);
int  evas_common_cpu_has_feature                        (unsigned int feature);
void evas_common_cpu_can_do                             (int *mmx, int *sse, int *sse2);
void evas_common_cpu_end_opt                            (void);

/****/
void evas_common_blend_init                             (void);
void evas_common_blend_init_evas_pow_lut                (void);
void evas_common_blend_free_evas_pow_lut                (void);

void evas_common_blend_pixels_rgba_to_rgb_c             (DATA32 *src, DATA32 *dst, int len);
void evas_common_blend_pixels_rgba_to_rgb_mmx           (DATA32 *src, DATA32 *dst, int len);
void evas_common_blend_pixels_rgba_to_rgba_c            (DATA32 *src, DATA32 *dst, int len);

void evas_common_copy_pixels_rgba_to_rgba_c             (DATA32 *src, DATA32 *dst, int len);
void evas_common_copy_pixels_rgba_to_rgba_mmx           (DATA32 *src, DATA32 *dst, int len);
void evas_common_copy_pixels_rgba_to_rgba_mmx2          (DATA32 *src, DATA32 *dst, int len);
void evas_common_copy_pixels_rgba_to_rgba_sse/*NB*/     (DATA32 *src, DATA32 *dst, int len);
void evas_common_copy_pixels_rgb_to_rgba_c              (DATA32 *src, DATA32 *dst, int len);

void evas_common_copy_pixels_rev_rgba_to_rgba_c         (DATA32 *src, DATA32 *dst, int len);
void evas_common_copy_pixels_rev_rgba_to_rgba_mmx       (DATA32 *src, DATA32 *dst, int len);
void evas_common_copy_pixels_rev_rgba_to_rgba_sse/*NB*/ (DATA32 *src, DATA32 *dst, int len);
void evas_common_copy_pixels_rev_rgb_to_rgba_c          (DATA32 *src, DATA32 *dst, int len);

void evas_common_blend_color_rgba_to_rgb_c              (DATA32 src, DATA32 *dst, int len);
void evas_common_blend_color_rgba_to_rgb_mmx            (DATA32 src, DATA32 *dst, int len);
void evas_common_blend_color_rgba_to_rgba_c             (DATA32 src, DATA32 *dst, int len);

void evas_common_copy_color_rgba_to_rgba_c              (DATA32 src, DATA32 *dst, int len);
void evas_common_copy_color_rgba_to_rgba_mmx            (DATA32 src, DATA32 *dst, int len);
void evas_common_copy_color_rgba_to_rgba_sse/*NB*/      (DATA32 src, DATA32 *dst, int len);
void evas_common_copy_color_rgb_to_rgba_c               (DATA32 src, DATA32 *dst, int len);

void evas_common_blend_pixels_cmod_rgba_to_rgb_c        (DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod);
void evas_common_blend_pixels_cmod_rgba_to_rgba_c       (DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod);

void evas_common_copy_pixels_cmod_rgba_to_rgba_c        (DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod);
void evas_common_copy_pixels_cmod_rgb_to_rgba_c         (DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod);

void evas_common_blend_pixels_mul_color_rgba_to_rgb_c   (DATA32 *src, DATA32 *dst, int len, DATA32 mul_color);
void evas_common_blend_pixels_mul_color_rgba_to_rgb_mmx (DATA32 *src, DATA32 *dst, int len, DATA32 mul_color);
void evas_common_blend_pixels_mul_color_rgba_to_rgba_c  (DATA32 *src, DATA32 *dst, int len, DATA32 mul_color);

void evas_common_blend_alpha_color_rgba_to_rgb_c        (DATA8 *src, DATA32 *dst, int len, DATA32 col);
void evas_common_blend_alpha_color_rgba_to_rgb_mmx      (DATA8 *src, DATA32 *dst, int len, DATA32 col);
void evas_common_blend_alpha_color_rgba_to_rgba_c       (DATA8 *src, DATA32 *dst, int len, DATA32 col);

/****/
void             evas_common_convert_init               (void);
Gfx_Func_Convert evas_common_convert_func_get           (DATA8 *dest, int w, int h, int depth, DATA32 rmask, DATA32 gmask, DATA32 bmask, Convert_Pal_Mode pal_mode, int rotation);

void evas_common_convert_rgba2_to_16bpp_rgb_565_dith            (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_565_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba2_to_16bpp_rgb_444_dith            (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_444_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba2_to_16bpp_rgb_454645_dith         (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_454645_dith          (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba2_to_16bpp_rgb_555_dith            (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_555_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

void evas_common_convert_rgba2_to_16bpp_rgb_565_dith_rot_270    (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_565_dith_rot_270     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba2_to_16bpp_rgb_444_dith_rot_270    (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_444_dith_rot_270     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba2_to_16bpp_rgb_454645_dith_rot_270 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_454645_dith_rot_270  (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba2_to_16bpp_rgb_555_dith_rot_270    (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_555_dith_rot_270     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

void evas_common_convert_rgba2_to_16bpp_rgb_565_dith_rot_90     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_565_dith_rot_90      (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba2_to_16bpp_rgb_444_dith_rot_90     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_444_dith_rot_90      (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba2_to_16bpp_rgb_454645_dith_rot_90  (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_454645_dith_rot_90   (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba2_to_16bpp_rgb_555_dith_rot_90     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_16bpp_rgb_555_dith_rot_90      (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

void evas_common_convert_rgba_to_24bpp_rgb_888                 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_24bpp_bgr_888                 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

void evas_common_convert_rgba_to_32bpp_rgb_8888                (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_rgb_8888_rot_270        (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_rgb_8888_rot_90         (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_rgbx_8888               (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_rgbx_8888_rot_270       (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_rgbx_8888_rot_90        (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_bgr_8888                (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_bgr_8888_rot_270        (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_bgr_8888_rot_90         (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_bgrx_8888               (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_bgrx_8888_rot_270       (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_32bpp_bgrx_8888_rot_90        (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

void evas_common_convert_rgba_to_8bpp_rgb_332_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_666_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_232_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_222_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_221_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_121_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_111_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_gry_256_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_gry_64_dith              (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_gry_16_dith              (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_gry_4_dith               (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_gry_1_dith               (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

void evas_common_convert_rgba_to_4bpp_gry_4_dith               (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_4bpp_gry_1_dith               (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

void evas_common_convert_rgba_to_1bpp_gry_1_dith               (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

void evas_common_convert_yuv_420p_601_rgba                     (DATA8 **src, DATA8 *dst, int w, int h);

void evas_common_convert_hsv_to_rgb                     (float h, float s, float v, int *r, int *g, int *b);
void evas_common_convert_rgb_to_hsv                     (int r, int g, int b, float *h, float *s, float *v);
void evas_common_convert_hsv_to_rgb_int                 (int h, int s, int v, int *r, int *g, int *b);
void evas_common_convert_rgb_to_hsv_int                 (int r, int g, int b, int *h, int *s, int *v);

/****/
void evas_common_scale_init                            (void);

void evas_common_scale_rgba_mipmap_down_2x2_c          (DATA32 *src, DATA32 *dst, int src_w, int src_h);
void evas_common_scale_rgba_mipmap_down_2x1_c          (DATA32 *src, DATA32 *dst, int src_w, int src_h);
void evas_common_scale_rgba_mipmap_down_1x2_c          (DATA32 *src, DATA32 *dst, int src_w, int src_h);
void evas_common_scale_rgb_mipmap_down_2x2_c           (DATA32 *src, DATA32 *dst, int src_w, int src_h);
void evas_common_scale_rgb_mipmap_down_2x1_c           (DATA32 *src, DATA32 *dst, int src_w, int src_h);
void evas_common_scale_rgb_mipmap_down_1x2_c           (DATA32 *src, DATA32 *dst, int src_w, int src_h);

void evas_common_scale_rgba_mipmap_down_2x2_mmx        (DATA32 *src, DATA32 *dst, int src_w, int src_h);
void evas_common_scale_rgba_mipmap_down_2x1_mmx        (DATA32 *src, DATA32 *dst, int src_w, int src_h);
void evas_common_scale_rgba_mipmap_down_1x2_mmx        (DATA32 *src, DATA32 *dst, int src_w, int src_h);

void evas_common_scale_rgba_in_to_out_clip_smooth_mmx  (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
void evas_common_scale_rgba_in_to_out_clip_smooth_c    (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

void evas_common_scale_rgba_in_to_out_clip_smooth      (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
void evas_common_scale_rgba_in_to_out_clip_sample      (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

/****/
void          evas_common_image_init              (void);
void          evas_common_image_shutdown          (void);

RGBA_Surface *evas_common_image_surface_new       (RGBA_Image *im);
void          evas_common_image_surface_free      (RGBA_Surface *is);
void          evas_common_image_surface_alloc     (RGBA_Surface *is);
void          evas_common_image_surface_dealloc   (RGBA_Surface *is);

RGBA_Image   *evas_common_image_create            (int w, int h);
RGBA_Image   *evas_common_image_new               (void);
void          evas_common_image_free              (RGBA_Image *im);
void          evas_common_image_ref               (RGBA_Image *im);
void          evas_common_image_unref             (RGBA_Image *im);
void          evas_common_image_cache             (RGBA_Image *im);
void          evas_common_image_uncache           (RGBA_Image *im);
void          evas_common_image_flush_cache       (void);
void          evas_common_image_set_cache         (int size);
int           evas_common_image_get_cache         (void);
void          evas_common_image_store             (RGBA_Image *im);
void          evas_common_image_unstore           (RGBA_Image *im);
RGBA_Image   *evas_common_image_find              (const char *filename, const char *key, DATA64 timestamp);
int           evas_common_image_ram_usage         (RGBA_Image *im);
void          evas_common_image_dirty             (RGBA_Image *im);
void          evas_common_image_cache_free        (void);

RGBA_Image         *evas_common_load_image_from_file     (const char *file, const char *key);
void                evas_common_load_image_data_from_file(RGBA_Image *im);
int                 evas_common_save_image_to_file       (RGBA_Image *im, const char *file, const char *key, int quality, int compress);

RGBA_Image   *evas_common_image_line_buffer_obtain       (int len);
void          evas_common_image_line_buffer_release      (void);
void          evas_common_image_line_buffer_free         (void);

/****/
void           evas_common_rectangle_init          (void);

void           evas_common_rectangle_draw          (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

/****/
void           evas_common_gradient_init           (void);

RGBA_Gradient *evas_common_gradient_new            (void);
void           evas_common_gradient_free           (RGBA_Gradient *gr);
void           evas_common_gradient_colors_clear   (RGBA_Gradient *gr);
void           evas_common_gradient_color_add      (RGBA_Gradient *gr, int r, int g, int b, int a, int dist);
void           evas_common_gradient_type_set        (RGBA_Gradient *gr, char *name);
void           evas_common_gradient_type_params_set (RGBA_Gradient *gr, char *params);
void           evas_common_gradient_fill_set        (RGBA_Gradient *gr, int x, int y, int w, int h);
RGBA_Gradient *evas_common_gradient_geometry_init   (RGBA_Gradient *gr, int spread);
int            evas_common_gradient_has_alpha       (RGBA_Gradient *gr, int spread);
void           evas_common_gradient_map             (RGBA_Draw_Context *dc, RGBA_Gradient *gr, int spread);
void           evas_common_gradient_draw            (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient *gr, double angle, int spread);

RGBA_Gradient_Type  *evas_common_gradient_geometer_get     (char *name);
RGBA_Gradient_Type  *evas_common_gradient_linear_get       (void);
RGBA_Gradient_Type  *evas_common_gradient_radial_get       (void);
RGBA_Gradient_Type  *evas_common_gradient_angular_get      (void);
RGBA_Gradient_Type  *evas_common_gradient_rectangular_get  (void);
RGBA_Gradient_Type  *evas_common_gradient_sinusoidal_get   (void);
char                *evas_common_gradient_get_key_fval     (char *in, char *key, float *val);

/****/
void          evas_common_line_init               (void);

void          evas_common_line_draw               (RGBA_Image *dst, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2);

/****/
void                evas_common_polygon_init         (void);

RGBA_Polygon_Point *evas_common_polygon_point_add    (RGBA_Polygon_Point *points, int x, int y);
RGBA_Polygon_Point *evas_common_polygon_points_clear (RGBA_Polygon_Point *points);
void                evas_common_polygon_draw         (RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points);

/****/
void          evas_common_blit_init               (void);

void          evas_common_blit_rectangle          (RGBA_Image *src, RGBA_Image *dst, int src_x, int src_y, int w, int h, int dst_x, int dst_y);

/****/

/****/
void             evas_common_font_init              (void);
void             evas_common_font_shutdown          (void);
RGBA_Font_Source *evas_common_font_source_memory_load(const char *name, const void *data, int data_size);
RGBA_Font_Source *evas_common_font_source_load       (const char *name);
RGBA_Font_Source *evas_common_font_source_find       (const char *name);
void              evas_common_font_source_free       (RGBA_Font_Source *fs);
void              evas_common_font_size_use         (RGBA_Font *fn);
RGBA_Font       *evas_common_font_memory_load       (const char *name, int size, const void *data, int data_size);
RGBA_Font       *evas_common_font_load              (const char *name, int size);
RGBA_Font       *evas_common_font_add               (RGBA_Font *fn, const char *name, int size);
RGBA_Font       *evas_common_font_memory_add        (RGBA_Font *fn, const char *name, int size, const void *data, int data_size);
RGBA_Font_Int   *evas_common_font_int_load_init     (RGBA_Font_Int *fn);
void             evas_common_font_free              (RGBA_Font *fn);
void             evas_common_font_hinting_set       (RGBA_Font *fn, Font_Hint_Flags hinting);
Evas_Bool        evas_common_hinting_available      (Font_Hint_Flags hinting);
RGBA_Font       *evas_common_font_memory_hinting_load(const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting);
RGBA_Font       *evas_common_font_hinting_load      (const char *name, int size, Font_Hint_Flags hinting);
RGBA_Font       *evas_common_font_hinting_add       (RGBA_Font *fn, const char *name, int size, Font_Hint_Flags hinting);
RGBA_Font       *evas_common_font_memory_hinting_add(RGBA_Font *fn, const char *name, int size, const void *data, int data_size, Font_Hint_Flags hinting);
void             evas_common_font_int_modify_cache_by(RGBA_Font_Int *fi, int dir);
int              evas_common_font_cache_get         (void);
void             evas_common_font_cache_set         (int size);
void             evas_common_font_flush             (void);
void             evas_common_font_flush_last        (void);
RGBA_Font_Int   *evas_common_font_int_find          (const char *name, int size);
int              evas_common_font_glyph_search      (RGBA_Font *fn, RGBA_Font_Int **fi_ret, int gl);
void             evas_common_font_draw              (RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const char *text);
RGBA_Font_Glyph *evas_common_font_int_cache_glyph_get(RGBA_Font_Int *fi, FT_UInt index);
int              evas_common_font_ascent_get        (RGBA_Font *fn);
int              evas_common_font_descent_get       (RGBA_Font *fn);
int              evas_common_font_max_ascent_get    (RGBA_Font *fn);
int              evas_common_font_max_descent_get   (RGBA_Font *fn);
int              evas_common_font_get_line_advance  (RGBA_Font *fn);
void             evas_common_font_query_size        (RGBA_Font *fn, const char *text, int *w, int *h);
int              evas_common_font_query_inset       (RGBA_Font *fn, const char *text);
void             evas_common_font_query_advance     (RGBA_Font *fn, const char *text, int *h_adv, int *v_adv);
int              evas_common_font_query_char_coords (RGBA_Font *fn, const char *text, int pos, int *cx, int *cy, int *cw, int *ch);
int              evas_common_font_query_text_at_pos (RGBA_Font *fn, const char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
int              evas_common_font_utf8_get_next     (unsigned char *buf, int *iindex);
int              evas_common_font_utf8_get_prev     (unsigned char *buf, int *iindex);
int              evas_common_font_utf8_get_last     (unsigned char *buf, int buflen);

/****/
void         evas_common_tilebuf_init               (void);

Tilebuf      *evas_common_tilebuf_new               (int w, int h);
void          evas_common_tilebuf_free              (Tilebuf *tb);
void          evas_common_tilebuf_set_tile_size     (Tilebuf *tb, int tw, int th);
void          evas_common_tilebuf_get_tile_size     (Tilebuf *tb, int *tw, int *th);
int           evas_common_tilebuf_add_redraw        (Tilebuf *tb, int x, int y, int w, int h);
int           evas_common_tilebuf_del_redraw        (Tilebuf *tb, int x, int y, int w, int h);
int           evas_common_tilebuf_add_motion_vector (Tilebuf *tb, int x, int y, int w, int h, int dx, int dy, int alpha);
void          evas_common_tilebuf_clear             (Tilebuf *tb);
Tilebuf_Rect *evas_common_tilebuf_get_render_rects  (Tilebuf *tb);
void          evas_common_tilebuf_free_render_rects (Tilebuf_Rect *rects);

/*
Regionbuf    *evas_common_regionbuf_new       (int w, int h);
void          evas_common_regionbuf_free      (Regionbuf *rb);
void          evas_common_regionbuf_clear     (Regionbuf *rb);
void          evas_common_regionbuf_span_add  (Regionbuf *rb, int x1, int x2, int y);
void          evas_common_regionbuf_span_del  (Regionbuf *rb, int x1, int x2, int y);
Tilebuf_Rect *evas_common_regionbuf_rects_get (Regionbuf *rb);
*/
   
/****/
void               evas_common_draw_init                      (void);

RGBA_Draw_Context *evas_common_draw_context_new               (void);
void               evas_common_draw_context_free              (RGBA_Draw_Context *dc);
void               evas_common_draw_context_font_ext_set      (RGBA_Draw_Context *dc,
							 void *data,
							 void *(*gl_new)  (void *data, RGBA_Font_Glyph *fg),
							 void  (*gl_free) (void *ext_dat),
							 void  (*gl_draw) (void *data, void *dest, void *context, RGBA_Font_Glyph *fg, int x, int y));
void               evas_common_draw_context_clip_clip         (RGBA_Draw_Context *dc, int x, int y, int w, int h);
void               evas_common_draw_context_set_clip          (RGBA_Draw_Context *dc, int x, int y, int w, int h);
void               evas_common_draw_context_unset_clip        (RGBA_Draw_Context *dc);
void               evas_common_draw_context_set_color         (RGBA_Draw_Context *dc, int r, int g, int b, int a);
void               evas_common_draw_context_set_multiplier    (RGBA_Draw_Context *dc, int r, int g, int b, int a);
void               evas_common_draw_context_unset_multiplier  (RGBA_Draw_Context *dc);
void               evas_common_draw_context_set_modifiers     (RGBA_Draw_Context *dc, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod);
void               evas_common_draw_context_ununset_modifiers (RGBA_Draw_Context *dc);
void               evas_common_draw_context_add_cutout        (RGBA_Draw_Context *dc, int x, int y, int w, int h);
void               evas_common_draw_context_clear_cutouts     (RGBA_Draw_Context *dc);
Cutout_Rect       *evas_common_draw_context_apply_cutouts     (RGBA_Draw_Context *dc);
void               evas_common_draw_context_apply_free_cutouts(Cutout_Rect *rects);
Cutout_Rect       *evas_common_draw_context_cutouts_split     (Cutout_Rect *in, Cutout_Rect *split);
Cutout_Rect       *evas_common_draw_context_cutout_split      (Cutout_Rect *in, Cutout_Rect *split);
Cutout_Rect       *evas_common_draw_context_cutout_merge      (Cutout_Rect *in, Cutout_Rect *merge);
void               evas_common_draw_context_set_anti_alias    (RGBA_Draw_Context *dc, unsigned char aa);
void               evas_common_draw_context_set_color_interpolation    (RGBA_Draw_Context *dc, int color_space);

Gfx_Func_Blend_Src_Dst           evas_common_draw_func_blend_get       (RGBA_Image *src, RGBA_Image *dst, int pixels);
Gfx_Func_Blend_Color_Dst         evas_common_draw_func_blend_color_get (DATA32 src, RGBA_Image *dst, int pixels);
Gfx_Func_Blend_Src_Cmod_Dst      evas_common_draw_func_blend_cmod_get  (RGBA_Image *src, RGBA_Image *dst, int pixels);
Gfx_Func_Blend_Src_Mul_Dst       evas_common_draw_func_blend_mul_get   (RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels);
Gfx_Func_Blend_Src_Alpha_Mul_Dst evas_common_draw_func_blend_alpha_get (RGBA_Image *dst);
Gfx_Func_Blend_Src_Dst           evas_common_draw_func_copy_get        (int pixels, int reverse);

void              evas_font_dir_cache_free(void);

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

/* FIXME: need... */

/* modular image loader system (from ram, or fd) */
/* loaders: png, jpg, ppm, pgm, argb */
/* finish renderers for lower bit depths & color allocator */

/* and perhaps later on...                               */
/*   multiply pixels by pixels                           */
/*   oval / oval segment (arc) (filled/unfilled)         */
/*   radial gradient fill                                */
/*   my own font renderer that can load bdf & pcf fonts? */
