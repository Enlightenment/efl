#include "../src/Evas.h"
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

Evas_Object o_logo, o_logo_shadow, o_software, o_hardware, o_x11;
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
   
   display = XOpenDisplay(NULL);
   
   evas_control = evas_new();
   evas_set_output_method(evas_control, RENDER_METHOD_ALPHA_SOFTWARE);
   evas_set_output_colors(evas_control, 216);
   vis_control = evas_get_optimal_visual(evas_control, display);
   cmap_control = evas_get_optimal_colormap(evas_control, display);
   
   att.background_pixel = 0;
   att.colormap = cmap_control;
   att.border_pixel = 0;
   att.event_mask = 0;
   win_base = XCreateWindow(display,
			    RootWindow(display, DefaultScreen(display)),
			    0, 0, 1024, 768, 0,
			    imlib_get_visual_depth(display, vis_control),
			    InputOutput,
			    vis_control,
			    CWColormap | CWBorderPixel | CWEventMask | CWBackPixel,
			    &att);
   att.background_pixmap = None;
   att.colormap = cmap_control;
   att.border_pixel = 0;
   att.event_mask = 0;
   win_control = XCreateWindow(display,
			       win_base,
			       0, 0, 128, 768, 0,
			       imlib_get_visual_depth(display, vis_control),
			       InputOutput,
			       vis_control,
			       CWColormap | CWBorderPixel | CWEventMask | CWBackPixmap,
			       &att);
   XSelectInput(display, win_control, ButtonPressMask | 
		ButtonReleaseMask | PointerMotionMask | ExposureMask);
   evas_font_add_path(evas_control, FNTDIR);
   evas_set_output(evas_control, display, win_control, vis_control, cmap_control);
   evas_set_output_size(evas_control, 128, 768);
   evas_set_output_viewport(evas_control, 0, 0, 128, 768);
   evas_set_font_cache(evas_control, 1 * 1024 * 1024);
   evas_set_image_cache(evas_control, 8 * 1024 * 1024);   

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
   
   o = evas_add_text(e, "notepad", 16, "Software");
   evas_set_color(e, o, 255,  255,  255, 160);
   evas_move(e, o, 4, 4);
   evas_show(e, o);
   evas_text_get_max_ascent_descent(e, o, &ascent, &descent);
   evas_callback_add(e, o, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_OUT, mouse_out, NULL);
   o_software = o;

   o = evas_add_text(e, "notepad", 16, "Hardware");
   evas_set_color(e, o, 255,  255,  255, 160);
   evas_move(e, o, 4, 4 + (ascent - descent));
   evas_show(e, o);
   evas_text_get_max_ascent_descent(e, o, &ascent, &descent);
   evas_callback_add(e, o, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_OUT, mouse_out, NULL);
   o_hardware = o;

   o = evas_add_text(e, "notepad", 16, "Basic X11");
   evas_set_color(e, o, 255,  255,  255, 160);
   evas_move(e, o, 4, 4 + ((ascent - descent) * 2));
   evas_show(e, o);
   evas_text_get_max_ascent_descent(e, o, &ascent, &descent);
   evas_callback_add(e, o, CALLBACK_MOUSE_DOWN, mouse_down, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_UP, mouse_up, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_IN, mouse_in, NULL);
   evas_callback_add(e, o, CALLBACK_MOUSE_OUT, mouse_out, NULL);
   o_x11 = o;
   
}

void
setup_view(Evas_Render_Method method)
{
   XSetWindowAttributes att;
   Evas_Object o;
   Evas e;
   int w, h;
   double x, y;

   if (win_view) XDestroyWindow(display, win_view);
   evas_view = evas_new();   
   evas_set_output_method(evas_view, method);
   if (method == RENDER_METHOD_BASIC_HARDWARE)
      evas_set_scale_smoothness(evas_view, 0);
   evas_set_output_colors(evas_view, 216);
   vis_view = evas_get_optimal_visual(evas_view, display);
   cmap_view = evas_get_optimal_colormap(evas_view, display);

   att.background_pixmap = None;
   att.colormap = cmap_view;
   att.border_pixel = 0;
   att.event_mask = 0;
   win_view = XCreateWindow(display,
			    win_base,
			    128, 0, 1024 - 128, 768, 0,
			    imlib_get_visual_depth(display, vis_view),
			    InputOutput,
			    vis_view,
			    CWColormap | CWBorderPixel | CWEventMask | CWBackPixmap,
			    &att);   
   XMapWindow(display, win_view);
   
   XSelectInput(display, win_view, ButtonPressMask | 
		ButtonReleaseMask | PointerMotionMask | ExposureMask);
   
   evas_font_add_path(evas_view, FNTDIR);
   evas_set_output(evas_view, display, win_view, vis_view, cmap_view);
   evas_set_output_size(evas_view, 1024 - 128, 768);
   evas_set_output_viewport(evas_view, 0, 0, 1024 - 128, 768);
   evas_set_font_cache(evas_view, 1 * 1024 * 1024);
   evas_set_image_cache(evas_view, 8 * 1024 * 1024);   
   
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
}

void
animate(double val)
{
   double x, y;
   int w, h;
   
   evas_get_image_size(evas_view, o_logo, &w, &h);
   x = ((1024 - 128) / 2) + (256 * cos((val * 3.141592654 * 2 / 100) * 27));
   y = (768 / 2)  + (256 * sin((val * 3.141592654 * 2 / 100) * 16));
   evas_move(evas_view, o_logo, x - (w / 2), y - (h / 2));
   evas_move(evas_view, o_logo_shadow, 
	     x - (w / 2) - ((mouse_x - x) / 16), 
	     y - (h / 2) - ((mouse_y - y) / 16));
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
	if (val >= 100.0) val -= 100.0;
     }
}

/* callbacks */
void
mouse_down (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   if ((_e == evas_control) && 
       ((_o == o_software) || (_o == o_hardware) || (_o == o_x11)))
      evas_set_color(_e, _o, 255,  255,  0, 255);
}

void
mouse_up (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   if ((_e == evas_control) && 
       ((_o == o_software) || (_o == o_hardware) || (_o == o_x11)))
     {
	evas_set_color(_e, _o, 255,  255,  255, 255);
	if (_o == o_software)
	   setup_view(RENDER_METHOD_ALPHA_SOFTWARE);
	else if (_o == o_hardware)
	   setup_view(RENDER_METHOD_3D_HARDWARE);
	else if (_o == o_x11)
	   setup_view(RENDER_METHOD_BASIC_HARDWARE);
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
      evas_set_color(_e, _o, 255,  255,  255, 255);
}

void
mouse_out (void *_data, Evas _e, Evas_Object _o, int _b, int _x, int _y)
{
   if ((_e == evas_control) && 
       ((_o == o_software) || (_o == o_hardware) || (_o == o_x11)))
      evas_set_color(_e, _o, 255,  255,  255, 160);
}

/* Mr. main */
int
main(int argc, char **argv)
{
   setup();
   handle_events();
}
