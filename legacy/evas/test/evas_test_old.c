#include "../src/Evas.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

#define IMGDIR DATADIR"/evas/img/"
#define FNTDIR DATADIR"/evas/fnt/"

double get_time(void);

double
get_time(void)
{
   struct timeval      timev;
   
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}


/* callbacks for logo object */

void
mouse_down (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   evas_put_data(_e, _o, "clicked", (void *)1);
   evas_put_data(_e, _o, "x", (void *)_x);
   evas_put_data(_e, _o, "y", (void *)_y);
   evas_set_layer(_e, _o, 200);
}

void
mouse_up (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   evas_remove_data(_e, _o, "clicked");
   evas_set_layer(_e, _o, 50);
}

void
mouse_move (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   if (evas_get_data(_e, _o, "clicked"))
     {
	double ox, oy;
	int x, y;
	
	evas_get_geometry(_e, _o, &ox, &oy, NULL, NULL);
	x = evas_get_data(_e, _o, "x");
	y = evas_get_data(_e, _o, "y");
	evas_put_data(_e, _o, "x", (void *)_x);
	evas_put_data(_e, _o, "y", (void *)_y);
	evas_move(_e, _o, ox + _x - x, oy + _y - y);
     }
}

void
mouse_in (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
}

void
mouse_out (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
}

/* done with callbacks */


int
main(int argc, char **argv)
{
   Imlib_Image buffer = NULL;
   Display *d;
   Visual *vis;
   Colormap cmap;
   Window win;
   int win_w, win_h;
   int i, a, w, h, m;
   Evas e;
   Evas_Object o[128], o_rect, o_line, o_grad, o_fps, o_text, o_poly;
   Evas_Gradient grad;
   int down;
   double t1, t2;
   char *save_file = NULL;
   char *imgs[8] =
     {
	IMGDIR"mush.png",
	IMGDIR"book.png",
	IMGDIR"bulb.png",
	IMGDIR"term.png",
	IMGDIR"calc.png",
	IMGDIR"worlds.png",
	IMGDIR"spider.png",
	IMGDIR"mouse.png"	   
     };
   
   win_w = 640; win_h = 480;
   e = evas_new();
   for (i = 1; i < argc; i++)
     {
	if ((!strcmp(argv[i], "-x")) && (i < (argc - 1)))
	  {
	     i++;
	     win_w = atoi(argv[i]);
	  }
	else if ((!strcmp(argv[i], "-y")) && (i < (argc - 1)))
	  {
	     i++;
	     win_h = atoi(argv[i]);
	  }
	else if ((!strcmp(argv[i], "-m")) && (i < (argc - 1)))
	  {
	     i++;
	     if (!strcmp(argv[i], "x11"))
		evas_set_output_method(e, RENDER_METHOD_BASIC_HARDWARE);
	     else if (!strcmp(argv[i], "render"))
		evas_set_output_method(e, RENDER_METHOD_ALPHA_HARDWARE);
	     else if (!strcmp(argv[i], "soft"))
		evas_set_output_method(e, RENDER_METHOD_ALPHA_SOFTWARE);
	     else if (!strcmp(argv[i], "hard"))
		evas_set_output_method(e, RENDER_METHOD_3D_HARDWARE);
	     else if (!strcmp(argv[i], "buf"))
	       {
		  DATA32 *data;
		  
		  buffer = imlib_create_image(win_w, win_h);
		  imlib_context_set_image(buffer);
		  evas_set_output_method(e, RENDER_METHOD_IMAGE);
		  evas_set_output_image(e, buffer);
	       }
	  }
	else if ((!strcmp(argv[i], "-c")) && (i < (argc - 1)))
	  {
	     i++;
	     evas_set_output_colors(e, atoi(argv[i]));
	  }
	else if ((!strcmp(argv[i], "-s")) && (i < (argc - 1)))
	  {
	     i++;
	     evas_set_scale_smoothness(e, atoi(argv[i]));
	  }
	else if ((!strcmp(argv[i], "-v")) && (i < (argc - 1)))
	  {
	     i++;
	     save_file = argv[i];
	  }
	else
	  {
	     printf("Usage:\n");
	     printf("      %s [options]\n", argv[0]);
	     printf("Where options is one or more of:\n");
	     printf("      -x width                     - width of window in pixels\n");
	     printf("      -y height                    - height of window in pixels\n");
	     printf("      -m [x11 | soft | hard | buf] - rendering mode\n");
	     printf("      -c colors                    - maximum colors allocated\n");
	     printf("      -s [1 | 0]                   - smooth scaling / rendering\n");
	     printf("      -v file.png                  - save buffer to file.png\n");
	     printf("\n");
	     printf("Examples:\n");
	     printf(" %s -x 640 -y 480 -m soft      - run test in 640x480 in software mode\n", argv[0]);
	     printf(" %s -x 800 -y 600 -m x11       - run test in 800x600 in X11 mode\n", argv[0]);
	     printf(" %s -x 1024 -y 768 -m hard     - run test in 1024x768 in OpenGL mode\n", argv[0]);
	     printf(" %s -x 640 -y 480 -m soft -s 0 - run test in 640x480 in software mode, no smooth scaling\n", argv[0]);
	     exit(0);
	  }
     }
   
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
   evas_font_add_path(e, FNTDIR);
   evas_set_output(e, d, win, vis, cmap);
   evas_set_output_size(e, win_w, win_h);
   evas_set_output_viewport(e, 0, 0, win_w, win_h);

   evas_set_font_cache(e, 512 * 1024);
   evas_set_image_cache(e, 1 * 1024 * 1024);   
   
   o[0] = evas_add_image_from_file(e, IMGDIR"sky001.png");
   evas_show(e, o[0]);
   o[1] = evas_add_image_from_file(e, IMGDIR"logo001.png");
   evas_callback_add(e, o[1], CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o[1], CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o[1], CALLBACK_MOUSE_MOVE, mouse_move, NULL);
   evas_callback_add(e, o[1], CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o[1], CALLBACK_MOUSE_OUT, mouse_out, NULL);
   w /= 2;
   h /= 2;
   evas_show(e, o[1]);
   
   o_rect = evas_add_rectangle(e);
   evas_show(e, o_rect);
   evas_move(e, o_rect, 100, 80);
   evas_resize(e, o_rect, 120, 120);
   evas_set_color(e, o_rect, rand()&0xff,  rand()&0xff,  rand()&0xff, 200);
   evas_set_layer(e, o_rect, 180);
   evas_callback_add(e, o_rect, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o_rect, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o_rect, CALLBACK_MOUSE_MOVE, mouse_move, NULL);
   evas_callback_add(e, o_rect, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o_rect, CALLBACK_MOUSE_OUT, mouse_out, NULL);

   o_rect = evas_add_rectangle(e);
   evas_show(e, o_rect);
   evas_move(e, o_rect, 200, 100);
   evas_resize(e, o_rect, 300, 200);
   evas_set_color(e, o_rect, rand()&0xff,  rand()&0xff,  rand()&0xff, 200);
   evas_set_layer(e, o_rect, 150);
   
   for (i = 2 ; i < 120; i++)
     {
	o[i] = evas_add_image_from_file(e, IMGDIR"mush.png");
	evas_show(e, o[i]);
	evas_set_layer(e, o[i], 100);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_DOWN, mouse_down, NULL);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_UP, mouse_up, NULL);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_MOVE, mouse_move, NULL);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_IN, mouse_in, NULL);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_OUT, mouse_out, NULL);
	evas_set_clip(e, o[i], o_rect);	
     }
   for (i = 120; i < 128; i++)
     {
	o[i] = evas_add_text(e, "notepad", 16, imgs[i & 0x7]);
	evas_set_color(e, o[i], rand()&0xff,  rand()&0xff,  rand()&0xff, 255);
	evas_show(e, o[i]);
	evas_set_layer(e, o[i], 100);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_DOWN, mouse_down, NULL);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_UP, mouse_up, NULL);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_MOVE, mouse_move, NULL);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_IN, mouse_in, NULL);
	evas_callback_add(e, o[i], CALLBACK_MOUSE_OUT, mouse_out, NULL);
     }

   o_poly = evas_add_poly(e);
   evas_show(e, o_poly);
   evas_add_point(e, o_poly, 100, 100);
   evas_add_point(e, o_poly, 200, 150);
   evas_add_point(e, o_poly, 300, 80);
   evas_add_point(e, o_poly, 500, 150);
   evas_add_point(e, o_poly, 200, 120);
   evas_add_point(e, o_poly, 55, 190);
   evas_add_point(e, o_poly, 130, 110);
   evas_set_color(e, o_poly, rand()&0xff,  rand()&0xff,  rand()&0xff, 180);
   evas_set_layer(e, o_poly, 150);
   evas_callback_add(e, o_poly, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o_poly, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o_poly, CALLBACK_MOUSE_MOVE, mouse_move, NULL);
   evas_callback_add(e, o_poly, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o_poly, CALLBACK_MOUSE_OUT, mouse_out, NULL);
   
   o_line = evas_add_line(e);
   evas_show(e, o_line);
   evas_set_line_xy(e, o_line, 10, 20, 100, 50);
   evas_set_color(e, o_line, rand()&0xff,  rand()&0xff,  rand()&0xff, 140);
   evas_set_layer(e, o_rect, 150);
   evas_callback_add(e, o_line, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o_line, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o_line, CALLBACK_MOUSE_MOVE, mouse_move, NULL);
   evas_callback_add(e, o_line, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o_line, CALLBACK_MOUSE_OUT, mouse_out, NULL);
   
   o_grad = evas_add_gradient_box(e);
   evas_show(e, o_grad);
   evas_move(e, o_grad, 300, 50);
   evas_resize(e, o_grad, 300, 300);
   evas_set_layer(e, o_grad, 150);
   grad = evas_gradient_new();
   evas_gradient_add_color(grad, 255, 255, 255, 255, 8);
   evas_gradient_add_color(grad, 255, 255, 0,   200, 8);
   evas_gradient_add_color(grad, 255, 0  , 0,   150, 8);
   evas_gradient_add_color(grad, 0  , 0  , 0,   0,   8);
   evas_set_gradient(e, o_grad, grad);
   evas_callback_add(e, o_grad, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o_grad, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o_grad, CALLBACK_MOUSE_MOVE, mouse_move, NULL);
   evas_callback_add(e, o_grad, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o_grad, CALLBACK_MOUSE_OUT, mouse_out, NULL);
/*   evas_set_clip(e, o_grad, o_rect);*/

   o_text = evas_add_text(e, "grunge", 14, "Click and Drag Objects...");
   evas_set_color(e, o_text, 0, 0, 0, 160);
   evas_move(e, o_text, 30, 60); 
   evas_show(e, o_text);
   evas_set_layer(e, o_text, 200);
   evas_callback_add(e, o_text, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o_text, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o_text, CALLBACK_MOUSE_MOVE, mouse_move, NULL);
   evas_callback_add(e, o_text, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o_text, CALLBACK_MOUSE_OUT, mouse_out, NULL);

   o_fps = evas_add_text(e, "morpheus", 16, "FPS...");
   evas_set_color(e, o_fps, 255, 255, 255, 140);
   evas_move(e, o_fps, win_w, win_h); 
   evas_show(e, o_fps);
   evas_set_layer(e, o_fps, 500);
   
   evas_raise(e, o[1]);
   evas_move(e, o[0], 0, 0);
   evas_resize(e, o[0], win_w, win_h);
   evas_set_image_fill(e, o[0], 0, 0, win_w, win_h);
   a = 0;
   down = 0;
   t1 = get_time();
   m = 0;
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
		       evas_event_button_down(e, mouse_x, mouse_y, button);
		    }
		  break;
	       case ButtonRelease:
		    {
		       int button, mouse_x, mouse_y;
		       
		       down = 0;
		       button = ev.xbutton.button;
		       if (button == 3)
			 {
			    evas_free(e);
			    exit(0);
			 }
		       mouse_x = ev.xbutton.x;
		       mouse_y = ev.xbutton.y;
		       evas_event_button_up(e, mouse_x, mouse_y, button);
		    }
		  break;
	       case MotionNotify:
		    {
		       int mouse_x, mouse_y;
		       
		       mouse_x = ev.xmotion.x;
		       mouse_y = ev.xmotion.y;
		       evas_event_move(e, mouse_x, mouse_y);
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
	     double ww, hh;
	     
	     if (!evas_get_data(e, o[i], "clicked"))
	       {
		  j = (i * 50) + i;
		  k = (i * -60) - (i * 2);
		  x = (win_w + (cos((double)(a + j) * 2 * 3.141592654 / 1000) * (win_h - 100))) / 2;
		  y = (win_h + (sin((double)(a + k) * 2 * 3.141592654 / 1000) * (win_h - 100))) / 2;
		  if (i < 100)
		     evas_set_image_file(e, o[i], imgs[(i) & 0x7]);
		  evas_move(e, o[i], x, y);
/*		  
		  ww =  ((1.2 + cos((double)(a + j + m) * 2 * 3.141592654 / 1000)) * 48);
		  hh = ww;
		  evas_resize(e, o[i], ww, hh);
		  evas_set_image_fill(e, o[i], 0, 0, ww, hh);
		  evas_set_color(e, o[i], 255, 255, 255, 
				 (((1.0 + cos((double)(a + j) * 2 * 3 * 3.141592654 / 1000)) / 2) * 255));
		  */
	       }
	  }
	evas_set_angle(e, o_grad, (double)a * 360 / 1000);
	evas_render(e);
	if ((save_file) && (buffer))
	  {
	     imlib_context_set_image(buffer);
	     imlib_image_set_format("png");
	     imlib_save_image(save_file);
	  }
	a++;
	m++;
	if ((a % 25) == 0)
	  {
	     char buf[64];
	     double gw, gh;
	     
	      t2 = get_time() - t1;
	      t1 = get_time();
	     sprintf(buf, "FPS: %3.1f", 25 / t2);
	     printf("%s\n", buf);
	     evas_set_text(e, o_fps, buf);
	     evas_get_geometry(e, o_fps, NULL, NULL, &gw, &gh);
	     evas_move(e, o_fps, win_w - gw, win_h - gh);
	  }
	if (a >= 1000) 
	   {
	      a = 0;
	   }
     }
}
