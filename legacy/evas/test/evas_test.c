#include <Evas.h>
#include "../src/evas_gl_routines.h"
#include <math.h>

double get_time(void);

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
   __evas_gl_init(d, DefaultScreen(d));
   vis = __evas_gl_get_visual(d, DefaultScreen(d));
   cmap = __evas_gl_get_colormap(d, DefaultScreen(d));

   win_w = 640; win_h = 480;
   if (argc == 3)
     {
	win_w = atoi(argv[1]);
	win_h = atoi(argv[2]);
     }
     {
	XSetWindowAttributes att;
	
	att.colormap = cmap;
	att.border_pixel = 0;
	att.event_mask = 0;
	
	win = XCreateWindow(d,
			    RootWindow(d, DefaultScreen(d)),
			    0, 0, win_w, win_h, 0,
			    (__evas_gl_get_visual_info(d, DefaultScreen(d)))->depth,
			    InputOutput,
			    vis,
			    CWColormap | CWBorderPixel | CWEventMask,
			    &att);
	XMapWindow(d, win);
     }
   if (__evas_gl_capable(d))
     {
	int a = 0;
	double t1, t2;
	Evas_GL_Image *i[4], *bg, *l, *sel, *ic, *tm;
	Evas_GL_Font *fn[4];
	Evas_GL_Graident *gr;
	
	bg = __evas_gl_image_new_from_file(d, "img/sky001.png");
	i[0] = __evas_gl_image_new_from_file(d, "img/fog1001.png");
	i[1] = __evas_gl_image_new_from_file(d, "img/fog2001.png");
	i[2] = __evas_gl_image_new_from_file(d, "img/fog3001.png");
	i[3] = __evas_gl_image_new_from_file(d, "img/fog4001.png");
	ic = __evas_gl_image_new_from_file(d, "img/mush.png");
	tm = __evas_gl_image_new_from_file(d, "img/tm.png");
	l = __evas_gl_image_new_from_file(d, "img/logo001.png");
	sel = __evas_gl_image_new_from_file(d, "img/sel.png");
	__evas_gl_image_set_borders(sel, 3, 3, 3, 3);
	__evas_gl_text_font_add_path("./fnt");
	fn[0] = __evas_gl_text_font_new(d, "cinema", 24);
	fn[1] = __evas_gl_text_font_new(d, "grunge", 16);
	fn[2] = __evas_gl_text_font_new(d, "morpheus", 24);
	fn[3] = __evas_gl_text_font_new(d, "notepad", 24);
	gr = __evas_gl_gradient_new(d);
	__evas_gl_gradient_color_add(gr, 255, 255, 255, 255, 4);
	__evas_gl_gradient_color_add(gr, 255, 255, 0, 200, 4);
	__evas_gl_gradient_color_add(gr, 255, 0, 0, 100, 4);
	__evas_gl_gradient_color_add(gr, 0, 0, 0, 0, 4);
	if (!bg)
	  {
	     printf("cannot find images!\n");
	     exit(1);
	  }
	t1 = 0;
	for(;;)
	  {
	     int j, k;
	     
	     if (a == 0)
		t1 = get_time();
	     __evas_gl_draw_add_rect(d, win, 0, 0, win_w, win_h);
	     __evas_gl_image_draw(bg, d, win, win_w, win_h,
					0, 0, __evas_gl_image_get_width(bg), __evas_gl_image_get_height(bg),
					0, 0, win_w, win_h);
	     for (k = 0; k < 4; k++)
	       {
		  int xx, yy;
		  
		  j = 3 - (k % 4);
		  xx = (((k + 1) * a) / 2) % win_w;
		  yy = 0;
		  if (k == 2)
		     __evas_gl_image_draw(l, d, win, win_w, win_h,
						0, 0, __evas_gl_image_get_width(l), __evas_gl_image_get_height(l),
						(win_w - __evas_gl_image_get_width(l)) / 2, (win_h - __evas_gl_image_get_height(l)) /2, __evas_gl_image_get_width(l), __evas_gl_image_get_height(l));
		  __evas_gl_image_draw(i[j], d, win, win_w, win_h,
					     0, 0, __evas_gl_image_get_width(i[j]), __evas_gl_image_get_height(i[j]),
					     xx, yy, win_w, win_h);
		  __evas_gl_image_draw(i[j], d, win, win_w, win_h,
					     0, 0, __evas_gl_image_get_width(i[j]), __evas_gl_image_get_height(i[j]),
					     xx - win_w, yy, win_w, win_h);
	       }
	     __evas_gl_text_draw(fn[0], d, win, win_w, win_h, 30, 50, 
				 "This is a line...", 
				 255, 255, 255, 255);
	     __evas_gl_text_draw(fn[1], d, win, win_w, win_h, 30, 100, 
				 "Of anti-aliased text drawn nicely...", 
				 255, 100, 100, 255);
	     __evas_gl_text_draw(fn[2], d, win, win_w, win_h, 30, 150, 
				 "With evas... Oh goodie isn't this fun!", 
				 100, 100, 255, 200);
	     __evas_gl_text_draw(fn[3], d, win, win_w, win_h, 30, 200, 
				 "With Lots of colors & transparency too!", 
				 50, 200, 100, 100);
	     __evas_gl_rectangle_draw(d, win, win_w, win_h,
				      80, 100, 100, 30,
				      200, 40, 100, 100);
	     __evas_gl_rectangle_draw(d, win, win_w, win_h,
				      150, 110, 25, 25,
				      200, 40, 100, 255);
	     __evas_gl_rectangle_draw(d, win, win_w, win_h,
				      170, 160, 200, 180,
				      30, 40, 200, 120);
	     __evas_gl_line_draw(d, win, win_w, win_h,
				 10, 20, 100, 160,
				 255, 255, 255, 255);
	     __evas_gl_line_draw(d, win, win_w, win_h,
				 10, 20, 100, 40,
				 0, 0, 255, 180);
	     __evas_gl_line_draw(d, win, win_w, win_h,
				 10, 20, 100, 20,
				 0, 0, 0, 255);
	     __evas_gl_line_draw(d, win, win_w, win_h,
				 10, 20, 100, 5,
				 255, 0, 0, 255);
	     __evas_gl_line_draw(d, win, win_w, win_h,
				 10, 20, 12, 2,
				 80, 160, 80, 255);
	     __evas_gl_line_draw(d, win, win_w, win_h,
				 10, 20, 10, 200,
				 255, 255, 0, 100);
	     __evas_gl_image_draw(tm, d, win, win_w, win_h,
				  
				  (__evas_gl_image_get_width(tm) / 2) - (((a / 4) + 4) / 2), 
				  (__evas_gl_image_get_height(tm) / 2) - (((a / 4) + 4) / 2), 
				  (a / 4) + 4, (a / 4) + 4,
				  
				  360, 220, 
				  200, 250);
	     __evas_gl_image_draw(tm, d, win, win_w, win_h,
				  
				  0, 0,
				  (a / 4) + 4, (a / 4) + 4,
				  
				  110, 320, 
				  100, 100);
	     __evas_gl_image_draw(sel, d, win, win_w, win_h,
				  0, 0, __evas_gl_image_get_width(sel), __evas_gl_image_get_height(sel),
				  10, 50, 
				  abs(sin((double)a / win_w) * 300) + 20, abs(cos((double)a / win_w) * 300) + 20);
	     __evas_gl_gradient_draw(gr, 
				     d, win, win_w, win_h,
				     300, 100, 200, 200, 
				     ((double)a * 360) / (double)win_w);
	     __evas_gl_image_draw(ic, d, win, win_w, win_h,
				  
				  0, 0, 
				  __evas_gl_image_get_width(ic),
				  __evas_gl_image_get_height(ic),
				  
				  a / 4, a / 8, 
				  __evas_gl_image_get_width(ic),
				  __evas_gl_image_get_height(ic));
	     __evas_gl_image_draw(ic, d, win, win_w, win_h,
				  
				  0, 0, 
				  __evas_gl_image_get_width(ic),
				  __evas_gl_image_get_height(ic),
				  
				  a / 4, a / 4, 
				  __evas_gl_image_get_width(ic),
				  __evas_gl_image_get_height(ic));
	     __evas_gl_image_draw(ic, d, win, win_w, win_h,
				  
				  0, 0, 
				  __evas_gl_image_get_width(ic),
				  __evas_gl_image_get_height(ic),
				  
				  a / 8, a / 4, 
				  __evas_gl_image_get_width(ic),
				  __evas_gl_image_get_height(ic));
	     __evas_gl_flush_draw(d, win);
	     a++;
	     if (a == (win_w * 4)) 
		{
		   double pixels;
		   double tim;
		   
		   t2 = get_time();
		   tim = t2 - t1;
		   pixels = (((((double)k + 1) * (double)win_w * (double)win_h) + ((double)__evas_gl_image_get_width(l) * (double)__evas_gl_image_get_height(l))) * (double)a);
		   printf("%3.0f pixels in %3.3f seconds\n", pixels, tim);
		   printf("..... %4.1f Mpixels/second\n", pixels / (tim * 1000000));
		   a = 0;
		}
	  }
     }
   else
     {
	printf("Your X server is unable to do GLX - no go buddy.\n");
     }
   argv = NULL;
   argc = 0;
   return 0;
}
