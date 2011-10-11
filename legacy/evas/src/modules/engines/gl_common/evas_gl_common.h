#ifndef EVAS_GL_COMMON_H
#define EVAS_GL_COMMON_H

#include "evas_common.h"
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

#ifdef BUILD_ENGINE_GL_QUARTZ
# include <OpenGL/gl.h>
# include <OpenGL/glext.h>
#else
# ifdef _EVAS_ENGINE_SDL_H
#  if defined(GLES_VARIETY_S3C6410) || defined(GLES_VARIETY_SGX)
#   include <SDL/SDL_opengles.h>
#  else
#   include <SDL/SDL_opengl.h>
#  endif
# else
#  if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#   if defined(GLES_VARIETY_S3C6410)
#    include <GLES2/gl2.h>
#   elif defined(GLES_VARIETY_SGX)
#    include <GLES2/gl2.h>
#    include <GLES2/gl2ext.h>
#   endif
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

#define SHAD_VERTEX 0
#define SHAD_COLOR  1
#define SHAD_TEXUV  2
#define SHAD_TEXUV2 3
#define SHAD_TEXUV3 4
#define SHAD_TEXM   5

typedef struct _Evas_GL_Program                      Evas_GL_Program;
typedef struct _Evas_GL_Program_Source               Evas_GL_Program_Source;
typedef struct _Evas_GL_Shared                       Evas_GL_Shared;
typedef struct _Evas_Engine_GL_Context               Evas_Engine_GL_Context;
typedef struct _Evas_GL_Texture_Pool                 Evas_GL_Texture_Pool;
typedef struct _Evas_GL_Texture                      Evas_GL_Texture;
typedef struct _Evas_GL_Image                        Evas_GL_Image;
typedef struct _Evas_GL_Font_Texture                 Evas_GL_Font_Texture;
typedef struct _Evas_GL_Polygon                      Evas_GL_Polygon;
typedef struct _Evas_GL_Polygon_Point                Evas_GL_Polygon_Point;

typedef enum {
  SHADER_RECT,
  SHADER_FONT,
  SHADER_IMG,
  SHADER_IMG_NOMUL,
  SHADER_IMG_BGRA,
  SHADER_IMG_BGRA_NOMUL,
  SHADER_IMG_MASK,
  SHADER_YUV,
  SHADER_YUV_NOMUL,
  SHADER_YUY2,
  SHADER_YUY2_NOMUL,
  SHADER_NV12,
  SHADER_NV12_NOMUL,
  SHADER_TEX,
  SHADER_TEX_NOMUL,
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
      // tuning params - per gpu/cpu combo?
#define MAX_CUTOUT             512
#define DEF_CUTOUT                  512

// FIXME bug with pipes > 1 right now, should default to 32
#define MAX_PIPES              128
#define DEF_PIPES                    32
#define DEF_PIPES_SGX_540            32
#define DEF_PIPES_TEGRA_2             1

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
	 Eina_Bool line: 1;
         Eina_Bool use_vertex : 1;
         Eina_Bool use_color : 1;
         Eina_Bool use_texuv : 1;
         Eina_Bool use_texuv2 : 1;
         Eina_Bool use_texuv3 : 1;
         Eina_Bool use_texm : 1;
         Evas_GL_Image *im;
      } array;
   } pipe[MAX_PIPES];

   struct {
      Eina_Bool size : 1;
   } change;

   Eina_Bool havestuff : 1;

   Evas_GL_Image *def_surface;

   /* If this is set: Force drawing with a particular filter */
   GLuint	filter_prog;

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
// FIXME: hack. expose egl display to gl core for egl image sec extn.
   void *egldisp;
#endif
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
   } dyn;
   Eina_List       *allocations;
   Eina_Bool        whole : 1;
   Eina_Bool        render : 1;
   Eina_Bool        native : 1;
   Eina_Bool        dynamic : 1;
};

struct _Evas_GL_Texture
{
   Evas_Engine_GL_Context *gc;
   Evas_GL_Image   *im;
   Evas_GL_Texture_Pool *pt, *ptu, *ptv, *ptuv;
   int              x, y, w, h;
   double           sx1, sy1, sx2, sy2;
   int              references;

   struct
   {
      Evas_GL_Texture_Pool *pt[2], *ptuv[2];
      int              source;
   } double_buffer;

   Eina_Bool        alpha : 1;
   Eina_Bool        dyn : 1;
};

struct _Evas_GL_Image
{
   Evas_Engine_GL_Context *gc;
   RGBA_Image      *im;
   Evas_GL_Texture *tex;
   RGBA_Image_Loadopts load_opts;
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

   unsigned char    dirty : 1;
   unsigned char    cached : 1;
   unsigned char    alpha : 1;
   unsigned char    tex_only : 1;
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

extern Evas_GL_Program_Source shader_filter_invert_frag_src;
extern Evas_GL_Program_Source shader_filter_invert_nomul_frag_src;
extern Evas_GL_Program_Source shader_filter_invert_bgra_frag_src;
extern Evas_GL_Program_Source shader_filter_invert_bgra_nomul_frag_src;
extern Evas_GL_Program_Source shader_filter_sepia_frag_src;
extern Evas_GL_Program_Source shader_filter_sepia_nomul_frag_src;
extern Evas_GL_Program_Source shader_filter_sepia_bgra_frag_src;
extern Evas_GL_Program_Source shader_filter_sepia_bgra_nomul_frag_src;
extern Evas_GL_Program_Source shader_filter_greyscale_frag_src;
extern Evas_GL_Program_Source shader_filter_greyscale_nomul_frag_src;
extern Evas_GL_Program_Source shader_filter_greyscale_bgra_frag_src;
extern Evas_GL_Program_Source shader_filter_greyscale_bgra_nomul_frag_src;
/* blur (annoyingly) needs (aka is faster with) a vertex shader */
extern Evas_GL_Program_Source shader_filter_blur_vert_src;
extern Evas_GL_Program_Source shader_filter_blur_frag_src;
extern Evas_GL_Program_Source shader_filter_blur_nomul_frag_src;
extern Evas_GL_Program_Source shader_filter_blur_bgra_frag_src;
extern Evas_GL_Program_Source shader_filter_blur_bgra_nomul_frag_src;
#endif

void glerr(int err, const char *file, const char *func, int line, const char *op);

Evas_Engine_GL_Context  *evas_gl_common_context_new(void);
void              evas_gl_common_context_free(Evas_Engine_GL_Context *gc);
void              evas_gl_common_context_use(Evas_Engine_GL_Context *gc);
void              evas_gl_common_context_newframe(Evas_Engine_GL_Context *gc);
void              evas_gl_common_context_resize(Evas_Engine_GL_Context *gc, int w, int h, int rot);
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

void              evas_gl_common_rect_draw(Evas_Engine_GL_Context *gc, int x, int y, int w, int h);

void              evas_gl_texture_pool_empty(Evas_GL_Texture_Pool *pt);
Evas_GL_Texture  *evas_gl_common_texture_new(Evas_Engine_GL_Context *gc, RGBA_Image *im);
Evas_GL_Texture  *evas_gl_common_texture_native_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, Evas_GL_Image *im);
Evas_GL_Texture  *evas_gl_common_texture_render_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha);
Evas_GL_Texture  *evas_gl_common_texture_dynamic_new(Evas_Engine_GL_Context *gc, Evas_GL_Image *im);
void              evas_gl_common_texture_update(Evas_GL_Texture *tex, RGBA_Image *im);
void              evas_gl_common_texture_free(Evas_GL_Texture *tex);
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

void              evas_gl_common_image_all_unload(Evas_Engine_GL_Context *gc);

void              evas_gl_common_image_ref(Evas_GL_Image *im);
void              evas_gl_common_image_unref(Evas_GL_Image *im);
Evas_GL_Image    *evas_gl_common_image_load(Evas_Engine_GL_Context *gc, const char *file, const char *key, Evas_Image_Load_Opts *lo, int *error);
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

#if 0 // filtering disabled
void              evas_gl_common_filter_draw(Evas_Engine_GL_Context *context, Evas_GL_Image *im, Evas_Filter_Info *filter);
Filtered_Image   *evas_gl_common_image_filtered_get(Evas_GL_Image *im, uint8_t *key, size_t keylen);
Filtered_Image   *evas_gl_common_image_filtered_save(Evas_GL_Image *im, Evas_GL_Image *fimage, uint8_t *key, size_t keylen);
void              evas_gl_common_image_filtered_free(Evas_GL_Image *im, Filtered_Image *);
#endif

extern void (*glsym_glGenFramebuffers)      (GLsizei a, GLuint *b);
extern void (*glsym_glBindFramebuffer)      (GLenum a, GLuint b);
extern void (*glsym_glFramebufferTexture2D) (GLenum a, GLenum b, GLenum c, GLuint d, GLint e);
extern void (*glsym_glDeleteFramebuffers)   (GLsizei a, const GLuint *b);
extern void (*glsym_glGetProgramBinary)     (GLuint a, GLsizei b, GLsizei *c, GLenum *d, void *e);
extern void (*glsym_glProgramBinary)        (GLuint a, GLenum b, const void *c, GLint d);
extern void (*glsym_glProgramParameteri)    (GLuint a, GLuint b, GLint d);

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
extern void          *(*secsym_eglCreateImage)               (void *a, void *b, GLenum c, void *d, const int *e);
extern unsigned int   (*secsym_eglDestroyImage)              (void *a, void *b);
extern void           (*secsym_glEGLImageTargetTexture2DOES) (int a, void *b);
extern void          *(*secsym_eglMapImageSEC)               (void *a, void *b);
extern unsigned int   (*secsym_eglUnmapImageSEC)             (void *a, void *b);
extern unsigned int   (*secsym_eglGetImageAttribSEC)         (void *a, void *b, int c, int *d);
#endif

#define GL_ERRORS 1

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

#endif
