#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdCmap.h>

#include "Evas.h"

typedef struct _evas_gl_image       Evas_GL_Image;
typedef struct _evas_gl_font        Evas_GL_Font;
typedef enum   _evas_gl_image_state Evas_GL_Image_State;

enum _evas_gl_image_state
{
   EVAS_STATE_DATA,
   EVAS_STATE_TEXTURE
};

struct _evas_gl_image
{
   Evas_GL_Image_State state;
   int w, h;
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
   
   GLXContext context;
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

/***************/
/* image stuff */
/***************/
Evas_GL_Image *__evas_gl_image_new_from_file(Display *disp, char *file);
void           __evas_gl_image_free(Evas_GL_Image *im);
void           __evas_gl_image_cache_empty(Display *disp);
void           __evas_gl_image_cache_set_size(Display *disp, int size);
int            __evas_gl_image_cache_get_size(Display *disp);
void           __evas_gl_image_draw(Evas_GL_Image *im, Display *disp, Window w, int win_w, int win_h, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h);
int            __evas_gl_image_get_width(Evas_GL_Image *im);
int            __evas_gl_image_get_height(Evas_GL_Image *im);

/********/
/* text */
/********/
Evas_GL_Font  *__evas_gl_text_font_new(Display *disp, char *font, int size);
void           __evas_gl_text_font_free(Evas_GL_Font *fn);
void           __evas_gl_text_font_add_path(char *path);
void           __evas_gl_text_font_del_path(char *path);
char         **__evas_gl_text_font_list_paths(int *count);
void           __evas_gl_text_cache_empty(Display *disp);
void           __evas_gl_text_cache_set_size(Display *disp, int size);
int            __evas_gl_text_cache_get_size(Display *disp);
void           __evas_gl_text_draw(Evas_GL_Font *fn, Display *disp, Window win, int x, int y, char *text, int r, int g, int b, int a);

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
void         __evas_gl_init(Display *disp, int screen);
int          __evas_gl_capable(Display *disp);
void         __evas_gl_flush_draw(Display *disp, Window win);
void         __evas_gl_sync(Display *disp);
Visual      *__evas_gl_get_visual(Display *disp, int screen);
XVisualInfo *__evas_gl_get_visual_info(Display *disp, int screen);
Colormap     __evas_gl_get_colormap(Display *disp, int screen);
void         __evas_gl_draw_add_rect(Display *disp, Window win, int x, int y, int w, int h);
