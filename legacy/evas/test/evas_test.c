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

Evas_Object 
o_logo, o_logo_shadow, o_logo_impress,
o_software, o_hardware, o_x11, 
o_box1, o_box2, o_box3, 
o_brush, o_paint,
o_bubble1, o_shadow1, o_bubble2, o_shadow2, o_bubble3, o_shadow3,
o_fps;
int         mouse_x, mouse_y;
int         framecount = -1;
double      last_time = 0;

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

typedef struct _textblock TextBlock;
typedef struct _imageblock ImageBlock;

struct _textblock
{
   double time1, time2, time3, time4;
   char *text;
   Evas_Object o_text, o_shadow;
};

struct _imageblock
{
   double time1, time2, time3, time4;
   double x, y;
   char *file;
   Evas_Object o_image;
};

static double texts_loop = 75;
static TextBlock texts[] =
{
     { 10.0, 12.0, 14.0, 16.0, "What are the 7 Wonders of the world?", NULL, NULL},
     { 14.0, 16.0, 18.0, 20.0, "The Temple of Artemis at Ephesus ...", NULL, NULL},
     { 18.0, 20.0, 22.0, 24.0, "The Colossus of Rhodes ...", NULL, NULL},
     { 22.0, 24.0, 26.0, 28.0, "The Hanging Gardens of Babylon ...", NULL, NULL},
     { 26.0, 28.0, 30.0, 32.0, "The Mausoleum at Halicarnassus ...", NULL, NULL},
     { 30.0, 32.0, 34.0, 36.0, "The Lighthouse at Alexandria ...", NULL, NULL},
     { 34.0, 36.0, 38.0, 40.0, "The Great Pyriamids at Giza ...", NULL, NULL},
     { 38.0, 40.0, 42.0, 44.0, "The Statue of Zeus at Olympia ...", NULL, NULL},

     { 46.0, 48.0, 50.0, 52.0, "Is there an 8th wonder?", NULL, NULL},
   
     { 52.0, 54.0, 58.0, 60.0, "Yes ...", NULL, NULL},
     { 60.0, 64.0, 70.0, 74.0, "EVAS", NULL, NULL}
};
static double images_loop = 75;
static ImageBlock images[] =
{
     { 14.0, 16.0, 18.0, 20.0, 300, 100, IMGDIR"evas_test_wonder_1.png", NULL},
     { 18.0, 20.0, 22.0, 24.0, 100, 200, IMGDIR"evas_test_wonder_2.png", NULL},
     { 22.0, 24.0, 26.0, 28.0, 500, 500, IMGDIR"evas_test_wonder_3.png", NULL},
     { 26.0, 28.0, 30.0, 32.0, 200, 350, IMGDIR"evas_test_wonder_4.png", NULL},
     { 30.0, 32.0, 34.0, 36.0, 400,   0, IMGDIR"evas_test_wonder_5.png", NULL},
     { 34.0, 36.0, 38.0, 40.0, 150, 400, IMGDIR"evas_test_wonder_6.png", NULL},
     { 38.0, 40.0, 42.0, 44.0, 600,  25, IMGDIR"evas_test_wonder_7.png", NULL}
};

/* functions */
double
get_time(void)
{
   struct timeval      timev;
   
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}

void
set_blank_pointer(Display *d, Window w)
{
   Cursor c;
   XColor cl;
   Pixmap p, m;
   GC gc;
   XGCValues gcv;
   
   p = XCreatePixmap(d, w, 1, 1, 1);
   m = XCreatePixmap(d, w, 1, 1, 1);
   gc = XCreateGC(d, m, 0, &gcv);
   XSetForeground(d, gc, 0);
   XDrawPoint(d, m, gc, 0, 0);
   XFreeGC(d, gc);
   c = XCreatePixmapCursor(d, p, m, &cl, &cl, 0, 0);
   XDefineCursor(d, w, c);
   XFreeCursor(d, c);
   XFreePixmap(d, p);
   XFreePixmap(d, m);
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
   set_blank_pointer(display, win_base);
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
		ButtonReleaseMask | PointerMotionMask | ExposureMask |
		EnterWindowMask | LeaveWindowMask);

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
   
   o = evas_add_text(e, "andover", 28, "Software");
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

   o = evas_add_text(e, "andover", 28, "Hardware");
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

   o = evas_add_text(e, "andover", 28, "Basic X11");
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
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_pointer.png");
   evas_set_pass_events(e, o, 1);
   evas_set_layer(e, o, 1000);
   evas_object_set_name(e, o, "pointer");
}

void
setup_view(Evas_Render_Method method)
{
   XSetWindowAttributes att;
   Evas_Object o;
   Evas e;
   int w, h, i;
   double x, y;

   e = evas_new_all(display, win_base, 128, 0, 1024 - 128, 768,
			    method, 216, 1 * 1024 * 1024, 8 * 1024 * 1024,
			    FNTDIR);
   if (method == RENDER_METHOD_BASIC_HARDWARE)
      evas_set_scale_smoothness(e, 0);
   else
      evas_set_scale_smoothness(e, 1);
   win_view = evas_get_window(e);   
   XSelectInput(display, win_view, ButtonPressMask | 
		ButtonReleaseMask | PointerMotionMask | ExposureMask |
		EnterWindowMask | LeaveWindowMask);
   XMapWindow(display, win_view);
   
   for (i = 0; i < (sizeof(texts) / sizeof(TextBlock)); i++) texts[i].o_text = NULL;
   for (i = 0; i < (sizeof(images) / sizeof(ImageBlock)); i++) images[i].o_image = NULL;
   if (evas_view) evas_free(evas_view);
   evas_view = e;
   
   e = evas_view;
   
   o = evas_add_text(e, "andover", 20, "FPS:");
   evas_set_color(e, o, 255,  255,  255, 100);
   evas_move(e, o, 0, 0);
   evas_show(e, o);
   o_fps = o;
   
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
   o_logo = o;

   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_logo_impress.png");
   evas_get_image_size(e, o, &w, &h);
   x = (1024 - 128 - w) / 2; y  = (768 - h) / 2;
   evas_move(e, o, x, y);
   evas_set_layer(e, o, 98);
   o_logo_impress = o;
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_logo_shadow.png");
   evas_get_image_size(e, o, &w, &h);
   x += 10; y += 10;
   evas_move(e, o, x, y);
   evas_set_layer(e, o, 100);
   o_logo_shadow = o;

   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_logo_image_paint.png");
   evas_get_image_size(e, o, &w, &h);
   x = (1024 - 128 - w) / 2; y  = (768 - h) / 2;
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 98);
   o_paint = o;
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_logo_image_brush.png");
   evas_get_image_size(e, o, &w, &h);
   x = (1024 - 128 - w) / 2; y  = (768 - h) / 2;
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 99);
   o_brush = o;

   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_bubble.png");
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 102);
   o_bubble1 = o;
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_bubble_shadow.png");
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 100);
   o_shadow1 = o;

   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_bubble.png");
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 102);
   o_bubble2 = o;
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_bubble_shadow.png");
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 100);
   o_shadow2 = o;

   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_bubble.png");
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 102);
   o_bubble3 = o;
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_view_bubble_shadow.png");
   evas_move(e, o, 0, 0);
   evas_set_layer(e, o, 100);
   o_shadow3 = o;
   
   o = evas_add_image_from_file(e, IMGDIR"evas_test_pointer.png");
   evas_set_pass_events(e, o, 1);
   evas_set_layer(e, o, 1000);
   evas_object_set_name(e, o, "pointer");
}

void
image(double val)
{
   int i;

   if (images_loop > 0)
     {
	while (val > texts_loop) val -= texts_loop;
     }
   for (i = 0; i < (sizeof(images) / sizeof(ImageBlock)); i++)
     {
	if (!images[i].o_image)
	  {
	     images[i].o_image = evas_add_image_from_file(evas_view, images[i].file);
	     evas_set_layer(evas_view, images[i].o_image, 100);
	  }
	if ((val >= images[i].time1) && (val <= images[i].time4))
	  {
	     double tw, th, dx, dy, x, y;
	     double alpha, a1;
	     
	     alpha = 255;
	     if (val <= images[i].time2)
		alpha = ((val - images[i].time1) / 
			 (images[i].time2 - images[i].time1));
	     else if (val <= images[i].time3)
		alpha = 1;
	     else
		alpha = 1.0 - ((val - images[i].time3) / 
			       (images[i].time4 - images[i].time3));
	     a1 = 1 - alpha;
	     dx = (a1 * a1 * a1 * a1) * 500 * sin(val * 2.3);
	     dy = (a1 * a1 * a1 * a1) * 600 * cos(val * 3.7);
	     evas_move(evas_view, images[i].o_image, images[i].x, images[i].y);
	     evas_set_color(evas_view, images[i].o_image, 255, 255, 255, 255 * alpha);
	     evas_show(evas_view, images[i].o_image);
	  }
	else
	  {
	     evas_hide(evas_view, images[i].o_image);
	  }
     }
}

void
text(double val)
{
   int i;

   if (texts_loop > 0)
     {
	while (val > texts_loop) val -= texts_loop;
     }
   for (i = 0; i < (sizeof(texts) / sizeof(TextBlock)); i++)
     {
	if (!texts[i].o_text)
	  {
	     texts[i].o_text = evas_add_text(evas_view, "andover", 64, texts[i].text);
	     texts[i].o_shadow = evas_add_text(evas_view, "andover", 64, texts[i].text);
	     evas_set_color(evas_view, texts[i].o_text, 255, 255, 255, 255);
	     evas_set_layer(evas_view, texts[i].o_text, 101);
	     evas_set_color(evas_view, texts[i].o_shadow, 0, 0, 0, 128);
	     evas_set_layer(evas_view, texts[i].o_shadow, 100);
	  }
	if ((val >= texts[i].time1) && (val <= texts[i].time4))
	  {
	     double tw, th, dx, dy, x, y;
	     double alpha, a1;
	     
	     alpha = 255;
	     if (val <= texts[i].time2)
		alpha = ((val - texts[i].time1) / 
			 (texts[i].time2 - texts[i].time1));
	     else if (val <= texts[i].time3)
		alpha = 1;
	     else
		alpha = 1.0 - ((val - texts[i].time3) / 
			       (texts[i].time4 - texts[i].time3));
	     a1 = 1 - alpha;
	     dx = (a1 * a1 * a1 * a1) * 500 * sin(val * 2.3);
	     dy = (a1 * a1 * a1 * a1) * 600 * cos(val * 3.7);
	     evas_get_geometry(evas_view, texts[i].o_text, NULL, NULL, &tw, &th);
	     x = ((1024 - 128 - tw) / 2) + dx;
	     y = ((768 - th) / 2) + dy;
	     evas_move(evas_view, texts[i].o_text, x, y);
	     evas_move(evas_view, texts[i].o_shadow, 
		       x - ((mouse_x - x) / 16),
		       y - ((mouse_y - y) / 16));
	     evas_set_color(evas_view, texts[i].o_text, 255, 255, 255, 255 * alpha);
	     evas_set_color(evas_view, texts[i].o_shadow, 0, 0, 0, 255 * alpha / 2);
	     evas_show(evas_view, texts[i].o_text);
	     evas_show(evas_view, texts[i].o_shadow);
	  }
	else
	  {
	     evas_hide(evas_view, texts[i].o_text);
	     evas_hide(evas_view, texts[i].o_shadow);
	  }
     }
}

void
animate(double val)
{
   double x, y, z, r;
   int w, h;
   Evas_Object o;
   
   framecount++;
   if ((val - last_time) >= 2.0)
     {
	double fps;
	char buf[256];
	
	fps = (double)framecount / (val - last_time);
	sprintf(buf, "FPS: %3.2f", fps);
	evas_set_text(evas_view, o_fps, buf);
	framecount = -1;
	last_time = val;
     }
   o = evas_object_get_named(evas_view, "pointer");
   if (o) 
     {
	evas_move(evas_view, o, mouse_x, mouse_y);
	if (evas_pointer_in(evas_view) || evas_pointer_in(evas_control))
	   evas_show(evas_view, o);
     }
   o = evas_object_get_named(evas_control, "pointer");
   if (o) 
      {
	 evas_move(evas_control, o, mouse_x + 128, mouse_y);
	if (evas_pointer_in(evas_view) || evas_pointer_in(evas_control))
	    evas_show(evas_control, o);
      }
   
   evas_get_image_size(evas_view, o_logo, &w, &h);
   if (val < 20)
     {
	x = ((1024 - 128) / 2) + ((((20 - val) * 256) / 20) * cos((val * 3.141592654 * 2 / 100) * 27));
	y = (768 / 2)  + ((((20 - val) * 256) / 20) * sin((val * 3.141592654 * 2 / 100) * 16));
     }
   else if (val < 30)
     {
	evas_set_color(evas_view, o_logo, 255, 255, 255,
		       (255 * (10 - (val - 20))) / 10);
	evas_set_color(evas_view, o_logo_shadow, 255, 255, 255,
		       (255 * (10 - (val - 20))) / 10);
	x = ((1024 - 128) / 2);
	y = (768 / 2);
/*	wait_for_events = 1;*/
     }
   else if (val < 40)
     {
	evas_hide(evas_view, o_logo);
	evas_hide(evas_view, o_logo_shadow);
     }
   else if (val < 70)
     {
	evas_hide(evas_view, o_logo);
	evas_hide(evas_view, o_logo_shadow);
	evas_set_color(evas_view, o_logo_impress, 255, 255, 255,
		       (255 * (val - 40)) / 60);
     }
   else
     {
	evas_set_color(evas_view, o_logo_impress, 255, 255, 255,
		       128);
     }
   if (val < 30)
     {
	evas_move(evas_view, o_logo, x - (w / 2), y - (h / 2));
	evas_move(evas_view, o_logo_shadow, 
		  x - (w / 2) - ((mouse_x - x) / 16), 
		  y - (h / 2) - ((mouse_y - y) / 16));
     }
   r = 48;
   z = ((2 + sin(val * 6 + (3.14159 * 0))) / 3) * 64;
   x = (1024 - 128 - 32 - r) + (cos(val * 4 + (3.14159 * 0)) * r) - (z / 2);
   y = (r + 32) + (sin(val * 6 + (3.14159 * 0)) * r) - (z / 2);
   evas_resize(evas_view, o_bubble1, z, z);
   evas_set_image_fill(evas_view, o_bubble1, 0, 0, z, z);
   evas_move(evas_view, o_bubble1, x, y);
   evas_resize(evas_view, o_shadow1, z, z);
   evas_set_image_fill(evas_view, o_shadow1, 0, 0, z, z);
   evas_move(evas_view, o_shadow1, 
	     x - ((mouse_x - x) / 16) + (z / 2), 
	     y - ((mouse_y - y) / 16) + (z / 2));
   z = ((2 + sin(val * 6 + (3.14159 * 0.66))) / 3) * 64;
   x = (1024 - 128 - 32 - r) + (cos(val * 4 + (3.14159 * 0.66)) * r) - (z / 2);
   y = (r + 32) + (sin(val * 6 + (3.14159 * 0.66)) * r) - (z / 2);
   evas_resize(evas_view, o_bubble2, z, z);
   evas_set_image_fill(evas_view, o_bubble2, 0, 0, z, z);
   evas_move(evas_view, o_bubble2, x, y);
   evas_resize(evas_view, o_shadow2, z, z);
   evas_set_image_fill(evas_view, o_shadow2, 0, 0, z, z);
   evas_move(evas_view, o_shadow2, 
	     x - ((mouse_x - x) / 16) + (z / 2), 
	     y - ((mouse_y - y) / 16) + (z / 2));
   z = ((2 + sin(val * 6 + (3.14159 * 1.33))) / 3) * 64;
   x = (1024 - 128 - 32 - r) + (cos(val * 4 + (3.14159 * 1.33)) * r) - (z / 2);
   y = (r + 32) + (sin(val * 6 + (3.14159 * 1.33)) * r) - (z / 2);
   evas_resize(evas_view, o_bubble3, z, z);
   evas_set_image_fill(evas_view, o_bubble3, 0, 0, z, z);
   evas_move(evas_view, o_bubble3, x, y);
   evas_resize(evas_view, o_shadow3, z, z);
   evas_set_image_fill(evas_view, o_shadow3, 0, 0, z, z);
   evas_move(evas_view, o_shadow3, 
	     x - ((mouse_x - x) / 16) + (z / 2), 
	     y - ((mouse_y - y) / 16) + (z / 2));
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
   if (val < 30)
     {
	evas_show(evas_view, o_logo);
	evas_show(evas_view, o_logo_shadow);
     }
   if (val > 40)
     {
	evas_show(evas_view, o_logo_impress);
     }
   evas_show(evas_view, o_paint);
   evas_show(evas_view, o_brush);
   evas_show(evas_view, o_bubble1);
   evas_show(evas_view, o_shadow1);
   evas_show(evas_view, o_bubble2);
   evas_show(evas_view, o_shadow2);
   evas_show(evas_view, o_bubble3);
   evas_show(evas_view, o_shadow3);
   
   text(val);
   image(val);
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
		  XSync(display, False);
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
			    else
			      {
				 mouse_x = ev.xmotion.x - 128;
				 mouse_y = ev.xmotion.y;
			      }
			    evas_event_move(e, ev.xmotion.x, ev.xmotion.y);
			    break;
			 case Expose:
			    evas_update_rect(e, ev.xexpose.x, ev.xexpose.y, ev.xexpose.width, ev.xexpose.height);
			    break;
			 case EnterNotify:
			    if (e == evas_view)
			      {
				 Evas_Object o;
				 
				 o = evas_object_get_named(evas_view, "pointer");
				 if (o) evas_show(evas_view, o);
			      }
			    if (e == evas_control)
			      {
				 Evas_Object o;
				 
				 o = evas_object_get_named(evas_control, "pointer");
				 if (o) evas_show(evas_control, o);
			      }
			    if (e == evas_view)
			       evas_event_enter(evas_view);
			    else if (e == evas_control)
			       evas_event_enter(evas_control);
			    break;
			 case LeaveNotify:
			      {
				 XEvent ev_next;
				 
				 if (XCheckTypedEvent(display, EnterNotify, &ev_next))
				   {
				      XPutBackEvent(display, &ev_next);
				   }
				 else
				   {
				      if (e == evas_view)
					{
					   Evas_Object o;
					   
					   o = evas_object_get_named(evas_view, "pointer");
					   if (o) evas_hide(evas_view, o);
					}
				      if (e == evas_control)
					{
					   Evas_Object o;
					   
					   o = evas_object_get_named(evas_control, "pointer");
					   if (o) evas_hide(evas_control, o);
					}
				   }
			      }
			    if (e == evas_view)
			       evas_event_leave(evas_view);
			    else if (e == evas_control)
			       evas_event_leave(evas_control);
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
