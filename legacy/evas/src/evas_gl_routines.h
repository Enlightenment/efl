#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef HAVE_GL
#include <GL/gl.h>
#include <GL/glx.h>
#ifdef HAVE_GLU
#include <GL/glu.h>
#endif
#else
typedef int GLXContext;
typedef int GLuint;
#endif

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xmu/StdCmap.h>

#ifdef HAVE_FREETYPE1_FREETYPE_FREETYPE_H
#include <freetype1/freetype/freetype.h>
#else
#ifdef HAVE_FREETYPE_FREETYPE_H
#include <freetype/freetype.h>
#else
#include <freetype.h>
#endif
#endif

#include "Evas_private.h"
#include "Evas.h"

typedef struct _evas_gl_image            Evas_GL_Image;
typedef struct _evas_gl_font             Evas_GL_Font;
typedef struct _evas_gl_gradient         Evas_GL_Graident;

typedef struct _evas_gl_font_texture     Evas_GL_Font_Texture;
typedef struct _evas_gl_glpyh_texture    Evas_GL_Glyph_Texture;
typedef struct _evas_gl_glyph            Evas_GL_Glyph;
typedef struct _evas_gl_gradient_texture Evas_GL_Graident_Texture;
typedef struct _evas_gl_window           Evas_GL_Window;
typedef struct _evas_gl_context          Evas_GL_Context;
typedef struct _evas_gl_rect             Evas_GL_Rect;
typedef struct _evas_gl_texture          Evas_GL_Texture;
typedef struct _evas_gl_texmesh          Evas_GL_Texmesh;

struct _evas_gl_window
{
   Display         *disp;
   Window           win;
   Window           root;
   int              screen;
   Evas_GL_Context *context;
   Evas_List        updates;
   int              w, h;
};

struct _evas_gl_context
{
   Display        *disp;
   int             screen;
   GLXContext      context;
   Window          win;
   Window          root;
   XVisualInfo    *visualinfo;
   Visual         *visual;
   Colormap        colormap;
   int             dither;
   int             blend;
   int             texture;
   DATA32          color;
   struct {
      int active;
      int x, y, w, h;
   } clip;
#ifdef HAVE_GL
   GLenum          read_buf;
   GLenum          write_buf;
#endif
   Evas_GL_Texture *bound_texture;
   
   int             max_texture_depth;
   int             max_texture_size;
};

struct _evas_gl_rect
{
   int x, y, w, h;
};

struct _evas_gl_texture
{
   int w, h;
#ifdef HAVE_GL
   GLuint texture;
#endif
   int smooth;
};

struct _evas_gl_texmesh
{
   struct {
      int x, y;
      int x_edge, y_edge;
      int x_left, y_left;
   } tiles;
   Evas_GL_Texture **textures;
   Evas_GL_Window  *window;
   Evas_GL_Context *context;
};

struct _evas_gl_image
{
   char *file;
   Imlib_Image im;
   int w, h;
   struct {
      int l, r, t, b;
   } border;
   int has_alpha;
   Evas_List textures;
   int references;
};

struct _evas_gl_gradient_texture
{
   Evas_GL_Window  *window;
   Evas_GL_Context *context;   
   Evas_GL_Texture *texture;
};

struct _evas_gl_gradient
{
   Imlib_Color_Range col_range;
   Evas_List textures;
};

struct _evas_gl_glpyh_texture
{
   struct {
      double        x1, x2, y1, y2;
   } tex;
   Evas_GL_Window  *window;
   Evas_GL_Context *context;   
   Evas_GL_Texture *texture;
};

struct _evas_gl_glyph
{
   int              glyph_id;
   TT_Glyph         glyph;
   
   TT_Glyph_Metrics metrics;
   
   Evas_GL_Glyph_Texture *texture;
   
   struct {
      double        x1, x2, y1, y2;
   } tex;
   
   Evas_List textures;
};

struct _evas_gl_font_texture
{
   struct {
      int x, y;
      int row_h;
   } cursor;
   Evas_GL_Window  *window;
   Evas_GL_Context *context;
   Evas_GL_Texture *texture;
};

struct _evas_gl_font
{
   char *font;
   int   size;
   
   TT_Engine           engine;
   TT_Face             face;
   TT_Instance         instance;
   TT_Face_Properties  properties;   
   TT_CharMap          char_map;
   TT_Instance_Metrics metrics;
   
   Evas_List glyphs[256];
   
   Evas_List textures;
   
   int ascent;
   int descent;
   int max_descent;
   int max_ascent;
   
   int references;
};

/***************/
/* image stuff */
/***************/
Evas_GL_Image *__evas_gl_image_new_from_file(Display *disp, char *file);
void           __evas_gl_image_free(Evas_GL_Image *im);
void           __evas_gl_image_cache_empty(Display *disp);
void           __evas_gl_image_cache_set_size(Display *disp, int size);
int            __evas_gl_image_cache_get_size(Display *disp);
int            __evas_gl_image_get_width(Evas_GL_Image *im);
int            __evas_gl_image_get_height(Evas_GL_Image *im);
void           __evas_gl_image_set_borders(Evas_GL_Image *im, int left, int right, int top, int bottom);
void           __evas_gl_image_set_smooth_scaling(int on);
void           __evas_gl_image_draw(Evas_GL_Image *im, Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int cr, int cg, int cb, int ca);

/********/
/* text */
/********/
Evas_GL_Font  *__evas_gl_text_font_new(Display *disp, char *font, int size);
void           __evas_gl_text_font_free(Evas_GL_Font *fn);
int            __evas_gl_text_font_get_ascent(Evas_GL_Font *fn);
int            __evas_gl_text_font_get_descent(Evas_GL_Font *fn);
int            __evas_gl_text_font_get_max_ascent(Evas_GL_Font *fn);
int            __evas_gl_text_font_get_max_descent(Evas_GL_Font *fn);
void           __evas_gl_text_font_get_advances(Evas_GL_Font *fn, char *text, int *advance_horiz, int *advance_vert);
int            __evas_gl_text_font_get_first_inset(Evas_GL_Font *fn, char *text);
void           __evas_gl_text_font_add_path(char *path);
void           __evas_gl_text_font_del_path(char *path);
char         **__evas_gl_text_font_list_paths(int *count);
void           __evas_gl_text_cache_empty(Display *disp);
void           __evas_gl_text_cache_set_size(Display *disp, int size);
int            __evas_gl_text_cache_get_size(Display *disp);
void           __evas_gl_text_get_size(Evas_GL_Font *fn, char *text, int *w, int *h);
int            __evas_gl_text_get_character_at_pos(Evas_GL_Font *fn, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
void           __evas_gl_text_get_character_number(Evas_GL_Font *fn, char *text, int num, int *cx, int *cy, int *cw, int *ch);
void           __evas_gl_text_draw(Evas_GL_Font *fn, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, char *text, int r, int g, int b, int a);

/**************/
/* rectangles */
/**************/
void           __evas_gl_rectangle_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, int r, int g, int b, int a);

/*********/
/* lines */
/*********/
void           __evas_gl_line_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

/*************/
/* gradients */
/*************/
Evas_GL_Graident *__evas_gl_gradient_new(Display *disp);
void              __evas_gl_gradient_free(Evas_GL_Graident *gr);
void              __evas_gl_gradient_color_add(Evas_GL_Graident *gr, int r, int g, int b, int a, int dist);
void              __evas_gl_gradient_draw(Evas_GL_Graident *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle);

/************/
/* polygons */
/************/
void              __evas_gl_poly_draw (Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, Evas_List points, int r, int g, int b, int a);

/***********/
/* drawing */
/***********/
void         __evas_gl_set_clip_rect(int on, int x, int y, int w, int h, int r, int g, int b, int a);
void         __evas_gl_init(Display *disp, int screen, int colors);
void         __evas_gl_set_vis_cmap(Visual *vis, Colormap cmap);
int          __evas_gl_capable(Display *disp);
void         __evas_gl_flush_draw(Display *disp, Imlib_Image dstim, Window win);
void         __evas_gl_sync(Display *disp);
Visual      *__evas_gl_get_visual(Display *disp, int screen);
XVisualInfo *__evas_gl_get_visual_info(Display *disp, int screen);
Colormap     __evas_gl_get_colormap(Display *disp, int screen);
void         __evas_gl_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, int x, int y, int w, int h);
