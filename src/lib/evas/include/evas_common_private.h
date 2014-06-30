#ifndef EVAS_COMMON_H
#define EVAS_COMMON_H

#ifdef HAVE_CONFIG_H
#include "config.h"  /* so that EAPI in Evas.h is correctly defined */
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>

#ifndef _MSC_VER
# include <stdint.h>
#include <unistd.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef HAVE_ESCAPE
# include <Escape.h>
#endif

#ifdef HAVE_PIXMAN
#include <pixman.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_EXOTIC
# include <Exotic.h>
#endif

#include <Eina.h>
#include <Eo.h>

#ifdef BUILD_LOADER_EET
# include <Eet.h>
#endif
#include "Evas.h"
//#include "Evas_GL.h"

#ifndef HAVE_LROUND
/* right now i dont care about rendering bugs on platforms without lround
 (e.g. windows/vc++... yay!)
 FIXME: http://cgit.freedesktop.org/cairo/tree/src/cairo-misc.c#n487
*/
#define lround(x) (((x) < 0) ? (long int)ceil((x) - 0.5) : (long int)floor((x) + 0.5))
#endif

/* macros needed to log message through eina_log */
extern EAPI int _evas_log_dom_global;
#ifdef  _EVAS_DEFAULT_LOG_DOM
# undef _EVAS_DEFAULT_LOG_DOM
#endif
#define _EVAS_DEFAULT_LOG_DOM _evas_log_dom_global

#ifdef EVAS_DEFAULT_LOG_COLOR
# undef EVAS_DEFAULT_LOG_COLOR
#endif
#define EVAS_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_EVAS_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_EVAS_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_EVAS_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_EVAS_DEFAULT_LOG_DOM, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_EVAS_DEFAULT_LOG_DOM, __VA_ARGS__)

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

#define SLK(x) Eina_Spinlock x
#define SLKI(x) eina_spinlock_new(&(x))
#define SLKD(x) eina_spinlock_free(&(x))
#define SLKL(x) eina_spinlock_take(&(x))
#define SLKT(x) eina_spinlock_take_try(&(x))
#define SLKU(x) eina_spinlock_release(&(x))

#define LK(x)  Eina_Lock x
#define LKI(x) eina_lock_new(&(x))
#define LKD(x) eina_lock_free(&(x))
#define LKL(x) eina_lock_take(&(x))
#define LKT(x) eina_lock_take_try(&(x))
#define LKU(x) eina_lock_release(&(x))
#define LKDBG(x) eina_lock_debug(&(x))

/* for rwlocks */
#define RWLK(x) Eina_RWLock x
#define RWLKI(x) eina_rwlock_new(&(x))
#define RWLKD(x) eina_rwlock_free(&(x))
#define RDLKL(x) eina_rwlock_take_read(&(x))
#define WRLKL(x) eina_rwlock_take_write(&(x))
#define RWLKU(x) eina_rwlock_release(&(x))

# define TH(x)  pthread_t x
# define THI(x) int x
# define TH_MAX 8

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
#define IMG_MAX_SIZE 65000

#define IMG_TOO_BIG(w, h) \
   ((((unsigned long long)w) * ((unsigned long long)h)) >= \
       ((1ULL << (29 * (sizeof(void *) / 4))) - 2048))

/* not every converter supports no-dither or line-dither, then they will
 * fallback to table version, in these cases default to small table.
 */
#if defined(BUILD_NO_DITHER_MASK) || defined(BUILD_LINE_DITHER_MASK)
#ifndef BUILD_SMALL_DITHER_MASK
#define BUILD_SMALL_DITHER_MASK 1
#endif
#endif

#ifdef BUILD_SMALL_DITHER_MASK
# define DM_TABLE     _evas_dither_44
# define DM_SIZE      4
# define DM_BITS      4
# define DM_DIV       16
# define USE_DITHER_44 1
#else
# define DM_TABLE     _evas_dither_128128
# define DM_SIZE      128
# define DM_BITS      6
# define DM_DIV       64
# define USE_DITHER_128128 1
#endif

#define DM_MSK       (DM_SIZE - 1)
#define DM_SHF(_b)   (DM_BITS - (8 - _b))
/* Supports negative right shifts */
#define DM_SHR(x, _b)   ((DM_SHF(_b) >= 0) ? \
      ((x) >> DM_SHF(_b)) : ((x) << -DM_SHF(_b)))

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

// these here are in config.h - just here for documentation
//#ifdef __ARM_ARCH__
// *IF* you enable pixman, this determines which things pixman will do
////#define PIXMAN_FONT               1
////#define PIXMAN_RECT               1
////#define PIXMAN_LINE               1
////#define PIXMAN_POLY               1
//#define PIXMAN_IMAGE              1
//#define PIXMAN_IMAGE_SCALE_SAMPLE 1
//#endif
// not related to pixman but an alternate rotate code
//#define TILE_ROTATE               1

#define TILE_CACHE_LINE_SIZE      64

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

#include "../file/evas_module.h"

typedef unsigned long long		DATA64;
typedef unsigned int			DATA32;
typedef unsigned short			DATA16;
typedef unsigned char                   DATA8;

typedef struct _Image_Entry             Image_Entry;
typedef struct _Image_Entry_Task        Image_Entry_Task;
typedef struct _Image_Entry_Flags       Image_Entry_Flags;
typedef struct _Image_Entry_Frame       Image_Entry_Frame;
typedef struct _Image_Timestamp         Image_Timestamp;
typedef struct _Engine_Image_Entry      Engine_Image_Entry;
typedef struct _Evas_Cache_Target       Evas_Cache_Target;
typedef struct _Evas_Preload_Pthread    Evas_Preload_Pthread;

#ifdef BUILD_PIPE_RENDER
typedef struct _RGBA_Pipe_Op          RGBA_Pipe_Op;
typedef struct _RGBA_Pipe             RGBA_Pipe;
typedef struct _RGBA_Pipe_Thread_Info RGBA_Pipe_Thread_Info;
#endif
typedef struct _RGBA_Image            RGBA_Image;
typedef struct _RGBA_Image_Span       RGBA_Image_Span;
typedef struct _RGBA_Draw_Context     RGBA_Draw_Context;
typedef struct _RGBA_Polygon_Point    RGBA_Polygon_Point;
typedef struct _RGBA_Map_Point        RGBA_Map_Point;
typedef struct _RGBA_Map              RGBA_Map;
typedef struct _RGBA_Font             RGBA_Font;
typedef struct _RGBA_Font_Int         RGBA_Font_Int;
typedef struct _RGBA_Font_Source      RGBA_Font_Source;
typedef struct _RGBA_Font_Glyph       RGBA_Font_Glyph;
typedef struct _RGBA_Font_Glyph_Out   RGBA_Font_Glyph_Out;
typedef struct _RGBA_Gfx_Compositor   RGBA_Gfx_Compositor;

typedef struct _Cutout_Rect           Cutout_Rect;
typedef struct _Cutout_Rects            Cutout_Rects;

typedef struct _Convert_Pal             Convert_Pal;

typedef struct _Tilebuf                 Tilebuf;
typedef struct _Tilebuf_Tile            Tilebuf_Tile;
typedef struct _Tilebuf_Rect		Tilebuf_Rect;

typedef struct _Evas_Common_Transform        Evas_Common_Transform;

// RGBA_Map_Point
// all coords are 20.12
// fp type - an int for now
typedef int FPc;
// fp # of bits of float accuracy
#define FP 8
// fp half (half of an fp unit)
#define FPH (1 << (FP - 1))
// one fp unit
#define FP1 (1 << (FP))

/*
typedef struct _Regionbuf             Regionbuf;
typedef struct _Regionspan            Regionspan;
*/

typedef void (*RGBA_Gfx_Func)    (DATA32 *src, DATA8 *mask, DATA32 col, DATA32 *dst, int len);
typedef void (*RGBA_Gfx_Pt_Func) (DATA32 src, DATA8 mask, DATA32 col, DATA32 *dst);
typedef void (*Gfx_Func_Copy)    (DATA32 *src, DATA32 *dst, int len);

typedef void (*Gfx_Func_Convert) (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
typedef void (*Alpha_Gfx_Func)   (DATA8 *src, DATA8 *dst, int len);

typedef void (*Evas_Render_Done_Cb)(void *);

typedef void (*Evas_Engine_Thread_Task_Cb)(void *engine_data, Image_Entry *ie, void *custom_data);

#include "../cache/evas_cache.h"
#ifdef EVAS_CSERVE2
#include "../cache2/evas_cache2.h"
#endif

/*****************************************************************************/

typedef void (*Evas_Thread_Command_Cb)(void *data);
typedef struct _Evas_Thread_Command Evas_Thread_Command;

struct _Evas_Thread_Command
{
   Evas_Thread_Command_Cb cb;
   void *data;
};

/*****************************************************************************/

typedef enum _RGBA_Image_Flags
{
   RGBA_IMAGE_NOTHING       = (0),
/*    RGBA_IMAGE_HAS_ALPHA     = (1 << 0), */
   RGBA_IMAGE_IS_DIRTY      = (1 << 1),
   RGBA_IMAGE_INDEXED       = (1 << 2),
   /* RGBA_IMAGE_ALPHA_ONLY    = (1 << 3), */
   RGBA_IMAGE_ALPHA_TILES   = (1 << 4),
/*    RGBA_IMAGE_ALPHA_SPARSE  = (1 << 5), */
/*    RGBA_IMAGE_LOADED        = (1 << 6), */
/*    RGBA_IMAGE_NEED_DATA     = (1 << 7) */
   RGBA_IMAGE_TODO_LOAD     = (1 << 8),
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
   CPU_FEATURE_NEON    = (1 << 6),
   CPU_FEATURE_SSE3    = (1 << 7)
} CPU_Features;

typedef enum _Font_Hint_Flags
{
   FONT_NO_HINT,
   FONT_AUTO_HINT,
   FONT_BYTECODE_HINT
} Font_Hint_Flags;

typedef enum _Font_Rend_Flags
{
   FONT_REND_REGULAR   = 0,
   FONT_REND_SLANT     = (1 << 0),
   FONT_REND_WEIGHT    = (1 << 1),
} Font_Rend_Flags;

/*****************************************************************************/

struct _Image_Entry_Flags
{
   Eina_Bool loaded        : 1;
   Eina_Bool in_progress   : 1;
   Eina_Bool dirty         : 1;
   Eina_Bool activ         : 1;

   Eina_Bool need_data     : 1;
   Eina_Bool lru_nodata    : 1;
   Eina_Bool cached        : 1;
   Eina_Bool alpha         : 1;

   Eina_Bool lru           : 1;
   Eina_Bool alpha_sparse  : 1;
   Eina_Bool preload_done  : 1;
   Eina_Bool delete_me     : 1;
   
   Eina_Bool pending       : 1;
   Eina_Bool rotated       : 1;
   Eina_Bool unload_cancel : 1;
   Eina_Bool given_mmap    : 1;

   Eina_Bool updated_data  : 1;
};

struct _Image_Entry_Frame
{
   int       index;
   DATA32   *data;     /* frame decoding data */
   void     *info;     /* special image type info */
   Eina_Bool loaded       : 1;
};

struct _Evas_Cache_Target
{
  EINA_INLIST;
  const Eo *target;
  void *data;
};

struct _Image_Timestamp
{
   time_t mtime;
   off_t  size;
   ino_t  ino;
#ifdef _STAT_VER_LINUX
   unsigned long int mtime_nsec;
#endif
};

struct _Image_Entry_Task
{
   Evas_Engine_Thread_Task_Cb cb;
   const void *engine_data;
   const void *custom_data;
};

struct _Image_Entry
{
   EINA_INLIST;

   Evas_Cache_Image      *cache;
#ifdef EVAS_CSERVE2
   Evas_Cache2           *cache2;
#endif

   const char            *cache_key;

   const char            *file;
   const char            *key;

   Evas_Cache_Target     *targets;
   Evas_Preload_Pthread  *preload;
   Eina_List             *tasks;

   Image_Timestamp        tstamp;

   int                    references;

#ifdef BUILD_PIPE_RENDER
   RGBA_Pipe             *pipe;
#endif

   Evas_Image_Load_Opts   load_opts;
   Evas_Colorspace        space;
   const Evas_Colorspace *cspaces; // owned by the loader, live as long as the loader

   unsigned int           w;
   unsigned int           h;

   unsigned char          scale;

   struct
     {
        unsigned int w;
        unsigned int h;
     } allocated;

   struct
     {
        unsigned char l, r, t, b;
     } borders; // Duplicated borders for use by GL engines.

   struct
     {
        Evas_Module          *module;
        Evas_Image_Load_Func *loader;
     } info;

   SLK(lock);
   SLK(lock_cancel);
   SLK(lock_task);

   /* for animation feature */
   Evas_Image_Animated   animated;

   /* Reference to the file */
   Eina_File             *f;
   void                  *loader_data;
  
   Image_Entry_Flags      flags;
   Evas_Image_Scale_Hint  scale_hint;
   void                  *data1, *data2;
#ifdef EVAS_CSERVE2
   unsigned int           open_rid, load_rid, preload_rid;
#endif
   int                    server_id;
   int                    connect_num;
   int                    channel;
   int                    load_error;
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
     Eina_Bool                   cached : 1;
     Eina_Bool                   activ : 1;
     Eina_Bool                   dirty : 1;
     Eina_Bool                   loaded : 1;
     Eina_Bool                   need_parent : 1;
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
      Eina_Bool use : 1;
      DATA32 col;
   } mul;
   struct {
#ifdef HAVE_PIXMAN
   pixman_image_t  *pixman_color_image;
#endif
      DATA32 col;
   } col;
   struct RGBA_Draw_Context_clip {
      int    x, y, w, h;
      Eina_Bool use : 1;
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
   Eina_Bool anti_alias : 1;
};

#ifdef BUILD_PIPE_RENDER
#include "../common/evas_map_image.h"
#include "../common/evas_text_utils.h"

struct _RGBA_Pipe_Op
{
   RGBA_Draw_Context         context;
   Eina_Bool               (*prepare_func) (void *data, RGBA_Image *dst, RGBA_Pipe_Op *op);
   void                    (*op_func) (RGBA_Image *dst, const RGBA_Pipe_Op *op, const RGBA_Pipe_Thread_Info *info);
   void                    (*free_func) (RGBA_Pipe_Op *op);
   Cutout_Rects             *rects;

   union {
      struct {
	 int                 x, y, w, h;
      } rect;
      struct {
	 int                 x0, y0, x1, y1;
      } line;
      struct {
         int                 x, y;
	 RGBA_Polygon_Point *points;
      } poly;
      struct {
	 int                 x, y;
         Evas_Text_Props    *intl_props;
         RGBA_Gfx_Func       func;
      } text;
      struct {
	 RGBA_Image         *src;
	 int                 sx, sy, sw, sh, dx, dy, dw, dh;
	 int                 smooth;
	 char               *text;
      } image;
      struct {
	 RGBA_Image         *src;
	 RGBA_Map	    *m;
	 int                 npoints;
	 int                 smooth;
	 int                 level;
      } map;
   } op;

   Eina_Bool                 render : 1;
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
   EINA_INLIST;
   Eina_Rectangle area;
};
#endif

struct _RGBA_Image
{
   Image_Entry          cache_entry;

   RGBA_Image_Flags     flags;
   struct
     {
/*	void           *module; */
/*	void           *loader; */
/*	char           *real_file; */
	char           *comment;
//	int             format;
     } info;

   void                *extended_info;

/*    unsigned char        scale; */

   /* Colorspace stuff. */
   struct {
      void              *data;
      Eina_Bool          no_free : 1;
      Eina_Bool          dirty : 1;
   } cs;

   /* RGBA stuff */
   struct {
      union {
         DATA32         *data;    /* Normal image */
         DATA8          *data8;   /* Alpha Mask stuff */
      };
      Eina_Bool          no_free : 1;
   } image;

   struct {
      SLK(lock);
      Eina_List *list;
      Eina_Hash *hash;
      unsigned long long orig_usage;
      unsigned long long usage_count;
      int populate_count;
      unsigned long long newest_usage;
      unsigned long long newest_usage_count;
   } cache;

#ifdef HAVE_PIXMAN
   struct {
      pixman_image_t *im;
   } pixman;
#endif
};

struct _RGBA_Polygon_Point
{
   EINA_INLIST;
   int               x, y;
};

struct _RGBA_Map_Point
{
   FPc x, y; // x, y screenspace
   float fx, fy, fz; // x, y, z in floats
//   FPc x3, y3; // x, y 3d space
   FPc z; // z in world space. optional
   FPc u, v; // u, v in tex coords
   DATA32 col; // color at this point
   // for perspective correctness - only point 0 has relevant info
   FPc px, py, z0, foc;
};

struct _RGBA_Map
{
   void *engine_data;

   struct {
      int w, h;
   } image, uv;

   int x, y;
   int count;

   RGBA_Map_Point pts[1];
};

// for fonts...
/////
typedef struct _Fash_Item_Index_Map Fash_Item_Index_Map;
typedef struct _Fash_Int_Map        Fash_Int_Map;
typedef struct _Fash_Int_Map2       Fash_Int_Map2;
typedef struct _Fash_Int            Fash_Int;
struct _Fash_Item_Index_Map
{
   RGBA_Font_Int *fint;
   int            index;
};
struct _Fash_Int_Map
{
  Fash_Item_Index_Map item[256];
};
struct _Fash_Int_Map2
{
   Fash_Int_Map *bucket[256];
};
struct _Fash_Int
{
   Fash_Int_Map2 *bucket[256];
   void (*freeme) (Fash_Int *fash);
};

/////
typedef struct _Fash_Glyph_Map  Fash_Glyph_Map;
typedef struct _Fash_Glyph_Map2 Fash_Glyph_Map2;
typedef struct _Fash_Glyph      Fash_Glyph;
struct _Fash_Glyph_Map
{
   RGBA_Font_Glyph *item[256];
};
struct _Fash_Glyph_Map2
{
   Fash_Glyph_Map *bucket[256];
};
struct _Fash_Glyph
{
   Fash_Glyph_Map2 *bucket[256];
   void (*freeme) (Fash_Glyph *fash);
};
/////

struct _RGBA_Font
{
   Eina_List       *fonts;
   Fash_Int        *fash;
   Font_Hint_Flags  hinting;
   int              references;
   LK(lock);
   unsigned char    sizeok : 1;
};

#include "../common/evas_font_ot.h"

struct _RGBA_Font_Int
{
   EINA_INLIST;
   RGBA_Font_Source *src;
   Eina_Hash        *kerning;
   Fash_Glyph       *fash;
   unsigned int      size;
   int               real_size;
   int               max_h;
   int               references;
   int               usage;
   struct {
      FT_Size       size;
#ifdef USE_HARFBUZZ
      void         *hb_font;
#endif
   } ft;
   LK(ft_mutex);
   Font_Hint_Flags  hinting;
   Font_Rend_Flags  wanted_rend; /* The wanted rendering style */
   Font_Rend_Flags  runtime_rend; /* The rendering we need to do on runtime
                                     in order to comply with the wanted_rend. */

   Eina_List       *task;
#ifdef EVAS_CSERVE2
   void            *cs2_handler;
#endif

   int              generation;

   unsigned char    sizeok : 1;
   unsigned char    inuse : 1;
};

struct _RGBA_Font_Source
{
   const char       *name;
   const char       *file;
   void             *data;
   unsigned int      current_size;
   int               data_size;
   int               references;
   struct {
      int            orig_upem;
      FT_Face        face;
   } ft;
};

/*
 * laziness wins for now. The parts used from the freetpye struct are
 * kept intact to avoid changing the code using it until we know exactly
 * what needs to be changed
 */
struct _RGBA_Font_Glyph_Out
{
   unsigned char *rle;
   struct {
      unsigned char *buffer;
      unsigned short rows;
      unsigned short width;
      unsigned short pitch;
      unsigned short rle_alloc : 1;
      unsigned short no_free_glout : 1;
   } bitmap;
   int rle_size;
};

struct _RGBA_Font_Glyph
{
   FT_UInt         index;
   Evas_Coord      width;
   Evas_Coord      x_bear;
   Evas_Coord      y_bear;
   FT_Glyph        glyph;
   RGBA_Font_Glyph_Out *glyph_out;
   /* this is a problem - only 1 engine at a time can extend such a font... grrr */
   void           *ext_dat;
   void           (*ext_dat_free) (void *ext_dat);
   RGBA_Font_Int   *fi;
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
    int left;
    int top;
    int right;
    int bottom;
    int width;
    int height;
    int area;
};

struct rect_node
{
    struct list_node _lst;
    struct rect rect;
};
#endif /* EVAS_RECT_SPLIT */

struct _Tilebuf
{
   int outbuf_w, outbuf_h;
   struct {
      short w, h;
   } tile_size;
#ifdef RECTUPDATE
/*
   Regionbuf *rb;
 */
#elif defined(EVAS_RECT_SPLIT)
   int need_merge;
   list_t rects;
#else
/*
   struct {
      int           w, h;
      Tilebuf_Tile *tiles;
   } tiles;
 */
#endif
   struct {
      int x, y, w, h;
   } prev_add, prev_del;
   Eina_Bool strict_tiles : 1;
};

struct _Tilebuf_Tile
{
   Eina_Bool redraw : 1;
/* FIXME: need these flags later - but not now */
/*
   Eina_Bool done   : 1;
   Eina_Bool edge   : 1;
   Eina_Bool from   : 1;

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
#define A_VAL(p) (((DATA8 *)(p))[3])
#define R_VAL(p) (((DATA8 *)(p))[2])
#define G_VAL(p) (((DATA8 *)(p))[1])
#define B_VAL(p) (((DATA8 *)(p))[0])
#define AR_VAL(p) ((DATA16 *)(p)[1])
#define GB_VAL(p) ((DATA16 *)(p)[0])
#else
/* ppc */
#define A_VAL(p) (((DATA8 *)(p))[0])
#define R_VAL(p) (((DATA8 *)(p))[1])
#define G_VAL(p) (((DATA8 *)(p))[2])
#define B_VAL(p) (((DATA8 *)(p))[3])
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

#define SCALE_SIZE_MAX ((1 << 15) - 1)

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

/****/
#include "../common/evas_blend.h"

EAPI Gfx_Func_Copy        evas_common_draw_func_copy_get        (int pixels, int reverse);

/****/
#include "../common/evas_convert_color.h"
#include "../common/evas_convert_colorspace.h"
#include "../common/evas_convert_main.h"
#include "../common/evas_convert_yuv.h"
#include "../common/evas_scale_main.h"
#include "../common/evas_scale_smooth.h"
#include "../common/evas_scale_span.h"

/****/
#include "../common/evas_image.h"

/****/
#include "../common/evas_line.h"
#include "../common/evas_polygon.h"
#include "../common/evas_rectangle.h"

/****/
EAPI void     evas_common_blit_init               (void);

EAPI void     evas_common_blit_rectangle          (const RGBA_Image *src, RGBA_Image *dst, int src_x, int src_y, int w, int h, int dst_x, int dst_y);

/****/
#include "../common/evas_font.h"

/****/
EAPI void          evas_common_tilebuf_init               (void);

EAPI Tilebuf      *evas_common_tilebuf_new               (int w, int h);
EAPI void          evas_common_tilebuf_free              (Tilebuf *tb);
EAPI void          evas_common_tilebuf_set_tile_size     (Tilebuf *tb, int tw, int th);
EAPI void          evas_common_tilebuf_get_tile_size     (Tilebuf *tb, int *tw, int *th);
EAPI void          evas_common_tilebuf_tile_strict_set   (Tilebuf *tb, Eina_Bool strict);
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
#include "../common/evas_draw.h"

#include "../common/evas_map_image.h"

/****/
#include "../common/evas_pipe.h"

void              evas_font_dir_cache_free(void);

EAPI int          evas_async_events_process_blocking(void);
void	          evas_render_rendering_wait(Evas_Public_Data *evas);
void              evas_all_sync(void);

void              evas_thread_init(void);
void              evas_thread_shutdown(void);
EAPI void         evas_thread_cmd_enqueue(Evas_Thread_Command_Cb cb, void *data);
EAPI void         evas_thread_queue_flush(Evas_Thread_Command_Cb cb, void *data);

typedef enum _Evas_Render_Mode
{
   EVAS_RENDER_MODE_UNDEF,
   EVAS_RENDER_MODE_SYNC,
   EVAS_RENDER_MODE_ASYNC_INIT,
   EVAS_RENDER_MODE_ASYNC_END,
} Evas_Render_Mode;

/****/

void evas_common_rgba_image_scalecache_items_ref(Image_Entry *ie, Eina_Array *ret);
void evas_common_rgba_image_scalecache_item_unref(Image_Entry *ie);

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
