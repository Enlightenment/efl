#ifndef EVAS_GL_COMMON_H
#define EVAS_GL_COMMON_H

#include "evas_common.h"
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

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
   
   char            dither : 1;
   char            blend : 1;
   unsigned char   r, g, b, a;

   struct {
      char         size : 1;
      char         dither : 1;
      char         blend : 1;
      char         color : 1;
      char         texture : 1;
      char         clip : 1;
      char         buf : 1;
      char         other : 1;
   } change;
   
   struct {
      char         active : 1;
      int          x, y, w, h;
   } clip;
   
   struct {
      int checked : 1;
      int sgis_generate_mipmap : 1;
      int nv_texture_rectangle : 1;
   } ext;
   
   GLenum          read_buf;
   GLenum          write_buf;
   
   Evas_GL_Texture      *texture;
   GLuint                font_texture;
   char                  font_texture_not_power_of_two : 1;
   
   int             max_texture_depth;
   int             max_texture_size;
   
   int             references;
   
   Evas_List      *images;
   Evas_List      *tex_pool;
};

struct _Evas_GL_Texture
{
   Evas_GL_Context *gc;
   int              w, h;
   int              tw, th;
   int              uw, uh;
   
   GLuint           texture;
   
   char             smooth : 1;
   char             changed : 1;
   char             have_mipmaps : 1;
   char             not_power_of_two : 1; 
   char             opt : 1;
   
   int              references;
};

struct _Evas_GL_Image
{
   Evas_GL_Context *gc;
   RGBA_Image      *im;
   Evas_GL_Texture *tex;
   int              references;
   char             dirty : 1;
   char             cached : 1;
};

struct _Evas_GL_Polygon
{
   Evas_List *points;
};

struct _Evas_GL_Polygon_Point
{
   int x, y;
};

struct _Evas_GL_Gradient
{
   RGBA_Gradient   *grad;
   Evas_GL_Texture *tex;
};

struct _Evas_GL_Font_Texture
{
   Evas_GL_Context                      *gc;
   int                                   x, y, w, h;
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
   char             not_power_of_two : 1; 
   Evas_List       *allocations;
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
    
Evas_GL_Image    *evas_gl_common_image_load(Evas_GL_Context *gc, char *file, char *key);
Evas_GL_Image    *evas_gl_common_image_new_from_data(Evas_GL_Context *gc, int w, int h, int *data);
Evas_GL_Image    *evas_gl_common_image_new_from_copied_data(Evas_GL_Context *gc, int w, int h, int *data);
Evas_GL_Image    *evas_gl_common_image_new(Evas_GL_Context *gc, int w, int h);
void              evas_gl_common_image_free(Evas_GL_Image *im);
void              evas_gl_common_image_dirty(Evas_GL_Image *im);    

Evas_GL_Polygon  *evas_gl_common_poly_point_add(Evas_GL_Polygon *poly, int x, int y);
Evas_GL_Polygon  *evas_gl_common_poly_points_clear(Evas_GL_Polygon *poly);

Evas_GL_Gradient *evas_gl_common_gradient_color_add(Evas_GL_Gradient *gr, int r, int g, int b, int a, int distance);
Evas_GL_Gradient *evas_gl_common_gradient_colors_clear(Evas_GL_Gradient *gr);
    
void              evas_gl_common_swap_rect(Evas_GL_Context *gc, int x, int y, int w, int h);

void              evas_gl_common_rect_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, int x, int y, int w, int h);
void              evas_gl_common_image_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, Evas_GL_Image *im, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int smooth);
void              evas_gl_common_line_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2);
void              evas_gl_common_poly_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, Evas_GL_Polygon *poly);
void              evas_gl_common_gradient_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, Evas_GL_Gradient *gr, int x, int y, int w, int h, double angle);

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
