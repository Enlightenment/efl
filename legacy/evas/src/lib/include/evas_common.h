#ifndef EVAS_COMMON_H
#define EVAS_COMMON_H

#include "config.h"

/*****************************************************************************/

#include "evas_options.h"

#if ( \
         defined __i386__ || \
         defined __386__ || \
         defined __X86__ || \
         defined _M_IX86 || \
         defined i386)
#else
# undef BUILD_MMX
# undef BUILD_SSE
# ifndef BUILD_C
#  define BUILD_C
# endif
#endif

/*****************************************************************************/

#undef MIN
#define MIN(_x, _y) \
(((_x) < (_y)) ? (_x) : (_y))
#undef MAX
#define MAX(_x, _y) \
(((_x) > (_y)) ? (_x) : (_y))

#define POLY_EDGE_DEL(_i)                                               \
{                                                                       \
   int _j;                                                              \
                                                                        \
   for (_j = 0; (_j < num_active_edges) && (edges[_j].i != _i); _j++);  \
   if (_j < num_active_edges)                                           \
     {                                                                  \
	num_active_edges--;                                             \
	memmove(&(edges[_j]), &(edges[_j + 1]),                         \
	        (num_active_edges - _j) * sizeof(RGBA_Edge));           \
     }                                                                  \
}

#define POLY_EDGE_ADD(_i, _y)                                           \
{                                                                       \
   int _j;                                                              \
   float _dx;                                                           \
   RGBA_Vertex *_p, *_q;                                                \
   if (_i < (n - 1)) _j = _i + 1;                                       \
   else _j = 0;                                                         \
   if (point[_i].y < point[_j].y)                                       \
     {                                                                  \
	_p = &(point[_i]);                                              \
	_q = &(point[_j]);                                              \
     }                                                                  \
   else                                                                 \
     {                                                                  \
	_p = &(point[_j]);                                              \
	_q = &(point[_i]);                                              \
     }                                                                  \
   edges[num_active_edges].dx = _dx = (_q->x - _p->x) / (_q->y - _p->y); \
   edges[num_active_edges].x = (_dx * ((float)_y + 0.5 - _p->y)) + _p->x; \
   edges[num_active_edges].i = _i;                                      \
   num_active_edges++;                                                  \
}

#define INTERP_VAL(out, in1, in2, in3, in4, interp_x, interp_y)    \
   {                                                               \
      int _v, _vv;                                                 \
                                                                   \
      _v = (256 - (interp_x)) * (in1);                             \
      if ((interp_x) > 0) _v += (interp_x) * (in2);                \
      _v *= (256 - (interp_y));                                    \
      if ((interp_y) > 0)                                          \
	{                                                          \
	   _vv = (256 - (interp_x)) * (in3);                       \
	   if ((interp_x) > 0) _vv += (interp_x) * (in4);          \
	   _vv *= (interp_y);                                      \
	   (out) = ((_v + _vv) >> 16);                             \
	}                                                          \
      else (out) = (_v >> 16);                                     \
   }
#define INTERP_2(in1, in2, interp, interp_inv) \
   ((in1 * interp_inv) + (in2 * interp)) >> 8

#define SWAP32(x) (x) = \
   ((((x) & 0x000000ff ) << 24) |\
       (((x) & 0x0000ff00 ) << 8) |\
       (((x) & 0x00ff0000 ) >> 8) |\
       (((x) & 0xff000000 ) >> 24))

#define SWAP16(x) (x) = \
   ((((x) & 0x00ff ) << 8) |\
       (((x) & 0xff00 ) >> 8))

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>

#ifdef _WIN32_WCE
#include <windows.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

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

typedef struct _Evas_List             Evas_List;
typedef struct _Evas_Hash             Evas_Hash;
typedef struct _Evas_Hash_El          Evas_Hash_El;
#ifndef EVAS_PRIVATE_H
#if 1 /* able to change co-ordinate systems to remove all fp ops */
typedef int                           Evas_Bool;
#else
typedef char                          Evas_Bool;
#endif
#endif

typedef struct _RGBA_Image            RGBA_Image;
typedef struct _RGBA_Surface          RGBA_Surface;
typedef struct _RGBA_Draw_Context     RGBA_Draw_Context;
typedef struct _RGBA_Gradient         RGBA_Gradient;
typedef struct _RGBA_Gradient_Color   RGBA_Gradient_Color;
typedef struct _RGBA_Polygon_Point    RGBA_Polygon_Point;
typedef struct _RGBA_Font             RGBA_Font;
typedef struct _RGBA_Font_Glyph       RGBA_Font_Glyph;

typedef struct _Cutout_Rect           Cutout_Rect;

typedef struct _Convert_Pal           Convert_Pal;

typedef struct _Tilebuf               Tilebuf;
typedef struct _Tilebuf_Tile          Tilebuf_Tile;
typedef struct _Tilebuf_Rect          Tilebuf_Rect;

typedef void (*Gfx_Func_Blend_Src_Dst) (DATA32 *src, DATA32 *dst, int len);
typedef void (*Gfx_Func_Blend_Color_Dst) (DATA32 src, DATA32 *dst, int len);
typedef void (*Gfx_Func_Blend_Src_Cmod_Dst) (DATA32 *src, DATA32 *dst, int len, DATA8 *rmod, DATA8 *gmod, DATA8 *bmod, DATA8 *amod);
typedef void (*Gfx_Func_Blend_Src_Mul_Dst) (DATA32 *src, DATA32 *dst, int len, DATA32 mul_color);
typedef void (*Gfx_Func_Blend_Src_Alpha_Mul_Dst) (DATA8 *src, DATA32 *dst, int len, DATA32 col);

typedef void (*Gfx_Func_Convert) (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

/*****************************************************************************/

typedef enum _RGBA_Image_Flags
{
   RGBA_IMAGE_NOTHING   = (0),
   RGBA_IMAGE_HAS_ALPHA = (1 << 0),
   RGBA_IMAGE_IS_DIRTY  = (1 << 1),
   RGBA_IMAGE_INDEXED   = (1 << 2)
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

/*****************************************************************************/

struct _Evas_Object_List
{
   Evas_Object_List  *next, *prev;
   Evas_Object_List  *last;
};

struct _Evas_List
{
   void      *data;
   Evas_List *next, *prev;
   Evas_List *last;
   int        count;
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
};

struct _RGBA_Surface
{
   int     w, h;
   DATA32 *data;
   char    no_free : 1;
};

struct _RGBA_Image
{
   Evas_Object_List     _list_data;
   RGBA_Surface        *image;
   RGBA_Image_Flags     flags;
   struct 
     {
	int             num;
	RGBA_Surface  **levels;
     } mipmaps;
   struct 
     {
	int             format;
	char           *file;
	char           *real_file;
	char           *key;
	char           *comment;
     } info;
   int                  references;
   DATA64               timestamp;
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
};

struct _RGBA_Polygon_Point
{
   Evas_Object_List  _list_data;
   int               x, y;
};

struct _RGBA_Font
{
   Evas_Object_List  _list_data;
   char             *name;
   char             *file;
   int               size;
   
   struct {
      FT_Face       face;
   } ft;
   
   Evas_Hash       *glyphs;

   int              usage;
   
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
   
   struct {
      int           w, h;
      Tilebuf_Tile *tiles;
   } tiles;
};

struct _Tilebuf_Tile
{
   int redraw : 1;
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

#define BLEND_COLOR(a, nc, c, cc, tmp)                \
{                                                     \
  (tmp) = ((c) - (cc)) * (a);                         \
  (nc) = (cc) + (((tmp) + ((tmp) >> 8) + 0x80) >> 8); \
}

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


/*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
   
Evas_List *evas_list_append             (Evas_List *list, const void *data);
Evas_List *evas_list_prepend            (Evas_List *list, const void *data);
Evas_List *evas_list_append_relative    (Evas_List *list, const void *data, const void *relative);
Evas_List *evas_list_prepend_relative   (Evas_List *list, const void *data, const void *relative);
Evas_List *evas_list_remove             (Evas_List *list, const void *data);
Evas_List *evas_list_remove_list        (Evas_List *list, Evas_List *remove_list);
void      *evas_list_find               (Evas_List *list, const void *data);
Evas_List *evas_list_find_list          (Evas_List *list, const void *data);
Evas_List *evas_list_free               (Evas_List *list);
Evas_List *evas_list_last               (Evas_List *list);
Evas_List *evas_list_next               (Evas_List *list);
Evas_List *evas_list_prev               (Evas_List *list);
void      *evas_list_data               (Evas_List *list);
int        evas_list_count              (Evas_List *list);
void      *evas_list_nth                (Evas_List *list, int n);
Evas_List *evas_list_nth_list           (Evas_List *list, int n);
Evas_List *evas_list_reverse            (Evas_List *list);
int        evas_list_alloc_error        (void);
   
Evas_Hash *evas_hash_add                (Evas_Hash *hash, const char *key, const void *data);
Evas_Hash *evas_hash_del                (Evas_Hash *hash, const char *key, const void *data);
void      *evas_hash_find               (Evas_Hash *hash, const char *key);
int        evas_hash_size               (Evas_Hash *hash);
void       evas_hash_free               (Evas_Hash *hash);
void       evas_hash_foreach            (Evas_Hash *hash, Evas_Bool (*func) (Evas_Hash *hash, const char *key, void *data, void *fdata), const void *fdata);
int        evas_hash_alloc_error        (void);
   
void *evas_object_list_append           (void *in_list, void *in_item);
void *evas_object_list_prepend          (void *in_list, void *in_item);
void *evas_object_list_append_relative  (void *in_list, void *in_item, void *in_relative);
void *evas_object_list_prepend_relative (void *in_list, void *in_item, void *in_relative);
void *evas_object_list_remove           (void *in_list, void *in_item);
void *evas_object_list_find             (void *in_list, void *in_item);
    
/****/
void evas_common_cpu_init                               (void);

int  evas_common_cpu_have_cpuid                         (void);
int  evas_common_cpu_has_feature                        (unsigned int feature);
void evas_common_cpu_can_do                             (int *mmx, int *sse, int *sse2);
void evas_common_cpu_end_opt                            (void);
    
/****/
void evas_common_blend_init                             (void);

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

void evas_common_scale_rgba_gen_mipmap_table            (RGBA_Image *src);
void evas_common_scale_rgba_gen_mipmap_level            (RGBA_Image *src, int l);
int  evas_common_scale_rgba_get_mipmap_surfaces         (RGBA_Image *src, int src_region_w, int src_region_h, int dst_region_w, int dst_region_h, RGBA_Surface **srf1, RGBA_Surface **srf2);
RGBA_Surface *evas_common_scale_rgba_get_mipmap_surface (RGBA_Image *src, int src_region_w, int src_region_h, int dst_region_w, int dst_region_h);
  
void evas_common_scale_rgba_in_to_out_clip_smooth_mmx  (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
void evas_common_scale_rgba_in_to_out_clip_smooth_c    (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

void evas_common_scale_rgba_in_to_out_clip_smooth      (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
void evas_common_scale_rgba_in_to_out_clip_sample      (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

/****/
void          evas_common_image_init              (void);

RGBA_Surface *evas_common_image_surface_new       (void);
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

RGBA_Image         *evas_common_load_image_from_file     (const char *file, const char *key);
void                evas_common_load_image_data_from_file(RGBA_Image *im);

/****/
void           evas_common_rectangle_init          (void);

void           evas_common_rectangle_draw          (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

/****/
void           evas_common_gradient_init           (void);

RGBA_Gradient *evas_common_gradient_new            (void);
void           evas_common_gradient_free           (RGBA_Gradient *gr);
void           evas_common_gradient_colors_clear   (RGBA_Gradient *gr);
void           evas_common_gradient_color_add      (RGBA_Gradient *gr, int r, int g, int b, int a, int dist);
void           evas_common_gradient_draw           (RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, RGBA_Gradient *gr, double angle);
DATA32        *evas_common_gradient_map            (RGBA_Gradient *gr, RGBA_Draw_Context *dc, int len);

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

RGBA_Font       *evas_common_font_load              (const char *name, int size);
void             evas_common_font_free              (RGBA_Font *fn);
void             evas_common_font_modify_cache_by   (RGBA_Font *fn, int dir);
int              evas_common_font_cache_get         (void);
void             evas_common_font_cache_set         (int size);
void             evas_common_font_flush             (void);
void             evas_common_font_flush_last        (void);
RGBA_Font       *evas_common_font_find              (const char *name, int size);
void             evas_common_font_draw              (RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Font *fn, int x, int y, const char *text);
RGBA_Font_Glyph *evas_common_font_cache_glyph_get   (RGBA_Font *fn, FT_UInt index);
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
