#include "edje.h"

/* FIXME: need filename title and part name title */
/* FIXME: need to look a bit prettier! */
/* FIXME: edje test needs to load multiple edjes */

static int  main_start(int argc, char **argv);
static void main_stop(void);
static void main_resize(Ecore_Evas *ee);
static int  main_signal_exit(void *data, int ev_type, void *ev);
static void main_delete_request(Ecore_Evas *ee);

void bg_setup(void);
void bg_resize(double w, double h);

void test_setup(char *file, char *name);
void test_reize(double w, double h);
    
double       start_time = 0.0;
Ecore_Evas  *ecore_evas = NULL;
Evas        *evas       = NULL;

static int
main_start(int argc, char **argv)
{
   start_time = ecore_time_get();
   if (!ecore_init()) return -1;
   ecore_app_args_set(argc, (const char **)argv);
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, main_signal_exit, NULL);
   if (!ecore_evas_init()) return -1;
#if 0   
   if ((argc > 1) && (!strcmp(argv[1], "-fb")))
     ecore_evas = ecore_evas_fb_new(NULL, 0,  240, 320);
   else if ((argc > 1) && (!strcmp(argv[1], "-x")))
     ecore_evas = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
   else if ((argc > 1) && (!strcmp(argv[1], "-h")))
     {
	printf("%s -x         Run in X (default)\n"
	       "%s -fb        Run in the Framebuffer\n"
	       "%s -h         Display this help\n",
	       argv[0], argv[0], argv[0]);
	ecore_evas_shutdown();
	ecore_shutdown();
	return 0;
     }
   else
#endif     
     ecore_evas = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
   if (!ecore_evas) return -1;
   ecore_evas_callback_delete_request_set(ecore_evas, main_delete_request);
   ecore_evas_callback_resize_set(ecore_evas, main_resize);
   ecore_evas_title_set(ecore_evas, "Edje Test Program");
   ecore_evas_name_class_set(ecore_evas, "edje", "main");
   ecore_evas_show(ecore_evas);
   evas = ecore_evas_get(ecore_evas);
   evas_image_cache_set(evas, 1024 * 1024);
   evas_font_cache_set(evas, 256 * 1024);
//   evas_font_path_append(evas, FN);
   return 1;
}

static void
main_stop(void)
{
   ecore_evas_shutdown();
   ecore_shutdown();
}

static void
main_resize(Ecore_Evas *ee)
{
   double w, h;
   
   evas_output_viewport_get(evas, NULL, NULL, &w, &h);
   bg_resize(w, h);
   test_reize(w, h);
}

static int
main_signal_exit(void *data, int ev_type, void *ev)
{
   ecore_main_loop_quit();
   return 1;
}

static void
main_delete_request(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
}

static Evas_Object *o_bg = NULL;

void
bg_setup(void)
{
   Evas_Object *o;
   
   o = evas_object_rectangle_add(evas);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 240, 320);
   evas_object_layer_set(o, -999);
   evas_object_color_set(o, 240, 240, 240, 255);
   evas_object_show(o);   
   o_bg = o;
}

void
bg_resize(double w, double h)
{
   evas_object_resize(o_bg, w, h);
}

typedef struct _Demo_Edje Demo_Edje;

struct _Demo_Edje
{
   Evas_Object *edje;
   Evas_Object *left;
   Evas_Object *right;
   Evas_Object *top;
   Evas_Object *bottom;
   Evas_Object *title_text;
   int          down_top : 1;
   int          down_bottom : 1;
   int          hdir;
   int          vdir;
};

static Evas_List *edjes = NULL;

static void cb (void *data, Evas_Object *o, const char *sig, const char *src);

static void
top_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Demo_Edje *de;
   
   de = data;
   de->down_top = 1;
}

static void
top_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Demo_Edje *de;
   
   de = data;
   de->down_top = 0;
}

static void
top_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Demo_Edje *de;
   Evas_Event_Mouse_Move *ev;
   
   de = data;
   ev = event_info;
   if (de->down_top)
     {
	double x, y;
	
	evas_object_geometry_get(de->left, &x, &y, NULL, NULL);
	evas_object_move(de->left, 
			 x + ev->cur.canvas.x - ev->prev.canvas.x,
			 y + ev->cur.canvas.y - ev->prev.canvas.y);
	evas_object_geometry_get(de->right, &x, &y, NULL, NULL);
	evas_object_move(de->right, 
			 x + ev->cur.canvas.x - ev->prev.canvas.x,
			 y + ev->cur.canvas.y - ev->prev.canvas.y);
	evas_object_geometry_get(de->top, &x, &y, NULL, NULL);
	evas_object_move(de->top, 
			 x + ev->cur.canvas.x - ev->prev.canvas.x,
			 y + ev->cur.canvas.y - ev->prev.canvas.y);
	evas_object_geometry_get(de->bottom, &x, &y, NULL, NULL);
	evas_object_move(de->bottom, 
			 x + ev->cur.canvas.x - ev->prev.canvas.x,
			 y + ev->cur.canvas.y - ev->prev.canvas.y);
	evas_object_geometry_get(de->edje, &x, &y, NULL, NULL);
	evas_object_move(de->edje, 
			 x + ev->cur.canvas.x - ev->prev.canvas.x,
			 y + ev->cur.canvas.y - ev->prev.canvas.y);
     }
}

static void
bottom_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Demo_Edje *de;
   Evas_Event_Mouse_Down *ev;
   double x, y, w, h;
   int hdir, vdir;
   
   de = data;
   ev = event_info;
   de->down_bottom = 1;
   evas_object_geometry_get(de->edje, &x, &y, &w, &h);
   hdir = 1;
   vdir = 1;
   x -= 10;
   y -= 20;
   w += 20;
   h += 30;
   if ((ev->canvas.x - x) < (w / 2)) hdir = 0;
   if ((ev->canvas.y - y) < (h / 2)) vdir = 0;
   de->hdir = hdir;
   de->vdir = vdir;
}

static void
bottom_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Demo_Edje *de;
   
   de = data;
   de->down_bottom = 0;
}

static void
bottom_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Demo_Edje *de;
   Evas_Event_Mouse_Move *ev;
   
   de = data;
   ev = event_info;
   if (de->down_bottom)
     {
	double x, y, w, h;
	int hdir, vdir;
	
	evas_object_geometry_get(de->edje, &x, &y, &w, &h);
	hdir = de->hdir;
	vdir = de->vdir;
	x -= 10;
	y -= 20;
	w += 20;
	h += 30;
	if (hdir > 0)
	  {
	     w += ev->cur.canvas.x - ev->prev.canvas.x;
	     if (w < 20) w = 20;
	  }
	else
	  {
	     w -= ev->cur.canvas.x - ev->prev.canvas.x;
	     x += ev->cur.canvas.x - ev->prev.canvas.x;
	     if (w < 20)
	       {
		  x += w - 20;
		  w = 20;
	       }
	  }
	if (vdir > 0)
	  {
	     h += ev->cur.canvas.y - ev->prev.canvas.y;
	     if (h < 30) h = 30;
	  }
	else
	  {
	     h -= ev->cur.canvas.y - ev->prev.canvas.y;
	     y += ev->cur.canvas.y - ev->prev.canvas.y;
	     if (h < 30)
	       {
		  y += h - 30;
		  h = 30;
	       }
	  }
	evas_object_move(de->left, x, y + 20);
	evas_object_resize(de->left, 10, h - 30);
	evas_object_move(de->right, x + w - 10, y + 20);
	evas_object_resize(de->right, 10, h - 30);
	evas_object_move(de->top, x, y);
	evas_object_resize(de->top, w, 20);
	evas_object_move(de->bottom, x, y + (h - 10));
	evas_object_resize(de->bottom, w, 10);
	evas_object_move(de->edje, x + 10, y + 20);
	evas_object_resize(de->edje, w - 20, h - 30);
     }
}

static void
cb (void *data, Evas_Object *o, const char *sig, const char *src)
{
   printf("CALLBACK for %p %p \"%s\" \"%s\"\n", data, o, sig, src);
}

void
test_setup(char *file, char *name)
{
   Evas_Object *o;
   Demo_Edje *de;
   
   de = calloc(1, sizeof(Demo_Edje));
   edjes = evas_list_append(edjes, de);

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 40, 60, 80, 180);
   evas_object_move(o, 10, 10);
   evas_object_resize(o, 220, 20);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, top_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   top_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, top_move_cb, de);
   de->top = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 40, 60, 80, 140);
   evas_object_move(o, 10, 10 + 20 + 240);
   evas_object_resize(o, 220, 10);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, bottom_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   bottom_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, bottom_move_cb, de);
   de->bottom = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 40, 60, 80, 140);
   evas_object_move(o, 10, 10 + 20);
   evas_object_resize(o, 10, 240);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, bottom_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   bottom_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, bottom_move_cb, de);
   de->left = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 40, 60, 80, 140);
   evas_object_move(o, 10 + 10 + 200, 10 + 20);
   evas_object_resize(o, 10, 240);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, bottom_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   bottom_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, bottom_move_cb, de);
   de->right = o;
   
   o = edje_add(evas);
   edje_file_set(o, file, name);
   edje_signal_callback_add(o, "do_it", "the_source", cb, NULL);
   edje_signal_callback_add(o, "mouse,*", "logo", cb, NULL);
   evas_object_move(o, 10 + 10, 10 + 20);
   evas_object_resize(o, 200, 240);
   evas_object_show(o);
   de->edje = o;
}

void
test_reize(double w, double h)
{
//   evas_object_move(o_edje, 10, 10);   
//   evas_object_resize(o_edje, w - 20, h - 20);
}

int
main(int argc, char **argv)
{
   char *file;
   char *coll;
   
   if (main_start(argc, argv) < 1) return -1;

   edje_init();
   edje_frametime_set(1.0 / 60.0); /* 60 fps */
   
   bg_setup();

   if (argc < 2)
     {
	printf("Usage: %s file_to_show.eet [collection_to_show]\n", argv[0]);
	printf("       The default collection name to show is \"test\"\n");
	exit(-1);
     }
   file = "./data/e_logo.eet";
   coll = "test";
   file = argv[1];
   if (argc >= 3) coll = argv[2];
   /* FIXME: list collections */
   test_setup(file, coll);
   
   ecore_main_loop_begin();
   
   main_stop();
   
   return 0;
}

