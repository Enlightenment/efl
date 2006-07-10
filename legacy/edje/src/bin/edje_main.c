#include "edje_main.h"

#ifndef WIN32
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
   Evas_Coord       minw, minh;
   Evas_Coord       maxw, maxh;
   int          hdir;
   int          vdir;
   char         down_top : 1;
   char         down_bottom : 1;
};

typedef struct _Collection Collection;

struct _Collection
{
   char         header : 1;
   char         clicked : 1;
   Evas_Coord       maxw;
   char        *file;
   char        *part;
   Evas_Object *text;
   Evas_Object *bg;
   Evas_List   *entries;
};

static int  main_start(int argc, char **argv);
static void main_stop(void);
static void main_resize(Ecore_Evas *ee);
static int  main_signal_exit(void *data, int ev_type, void *ev);
static void main_delete_request(Ecore_Evas *ee);
static void main_pre_rend(Ecore_Evas *ee);
static void main_post_rend(Ecore_Evas *ee);

void        bg_setup(void);
void        bg_resize(Evas_Coord w, Evas_Coord h);
static void bg_key_down(void *data, Evas * e, Evas_Object * obj, void *event_info);

void        test_list_move(Collection *co_head);
void        test_list(char *file);

void        test_resize(Demo_Edje *de);
void        test_setup(char *file, char *name);

static Evas_List   *edjes = NULL;
static Evas_Object *o_bg = NULL;
static Evas_Object *o_shadow = NULL;

double       start_time = 0.0;
Ecore_Evas  *ecore_evas = NULL;
Evas        *evas       = NULL;
int          startw     = 240;
int          starth     = 320;
int          mdfill     = 0;

static int
main_start(int argc, char **argv)
{
   int mode = 0;
   char buf[4096];

   e_prefix_determine(argv[0]);
   start_time = ecore_time_get();
   if (!ecore_init()) return -1;
   ecore_app_args_set(argc, (const char **)argv);
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, main_signal_exit, NULL);
   if (!ecore_evas_init()) return -1;
     {
        int i;

        for (i = 1; i < argc; i++)
          {
	     if (((!strcmp(argv[i], "-g")) ||
		  (!strcmp(argv[i], "-geometry")) ||
		  (!strcmp(argv[i], "--geometry"))) && (i < (argc - 1)))
	       {
		  int n, w, h;
		  char buf[16], buf2[16];
		  
		  n = sscanf(argv[i +1], "%10[^x]x%10s", buf, buf2);
		  if (n == 2)
		    {
		       w = atoi(buf);
		       h = atoi(buf2);
		       startw = w;
		       starth = h;
		    }
		  i++;
	       }
             else if (!strcmp(argv[i], "-gl"))
               {
		  mode = 1;
               }
             else if (!strcmp(argv[i], "-fb"))
               {
		  mode = 2;
               }
             else if (!strcmp(argv[i], "-xr"))
               {
		  mode = 3;
               }
             else if (!strcmp(argv[i], "-fill"))
               {
		  mdfill = 1;
               }
          }
     }
   if (mode == 0)
     ecore_evas = ecore_evas_software_x11_new(NULL, 0,  0, 0, startw, starth);
   if ((!ecore_evas) && (mode == 1))
     ecore_evas = ecore_evas_gl_x11_new(NULL, 0, 0, 0, startw, starth);
   if ((!ecore_evas) && (mode == 2))
     ecore_evas = ecore_evas_fb_new(NULL, 270,  startw, starth);
   if ((!ecore_evas) && (mode == 3))
     ecore_evas = ecore_evas_xrender_x11_new(NULL, 0, 0, 0, startw, starth);
   
   if (!ecore_evas) return -1;
   ecore_evas_callback_delete_request_set(ecore_evas, main_delete_request);
   ecore_evas_callback_resize_set(ecore_evas, main_resize);
   ecore_evas_callback_pre_render_set(ecore_evas, main_pre_rend);
   ecore_evas_callback_post_render_set(ecore_evas, main_post_rend);
   ecore_evas_title_set(ecore_evas, "Edje Test Program");
   ecore_evas_name_class_set(ecore_evas, "edje", "main");
   ecore_evas_show(ecore_evas);
   evas = ecore_evas_get(ecore_evas);
   evas_image_cache_set(evas, 2 * 1024 * 1024);
   evas_font_cache_set(evas, 512 * 1024);
   snprintf(buf, sizeof(buf), "%s/data/test/fonts", e_prefix_data_get());
   evas_font_path_append(evas, buf);
   return 1;
}

static void
main_stop(void)
{
   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
}

static void
main_resize(Ecore_Evas *ee)
{
   Evas_Coord w, h;
   
   evas_output_viewport_get(evas, NULL, NULL, &w, &h);
   bg_resize(w, h);
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
//   edje_thaw();
}

static void
main_post_rend(Ecore_Evas *ee)
{
//   edje_freeze();   
}

void
bg_setup(void)
{
   Evas_Object *o;
   char buf[4096];
   
   o = evas_object_image_add(evas);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, startw, starth);
   evas_object_layer_set(o, -999);
   evas_object_color_set(o, 255, 255, 255, 255);
   snprintf(buf, sizeof(buf), "%s/data/test/images/bg.png", e_prefix_data_get());
   evas_object_image_file_set(o, buf, NULL);
   evas_object_image_fill_set(o, 0, 0, 128, 128);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);   
   evas_object_focus_set(o, 1);
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, bg_key_down, NULL);
   o_bg = o;

   o = evas_object_image_add(evas);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, startw, starth);
   evas_object_layer_set(o, -999);
   evas_object_color_set(o, 255, 255, 255, 255);
   snprintf(buf, sizeof(buf), "%s/data/test/images/shadow.png", e_prefix_data_get());
   evas_object_image_file_set(o, buf, NULL);
   evas_object_image_smooth_scale_set(o, 0);
   evas_object_image_fill_set(o, 0, 0, startw, starth);
   evas_object_pass_events_set(o, 1);
   evas_object_show(o);
   o_shadow = o;
}

void
bg_resize(Evas_Coord w, Evas_Coord h)
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

   if (!strcmp(ev->keyname, "q")) ecore_main_loop_quit ();

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
	       {
	          edje_object_color_class_set(de->edje, "bg",
					      255, 255, 255, 255,
					      0, 0, 0, 0,
					      0, 0, 0, 0);
	          edje_color_class_set("fg", 
				       0, 0, 0, 255,
				       0, 0, 0, 0,
				       0, 0, 0, 0);
	          edje_text_class_set("plainjane", "Vera", 10);
	       }
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
	Evas_Coord x, y;
	
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
   Evas_Coord x, y, w, h;
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
	Evas_Coord x, y, w, h;
	Evas_Coord minw, minh;
	Evas_Coord maxw, maxh;
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
	maxw = 20 + de->maxw;
	maxh = 30 + de->maxh;
	
	if (hdir > 0)
	  {
	     w += ev->cur.canvas.x - ev->prev.canvas.x;
	     if (w < minw) w = minw;
	     else if (w > maxw) w = maxw;
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
	     if (w > maxw)
	       {
		  x -= w - maxw;
		  w = maxw;
	       }
	  }
	if (vdir > 0)
	  {
	     h += ev->cur.canvas.y - ev->prev.canvas.y;
	     if (h < minh) h = minh;
	     if (h > maxh) h = maxh;
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
	     if (h > maxh)
	       {
		  y -= h - maxh;
		  h = maxh;
	       }
	  }
	evas_object_move(de->edje, x + 10, y + 20);
	evas_object_resize(de->edje, w - 20, h - 30);
	test_resize(de);
     }
}

static void
list_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Collection *co;
   char buf[4096];
   
   co = data;
   snprintf(buf, sizeof(buf), "%s/data/test/images/item_selected.png", e_prefix_data_get());
   evas_object_image_file_set(co->bg, buf, NULL);
}

static void
list_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Collection *co;
   char buf[4096];
   
   co = data;
   snprintf(buf, sizeof(buf), "%s/data/test/images/item_normal.png", e_prefix_data_get());
   evas_object_image_file_set(co->bg, buf, NULL);
   test_setup(co->file, co->part);
   evas_object_color_set(co->bg, 255, 255, 255, 128);
   evas_object_color_set(co->text, 0, 0, 0, 128);
   evas_object_pass_events_set(co->bg, 1);
}

static void
list_head_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Collection *co;
   
   co = data;
   co->clicked = 1;
}

static void
list_head_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Collection *co;
   
   co = data;
   co->clicked = 0;
}

static void
list_head_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Collection *co;
   Evas_Coord x, y;
   Evas_Event_Mouse_Move *ev;
   
   co = data;
   if (!co->clicked) return;
   ev = event_info;
   evas_object_geometry_get(co->bg, &x, &y, NULL, NULL);
   x += ev->cur.canvas.x - ev->prev.canvas.x;
   y += ev->cur.canvas.y - ev->prev.canvas.y;
   evas_object_move(co->bg, x, y);
   test_list_move(co);
}

static void
cb (void *data, Evas_Object *o, const char *sig, const char *src)
{
   printf("CALLBACK for \"%s\" \"%s\"\n", sig, src);
   if (!strcmp(sig, "drag"))
     {
	double x, y;
	
	edje_object_part_drag_value_get(o, src, &x, &y);
	printf("Drag %3.3f %3.3f\n", x, y);
     }
}

void
test_list_move(Collection *co_head)
{
   Evas_List *l;
   Evas_Coord x, y;
   
   evas_object_geometry_get(co_head->bg, &x, &y, NULL, NULL);
   for (l = co_head->entries; l; l = l->next)
     {
	Collection *co;
	Evas_Coord w, h;
	
	co = l->data;
	evas_object_geometry_get(co->text, NULL, NULL, &w, &h);
	evas_object_resize(co->bg, co_head->maxw + 20, h + 10 + 10);
	evas_object_image_fill_set(co->bg, 0, 0, co_head->maxw + 20, h + 10 + 10);
	evas_object_move(co->bg, x, y);
	evas_object_move(co->text, x + 10, y + ((h + 10 + 10 - h) / 2));
	evas_object_show(co->bg);
	evas_object_show(co->text);
	y += h + 10 + 10 - 4 - 4;
     }
}

void
test_list(char *file)
{
   Evas_List *entries;
   Evas_Coord maxw = 128;
   Collection *co_head;
   Evas_List *collections = NULL;
   char buf[4096];

   entries = edje_file_collection_list(file);
//   printf("%s\n", edje_file_data_get(file, "My Data");
//   printf("%s\n", edje_file_data_get(file, "The Key");
     {
	Collection *co;
	Evas_Object *o;
	Evas_Coord w, h;
	char buf[1024];
	
	co = calloc(1, sizeof(Collection));
	collections = evas_list_append(collections, co);
	co->file = strdup(file);
	co->part = NULL;
	co->header = 1;
	
	o = evas_object_image_add(evas);
	evas_object_layer_set(o, 10);
	evas_object_image_border_set(o, 10, 10, 10, 10);
	snprintf(buf, sizeof(buf), "%s/data/test/images/item_title.png", e_prefix_data_get());
	evas_object_image_file_set(o, buf, NULL);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, list_head_down_cb, co);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   list_head_up_cb, co);
	evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, list_head_move_cb, co);
	co->bg = o;
	
	o = evas_object_text_add(evas);
	evas_object_layer_set(o, 10);
	evas_object_color_set(o, 0, 0, 0, 255);
	snprintf(buf, sizeof(buf), "Collections: %s", file);
	evas_object_text_text_set(o, buf);
	evas_object_text_font_set(o, "Vera", 10);
	evas_object_pass_events_set(o, 1);
	evas_object_geometry_get(o, NULL, NULL, &w, &h);
	if (w > maxw) maxw = w;	     
	co->text = o;
	co_head = co;
     }
   if (entries)
     {
	Evas_List *l;
	
	for (l = entries; l; l = l->next)
	  {
	     char *name;
	     Collection *co;
	     Evas_Object *o;
	     Evas_Coord w, h;
	     
	     name = l->data;
	     co = calloc(1, sizeof(Collection));
	     collections = evas_list_append(collections, co);
	     co->file = strdup(file);
	     co->part = strdup(name);
	     
	     o = evas_object_image_add(evas);
	     evas_object_layer_set(o, 10);
	     evas_object_image_border_set(o, 10, 10, 10, 10);
	     snprintf(buf, sizeof(buf), "%s/data/test/images/item_normal.png", e_prefix_data_get());
	     evas_object_image_file_set(o, buf, NULL);
	     evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, list_down_cb, co);
	     evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   list_up_cb, co);
	     co->bg = o;
	     
	     o = evas_object_text_add(evas);
	     evas_object_layer_set(o, 10);
	     evas_object_color_set(o, 0, 0, 0, 255);
	     printf("%s\n", co->part);
	     evas_object_text_text_set(o, co->part);
	     evas_object_text_font_set(o, "Vera", 10);
	     evas_object_pass_events_set(o, 1);
	     evas_object_geometry_get(o, NULL, NULL, &w, &h);
	     if (w > maxw) maxw = w;	     
	     co->text = o;
	  }
	edje_file_collection_list_free(entries);
	co_head->maxw = maxw;
	co_head->entries = collections;
	test_list_move(co_head);
     }
}

void
test_resize(Demo_Edje *de)
{
   Evas_Coord x, y, w, h, tw, th;
   Evas_Coord minw, minh;
   
   evas_object_geometry_get(de->edje, &x, &y, &w, &h);
   x -= 10;
   y -= 20;
   w += 20;
   h += 30;
   minw = 20 + de->minw;
   minh = 30 + de->minh;
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

static void
message_cb(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg)
{
   printf("MESSAGE for %p from script type %i id %i\n", obj, type, id);
   if (type == EDJE_MESSAGE_STRING)
     {
	Edje_Message_String *emsg;
	
	emsg = (Edje_Message_String *)msg;
	printf("STWING: \"%s\"\n", emsg->str);
     }
   printf("Send msg to script...\n");
   edje_object_message_send(obj, EDJE_MESSAGE_NONE, 12345, NULL);
}

void
test_setup(char *file, char *name)
{
   Evas_Object *o;
   Demo_Edje *de;
   char buf[4096];
   Evas_Coord tw, th, w, h;
   Evas_Coord xx, yy, ww, hh;

   xx = 10;
   yy = 10;
   ww = startw - 40;
   hh = starth - 50;
   if (mdfill)
     {
	xx = -10;
	yy = -20;
	ww = startw;
	hh = starth;
     }
   
   printf("%i %i, %i %i\n", (int)startw, (int)starth, (int)ww, (int)hh);
   
   de = calloc(1, sizeof(Demo_Edje));
   edjes = evas_list_append(edjes, de);

   o = evas_object_image_add(evas);
   snprintf(buf, sizeof(buf), "%s/data/test/images/border.png", e_prefix_data_get());
   evas_object_image_file_set(o, buf, NULL);
   evas_object_image_smooth_scale_set(o, 0);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_image_border_set(o, 26, 26, 26, 26);
   evas_object_image_fill_set(o, 0, 0, ww, hh);
   evas_object_pass_events_set(o, 1);
   evas_object_move(o, xx, yy);   
   evas_object_resize(o, ww, hh);
   de->image = o;
   evas_object_show(o);
   
   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 0);
   evas_object_move(o, xx, yy);
   evas_object_resize(o, ww, 20);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, top_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   top_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, top_move_cb, de);
   de->top = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 0);
   evas_object_move(o, xx, yy + hh - 10);
   evas_object_resize(o, ww, 10);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, bottom_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   bottom_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, bottom_move_cb, de);
   de->bottom = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 0);
   evas_object_move(o, xx, yy + 20);
   evas_object_resize(o, 10, hh - 20 - 10);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, bottom_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   bottom_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, bottom_move_cb, de);
   de->left = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 0);
   evas_object_move(o, xx + ww - 10, yy + 20);
   evas_object_resize(o, 10, hh - 20 - 10);
   evas_object_show(o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, bottom_down_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP,   bottom_up_cb, de);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, bottom_move_cb, de);
   de->right = o;

   o = evas_object_rectangle_add(evas);
   evas_object_color_set(o, 255, 255, 255, 255);
   evas_object_pass_events_set(o, 1);
   evas_object_move(o, xx + 20, yy);
   evas_object_resize(o, ww - 20 - 20, 20);
   evas_object_show(o);
   de->title_clip = o;
   
   o = evas_object_text_add(evas);
   evas_object_color_set(o, 0, 0, 0, 255);
   snprintf(buf, sizeof(buf), "%s - %s", file, name);
   evas_object_text_text_set(o, buf);
   evas_object_text_font_set(o, "Vera", 10);
   evas_object_geometry_get(o, NULL, NULL, &tw, &th);
   evas_object_move(o, xx + ((ww - tw) / 2), yy + 4 + ((16 - th) / 2));
   evas_object_pass_events_set(o, 1);
   evas_object_clip_set(o, de->title_clip);
   evas_object_show(o);
   de->title = o;
   
   o = edje_object_add(evas);
   edje_object_message_handler_set(o, message_cb, NULL);
//   edje_object_signal_callback_add(o, "do_it", "the_source", cb, NULL);
//   edje_object_signal_callback_add(o, "mouse,*", "logo", cb, NULL);
   edje_object_signal_callback_add(o, "*", "*", cb, NULL);
   edje_object_file_set(o, file, name);
   edje_object_part_drag_size_set(o, "dragable", 0.01, 0.5);
//   edje_object_part_drag_value_set(o, "dragable", 0.5, 0.5);
   edje_object_part_drag_step_set(o, "dragable", 0.1, 0.1);
   edje_object_part_drag_page_set(o, "dragable", 0.2, 0.2);
   evas_object_move(o, xx + 10, yy + 20);
   evas_object_show(o);
   w = ww;
   h = hh;
   edje_object_size_min_get(o, &(de->minw), &(de->minh));
   if (ww < de->minw) w = de->minw;
   if (hh < de->minh) h = de->minh;
   edje_object_size_max_get(o, &(de->maxw), &(de->maxh));
   if (de->maxw > 0)
     {
	if (ww > de->maxw) w = de->maxw;
     }
   if (de->maxh > 0)
     {
	if (hh > de->maxh) h = de->maxh;
     }
   evas_object_resize(o, w, h);
   de->edje = o;

   test_resize(de);
//   printf("%s\n", edje_object_data_get(o, "My Data"));
//   printf("%s\n", edje_object_data_get(o, "The Key"));
   
/* test swallowing */
/*   
   o = edje_object_add(evas);
   edje_object_file_set(o, file, name);
   edje_object_part_swallow(de->edje, "swallow", o);
   evas_object_show(o);
 */
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
	printf("  %s [-gl] [-fb] [-g WxH] [-fill] [edje_file1] [edje_file2] ...\n", argv[0]);
	printf("\n");
	printf("Example:\n");
	printf("  %s data/e_logo.edj\n", argv[0]);
	printf("  %s -fill -g 800x600 data/e_logo.edj\n", argv[0]);
	printf("  %s -gl -fill -g 800x600 data/e_logo.edj\n", argv[0]);
	exit(-1);
     }
   for (i = 1; i < argc; i++)
     {
	char *file;
	int done;
	
	done = 0;
	file = argv[i];
	if (((!strcmp(file, "-g")) ||
	    (!strcmp(file, "-geometry")) ||
	    (!strcmp(file, "--geometry"))) && (i < (argc - 1)))
	     i++;
	if (file[0] == '-') continue;
	if (!done) test_list(file);
     }
   
   ecore_main_loop_begin();
   
   main_stop();
   
   return 0;
}
#else
int main ()
{
   fprintf(stderr, "no workie on win32 yet\n");

   return 0;
}
#endif
