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

#include "Evas.h"

#include <Imlib2.h>

#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))

typedef void Evas_Imlib_Image;
typedef void Evas_Imlib_Font;
typedef void Evas_Imlib_Graident;

typedef struct _evas_imlib_drawable Evas_Imlib_Drawable;
typedef struct _evas_imlib_update Evas_Imlib_Update;

struct _evas_imlib_drawable
{
   Display *disp;
   Window win;
   Evas_List tmp_images;
};

struct _evas_imlib_update
{
   Imlib_Image image;
   int x, y, w, h;
};

/***************/
/* image stuff */
/***************/
Evas_Imlib_Image *__evas_imlib_image_new_from_file(Display *disp, char *file);
void              __evas_imlib_image_free(Evas_Imlib_Image *im);
void              __evas_imlib_image_cache_empty(Display *disp);
void              __evas_imlib_image_cache_set_size(Display *disp, int size);
int               __evas_imlib_image_cache_get_size(Display *disp);
int               __evas_imlib_image_get_width(Evas_Imlib_Image *im);
int               __evas_imlib_image_get_height(Evas_Imlib_Image *im);
void              __evas_imlib_image_set_borders(Evas_Imlib_Image *im, int left, int right, int top, int bottom);
void              __evas_imlib_image_set_smooth_scaling(int on);
void              __evas_imlib_image_draw(Evas_Imlib_Image *im, Display *disp, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h);

/********/
/* text */
/********/
Evas_Imlib_Font  *__evas_imlib_text_font_new(Display *disp, char *font, int size);
void              __evas_imlib_text_font_free(Evas_Imlib_Font *fn);
int               __evas_imlib_text_font_get_ascent(Evas_Imlib_Font *fn);
int               __evas_imlib_text_font_get_descent(Evas_Imlib_Font *fn);
void              __evas_imlib_text_font_add_path(char *path);
void              __evas_imlib_text_font_del_path(char *path);
char            **__evas_imlib_text_font_list_paths(int *count);
void              __evas_imlib_text_cache_empty(Display *disp);
void              __evas_imlib_text_cache_set_size(Display *disp, int size);
int               __evas_imlib_text_cache_get_size(Display *disp);
void              __evas_imlib_text_get_size(Evas_Imlib_Font *fn, char *text, int *w, int *h);
int               __evas_imlib_text_get_character_at_pos(Evas_Imlib_Font *fn, char *text, int x, int y, int *cx, int *cy, int *cw, int *ch);
void              __evas_imlib_text_get_character_number(Evas_Imlib_Font *fn, char *text, int num, int *cx, int *cy, int *cw, int *ch);
void              __evas_imlib_text_draw(Evas_Imlib_Font *fn, Display *disp, Window win, int win_w, int win_h, int x, int y, char *text, int r, int g, int b, int a);

/**************/
/* rectangles */
/**************/

void              __evas_imlib_rectangle_draw(Display *disp, Window win, int win_w, int win_h, int x, int y, int w, int h, int r, int g, int b, int a);

/*********/
/* lines */
/*********/

void              __evas_imlib_line_draw(Display *disp, Window win, int win_w, int win_h, int x1, int y1, int x2, int y2, int r, int g, int b, int a);

/*************/
/* gradients */
/*************/
Evas_Imlib_Graident *__evas_imlib_gradient_new(Display *disp);
void                 __evas_imlib_gradient_free(Evas_Imlib_Graident *gr);
void                 __evas_imlib_gradient_color_add(Evas_Imlib_Graident *gr, int r, int g, int b, int a, int dist);
void                 __evas_imlib_gradient_draw(Evas_Imlib_Graident *gr, Display *disp, Window win, int win_w, int win_h, int x, int y, int w, int h, double angle);

/***********/
/* drawing */
/***********/
void         __evas_imlib_init(Display *disp, int screen);
int          __evas_imlib_capable(Display *disp);
void         __evas_imlib_flush_draw(Display *disp, Window win);
void         __evas_imlib_sync(Display *disp);
Visual      *__evas_imlib_get_visual(Display *disp, int screen);
XVisualInfo *__evas_imlib_get_visual_info(Display *disp, int screen);
Colormap     __evas_imlib_get_colormap(Display *disp, int screen);
void         __evas_imlib_draw_add_rect(Display *disp, Window win, int x, int y, int w, int h);

