#include "../src/Evas.h"
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

/* defines */
#define IMGDIR "./img/"
#define FNTDIR "./fnt"

#ifndef SPANS_COMMON
#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))
#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))
#endif

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
typedef struct _codeblock CodeBlock;

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

struct _codeblock
{
   double time1, time2, time3, time4;
   void (*function) (double v, double val, int no);
};


static void
fn_rect(double v, double val, int no)
{
   static Evas_Object o_fn_rects[8] = 
     {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
   static int init = 0;
   static int colors[] = 
     {
	255, 255, 255, 200,
	0, 0, 0, 50,
	200, 30, 30, 200,
	240, 230, 40, 220,
	30, 50, 200, 160,
	50, 220, 60, 100,
	220, 110, 30, 240,
	200, 60, 220, 80
     };
   static double coords[8 * 4];
   int i;
   
   if (no)
     {
	for (i = 0; i < 8; i++)
	  {
	     if (o_fn_rects[i])
		evas_del_object(evas_view, o_fn_rects[i]);
	     o_fn_rects[i] = NULL;
	  }
	return;
     }
   if (!init)
     {
	init = 1;
	for (i = 0; i < 8; i++)
	  {
	     coords[(i * 4) + 0] = ((1024 - 128) / 2) + ((rand() % 500) - 250) - 165;
	     coords[(i * 4) + 1] = ((768) / 2) + ((rand() % 200) - 0);
	     coords[(i * 4) + 2] = 30 + (rand() % 300);
	     coords[(i * 4) + 3] = 20 + (rand() % 180);
	  }
     }
   for (i = 0; i < 8; i++)
     {
	double alpha;
	
	if (v < 1)
	   alpha = v;
	else if (v < 2)
	   alpha = 1;
	else alpha = (3 - v);
	if (!o_fn_rects[i]) 
	   {
	      o_fn_rects[i] = evas_add_rectangle(evas_view);
	      evas_set_layer(evas_view, o_fn_rects[i], 99);
	   }
	evas_set_color(evas_view, o_fn_rects[i],
		       colors[(i * 4) + 0],
		       colors[(i * 4) + 1],
		       colors[(i * 4) + 2],
		       (double)colors[(i * 4) + 3] * alpha);
	evas_move(evas_view, o_fn_rects[i], 
		  coords[(i * 4) + 0] + (50 * cos((val * 2.7) + 3.4 + i)), 
		  coords[(i * 4) + 1] + (20 * sin((val * 3.6) + 1.2 + i))); 
	evas_resize(evas_view, o_fn_rects[i], 
		    coords[(i * 4) + 2], coords[(i * 4) + 3]); 
	evas_show(evas_view, o_fn_rects[i]);
     }   
}

static void
fn_line(double v, double val, int no)
{
   static Evas_Object o_fn_lines[64] = 
     {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
   static int init = 0;
   static int colors[64 * 4];
   static double coords[64 * 4];
   int i;
   
   if (no)
     {
	for (i = 0; i < 64; i++)
	  {
	     if (o_fn_lines[i])
		evas_del_object(evas_view, o_fn_lines[i]);
	     o_fn_lines[i] = NULL;
	  }
	return;
     }
   if (!init)
     {
	init = 1;
	for (i = 0; i < 64; i++)
	  {
	     coords[(i * 4) + 0] = ((1024 - 128) / 2) + ((rand() % 500) - 250);
	     coords[(i * 4) + 1] = ((768) / 2) + ((rand() % 200) - 0);
	     coords[(i * 4) + 2] = ((1024 - 128) / 2) + ((rand() % 500) - 250);
	     coords[(i * 4) + 3] = ((768) / 2) + ((rand() % 200) - 0);
	     colors[(i * 4) + 0] = rand() % 255;
	     colors[(i * 4) + 1] = rand() % 255;
	     colors[(i * 4) + 2] = rand() % 255;
	     colors[(i * 4) + 3] = rand() % 255;
	  }
     }
   for (i = 0; i < 64; i++)
     {
	double alpha;
	
	if (v < 1)
	   alpha = v;
	else if (v < 2)
	   alpha = 1;
	else alpha = (3 - v);
	if (!o_fn_lines[i]) 
	   {
	      o_fn_lines[i] = evas_add_line(evas_view);
	      evas_set_layer(evas_view, o_fn_lines[i], 99);
	   }
	evas_set_color(evas_view, o_fn_lines[i],
		       colors[(i * 4) + 0],
		       colors[(i * 4) + 1],
		       colors[(i * 4) + 2],
		       (double)colors[(i * 4) + 3] * alpha);
	evas_set_line_xy(evas_view, o_fn_lines[i],
			 coords[(i * 4) + 0] + (50 * cos((val * 2.7) + 3.4 + i)),
			 coords[(i * 4) + 1] + (20 * sin((val * 3.6) + 1.2 + i)),
			 coords[(i * 4) + 2] + (50 * cos((val * 4.2) + 1.7 + i)),
			 coords[(i * 4) + 3] + (20 * sin((val * 1.3) + 2.1 + i)));
	evas_show(evas_view, o_fn_lines[i]);
     }   
}

static void
fn_poly(double v, double val, int no)
{
}

static void
fn_text(double v, double val, int no)
{
}

static void
fn_grad(double v, double val, int no)
{
}

static void
fn_image(double v, double val, int no)
{
}


static double codes_loop = 170;
static CodeBlock codes[] =
{
   { 128.0, 2.0, 4.0, 6.0, fn_rect},
   { 132.0, 2.0, 4.0, 6.0, fn_line},
   { 136.0, 2.0, 4.0, 6.0, fn_poly},
   { 140.0, 2.0, 4.0, 6.0, fn_text},
   { 144.0, 2.0, 4.0, 6.0, fn_grad},
   { 148.0, 2.0, 4.0, 6.0, fn_image}
};
static double texts_loop = 170;
static TextBlock texts[] =
{
     { 10.0, 2.0, 4.0, 6.0,  "What are the 7 Wonders of the world?", NULL, NULL},
     { 14.0, 2.0, 4.0, 6.0,  "The Temple of Artemis at Ephesus ...", NULL, NULL},
     { 18.0, 2.0, 4.0, 6.0,  "The Colossus of Rhodes ...", NULL, NULL},
     { 22.0, 2.0, 4.0, 6.0,  "The Hanging Gardens of Babylon ...", NULL, NULL},
     { 26.0, 4.0, 4.0, 6.0,  "The Mausoleum at Halicarnassus ...", NULL, NULL},
     { 30.0, 2.0, 4.0, 6.0,  "The Lighthouse at Alexandria ...", NULL, NULL},
     { 34.0, 2.0, 4.0, 6.0,  "The Great Pyriamids at Giza ...", NULL, NULL},
     { 38.0, 2.0, 4.0, 6.0,  "The Statue of Zeus at Olympia ...", NULL, NULL},

     { 46.0, 2.0, 4.0, 6.0,  "Is there an 8th wonder?", NULL, NULL},
   
     { 52.0, 2.0, 4.0, 6.0,  "Yes ...", NULL, NULL},
     { 60.0, 2.0, 8.0, 12.0, "Rancid Cheese", NULL, NULL},
   
     { 76.0, 2.0, 4.0, 6.0,  "But for those who prefer to code ...", NULL, NULL},
     { 80.0, 2.0, 4.0, 6.0,  "There is ...", NULL, NULL},
     { 87.0, 2.0, 6.0, 8.0,  "E      ", NULL, NULL},
     { 87.5, 2.0, 6.0, 8.0,  "  V    ", NULL, NULL},
     { 88.0, 2.0, 6.0, 8.0,  "    A  ", NULL, NULL},
     { 88.5, 2.0, 6.0, 8.0,  "      S", NULL, NULL},
   
     { 100.0, 2.0, 4.0, 6.0,  "Evas is a Canvas ...", NULL, NULL},
     { 104.0, 2.0, 4.0, 6.0,  "That supports Anti-Aliasing ...", NULL, NULL},
     { 108.0, 2.0, 4.0, 6.0,  "Alpha Blending ...", NULL, NULL},
     { 112.0, 2.0, 4.0, 6.0,  "Intel MMX Assembly ...", NULL, NULL},
     { 116.0, 2.0, 4.0, 6.0,  "Hardware Acceleration ...", NULL, NULL},
     { 120.0, 2.0, 4.0, 6.0,  "For all objects.", NULL, NULL},
     { 124.0, 2.0, 4.0, 6.0,  "The Objects it supports are ...", NULL, NULL},
     { 128.0, 2.0, 4.0, 6.0,  "Rectangles ...", NULL, NULL},
     { 132.0, 2.0, 4.0, 6.0,  "Lines ...", NULL, NULL},
     { 136.0, 2.0, 4.0, 6.0,  "Polygons ...", NULL, NULL},
     { 140.0, 2.0, 4.0, 6.0,  "Truetype Text ...", NULL, NULL},
     { 144.0, 2.0, 4.0, 6.0,  "Gradients ...", NULL, NULL},
     { 148.0, 2.0, 4.0, 6.0,  "Images ...", NULL, NULL},
     { 152.0, 2.0, 4.0, 6.0,  "And they can all be layered ...", NULL, NULL},
     { 156.0, 2.0, 4.0, 6.0,  "Moved and Resized ...", NULL, NULL},
     { 160.0, 2.0, 4.0, 6.0,  "Faded in and out ...", NULL, NULL},
     { 164.0, 2.0, 4.0, 6.0,  "And much much more ...", NULL, NULL},
};
static double images_loop = 170;
static ImageBlock images[] =
{
     { 14.0, 2.0, 4.0, 6.0,  300, 100, IMGDIR"evas_test_wonder_1.png", NULL},
     { 18.0, 2.0, 4.0, 6.0,  100, 200, IMGDIR"evas_test_wonder_2.png", NULL},
     { 22.0, 2.0, 4.0, 6.0,  500, 500, IMGDIR"evas_test_wonder_3.png", NULL},
     { 26.0, 2.0, 4.0, 6.0,  200, 350, IMGDIR"evas_test_wonder_4.png", NULL},
     { 30.0, 2.0, 4.0, 6.0,  400,   0, IMGDIR"evas_test_wonder_5.png", NULL},
     { 34.0, 2.0, 4.0, 6.0,  150, 400, IMGDIR"evas_test_wonder_6.png", NULL},
     { 38.0, 2.0, 4.0, 6.0,  600,  25, IMGDIR"evas_test_wonder_7.png", NULL},
     { 60.0, 2.0, 8.0, 12.0, 400, 100, IMGDIR"evas_test_cheese.png", NULL},
     { 100.0, 2.0, 4.0, 6.0, 300,  50, IMGDIR"evas_test_canvas.png", NULL}
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
obscure(Evas e)
{
   Display *disp;
   Window win, root, parent, *children, www, prev_win;
   int x, y, w, h, wx, wy, ww, wh;
   unsigned int i, j, num, dummy;
   
   evas_clear_obscured_rects(e);
   win = evas_get_window(e);
   disp	= evas_get_display(e);
   evas_get_drawable_size(e, &ww, &wh);
   children = NULL;
   XQueryTree(disp, win, &root, &parent, &children, &num);
   XTranslateCoordinates(disp, win, root, 0, 0, &wx, &wy, &www);
   do 
     {
	for (i = 0; i < num; i++)
	  {
	     XWindowAttributes att;
	     
	     XGetWindowAttributes(disp, children[i], &att);
	     w = att.width;
	     h = att.height;
	     if ((att.map_state != IsUnmapped) &&
		 (att.class == InputOutput))
	       {
		  XRectangle *rect;
		  int r_num, ord;
		  int ok;
		  
		  ok = 1;
		  rect = XShapeGetRectangles(disp, children[i], ShapeBounding, 
					     &r_num, &ord);
		  if (rect) 
		     {
			if ((r_num == 1) && 
			    (rect[0].x == 0) && (rect[0].y == 0) &&
			    (rect[0].width == w) && (rect[0].height == h))
			   ok = 1;
			else
			   ok = 0;
			XFree(rect);
		     }
		  if (ok)
		    {
		       XTranslateCoordinates(disp, children[i], root, 0, 0, &x, &y, &www);
		       if (win == evas_get_window(e))
			 {
			    evas_add_obscured_rect(e, x - wx, y - wy, w, h);
			 }
		       else if (children[i] != prev_win)
			 {
			    int isbelow;
			    
			    isbelow = 0;
			    for (j = i + 1; j < num; j++)
			       if (children[j] == prev_win) isbelow = 1;
			    if (!isbelow)
			      {
				 evas_add_obscured_rect(e, x - wx, y - wy, w, h);
			      }
			 }
		    }
	       }
	  }
	prev_win = win;
	if (children) XFree(children);
	if (win == root) break;
	win = parent;
	children = NULL;
	parent = 0;
	XQueryTree(disp, win, &root, &parent, &children, &num);
     }
   while (1);
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
		EnterWindowMask | LeaveWindowMask | VisibilityChangeMask);

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
		EnterWindowMask | LeaveWindowMask | VisibilityChangeMask);
   XMapWindow(display, win_view);
   
   for (i = 0; i < (sizeof(texts) / sizeof(TextBlock)); i++) texts[i].o_text = NULL;
   for (i = 0; i < (sizeof(images) / sizeof(ImageBlock)); i++) images[i].o_image = NULL;
   for (i = 0; i < (sizeof(codes) / sizeof(CodeBlock)); i++)  codes[i].function(0, 0, 1);
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
code(double val)
{
   int i;

   if (codes_loop > 0)
     {
	while (val > codes_loop) val -= codes_loop;
     }
   for (i = 0; i < (sizeof(codes) / sizeof(CodeBlock)); i++)
     {
	if ((val >= codes[i].time1) && (val <= (codes[i].time1 + codes[i].time4)))
	  {
	     double v;
	     
	     v = 0;
	     if (val <= (codes[i].time1 + codes[i].time2))
		v = ((val - codes[i].time1) / (codes[i].time2));
	     else if (val <= (codes[i].time1 + codes[i].time3))
		v = 1 + ((val - codes[i].time1 - codes[i].time2) / (codes[i].time3 - codes[i].time2));
	     else
		v = 2.0 + ((val - codes[i].time1 - codes[i].time3) / (codes[i].time4 - codes[i].time3));
	     codes[i].function(v, val, 0);
	  }
	else
	   codes[i].function(0, val, 1);
     }
}

void
image(double val)
{
   int i;

   if (images_loop > 0)
     {
	while (val > images_loop) val -= images_loop;
     }
   for (i = 0; i < (sizeof(images) / sizeof(ImageBlock)); i++)
     {
	if (!images[i].o_image)
	  {
	     images[i].o_image = evas_add_image_from_file(evas_view, images[i].file);
	     evas_set_layer(evas_view, images[i].o_image, 97);
	  }
	if ((val >= images[i].time1) && (val <= (images[i].time1 + images[i].time4)))
	  {
	     double tw, th, dx, dy, x, y;
	     double alpha, a1;

	     alpha = 255;
	     if (val <= (images[i].time1 + images[i].time2))
		alpha = ((val - images[i].time1) / 
			 (images[i].time2));
	     else if (val <= (images[i].time1 + images[i].time3))
		alpha = 1;
	     else
		alpha = 1.0 - ((val - images[i].time1 - images[i].time3) / 
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
	if ((val >= texts[i].time1) && (val <= (texts[i].time1 + texts[i].time4)))
	  {
	     double tw, th, dx, dy, x, y;
	     double alpha, a1;
	     
	     alpha = 255;
	     if (val <= (texts[i].time1 + texts[i].time2))
		alpha = ((val - texts[i].time1) / 
			 (texts[i].time2));
	     else if (val <= (texts[i].time1 + texts[i].time3))
		alpha = 1;
	     else
		alpha = 1.0 - ((val - texts[i].time1 - texts[i].time3) / 
			       (texts[i].time4 - texts[i].time3));
	     a1 = 1 - alpha;
	     dx = (a1 * a1 * a1 * a1) * 500 * sin(val * 2.3);
	     dy = (a1 * a1 * a1 * a1) * 600 * cos(val * 3.7);
	     evas_get_geometry(evas_view, texts[i].o_text, NULL, NULL, &tw, &th);
	     x = ((1024 - 128 - tw) / 2) + dx;
	     y = ((768 - th) / 2) + dy;
	     evas_move(evas_view, texts[i].o_text, x, y);
	     evas_move(evas_view, texts[i].o_shadow, 
		       x - ((mouse_x - (x + (tw / 2))) / 16),
		       y - ((mouse_y - (y + (th / 2))) / 16));
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
	double w, h;
	
	evas_get_geometry(evas_view, o_logo_shadow, NULL, NULL, &w, &h);
	evas_move(evas_view, o_logo, x - (w / 2), y - (h / 2));
	evas_move(evas_view, o_logo_shadow, 
		  x - (w / 2) - ((mouse_x - (x + (w / 2))) / 16), 
		  y - (h / 2) - ((mouse_y - (y + (h / 2))) / 16));
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
	     x - ((mouse_x - (x + (z / 2))) / 16) + (z / 2), 
	     y - ((mouse_y - (y + (z / 2))) / 16) + (z / 2));
   z = ((2 + sin(val * 6 + (3.14159 * 0.66))) / 3) * 64;
   x = (1024 - 128 - 32 - r) + (cos(val * 4 + (3.14159 * 0.66)) * r) - (z / 2);
   y = (r + 32) + (sin(val * 6 + (3.14159 * 0.66)) * r) - (z / 2);
   evas_resize(evas_view, o_bubble2, z, z);
   evas_set_image_fill(evas_view, o_bubble2, 0, 0, z, z);
   evas_move(evas_view, o_bubble2, x, y);
   evas_resize(evas_view, o_shadow2, z, z);
   evas_set_image_fill(evas_view, o_shadow2, 0, 0, z, z);
   evas_move(evas_view, o_shadow2, 
	     x - ((mouse_x - (x + (z / 2))) / 16) + (z / 2), 
	     y - ((mouse_y - (y + (z / 2))) / 16) + (z / 2));
   z = ((2 + sin(val * 6 + (3.14159 * 1.33))) / 3) * 64;
   x = (1024 - 128 - 32 - r) + (cos(val * 4 + (3.14159 * 1.33)) * r) - (z / 2);
   y = (r + 32) + (sin(val * 6 + (3.14159 * 1.33)) * r) - (z / 2);
   evas_resize(evas_view, o_bubble3, z, z);
   evas_set_image_fill(evas_view, o_bubble3, 0, 0, z, z);
   evas_move(evas_view, o_bubble3, x, y);
   evas_resize(evas_view, o_shadow3, z, z);
   evas_set_image_fill(evas_view, o_shadow3, 0, 0, z, z);
   evas_move(evas_view, o_shadow3, 
	     x - ((mouse_x - (x + (z / 2))) / 16) + (z / 2), 
	     y - ((mouse_y - (y + (z / 2))) / 16) + (z / 2));
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
   code(val);
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
	int had_expose;
	
	had_expose = 0;	
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
			    else if (e == evas_control)
			      {
				 mouse_x = ev.xmotion.x - 128;
				 mouse_y = ev.xmotion.y;
			      }
			    evas_event_move(e, ev.xmotion.x, ev.xmotion.y);
			    break;
			 case Expose:
			    had_expose = 1;
			    evas_update_rect(e, ev.xexpose.x, ev.xexpose.y, ev.xexpose.width, ev.xexpose.height);
			    break;
			 case VisibilityNotify:
			    if (ev.xvisibility.state == VisibilityUnobscured)
			      {
				 evas_clear_obscured_rects(e);
			      }
			    else if (ev.xvisibility.state == VisibilityFullyObscured)
			      {
				 int w, h;
				 
				 evas_get_drawable_size(e, &w, &h);
				 evas_clear_obscured_rects(e);
				 evas_add_obscured_rect(e, 0, 0, w, h);
			      }
			    else if (ev.xvisibility.state == VisibilityPartiallyObscured)
			      {
				 obscure(e);
			      }
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
	if (had_expose)
	  {
/*	     obscure(evas_control);*/
	     obscure(evas_view);
	  }
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
