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
#include <X11/keysym.h>

#include "Evas.h"

typedef struct _evas_gl_image     Evas_GL_Image;
typedef enum _evas_gl_image_state Evas_GL_Image_State;

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

void __evas_gl_copy_image_rect_to_texture(Evas_GL_Image *im, int x, int y,
				     int w, int h, int tw, int th,
					  GLuint texture);
void __evas_gl_move_state_data_to_texture(Evas_GL_Image *im);
void __evas_calc_tex_and_poly(Evas_GL_Image *im, int x, double *x1, double *x2,
			      int *tx, int *txx, double *dtx, double *dtxx, int tw, int w, int edge);
void __evas_gl_set_conect_for_dest(Evas_GL_Image *im, Display *disp, Window w,
				   int win_w, int win_h);
void __evas_gl_render_to_window(Evas_GL_Image *im,
				Display *disp, Window w, int win_w, int win_h,
				int src_x, int src_y, int src_w, int src_h,
				int dst_x, int dst_y, int dst_w, int dst_h);
Evas_GL_Image * __evas_gl_create_image(void);
int __evas_gl_capable(Display *disp);
Visual * __evas_gl_get_visual(Display *disp);
XVisualInfo *__evas_gl_get_visual_info(Display *disp);
Colormap __evas_gl_get_colormap(Display *disp);
void __evas_gl_init(Display *disp);
Evas_GL_Image *__evas_gl_image_create_from_file(Display *disp, char *file);
Evas_GL_Image * __evas_gl_image_new_from_file(Display *disp, char *file);
void __evas_gl_image_free(Evas_GL_Image *im);
void __evas_gl_image_destroy(Evas_GL_Image *im);
void __evas_gl_image_cache_flush(Display *disp);
void __evas_gl_image_cache_empty(Display *disp);
void __evas_gl_image_cache_set_size(Display *disp, int size);
int __evas_gl_image_cache_get_size(Display *disp);
void __evas_gl_sync(Display *disp);
void __evas_gl_flush_draw(Display *disp, Window win);
void __evas_gl_draw_rectangle(Display *disp, Window win, int x, int y, int w, int h,
			   int r, int g, int b, int a);
