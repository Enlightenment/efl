#ifndef EVAS_GL_COMMON_H
#define EVAS_GL_COMMON_H

#include "evas_common_private.h"
#include "evas_private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <Eet.h>

#define GL_GLEXT_PROTOTYPES

#if !defined(HAVE_ECORE_X_XLIB) && !defined(MESA_EGL_NO_X11_HEADERS)
# define MESA_EGL_NO_X11_HEADERS
#endif

#ifndef EVAS_GL_NO_HEADERS

#ifdef BUILD_ENGINE_GL_COCOA
# include <OpenGL/gl.h>
# include <OpenGL/glext.h>
#else
# ifdef _EVAS_ENGINE_SDL_H
#  ifdef GL_GLES
#   include <SDL2/SDL_opengles.h>
#  else
#   include <SDL2/SDL_opengl.h>
#  endif
# else
#  ifdef GL_GLES
#   include <EGL/egl.h>
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#  else
#   include <GL/gl.h>
#   include <GL/glext.h>
#  endif
# endif
#endif

#endif

#include "evas_gl_define.h"

#define EVAS_GL_TILE_SIZE 16

#define SHAD_VERTEX 0
#define SHAD_COLOR  1
#define SHAD_TEXUV  2
#define SHAD_TEXUV2 3
#define SHAD_TEXUV3 4
#define SHAD_TEXA   5
#define SHAD_TEXSAM 6
#define SHAD_MASK   7
#define SHAD_MASKSAM 8

typedef struct _Evas_GL_Program               Evas_GL_Program;
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
typedef struct _Evas_GL_Image_Data_Map        Evas_GL_Image_Data_Map;
typedef struct _Evas_GL_Filter                Evas_GL_Filter;

typedef Eina_Bool (*evas_gl_make_current_cb)(void *engine_data, void *doit);

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#define PROGRAM_HITCOUNT_MAX 0x1000000

struct _Evas_GL_Filter
{
   struct {
      GLint loc[3];
      Eina_Bool known_locations;
   } attribute;
   struct {
      GLint blur_count_loc;
      GLint blur_count_value;
      GLint blur_texlen_loc;
      GLfloat blur_texlen_value;
      GLint blur_div_loc;
      GLfloat blur_div_value;
      Eina_Bool known_locations;
   } uniform;
   struct {
      GLuint tex_ids[1];
   } texture;
   double blur_radius;
};

struct _Evas_GL_Program
{
   unsigned int flags, hitcount, tex_count;
   struct {
      GLuint mvp, rotation_id;
   } uniform;
   Evas_GL_Filter *filter;
   GLuint    prog;

   Eina_Bool reset     : 1;
   Eina_Bool bin_saved : 1;
   Eina_Bool delete_me : 1;
};

struct _Evas_GL_Shared
{
   Eina_List          *images;

   int                 images_size;

   struct {
      GLint     max_texture_units;
      GLint     max_texture_size;
      GLint     max_vertex_elements;
      GLfloat   anisotropic;
      Eina_Bool rgb               : 1;
      Eina_Bool bgra              : 1;
      Eina_Bool tex_npo2          : 1;
      Eina_Bool tex_rect          : 1;
      Eina_Bool sec_image_map     : 1;
      Eina_Bool sec_tbm_surface   : 1;
      Eina_Bool egl_tbm_ext       : 1;
      Eina_Bool bin_program       : 1;
      Eina_Bool unpack_row_length : 1;
      Eina_Bool etc1              : 1;
      Eina_Bool etc2              : 1;
      Eina_Bool etc1_subimage     : 1;
      Eina_Bool s3tc              : 1;
      // tuning params - per gpu/cpu combo?
#define DEF_CUTOUT                 4096

#define MAX_PIPES                    32
#define DEF_PIPES                    32
#define DEF_PIPES_SGX_540            24
#define DEF_PIPES_TEGRA_2             8
#define DEF_PIPES_TEGRA_3            24

#define MIN_ATLAS_ALLOC              16
#define MAX_ATLAS_ALLOC            1024
#define DEF_ATLAS_ALLOC             256

#define MIN_ATLAS_ALLOC_ALPHA        16
#define MAX_ATLAS_ALLOC_ALPHA      4096
#define DEF_ATLAS_ALLOC_ALPHA       512

#define MAX_ATLAS_W                 512
#define DEF_ATLAS_W                 512

#define MAX_ATLAS_H                 512
#define DEF_ATLAS_H                 512

#define ATLAS_FORMATS_COUNT          12

      Eina_List *cspaces; // depend on the values of etc1, etc2 and st3c

      struct {
         struct {
            int        max;
         } cutout;
         struct {
            int        max;
         } pipes;
         struct {
            int        max_alloc_size;
            int        max_alloc_alpha_size;
            int        max_w;
            int        max_h;
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

   Eet_File           *shaders_cache;
   Eina_Hash          *shaders_hash;
   Eina_Stringshare   *shaders_checksum;

#ifdef GL_GLES
   // FIXME: hack.
   void *eglctxt;
#endif

   int references;
   int w, h;
   int rot;
   int mflip;
   // persp map
   int foc, z0, px, py;
   int ax, ay;
   int offx, offy;
   GLfloat proj[16];
   Eina_Bool needs_shaders_flush : 1;
};

typedef enum _Shader_Sampling Shader_Sampling;
typedef enum _Shader_Type Shader_Type;

enum _Shader_Sampling {
   SHD_SAM11,
   SHD_SAM12,
   SHD_SAM21,
   SHD_SAM22,
   SHD_SAM_LAST
};

enum _Shader_Type {
   SHD_UNKNOWN,
   SHD_RECT,
   SHD_LINE,
   SHD_FONT,
   SHD_IMAGE,
   SHD_IMAGENATIVE,
   SHD_YUV,
   SHD_YUY2,
   SHD_NV12,
   SHD_YUV_709,
   SHD_YUY2_709,
   SHD_NV12_709,
   SHD_RGB_A_PAIR,
   SHD_MAP,
   SHD_FILTER_DISPLACE,
   SHD_FILTER_CURVE,
   SHD_FILTER_BLUR_X,
   SHD_FILTER_BLUR_Y
};

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
         Evas_GL_Program *prog;
         GLuint          cur_tex, cur_texu, cur_texv, cur_texa, cur_texm;
         int             tex_target;
         int             cx, cy, cw, ch;
         signed char     render_op;
         Eina_Bool       smooth     : 2;
         Eina_Bool       blend      : 2;
         Eina_Bool       clip       : 2;
         Eina_Bool       anti_alias : 2;
         Eina_Bool       has_filter_data : 1;
      } current;
   } state;

   struct {
      int                x, y, w, h;
      Eina_Bool          enabled    : 1;
      Eina_Bool          used       : 1;
   } master_clip;

   struct {
      Eina_Bool          size       : 1;
   } change;

   Eina_Bool             havestuff  : 1;

   struct {
      struct {
         int              x, y, w, h;
         Shader_Type      type;
      } region;
      struct {
         Evas_GL_Program *prog;
         Evas_GL_Image   *surface;
         void            *cur_tex_dyn, *cur_texu_dyn, *cur_texv_dyn;
         GLuint           cur_tex, cur_texu, cur_texv, cur_texa, cur_texm;
         int              tex_target;
         int              cx, cy, cw, ch;
         signed char      render_op;
         Eina_Bool        smooth      : 2;
         Eina_Bool        blend       : 2;
         Eina_Bool        mask_smooth : 2;
         Eina_Bool        clip        : 2;
         struct {
            GLuint        map_tex;
            Eina_Bool     map_nearest : 1;
            Eina_Bool     map_delete  : 1;
         } filter;
      } shader;
      struct {
         int            num, alloc;
         GLshort       *vertex;
         GLubyte       *color;
         GLfloat       *texuv;
         GLfloat       *texuv2;
         GLfloat       *texuv3;
         GLfloat       *texa;
         GLfloat       *texsam;
         GLfloat       *mask;
         GLfloat       *masksam;
         int            filter_data_count; // number of vec2
         GLfloat       *filter_data;
         Evas_GL_Image *im;
         GLuint         buffer;
         int            buffer_alloc;
         int            buffer_use;
         Eina_Bool      line        : 1;
         Eina_Bool      use_vertex  : 1; // always true
         Eina_Bool      use_color   : 1;
         Eina_Bool      use_texuv   : 1;
         Eina_Bool      use_texuv2  : 1;
         Eina_Bool      use_texuv3  : 1;
         Eina_Bool      use_texa    : 1;
         Eina_Bool      use_texsam  : 1;
         Eina_Bool      use_mask    : 1;
         Eina_Bool      use_masksam : 1;
         Eina_Bool      anti_alias  : 1;
      } array;
   } pipe[MAX_PIPES];

   Eina_List     *font_glyph_textures;
   Eina_List     *font_glyph_images;
   Evas_GL_Image *def_surface;
   RGBA_Image    *font_surface;

#ifdef GL_GLES
   // FIXME: hack. expose egl display to gl core for egl image sec extn.
   void *egldisp;
   void *eglctxt;
#else
   int glxcfg_rgb;
   int glxcfg_rgba;
#endif

   GLuint preserve_bit;
   int    gles_version;
};

struct _Evas_GL_Texture_Pool
{
   Evas_Engine_GL_Context *gc;
   GLuint           texture, fb, stencil;
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
      int           target;
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
   int              x, w;
};

struct _Evas_GL_Texture
{
   Evas_Engine_GL_Context  *gc;
   Evas_GL_Image           *im;
   Evas_GL_Texture_Pool    *pt, *ptu, *ptv, *ptt;
   Evas_GL_Texture_Pool    *pt2, *ptu2, *ptv2;
   union {
      Evas_GL_Texture_Pool *ptuv;
      Evas_GL_Texture_Pool *pta;
   };
   RGBA_Font_Glyph *fglyph;
   int              references;
   int              x, y, w, h;
   int              tx, ty;
   double           sx1, sy1, sx2, sy2;

   struct {
      Evas_GL_Texture_Pool *pt[2], *ptuv[2];
      int                   source;
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
   RGBA_Image             *im;
   Evas_GL_Texture        *tex;
   Evas_Image_Load_Opts    load_opts;
   RGBA_Font_Glyph        *fglyph;
   int                     references;
   // if im->im == NULL, it's a render-surface so these here are used
   int                     w, h;
   struct {
      Evas_Colorspace      space;
      void                *data;
      unsigned char        no_free : 1;
   } cs;

   struct {
      void           *data;
      struct {
         void       (*bind)    (void *image);
         void       (*unbind)  (void *image);
         void       (*free)    (void *image);
         int        (*yinvert) (void *image);
      } func;
      void           *disp;
      Evas_GL_Shared *shared;
      int             target;
      Eina_Bool       yinvert : 1;
      Eina_Bool       mipmap  : 1;
      Eina_Bool       loose   : 1;
   } native;

   struct {
      Evas_GL_Image  *origin;
      Eina_Bool       smooth : 1;
   } scaled;

   Eina_List         *filtered;
   Eina_List         *targets;
   Eina_Inlist       *maps; /* Evas_GL_Image_Data_Map */

   Evas_Image_Orient  orient;
   int                scale_hint, content_hint;
   int                csize;

   Eina_Bool          dirty         : 1;
   Eina_Bool          cached        : 1;
   Eina_Bool          alpha         : 1;
   Eina_Bool          tex_only      : 1;
   Eina_Bool          locked        : 1; // gl_surface_lock/unlock
   Eina_Bool          direct        : 1; // evas gl direct renderable
   /*Disable generate atlas for texture unit, EINA_FALSE by default*/
   Eina_Bool          disable_atlas : 1;
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
   RGBA_Image      *im;

   Eina_Bool        unpack_row_length;
};

struct _Evas_GL_Image_Data_Map
{
   EINA_INLIST;
   Evas_GL_Texture *tex; // one or the other
   RGBA_Image      *im; // one or the other
   Evas_GL_Image   *glim;
   Eina_Rw_Slice    slice;
   int              stride; // in bytes
   int              rx, ry, rw, rh; // actual map region
   Evas_Colorspace  cspace;
   Efl_Gfx_Buffer_Access_Mode mode;
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
EAPI void         evas_gl_symbols(void *(*GetProcAddress)(const char *name), const char *extsn);
EAPI Eina_Bool    evas_gl_extension_string_check(const char *ext, const char *exts);

EAPI void         evas_gl_common_error_set(int error_enum);
EAPI int          evas_gl_common_error_get(void);
EAPI void        *evas_gl_common_current_context_get(void);

typedef int (*Evas_GL_Preload)(void);
typedef void (*Evas_GL_Common_Image_Call)(Evas_GL_Image *im);
typedef void (*Evas_GL_Common_Context_Call)(Evas_Engine_GL_Context *gc);
typedef Evas_GL_Image *(*Evas_GL_Common_Image_New_From_Data)(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, DATA32 *data, int alpha, Evas_Colorspace cspace);
typedef void (*Evas_GL_Preload_Render_Call)(evas_gl_make_current_cb make_current, void *engine_data);
typedef Evas_Engine_GL_Context *(*Evas_GL_Common_Context_New)(void);
typedef void (*Evas_GL_Common_Context_Resize_Call)(Evas_Engine_GL_Context *gc, int w, int h, int rot);
typedef int (*Evas_GL_Common_Buffer_Dump_Call)(Evas_Engine_GL_Context *gc,const char* dname, const char* fname, int frame, const char* suffix);
typedef void (*Evas_Gl_Symbols)(void *(*GetProcAddress)(const char *sym), const char *extsn);
typedef Eina_Bool (*Evas_Gl_Extension_String_Check)(const char *exts, const char *ext);

EAPI void __evas_gl_err(int err, const char *file, const char *func, int line, const char *op);

int               evas_gl_common_version_check(int *minor_version);
void              evas_gl_common_tiling_start(Evas_Engine_GL_Context *gc,
                                              int rot, int gw, int gh,
                                              int cx, int cy, int cw, int ch,
                                              int bitmask);
void              evas_gl_common_tiling_done(Evas_Engine_GL_Context *gc);
void              evas_gl_common_context_target_surface_set(Evas_Engine_GL_Context *gc, Evas_GL_Image *surface);

void              evas_gl_common_context_line_push(Evas_Engine_GL_Context *gc,
                                                   int x1, int y1, int x2, int y2,
                                                   int clip, int cx, int cy, int cw, int ch,
                                                   Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                   Eina_Bool mask_smooth, Eina_Bool mask_color,
                                                   int r, int g, int b, int a);
void              evas_gl_common_context_rectangle_push(Evas_Engine_GL_Context *gc,
                                                        int x, int y, int w, int h,
                                                        int r, int g, int b, int a,
                                                        Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                        Eina_Bool mask_smooth, Eina_Bool mask_color);
void              evas_gl_common_context_image_push(Evas_Engine_GL_Context *gc,
                                                    Evas_GL_Texture *tex,
                                                    double sx, double sy, double sw, double sh,
                                                    int x, int y, int w, int h,
                                                    Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                    Eina_Bool mask_smooth, Eina_Bool mask_color,
                                                    int r, int g, int b, int a,
                                                    Eina_Bool smooth, Eina_Bool tex_only, Eina_Bool alphaonly);
void              evas_gl_common_context_font_push(Evas_Engine_GL_Context *gc,
                                                   Evas_GL_Texture *tex,
                                                   double sx, double sy, double sw, double sh,
                                                   int x, int y, int w, int h,
                                                   Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                   Eina_Bool mask_smooth, Eina_Bool mask_color,
                                                   int r, int g, int b, int a);
void             evas_gl_common_context_yuv_push(Evas_Engine_GL_Context *gc,
                                                 Evas_GL_Texture *tex,
                                                 double sx, double sy, double sw, double sh,
                                                 int x, int y, int w, int h,
                                                 Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                 Eina_Bool mask_smooth, Eina_Bool mask_color,
                                                 int r, int g, int b, int a,
                                                 Eina_Bool smooth);
void             evas_gl_common_context_yuv_709_push(Evas_Engine_GL_Context *gc,
                                                     Evas_GL_Texture *tex,
                                                     double sx, double sy, double sw, double sh,
                                                     int x, int y, int w, int h,
                                                     Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                     Eina_Bool mask_smooth, Eina_Bool mask_color,
                                                     int r, int g, int b, int a,
                                                     Eina_Bool smooth);
void             evas_gl_common_context_yuy2_push(Evas_Engine_GL_Context *gc,
                                                  Evas_GL_Texture *tex,
                                                  double sx, double sy, double sw, double sh,
                                                  int x, int y, int w, int h,
                                                  Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                  Eina_Bool mask_smooth, Eina_Bool mask_color,
                                                  int r, int g, int b, int a,
                                                  Eina_Bool smooth);
void             evas_gl_common_context_nv12_push(Evas_Engine_GL_Context *gc,
                                                  Evas_GL_Texture *tex,
                                                  double sx, double sy, double sw, double sh,
                                                  int x, int y, int w, int h,
                                                  Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                  Eina_Bool mask_smooth, Eina_Bool mask_color,
                                                  int r, int g, int b, int a,
                                                  Eina_Bool smooth);
void             evas_gl_common_context_rgb_a_pair_push(Evas_Engine_GL_Context *gc,
                                                        Evas_GL_Texture *tex,
                                                        double sx, double sy, double sw, double sh,
                                                        int x, int y, int w, int h,
                                                        Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                        Eina_Bool mask_smooth, Eina_Bool mask_color,
                                                        int r, int g, int b, int a,
                                                        Eina_Bool smooth);
void             evas_gl_common_context_image_map_push(Evas_Engine_GL_Context *gc,
                                                       Evas_GL_Texture *tex,
                                                       int npoints,
                                                       RGBA_Map_Point *p,
                                                       int clip, int cx, int cy, int cw, int ch,
                                                       Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                                       Eina_Bool mask_smooth, Eina_Bool mask_color,
                                                       int r, int g, int b, int a,
                                                       Eina_Bool smooth,
                                                       Eina_Bool tex_only,
                                                       Evas_Colorspace cspace);

// Gfx Filters
void              evas_gl_common_filter_blend_push(Evas_Engine_GL_Context *gc, Evas_GL_Texture *tex,
                                                   double sx, double sy, double sw, double sh, double dx,
                                                   double dy, double dw, double dh, Eina_Bool alphaonly);
void              evas_gl_common_filter_displace_push(Evas_Engine_GL_Context *gc, Evas_GL_Texture *tex, Evas_GL_Texture *map_tex,
                                                      int x, int y, int w, int h, double dx, double dy, Eina_Bool nearest);
void              evas_gl_common_filter_curve_push(Evas_Engine_GL_Context *gc, Evas_GL_Texture *tex,
                                                   int x, int y, int w, int h, const uint8_t *points, int channel);
void              evas_gl_common_filter_blur_push(Evas_Engine_GL_Context *gc, Evas_GL_Texture *tex, double sx, double sy, double sw, double sh,
                                                  double dx, double dy, double dw, double dh, const double * const values, const double * const offsets, int count, double radius,
                                                  Eina_Bool horiz, Eina_Bool alphaonly);

int               evas_gl_common_shader_program_init(Evas_GL_Shared *shared);
void              evas_gl_common_shader_program_shutdown(Evas_GL_Shared *shared);
EAPI void         evas_gl_common_shaders_flush(Evas_GL_Shared *shared);

Evas_GL_Program  *evas_gl_common_shader_program_get(Evas_Engine_GL_Context *gc,
                                                    Shader_Type type,
                                                    RGBA_Map_Point *p, int npoints,
                                                    int r, int g, int b, int a,
                                                    int sw, int sh, int w, int h, Eina_Bool smooth,
                                                    Evas_GL_Texture *tex, Eina_Bool tex_only,
                                                    Evas_GL_Texture *mtex, Eina_Bool mask_smooth,
                                                    Eina_Bool mask_color, int mw, int mh,
                                                    Eina_Bool alphaonly,
                                                    Shader_Sampling *psam, int *pnomul,
                                                    Shader_Sampling *pmasksam);
void              evas_gl_common_shader_textures_bind(Evas_GL_Program *p);

Eina_Bool         evas_gl_common_file_cache_is_dir(const char *file);
Eina_Bool         evas_gl_common_file_cache_mkdir(const char *dir);
Eina_Bool         evas_gl_common_file_cache_file_exists(const char *file);
Eina_Bool         evas_gl_common_file_cache_mkpath_if_not_exists(const char *path);
Eina_Bool         evas_gl_common_file_cache_mkpath(const char *path);
int               evas_gl_common_file_cache_dir_check(char *cache_dir, int num);
int               evas_gl_common_file_cache_file_check(const char *cache_dir, const char *cache_name, char *cache_file, int dir_num);

void              evas_gl_common_rect_draw(Evas_Engine_GL_Context *gc, int x, int y, int w, int h);

void              evas_gl_texture_pool_empty(Evas_GL_Texture_Pool *pt);
Evas_GL_Texture  *evas_gl_common_texture_new(Evas_Engine_GL_Context *gc, RGBA_Image *im, Eina_Bool disable_atlas);
Evas_GL_Texture  *evas_gl_common_texture_native_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, Evas_GL_Image *im);
Evas_GL_Texture  *evas_gl_common_texture_render_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, int stencil);
Evas_GL_Texture  *evas_gl_common_texture_render_noscale_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha);
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
Evas_Colorspace   evas_gl_common_gl_format_to_colorspace(GLuint f);

void              evas_gl_common_image_alloc_ensure(Evas_GL_Image *im);
Evas_GL_Image    *evas_gl_common_image_mmap(Evas_Engine_GL_Context *gc, Eina_File *f, const char *key, Evas_Image_Load_Opts *lo, int *error);
Evas_GL_Image    *evas_gl_common_image_new_from_copied_data(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, DATA32 *data, int alpha, Evas_Colorspace cspace);
Evas_GL_Image    *evas_gl_common_image_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, Evas_Colorspace cspace);
Evas_GL_Image    *evas_gl_common_image_new_from_rgbaimage(Evas_Engine_GL_Context *gc, RGBA_Image *im, Evas_Image_Load_Opts *lo, int *error);
void              evas_gl_common_image_preload_watch(Evas_GL_Image *im);
void              evas_gl_common_image_preload_unwatch(Evas_GL_Image *im);
Evas_GL_Image    *evas_gl_common_image_alpha_set(Evas_GL_Image *im, int alpha);
void              evas_gl_common_image_scale_hint_set(Evas_GL_Image *im, int hint);
void              evas_gl_common_image_content_hint_set(Evas_GL_Image *im, int hint);
void              evas_gl_common_image_cache_flush(Evas_Engine_GL_Context *gc);
Evas_GL_Image    *evas_gl_common_image_surface_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha, int stencil);
Evas_GL_Image    *evas_gl_common_image_surface_noscale_new(Evas_Engine_GL_Context *gc, unsigned int w, unsigned int h, int alpha);
void              evas_gl_common_image_dirty(Evas_GL_Image *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void              evas_gl_common_image_update(Evas_Engine_GL_Context *gc, Evas_GL_Image *im);
void              evas_gl_common_image_map_draw(Evas_Engine_GL_Context *gc, Evas_GL_Image *im, int npoints, RGBA_Map_Point *p, int smooth, int level);
void              evas_gl_common_image_draw(Evas_Engine_GL_Context *gc, Evas_GL_Image *im, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int smooth);
Evas_GL_Image    *evas_gl_common_image_surface_update(Evas_GL_Image *im);
Evas_GL_Image    *evas_gl_common_image_surface_detach(Evas_GL_Image *im);
Evas_GL_Image    *evas_gl_common_image_virtual_scaled_get(Evas_GL_Image *scaled, Evas_GL_Image *image, int dst_w, int dst_h, Eina_Bool smooth);

void             *evas_gl_font_texture_new(void *gc, RGBA_Font_Glyph *fg);
void              evas_gl_font_texture_free(void *);
void              evas_gl_font_texture_draw(void *gc, void *surface, void *dc, RGBA_Font_Glyph *fg, int x, int y, int w, int h);
void             *evas_gl_font_image_new(void *gc, RGBA_Font_Glyph *fg, int alpha, Evas_Colorspace cspace);
void              evas_gl_font_image_free(void *im);
void              evas_gl_font_image_draw(void *gc, void *im, int dx, int dy, int dw, int dh, int smooth);

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
extern void       (*glsym_glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

#ifdef GL_GLES
EAPI void *           evas_gl_common_eglCreateImage          (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list);
EAPI int              evas_gl_common_eglDestroyImage         (EGLDisplay dpy, void *im);
extern unsigned int   (*eglsym_eglDestroyImage)              (void *a, void *b);
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

// Enable GL errors logging
//#define GL_ERRORS 1
// Enable very basic GL calls logging (requires GL_ERRORS)
//#define GL_ERRORS_TRACE 1

#ifdef GL_ERRORS

# ifndef _WIN32
#  include <dlfcn.h>
# endif

static inline void
__evas_gl_errdyn(int err, const char *file, const char *func, int line, const char *op)
{
   static void (*sym)(int,const char*,const char*,int,const char*) = NULL;
   if (!sym) sym = dlsym(RTLD_DEFAULT, "__evas_gl_err");
   sym(err, file, func, line, op);
}
# define GLERRV(op) do { \
      int __gl_err = glGetError(); \
      if (__gl_err != GL_NO_ERROR) \
        __evas_gl_errdyn(__gl_err, __FILE__, __FUNCTION__, __LINE__, op); \
   } while (0)
/* Redefine common gl funcs */
# ifndef GL_ERRORS_NODEF
#  ifndef GL_ERRORS_TRACE
#   define GL_ERROR_TRACE(f, _args, ...) do { GLERRV("Uncaught error before this point:"); f(__VA_ARGS__); GLERRV(#f); } while(0)
#   define GL_ERROR_TRACE_RET(t, f, _args, ...) ({ t _r = f(__VA_ARGS__); GLERRV(#f); _r; })
#  else
#   define GL_ERROR_TRACE(f, _args, ...) do { DBG("%s(%s);", #f, _args); f(__VA_ARGS__); GLERRV(#f); } while(0)
#   define GL_ERROR_TRACE_RET(t, f, _args, ...) ({ t _r; DBG("%s(%s);", #f, _args); _r = f(__VA_ARGS__); GLERRV(#f); _r; })
#  endif
#  define glActiveTexture(...) GL_ERROR_TRACE(glActiveTexture, #__VA_ARGS__, __VA_ARGS__)
#  define glBindAttribLocation(...) GL_ERROR_TRACE(glBindAttribLocation, #__VA_ARGS__, __VA_ARGS__)
#  define glBindBuffer(...) GL_ERROR_TRACE(glBindBuffer, #__VA_ARGS__, __VA_ARGS__)
#  define glBindTexture(...) GL_ERROR_TRACE(glBindTexture, #__VA_ARGS__, __VA_ARGS__)
#  define glBlendFunc(...) GL_ERROR_TRACE(glBlendFunc, #__VA_ARGS__, __VA_ARGS__)
#  define glBufferData(...) GL_ERROR_TRACE(glBufferData, #__VA_ARGS__, __VA_ARGS__)
#  define glCompressedTexSubImage2D(...) GL_ERROR_TRACE(glCompressedTexSubImage2D, #__VA_ARGS__, __VA_ARGS__)
#  define glDeleteBuffers(...) GL_ERROR_TRACE(glDeleteBuffers, #__VA_ARGS__, __VA_ARGS__)
#  define glDepthMask(...) GL_ERROR_TRACE(glDepthMask, #__VA_ARGS__, __VA_ARGS__)
#  define glDisable(...) GL_ERROR_TRACE(glDisable, #__VA_ARGS__, __VA_ARGS__)
#  define glDisableVertexAttribArray(...) GL_ERROR_TRACE(glDisableVertexAttribArray, #__VA_ARGS__, __VA_ARGS__)
#  define glDrawArrays(...) GL_ERROR_TRACE(glDrawArrays, #__VA_ARGS__, __VA_ARGS__)
#  define glEnable(...) GL_ERROR_TRACE(glEnable, #__VA_ARGS__, __VA_ARGS__)
#  define glEnableVertexAttribArray(...) GL_ERROR_TRACE(glEnableVertexAttribArray, #__VA_ARGS__, __VA_ARGS__)
#  define glGenBuffers(...) GL_ERROR_TRACE(glGenBuffers, #__VA_ARGS__, __VA_ARGS__)
#  define glGetFloatv(...) GL_ERROR_TRACE(glGetFloatv, #__VA_ARGS__, __VA_ARGS__)
#  define glGetIntegerv(...) GL_ERROR_TRACE(glGetIntegerv, #__VA_ARGS__, __VA_ARGS__)
#  define glGetUniformLocation(...) GL_ERROR_TRACE_RET(GLuint, glGetUniformLocation, #__VA_ARGS__, __VA_ARGS__)
#  define glHint(...) GL_ERROR_TRACE(glHint, #__VA_ARGS__, __VA_ARGS__)
#  define glReadPixels(...) GL_ERROR_TRACE(glReadPixels, #__VA_ARGS__, __VA_ARGS__)
#  define glScissor(...) GL_ERROR_TRACE(glScissor, #__VA_ARGS__, __VA_ARGS__)
#  define glGenFramebuffers(...) GL_ERROR_TRACE(glGenFramebuffers, #__VA_ARGS__, __VA_ARGS__)
#  define glBindFramebuffer(...) GL_ERROR_TRACE(glBindFramebuffer, #__VA_ARGS__, __VA_ARGS__)
#  define glEndTiling(...) GL_ERROR_TRACE(glEndTiling, #__VA_ARGS__, __VA_ARGS__)
#  define glGetProgramBinary(...) GL_ERROR_TRACE(glGetProgramBinary, #__VA_ARGS__, __VA_ARGS__)
#  define glMapBuffer(...) GL_ERROR_TRACE_RET(void *, glMapBuffer, #__VA_ARGS__, __VA_ARGS__)
#  define glStartTiling(...) GL_ERROR_TRACE(glStartTiling, #__VA_ARGS__, __VA_ARGS__)
#  define glUnmapBuffer(...) GL_ERROR_TRACE(glUnmapBuffer, #__VA_ARGS__, __VA_ARGS__)
#  define glTexParameterf(...) GL_ERROR_TRACE(glTexParameterf, #__VA_ARGS__, __VA_ARGS__)
#  define glTexParameteri(...) GL_ERROR_TRACE(glTexParameteri, #__VA_ARGS__, __VA_ARGS__)
#  define glTexSubImage2D(...) GL_ERROR_TRACE(glTexSubImage2D, #__VA_ARGS__, __VA_ARGS__)
#  define glTexImage2D(...) GL_ERROR_TRACE(glTexImage2D, #__VA_ARGS__, __VA_ARGS__)
#  define glUniform1f(...) GL_ERROR_TRACE(glUniform1f, #__VA_ARGS__, __VA_ARGS__)
#  define glUniform1i(...) GL_ERROR_TRACE(glUniform1i, #__VA_ARGS__, __VA_ARGS__)
#  define glUniform2fv(...) GL_ERROR_TRACE(glUniform2fv, #__VA_ARGS__, __VA_ARGS__)
#  define glUniform4fv(...) GL_ERROR_TRACE(glUniform4fv, #__VA_ARGS__, __VA_ARGS__)
#  define glUniformMatrix4fv(...) GL_ERROR_TRACE(glUniformMatrix4fv, #__VA_ARGS__, __VA_ARGS__)
#  define glUseProgram(...) GL_ERROR_TRACE(glUseProgram, #__VA_ARGS__, __VA_ARGS__)
#  define glVertexAttribPointer(...) GL_ERROR_TRACE(glVertexAttribPointer, #__VA_ARGS__, __VA_ARGS__)
#  define glViewport(...) GL_ERROR_TRACE(glViewport, #__VA_ARGS__, __VA_ARGS__)
#  define glPixelStorei(...) GL_ERROR_TRACE(glPixelStorei, #__VA_ARGS__, __VA_ARGS__)
#  define glCompressedTexImage2D(...) GL_ERROR_TRACE(glCompressedTexImage2D, #__VA_ARGS__, __VA_ARGS__)
#  define glCreateShader(...) GL_ERROR_TRACE_RET(GLuint, glCreateShader, #__VA_ARGS__, __VA_ARGS__)
#  define glCreateProgram(...) GL_ERROR_TRACE_RET(GLuint, glCreateProgram, #__VA_ARGS__, __VA_ARGS__)
#  define glAttachShader(...) GL_ERROR_TRACE(glAttachShader, #__VA_ARGS__, __VA_ARGS__)
#  define glLinkProgram(...) GL_ERROR_TRACE(glLinkProgram, #__VA_ARGS__, __VA_ARGS__)
#  define glGetProgramiv(...) GL_ERROR_TRACE(glGetProgramiv, #__VA_ARGS__, __VA_ARGS__)
#  define glGetProgramInfoLog(...) GL_ERROR_TRACE(glGetProgramInfoLog, #__VA_ARGS__, __VA_ARGS__)
#  define glGetShaderiv(...) GL_ERROR_TRACE(glGetShaderiv, #__VA_ARGS__, __VA_ARGS__)
#  define glShaderSource(...) GL_ERROR_TRACE(glShaderSource, #__VA_ARGS__, __VA_ARGS__)
#  define glCompileShader(...) GL_ERROR_TRACE(glCompileShader, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glGenFramebuffers(...) GL_ERROR_TRACE(glsym_glGenFramebuffers, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glBindFramebuffer(...) GL_ERROR_TRACE(glsym_glBindFramebuffer, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glFramebufferTexture2D(...) GL_ERROR_TRACE(glsym_glFramebufferTexture2D, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glDeleteFramebuffers(...) GL_ERROR_TRACE(glsym_glDeleteFramebuffers, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glGetProgramBinary(...) GL_ERROR_TRACE(glsym_glGetProgramBinary, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glProgramBinary(...) GL_ERROR_TRACE(glsym_glProgramBinary, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glProgramParameteri(...) GL_ERROR_TRACE(glsym_glProgramParameteri, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glReleaseShaderCompiler(...) GL_ERROR_TRACE(glsym_glReleaseShaderCompiler, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glMapBuffer(...) GL_ERROR_TRACE_RET(void *, glsym_glMapBuffer, #__VA_ARGS__, __VA_ARGS__)
#  define glsym_glUnmapBuffer(...) GL_ERROR_TRACE_RET(unsigned int, glsym_glUnmapBuffer, #__VA_ARGS__, __VA_ARGS__)
#  define eglsym_eglCreateImage(...) GL_ERROR_TRACE_RET(void *, eglsym_eglCreateImage, #__VA_ARGS__, __VA_ARGS__)
#  define eglsym_eglCreateImageKHR(...) GL_ERROR_TRACE_RET(void *, eglsym_eglCreateImageKHR, #__VA_ARGS__, __VA_ARGS__)
#  define eglsym_eglDestroyImage(...) GL_ERROR_TRACE_RET(unsigned int, eglsym_eglDestroyImage, #__VA_ARGS__, __VA_ARGS__)
#  define secsym_glEGLImageTargetTexture2DOES(...) GL_ERROR_TRACE(secsym_glEGLImageTargetTexture2DOES, #__VA_ARGS__, __VA_ARGS__)
#  define secsym_eglMapImageSEC(...) GL_ERROR_TRACE_RET(void *, secsym_eglMapImageSEC, #__VA_ARGS__, __VA_ARGS__)
#  define secsym_eglUnmapImageSEC(...) GL_ERROR_TRACE_RET(unsigned int, secsym_eglUnmapImageSEC, #__VA_ARGS__, __VA_ARGS__)
#  define secsym_eglGetImageAttribSEC(...) GL_ERROR_TRACE_RET(unsigned int, secsym_eglGetImageAttribSEC, #__VA_ARGS__, __VA_ARGS__)
# endif
#else
# define GLERRV(op)
#endif

Eina_Bool evas_gl_common_module_open(void);
void      evas_gl_common_module_close(void);

#ifndef EVAS_GL_NO_HEADERS

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

#endif

#include "evas_gl_3d_common.h"

#undef EAPI
#define EAPI

extern Eina_Bool _need_context_restore;
extern void _context_restore(void);

#endif
