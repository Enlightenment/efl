#include <Evas.h>
#include "../src/evas_gl_routines.h"
#include <math.h>

double
get_time(void)
{
   struct timeval      timev;
   
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

int
main(int argc, char **argv)
{
   Display *d;
   Visual *vis;
   Colormap cmap;
   Window win;
   int win_w, win_h;
   
   d = XOpenDisplay(NULL);
   __evas_gl_init(d);
   vis = __evas_gl_get_visual(d);
   cmap = __evas_gl_get_colormap(d);
   
   win_w = 640; win_h = 480;
     {
	XSetWindowAttributes att;
	
	att.colormap = cmap;
	att.border_pixel = 0;
	att.event_mask = 0;
	
	win = XCreateWindow(d,
			    RootWindow(d, DefaultScreen(d)),
			    0, 0, win_w, win_h, 0,
			    (__evas_gl_get_visual_info(d))->depth,
			    InputOutput,
			    vis,
			    CWColormap | CWBorderPixel | CWEventMask,
			    &att);
	XMapWindow(d, win);
     }
     {
	int a = 0;
	double t1, t2;
	Evas_GL_Image *i[4], *bg, *l;
	
	bg = __evas_gl_image_new_from_file(d, "img/sky001.png");
	i[0] = __evas_gl_image_new_from_file(d, "img/fog1001.png");
	i[1] = __evas_gl_image_new_from_file(d, "img/fog2001.png");
	i[2] = __evas_gl_image_new_from_file(d, "img/fog3001.png");
	i[3] = __evas_gl_image_new_from_file(d, "img/fog4001.png");
	l = __evas_gl_image_new_from_file(d, "img/logo001.png");
	if (!bg)
	  {
	     printf("cannot find images!\n");
	     exit(1);
	  }
	for(;;)
	  {
	     int x, y, j, k;
	     
	     if (a == 0)
		t1 = get_time();
	     __evas_gl_render_to_window(bg, d, win, win_w, win_h,
					0, 0, bg->w, bg->h,
					0, 0, win_w, win_h);
	     for (k = 0; k < 4; k++)
	       {
		  int xx, yy;
		  
		  j = k % 4;
		  xx = (((k + 1) * a) / 2) % win_w;
		  yy = 0;
		  if (k == 2)
		     __evas_gl_render_to_window(l, d, win, win_w, win_h,
						0, 0, l->w, l->h,
						(win_w - l->w) / 2, (win_h - l->h) /2, l->w, l->h);
		  __evas_gl_render_to_window(i[j], d, win, win_w, win_h,
					     0, 0, i[j]->w, i[j]->h,
					     xx, yy, win_w, win_h);
		  __evas_gl_render_to_window(i[j], d, win, win_w, win_h,
					     0, 0, i[j]->w, i[j]->h,
					     xx - win_w, yy, win_w, win_h);
	       }
	     __evas_flush_draw(d, win);
	     a++;
	     if (a == (win_w * 4)) 
		{
		   double pixels;
		   double tim;
		   
		   t2 = get_time();
		   tim = t2 - t1;
		   pixels = (((((double)k + 1) * (double)win_w * (double)win_h) + ((double)l->w * (double)l->h)) * (double)a);
		   printf("%3.0f pixels in %3.3f seconds\n", pixels, tim);
		   printf("..... %4.1f Mpixels/second\n", pixels / (tim * 1000000));
		   a = 0;
		}
	  }
     }
}
