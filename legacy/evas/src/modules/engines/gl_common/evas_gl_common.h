#ifndef EVAS_GL_COMMON_H
#define EVAS_GL_COMMON_H

#include "evas_common.h"
#include "evas_private.h"
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef BUILD_ENGINE_GL_GLEW
# include <GL/glew.h>
#else
# define GL_GLEXT_PROTOTYPES
#endif

#ifdef BUILD_ENGINE_GL_QUARTZ
# include <OpenGL/gl.h>
#else
# if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#  if defined(GLES_VARIETY_S3C6410)
#   include <GLES/gl.h>
#  elif defined(GLES_VARIETY_SGX)
#   include <GLES2/gl2.h>
#  endif
# else
#  include <GL/gl.h>
# endif
#endif

#ifndef GL_TEXTURE_RECTANGLE_NV
#define GL_TEXTURE_RECTANGLE_NV 0x84F5
#endif


#define SHAD_VERTEX 0
#define SHAD_COLOR  1
#define SHAD_TEXUV  2
#define SHAD_TEXUV2 3
#define SHAD_TEXUV3 4

typedef struct _Evas_GL_Program                      Evas_GL_Program;
typedef struct _Evas_GL_Program_Source               Evas_GL_Program_Source;
typedef struct _Evas_GL_Context                      Evas_GL_Context;
typedef struct _Evas_GL_Texture_Pool                 Evas_GL_Texture_Pool;
typedef struct _Evas_GL_Texture                      Evas_GL_Texture;
typedef struct _Evas_GL_Image                        Evas_GL_Image;
typedef struct _Evas_GL_Font_Texture                 Evas_GL_Font_Texture;
/*
typedef struct _Evas_GL_Polygon                      Evas_GL_Polygon;
typedef struct _Evas_GL_Polygon_Point                Evas_GL_Polygon_Point;
typedef struct _Evas_GL_Gradient                     Evas_GL_Gradient;
typedef struct _Evas_GL_Font_Texture_Pool            Evas_GL_Font_Texture_Pool;
typedef struct _Evas_GL_Font_Texture_Pool_Allocation Evas_GL_Font_Texture_Pool_Allocation;
*/

struct _Evas_GL_Program
{
   GLuint vert, frag, prog;
};

struct _Evas_GL_Program_Source
{
   const char *src;
   const unsigned int *bin;
   int bin_size;
};

struct _Evas_GL_Context
{
   int                references;
   int                w, h;
   RGBA_Draw_Context  *dc;
   
   Eina_List          *images;

   struct {
      Eina_List       *whole;
      Eina_List       *atlas[33][3];
   } tex;
   
   struct {
      GLint max_texture_units;
      GLint max_texture_size;
      Eina_Bool tex_npo2 : 1;
      Eina_Bool tex_rect : 1;
   } info;
   struct {
      int             x, y, w, h;
      Eina_Bool       active : 1;
   } clip;
   struct {
      Evas_GL_Program rect, img, font, yuv;
      GLuint          cur_prog;
      GLuint          cur_tex, cur_texu, cur_texv;
      Eina_Bool       smooth : 1;
      Eina_Bool       blend : 1;
      struct {
         GLuint          cur_prog;
         GLuint          cur_tex, cur_texum, cur_texv;
         Eina_Bool       smooth : 1;
         Eina_Bool       blend : 1;
      } current;
   } shader;
   struct {
      int num;
      int alloc;
      GLint   *vertex;
      GLfloat *color;
      GLfloat *texuv;
      GLfloat *texuv2;
      GLfloat *texuv3;
   } array;
   struct {
      Eina_Bool size : 1;
   } change;
   Eina_Bool checked : 1;
};

struct _Evas_GL_Texture_Pool
{
   Evas_GL_Context *gc;
   GLuint           texture;
   GLuint           format;
   int              w, h;
   int              references;
   int              slot, fslot;
   Eina_List       *allocations;
   Eina_Bool        whole : 1;
};

struct _Evas_GL_Texture
{
   Evas_GL_Context *gc;
   Evas_GL_Texture_Pool *pt, *ptu, *ptv;
   int              x, y, w, h;
   int              references;
};

struct _Evas_GL_Image
{
   Evas_GL_Context *gc;
   RGBA_Image      *im;
   Evas_GL_Texture *tex;
   RGBA_Image_Loadopts load_opts;
   int              references;
   struct {
      int           space;
      void         *data;
      unsigned char no_free : 1;
   } cs;
   unsigned char    dirty : 1;
   unsigned char    cached : 1;
};

struct _Evas_GL_Font_Texture
{
   Evas_GL_Texture *tex;
};
/*
struct _Evas_GL_Polygon
{
   Eina_List *points;
   GLuint     dl;
   Eina_Bool  changed : 1;
};

struct _Evas_GL_Polygon_Point
{
   int x, y;
};

struct _Evas_GL_Gradient
{
   RGBA_Gradient   *grad;
   Evas_GL_Texture *tex;
   int              tw, th;
   unsigned char    changed : 1;
};

struct _Evas_GL_Font_Texture_Pool
{
   Evas_GL_Context *gc;
   int              w, h;
   GLuint           texture;
   unsigned char    rectangle : 1;
};
*/

extern Evas_GL_Program_Source shader_rect_frag_src;
extern Evas_GL_Program_Source shader_rect_vert_src;
extern Evas_GL_Program_Source shader_img_frag_src;
extern Evas_GL_Program_Source shader_img_vert_src;
extern Evas_GL_Program_Source shader_font_frag_src;
extern Evas_GL_Program_Source shader_font_vert_src;
extern Evas_GL_Program_Source shader_yuv_frag_src;
extern Evas_GL_Program_Source shader_yuv_vert_src;

void glerr(const char *file, const char *func, int line, const char *op);
 
Evas_GL_Context  *evas_gl_common_context_new(void);
void              evas_gl_common_context_free(Evas_GL_Context *gc);
void              evas_gl_common_context_use(Evas_GL_Context *gc);
void              evas_gl_common_context_resize(Evas_GL_Context *gc, int w, int h);

void              evas_gl_common_context_rectangle_push(Evas_GL_Context *gc,
                                                        int x, int y, int w, int h,
                                                        int r, int g, int b, int a);
void              evas_gl_common_context_image_push(Evas_GL_Context *gc,
                                                    Evas_GL_Texture *tex,
                                                    double sx, double sy, double sw, double sh,
                                                    int x, int y, int w, int h,
                                                    int r, int g, int b, int a,
                                                    Eina_Bool smooth);
void              evas_gl_common_context_font_push(Evas_GL_Context *gc,
                                                   Evas_GL_Texture *tex,
                                                   double sx, double sy, double sw, double sh,
                                                   int x, int y, int w, int h,
                                                   int r, int g, int b, int a);
void             evas_gl_common_context_yuv_push(Evas_GL_Context *gc,
                                                 Evas_GL_Texture *tex,
                                                 double sx, double sy, double sw, double sh,
                                                 int x, int y, int w, int h,
                                                 int r, int g, int b, int a,
                                                 Eina_Bool smooth);
void              evas_gl_common_context_flush(Evas_GL_Context *gc);

void              evas_gl_common_shader_program_init(Evas_GL_Program *p,
                                                     Evas_GL_Program_Source *vert,
                                                     Evas_GL_Program_Source *frag);
    
void              evas_gl_common_rect_draw(Evas_GL_Context *gc, int x, int y, int w, int h);

Evas_GL_Texture  *evas_gl_common_texture_new(Evas_GL_Context *gc, RGBA_Image *im);
void              evas_gl_common_texture_update(Evas_GL_Texture *tex, RGBA_Image *im);
void              evas_gl_common_texture_free(Evas_GL_Texture *tex);
Evas_GL_Texture  *evas_gl_common_texture_alpha_new(Evas_GL_Context *gc, DATA8 *pixels, int w, int h, int fh);
void              evas_gl_common_texture_alpha_update(Evas_GL_Texture *tex, DATA8 *pixels, int w, int h, int fh);
Evas_GL_Texture  *evas_gl_common_texture_yuv_new(Evas_GL_Context *gc, DATA8 **rows, int w, int h);
void              evas_gl_common_texture_yuv_update(Evas_GL_Texture *tex, DATA8 **rows, int w, int h);

Evas_GL_Image    *evas_gl_common_image_load(Evas_GL_Context *gc, const char *file, const char *key, Evas_Image_Load_Opts *lo);
Evas_GL_Image    *evas_gl_common_image_new_from_data(Evas_GL_Context *gc, int w, int h, DATA32 *data, int alpha, int cspace);
Evas_GL_Image    *evas_gl_common_image_new_from_copied_data(Evas_GL_Context *gc, int w, int h, DATA32 *data, int alpha, int cspace);
Evas_GL_Image    *evas_gl_common_image_new(Evas_GL_Context *gc, int w, int h, int alpha, int cspace);
void              evas_gl_common_image_free(Evas_GL_Image *im);
void              evas_gl_common_image_dirty(Evas_GL_Image *im);
void              evas_gl_common_image_draw(Evas_GL_Context *gc, Evas_GL_Image *im, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int smooth);

Evas_GL_Texture  *evas_gl_font_texture_new(Evas_GL_Context *gc, RGBA_Font_Glyph *fg);
void              evas_gl_font_texture_free(Evas_GL_Texture *ft);
void              evas_gl_font_texture_draw(Evas_GL_Context *gc, void *surface, RGBA_Draw_Context *dc, RGBA_Font_Glyph *fg, int x, int y);




















/*
Evas_GL_Polygon  *evas_gl_common_poly_point_add(Evas_GL_Polygon *poly, int x, int y);
Evas_GL_Polygon  *evas_gl_common_poly_points_clear(Evas_GL_Polygon *poly);

Evas_GL_Gradient *evas_gl_common_gradient_new(void);
void              evas_gl_common_gradient_free(Evas_GL_Gradient *gr);
void              evas_gl_common_gradient_color_stop_add(Evas_GL_Gradient *gr, int r, int g, int b, int a, int delta);
void              evas_gl_common_gradient_alpha_stop_add(Evas_GL_Gradient *gr, int a, int delta);
void              evas_gl_common_gradient_color_data_set(Evas_GL_Gradient *gr, void *data, int len, int has_alpha);
void              evas_gl_common_gradient_alpha_data_set(Evas_GL_Gradient *gr, void *alpha_data, int len);
void              evas_gl_common_gradient_clear(Evas_GL_Gradient *gr);
void              evas_gl_common_gradient_fill_set(Evas_GL_Gradient *gr, int x, int y, int w, int h);
void              evas_gl_common_gradient_fill_angle_set(Evas_GL_Gradient *gr, double angle);
void              evas_gl_common_gradient_fill_spread_set(Evas_GL_Gradient *gr, int spread);
void              evas_gl_common_gradient_map_angle_set(Evas_GL_Gradient *gr, double angle);
void              evas_gl_common_gradient_map_offset_set(Evas_GL_Gradient *gr, float offset);
void              evas_gl_common_gradient_map_direction_set(Evas_GL_Gradient *gr, int direction);
void              evas_gl_common_gradient_type_set(Evas_GL_Gradient *gr, char *name, char *params);
int               evas_gl_common_gradient_is_opaque(Evas_GL_Context *gc, Evas_GL_Gradient *gr, int x, int y, int w, int h);
int               evas_gl_common_gradient_is_visible(Evas_GL_Context *gc, Evas_GL_Gradient *gr, int x, int y, int w, int h);
void              evas_gl_common_gradient_render_pre(Evas_GL_Context *gc, Evas_GL_Gradient *gr);
void              evas_gl_common_gradient_render_post(Evas_GL_Gradient *gr);
void              evas_gl_common_gradient_draw(Evas_GL_Context *gc, Evas_GL_Gradient *gr,
                                               int x, int y, int w, int h);

void              evas_gl_common_swap_rect(Evas_GL_Context *gc, int x, int y, int w, int h);

void              evas_gl_common_line_draw(Evas_GL_Context *gc, int x1, int y1, int x2, int y2);
void              evas_gl_common_poly_draw(Evas_GL_Context *gc, Evas_GL_Polygon *poly);
*/

#endif
