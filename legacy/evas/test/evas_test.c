#include <Evas.h>
#include "../src/evas_gl_routines.h"
#include <math.h>

int
main(int argc, char **argv)
{
   Display *d;
   Visual *vis;
   Colormap cmap;
   Window win;
   
   d = XOpenDisplay(NULL);
   __evas_gl_init(d);
   vis = __evas_gl_get_visual(d);
   cmap = __evas_gl_get_colormap(d);
   
     {
	XSetWindowAttributes att;
	
	att.colormap = cmap;
	att.border_pixel = 0;
	att.event_mask = 0;
	
	win = XCreateWindow(d,
			    RootWindow(d, DefaultScreen(d)),
			    0, 0, 800, 600, 0,
			    (__evas_gl_get_visual_info(d))->depth,
			    InputOutput,
			    vis,
			    CWColormap | CWBorderPixel | CWEventMask,
			    &att);
	XMapWindow(d, win);
     }
     {
	int a = 0;
	Evas_GL_Image *i[4], *bg, *l;
	
	bg = __evas_gl_image_new_from_file(d, "img/sky001.png");
	i[0] = __evas_gl_image_new_from_file(d, "img/fog1001.png");
	i[1] = __evas_gl_image_new_from_file(d, "img/fog2001.png");
	i[2] = __evas_gl_image_new_from_file(d, "img/fog3001.png");
	i[3] = __evas_gl_image_new_from_file(d, "img/fog4001.png");
	l = __evas_gl_image_new_from_file(d, "img/logo001.png");
	for(;;)
	  {
	     int x, y, j, k;
	     
	     __evas_gl_render_to_window(bg, d, win, 800, 600,
					0, 0, bg->w, bg->h,
					0, 0, 800, 600);
	     for (k = 0; k < 6; k++)
	       {
		  int xx, yy;
		  
		  j = k % 4;
		  xx = (((k + 1) * a) / 2) % 800;
		  yy = 0;
		  if (k == 0)
		     __evas_gl_render_to_window(l, d, win, 800, 600,
						0, 0, l->w, l->h,
						(800 - l->w) / 2, (600 - l->h) /2, l->w, l->h);
		  __evas_gl_render_to_window(i[j], d, win, 800, 600,
					     0, 0, i[j]->w, i[j]->h,
					     xx, yy, 800, 600);
		  __evas_gl_render_to_window(i[j], d, win, 800, 600,
					     0, 0, i[j]->w, i[j]->h,
					     xx - 800, yy, 800, 600);
	       }
	     __evas_flush_draw(d, win);
	     a++;
	     if (a == (800 * 4)) a = 0;
	  }
     }
}
