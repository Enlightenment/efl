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
	int a = -4096;
	Evas_GL_Image *i[10], *bg;
	
	bg = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/backgrounds/Propaganda/Vol6/8a.jpg");
	i[0] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-gmush.png");
	i[1] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-term.png");
	i[2] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-word.png");
	i[3] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-help.png");
	i[4] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-info.png");
	i[5] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-modem.png");
	i[6] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-money.png");
	i[7] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-mouse.png");
	i[8] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-stones.png");
	i[9] = __evas_gl_image_new_from_file(d, "/usr/share/pixmaps/gnome-balsa2.png");
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
	     for (j = 0, y = 0; y < 128; y++)
	       {
		  for (x = 0; x < 128; x++, j++)
		    {
		       int xx, yy;
		       
		       xx = (x * 64) + a;
		       yy = (y * 64) + a;
		       if ((xx > -64) && (xx < 800) && (yy > -64) && (yy < 600))
		       __evas_gl_render_to_window(i[j], d, win, 800, 600,
						  0, 0, i[j]->w, i[j]->h,
						  xx, yy, i[j]->w, i[j]->h);
		       if (j == 9) j = -1;
		    }
	       }
/*	     __evas_sync(d);*/
	     __evas_flush_draw(d, win);
	     a++;
	     if (a == 0) a = -4096;
	  }
     }
}
