#include "edje.h"

static int  main_start(int argc, char **argv);
static void main_stop(void);
static void main_resize(Ecore_Evas *ee);
static int  main_signal_exit(void *data, int ev_type, void *ev);
static void main_delete_request(Ecore_Evas *ee);
static void main_pre_rend(Ecore_Evas *ee);
static void main_post_rend(Ecore_Evas *ee);

void        bg_setup(void);
void        bg_resize(double w, double h);
static void bg_key_down(void *data, Evas * e, Evas_Object * obj, void *event_info);

void        test_setup(char *file, char *name);
void        test_reize(double w, double h);
typedef struct _Demo_Edje Demo_Edje;

struct _Demo_Edje
{
   Evas_Object *edje;
   Evas_Object *left;
   Evas_Object *right;
   Evas_Object *top;
   Evas_Object *bottom;
   Evas_Object *title;
   Evas_Object *title_clip;
   Evas_Object *image;
   double       minw, minh;
   int          down_top : 1;
   int          down_bottom : 1;
   int          hdir;
   int          vdir;
};

static Evas_List   *edjes = NULL;
static Evas_Object *o_bg = NULL;
static Evas_Object *o_shadow = NULL;
    
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
#ifndef EDJE_FB_ONLY  
   ecore_evas = ecore_evas_software_x11_new(NULL, 0,  0, 0, 240, 320);
#else
   ecore_evas = ecore_evas_fb_new(NULL, 270,  240, 320);
#endif
   if (!ecore_evas) return -1;
   ecore_evas_callback_delete_request_set(ecore_evas, main_delete_request);
   ecore_evas_callback_resize_set(ecore_evas, main_resize);
   ecore_evas_callback_pre_render_set(ecore_evas, main_pre_rend);
   ecore_evas_callback_post_render_set(ecore_evas, main_post_rend);
   ecore_evas_title_set(ecore_evas, "Edje Test Program");
   ecore_evas_name_class_set(ecore_evas, "edje", "main");
   ecore_evas_show(ecore_evas);
   evas = ecore_evas_get(ecore_evas);
   evas_image_cache_set(evas, 16 * 1024 * 1024);
   evas_font_cache_set(evas, 1 * 1024 * 1024);
   evas_font_path_append(evas, DAT"data/test/fonts");
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

static void
main_pre_rend(Ecore_Evas *ee)
{
   edje_thaw();
}

static void
main_post_rend(Ecore_Evas *ee)
{
   edje_freeze();   
}

void
bg_setup(void)
{
   Evas_Object *o;
   
   o = evas_object_image_add(evas);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 240, 320);
   evas_object_layer_set(o, -999);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_image_file_set(o, DAT"data/test/images/bg.png", NULL);
   evas_object_image_fill_set(o, 0, 0, 128, 128);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);   
   evas_object_focus_set(o, 1);
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, bg_key_down, NULL);
   o_bg = o;

   o = evas_object_image_add(evas);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 240, 320);
   evas_object_layer_set(o, -999);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_image_file_set(o, DAT"data/test/images/shadow.png", NULL);
   evas_object_image_smooth_scale_set(o, 0);
   evas_object_image_fill_set(o, 0, 0, 240, 320);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);
   o_shadow = o;
}

void
bg_resize(double w, double h)
{
   evas_object_resize(o_bg, w, h);
   evas_object_resize(o_shadow, w, h);
   evas_object_image_fill_set(o_shadow, 0, 0, w, h);
}

static void
bg_key_down(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Key_Down *ev;
   
   ev = (Evas_Event_Key_Down *)event_info;
     {
	Evas_List *l;
	
	for (l = edjes; l; l = l->next)
	  {
	     Demo_Edje *de;
	     
	     de = l->data;
	     if      (!strcmp(ev->keyname, "Return"))
	       edje_object_play_set(de->edje, 1);
	     else if (!strcmp(ev->keyname, "Escape"))
	       edje_object_play_set(de->edje, 0);
	     else if (!strcmp(ev->keyname, "a"))
	       edje_object_animation_set(de->edje, 1);
	     else if (!strcmp(ev->keyname, "s"))
	       edje_object_animation_set(de->edje, 0);
	     else if (!strcmp(ev->keyname, "Left"))
	       edje_object_part_text_set(de->edje, "text", "String 1");
	     else if (!strcmp(ev->keyname, "Up"))
	       edje_object_part_text_set(de->edje, "text", "Buttox");
	     else if (!strcmp(ev->keyname, "Right"))
	       edje_object_part_text_set(de->edje, "text", "You pressed \"U\". Nice one stenchie!");
	     else if (!strcmp(ev->keyname, "F1"))
	       edje_object_color_class_set(de->edje, "bg", 
					   255, 255, 255, 255,
					   0, 0, 0, 0,
					   0, 0, 0, 0);
	     else if (!strcmp(ev->keyname, "F2"))
	       edje_object_color_class_set(de->edje, "bg", 
					   255, 200, 120, 255,
					   0, 0, 0, 0,
					   0, 0, 0, 0);
	     else if (!strcmp(ev->keyname, "F3"))
	       edje_object_color_class_set(de->edje, "bg", 
					   120, 200, 255, 200,
					   0, 0, 0, 0,
					   0, 0, 0, 0);
	     else if (!strcmp(ev->keyname, "F4"))
	       edje_object_color_class_set(de->edje, "bg", 
					   255, 200, 50, 100,
					   0, 0, 0, 0,
					   0, 0, 0, 0);
	  }
     }
}

static void cb (void *data, Evas_Object *o, const char *sig, const char *src);

static void
top_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Demo_Edje *de;
   
   de = data;
   de->down_top = 1;
   evas_object_raise(de->image);
   evas_object_raise(de->top);
   evas_object_raise(de->bottom);
   evas_object_raise(de->left);
   evas_object_raise(de->right);
   evas_object_raise(de->title_clip);
   evas_object_raise(de->title);
   evas_object_raise(de->edje);
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
	evas_object_geometry_get(de->title_clip, &x, &y, NULL, NULL);
	evas_object_move(de->title_clip, 
			 x + ev->cur.canvas.x - ev->prev.canvas.x,
			 y + ev->cur.canvas.y - ev->prev.canvas.y);
	evas_object_geometry_get(de->title, &x, &y, NULL, NULL);
	evas_object_move(de->title, 
			 x + ev->cur.canvas.x - ev->prev.canvas.x,
			 y + ev->cur.canvas.y - ev->prev.canvas.y);
	evas_object_geometry_get(de->image, &x, &y, NULL, NULL);
	evas_object_move(de->image, 
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
	double x, y, w, h, tw, th;
	double minw, minh;
	int hdir, vdir;
	
	evas_object_geometry_get(de->edje, &x, &y, &w, &h);
	hdir = de->hdir;
	vdir = de->vdir;
	x -= 10;
	y -= 20;
	w += 20;
	h += 30;
	minw = 20 + de->minw;
	minh = 30 + de->minh;
	if (hdir > 0)
	  {
	     w += ev->cur.canvas.x - ev->prev.canvas.x;
	     if (w < minw) w = minw;
	  }
	else
	  {
	     w -= ev->cur.canvas.x - ev->prev.canvas.x;
	     x += ev->cur.canvas.x - ev->prev.canvas.x;
	     if (w < minw)
	       {
		  x += w - minw;
		  w = minw;
	       }
	  }
	if (vdir > 0)
	  {
	     h += ev->cur.canvas.y - ev->prev.canvas.y;
	     if (h < minh) h = minh;
	  }
	else
	  {
	     h -= ev->cur.canvas.y - ev->prev.canvas.y;
	     y += ev->cur.canvas.y - ev->prev.canvas.y;
	     if (h < minh)
	       {
		  y += h - minh;
		  h = minh;
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
	evas_object_move(de->title_clip, x + 20, y);
	evas_object_resize(de->title_clip, w - 40, 20);
	evas_object_geometry_get(de->title, NULL, NULL, &tw, &th);
	evas_object_move(de->title, x + ((w - tw) / 2), y + 4 + ((16 - th) / 2));
	evas_object_move(de->image, x, y);
	evas_object_resize(de->image, w, h);
	evas_object_image_fill_set(de->image, 0, 0, w, h);
	evas_object_move(de->edje, x + 10, y + 20);
	evas_object_resize(de->edje, w - 20, h - 30);
     }
}

static void
cb (void *data, Evas_Object *o, const char *sig, const char *src)
{
//   printf("CALLBACK for %p %p \"%s\" \"%s\"\n", data, o, sig, src);
}

void
test_setup(char *file, char *name)
{
   Evas_Object *o;
   Demo_Edje *de;
   char buf[1024];
   double tw, th;
   
   de = calloc(1, sizeof(Demo_Edje));
   edjes = evas_list_append(edjes, de);

   o = evas_object_image_add(evas);
   evas_object_image_file_set(o, DAT"data/test/images/border.png", NULL);
   evas_object_image_smooth_scale_set(o, 0);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_image_border_set(o, 26, 26, 26, 26);
   evas_object_image_fill_set(o, 0, 0, 220, 270);
   evas_object_pass_events_set(o, 1);
   evas_object_move(o, 10, 10);   
   evas_object_resize(o, 220, 270);
   de->image = o;
   evas_object_show(o);
   
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 0);
   evas_object_move(o, 10, 10);
   evas_object_resize(o, 220, 20);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, top_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   top_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, top_move_cb, de);
   de->top = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 0);
   evas_object_move(o, 10, 10 + 20 + 240);
   evas_object_resize(o, 220, 10);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, bottom_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   bottom_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, bottom_move_cb, de);
   de->bottom = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 0);
   evas_object_move(o, 10, 10 + 20);
   evas_object_resize(o, 10, 240);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, bottom_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   bottom_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, bottom_move_cb, de);
   de->left = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 0);
   evas_object_move(o, 10 + 10 + 200, 10 + 20);
   evas_object_resize(o, 10, 240);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, bottom_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   bottom_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, bottom_move_cb, de);
   de->right = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_pass_events_set(o, 1);
   evas_object_move(o, 10 + 20, 10);
   evas_object_resize(o, 180, 20);
   evas_object_show(o);
   de->title_clip = o;
   
   o = evas_object_text_add(evas);
   evas_object_color_set(o, 0, 0, 0, 255);
   snprintf(buf, sizeof(buf), "%s - %s", file, name);
   evas_object_text_text_set(o, buf);
   evas_object_text_font_set(o, "Vera", 6);
   evas_object_geometry_get(o, NULL, NULL, &tw, &th);
   evas_object_move(o, 10 + ((220 - tw) / 2), 10 + 4 + ((16 - th) / 2));
   evas_object_pass_events_set(o, 1);
   evas_object_clip_set(o, de->title_clip);
   evas_object_show(o);
   de->title = o;
   
   o = edje_object_add(evas);
   edje_object_file_set(o, file, name);
   edje_object_signal_callback_add(o, "do_it", "the_source", cb, NULL);
   edje_object_signal_callback_add(o, "mouse,*", "logo", cb, NULL);
   evas_object_move(o, 10 + 10, 10 + 20);
   evas_object_resize(o, 200, 240);
   evas_object_show(o);
   edje_object_size_min_get(o, &(de->minw), &(de->minh));
   de->edje = o;
   
/* test swallowing */
/*   
   o = edje_object_add(evas);
   edje_object_file_set(o, file, name);
   edje_object_part_swallow(de->edje, "swallow", o);
   evas_object_show(o);
 */
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
   int i;
   
   if (main_start(argc, argv) < 1) return -1;

   edje_init();
   edje_frametime_set(1.0 / 60.0); /* 60 fps */
   
   bg_setup();

   if (argc < 2)
     {
	printf("Usage:\n");
	printf("  %s file_to_show.eet collection_to_show ...\n", argv[0]);
	printf("\n");
	printf("Example:\n");
	printf("  %s data/e_logo.eet test\n", argv[0]);
	printf("  %s data/e_logo.eet test ~/test.eet my_thing ...\n", argv[0]);
	exit(-1);
     }
   for (i = 1; i < (argc - 1); i += 2)
     {
	char *file;
	char *coll;
	
	file = argv[i];
	coll = argv[i + 1];
	test_setup(file, coll);
     }
   
   ecore_main_loop_begin();
   
   main_stop();
   
   return 0;
}

