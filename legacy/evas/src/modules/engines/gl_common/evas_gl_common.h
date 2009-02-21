#ifndef EVAS_GL_COMMON_H
#define EVAS_GL_COMMON_H

#define EVAS_GL_COMMON_NOCUTOUTS 1

/* FIXME: need to handle memory errors */
/* FIXME: need to handle list errors */
/* FIXME: need to handle gl errors */
/* FIXME: need to free textures is texture ream runs out */
/* FIXME: need to break image textures into meshes if too big */
/* FIXME: need to page mesh textures if texture alloc fails */

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
#include <GL/glew.h>
#endif /* BUILD_ENGINE_GL_GLEW */

#ifdef BUILD_ENGINE_GL_QUARTZ
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif /* BUILD_ENGINE_GL_QUARTZ */

typedef struct _Evas_GL_Context                      Evas_GL_Context;
typedef struct _Evas_GL_Texture                      Evas_GL_Texture;
typedef struct _Evas_GL_Image                        Evas_GL_Image;
typedef struct _Evas_GL_Polygon                      Evas_GL_Polygon;
typedef struct _Evas_GL_Polygon_Point                Evas_GL_Polygon_Point;
typedef struct _Evas_GL_Gradient                     Evas_GL_Gradient;
typedef struct _Evas_GL_Font_Texture                 Evas_GL_Font_Texture;
typedef struct _Evas_GL_Font_Texture_Pool            Evas_GL_Font_Texture_Pool;
typedef struct _Evas_GL_Font_Texture_Pool_Allocation Evas_GL_Font_Texture_Pool_Allocation;

struct _Evas_GL_Context
{
   int             w, h;

   unsigned char   dither : 1;
   unsigned char   blend : 1;
   unsigned char   blend_alpha : 1;
   unsigned char   r, g, b, a;

   struct {
      unsigned char size : 1;
      unsigned char dither : 1;
      unsigned char blend : 1;
      unsigned char color : 1;
      unsigned char texture : 1;
      unsigned char clip : 1;
      unsigned char buf : 1;
      unsigned char other : 1;
   } change;

   struct {
      unsigned char active : 1;
      int           x, y, w, h;
   } clip;

   struct {
      int checked : 1;
      int sgis_generate_mipmap : 1;
      int nv_texture_rectangle : 1;
      int arb_texture_non_power_of_two : 1;
      int arb_program : 1;
   } ext;

   GLenum          read_buf;
   GLenum          write_buf;

   Evas_GL_Texture      *texture;
   GLuint                font_texture;
   unsigned char         font_texture_rectangle : 1;
   unsigned char         texture_program : 1;

   int             max_texture_depth;
   int             max_texture_size;

   int             references;

   Eina_List      *images;
   Eina_List      *tex_pool;

   RGBA_Draw_Context  *dc;
   
   struct {
      GLhandleARB  prog, fshad;
   } yuv422p;
};

struct _Evas_GL_Texture
{
   Evas_GL_Context *gc;
   int              w, h;
   int              tw, th;
   int              uw, uh;

   GLuint           texture, texture2, texture3;

   unsigned char    smooth : 1;
   unsigned char    changed : 1;
   unsigned char    have_mipmaps : 1;
   unsigned char    rectangle : 1;
   unsigned char    not_power_of_two : 1;
   unsigned char    opt : 1;

   int              references;
   GLhandleARB      prog;
};

struct _Evas_GL_Image
{
   Evas_GL_Context *gc;
   RGBA_Image      *im;
   Evas_GL_Texture *tex;
   RGBA_Image_Loadopts load_opts;
   int              putcount;
   int              references;
   struct {
      int           space;
      void         *data;
      unsigned char no_free : 1;
   } cs;
   unsigned char    dirty : 1;
   unsigned char    cached : 1;
};

struct _Evas_GL_Polygon
{
   Eina_List *points;
   GLuint     dl;
   Evas_Bool  changed : 1;
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

struct _Evas_GL_Font_Texture
{
   Evas_GL_Context                      *gc;
   int                                   x, y, w, h;
   double                                tx1, ty1, tx2, ty2;
   int                                   aw, ah;
   GLuint                                texture;
   Evas_GL_Font_Texture_Pool            *pool;
   Evas_GL_Font_Texture_Pool_Allocation *alloc;
};

struct _Evas_GL_Font_Texture_Pool
{
   Evas_GL_Context *gc;
   int              w, h;
   GLuint           texture;
   int              references;
   unsigned char    rectangle : 1;
   Eina_List       *allocations;
};

struct _Evas_GL_Font_Texture_Pool_Allocation
{
   Evas_GL_Font_Texture_Pool *pool;
   int x, y, w, h;
};

Evas_GL_Context  *evas_gl_common_context_new(void);
void              evas_gl_common_context_free(Evas_GL_Context *gc);
void              evas_gl_common_context_use(Evas_GL_Context *gc);
void              evas_gl_common_context_resize(Evas_GL_Context *gc, int w, int h);
void              evas_gl_common_context_color_set(Evas_GL_Context *gc, int r, int g, int b, int a);
void              evas_gl_common_context_blend_set(Evas_GL_Context *gc, int blend);
void              evas_gl_common_context_dither_set(Evas_GL_Context *gc, int dither);
void              evas_gl_common_context_texture_set(Evas_GL_Context *gc, Evas_GL_Texture *tex, int smooth, int w, int h);
void              evas_gl_common_context_font_texture_set(Evas_GL_Context *gc, Evas_GL_Font_Texture *ft);
void              evas_gl_common_context_clip_set(Evas_GL_Context *gc, int on, int x, int y, int w, int h);
void              evas_gl_common_context_read_buf_set(Evas_GL_Context *gc, GLenum buf);
void              evas_gl_common_context_write_buf_set(Evas_GL_Context *gc, GLenum buf);

Evas_GL_Texture  *evas_gl_common_texture_new(Evas_GL_Context *gc, RGBA_Image *im, int smooth);
void              evas_gl_common_texture_update(Evas_GL_Texture *tex, RGBA_Image *im, int smooth);
void              evas_gl_common_texture_free(Evas_GL_Texture *tex);
void              evas_gl_common_texture_mipmaps_build(Evas_GL_Texture *tex, RGBA_Image *im, int smooth);
Evas_GL_Texture  *evas_gl_common_ycbcr601pl_texture_new(Evas_GL_Context *gc, unsigned char **rows, int w, int h, int smooth);
void              evas_gl_common_ycbcr601pl_texture_update(Evas_GL_Texture *tex, unsigned char **rows, int w, int h, int smooth);
    
Evas_GL_Image    *evas_gl_common_image_load(Evas_GL_Context *gc, const char *file, const char *key, Evas_Image_Load_Opts *lo);
Evas_GL_Image    *evas_gl_common_image_new_from_data(Evas_GL_Context *gc, int w, int h, DATA32 *data, int alpha, int cspace);
Evas_GL_Image    *evas_gl_common_image_new_from_copied_data(Evas_GL_Context *gc, int w, int h, DATA32 *data, int alpha, int cspace);
Evas_GL_Image    *evas_gl_common_image_new(Evas_GL_Context *gc, int w, int h, int alpha, int cspace);
void              evas_gl_common_image_free(Evas_GL_Image *im);
void              evas_gl_common_image_dirty(Evas_GL_Image *im);

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

void              evas_gl_common_rect_draw(Evas_GL_Context *gc, int x, int y, int w, int h);
void              evas_gl_common_image_draw(Evas_GL_Context *gc, Evas_GL_Image *im, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int smooth);
void              evas_gl_common_line_draw(Evas_GL_Context *gc, int x1, int y1, int x2, int y2);
void              evas_gl_common_poly_draw(Evas_GL_Context *gc, Evas_GL_Polygon *poly);

Evas_GL_Font_Texture *evas_gl_font_texture_new(Evas_GL_Context *gc, RGBA_Font_Glyph *fg);
void                  evas_gl_font_texture_free(Evas_GL_Font_Texture *ft);
void                  evas_gl_font_texture_draw(Evas_GL_Context *gc, void *surface, RGBA_Draw_Context *dc, RGBA_Font_Glyph *fg, int x, int y);

/* FIXME:
 *
 * for images:
 * speculative cache for textures too
 * texture mesh support
 *
 * for text/fonts:
 * need to not render to a texture each time.... this is sloooooow.
 * but its a "bootstrap" for just right now.
 */

#endif
