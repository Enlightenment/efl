#include "../src/Evas.h"
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

/* defines */
#define IMGDIR "./img/"
#define FNTDIR "./fnt"

/* global variables */
Display  *display = NULL;
Window    win_base = 0, win_control = 0, win_view = 0;
Visual   *vis_control = NULL, *vis_view = NULL;
Colormap  cmap_control = 0, cmap_view  = 0;
Evas      evas_control = NULL, evas_view = NULL;
int       wait_for_events = 1;

Evas_Object o_logo, o_logo_shadow, o_software, o_hardware, o_x11, o_box1,
            o_box2, o_box3, o_brush, o_paint;
int         mouse_x, mouse_y;

/* prototypes  */
double get_time       (void);
void   setup          (void);
void   setup_controls (void);
void   setup_view     (Evas_Render_Method method);
void   animate        (double val);
void   handle_events  (void);

void   mouse_down    (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);
void   mouse_up      (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);
void   mouse_move    (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);
void   mouse_in      (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);
void   mouse_out     (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y);

/* functions */
double
get_time(void)
{
   struct timeval      timev;
   
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

void
setup(void)
{
   XSetWindowAttributes att;
   XSizeHints hnt;
   XClassHint *xch;
   
   display = XOpenDisplay(NULL);
   
   att.background_pixel = 0;
   att.colormap = cmap_control;
   att.border_pixel = 0;
   att.event_mask = 0;
   win_base = XCreateWindow(display,
			    RootWindow(display, DefaultScreen(display)),
			    0, 0, 1024, 768, 0,
			    DefaultDepth(display, DefaultScreen(display)),
			    InputOutput,
			    DefaultVisual(display, DefaultScreen(display)),
			    CWColormap | CWBorderPixel | CWEventMask | CWBackPixel,
			    &att);
   XStoreName(display, win_base, "Evas Test Program");
   hnt.flags = USSize | PSize | PMinSize | PMaxSize;
   hnt.width = 1024;
   hnt.height = 768;
   hnt.min_width = 1024;
   hnt.max_width = 1024;
   hnt.min_height = 768;
   hnt.max_height = 768;
   XSetWMNormalHints(display, win_base, &hnt);
   xch = XAllocClassHint();
   xch->res_name = "Main";
   xch->res_class = "Evas_Test";
   XSetClassHint(display, win_base, xch);
   XFree(xch);

   evas_control = evas_new_all(display, win_base, 0, 0, 128,768, 
			       RENDER_METHOD_ALPHA_SOFTWARE, 216,
			       1 * 1024 * 1024, 8 * 1024 * 1024,
			       FNTDIR);
   win_control = evas_get_window(evas_control);

   XSelectInput(display, win_control, ButtonPressMask | 
		ButtonReleaseMask | PointerMotionMask | ExposureMask);

   XMapWindow(display, win_control);
   XMapWindow(display, win_base);
   
   setup_controls();
   setup_view(RENDER_METHOD_ALPHA_SOFTWARE);
}

void
setup_controls(void)
{
   Evas_Object o;
   Evas e;
   double ascent, descent;
   
   e = evas_control;
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_control_bg.png");
   evas_move(e, o, 0, 0);
   evas_resize(e, o, 128, 768);
   evas_set_image_fill(e, o, 0, 0, 128, 768);
   evas_set_layer(e, o, -999);
   evas_show(e, o);
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_control_selection1.png");
   evas_set_image_border(e, o, 3, 3, 3, 3);
   evas_show(e, o);
   o_box1 = o;

   o = evas_add_image_from_file(e, IMGDIR"evas_test_control_selection1.png");
   evas_set_image_border(e, o, 3, 3, 3, 3);
   evas_show(e, o);
   o_box2 = o;

   o = evas_add_image_from_file(e, IMGDIR"evas_test_control_selection1.png");
   evas_set_image_border(e, o, 3, 3, 3, 3);
   evas_show(e, o);
   o_box3 = o;
   
   o = evas_add_text(e, "notepad", 16, "Software");
   evas_set_color(e, o, 0,  0,  0, 160);
   evas_move(e, o, 8, 44);
   evas_show(e, o);
   evas_text_get_max_ascent_descent(e, o, &ascent, &descent);
   evas_callback_add(e, o, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_OUT, mouse_out, NULL);
   o_software = o;
   evas_move(e, o_box1, 4, 42);
   evas_resize(e, o_box1, 114, ascent - descent + 4);
   evas_set_image_fill(e, o_box1, 0, 0, 114, ascent - descent + 4);

   o = evas_add_text(e, "notepad", 16, "Hardware");
   evas_set_color(e, o, 0,  0,  0, 160);
   evas_move(e, o, 8, 44 + 8 + (ascent - descent));
   evas_show(e, o);
   evas_text_get_max_ascent_descent(e, o, &ascent, &descent);
   evas_callback_add(e, o, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_OUT, mouse_out, NULL);
   o_hardware = o;
   evas_move(e, o_box2, 4, 42 + 8 + (ascent - descent));
   evas_resize(e, o_box2, 114, ascent - descent + 4);
   evas_set_image_fill(e, o_box2, 0, 0, 114, ascent - descent + 4);

   o = evas_add_text(e, "notepad", 16, "Basic X11");
   evas_set_color(e, o, 0,  0,  0, 160);
   evas_move(e, o, 8, 44 + 16 + ((ascent - descent) * 2));
   evas_show(e, o);
   evas_text_get_max_ascent_descent(e, o, &ascent, &descent);
   evas_callback_add(e, o, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_OUT, mouse_out, NULL);
   o_x11 = o;
   evas_move(e, o_box3, 4, 42 + 16 + ((ascent - descent) * 2));
   evas_resize(e, o_box3, 114, ascent - descent + 4);
   evas_set_image_fill(e, o_box3, 0, 0, 114, ascent - descent + 4);
   
}

void
setup_view(Evas_Render_Method method)
{
   XSetWindowAttributes att;
   Evas_Object o;
   Evas e;
   int w, h;
   double x, y;

   e = evas_new_all(display, win_base, 128, 0, 1024 - 128, 768,
			    method, 216, 1 * 1024 * 1024, 8 * 1024 * 1024,
			    FNTDIR);
/*   
   if (method == RENDER_METHOD_BASIC_HARDWARE)
      evas_set_scale_smoothness(e, 0);
   else
 */
      evas_set_scale_smoothness(e, 1);
   win_view = evas_get_window(e);   
   XSelectInput(display, win_view, ButtonPressMask | 
		ButtonReleaseMask | PointerMotionMask | ExposureMask);
   XMapWindow(display, win_view);
   if (evas_view) evas_free(evas_view);
   evas_view = e;
   
   e = evas_view;
   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_bg.png");
   evas_move(e, o, 0, 0);
   evas_resize(e, o, 1024 - 128, 768);
   evas_set_layer(e, o, -999);
   evas_show(e, o);

   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_logo.png");
   evas_get_image_size(e, o, &w, &h);
   x = (1024 - 128 - w) / 2; y  = (768 - h) / 2;
   evas_move(e, o, x, y);
   evas_set_layer(e, o, 101);
   evas_show(e, o);
   o_logo = o;
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_logo_shadow.png");
   evas_get_image_size(e, o, &w, &h);
   x += 10; y += 10;
   evas_move(e, o, x, y);
   evas_set_layer(e, o, 100);
   evas_show(e, o);
   o_logo_shadow = o;

   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_logo_image_paint.png");
   evas_get_image_size(e, o, &w, &h);
   x = (1024 - 128 - w) / 2; y  = (768 - h) / 2;
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 98);
   evas_show(e, o);
   o_paint = o;
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_logo_image_brush.png");
   evas_get_image_size(e, o, &w, &h);
   x = (1024 - 128 - w) / 2; y  = (768 - h) / 2;
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 99);
   evas_show(e, o);
   o_brush = o;
}

void
animate(double val)
{
   double x, y;
   int w, h;
   
   evas_get_image_size(evas_view, o_logo, &w, &h);
   if (val < 20)
     {
	x = ((1024 - 128) / 2) + ((((20 - val) * 256) / 20) * cos((val * 3.141592654 * 2 / 100) * 27));
	y = (768 / 2)  + ((((20 - val) * 256) / 20) * sin((val * 3.141592654 * 2 / 100) * 16));
     }
   else
     {
	x = ((1024 - 128) / 2);
	y = (768 / 2);
	wait_for_events = 1;
     }
   evas_move(evas_view, o_logo, x - (w / 2), y - (h / 2));
   evas_move(evas_view, o_logo_shadow, 
	     x - (w / 2) - ((mouse_x - x) / 16), 
	     y - (h / 2) - ((mouse_y - y) / 16));
   if (val < 8)
     {
	evas_get_image_size(evas_view, o_paint, &w, &h);
	x = (1024 - 128 - w) / 2; y  = (768 - h) / 2;
	evas_move(evas_view, o_paint, x, y);
     }
   if (val < 5)
     {
	double brush_x[500], brush_y[500];
	int i;
	const double points[15][2] =
	  {
	       { 10, -18},
	       {-18,  15},
	       { 25, -13},
	       { 15,  20},
	       { 30,   5},
	       { 34,  17},
	       { 38,  15},
	       { 38,  27},
	       { 45,  16},
	       { 48,  23},
	       { 55,  23},
	       { 56,  31},
	       { 63,  33},
	       { 100, 50},
	       { 150, 75}
	  };
	
	evas_set_color(evas_view, o_paint, 255, 255, 255, (val * 255) / 5);
	evas_get_image_size(evas_view, o_brush, &w, &h);
	x = ((1024 - 128) - w) / 2;
	y = (768 - h) / 2;
	
	for (i = 0; i < 500; i++)
	  {
	     int    j;
	     double a;
	     
	     j = (i * 13) / 500;
	     a = (double)(((double)i * 13) / 500) - (double)j;
	     a = 0.5 + (sin((a - 0.5) * 3.141592654) / 2);
	     brush_x[i] = ((1.0 - a) * points[j][0]) + (a * points[j + 1][0]);
	     brush_y[i] = ((1.0 - a) * points[j][1]) + (a * points[j + 1][1]);
	  }
	
	x += brush_x[(int)(val * 100)];
	y += brush_y[(int)(val * 100)];
	
	evas_move(evas_view, o_brush, x, y);
	if (val > 4)
	   evas_set_color(evas_view, o_brush, 255, 255, 255, (1 - (val - 4)) * 255);
     }
   else if (val < 8)
     {
	evas_get_image_size(evas_view, o_brush, &w, &h);
	x = ((1024 - 128) - w) / 2;
	y = (768 - h) / 2;
	evas_move(evas_view, o_brush, x, y);
	evas_set_color(evas_view, o_paint, 255, 255, 255, 255);
	evas_set_color(evas_view, o_brush, 255, 255, 255, ((val - 5) * 255) / 3);
     }
   else if (val < 15)
     {
	double a;
	
	evas_get_image_size(evas_view, o_brush, &w, &h);
	x = ((1024 - 128) - w) / 2;
	y = (768 - h) / 2;
	a = (7 - (val - 8)) / 7;
	a = 0.5 + (sin((a - 0.5) * 3.141592654) / 2);
	x *= a;
	y *= a;
	evas_set_color(evas_view, o_brush, 255, 255, 255, 255);
	evas_move(evas_view, o_brush, x, y);
	evas_move(evas_view, o_paint, x, y);
     }
}

void
handle_events(void)
{ 
   double val, t1, t2;
   
   val = 0.0;
   t1 = get_time();
   wait_for_events = 0;
   for (;;)
     {
	XEvent              ev;
	Evas                e = NULL;
	
	/* input events */
	do
	  {
	     int event_ok;
	     
	     event_ok = 0;
	     if (wait_for_events)
	       {
		  XNextEvent(display, &ev);
		  event_ok = 1;
	       }
	     else
	       {
		  if (XPending(display))
		    {
		       XNextEvent(display, &ev);		  
		       event_ok = 1;
		    }
	       }
	     if (event_ok)
	       {
		  if (ev.xany.window == win_control)   e = evas_control;
		  else if (ev.xany.window == win_view) e = evas_view;
		  if (e)
		    {
		       switch(ev.type)
			 {
			 case ButtonPress:
			    evas_event_button_down(e, ev.xbutton.x, ev.xbutton.y, ev.xbutton.button);
			    break;
			 case ButtonRelease:
			    evas_event_button_up(e, ev.xbutton.x, ev.xbutton.y, ev.xbutton.button);
			    break;
			 case MotionNotify:
			    if (e == evas_view)
			      {
				 mouse_x = ev.xmotion.x;
				 mouse_y = ev.xmotion.y;
			      }
			    evas_event_move(e, ev.xmotion.x, ev.xmotion.y);
			    break;
			 case Expose:
			    evas_update_rect(e, ev.xexpose.x, ev.xexpose.y, ev.xexpose.width, ev.xexpose.height);
			    break;
			 default:
			    break;
			 }
		    }
	       }
	  }
	while (XPending(display));
	/* stuff to do outside events */
	animate(val);
	/* display any changes */
	evas_render(evas_control);
	evas_render(evas_view);
	/* caluclate time taken since the last render */
	t2 = get_time();
	val += t2 - t1;
	t1 = t2;
     }
}

/* callbacks */
void
mouse_down (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   if ((_e == evas_control) && 
       ((_o == o_software) || (_o == o_hardware) || (_o == o_x11)))
     {
	double w, h;
	
	evas_set_color(_e, _o, 255,  255,  255, 255);
	if (_o == o_software)
	  {
	     evas_get_geometry(_e, o_box1, NULL, NULL, &w, &h);
	     evas_set_image_file(_e, o_box1, IMGDIR"evas_test_control_selection2.png");
	     evas_resize(_e, o_box1, w, h);
	     evas_set_image_fill(_e, o_box1, 0, 0, w, h);
	  }
	else if (_o == o_hardware)
	  {
	     evas_get_geometry(_e, o_box2, NULL, NULL, &w, &h);
	     evas_set_image_file(_e, o_box2, IMGDIR"evas_test_control_selection2.png");
	     evas_resize(_e, o_box2, w, h);
	     evas_set_image_fill(_e, o_box2, 0, 0, w, h);
	  }
	else if (_o == o_x11)
	  {
	     evas_get_geometry(_e, o_box3, NULL, NULL, &w, &h);
	     evas_set_image_file(_e, o_box3, IMGDIR"evas_test_control_selection2.png");
	     evas_resize(_e, o_box3, w, h);
	     evas_set_image_fill(_e, o_box3, 0, 0, w, h);
	  }
     }
}

void
mouse_up (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   if ((_e == evas_control) && 
       ((_o == o_software) || (_o == o_hardware) || (_o == o_x11)))
     {
	double w, h;

	evas_set_color(_e, _o, 0,  0,  0, 255);
	if (_o == o_software)
	  {
	     evas_get_geometry(_e, o_box1, NULL, NULL, &w, &h);
	     evas_set_image_file(_e, o_box1, IMGDIR"evas_test_control_selection1.png");
	     evas_resize(_e, o_box1, w, h);
	     evas_set_image_fill(_e, o_box1, 0, 0, w, h);
	     setup_view(RENDER_METHOD_ALPHA_SOFTWARE);
	  }
	else if (_o == o_hardware)
	  {
	     evas_get_geometry(_e, o_box2, NULL, NULL, &w, &h);
	     evas_set_image_file(_e, o_box2, IMGDIR"evas_test_control_selection1.png");
	     evas_resize(_e, o_box2, w, h);
	     evas_set_image_fill(_e, o_box2, 0, 0, w, h);
	     setup_view(RENDER_METHOD_3D_HARDWARE);
	  }
	else if (_o == o_x11)
	  {
	     evas_get_geometry(_e, o_box3, NULL, NULL, &w, &h);
	     evas_set_image_file(_e, o_box3, IMGDIR"evas_test_control_selection1.png");
	     evas_resize(_e, o_box3, w, h);
	     evas_set_image_fill(_e, o_box3, 0, 0, w, h);
	     setup_view(RENDER_METHOD_BASIC_HARDWARE);
	  }
     }
}

void
mouse_move (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
}

void
mouse_in (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   if ((_e == evas_control) && 
       ((_o == o_software) || (_o == o_hardware) || (_o == o_x11)))
      evas_set_color(_e, _o, 0,  0,  0, 255);
}

void
mouse_out (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   if ((_e == evas_control) && 
       ((_o == o_software) || (_o == o_hardware) || (_o == o_x11)))
      evas_set_color(_e, _o, 0,  0,  0, 160);
}

/* Mr. main */
int
main(int argc, char **argv)
{
   setup();
   handle_events();
}
