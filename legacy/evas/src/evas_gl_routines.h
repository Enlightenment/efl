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

#ifdef HAVE_FREETYPE_FREETYPE_H
#include <freetype/freetype.h>
#else
#include <freetype.h>
#endif

#include "Evas.h"

typedef struct _evas_gl_image          Evas_GL_Image;
typedef struct _evas_gl_font           Evas_GL_Font;
typedef struct _evas_gl_glyph_info     Evas_GL_Glyph_Info;
typedef struct _evas_gl_gradient       Evas_GL_Graident;
typedef struct _evas_gl_gradient_color Evas_GL_Graident_Color;
typedef enum   _evas_gl_image_state    Evas_GL_Image_State;

enum _evas_gl_image_state
{
   EVAS_STATE_DATA,
   EVAS_STATE_TEXTURE
};

struct _evas_gl_image
{
   Evas_GL_Image_State state;
   int w, h;
   int direct;
   int bl, br, bt, bb;
   int alpha;
   char *file;
   /* data specific params */
   DATA32 *data;
   /* common GL params */
   GLXContext context;
   /* texture state specific params */
   struct
     {
	int max_size;
	int w, h;
	int edge_w, edge_h;
	GLuint *textures;
     } texture;
   /* buffer specific params */
   struct
     {
	Display *display;
	XVisualInfo *visual_info;
	Colormap colormap;
	Window window, dest;
	int dest_w, dest_h;
     } buffer;
   int references;
};

struct _evas_gl_font
{
   char *file;
   int   size;

   TT_Engine           engine;
   TT_Face             face;
   TT_Instance         instance;
   TT_Face_Properties  properties;
   int                 num_glyph;
   TT_Glyph           *glyphs;
   Evas_GL_Glyph_Info *glyphinfo;
   int                 max_descent;
   int                 max_ascent;
   int                 descent;
   int                 ascent;
   int                 mem_use;
   
   GLXContext  context;
   int         max_texture_size;
   int         num_textures;
   GLuint     *textures;
   struct
     {
	Display *display;
	XVisualInfo *visual_info;
	Colormap colormap;
	Window window, dest;
	int dest_w, dest_h;
     } buffer;
   int   references;
};

struct _evas_gl_glyph_info
{
   GLuint texture;
   int    px, py, pw, ph;
   double x1, y1, x2, y2;
};

struct _evas_gl_gradient_color
{
   int r, g, b, a;
   int dist;
};

struct _evas_gl_gradient
{
   Evas_List   colors;
   GLXContext  context;
   int         max_texture_size;
   int         texture_w, texture_h;
   GLuint      texture;
   struct
     {
	Display *display;
	XVisualInfo *visual_info;
	Colormap colormap;
	Window window, dest;
	int dest_w, dest_h;
     } buffer;
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
void           __evas_gl_image_draw(Evas_GL_Image *im, Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h);

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

/***********/
/* drawing */
/***********/
void         __evas_gl_init(Display *disp, int screen);
int          __evas_gl_capable(Display *disp);
void         __evas_gl_flush_draw(Display *disp, Imlib_Image dstim, Window win);
void         __evas_gl_sync(Display *disp);
Visual      *__evas_gl_get_visual(Display *disp, int screen);
XVisualInfo *__evas_gl_get_visual_info(Display *disp, int screen);
Colormap     __evas_gl_get_colormap(Display *disp, int screen);
void         __evas_gl_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, int x, int y, int w, int h);
