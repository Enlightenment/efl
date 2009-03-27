/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef EVAS_COMMON_H
#define EVAS_COMMON_H

#ifdef HAVE_CONFIG_H
# include "config.h"  /* so that EAPI in Evas.h is correctly defined */
#endif

#include <Eina.h>
#include "Evas.h"

/*****************************************************************************/

#include "evas_options.h"

#if defined(__ARM_ARCH_3M__)
# define __ARM_ARCH__ 40
#endif
#if defined(__ARM_ARCH_4__)
# define __ARM_ARCH__ 40
#endif
#if defined(__ARM_ARCH_4T__)
# define __ARM_ARCH__ 41
#endif

#if defined(__ARM_ARCH_5__)
# define __ARM_ARCH__ 50
#endif
#if defined(__ARM_ARCH_5T__)
# define __ARM_ARCH__ 51
#endif
#if defined(__ARM_ARCH_5E__)
# define __ARM_ARCH__ 52
#endif
#if defined(__ARM_ARCH_5TE__)
# define __ARM_ARCH__ 53
#endif
#if defined(__ARM_ARCH_5TEJ__)
# define __ARM_ARCH__ 54
#endif

#if defined(__ARM_ARCH_6__)
# define __ARM_ARCH__ 60
#endif
#if defined(__ARM_ARCH_6J__)
# define __ARM_ARCH__ 61
#endif
#if defined(__ARM_ARCH_6K__)
# define __ARM_ARCH__ 62
#endif
#if defined(__ARM_ARCH_6Z__)
# define __ARM_ARCH__ 63
#endif
#if defined(__ARM_ARCH_6ZK__)
# define __ARM_ARCH__ 64
#endif
#if defined(__ARM_ARCH_6T2__)
# define __ARM_ARCH__ 65
#endif

#if defined(__ARM_ARCH_7__)
# define __ARM_ARCH__ 70
#endif
#if defined(__ARM_ARCH_7A__)
# define __ARM_ARCH__ 71
#endif
#if defined(__ARM_ARCH_7R__)
# define __ARM_ARCH__ 72
#endif
#if defined(__ARM_ARCH_7M__)
# define __ARM_ARCH__ 73
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef BUILD_PTHREAD
#undef BUILD_PIPE_RENDER
#endif

#ifdef BUILD_PTHREAD

#ifndef __USE_GNU
#define __USE_GNU
#endif

# include <pthread.h>
# include <sched.h>
# define LK(x)  pthread_mutex_t x
# define LKI(x) pthread_mutex_init(&(x), NULL);
# define LKD(x) pthread_mutex_destroy(&(x));
# define LKL(x) pthread_mutex_lock(&(x));
# define LKU(x) pthread_mutex_unlock(&(x));
# define TH(x)  pthread_t x
# define THI(x) int x
# define TH_MAX 8

// even though in theory having every Nth rendered line done by a different
// thread might even out load across threads - it actually slows things down.
//#define EVAS_SLI 1

#else
# define LK(x)
# define LKI(x)
# define LKD(x)
# define LKL(x)
# define LKU(x)
# define TH(x)
# define THI(x)
# define TH_MAX 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_SIZES_H
#include FT_MODULE_H

#ifdef __GNUC__
# if __GNUC__ >= 4
// BROKEN in gcc 4 on amd64
//#  pragma GCC visibility push(hidden)
# endif
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
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

/* if more than 1/ALPHA_SPARSE_INV_FRACTION is "alpha" (1-254) then sparse
 * alpha flag gets set */
#define ALPHA_SPARSE_INV_FRACTION 3

/*****************************************************************************/

#if defined(__ARM_ARCH_3M__)
# define __ARM_ARCH__ 40
#endif
#if defined(__ARM_ARCH_4__)
# define __ARM_ARCH__ 40
#endif
#if defined(__ARM_ARCH_4T__)
# define __ARM_ARCH__ 41
#endif

#if defined(__ARM_ARCH_5__)
# define __ARM_ARCH__ 50
#endif
#if defined(__ARM_ARCH_5T__)
# define __ARM_ARCH__ 51
#endif
#if defined(__ARM_ARCH_5E__)
# define __ARM_ARCH__ 52
#endif
#if defined(__ARM_ARCH_5TE__)
# define __ARM_ARCH__ 53
#endif
#if defined(__ARM_ARCH_5TEJ__)
# define __ARM_ARCH__ 54
#endif

#if defined(__ARM_ARCH_6__)
# define __ARM_ARCH__ 60
#endif
#if defined(__ARM_ARCH_6J__)
# define __ARM_ARCH__ 61
#endif
#if defined(__ARM_ARCH_6K__)
# define __ARM_ARCH__ 62
#endif
#if defined(__ARM_ARCH_6Z__)
# define __ARM_ARCH__ 63
#endif
#if defined(__ARM_ARCH_6ZK__)
# define __ARM_ARCH__ 64
#endif
#if defined(__ARM_ARCH_6T2__)
# define __ARM_ARCH__ 65
#endif

#if defined(__ARM_ARCH_7__)
# define __ARM_ARCH__ 70
#endif
#if defined(__ARM_ARCH_7A__)
# define __ARM_ARCH__ 71
#endif
#if defined(__ARM_ARCH_7R__)
# define __ARM_ARCH__ 72
#endif
#if defined(__ARM_ARCH_7M__)
# define __ARM_ARCH__ 73
#endif

#if defined(__ARM_ARCH__) && (__ARM_ARCH__ >= 52)
/* tested on ARMv6 (arm1136j-s), Nokia N800 CPU */
#define pld(addr, off)                                                  \
   __asm__("pld [%[address], %[offset]]"::                              \
           [address] "r" (addr), [offset] "i" (off))
#else
#define pld(addr, off)
#endif /* __ARMEL__ */

/*****************************************************************************/

#define UNROLL2(op...) op op
#define UNROLL4(op...) UNROLL2(op) UNROLL2(op)
#define UNROLL8(op...) UNROLL4(op) UNROLL4(op)
#define UNROLL16(op...) UNROLL8(op) UNROLL8(op)

#define UNROLL8_PLD_WHILE(start, size, end, op)         \
    pld(start, 0);                                      \
    end = start + (size & ~7);                          \
    while (start < end)                                 \
        {                                               \
            pld(start, 32);                             \
            UNROLL8(op);                                \
        }                                               \
    end += (size & 7);                                  \
    pld(start, 32);                                     \
    while (start <  end)                                \
        {                                               \
        op;                                             \
        }

/*****************************************************************************/

typedef unsigned long long		DATA64;
typedef unsigned int			DATA32;
typedef unsigned short			DATA16;
typedef unsigned char                   DATA8;

typedef struct _Image_Entry             Image_Entry;
typedef struct _Image_Entry_Flags	Image_Entry_Flags;
typedef struct _Engine_Image_Entry      Engine_Image_Entry;
typedef struct _Evas_Cache_Target       Evas_Cache_Target;

typedef struct _RGBA_Image_Loadopts   RGBA_Image_Loadopts;
#ifdef BUILD_PIPE_RENDER
typedef struct _RGBA_Pipe_Op          RGBA_Pipe_Op;
typedef struct _RGBA_Pipe             RGBA_Pipe;
typedef struct _RGBA_Pipe_Thread_Info RGBA_Pipe_Thread_Info;
#endif
typedef struct _RGBA_Image            RGBA_Image;
typedef struct _RGBA_Image_Span       RGBA_Image_Span;
typedef struct _RGBA_Draw_Context     RGBA_Draw_Context;
typedef struct _RGBA_Gradient         RGBA_Gradient;
typedef struct _RGBA_Gradient_Color_Stop   RGBA_Gradient_Color_Stop;
typedef struct _RGBA_Gradient_Alpha_Stop   RGBA_Gradient_Alpha_Stop;
typedef struct _RGBA_Gradient_Type    RGBA_Gradient_Type;
typedef struct _RGBA_Gradient2         RGBA_Gradient2;
typedef struct _RGBA_Gradient2_Type    RGBA_Gradient2_Type;
typedef struct _RGBA_Gradient2_Color_Np_Stop   RGBA_Gradient2_Color_Np_Stop;
typedef struct _RGBA_Polygon_Point    RGBA_Polygon_Point;
typedef struct _RGBA_Font             RGBA_Font;
typedef struct _RGBA_Font_Int         RGBA_Font_Int;
typedef struct _RGBA_Font_Source      RGBA_Font_Source;
typedef struct _RGBA_Font_Glyph       RGBA_Font_Glyph;
typedef struct _RGBA_Gfx_Compositor   RGBA_Gfx_Compositor;

typedef struct _Cutout_Rect           Cutout_Rect;
typedef struct _Cutout_Rects            Cutout_Rects;

typedef struct _Convert_Pal             Convert_Pal;

typedef struct _Tilebuf                 Tilebuf;
typedef struct _Tilebuf_Tile            Tilebuf_Tile;
typedef struct _Tilebuf_Rect		Tilebuf_Rect;

typedef struct _Evas_Common_Transform        Evas_Common_Transform;

/*
typedef struct _Regionbuf             Regionbuf;
typedef struct _Regionspan            Regionspan;
*/

typedef void (*RGBA_Gfx_Func)    (DATA32 *src, DATA8 *mask, DATA32 col, DATA32 *dst, int len);
typedef void (*RGBA_Gfx_Pt_Func) (DATA32 src, DATA8 mask, DATA32 col, DATA32 *dst);
typedef void (*Gfx_Func_Copy)    (DATA32 *src, DATA32 *dst, int len);

typedef void (*Gfx_Func_Convert) (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);

typedef void (*Gfx_Func_Gradient_Fill)(DATA32 *src, int src_len,
                                         DATA32 *dst, DATA8 *mask, int len,
                                         int x, int y, int axx, int axy, int ayx, int ayy,
                                         void *geom_data);

typedef void (*Gfx_Func_Gradient2_Fill)(DATA32 *src, int src_len,
                                         DATA32 *dst, DATA8 *mask, int len,
                                         int x, int y,
                                         void *geom_data);

#include "../cache/evas_cache.h"

/*****************************************************************************/

typedef enum _RGBA_Image_Flags
{
   RGBA_IMAGE_NOTHING       = (0),
/*    RGBA_IMAGE_HAS_ALPHA     = (1 << 0), */
   RGBA_IMAGE_IS_DIRTY      = (1 << 1),
   RGBA_IMAGE_INDEXED       = (1 << 2),
   RGBA_IMAGE_ALPHA_ONLY    = (1 << 3),
   RGBA_IMAGE_ALPHA_TILES   = (1 << 4),
/*    RGBA_IMAGE_ALPHA_SPARSE  = (1 << 5), */
/*    RGBA_IMAGE_LOADED        = (1 << 6), */
/*    RGBA_IMAGE_NEED_DATA     = (1 << 7) */
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
   PAL_MODE_RGB332,
   PAL_MODE_RGB666,
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
   CPU_FEATURE_VIS2    = (1 << 5),
   CPU_FEATURE_NEON    = (1 << 6)
} CPU_Features;

typedef enum _Font_Hint_Flags
{
   FONT_NO_HINT,
   FONT_AUTO_HINT,
   FONT_BYTECODE_HINT
} Font_Hint_Flags;

/*****************************************************************************/

struct _RGBA_Image_Loadopts
{
   int                  scale_down_by; // if > 1 then use this
   double               dpi; // if > 0.0 use this
   int                  w, h; // if > 0 use this
};

struct _Image_Entry_Flags
{
   Evas_Bool loaded       : 1;
   Evas_Bool dirty        : 1;
   Evas_Bool activ        : 1;
   Evas_Bool need_data    : 1;
   Evas_Bool lru_nodata   : 1;
   Evas_Bool cached       : 1;
   Evas_Bool alpha        : 1;
   Evas_Bool alpha_sparse : 1;
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Bool preload      : 1;
#endif
};

struct _Evas_Cache_Target
{
  EINA_INLIST;
  const void *target;
};

struct _Image_Entry
{
   EINA_INLIST;

   Evas_Cache_Image      *cache;
   
   const char            *cache_key;
   
   const char            *file;
   const char            *key;
   
   Evas_Cache_Target     *targets;
   
   time_t                 timestamp;
   time_t                 laststat;
   
   int                    references;
   
   unsigned char          scale;
   
   RGBA_Image_Loadopts    load_opts;
   int                    space;
   int                    w;
   int                    h;
   
   struct
     {
        int		 w;
        int		 h;
     } allocated;
   
   struct
     {
        void		*module;
        void		*loader;
     } info;
   
#ifdef BUILD_ASYNC_PRELOAD
   LK(lock);
#endif

   Image_Entry_Flags      flags;
};

struct _Engine_Image_Entry
{
   EINA_INLIST;

   /* Upper Engine data. */
   Image_Entry                  *src;

   /* Cache stuff. */
   Evas_Cache_Engine_Image      *cache;
   const char                   *cache_key;

   struct
   {
     Evas_Bool                   cached : 1;
     Evas_Bool                   activ : 1;
     Evas_Bool                   dirty : 1;
     Evas_Bool                   loaded : 1;
     Evas_Bool                   need_parent : 1;
   } flags;

   int                           references;
   int                           w;
   int                           h;
};

struct _Cutout_Rect
{
   int               x, y, w, h;
};

struct _Cutout_Rects
{
   Cutout_Rect*      rects;
   int               active;
   int               max;
};

struct _Evas_Common_Transform
{
   float  mxx, mxy, mxz;
   float  myx, myy, myz;
   float  mzx, mzy, mzz;
};

struct _RGBA_Draw_Context
{
   struct {
      Evas_Bool use : 1;
      DATA32 col;
   } mul;
   struct {
      DATA32 col;
   } col;
   struct RGBA_Draw_Context_clip {
      int    x, y, w, h;
      Evas_Bool use : 1;
   } clip;
   Cutout_Rects cutout;
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
   struct {
      int y, h;
   } sli;
   int            render_op;
   Evas_Bool anti_alias : 1;
};

#ifdef BUILD_PIPE_RENDER
struct _RGBA_Pipe_Op
{
   RGBA_Draw_Context         context;
   void                    (*op_func) (RGBA_Image *dst, RGBA_Pipe_Op *op, RGBA_Pipe_Thread_Info *info);
   void                    (*free_func) (RGBA_Pipe_Op *op);

   union {
      struct {
	 int                 x, y, w, h;
      } rect;
      struct {
	 int                 x0, y0, x1, y1;
      } line;
      struct {
	 RGBA_Polygon_Point *points;
      } poly;
      struct {
	 RGBA_Gradient      *grad;
	 int                 x, y, w, h;
      } grad;
      struct {
	 RGBA_Gradient2      *grad;
	 int                 x, y, w, h;
      } grad2;
      struct {
	 RGBA_Font          *font;
	 int                 x, y;
	 char               *text;
      } text;
      struct {
	 RGBA_Image         *src;
	 int                 sx, sy, sw, sh, dx, dy, dw, dh;
	 int                 smooth;
	 char               *text;
      } image;
   } op;
};

#define PIPE_LEN 256

struct _RGBA_Pipe
{
   EINA_INLIST;
   int               op_num;
   RGBA_Pipe_Op      op[PIPE_LEN];
};

struct _RGBA_Pipe_Thread_Info
{
   RGBA_Image *im;
   int         x, y, w, h;
};
#endif

struct _RGBA_Image
{
   Image_Entry          cache_entry;

   RGBA_Image_Flags     flags;
   struct
     {
/* 	void           *module; */
/* 	void           *loader; */
/* 	char           *real_file; */
	char           *comment;
//	int             format;
     } info;

   void                *extended_info;
#ifdef BUILD_PIPE_RENDER
   RGBA_Pipe           *pipe;
#endif   
   int                  ref;

/*    unsigned char        scale; */

   /* Colorspace stuff. */
   struct {
      void              *data;
      Evas_Bool          no_free : 1;
      Evas_Bool          dirty : 1;
   } cs;

   /* RGBA stuff */
   struct {
      DATA32            *data;
      Evas_Bool          no_free : 1;
   } image;
   
   struct {
      LK(lock);
      Eina_List *list;
      unsigned long long orig_usage;
      unsigned long long usage_count;
      int populate_count;
      unsigned long long newest_usage;
      unsigned long long newest_usage_count;
   } cache;
};

struct _RGBA_Gradient_Color_Stop
{
   EINA_INLIST;
   int               r, g, b, a;
   int               dist;
};

struct _RGBA_Gradient_Alpha_Stop
{
   EINA_INLIST;
   int               a;
   int               dist;
};

struct _RGBA_Gradient
{
   struct
     {
	DATA32        *data;
	int            len;
	float          angle;
	int            direction;
	float          offset;
	Evas_Bool      has_alpha : 1;
     } map;

   struct {
	Eina_Inlist *stops;
	DATA32           *data;
	int               nstops;
	int               len;
   }  color;
   struct {
	Eina_Inlist *stops;
	DATA8            *data;
	int               nstops;
	int               len;
   }  alpha;

   struct
     {
	int            x, y, w, h;
	int            spread;
	float          angle;
     } fill;
   struct
     {
	char          *name;
	char          *params;
	RGBA_Gradient_Type *geometer;
	void          *gdata;
     } type;

   int references;

   Evas_Bool imported_data : 1;
   Evas_Bool has_alpha : 1;
};

struct _RGBA_Gradient_Type
{
   const char              *name;
   void                    (*init)(void);
   void                    (*shutdown)(void);
   void                    (*geom_init)(RGBA_Gradient *gr);
   void                    (*geom_set)(RGBA_Gradient *gr);
   void                    (*geom_free)(void *gdata);
   int                     (*has_alpha)(RGBA_Gradient *gr, int render_op);
   int                     (*has_mask)(RGBA_Gradient *gr, int render_op);
   int                     (*get_map_len)(RGBA_Gradient *gr);
   Gfx_Func_Gradient_Fill  (*get_fill_func)(RGBA_Gradient *gr, int render_op, unsigned char aa);
};

struct _RGBA_Gradient2_Color_Np_Stop
{
   EINA_INLIST;
   int               r, g, b, a;
   float             pos;
   int               dist;
};

struct _RGBA_Gradient2
{
   struct
     {
	DATA32        *data;
	int            len;
	Evas_Bool      has_alpha : 1;
     } map;

   struct {
	Eina_Inlist *stops;
	int               nstops;
	DATA32           *cdata;
	DATA8            *adata;
	int               len;
   }  stops;

   struct
     {
	Evas_Common_Transform  transform;
	int                    spread;
     } fill;
   struct
     {
        int                 id;
	RGBA_Gradient2_Type *geometer;
	void                *gdata;
     } type;

   int references;

   Evas_Bool has_alpha : 1;
};

struct _RGBA_Gradient2_Type
{
   const char              *name;
   void                    (*init)(void);
   void                    (*shutdown)(void);
   void                    (*geom_init)(RGBA_Gradient2 *gr);
   void                    (*geom_update)(RGBA_Gradient2 *gr);
   void                    (*geom_free)(void *gdata);
   int                     (*has_alpha)(RGBA_Gradient2 *gr, int render_op);
   int                     (*has_mask)(RGBA_Gradient2 *gr, int render_op);
   int                     (*get_map_len)(RGBA_Gradient2 *gr);
   Gfx_Func_Gradient2_Fill  (*get_fill_func)(RGBA_Gradient2 *gr, int render_op);
};


struct _RGBA_Polygon_Point
{
   EINA_INLIST;
   int               x, y;
};

struct _RGBA_Font
{
   Eina_List *fonts;
   Font_Hint_Flags hinting;
   int references;
   LK(lock);
};

struct _RGBA_Font_Int
{
   EINA_INLIST;

   RGBA_Font_Source *src;

   int               size;
   int               real_size;

   struct {
      FT_Size       size;
   } ft;

   Eina_Hash       *glyphs;

   LK(ft_mutex);
   
   Eina_Hash       *kerning;
   Eina_Hash       *indexes;

   int              usage;
   Font_Hint_Flags hinting;

   int              references;
};

struct _RGBA_Font_Source
{
   EINA_INLIST;

   const char       *name;
   const char       *file;

   void             *data;
   int               data_size;
   int               current_size;
   Evas_Array_Hash  *charmap;

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
   /* this is a problem - only 1 engine at a time can extend such a font... grrr */
   void           *ext_dat;
   void           (*ext_dat_free) (void *ext_dat);
};

struct _RGBA_Gfx_Compositor
{
   const char *name;

   void              (*init)(void);
   void              (*shutdown)(void);

   RGBA_Gfx_Func  (*composite_pixel_span_get)(RGBA_Image *src, RGBA_Image *dst, int pixels);
   RGBA_Gfx_Func  (*composite_color_span_get)(DATA32 col, RGBA_Image *dst, int pixels);
   RGBA_Gfx_Func  (*composite_pixel_color_span_get)(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels);
   RGBA_Gfx_Func  (*composite_mask_color_span_get)(DATA32 col, RGBA_Image *dst, int pixels);
   RGBA_Gfx_Func  (*composite_pixel_mask_span_get)(RGBA_Image *src, RGBA_Image *dst, int pixels);

   RGBA_Gfx_Pt_Func  (*composite_pixel_pt_get)(Image_Entry_Flags src_flags, RGBA_Image *dst);
   RGBA_Gfx_Pt_Func  (*composite_color_pt_get)(DATA32 col, RGBA_Image *dst);
   RGBA_Gfx_Pt_Func  (*composite_pixel_color_pt_get)(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst);
   RGBA_Gfx_Pt_Func  (*composite_mask_color_pt_get)(DATA32 col, RGBA_Image *dst);
   RGBA_Gfx_Pt_Func  (*composite_pixel_mask_pt_get)(Image_Entry_Flags src_flags, RGBA_Image *dst);
};

#define EVAS_RECT_SPLIT 1
#ifdef EVAS_RECT_SPLIT
typedef struct list_node list_node_t;
typedef struct list list_t;
typedef struct rect rect_t;
typedef struct rect_node rect_node_t;

struct list_node
{
    struct list_node *next;
};

struct list
{
    struct list_node *head;
    struct list_node *tail;
};

struct rect
{
    short left;
    short top;
    short right;
    short bottom;
    short width;
    short height;
    int area;
};

struct rect_node
{
    struct list_node _lst;
    struct rect rect;
};

void rect_list_node_pool_set_max(int max);
void rect_list_node_pool_flush(void);
list_node_t *rect_list_node_pool_get(void);
void rect_list_node_pool_put(list_node_t *node);

void rect_init(rect_t *r, int x, int y, int w, int h);
void rect_list_append_node(list_t *rects, list_node_t *node);
void rect_list_append(list_t *rects, const rect_t r);
void rect_list_append_xywh(list_t *rects, int x, int y, int w, int h);
void rect_list_concat(list_t *rects, list_t *other);
list_node_t *rect_list_unlink_next(list_t *rects, list_node_t *parent_node);
void rect_list_del_next(list_t *rects, list_node_t *parent_node);
void rect_list_clear(list_t *rects);
void rect_list_del_split_strict(list_t *rects, const rect_t del_r);
void rect_list_add_split_strict(list_t *rects, list_node_t *node);
list_node_t *rect_list_add_split_fuzzy(list_t *rects, list_node_t *node, int accepted_error);
void rect_list_merge_rects(list_t *rects, list_t *to_merge, int accepted_error);void rect_list_add_split_fuzzy_and_merge(list_t *rects, list_node_t *node, int split_accepted_error, int merge_accepted_error);

void rect_print(const rect_t r);
void rect_list_print(const list_t rects);
#endif /* EVAS_RECT_SPLIT */

struct _Tilebuf
{
   int outbuf_w;
   int outbuf_h;

   struct {
      int           w, h;
   } tile_size;

#ifdef RECTUPDATE
   Regionbuf *rb;
#elif defined(EVAS_RECT_SPLIT)
   int need_merge;
   list_t rects;
#else
   struct {
      int           w, h;
      Tilebuf_Tile *tiles;
   } tiles;
#endif
};

struct _Tilebuf_Tile
{
   Evas_Bool redraw : 1;
/* FIXME: need these flags later - but not now */
/*
   Evas_Bool done   : 1;
   Evas_Bool edge   : 1;
   Evas_Bool from   : 1;

   struct {
      int dx, dy;
   } vector;
 */
};

struct _Tilebuf_Rect
{
   EINA_INLIST;
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
  EINA_INLIST;
   int x1, x2;
};
*/

struct _Convert_Pal
{
   int               references;
   int               count;
   Convert_Pal_Mode  colors;
   DATA8            *lookup;
   void             *data;
};

/****/

/*****************************************************************************/
#include "evas_macros.h"

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

#include "evas_blend_ops.h"

#define _EVAS_RENDER_FILL        -1
#define _EVAS_RENDER_BLEND        0
#define _EVAS_RENDER_BLEND_REL    1
#define _EVAS_RENDER_COPY         2
#define _EVAS_RENDER_COPY_REL     3
#define _EVAS_RENDER_ADD          4
#define _EVAS_RENDER_ADD_REL      5
#define _EVAS_RENDER_SUB          6
#define _EVAS_RENDER_SUB_REL      7
#define _EVAS_RENDER_TINT         8
#define _EVAS_RENDER_TINT_REL     9
#define _EVAS_RENDER_MASK         10
#define _EVAS_RENDER_MUL          11
#define _EVAS_RENDER_CLIP         12

#define _EVAS_TEXTURE_REFLECT           0
#define _EVAS_TEXTURE_REPEAT            1
#define _EVAS_TEXTURE_RESTRICT          2
#define _EVAS_TEXTURE_RESTRICT_REFLECT  3
#define _EVAS_TEXTURE_RESTRICT_REPEAT   4
#define _EVAS_TEXTURE_PAD               5

#define _EVAS_COLOR_SPACE_ARGB    0
#define _EVAS_COLOR_SPACE_AHSV    1

/*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/****/
void evas_common_init                                   (void);
void evas_common_shutdown                               (void);

EAPI void evas_common_cpu_init                          (void);

int  evas_common_cpu_have_cpuid                         (void);
int  evas_common_cpu_has_feature                        (unsigned int feature);
EAPI void evas_common_cpu_can_do                        (int *mmx, int *sse, int *sse2);
EAPI void evas_common_cpu_end_opt                       (void);
EAPI int evas_common_cpu_count                          (void);

/****/
#include "../engines/common/evas_blend.h"

EAPI Gfx_Func_Copy        evas_common_draw_func_copy_get        (int pixels, int reverse);

/****/
#include "../engines/common/evas_convert_color.h"
#include "../engines/common/evas_convert_colorspace.h"
#include "../engines/common/evas_convert_main.h"
#include "../engines/common/evas_convert_yuv.h"
#include "../engines/common/evas_scale_main.h"
#include "../engines/common/evas_scale_smooth.h"
#include "../engines/common/evas_scale_span.h"

/****/
#include "../engines/common/evas_image.h"

/****/
#include "../engines/common/evas_line.h"
#include "../engines/common/evas_polygon.h"
#include "../engines/common/evas_rectangle.h"

/****/
EAPI void     evas_common_blit_init               (void);

EAPI void     evas_common_blit_rectangle          (const RGBA_Image *src, RGBA_Image *dst, int src_x, int src_y, int w, int h, int dst_x, int dst_y);

/****/
#include "../engines/common/evas_gradient.h"
#include "../engines/common/evas_font.h"

/****/
EAPI void          evas_common_tilebuf_init               (void);

EAPI Tilebuf      *evas_common_tilebuf_new               (int w, int h);
EAPI void          evas_common_tilebuf_free              (Tilebuf *tb);
EAPI void          evas_common_tilebuf_set_tile_size     (Tilebuf *tb, int tw, int th);
EAPI void          evas_common_tilebuf_get_tile_size     (Tilebuf *tb, int *tw, int *th);
EAPI int           evas_common_tilebuf_add_redraw        (Tilebuf *tb, int x, int y, int w, int h);
EAPI int           evas_common_tilebuf_del_redraw        (Tilebuf *tb, int x, int y, int w, int h);
EAPI int           evas_common_tilebuf_add_motion_vector (Tilebuf *tb, int x, int y, int w, int h, int dx, int dy, int alpha);
EAPI void          evas_common_tilebuf_clear             (Tilebuf *tb);
EAPI Tilebuf_Rect *evas_common_tilebuf_get_render_rects  (Tilebuf *tb);
EAPI void          evas_common_tilebuf_free_render_rects (Tilebuf_Rect *rects);

/*
Regionbuf    *evas_common_regionbuf_new       (int w, int h);
void          evas_common_regionbuf_free      (Regionbuf *rb);
void          evas_common_regionbuf_clear     (Regionbuf *rb);
void          evas_common_regionbuf_span_add  (Regionbuf *rb, int x1, int x2, int y);
void          evas_common_regionbuf_span_del  (Regionbuf *rb, int x1, int x2, int y);
Tilebuf_Rect *evas_common_regionbuf_rects_get (Regionbuf *rb);
*/

/****/
#include "../engines/common/evas_draw.h"

/****/
#ifdef BUILD_PIPE_RENDER
# include "../engines/common/evas_pipe.h"
#endif
   
void              evas_font_dir_cache_free(void);

/****/

/*****************************************************************************/

#if defined(__ARM_ARCH__) && (__ARM_ARCH__ >= 70)
#ifdef BUILD_NEON
# include <arm_neon.h>
#endif
#endif   
   
#ifdef __cplusplus
}
#endif

#endif
