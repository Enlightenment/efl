#include <Evas.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
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
   int i, a, w, h;
   Evas e;
   Evas_Object o[128], o_rect;
   int down;
   double t1, t2;
   char *imgs[8] =
     {
	"img/mush.png",
	"img/book.png",
	"img/bulb.png",
	"img/term.png",
	"img/calc.png",
	"img/worlds.png",
	"img/spider.png",
	"img/mouse.png"	   
     };
   
   win_w = 640; win_h = 480;
   if (argc == 4)
     {
	win_w = atoi(argv[1]);
	win_h = atoi(argv[2]);
     }
   
   e = evas_new();
   if (!strcmp(argv[3], "software"))
      evas_set_output_method(e, RENDER_METHOD_ALPHA_SOFTWARE);
   else
      evas_set_output_method(e, RENDER_METHOD_3D_HARDWARE);
   
   d = XOpenDisplay(NULL);
   vis = evas_get_optimal_visual(e, d);
   cmap = evas_get_optimal_colormap(e, d);

     {
	XSetWindowAttributes att;
	
	att.colormap = cmap;
	att.border_pixel = 0;
	att.event_mask = 0;
	
	win = XCreateWindow(d,
			    RootWindow(d, DefaultScreen(d)),
			    0, 0, win_w, win_h, 0,
			    imlib_get_visual_depth(d, vis),
			    InputOutput,
			    vis,
			    CWColormap | CWBorderPixel | CWEventMask,
			    &att);
	XSelectInput(d, win, ButtonPressMask | ButtonReleaseMask | 
		     PointerMotionMask | ExposureMask);
	XMapWindow(d, win);
	XSync(d, False);
     }
   evas_font_add_path(e, "./fnt");
   evas_set_output(e, d, win, vis, cmap);
   evas_set_output_size(e, win_w, win_h);
   evas_set_output_viewport(e, 0, 0, win_w, win_h);
   
   o[0] = evas_add_image_from_file(e, "img/sky001.png");
   evas_show(e, o[0]);
   o[1] = evas_add_image_from_file(e, "img/logo001.png");
   evas_get_image_size(e, o[1], &w, &h);
   w /= 2;
   h /= 2;
   evas_show(e, o[1]);
   
   for (i = 2 ; i < 100; i++)
     {
	o[i] = evas_add_image_from_file(e, "img/mush.png");
	evas_show(e, o[i]);
	evas_set_layer(e, o[i], 100);
     }
   for (i = 100; i < 128; i++)
     {
	o[i] = evas_add_text(e, "notepad", 20, imgs[i & 0x7]);
	evas_set_color(e, o[i], rand()&0xff,  rand()&0xff,  rand()&0xff, 255);
	evas_show(e, o[i]);
	evas_set_layer(e, o[i], 100);
     }
   o_rect = evas_add_rectangle(e);
   evas_show(e, o_rect);
   evas_move(e, o_rect, 100, 100);
   evas_resize(e, o_rect, 200, 100);
   evas_set_color(e, o_rect, rand()&0xff,  rand()&0xff,  rand()&0xff, 120);
   evas_set_layer(e, o_rect, 150);
   
   evas_raise(e, o[1]);
   evas_move(e, o[0], 0, 0);
   evas_resize(e, o[0], win_w, win_h);
   evas_set_image_fill(e, o[0], 0, 0, win_w, win_h);
   a = 0;
   down = 0;
   t1 = get_time();
   for (;;)
     {
	double x, y;
	XEvent              ev;

	while (XPending(d))
/*	do*/
	  {
	     XNextEvent(d, &ev);
	     switch(ev.type)
	       {
	       case ButtonPress:
		    {
		       int button, mouse_x, mouse_y;

		       down = 1;
		       button = ev.xbutton.button;
		       mouse_x = ev.xbutton.x;
		       mouse_y = ev.xbutton.y;
		       evas_move(e, o[1], mouse_x - w, mouse_y - h);
		       evas_set_layer(e, o[1], 200);
		    }
		  break;
	       case ButtonRelease:
		    {
		       int button, mouse_x, mouse_y;
		       
		       down = 0;
		       button = ev.xbutton.button;
		       mouse_x = ev.xbutton.x;
		       mouse_y = ev.xbutton.y;
		       evas_move(e, o[1], mouse_x - w, mouse_y - h);
		       evas_set_layer(e, o[1], 1);
		    }
		  break;
	       case MotionNotify:
		    {
		       int mouse_x, mouse_y;
		       
		       mouse_x = ev.xmotion.x;
		       mouse_y = ev.xmotion.y;
		       if (down)
			  evas_move(e, o[1], mouse_x - w, mouse_y - h);
		    }
		  break;
	       case Expose:
		    {
		       evas_update_rect(e, ev.xexpose.x, ev.xexpose.y, ev.xexpose.width, ev.xexpose.height);
		    }
		  break;
	       default:
		  break;
	       }
	  }
/*	while (XPending(d));*/
	for (i = 2; i < 128; i++)
	  {
	     int j, k;
	     
	     j = (i * 50) + i;
	     k = (i * -60) - (i * 2);
	     x = (win_w + (cos((double)(a + j) * 2 * 3.141592654 / 1000) * (win_h - 100))) / 2;
	     y = (win_h + (sin((double)(a + k) * 2 * 3.141592654 / 1000) * (win_h - 100))) / 2;
	     if (i < 100)
		evas_set_image_file(e, o[i], imgs[(i) & 0x7]);
	     evas_move(e, o[i], x, y);
	  }
	evas_render(e);
	a++;
	if ((a % 25) == 0)
	  {
	      t2 = get_time() - t1;
	      t1 = get_time();
	      printf("%3.3f fps\n", 25 / t2);
	  }
	if (a >= 1000) 
	   {
	      a = 0;
	   }
     }
}















































































































#if 0
#include "../src/evas_gl_routines.h"


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
	int a = 0, aa = 0;
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
		   __evas_gl_image_set_smooth_scaling(aa & 0x1);
		   aa++;
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

#endif
