#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#include "Evas_private.h"
#include "Evas.h"

#include <X11/Xlib.h>
#include <Imlib2.h>

#ifdef HAVE_RENDER
#include <X11/extensions/Xrender.h>
#else
typedef Pixmap Picture;
#endif

#ifndef SPANS_COMMON
# define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))
#endif

typedef struct _evas_render_image Evas_Render_Image;
typedef void Evas_Render_Font;
typedef void Evas_Render_Graident;

struct _evas_render_image
{
   char    *file;
   Imlib_Image image;
   int      references;
   int      w, h;
   Display *disp;
   struct {
      int w, h;
      struct 
	{
	   int l, r, t, b;
	} border;
      int smooth;
      int pr, pg, pb, pa;
   } current;
   Picture  pic;
   Pixmap   pmap;
   int      has_alpha;
};

typedef struct _evas_render_drawable Evas_Render_Drawable;
typedef struct _evas_render_update Evas_Render_Update;

struct _evas_render_drawable
{
   Display *disp;
   Window win;
   Evas_List tmp_images;
};

struct _evas_render_update
{
   Display *disp;
   Drawable drawable;
   Picture pic;
   Pixmap pmap;
   int x, y, w, h;
};

/***************/
/* image stuff */
/***************/
Evas_Render_Image *__evas_render_image_new_from_file(Display *disp, char *file);
void              __evas_render_image_free(Evas_Render_Image *im);
void              __evas_render_image_cache_empty(Display *disp);
void              __evas_render_image_cache_set_size(Display *disp, int size);
int               __evas_render_image_cache_get_size(Display *disp);
int               __evas_render_image_get_width(Evas_Render_Image *im);
int               __evas_render_image_get_height(Evas_Render_Image *im);
void              __evas_render_image_set_borders(Evas_Render_Image *im, int left, int right, int top, int bottom);
void              __evas_render_image_set_smooth_scaling(int on);
void              __evas_render_image_draw(Evas_Render_Image *im, Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int cr, int cg, int cb, int ca);

/********/
/* text */
/********/
Evas_Render_Font  *__evas_render_text_font_new(Display *disp, char *font, int size);
void              __evas_render_text_font_free(Evas_Render_Font *fn);
int               __evas_render_text_font_get_ascent(Evas_Render_Font *fn);
int               __evas_render_text_font_get_descent(Evas_Render_Font *fn);
int               __evas_render_text_font_get_max_ascent(Evas_Render_Font *fn);
int               __evas_render_text_font_get_max_descent(Evas_Render_Font *fn);
void              __evas_render_text_font_get_advances(Evas_Render_Font *fn, char *text, int *advance_horiz, int *advance_vert);
int               __evas_render_text_font_get_first_inset(Evas_Render_Font *fn, char *text);
void              __evas_render_text_font_add_path(char *path);
void              __evas_render_text_font_del_path(char *path);
char            **__evas_render_text_font_list_paths(int *count);
void              __evas_render_text_cache_empty(Display *disp);
void              __evas_render_text_cache_set_size(Display *disp, int size);
int               __evas_render_text_cache_get_size(Display *disp);
void              __evas_render_text_get_size(Evas_Render_Font *fn, char *text, int *w, int *h);
int               __evas_render_text_get_character_at_pos(Evas_Render_Font *fn, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
void              __evas_render_text_get_character_number(Evas_Render_Font *fn, char *text, int num, int *cx, int *cy, int *cw, int *ch);
void              __evas_render_text_draw(Evas_Render_Font *fn, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, char *text, int r, int g, int b, int a);

/**************/
/* rectangles */
/**************/

void              __evas_render_rectangle_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, int r, int g, int b, int a);

/*********/
/* lines */
/*********/

void              __evas_render_line_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

/*************/
/* gradients */
/*************/
Evas_Render_Graident *__evas_render_gradient_new(Display *disp);
void                 __evas_render_gradient_free(Evas_Render_Graident *gr);
void                 __evas_render_gradient_color_add(Evas_Render_Graident *gr, int r, int g, int b, int a, int dist);
void                 __evas_render_gradient_draw(Evas_Render_Graident *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle);

/************/
/* polygons */
/************/
void                 __evas_render_poly_draw (Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, Evas_List points, int r, int g, int b, int a);

/***********/
/* drawing */
/***********/
void         __evas_render_set_clip_rect(int on, int x, int y, int w, int h, int r, int g, int b, int a);
void         __evas_render_init(Display *disp, int screen, int colors);
void         __evas_render_set_vis_cmap(Visual *vis, Colormap cmap);
int          __evas_render_capable(Display *disp);
void         __evas_render_flush_draw(Display *disp, Imlib_Image dstim, Window win);
void         __evas_render_sync(Display *disp);
Visual      *__evas_render_get_visual(Display *disp, int screen);
XVisualInfo *__evas_render_get_visual_info(Display *disp, int screen);
Colormap     __evas_render_get_colormap(Display *disp, int screen);
void         __evas_render_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, int x, int y, int w, int h);

