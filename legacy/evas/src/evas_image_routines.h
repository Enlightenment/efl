#include "config.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "Evas_private.h"
#include "Evas.h"

#include <Imlib2.h>

#ifndef SPANS_COMMON
# define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))
#endif

typedef void Evas_Image_Image;
typedef void Evas_Image_Font;

typedef struct _evas_image_drawable Evas_Image_Drawable;
typedef struct _evas_image_update Evas_Image_Update;
typedef struct _evas_image_color Evas_Image_Color;
typedef struct _evas_image_gradient Evas_Image_Graident;

struct _evas_image_drawable
{
   Imlib_Image im;
   Evas_List tmp_images;
};

struct _evas_image_update
{
   Imlib_Image image;
   int x, y, w, h;
};

struct _evas_image_color
{
   int r, g, b, a;
   int dist;
};

struct _evas_image_gradient
{
   Evas_List colors;
};

/***************/
/* image stuff */
/***************/
Evas_Image_Image *__evas_image_image_new_from_file(Display *disp, char *file);
void              __evas_image_image_free(Evas_Image_Image *im);
void              __evas_image_image_cache_empty(Display *disp);
void              __evas_image_image_cache_set_size(Display *disp, int size);
int               __evas_image_image_cache_get_size(Display *disp);
int               __evas_image_image_get_width(Evas_Image_Image *im);
int               __evas_image_image_get_height(Evas_Image_Image *im);
void              __evas_image_image_set_borders(Evas_Image_Image *im, int left, int right, int top, int bottom);
void              __evas_image_image_set_smooth_scaling(int on);
void              __evas_image_image_draw(Evas_Image_Image *im, Display *disp, Imlib_Image dstim, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int cr, int cg, int cb, int ca);

/********/
/* text */
/********/
Evas_Image_Font  *__evas_image_text_font_new(Display *disp, char *font, int size);
void              __evas_image_text_font_free(Evas_Image_Font *fn);
int               __evas_image_text_font_get_ascent(Evas_Image_Font *fn);
int               __evas_image_text_font_get_descent(Evas_Image_Font *fn);
int               __evas_image_text_font_get_max_ascent(Evas_Image_Font *fn);
int               __evas_image_text_font_get_max_descent(Evas_Image_Font *fn);
void              __evas_image_text_font_get_advances(Evas_Image_Font *fn, char *text, int *advance_horiz, int *advance_vert);
int               __evas_image_text_font_get_first_inset(Evas_Image_Font *fn, char *text);
void              __evas_image_text_font_add_path(char *path);
void              __evas_image_text_font_del_path(char *path);
char            **__evas_image_text_font_list_paths(int *count);
void              __evas_image_text_cache_empty(Display *disp);
void              __evas_image_text_cache_set_size(Display *disp, int size);
int               __evas_image_text_cache_get_size(Display *disp);
void              __evas_image_text_get_size(Evas_Image_Font *fn, char *text, int *w, int *h);
int               __evas_image_text_get_character_at_pos(Evas_Image_Font *fn, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
void              __evas_image_text_get_character_number(Evas_Image_Font *fn, char *text, int num, int *cx, int *cy, int *cw, int *ch);
void              __evas_image_text_draw(Evas_Image_Font *fn, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, char *text, int r, int g, int b, int a);

/**************/
/* rectangles */
/**************/

void              __evas_image_rectangle_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, int r, int g, int b, int a);

/*********/
/* lines */
/*********/

void              __evas_image_line_draw(Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

/*************/
/* gradients */
/*************/
Evas_Image_Graident *__evas_image_gradient_new(Display *disp);
void                 __evas_image_gradient_free(Evas_Image_Graident *gr);
void                 __evas_image_gradient_color_add(Evas_Image_Graident *gr, int r, int g, int b, int a, int dist);
void                 __evas_image_gradient_draw(Evas_Image_Graident *gr, Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle);

/************/
/* polygons */
/************/
void                 __evas_image_poly_draw (Display *disp, Imlib_Image dstim, Window win, int win_w, int win_h, Evas_List points, int r, int g, int b, int a);

/***********/
/* drawing */
/***********/
void         __evas_image_set_clip_rect(int on, int x, int y, int w, int h, int r, int g, int b, int a);
void         __evas_image_init(Display *disp, int screen, int colors);
int          __evas_image_capable(Display *disp);
void         __evas_image_flush_draw(Display *disp, Imlib_Image dstim, Window win);
void         __evas_image_sync(Display *disp);
Visual      *__evas_image_get_visual(Display *disp, int screen);
XVisualInfo *__evas_image_get_visual_info(Display *disp, int screen);
Colormap     __evas_image_get_colormap(Display *disp, int screen);
void         __evas_image_draw_add_rect(Display *disp, Imlib_Image dstim, Window win, int x, int y, int w, int h);

