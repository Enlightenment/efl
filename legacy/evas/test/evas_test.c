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
	double a = 0.0;
	Evas_GL_Image *i, *bg;
	
	bg = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/backgrounds/Propaganda/Vol6/8a.jpg");
	i = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-gmush.png");
	for(;;)
	  {
	     int x, y, j;
	     
	     for (y = 0; y < 600; y += bg->h)
	       {
		  for (x = 0; x < 800; x += bg->w)
		     __evas_gl_render_to_window(bg, d, win, 800, 600,
						0, 0, bg->w, bg->h,
						x, y, bg->w, bg->h);
	       }
	     for (j = 0; j < 32; j++)
	       {
		  x = 400 + (cos((a + ((double)j / 10))) * (1 * a));
		  y = 300 + (sin((a + ((double)j / 10))) * (1 * a));
		  __evas_gl_render_to_window(i, d, win, 800, 600,
					     0, 0, i->w, i->h,
					     x, y, i->w, i->h);
	       }
/*	     __evas_sync(d);*/
	     __evas_flush_draw(d, win);
	     a += 0.1;
	  }
     }
}
