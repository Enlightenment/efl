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

typedef void Evas_Imlib_Image;
typedef void Evas_Imlib_Font;

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
void              __evas_imlib_image_draw(Evas_Imlib_Image *im, Display *disp, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h);
int               __evas_imlib_image_get_width(Evas_Imlib_Image *im);
int               __evas_imlib_image_get_height(Evas_Imlib_Image *im);

/********/
/* text */
/********/
Evas_Imlib_Font  *__evas_imlib_text_font_new(Display *disp, char *font, int size);
void              __evas_imlib_text_font_free(Evas_Imlib_Font *fn);
void              __evas_imlib_text_font_add_path(char *path);
void              __evas_imlib_text_font_del_path(char *path);
char            **__evas_imlib_text_font_list_paths(int *count);
void              __evas_imlib_text_cache_empty(Display *disp);
void              __evas_imlib_text_cache_set_size(Display *disp, int size);
int               __evas_imlib_text_cache_get_size(Display *disp);
void              __evas_imlib_text_draw(Evas_Imlib_Font *fn, Display *disp, Window win, int x, int y, char *text, int r, int g, int b, int a);

/**************/
/* rectangles */
/**************/

/*********/
/* lines */
/*********/

/*************/
/* gradients */
/*************/

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

