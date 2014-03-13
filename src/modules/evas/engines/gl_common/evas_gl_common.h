#ifndef EVAS_GL_COMMON_H
#define EVAS_GL_COMMON_H

#include "evas_common_private.h"
#include "evas_private.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <Eet.h>

#define GL_GLEXT_PROTOTYPES

#ifdef BUILD_ENGINE_GL_COCOA
# include <OpenGL/gl.h>
# include <OpenGL/glext.h>
#else
# ifdef _EVAS_ENGINE_SDL_H
#  ifdef GL_GLES
#   include <SDL/SDL_opengles.h>
#  else
#   include <SDL/SDL_opengl.h>
#  endif
# else
#  ifdef GL_GLES
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#  else
#   include <GL/gl.h>
#   include <GL/glext.h>
#  endif
# endif
#endif

#ifndef GL_TEXTURE_RECTANGLE_NV
# define GL_TEXTURE_RECTANGLE_NV 0x84F5
#endif
#ifndef GL_BGRA
# define GL_BGRA 0x80E1
#endif
#ifndef GL_RGBA4
# define GL_RGBA4 0x8056
#endif
#ifndef GL_RGBA8
# define GL_RGBA8 0x8058
#endif
#ifndef GL_RGBA12
# define GL_RGBA12 0x805A
#endif
#ifndef GL_RGBA16
# define GL_RGBA16 0x805B
#endif
#ifndef GL_R3_G3_B2
# define GL_R3_G3_B2 0x2A10
#endif
#ifndef GL_RGB4
# define GL_RGB4 0x804F
#endif
#ifndef GL_RGB5
# define GL_RGB5 0x8050
#endif
#ifndef GL_RGB8
# define GL_RGB8 0x8051
#endif
#ifndef GL_RGB10
# define GL_RGB10 0x8052
#endif
#ifndef GL_RGB12
# define GL_RGB12 0x8053
#endif
#ifndef GL_RGB16
# define GL_RGB16 0x8054
#endif
#ifndef GL_ALPHA4
# define GL_ALPHA4 0x803B
#endif
#ifndef GL_ALPHA8
# define GL_ALPHA8 0x803C
#endif
#ifndef GL_ALPHA12
# define GL_ALPHA12 0x803D
#endif
#ifndef GL_ALPHA16
# define GL_ALPHA16 0x803E
#endif
#ifndef GL_LUMINANCE4
# define GL_LUMINANCE4 0x803F
#endif
#ifndef GL_LUMINANCE8
# define GL_LUMINANCE8 0x8040
#endif
#ifndef GL_LUMINANCE12
# define GL_LUMINANCE12 0x8041
#endif
#ifndef GL_LUMINANCE16
# define GL_LUMINANCE16 0x8042
#endif
#ifndef GL_LUMINANCE4_ALPHA4
# define GL_LUMINANCE4_ALPHA4 0x8043
#endif
#ifndef GL_LUMINANCE8_ALPHA8
# define GL_LUMINANCE8_ALPHA8 0x8045
#endif
#ifndef GL_LUMINANCE12_ALPHA12
# define GL_LUMINANCE12_ALPHA12 0x8047
#endif
#ifndef GL_LUMINANCE16_ALPHA16
# define GL_LUMINANCE16_ALPHA16 0x8048
#endif

#ifndef GL_UNPACK_ROW_LENGTH
# define GL_UNPACK_ROW_LENGTH 0x0cf2
#endif

#ifndef EGL_NO_CONTEXT
# define EGL_NO_CONTEXT 0
#endif
#ifndef EGL_NONE
# define EGL_NONE 0x3038
#endif
#ifndef EGL_TRUE
# define EGL_TRUE 1
#endif
#ifndef EGL_FALSE
# define EGL_FALSE 0
#endif

#ifndef EGL_MAP_GL_TEXTURE_2D_SEC
# define EGL_MAP_GL_TEXTURE_2D_SEC 0x3201
#endif
#ifndef  EGL_MAP_GL_TEXTURE_HEIGHT_SEC
# define EGL_MAP_GL_TEXTURE_HEIGHT_SEC 0x3202
#endif
#ifndef EGL_MAP_GL_TEXTURE_WIDTH_SEC
# define EGL_MAP_GL_TEXTURE_WIDTH_SEC 0x3203
#endif
#ifndef EGL_MAP_GL_TEXTURE_FORMAT_SEC
# define EGL_MAP_GL_TEXTURE_FORMAT_SEC 0x3204
#endif
#ifndef EGL_MAP_GL_TEXTURE_RGB_SEC
# define EGL_MAP_GL_TEXTURE_RGB_SEC 0x3205
#endif
#ifndef EGL_MAP_GL_TEXTURE_RGBA_SEC
# define EGL_MAP_GL_TEXTURE_RGBA_SEC 0x3206
#endif
#ifndef EGL_MAP_GL_TEXTURE_BGRA_SEC
# define EGL_MAP_GL_TEXTURE_BGRA_SEC 0x3207
#endif
#ifndef EGL_MAP_GL_TEXTURE_LUMINANCE_SEC
# define EGL_MAP_GL_TEXTURE_LUMINANCE_SEC 0x3208
#endif
#ifndef EGL_MAP_GL_TEXTURE_LUMINANCE_ALPHA_SEC
# define EGL_MAP_GL_TEXTURE_LUMINANCE_ALPHA_SEC	0x3209
#endif
#ifndef EGL_MAP_GL_TEXTURE_PIXEL_TYPE_SEC
# define EGL_MAP_GL_TEXTURE_PIXEL_TYPE_SEC 0x320a
#endif
#ifndef EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC
# define EGL_MAP_GL_TEXTURE_UNSIGNED_BYTE_SEC 0x320b
#endif
#ifndef EGL_MAP_GL_TEXTURE_STRIDE_IN_BYTES_SEC
# define EGL_MAP_GL_TEXTURE_STRIDE_IN_BYTES_SEC 0x320c
#endif
#ifndef GL_PROGRAM_BINARY_LENGTH
# define GL_PROGRAM_BINARY_LENGTH 0x8741
#endif
#ifndef GL_NUM_PROGRAM_BINARY_FORMATS
# define GL_NUM_PROGRAM_BINARY_FORMATS 0x87FE
#endif
#ifndef GL_PROGRAM_BINARY_FORMATS
# define GL_PROGRAM_BINARY_FORMATS 0x87FF
#endif
#ifndef GL_PROGRAM_BINARY_RETRIEVABLE_HINT
# define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257
#endif
#ifndef GL_MAX_SAMPLES_IMG
#define GL_MAX_SAMPLES_IMG 0x9135
#endif
#ifndef GL_WRITE_ONLY
#define GL_WRITE_ONLY 0x88B9
#endif
#ifndef EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC
#define EGL_MAP_GL_TEXTURE_DEVICE_CPU_SEC 1
#endif
#ifndef EGL_MAP_GL_TEXTURE_DEVICE_G2D_SEC
#define EGL_MAP_GL_TEXTURE_DEVICE_G2D_SEC 2
#endif
#ifndef EGL_MAP_GL_TEXTURE_OPTION_READ_SEC
#define EGL_MAP_GL_TEXTURE_OPTION_READ_SEC (1<<0)
#endif
#ifndef EGL_MAP_GL_TEXTURE_OPTION_WRITE_SEC
#define EGL_MAP_GL_TEXTURE_OPTION_WRITE_SEC (1<<1)
#endif

#ifndef GL_COLOR_BUFFER_BIT0_QCOM
// if GL_COLOR_BUFFER_BIT0_QCOM  just assume the rest arent... saves fluff
#define GL_COLOR_BUFFER_BIT0_QCOM                     0x00000001
#define GL_COLOR_BUFFER_BIT1_QCOM                     0x00000002
#define GL_COLOR_BUFFER_BIT2_QCOM                     0x00000004
#define GL_COLOR_BUFFER_BIT3_QCOM                     0x00000008
#define GL_COLOR_BUFFER_BIT4_QCOM                     0x00000010
#define GL_COLOR_BUFFER_BIT5_QCOM                     0x00000020
#define GL_COLOR_BUFFER_BIT6_QCOM                     0x00000040
#define GL_COLOR_BUFFER_BIT7_QCOM                     0x00000080
#define GL_DEPTH_BUFFER_BIT0_QCOM                     0x00000100
#define GL_DEPTH_BUFFER_BIT1_QCOM                     0x00000200
#define GL_DEPTH_BUFFER_BIT2_QCOM                     0x00000400
#define GL_DEPTH_BUFFER_BIT3_QCOM                     0x00000800
#define GL_DEPTH_BUFFER_BIT4_QCOM                     0x00001000
#define GL_DEPTH_BUFFER_BIT5_QCOM                     0x00002000
#define GL_DEPTH_BUFFER_BIT6_QCOM                     0x00004000
#define GL_DEPTH_BUFFER_BIT7_QCOM                     0x00008000
#define GL_STENCIL_BUFFER_BIT0_QCOM                   0x00010000
#define GL_STENCIL_BUFFER_BIT1_QCOM                   0x00020000
#define GL_STENCIL_BUFFER_BIT2_QCOM                   0x00040000
#define GL_STENCIL_BUFFER_BIT3_QCOM                   0x00080000
#define GL_STENCIL_BUFFER_BIT4_QCOM                   0x00100000
#define GL_STENCIL_BUFFER_BIT5_QCOM                   0x00200000
#define GL_STENCIL_BUFFER_BIT6_QCOM                   0x00400000
#define GL_STENCIL_BUFFER_BIT7_QCOM                   0x00800000
#define GL_MULTISAMPLE_BUFFER_BIT0_QCOM               0x01000000
#define GL_MULTISAMPLE_BUFFER_BIT1_QCOM               0x02000000
#define GL_MULTISAMPLE_BUFFER_BIT2_QCOM               0x04000000
#define GL_MULTISAMPLE_BUFFER_BIT3_QCOM               0x08000000
#define GL_MULTISAMPLE_BUFFER_BIT4_QCOM               0x10000000
#define GL_MULTISAMPLE_BUFFER_BIT5_QCOM               0x20000000
#define GL_MULTISAMPLE_BUFFER_BIT6_QCOM               0x40000000
#define GL_MULTISAMPLE_BUFFER_BIT7_QCOM               0x80000000
#endif

#define EVAS_GL_TILE_SIZE 16

#define SHAD_VERTEX 0
#define SHAD_COLOR  1
#define SHAD_TEXUV  2
#define SHAD_TEXUV2 3
#define SHAD_TEXUV3 4
#define SHAD_TEXM   5
#define SHAD_TEXSAM 6

typedef struct _Evas_GL_Program               Evas_GL_Program;
typedef struct _Evas_GL_Program_Source        Evas_GL_Program_Source;
typedef struct _Evas_GL_Shared                Evas_GL_Shared;
typedef struct _Evas_Engine_GL_Context        Evas_Engine_GL_Context;
typedef struct _Evas_GL_Texture_Pool          Evas_GL_Texture_Pool;
typedef struct _Evas_GL_Texture_Alloca        Evas_GL_Texture_Alloca;
typedef struct _Evas_GL_Texture               Evas_GL_Texture;
typedef struct _Evas_GL_Image                 Evas_GL_Image;
typedef struct _Evas_GL_Font_Texture          Evas_GL_Font_Texture;
typedef struct _Evas_GL_Polygon               Evas_GL_Polygon;
typedef struct _Evas_GL_Polygon_Point         Evas_GL_Polygon_Point;
typedef struct _Evas_GL_Texture_Async_Preload Evas_GL_Texture_Async_Preload;

typedef Eina_Bool (*evas_gl_make_current_cb)(void *engine_data, void *doit);

typedef enum {
  SHADER_RECT,
  SHADER_FONT,
   
  SHADER_IMG_MASK,
   
  SHADER_IMG,
  SHADER_IMG_NOMUL,
  SHADER_IMG_BGRA,
  SHADER_IMG_BGRA_NOMUL,
  SHADER_TEX,
  SHADER_TEX_NOMUL,
  SHADER_TEX_AFILL,
  SHADER_TEX_NOMUL_AFILL,
   
  SHADER_IMG_21,
  SHADER_IMG_21_NOMUL,
  SHADER_IMG_21_BGRA,
  SHADER_IMG_21_BGRA_NOMUL,
  SHADER_TEX_21,
  SHADER_TEX_21_NOMUL,
  SHADER_TEX_21_AFILL,
  SHADER_TEX_21_NOMUL_AFILL,
   
  SHADER_IMG_12,
  SHADER_IMG_12_NOMUL,
  SHADER_IMG_12_BGRA,
  SHADER_IMG_12_BGRA_NOMUL,
  SHADER_TEX_12,
  SHADER_TEX_12_NOMUL,
  SHADER_TEX_12_AFILL,
  SHADER_TEX_12_NOMUL_AFILL,
   
  SHADER_IMG_22,
  SHADER_IMG_22_NOMUL,
  SHADER_IMG_22_BGRA,
  SHADER_IMG_22_BGRA_NOMUL,
  SHADER_TEX_22,
  SHADER_TEX_22_NOMUL,
  SHADER_TEX_22_AFILL,
  SHADER_TEX_22_NOMUL_AFILL,
   
  SHADER_YUV,
  SHADER_YUV_NOMUL,
  SHADER_YUY2,
  SHADER_YUY2_NOMUL,
  SHADER_NV12,
  SHADER_NV12_NOMUL,
/*   
  SHADER_FILTER_INVERT,
  SHADER_FILTER_INVERT_NOMUL,
  SHADER_FILTER_INVERT_BGRA,
  SHADER_FILTER_INVERT_BGRA_NOMUL,
  SHADER_FILTER_GREYSCALE,
  SHADER_FILTER_GREYSCALE_NOMUL,
  SHADER_FILTER_GREYSCALE_BGRA,
  SHADER_FILTER_GREYSCALE_BGRA_NOMUL,
  SHADER_FILTER_SEPIA,
  SHADER_FILTER_SEPIA_NOMUL,
  SHADER_FILTER_SEPIA_BGRA,
  SHADER_FILTER_SEPIA_BGRA_NOMUL,
 */
  /* SHADER_FILTER_BLUR, */
  /* SHADER_FILTER_BLUR_NOMUL, */
  /* SHADER_FILTER_BLUR_BGRA, */
  /* SHADER_FILTER_BLUR_BGRA_NOMUL, */
  SHADER_LAST
} Evas_GL_Shader;

struct _Evas_GL_Program
{
   GLuint vert, frag, prog;

   int tex_count;
};

struct _Evas_GL_Program_Source
{
   const char *src;
   const unsigned int *bin;
   int bin_size;
};

struct _Evas_GL_Shared
{
   Eina_List          *images;

   int                 images_size;

   struct {
      GLint max_texture_units;
      GLint max_texture_size;
      GLint max_vertex_elements;
      GLfloat anisotropic;
      Eina_Bool rgb : 1;
      Eina_Bool bgra : 1;
      Eina_Bool tex_npo2 : 1;
      Eina_Bool tex_rect : 1;
      Eina_Bool sec_image_map : 1;
      Eina_Bool bin_program : 1;
      Eina_Bool unpack_row_length : 1;
      // tuning params - per gpu/cpu combo?
#define MAX_CUTOUT             512
#define DEF_CUTOUT                  512

#define MAX_PIPES              128
#define DEF_PIPES                    32
#define DEF_PIPES_SGX_540            24
#define DEF_PIPES_TEGRA_2             8
#define DEF_PIPES_TEGRA_3            24

#define MIN_ATLAS_ALLOC         16
#define MAX_ATLAS_ALLOC       1024
#define DEF_ATLAS_ALLOC            1024

#define MIN_ATLAS_ALLOC_ALPHA   16
#define MAX_ATLAS_ALLOC_ALPHA 4096
#define DEF_ATLAS_ALLOC_ALPHA      4096

#define MAX_ATLAS_W            512
#define DEF_ATLAS_W                 512

#define MAX_ATLAS_H            512
#define DEF_ATLAS_H                 512

#define MIN_ATLAS_SLOT          16
#define MAX_ATLAS_SLOT         512
#define DEF_ATLAS_SLOT               16

      struct {
         struct {
            int max;
         } cutout;
         struct {
            int max;
         } pipes;
         struct {
            int max_alloc_size;
            int max_alloc_alpha_size;
            int max_w;
            int max_h;
            int slot_size;
         } atlas;
      } tune;
   } info;

   struct {
      Eina_List       *whole;
      Eina_List       *atlas[33][3];
   } tex;

   Eina_Hash          *native_pm_hash;
   Eina_Hash          *native_tex_hash;

#ifdef GL_GLES
   // FIXME: hack.
   void *eglctxt;
#endif
   
   Evas_GL_Program     shader[SHADER_LAST];

   int references;
   int w, h;
   int rot;
   int mflip;
   // persp map
   int foc, z0, px, py;
   int ax, ay;
};

#define RTYPE_RECT  1
#define RTYPE_IMAGE 2
#define RTYPE_FONT  3
#define RTYPE_YUV   4
#define RTYPE_MAP   5 /* need to merge with image */
#define RTYPE_IMASK 6
#define RTYPE_YUY2  7
#define RTYPE_NV12  8
#define ARRAY_BUFFER_USE 500
#define ARRAY_BUFFER_USE_SHIFT 100

struct _Evas_Engine_GL_Context
{
   int                references;
   int                w, h;
   int                rot;
   int                foc, z0, px, py;
   RGBA_Draw_Context *dc;

   Evas_GL_Shared     *shared;

   int flushnum;
   struct {
      int                top_pipe;
      struct {
         GLuint          cur_prog;
         GLuint          cur_tex, cur_texu, cur_texv;
         GLuint          cur_texm, cur_texmu, cur_texmv;
         int             render_op;
         int             cx, cy, cw, ch;
         int             smooth;
         int             blend;
         int             clip;
      } current;
   } state;
   
   struct {
      int                x, y, w, h;
      Eina_Bool          enabled : 1;
      Eina_Bool          used : 1;
   } master_clip;

   struct {
      struct {
         int             x, y, w, h;
         int             type;
      } region;
      struct {
         int             x, y, w, h;
         Eina_Bool       active : 1;
      } clip;
      struct {
         Evas_GL_Image  *surface;
         GLuint          cur_prog;
         GLuint          cur_tex, cur_texu, cur_texv, cur_texm;
         void           *cur_tex_dyn, *cur_texu_dyn, *cur_texv_dyn;
         int             render_op;
         int             cx, cy, cw, ch;
         int             smooth;
         int             blend;
         int             clip;
      } shader;
      struct {
         int num, alloc;
         GLshort *vertex;
         GLubyte *color;
         GLfloat *texuv;
         GLfloat *texuv2;
         GLfloat *texuv3;
         GLfloat *texm;
         GLfloat *texsam;
         Eina_Bool line: 1;
         Eina_Bool use_vertex : 1;
         Eina_Bool use_color : 1;
         Eina_Bool use_texuv : 1;
         Eina_Bool use_texuv2 : 1;
         Eina_Bool use_texuv3 : 1;
         Eina_Bool use_texm : 1;
         Eina_Bool use_texsam : 1;
         Evas_GL_Image *im;
         GLuint buffer;
         int buffer_alloc;
         int buffer_use;
      } array;
   } pipe[MAX_PIPES];

   struct {
      Eina_Bool size : 1;
   } change;
   
   Eina_List *font_glyph_textures;

   Eina_Bool havestuff : 1;

   Evas_GL_Image *def_surface;

   /* If this is set: Force drawing with a particular filter */
   GLuint filter_prog;

#ifdef GL_GLES
   // FIXME: hack. expose egl display to gl core for egl image sec extn.
   void *egldisp;
   void *eglctxt;
#endif

   GLuint preserve_bit;
};

struct _Evas_GL_Texture_Pool
{
   Evas_Engine_GL_Context *gc;
   GLuint           texture, fb;
   GLuint           intformat, format, dataformat;
   int              w, h;
   int              references;
   int              slot, fslot;
   struct {
      void         *img;
      unsigned int *data;
      int           w, h;
      int           stride;
      int           checked_out;
   } dyn;
   Eina_List       *allocations;
   Eina_Bool        whole : 1;
   Eina_Bool        render : 1;
   Eina_Bool        native : 1;
   Eina_Bool        dynamic : 1;
};

struct _Evas_GL_Texture_Alloca
{
   Evas_GL_Texture *tex;
   int x, w;
};

struct _Evas_GL_Texture
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image   *im;
   Evas_GL_Texture_Pool *pt, *ptu, *ptv, *ptuv, *ptt;
   Evas_GL_Texture_Alloca *apt, *aptt;
   RGBA_Font_Glyph *fglyph;
   int              x, y, w, h;
   int              tx, ty;
   double           sx1, sy1, sx2, sy2;
   int              references;

   struct
   {
      Evas_GL_Texture_Pool *pt[2], *ptuv[2];
      int              source;
   } double_buffer;

   Eina_List       *targets;

   Eina_Bool        alpha : 1;
   Eina_Bool        dyn : 1;
   Eina_Bool        uploaded : 1;
   Eina_Bool        was_preloaded : 1;
};

struct _Evas_GL_Image
{
   Evas_Engine_GL_Context *gc;
   RGBA_Image      *im;
   Evas_GL_Texture *tex;
   Evas_Image_Load_Opts load_opts;
   int              references;
   // if im->im == NULL, it's a render-surface so these here are used
   int              w, h;
   struct {
      int           space;
      void         *data;
      unsigned char no_free : 1;
   } cs;

   struct {
      void         *data;
      struct {
         void     (*bind)   (void *data, void *image);
         void     (*unbind) (void *data, void *image);
         void     (*free)   (void *data, void *image);
         void      *data;
      } func;
      int           yinvert;
      int           target;
      int           mipmap;
      unsigned char loose : 1;
   } native;

   int scale_hint, content_hint;
   int csize;

   Eina_List       *filtered;
   Eina_List       *targets;

   unsigned char    dirty : 1;
   unsigned char    cached : 1;
   unsigned char    alpha : 1;
   unsigned char    tex_only : 1;
   unsigned char    locked : 1; // gl_surface_lock/unlock
};

struct _Evas_GL_Font_Texture
{
   Evas_GL_Texture *tex;
};

struct _Evas_GL_Polygon
{
   Eina_List *points;
   Eina_Bool  changed : 1;
};

struct _Evas_GL_Polygon_Point
{
   int x, y;
};

struct _Evas_GL_Texture_Async_Preload
{
   Evas_GL_Texture *tex;
   RGBA_Image *im;

   Eina_Bool unpack_row_length;
};

#if 0
extern Evas_GL_Program_Source shader_rect_frag_src;
extern Evas_GL_Program_Source shader_rect_vert_src;
extern Evas_GL_Program_Source shader_font_frag_src;
extern Evas_GL_Program_Source shader_font_vert_src;

extern Evas_GL_Program_Source shader_img_frag_src;
extern Evas_GL_Program_Source shader_img_vert_src;
extern Evas_GL_Program_Source shader_img_nomul_frag_src;
extern Evas_GL_Program_Source shader_img_nomul_vert_src;
extern Evas_GL_Program_Source shader_img_bgra_frag_src;
extern Evas_GL_Program_Source shader_img_bgra_vert_src;
extern Evas_GL_Program_Source shader_img_bgra_nomul_frag_src;
extern Evas_GL_Program_Source shader_img_bgra_nomul_vert_src;
extern Evas_GL_Program_Source shader_img_mask_frag_src;
extern Evas_GL_Program_Source shader_img_mask_vert_src;

extern Evas_GL_Program_Source shader_yuv_frag_src;
extern Evas_GL_Program_Source shader_yuv_vert_src;
extern Evas_GL_Program_Source shader_yuv_nomul_frag_src;
extern Evas_GL_Program_Source shader_yuv_nomul_vert_src;

extern Evas_GL_Program_Source shader_yuy2_frag_src;
extern Evas_GL_Program_Source shader_yuy2_vert_src;
extern Evas_GL_Program_Source shader_yuy2_nomul_frag_src;
extern Evas_GL_Program_Source shader_yuy2_nomul_vert_src;

extern Evas_GL_Program_Source shader_tex_frag_src;
extern Evas_GL_Program_Source shader_tex_vert_src;
extern Evas_GL_Program_Source shader_tex_nomul_frag_src;
extern Evas_GL_Program_Source shader_tex_nomul_vert_src;
#endif

void glerr(int err, const char *file, const char *func, int line, const char *op);

Evas_Engine_GL_Context  *evas_gl_common_context_new(void);
void              evas_gl_common_context_free(Evas_Engine_GL_Context *gc);
void              evas_gl_common_context_use(Evas_Engine_GL_Context *gc);
void              evas_gl_common_context_newframe(Evas_Engine_GL_Context *gc);
void              evas_gl_common_context_resize(Evas_Engine_GL_Context *gc, int w, int h, int rot);
void              evas_gl_common_tiling_start(Evas_Engine_GL_Context *gc,
                                              int rot, int gw, int gh,
                                              int cx, int cy, int cw, int ch,
                                              int bitmask);
void              evas_gl_common_context_done(Evas_Engine_GL_Context *gc);
void              evas_gl_common_tiling_done(Evas_Engine_GL_Context *gc);
void              evas_gl_common_context_target_surface_set(Evas_Engine_GL_Context *gc, Evas_GL_Image *surface);

void              evas_gl_common_context_line_push(Evas_Engine_GL_Context *gc,
                                                   int x1, int y1, int x2, int y2,
                                                   int clip, int cx, int cy, int cw, int ch,
                                                   int r, int g, int b, int a);
void              evas_gl_common_context_rectangle_push(Evas_Engine_GL_Context *gc,
                                                        int x, int y, int w, int h,
                                                        int r, int g, int b, int a);
void              evas_gl_common_context_image_push(Evas_Engine_GL_Context *gc,
                                                    Evas_GL_Texture *tex,
                                                    double sx, double sy, double sw, double sh,
                                                    int x, int y, int w, int h,
                                                    int r, int g, int b, int a,
                                                    Eina_Bool smooth, Eina_Bool tex_only);
void              evas_gl_common_context_image_mask_push(Evas_Engine_GL_Context *gc,
                                                    Evas_GL_Texture *tex,
                                                    Evas_GL_Texture *texm,
                                                    double sx, double sy, double sw, double sh,
                                                    double sxm, double sym, double swm, double shm,
                                                    int x, int y, int w, int h,
                                                    int r, int g, int b, int a,
                                                    Eina_Bool smooth);


void              evas_gl_common_context_font_push(Evas_Engine_GL_Context *gc,
                                                   Evas_GL_Texture *tex,
                                                   double sx, double sy, double sw, double sh,
                                                   int x, int y, int w, int h,
                                                   int r, int g, int b, int a);
void             evas_gl_common_context_yuv_push(Evas_Engine_GL_Context *gc,
                                                 Evas_GL_Texture *tex,
                                                 double sx, double sy, double sw, double sh,
                                                 int x, int y, int w, int h,
                                                 int r, int g, int b, int a,
                                                 Eina_Bool smooth);
void             evas_gl_common_context_yuy2_push(Evas_Engine_GL_Context *gc,
                                                  Evas_GL_Texture *tex,
                                                  double sx, double sy, double sw, double sh,
                                                  int x, int y, int w, int h,
                                                  int r, int g, int b, int a,
                                                  Eina_Bool smooth);
void             evas_gl_common_context_nv12_push(Evas_Engine_GL_Context *gc,
                                                  Evas_GL_Texture *tex,
                                                  double sx, double sy, double sw, double sh,
                                                  int x, int y, int w, int h,
                                                  int r, int g, int b, int a,
                                                  Eina_Bool smooth);
void             evas_gl_common_context_image_map_push(Evas_Engine_GL_Context *gc,
                                                       Evas_GL_Texture *tex,
                                                       int npoints,
                                                       RGBA_Map_Point *p,
                                                       int clip, int cx, int cy, int cw, int ch,
                                                       int r, int g, int b, int a,
                                                       Eina_Bool smooth,
                                                       Eina_Bool tex_only,
                                                       Evas_Colorspace cspace);
void              evas_gl_common_context_flush(Evas_Engine_GL_Context *gc);

int               evas_gl_common_shader_program_init(Evas_GL_Shared *shared);
void              evas_gl_common_shader_program_init_done(void);
void              evas_gl_common_shader_program_shutdown(Evas_GL_Program *p);

Eina_Bool         evas_gl_common_file_cache_is_dir(const char *file);
Eina_Bool         evas_gl_common_file_cache_mkdir(const char *dir);
Eina_Bool         evas_gl_common_file_cache_file_exists(const char *file);
Eina_Bool         evas_gl_common_file_cache_mkpath_if_not_exists(const char *path);
Eina_Bool         evas_gl_common_file_cache_mkpath(const char *path);
int               evas_gl_common_file_cache_dir_check(char *cache_dir, int num);
int               evas_gl_common_file_cache_file_check(const char *cache_dir, const char *cache_name, char *cache_file, int dir_num);
int               evas_gl_common_file_cache_save(Evas_GL_Shared *shared);

void              evas_gl_common_rect_draw(Evas_Engine_GL_Context *gc, int x, int y, int w, int h);

void              evas_gl_texture_pool_empty(Evas_GL_Texture_Pool *pt);
Evas_GL_Texture  *evas_gl_common_texture_new(Evas_Engine_GL_Context *gc, RGBA_Image *im);
Evas_GL_Texture  *evas_gl_common_texture_native_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, Evas_GL_Image *im);
Evas_GL_Texture  *evas_gl_common_texture_render_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha);
Evas_GL_Texture  *evas_gl_common_texture_dynamic_new(Evas_Engine_GL_Context *gc, Evas_GL_Image *im);
void              evas_gl_common_texture_update(Evas_GL_Texture *tex, RGBA_Image *im);
void              evas_gl_common_texture_free(Evas_GL_Texture *tex, Eina_Bool force);
Evas_GL_Texture  *evas_gl_common_texture_alpha_new(Evas_Engine_GL_Context *gc, DATA8 *pixels, unsigned int w, unsigned int h, int fh);
void              evas_gl_common_texture_alpha_update(Evas_GL_Texture *tex, DATA8 *pixels, unsigned int w, unsigned int h, int fh);
Evas_GL_Texture  *evas_gl_common_texture_yuv_new(Evas_Engine_GL_Context *gc, DATA8 **rows, unsigned int w, unsigned int h);
void              evas_gl_common_texture_yuv_update(Evas_GL_Texture *tex, DATA8 **rows, unsigned int w, unsigned int h);
Evas_GL_Texture  *evas_gl_common_texture_yuy2_new(Evas_Engine_GL_Context *gc, DATA8 **rows, unsigned int w, unsigned int h);
void              evas_gl_common_texture_yuy2_update(Evas_GL_Texture *tex, DATA8 **rows, unsigned int w, unsigned int h);
Evas_GL_Texture  *evas_gl_common_texture_nv12_new(Evas_Engine_GL_Context *gc, DATA8 **rows, unsigned int w, unsigned int h);
void              evas_gl_common_texture_nv12_update(Evas_GL_Texture *tex, DATA8 **row, unsigned int w, unsigned int h);
Evas_GL_Texture  *evas_gl_common_texture_nv12tiled_new(Evas_Engine_GL_Context *gc, DATA8 **rows, unsigned int w, unsigned int h);
void              evas_gl_common_texture_nv12tiled_update(Evas_GL_Texture *tex, DATA8 **row, unsigned int w, unsigned int h);

void              evas_gl_common_image_alloc_ensure(Evas_GL_Image *im);
void              evas_gl_common_image_all_unload(Evas_Engine_GL_Context *gc);

void              evas_gl_common_image_ref(Evas_GL_Image *im);
void              evas_gl_common_image_unref(Evas_GL_Image *im);
Evas_GL_Image    *evas_gl_common_image_load(Evas_Engine_GL_Context *gc, const char *file, const char *key, Evas_Image_Load_Opts *lo, int *error);
Evas_GL_Image    *evas_gl_common_image_mmap(Evas_Engine_GL_Context *gc, Eina_File *f, const char *key, Evas_Image_Load_Opts *lo, int *error);
Evas_GL_Image    *evas_gl_common_image_new_from_data(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, DATA32 *data, int alpha, int cspace);
Evas_GL_Image    *evas_gl_common_image_new_from_copied_data(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, DATA32 *data, int alpha, int cspace);
Evas_GL_Image    *evas_gl_common_image_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, int cspace);
Evas_GL_Image    *evas_gl_common_image_alpha_set(Evas_GL_Image *im, int alpha);
void              evas_gl_common_image_native_enable(Evas_GL_Image *im);
void              evas_gl_common_image_native_disable(Evas_GL_Image *im);
void              evas_gl_common_image_scale_hint_set(Evas_GL_Image *im, int hint);
void              evas_gl_common_image_content_hint_set(Evas_GL_Image *im, int hint);
void              evas_gl_common_image_cache_flush(Evas_Engine_GL_Context *gc);
void              evas_gl_common_image_free(Evas_GL_Image *im);
Evas_GL_Image    *evas_gl_common_image_surface_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha);
void              evas_gl_common_image_dirty(Evas_GL_Image *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void              evas_gl_common_image_update(Evas_Engine_GL_Context *gc, Evas_GL_Image *im);
void              evas_gl_common_image_map_draw(Evas_Engine_GL_Context *gc, Evas_GL_Image *im, int npoints, RGBA_Map_Point *p, int smooth, int level);
void              evas_gl_common_image_draw(Evas_Engine_GL_Context *gc, Evas_GL_Image *im, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int smooth);

void             *evas_gl_font_texture_new(void *gc, RGBA_Font_Glyph *fg);
void              evas_gl_font_texture_free(void *);
void              evas_gl_font_texture_draw(void *gc, void *surface, void *dc, RGBA_Font_Glyph *fg, int x, int y);

Evas_GL_Polygon  *evas_gl_common_poly_point_add(Evas_GL_Polygon *poly, int x, int y);
Evas_GL_Polygon  *evas_gl_common_poly_points_clear(Evas_GL_Polygon *poly);
void              evas_gl_common_poly_draw(Evas_Engine_GL_Context *gc, Evas_GL_Polygon *poly, int x, int y);

void              evas_gl_common_line_draw(Evas_Engine_GL_Context *gc, int x1, int y1, int x2, int y2);

int               evas_gl_common_buffer_dump(Evas_Engine_GL_Context *gc, const char* dname, const char* fname, int frame, const char* suffix);

extern void       (*glsym_glGenFramebuffers)      (GLsizei a, GLuint *b);
extern void       (*glsym_glBindFramebuffer)      (GLenum a, GLuint b);
extern void       (*glsym_glFramebufferTexture2D) (GLenum a, GLenum b, GLenum c, GLuint d, GLint e);
extern void       (*glsym_glDeleteFramebuffers)   (GLsizei a, const GLuint *b);
extern void       (*glsym_glGetProgramBinary)     (GLuint a, GLsizei b, GLsizei *c, GLenum *d, void *e);
extern void       (*glsym_glProgramBinary)        (GLuint a, GLenum b, const void *c, GLint d);
extern void       (*glsym_glProgramParameteri)    (GLuint a, GLuint b, GLint d);
extern void       (*glsym_glReleaseShaderCompiler)(void);
extern void      *(*glsym_glMapBuffer)            (GLenum a, GLenum b);
extern GLboolean  (*glsym_glUnmapBuffer)          (GLenum a);

#ifdef GL_GLES
extern void          *(*secsym_eglCreateImage)               (void *a, void *b, GLenum c, void *d, const int *e);
extern unsigned int   (*secsym_eglDestroyImage)              (void *a, void *b);
extern void           (*secsym_glEGLImageTargetTexture2DOES) (int a, void *b);
extern void          *(*secsym_eglMapImageSEC)               (void *a, void *b, int c, int d);
extern unsigned int   (*secsym_eglUnmapImageSEC)             (void *a, void *b, int c);
extern unsigned int   (*secsym_eglGetImageAttribSEC)         (void *a, void *b, int c, int *d);
#endif

Eina_Bool evas_gl_preload_push(Evas_GL_Texture_Async_Preload *async);
void evas_gl_preload_pop(Evas_GL_Texture *tex);
int evas_gl_preload_init(void);
int evas_gl_preload_shutdown(void);
void evas_gl_preload_render_lock(evas_gl_make_current_cb make_current, void *engine_data);
void evas_gl_preload_render_unlock(evas_gl_make_current_cb make_current, void *engine_data);
void evas_gl_preload_render_relax(evas_gl_make_current_cb make_current, void *engine_data);
void evas_gl_preload_target_register(Evas_GL_Texture *tex, Eo *target);
void evas_gl_preload_target_unregister(Evas_GL_Texture *tex, Eo *target);

void pt_unref(Evas_GL_Texture_Pool *pt);

//#define GL_ERRORS 1

#ifdef GL_ERRORS
# define GLERR(fn, fl, ln, op) \
   { \
      int __gl_err = glGetError(); \
      if (__gl_err != GL_NO_ERROR) glerr(__gl_err, fl, fn, ln, op); \
   }
#else
# define GLERR(fn, fl, ln, op)
#endif

Eina_Bool evas_gl_common_module_open(void);
void      evas_gl_common_module_close(void);

static inline void
_tex_sub_2d(Evas_Engine_GL_Context *gc, int x, int y, int w, int h, int fmt, int type, const void *pix)
{
   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size)) return;
   glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt, type, pix);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
}

#endif
