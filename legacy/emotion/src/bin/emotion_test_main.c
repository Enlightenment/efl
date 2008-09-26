#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(HAVE_ECORE_X_H) || defined(HAVE_ECORE_FB_H)

#include <Evas.h>
#include <Ecore.h>
#ifndef FB_ONLY
#include <Ecore_X.h>
#else
#include <Ecore_Fb.h>
#endif
#include <Ecore_Evas.h>
#include <Edje.h>

#include "Emotion.h"

typedef struct _Frame_Data Frame_Data;

struct _Frame_Data
{
   unsigned char moving : 1;
   unsigned char resizing : 1;
   int        button;
   Evas_Coord x, y;
};

static int  main_start(int argc, char **argv);
static void main_stop(void);
static void main_resize(Ecore_Evas *ee);
static int  main_signal_exit(void *data, int ev_type, void *ev);
static void main_delete_request(Ecore_Evas *ee);

static void bg_setup(void);
static void bg_resize(Evas_Coord w, Evas_Coord h);
static void bg_key_down(void *data, Evas * e, Evas_Object * obj, void *event_info);

static Evas_Object *o_bg = NULL;

static double       start_time = 0.0;
static Ecore_Evas  *ecore_evas = NULL;
static Evas        *evas       = NULL;
static int          startw     = 800;
static int          starth     = 600;

static Evas_List   *video_objs = NULL;
static Emotion_Vis  vis        = EMOTION_VIS_NONE;

static int
main_start(int argc, char **argv)
{
   int mode = 0;

   start_time = ecore_time_get();
   if (!ecore_init()) return -1;
   ecore_app_args_set(argc, (const char **)argv);
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, main_signal_exit, NULL);
   if (!ecore_evas_init()) return -1;
#ifndef FB_ONLY
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
	     else if ((!strcmp(argv[i], "-vis")) && (i < (argc - 1)))
	       {
		  vis = atoi(argv[i + 1]);
		  i++;
	       }
          }
     }
#if HAVE_ECORE_EVAS_X
   if (mode == 0)
     ecore_evas = ecore_evas_software_x11_new(NULL, 0,  0, 0, startw, starth);
#endif
#if HAVE_ECORE_EVAS_X11_GL
   if (mode == 1)
     ecore_evas = ecore_evas_gl_x11_new(NULL, 0, 0, 0, startw, starth);
#endif
#if HAVE_ECORE_EVAS_FB
   if (mode == 2)
     ecore_evas = ecore_evas_fb_new(NULL, 0, startw, starth);
#endif
   if (mode == 3)
     ecore_evas = ecore_evas_xrender_x11_new(NULL, 0, 0, 0, startw, starth);

#else
   startw = 240;
   starth = 320;
   ecore_evas = ecore_evas_fb_new(NULL, 270,  startw, starth);
#endif
   if (!ecore_evas) return -1;
   ecore_evas_callback_delete_request_set(ecore_evas, main_delete_request);
   ecore_evas_callback_resize_set(ecore_evas, main_resize);
   ecore_evas_title_set(ecore_evas, "Evas Media Test Program");
   ecore_evas_name_class_set(ecore_evas, "evas_media_test", "main");
   ecore_evas_show(ecore_evas);
   evas = ecore_evas_get(ecore_evas);
   evas_image_cache_set(evas, 8 * 1024 * 1024);
   evas_font_cache_set(evas, 1 * 1024 * 1024);
   evas_font_path_append(evas, PACKAGE_DATA_DIR"/data/fonts");

   edje_init();
   edje_frametime_set(1.0 / 30.0);
   return 1;
}

static void
main_stop(void)
{
   main_signal_exit(NULL, 0, NULL);
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
   while (video_objs)
     {
	printf("del obj!\n");
	evas_object_del(video_objs->data);
	video_objs = evas_list_remove_list(video_objs, video_objs);
	printf("done\n");
     }
   return 1;
}

static void
main_delete_request(Ecore_Evas *ee)
{
   ecore_main_loop_quit();
}

void
bg_setup(void)
{
   Evas_Object *o;

   o = edje_object_add(evas);
   edje_object_file_set(o, PACKAGE_DATA_DIR"/data/theme.edj", "background");
   evas_object_move(o, 0, 0);
   evas_object_resize(o, startw, starth);
   evas_object_layer_set(o, -999);
   evas_object_show(o);
   evas_object_focus_set(o, 1);
   evas_object_event_callback_add(o, EVAS_CALLBACK_KEY_DOWN, bg_key_down, NULL);
   o_bg = o;
}

void
bg_resize(Evas_Coord w, Evas_Coord h)
{
   evas_object_resize(o_bg, w, h);
}

static void
broadcast_event(Emotion_Event ev)
{
   Evas_List *l;

   for (l = video_objs; l; l = l->next)
     {
	Evas_Object *obj;

	obj = l->data;
	emotion_object_event_simple_send(obj, ev);
     }
}

static void
bg_key_down(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
   Evas_Event_Key_Down *ev;

   ev = (Evas_Event_Key_Down *)event_info;
   if      (!strcmp(ev->keyname, "Escape"))
     ecore_main_loop_quit();
   else if (!strcmp(ev->keyname, "Up"))
     broadcast_event(EMOTION_EVENT_UP);
   else if (!strcmp(ev->keyname, "Down"))
     broadcast_event(EMOTION_EVENT_DOWN);
   else if (!strcmp(ev->keyname, "Left"))
     broadcast_event(EMOTION_EVENT_LEFT);
   else if (!strcmp(ev->keyname, "Right"))
     broadcast_event(EMOTION_EVENT_RIGHT);
   else if (!strcmp(ev->keyname, "Return"))
     broadcast_event(EMOTION_EVENT_SELECT);
   else if (!strcmp(ev->keyname, "m"))
     broadcast_event(EMOTION_EVENT_MENU1);
   else if (!strcmp(ev->keyname, "Prior"))
     broadcast_event(EMOTION_EVENT_PREV);
   else if (!strcmp(ev->keyname, "Next"))
     broadcast_event(EMOTION_EVENT_NEXT);
   else if (!strcmp(ev->keyname, "0"))
     broadcast_event(EMOTION_EVENT_0);
   else if (!strcmp(ev->keyname, "1"))
     broadcast_event(EMOTION_EVENT_1);
   else if (!strcmp(ev->keyname, "2"))
     broadcast_event(EMOTION_EVENT_2);
   else if (!strcmp(ev->keyname, "3"))
     broadcast_event(EMOTION_EVENT_3);
   else if (!strcmp(ev->keyname, "4"))
     broadcast_event(EMOTION_EVENT_4);
   else if (!strcmp(ev->keyname, "5"))
     broadcast_event(EMOTION_EVENT_5);
   else if (!strcmp(ev->keyname, "6"))
     broadcast_event(EMOTION_EVENT_6);
   else if (!strcmp(ev->keyname, "7"))
     broadcast_event(EMOTION_EVENT_7);
   else if (!strcmp(ev->keyname, "8"))
     broadcast_event(EMOTION_EVENT_8);
   else if (!strcmp(ev->keyname, "9"))
     broadcast_event(EMOTION_EVENT_9);
   else if (!strcmp(ev->keyname, "-"))
     broadcast_event(EMOTION_EVENT_10);
   else if (!strcmp(ev->keyname, "bracketleft"))
     {
	Evas_List *l;

	for (l = video_objs; l; l = l->next)
	  {
	     Evas_Object *obj;

	     obj = l->data;
	     emotion_object_audio_volume_set(obj, emotion_object_audio_volume_get(obj) - 0.1);
	  }
     }
   else if (!strcmp(ev->keyname, "bracketright"))
     {
	Evas_List *l;

	for (l = video_objs; l; l = l->next)
	  {
	     Evas_Object *obj;

	     obj = l->data;
	     emotion_object_audio_volume_set(obj, emotion_object_audio_volume_get(obj) + 0.1);
	  }
     }
   else if (!strcmp(ev->keyname, "v"))
     {
	Evas_List *l;

	for (l = video_objs; l; l = l->next)
	  {
	     Evas_Object *obj;

	     obj = l->data;
	     if (emotion_object_video_mute_get(obj))
	       emotion_object_video_mute_set(obj, 0);
	     else
	       emotion_object_video_mute_set(obj, 1);
	  }
     }
   else if (!strcmp(ev->keyname, "a"))
     {
	Evas_List *l;

	for (l = video_objs; l; l = l->next)
	  {
	     Evas_Object *obj;

	     obj = l->data;
	     if (emotion_object_audio_mute_get(obj))
	       {
		  emotion_object_audio_mute_set(obj, 0);
		  printf("unmute\n");
	       }
	     else
	       {
		  emotion_object_audio_mute_set(obj, 1);
		  printf("mute\n");
	       }
	  }
     }
   else if (!strcmp(ev->keyname, "i"))
     {
	Evas_List *l;

	for (l = video_objs; l; l = l->next)
	  {
	     Evas_Object *obj;

	     obj = l->data;
	     printf("audio channels: %i\n", emotion_object_audio_channel_count(obj));
	     printf("video channels: %i\n", emotion_object_video_channel_count(obj));
	     printf("spu channels: %i\n", emotion_object_spu_channel_count(obj));
	     printf("seekable: %i\n", emotion_object_seekable_get(obj));
	  }
     }
   else if (!strcmp(ev->keyname, "f"))
     {
	if (!ecore_evas_fullscreen_get(ecore_evas))
	  ecore_evas_fullscreen_set(ecore_evas, 1);
	else
	  ecore_evas_fullscreen_set(ecore_evas, 0);
     }
   else if (!strcmp(ev->keyname, "d"))
     {
	if (!ecore_evas_avoid_damage_get(ecore_evas))
	  ecore_evas_avoid_damage_set(ecore_evas, 1);
	else
	  ecore_evas_avoid_damage_set(ecore_evas, 0);
     }
   else if (!strcmp(ev->keyname, "s"))
     {
	if (!ecore_evas_shaped_get(ecore_evas))
	  {
	     ecore_evas_shaped_set(ecore_evas, 1);
	     evas_object_hide(o_bg);
	  }
	else
	  {
	     ecore_evas_shaped_set(ecore_evas, 0);
	     evas_object_show(o_bg);
	  }
     }
   else if (!strcmp(ev->keyname, "b"))
     {
	if (!ecore_evas_borderless_get(ecore_evas))
	  ecore_evas_borderless_set(ecore_evas, 1);
	else
	  ecore_evas_borderless_set(ecore_evas, 0);
     }
   else if (!strcmp(ev->keyname, "q"))
     {
	ecore_main_loop_quit();
	while (video_objs)
	  {
	     printf("del obj!\n");
	     evas_object_del(video_objs->data);
	     video_objs = evas_list_remove_list(video_objs, video_objs);
	     printf("done\n");
	  }
     }
   else if (!strcmp(ev->keyname, "z"))
     {
	Evas_List *l;

	vis = (vis + 1) % EMOTION_VIS_LAST;
	printf("new visualization: %d\n", vis);


	for (l = video_objs; l; l = l->next)
	  {
	     Evas_Object *obj;
	     Evas_Bool supported;

	     obj = l->data;
	     supported = emotion_object_vis_supported(obj, vis);
	     if (supported)
	       emotion_object_vis_set(obj, vis);
	     else
	       {
		  const char *file;

		  file = emotion_object_file_get(obj);
		  printf("object %p (%s) does not support visualization %d\n",
			 obj, file, vis);
	       }
	  }
     }
   else
     {
	printf("UNHANDLED: %s\n", ev->keyname);
     }
}

static void
video_obj_in_cb(void *data, Evas *ev, Evas_Object *obj, void *event_info)
{
//   evas_object_color_set(obj, 255, 255, 255, 100);
}

static void
video_obj_out_cb(void *data, Evas *ev, Evas_Object *obj, void *event_info)
{
//   evas_object_color_set(obj, 255, 255, 255, 200);
}

static void
video_obj_down_cb(void *data, Evas *ev, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Down *e;

   e = event_info;
   evas_object_color_set(obj, 200, 50, 40, 200);
   evas_object_raise(obj);
}

static void
video_obj_up_cb(void *data, Evas *ev, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Up *e;

   e = event_info;
   evas_object_color_set(obj, 100, 100, 100, 100);
}

static void
video_obj_move_cb(void *data, Evas *ev, Evas_Object *obj, void *event_info)
{
   Evas_Event_Mouse_Move *e;

   e = event_info;
   if (e->buttons & 0x1)
     {
	Evas_Coord x, y;

	evas_object_geometry_get(obj, &x, &y, NULL, NULL);
	x += e->cur.canvas.x - e->prev.canvas.x;
	y += e->cur.canvas.y - e->prev.canvas.y;
	evas_object_move(obj, x, y);
     }
   else if (e->buttons & 0x4)
     {
	Evas_Coord w, h;

	evas_object_geometry_get(obj, NULL, NULL, &w, &h);
	w += e->cur.canvas.x - e->prev.canvas.x;
	h += e->cur.canvas.y - e->prev.canvas.y;
	evas_object_resize(obj, w, h);
     }
}

static void
video_obj_time_changed(Evas_Object *obj, Evas_Object *edje)
{
   double pos, len, scale;
   char buf[256];
   int ph, pm, ps, pf, lh, lm, ls;

   pos = emotion_object_position_get(obj);
   len = emotion_object_play_length_get(obj);
//   printf("%3.3f, %3.3f\n", pos, len);
   scale = (len > 0.0) ? pos / len : 0.0;
   edje_object_part_drag_value_set(edje, "video_progress", scale, 0.0);
   lh = len / 3600;
   lm = len / 60 - (lh * 60);
   ls = len - (lm * 60);
   ph = pos / 3600;
   pm = pos / 60 - (ph * 60);
   ps = pos - (pm * 60);
   pf = pos * 100 - (ps * 100) - (pm * 60 * 100) - (ph * 60 * 60 * 100);
   snprintf(buf, sizeof(buf), "%i:%02i:%02i.%02i / %i:%02i:%02i",
	    ph, pm, ps, pf, lh, lm, ls);
   edje_object_part_text_set(edje, "video_progress_txt", buf);
}

static void
video_obj_frame_decode_cb(void *data, Evas_Object *obj, void *event_info)
{
   video_obj_time_changed(obj, data);

   if (0)
     {
	double t;
	static double pt = 0.0;
	t = ecore_time_get();
	printf("FPS: %3.3f\n", 1.0 / (t - pt));
	pt = t;
     }
}

static void
video_obj_frame_resize_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *oe;
   int iw, ih;
   Evas_Coord w, h;
   double ratio;

   oe = data;
   emotion_object_size_get(obj, &iw, &ih);
   ratio = emotion_object_ratio_get(obj);
   printf("HANDLE %ix%i @ %3.3f\n", iw, ih, ratio);
   if (ratio > 0.0) iw = (ih * ratio) + 0.5;
   edje_extern_object_min_size_set(obj, iw, ih);
   edje_object_part_swallow(oe, "video_swallow", obj);
   edje_object_size_min_calc(oe, &w, &h);
   evas_object_resize(oe, w, h);
   edje_extern_object_min_size_set(obj, 0, 0);
   edje_object_part_swallow(oe, "video_swallow", obj);
}

static void
video_obj_length_change_cb(void *data, Evas_Object *obj, void *event_info)
{
   video_obj_time_changed(obj, data);
}

static void
video_obj_position_update_cb(void *data, Evas_Object *obj, void *event_info)
{
   video_obj_time_changed(obj, data);
}

static void
video_obj_stopped_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *oe;

   oe = data;
   printf("video stopped!\n");
   emotion_object_position_set(obj, 0.0);
   emotion_object_play_set(obj, 1);
}

static void
video_obj_channels_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *oe;

   oe = data;
   printf("channels changed: [AUD %i][VID %i][SPU %i]\n",
	  emotion_object_audio_channel_count(obj),
	  emotion_object_video_channel_count(obj),
	  emotion_object_spu_channel_count(obj));
}

static void
video_obj_title_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *oe;

   oe = data;
   printf("video title to: \"%s\"\n", emotion_object_title_get(obj));
}

static void
video_obj_progress_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *oe;

   oe = data;
   printf("progress: \"%s\" %3.3f\n",
	  emotion_object_progress_info_get(obj),
	  emotion_object_progress_status_get(obj));
}

static void
video_obj_ref_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *oe;

   oe = data;
   printf("video ref to: \"%s\" %i\n",
	  emotion_object_ref_file_get(obj),
	  emotion_object_ref_num_get(obj));
}

static void
video_obj_button_num_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *oe;

   oe = data;
   printf("video spu buttons to: %i\n",
	  emotion_object_spu_button_count_get(obj));
}

static void
video_obj_button_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *oe;

   oe = data;
   printf("video selected spu button: %i\n",
	  emotion_object_spu_button_get(obj));
}



static void
video_obj_signal_play_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Evas_Object *ov;

   ov = data;
   emotion_object_play_set(ov, 1);
   edje_object_signal_emit(o, "video_state", "play");
}

static void
video_obj_signal_pause_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Evas_Object *ov;

   ov = data;
   emotion_object_play_set(ov, 0);
   edje_object_signal_emit(o, "video_state", "pause");
}

static void
video_obj_signal_stop_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Evas_Object *ov;

   ov = data;
   emotion_object_play_set(ov, 0);
   emotion_object_position_set(ov, 0);
   edje_object_signal_emit(o, "video_state", "stop");
}

static void
video_obj_signal_jump_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Evas_Object *ov;
   double len;
   double x, y;

   ov = data;
   edje_object_part_drag_value_get(o, source, &x, &y);
   len = emotion_object_play_length_get(ov);
   emotion_object_position_set(ov, x * len);
}

static void
video_obj_signal_speed_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Evas_Object *ov;
   double spd;
   double x, y;
   char buf[256];

   ov = data;
   edje_object_part_drag_value_get(o, source, &x, &y);
   spd = 255 * y;
   evas_object_color_set(ov, spd, spd, spd, spd);
   snprintf(buf, sizeof(buf), "%.0f", spd);
   edje_object_part_text_set(o, "video_speed_txt", buf);
}

static void
video_obj_signal_frame_move_start_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Frame_Data *fd;
   Evas_Coord x, y;

   fd = evas_object_data_get(o, "frame_data");
   fd->moving = 1;
   evas_pointer_canvas_xy_get(evas_object_evas_get(o), &x, &y);
   fd->x = x;
   fd->y = y;
   evas_object_raise(o);
}

static void
video_obj_signal_frame_move_stop_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Frame_Data *fd;

   fd = evas_object_data_get(o, "frame_data");
   fd->moving = 0;
}

static void
video_obj_signal_frame_resize_start_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Frame_Data *fd;
   Evas_Coord x, y;

   fd = evas_object_data_get(o, "frame_data");
   fd->resizing = 1;
   evas_pointer_canvas_xy_get(evas_object_evas_get(o), &x, &y);
   fd->x = x;
   fd->y = y;
   evas_object_raise(o);
}

static void
video_obj_signal_frame_resize_stop_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Frame_Data *fd;

   fd = evas_object_data_get(o, "frame_data");
   fd->resizing = 0;
}

static void
video_obj_signal_frame_move_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
   Frame_Data *fd;

   fd = evas_object_data_get(o, "frame_data");
   if (fd->moving)
     {
	Evas_Coord x, y, ox, oy;

	evas_pointer_canvas_xy_get(evas_object_evas_get(o), &x, &y);
	evas_object_geometry_get(o, &ox, &oy, NULL, NULL);
	evas_object_move(o, ox + (x - fd->x), oy + (y - fd->y));
	fd->x = x;
	fd->y = y;
     }
   else if (fd->resizing)
     {
	Evas_Coord x, y, ow, oh;

	evas_pointer_canvas_xy_get(evas_object_evas_get(o), &x, &y);
	evas_object_geometry_get(o, NULL, NULL, &ow, &oh);
	evas_object_resize(o, ow + (x - fd->x), oh + (y - fd->y));
	fd->x = x;
	fd->y = y;
     }
}


static void
init_video_object(char *module_filename, char *filename)
{
   Evas_Object *o, *oe;
   int iw, ih;
   Evas_Coord w, h;
   Frame_Data *fd;


/* basic video object setup */
   o = emotion_object_add(evas);
   if (!emotion_object_init(o, module_filename))
     return;
   emotion_object_vis_set(o, vis);
   emotion_object_file_set(o, filename);
   emotion_object_play_set(o, 1);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 320, 240);
   emotion_object_smooth_scale_set(o, 1);
   evas_object_show(o);
/* end basic video setup. all the rest here is just to be fancy */


   video_objs = evas_list_append(video_objs, o);

   emotion_object_size_get(o, &iw, &ih);
   w = iw; h = ih;

   fd = calloc(1, sizeof(Frame_Data));

   oe = edje_object_add(evas);
   evas_object_data_set(oe, "frame_data", fd);
   edje_object_file_set(oe, PACKAGE_DATA_DIR"/data/theme.edj", "video_controller");
   edje_extern_object_min_size_set(o, w, h);
   edje_object_part_swallow(oe, "video_swallow", o);
   edje_object_size_min_calc(oe, &w, &h);
//   evas_object_move(oe, rand() % (int)(startw - w), rand() % (int)(starth - h));
   evas_object_move(oe, 0, 0);
   evas_object_resize(oe, w, h);
   edje_extern_object_min_size_set(o, 0, 0);
   edje_object_part_swallow(oe, "video_swallow", o);

   evas_object_smart_callback_add(o, "frame_decode", video_obj_frame_decode_cb, oe);
   evas_object_smart_callback_add(o, "frame_resize", video_obj_frame_resize_cb, oe);
   evas_object_smart_callback_add(o, "length_change", video_obj_length_change_cb, oe);
   evas_object_smart_callback_add(o, "position_update", video_obj_position_update_cb, oe);

   evas_object_smart_callback_add(o, "decode_stop", video_obj_stopped_cb, oe);
   evas_object_smart_callback_add(o, "channels_change", video_obj_channels_cb, oe);
   evas_object_smart_callback_add(o, "title_change", video_obj_title_cb, oe);
   evas_object_smart_callback_add(o, "progress_change", video_obj_progress_cb, oe);
   evas_object_smart_callback_add(o, "ref_change", video_obj_ref_cb, oe);
   evas_object_smart_callback_add(o, "button_num_change", video_obj_button_num_cb, oe);
   evas_object_smart_callback_add(o, "button_change", video_obj_button_cb, oe);

   edje_object_signal_callback_add(oe, "video_control", "play", video_obj_signal_play_cb, o);
   edje_object_signal_callback_add(oe, "video_control", "pause", video_obj_signal_pause_cb, o);
   edje_object_signal_callback_add(oe, "video_control", "stop", video_obj_signal_stop_cb, o);
   edje_object_signal_callback_add(oe, "drag", "video_progress", video_obj_signal_jump_cb, o);
   edje_object_signal_callback_add(oe, "drag", "video_speed", video_obj_signal_speed_cb, o);

   edje_object_signal_callback_add(oe, "frame_move", "start", video_obj_signal_frame_move_start_cb, oe);
   edje_object_signal_callback_add(oe, "frame_move", "stop", video_obj_signal_frame_move_stop_cb, oe);
   edje_object_signal_callback_add(oe, "frame_resize", "start", video_obj_signal_frame_resize_start_cb, oe);
   edje_object_signal_callback_add(oe, "frame_resize", "stop", video_obj_signal_frame_resize_stop_cb, oe);
   edje_object_signal_callback_add(oe, "mouse,move", "*", video_obj_signal_frame_move_cb, oe);

   edje_object_part_drag_value_set(oe, "video_speed", 0.0, 1.0);
   edje_object_part_text_set(oe, "video_speed_txt", "1.0");

   edje_object_signal_emit(o, "video_state", "play");

   evas_object_show(oe);
}

static int
enter_idle(void *data)
{
   double t;
   static double pt = 0.0;
   static int frames = 0;

   t = ecore_time_get();
   if (frames == 0) pt = t;
   frames++;
   if (frames == 100)
     {
//	printf("FPS: %3.3f\n", frames / (t - pt));
	frames = 0;
     }
   return 1;
}

static int
check_positions(void *data)
{
   const Evas_List *lst;

   for (lst = video_objs; lst != NULL; lst = lst->next)
     video_obj_time_changed(lst->data, evas_object_smart_parent_get(lst->data));

   return !!video_objs;
}

int
main(int argc, char **argv)
{
   char *module_filename;
   int i;

   if (main_start(argc, argv) < 1) return -1;
   bg_setup();

   module_filename = "xine";

   for (i = 1; i < argc; i++)
     {
	if (((!strcmp(argv[i], "-g")) ||
	    (!strcmp(argv[i], "-geometry")) ||
	    (!strcmp(argv[i], "--geometry"))) && (i < (argc - 1)))
	     i++;
	else if (((!strcmp(argv[i], "-h")) ||
	    (!strcmp(argv[i], "-help")) ||
	    (!strcmp(argv[i], "--help"))))
	  {
	     printf("Usage:\n");
	     printf("  %s [-gl] [-g WxH] [-vis NUMBER] [-xine] [-gstreamer] filename\n", argv[0]);
	     exit(-1);
	  }
	else if (!strcmp(argv[i], "-gl"))
	  {
	  }
	else if (!strcmp(argv[i], "-fb"))
	  {
	  }
	else if (!strcmp(argv[i], "-xr"))
	  {
	  }
	else if (!strcmp(argv[i], "-xine"))
	  {
             module_filename = "xine";
	  }
	else if (!strcmp(argv[i], "-gstreamer"))
	  {
             module_filename = "gstreamer";
	  }
	else if ((!strcmp(argv[i], "-vis")) && (i < (argc - 1)))
	  {
	     i++;
	  }
        else
	  {
             printf ("module : %s\n", module_filename);
	     init_video_object(module_filename, argv[i]);
	  }
     }

   ecore_idle_enterer_add(enter_idle, NULL);
   ecore_animator_add(check_positions, NULL);

   ecore_main_loop_begin();
   main_stop();
   return 0;
}

#else
int main()
{
	puts("Could not find Ecore_X.h or Ecore_Fb.h so test is disabled");
	return 0;
}
#endif
