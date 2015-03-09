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
#ifndef GL_ETC1_RGB8_OES
# define GL_ETC1_RGB8_OES 0x8D64
#endif
#ifndef GL_COMPRESSED_RGB8_ETC2
# define GL_COMPRESSED_RGB8_ETC2 0x9274
#endif
#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
# define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#endif
#ifndef GL_COMPRESSED_RGB_S3TC_DXT1_EXT
# define GL_COMPRESSED_RGB_S3TC_DXT1_EXT 0x83F0
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
# define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 0x83F1
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
# define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 0x83F2
#endif
#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
# define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3
#endif
#ifndef GL_TEXTURE_EXTERNAL_OES
# define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif
#ifndef GL_UNPACK_ROW_LENGTH
# define GL_UNPACK_ROW_LENGTH 0x0CF2
#endif
#ifndef EGL_NO_DISPLAY
# define EGL_NO_DISPLAY 0
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
#ifndef EGL_IMAGE_PRESERVED_KHR
# define EGL_IMAGE_PRESERVED_KHR 0x30D2
#endif
#ifndef EGL_NATIVE_SURFACE_TIZEN
#define EGL_NATIVE_SURFACE_TIZEN 0x32A1
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
#ifndef GL_MAX_SAMPLES_EXT
#define GL_MAX_SAMPLES_EXT 0x8D57
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

// Evas_3d require GL_BGR, but that's an extention and will not be necessary once we move to Evas_GL_Image
#ifndef GL_BGR
#define GL_BGR 0x80E0
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
#define SHAD_TEXA   5
#define SHAD_TEXSAM 6
#define SHAD_TEXM   7

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

/* enum Evas_GL_Shader is defined below */
#include "shader/evas_gl_enum.x"

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
      Eina_Bool sec_tbm_surface : 1;
      Eina_Bool bin_program : 1;
      Eina_Bool unpack_row_length : 1;
      Eina_Bool etc1 : 1;
      Eina_Bool etc2 : 1;
      Eina_Bool etc1_subimage : 1;
      Eina_Bool s3tc : 1;
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
#define DEF_ATLAS_ALLOC            256

#define MIN_ATLAS_ALLOC_ALPHA   16
#define MAX_ATLAS_ALLOC_ALPHA 4096
#define DEF_ATLAS_ALLOC_ALPHA      512

#define MAX_ATLAS_W            512
#define DEF_ATLAS_W                 512

#define MAX_ATLAS_H            512
#define DEF_ATLAS_H                 512

#define ATLAS_FORMATS_COUNT    12

      Eina_List *cspaces; // depend on the values of etc1, etc2 and st3c

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
         } atlas;
      } tune;
   } info;

   struct {
      Eina_List       *whole;
      Eina_List       *atlas[ATLAS_FORMATS_COUNT];
   } tex;

   Eina_Hash          *native_pm_hash;
   Eina_Hash          *native_tex_hash;
   Eina_Hash          *native_wl_hash;
   Eina_Hash          *native_tbm_hash;
   Eina_Hash          *native_evasgl_hash;

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
#define RTYPE_YUY2  6
#define RTYPE_NV12  7
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
         Evas_GL_Shader  id; // debug info
         GLuint          cur_prog;
         GLuint          cur_tex, cur_texu, cur_texv, cur_texa, cur_texm;
         int             render_op;
         int             cx, cy, cw, ch;
         int             smooth;
         int             blend;
         int             clip;
         Eina_Bool       anti_alias : 1;
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
         Evas_GL_Shader  id; // debug info
         Evas_GL_Image  *surface;
         GLuint          cur_prog;
         GLuint          cur_tex, cur_texu, cur_texv, cur_texa, cur_texm;
         void           *cur_tex_dyn, *cur_texu_dyn, *cur_texv_dyn;
         int             render_op;
         int             cx, cy, cw, ch;
         int             smooth;
         int             blend;
         int             mask_smooth;
         int             clip;
      } shader;
      struct {
         int num, alloc;
         GLshort *vertex;
         GLubyte *color;
         GLfloat *texuv;
         GLfloat *texuv2;
         GLfloat *texuv3;
         GLfloat *texa;
         GLfloat *texsam;
         GLfloat *texm;
         Eina_Bool line: 1;
         Eina_Bool use_vertex : 1;
         Eina_Bool use_color : 1;
         Eina_Bool use_texuv : 1;
         Eina_Bool use_texuv2 : 1;
         Eina_Bool use_texuv3 : 1;
         Eina_Bool use_texa : 1;
         Eina_Bool use_texsam : 1;
         Eina_Bool use_texm : 1;
         Eina_Bool anti_alias : 1;
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

#ifdef GL_GLES
   // FIXME: hack. expose egl display to gl core for egl image sec extn.
   void *egldisp;
   void *eglctxt;
#endif

   GLuint preserve_bit;
   int gles_version;
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
      void         *buffer;
      unsigned int *data;
      int           w, h;
      int           stride;
      int           checked_out;
   } dyn;
   Eina_List       *allocations;
   Eina_Rectangle_Pool *eina_pool;
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
   Evas_GL_Texture_Pool *pt, *ptu, *ptv, *ptt;
   union {
      Evas_GL_Texture_Pool *ptuv;
      Evas_GL_Texture_Pool *pta;
   };
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
   Eina_Rectangle  *apt, *aptt;

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
      Evas_Colorspace space;
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

   struct {
      Evas_GL_Image *origin;
      int            w, h;
      Eina_Bool      smooth : 1;
   } scaled;

   int scale_hint, content_hint;
   int csize;

   Eina_List       *filtered;
   Eina_List       *targets;

   unsigned char    dirty : 1;
   unsigned char    cached : 1;
   unsigned char    alpha : 1;
   unsigned char    tex_only : 1;
   unsigned char    locked : 1; // gl_surface_lock/unlock
   unsigned char    direct : 1; // evas gl direct renderable
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

/* GL_Common function that are used by gl_generic inherited module */
EAPI void         evas_gl_common_image_all_unload(Evas_Engine_GL_Context *gc);
EAPI void         evas_gl_common_image_ref(Evas_GL_Image *im);
EAPI void         evas_gl_common_image_unref(Evas_GL_Image *im);
EAPI Evas_GL_Image *evas_gl_common_image_new_from_data(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, DATA32 *data, int alpha, Evas_Colorspace cspace);
EAPI void         evas_gl_common_image_native_disable(Evas_GL_Image *im);
EAPI void         evas_gl_common_image_free(Evas_GL_Image *im);
EAPI void         evas_gl_common_image_native_enable(Evas_GL_Image *im);

EAPI int          evas_gl_preload_init(void);
EAPI int          evas_gl_preload_shutdown(void);
EAPI Eina_Bool    evas_gl_preload_enabled(void);

EAPI Evas_Engine_GL_Context  *evas_gl_common_context_new(void);

EAPI void         evas_gl_common_context_flush(Evas_Engine_GL_Context *gc);
EAPI void         evas_gl_common_context_free(Evas_Engine_GL_Context *gc);
EAPI void         evas_gl_common_context_use(Evas_Engine_GL_Context *gc);
EAPI void         evas_gl_common_context_newframe(Evas_Engine_GL_Context *gc);
EAPI void         evas_gl_common_context_done(Evas_Engine_GL_Context *gc);

EAPI void         evas_gl_common_context_resize(Evas_Engine_GL_Context *gc, int w, int h, int rot);
EAPI int          evas_gl_common_buffer_dump(Evas_Engine_GL_Context *gc, const char* dname, const char* fname, int frame, const char* suffix);

EAPI void         evas_gl_preload_render_lock(evas_gl_make_current_cb make_current, void *engine_data);
EAPI void         evas_gl_preload_render_unlock(evas_gl_make_current_cb make_current, void *engine_data);
EAPI void         evas_gl_preload_render_relax(evas_gl_make_current_cb make_current, void *engine_data);
EAPI void         evas_gl_symbols(void *(*GetProcAddress)(const char *name));

EAPI void         evas_gl_common_error_set(void *data, int error_enum);
EAPI int          evas_gl_common_error_get(void *data);
EAPI void        *evas_gl_common_current_context_get(void);

typedef int (*Evas_GL_Preload)(void);
typedef void (*Evas_GL_Common_Image_Call)(Evas_GL_Image *im);
typedef void (*Evas_GL_Common_Context_Call)(Evas_Engine_GL_Context *gc);
typedef Evas_GL_Image *(*Evas_GL_Common_Image_New_From_Data)(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, DATA32 *data, int alpha, Evas_Colorspace cspace);
typedef void (*Evas_GL_Preload_Render_Call)(evas_gl_make_current_cb make_current, void *engine_data);
typedef Evas_Engine_GL_Context *(*Evas_GL_Common_Context_New)(void);
typedef void (*Evas_GL_Common_Context_Resize_Call)(Evas_Engine_GL_Context *gc, int w, int h, int rot);
typedef int (*Evas_GL_Common_Buffer_Dump_Call)(Evas_Engine_GL_Context *gc,const char* dname, const char* fname, int frame, const char* suffix);
typedef void (*Evas_Gl_Symbols)(void *(*GetProcAddress)(const char *sym));

EAPI void __evas_gl_err(int err, const char *file, const char *func, int line, const char *op);

void              evas_gl_common_tiling_start(Evas_Engine_GL_Context *gc,
                                              int rot, int gw, int gh,
                                              int cx, int cy, int cw, int ch,
                                              int bitmask);
void              evas_gl_common_tiling_done(Evas_Engine_GL_Context *gc);
void              evas_gl_common_context_target_surface_set(Evas_Engine_GL_Context *gc, Evas_GL_Image *surface);

void              evas_gl_common_context_line_push(Evas_Engine_GL_Context *gc,
                                                   int x1, int y1, int x2, int y2,
                                                   int clip, int cx, int cy, int cw, int ch,
                                                   int r, int g, int b, int a);
void              evas_gl_common_context_rectangle_push(Evas_Engine_GL_Context *gc,
                                                        int x, int y, int w, int h,
                                                        int r, int g, int b, int a,
                                                        Evas_GL_Texture *mtex, double mx, double my, double mw, double mh, Eina_Bool mask_smooth);
void              evas_gl_common_context_image_push(Evas_Engine_GL_Context *gc,
                                                    Evas_GL_Texture *tex,
                                                    double sx, double sy, double sw, double sh,
                                                    int x, int y, int w, int h,
                                                    Evas_GL_Texture *mtex, double mx, double my, double mw, double mh, Eina_Bool mask_smooth,
                                                    int r, int g, int b, int a,
                                                    Eina_Bool smooth, Eina_Bool tex_only);
void              evas_gl_common_context_font_push(Evas_Engine_GL_Context *gc,
                                                   Evas_GL_Texture *tex,
                                                   double sx, double sy, double sw, double sh,
                                                   int x, int y, int w, int h,
                                                   Evas_GL_Texture *mtex, double mx, double my, double mw, double mh, Eina_Bool mask_smooth,
                                                   int r, int g, int b, int a);
void             evas_gl_common_context_yuv_push(Evas_Engine_GL_Context *gc,
                                                 Evas_GL_Texture *tex,
                                                 double sx, double sy, double sw, double sh,
                                                 int x, int y, int w, int h,
                                                 Evas_GL_Texture *mtex, double mx, double my, double mw, double mh, Eina_Bool mask_smooth,
                                                 int r, int g, int b, int a,
                                                 Eina_Bool smooth);
void             evas_gl_common_context_yuy2_push(Evas_Engine_GL_Context *gc,
                                                  Evas_GL_Texture *tex,
                                                  double sx, double sy, double sw, double sh,
                                                  int x, int y, int w, int h,
                                                  Evas_GL_Texture *mtex, double mx, double my, double mw, double mh, Eina_Bool mask_smooth,
                                                  int r, int g, int b, int a,
                                                  Eina_Bool smooth);
void             evas_gl_common_context_nv12_push(Evas_Engine_GL_Context *gc,
                                                  Evas_GL_Texture *tex,
                                                  double sx, double sy, double sw, double sh,
                                                  int x, int y, int w, int h,
                                                  Evas_GL_Texture *mtex, double mx, double my, double mw, double mh, Eina_Bool mask_smooth,
                                                  int r, int g, int b, int a,
                                                  Eina_Bool smooth);
void             evas_gl_common_context_rgb_a_pair_push(Evas_Engine_GL_Context *gc,
                                                        Evas_GL_Texture *tex,
                                                        double sx, double sy, double sw, double sh,
                                                        int x, int y, int w, int h,
                                                        Evas_GL_Texture *mtex, double mx, double my, double mw, double mh, Eina_Bool mask_smooth,
                                                        int r, int g, int b, int a,
                                                        Eina_Bool smooth);
void             evas_gl_common_context_image_map_push(Evas_Engine_GL_Context *gc,
                                                       Evas_GL_Texture *tex,
                                                       int npoints,
                                                       RGBA_Map_Point *p,
                                                       int clip, int cx, int cy, int cw, int ch,
                                                       Evas_GL_Texture *mtex, int mx, int my, int mw, int mh, Eina_Bool mask_smooth,
                                                       int r, int g, int b, int a,
                                                       Eina_Bool smooth,
                                                       Eina_Bool tex_only,
                                                       Evas_Colorspace cspace);

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
void              evas_gl_common_texture_upload(Evas_GL_Texture *tex, RGBA_Image *im, unsigned int bytes_count);
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
Evas_GL_Texture  *evas_gl_common_texture_rgb_a_pair_new(Evas_Engine_GL_Context *gc, RGBA_Image *im);
void              evas_gl_common_texture_rgb_a_pair_update(Evas_GL_Texture *tex, RGBA_Image *im);

void              evas_gl_common_image_alloc_ensure(Evas_GL_Image *im);
Evas_GL_Image    *evas_gl_common_image_load(Evas_Engine_GL_Context *gc, const char *file, const char *key, Evas_Image_Load_Opts *lo, int *error);
Evas_GL_Image    *evas_gl_common_image_mmap(Evas_Engine_GL_Context *gc, Eina_File *f, const char *key, Evas_Image_Load_Opts *lo, int *error);
Evas_GL_Image    *evas_gl_common_image_new_from_copied_data(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, DATA32 *data, int alpha, Evas_Colorspace cspace);
Evas_GL_Image    *evas_gl_common_image_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, Evas_Colorspace cspace);
Evas_GL_Image    *evas_gl_common_image_alpha_set(Evas_GL_Image *im, int alpha);
void              evas_gl_common_image_scale_hint_set(Evas_GL_Image *im, int hint);
void              evas_gl_common_image_content_hint_set(Evas_GL_Image *im, int hint);
void              evas_gl_common_image_cache_flush(Evas_Engine_GL_Context *gc);
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

// TBM Surface stuff
#define TBM_SURF_PLANE_MAX 4 /**< maximum number of planes  */

/* option to map the tbm_surface */
#define TBM_SURF_OPTION_READ      (1 << 0) /**< access option to read  */
#define TBM_SURF_OPTION_WRITE     (1 << 1) /**< access option to write */

#define __tbm_fourcc_code(a,b,c,d) ((uint32_t)(a) | ((uint32_t)(b) << 8) | \
			      ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define TBM_FORMAT_C8       __tbm_fourcc_code('C', '8', ' ', ' ')
#define TBM_FORMAT_RGBA8888 __tbm_fourcc_code('R', 'A', '2', '4')
#define TBM_FORMAT_BGRA8888 __tbm_fourcc_code('B', 'A', '2', '4')
#define TBM_FORMAT_RGB888   __tbm_fourcc_code('R', 'G', '2', '4')

typedef struct _tbm_surface * tbm_surface_h;
typedef uint32_t tbm_format;
typedef struct _tbm_surface_plane
{
   unsigned char *ptr;   /**< Plane pointer */
   uint32_t size;        /**< Plane size */
   uint32_t offset;      /**< Plane offset */
   uint32_t stride;      /**< Plane stride */

   void *reserved1;      /**< Reserved pointer1 */
   void *reserved2;      /**< Reserved pointer2 */
   void *reserved3;      /**< Reserved pointer3 */
} tbm_surface_plane_s;

typedef struct _tbm_surface_info
{
   uint32_t width;      /**< TBM surface width */
   uint32_t height;     /**< TBM surface height */
   tbm_format format;   /**< TBM surface format*/
   uint32_t bpp;        /**< TBM surface bbp */
   uint32_t size;       /**< TBM surface size */

   uint32_t num_planes;                            /**< The number of planes */
   tbm_surface_plane_s planes[TBM_SURF_PLANE_MAX]; /**< Array of planes */

   void *reserved4;   /**< Reserved pointer4 */
   void *reserved5;   /**< Reserved pointer5 */
   void *reserved6;   /**< Reserved pointer6 */
} tbm_surface_info_s;


extern void *(*secsym_tbm_surface_create) (int width, int height, unsigned int format);
extern int   (*secsym_tbm_surface_destroy) (void *surface);
extern int   (*secsym_tbm_surface_map) (void *surface, int opt, void *info);
extern int   (*secsym_tbm_surface_unmap) (void *surface);
extern int   (*secsym_tbm_surface_get_info) (void *surface, void *info);
#endif

Eina_Bool evas_gl_preload_push(Evas_GL_Texture_Async_Preload *async);
void evas_gl_preload_pop(Evas_GL_Texture *tex);
void evas_gl_preload_target_register(Evas_GL_Texture *tex, Eo *target);
void evas_gl_preload_target_unregister(Evas_GL_Texture *tex, Eo *target);

void pt_unref(Evas_GL_Texture_Pool *pt);

//#define GL_ERRORS 1

#ifdef GL_ERRORS
#include <dlfcn.h>
static inline void
__evas_gl_errdyn(int err, const char *file, const char *func, int line, const char *op)
{
   static void (*sym)(int,const char*,const char*,int,const char*) = NULL;
   if (!sym) sym = dlsym(RTLD_DEFAULT, "__evas_gl_err");
   sym(err, file, func, line, op);
}
# define GLERRV(op) \
  { \
      int __gl_err = glGetError(); \
      if (__gl_err != GL_NO_ERROR) \
        __evas_gl_errdyn(__gl_err, __FILE__, __FUNCTION__, __LINE__, op); \
   }
/* Redefine common gl funcs */
# ifndef GL_ERRORS_NODEF
#  define glActiveTexture(...) do { glActiveTexture(__VA_ARGS__); GLERRV("glActiveTexture"); } while(0)
#  define glBindAttribLocation(...) do { glBindAttribLocation(__VA_ARGS__); GLERRV("glBindAttribLocation"); } while(0)
#  define glBindBuffer(...) do { glBindBuffer(__VA_ARGS__); GLERRV("glBindBuffer"); } while(0)
#  define glBindTexture(...) do { glBindTexture(__VA_ARGS__); GLERRV("glBindTexture"); } while(0)
#  define glBlendFunc(...) do { glBlendFunc(__VA_ARGS__); GLERRV("glBlendFunc"); } while(0)
#  define glBufferData(...) do { glBufferData(__VA_ARGS__); GLERRV("glBufferData"); } while(0)
#  define glCompressedTexSubImage2D(...) do { glCompressedTexSubImage2D(__VA_ARGS__); GLERRV("glCompressedTexSubImage2D"); } while(0)
#  define glDeleteBuffers(...) do { glDeleteBuffers(__VA_ARGS__); GLERRV("glDeleteBuffers"); } while(0)
#  define glDepthMask(...) do { glDepthMask(__VA_ARGS__); GLERRV("glDepthMask"); } while(0)
#  define glDisable(...) do { glDisable(__VA_ARGS__); GLERRV("glDisable"); } while(0)
#  define glDisableVertexAttribArray(...) do { glDisableVertexAttribArray(__VA_ARGS__); GLERRV("glDisableVertexAttribArray"); } while(0)
#  define glDrawArrays(...) do { glDrawArrays(__VA_ARGS__); GLERRV("glDrawArrays"); } while(0)
#  define glEnable(...) do { glEnable(__VA_ARGS__); GLERRV("glEnable"); } while(0)
#  define glEnableVertexAttribArray(...) do { glEnableVertexAttribArray(__VA_ARGS__); GLERRV("glEnableVertexAttribArray"); } while(0)
#  define glGenBuffers(...) do { glGenBuffers(__VA_ARGS__); GLERRV("glGenBuffers"); } while(0)
#  define glGetFloatv(...) do { glGetFloatv(__VA_ARGS__); GLERRV("glGetFloatv"); } while(0)
#  define glGetIntegerv(...) do { glGetIntegerv(__VA_ARGS__); GLERRV("glGetIntegerv"); } while(0)
#  define glGetUniformLocation(...) ({ GLint _x = glGetUniformLocation(__VA_ARGS__); GLERRV("glGetUniformLocation"); _x; })
#  define glHint(...) do { glHint(__VA_ARGS__); GLERRV("glHint"); } while(0)
#  define glReadPixels(...) do { glReadPixels(__VA_ARGS__); GLERRV("glReadPixels"); } while(0)
#  define glScissor(...) do { glScissor(__VA_ARGS__); GLERRV("glScissor"); } while(0)
#  define glGenFramebuffers(...) do { glGenFramebuffers(__VA_ARGS__); GLERRV("glGenFramebuffers"); } while(0)
#  define glBindFramebuffer(...) do { glBindFramebuffer(__VA_ARGS__); GLERRV("glBindFramebuffer"); } while(0)
#  define glEndTiling(...) do { glEndTiling(__VA_ARGS__); GLERRV("glEndTiling"); } while(0)
#  define glGetProgramBinary(...) do { glGetProgramBinary(__VA_ARGS__); GLERRV("glGetProgramBinary"); } while(0)
#  define glMapBuffer(...) ({ void *_x = glMapBuffer(__VA_ARGS__); GLERRV("glMapBuffer"); _x; })
#  define glStartTiling(...) do { glStartTiling(__VA_ARGS__); GLERRV("glStartTiling"); } while(0)
#  define glUnmapBuffer(...) do { glUnmapBuffer(__VA_ARGS__); GLERRV("glUnmapBuffer"); } while(0)
#  define glTexParameterf(...) do { glTexParameterf(__VA_ARGS__); GLERRV("glTexParameterf"); } while(0)
#  define glTexParameteri(...) do { glTexParameteri(__VA_ARGS__); GLERRV("glTexParameteri"); } while(0)
#  define glTexSubImage2D(...) do { glTexSubImage2D(__VA_ARGS__); GLERRV("glTexSubImage2D"); } while(0)
#  define glUniform1f(...) do { glUniform1f(__VA_ARGS__); GLERRV("glUniform1f"); } while(0)
#  define glUniform1i(...) do { glUniform1i(__VA_ARGS__); GLERRV("glUniform1i"); } while(0)
#  define glUniform2fv(...) do { glUniform2fv(__VA_ARGS__); GLERRV("glUniform2fv"); } while(0)
#  define glUniform4fv(...) do { glUniform4fv(__VA_ARGS__); GLERRV("glUniform4fv"); } while(0)
#  define glUniformMatrix4fv(...) do { glUniformMatrix4fv(__VA_ARGS__); GLERRV("glUniformMatrix4fv"); } while(0)
#  define glUseProgram(...) do { glUseProgram(__VA_ARGS__); GLERRV("glUseProgram"); } while(0)
#  define glVertexAttribPointer(...) do { glVertexAttribPointer(__VA_ARGS__); GLERRV("glVertexAttribPointer"); } while(0)
#  define glViewport(...) do { glViewport(__VA_ARGS__); GLERRV("glViewport"); } while(0)
#  define glPixelStorei(...) do { glPixelStorei(__VA_ARGS__); GLERRV("glPixelStorei"); } while(0)
#  define glCompressedTexImage2D(...) do { glCompressedTexImage2D(__VA_ARGS__); GLERRV("glCompressedTexImage2D"); } while(0)
#  define glCreateShader(...) ({ GLuint _x = glCreateShader(__VA_ARGS__); GLERRV("glCreateShader"); _x; })
#  define glCreateProgram(...) ({ GLuint _x = glCreateProgram(__VA_ARGS__); GLERRV("glCreateProgram"); _x; })
#  define glAttachShader(...) do { glAttachShader(__VA_ARGS__); GLERRV("glAttachShader"); } while(0)
#  define glLinkProgram(...) do { glLinkProgram(__VA_ARGS__); GLERRV("glLinkProgram"); } while(0)
#  define glGetProgramiv(...) do { glGetProgramiv(__VA_ARGS__); GLERRV("glGetProgramiv"); } while(0)
#  define glGetProgramInfoLog(...) do { glGetProgramInfoLog(__VA_ARGS__); GLERRV("glGetProgramInfoLog"); } while(0)
#  define glGetShaderiv(...) do { glGetShaderiv(__VA_ARGS__); GLERRV("glGetShaderiv"); } while(0)
#  define glShaderSource(...) do { glShaderSource(__VA_ARGS__); GLERRV("glShaderSource"); } while(0)
#  define glCompileShader(...) do { glCompileShader(__VA_ARGS__); GLERRV("glCompileShader"); } while(0)
# endif
#else
# define GLERRV(op)
#endif

Eina_Bool evas_gl_common_module_open(void);
void      evas_gl_common_module_close(void);

static inline void
_tex_sub_2d(Evas_Engine_GL_Context *gc, int x, int y, int w, int h, int fmt, int type, const void *pix)
{
   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size)) return;
   glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt, type, pix);
}

static inline void
_comp_tex_sub_2d(Evas_Engine_GL_Context *gc, int x, int y, int w, int h, int fmt, int imgsize, const void *pix)
{
   if ((w > gc->shared->info.max_texture_size) ||
       (h > gc->shared->info.max_texture_size)) return;
   glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, fmt, imgsize, pix);
}

#include "evas_gl_3d_common.h"

#endif
