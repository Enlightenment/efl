#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Ecore_Evas.h>
#include <Edje.h>

#include "Emotion.h"

static const Ecore_Getopt options = {
   "emotion_test",
   "%prog [options] <filename>",
   "1.0.0",
   "(C) 2011 Enlightenment",
   "BSD\nThis is a 3 clause bsd bla bla",
   "a simple test program for emotion.",
   1,
   {
      ECORE_GETOPT_STORE_STR('e', "engine", "ecore-evas engine to use"),
      ECORE_GETOPT_CALLBACK_NOARGS('E', "list-engines", "list ecore-evas engines",
                                   ecore_getopt_callback_ecore_evas_list_engines, NULL),
      ECORE_GETOPT_CALLBACK_ARGS('g', "geometry", "geometry to use in x:y:w:h form.", "X:Y:W:H",
                                 ecore_getopt_callback_geometry_parse, NULL),
      ECORE_GETOPT_STORE_STR('b', "backend", "backend to use"),
      ECORE_GETOPT_STORE_INT('v', "vis", "visualization type"),
      ECORE_GETOPT_COUNT('v', "verbose", "be more verbose"),
      ECORE_GETOPT_STORE_TRUE('R', "reflex", "show video reflex effect"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('R', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
};

typedef struct _Frame_Data Frame_Data;

struct _Frame_Data
{
   unsigned char moving : 1;
   unsigned char resizing : 1;
   int        button;
   Evas_Coord x, y;
};

static void main_resize(Ecore_Evas *ee);
static Eina_Bool  main_signal_exit(void *data, int ev_type, void *ev);
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

static Eina_List   *video_objs = NULL;
static Emotion_Vis  vis        = EMOTION_VIS_NONE;
static unsigned char reflex    = 0;

static void
main_resize(Ecore_Evas *ee)
{
   Evas_Coord w, h;

   evas_output_viewport_get(ecore_evas_get(ee), NULL, NULL, &w, &h);
   bg_resize(w, h);
}

static Eina_Bool
main_signal_exit(void *data __UNUSED__, int ev_type __UNUSED__, void *ev __UNUSED__)
{
   Evas_Object *o;

   ecore_main_loop_quit();
   EINA_LIST_FREE(video_objs, o)
     {
	emotion_object_last_position_save(o);
	evas_object_del(o);
     }
   return EINA_TRUE;
}

static void
main_delete_request(Ecore_Evas *ee __UNUSED__)
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
   Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH(video_objs, l, obj)
     emotion_object_event_simple_send(obj, ev);
}

static void
bg_key_down(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Eina_List *l;
   Evas_Object *o;

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
	EINA_LIST_FOREACH(video_objs, l, o)
	  emotion_object_audio_volume_set(o, emotion_object_audio_volume_get(o) - 0.1);
     }
   else if (!strcmp(ev->keyname, "bracketright"))
     {
        EINA_LIST_FOREACH(video_objs, l, o)
	  emotion_object_audio_volume_set(o, emotion_object_audio_volume_get(o) + 0.1);
     }
   else if (!strcmp(ev->keyname, "v"))
     {
        EINA_LIST_FOREACH(video_objs, l, o)
	  {
	     if (emotion_object_video_mute_get(o))
	       emotion_object_video_mute_set(o, 0);
	     else
	       emotion_object_video_mute_set(o, 1);
	  }
     }
   else if (!strcmp(ev->keyname, "a"))
     {
        EINA_LIST_FOREACH(video_objs, l, o)
	  {
	     if (emotion_object_audio_mute_get(o))
	       {
		  emotion_object_audio_mute_set(o, 0);
		  printf("unmute\n");
	       }
	     else
	       {
		  emotion_object_audio_mute_set(o, 1);
		  printf("mute\n");
	       }
	  }
     }
   else if (!strcmp(ev->keyname, "i"))
     {
	EINA_LIST_FOREACH(video_objs, l, o)
	  {
	     printf("audio channels: %i\n", emotion_object_audio_channel_count(o));
	     printf("video channels: %i\n", emotion_object_video_channel_count(o));
	     printf("spu channels: %i\n", emotion_object_spu_channel_count(o));
	     printf("seekable: %i\n", emotion_object_seekable_get(o));
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
	     video_objs = eina_list_remove_list(video_objs, video_objs);
	     printf("done\n");
	  }
     }
   else if (!strcmp(ev->keyname, "z"))
     {
	vis = (vis + 1) % EMOTION_VIS_LAST;
	printf("new visualization: %d\n", vis);

	EINA_LIST_FOREACH(video_objs, l, o)
	  {
	     Eina_Bool supported;

	     supported = emotion_object_vis_supported(o, vis);
	     if (supported)
	       emotion_object_vis_set(o, vis);
	     else
	       {
		  const char *file;

		  file = emotion_object_file_get(o);
		  printf("object %p (%s) does not support visualization %d\n",
			 o, file, vis);
	       }
	  }
     }
   else
     {
	printf("UNHANDLED: %s\n", ev->keyname);
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
video_obj_frame_decode_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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
video_obj_frame_resize_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
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
video_obj_length_change_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   video_obj_time_changed(obj, data);
}

static void
video_obj_position_update_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   video_obj_time_changed(obj, data);
}

static void
video_obj_stopped_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("video stopped!\n");
   emotion_object_position_set(obj, 0.0);
   emotion_object_play_set(obj, 1);
}

static void
video_obj_channels_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("channels changed: [AUD %i][VID %i][SPU %i]\n",
	  emotion_object_audio_channel_count(obj),
	  emotion_object_video_channel_count(obj),
	  emotion_object_spu_channel_count(obj));
}

static void
video_obj_title_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("video title to: \"%s\"\n", emotion_object_title_get(obj));
}

static void
video_obj_progress_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("progress: \"%s\" %3.3f\n",
	  emotion_object_progress_info_get(obj),
	  emotion_object_progress_status_get(obj));
}

static void
video_obj_ref_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("video ref to: \"%s\" %i\n",
	  emotion_object_ref_file_get(obj),
	  emotion_object_ref_num_get(obj));
}

static void
video_obj_button_num_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("video spu buttons to: %i\n",
	  emotion_object_spu_button_count_get(obj));
}

static void
video_obj_button_cb(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   printf("video selected spu button: %i\n",
	  emotion_object_spu_button_get(obj));
}



static void
video_obj_signal_play_cb(void *data, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *ov = data;
   emotion_object_play_set(ov, 1);
   edje_object_signal_emit(o, "video_state", "play");
}

static void
video_obj_signal_pause_cb(void *data, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *ov = data;
   emotion_object_play_set(ov, 0);
   edje_object_signal_emit(o, "video_state", "pause");
}

static void
video_obj_signal_stop_cb(void *data, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *ov = data;
   emotion_object_play_set(ov, 0);
   emotion_object_position_set(ov, 0);
   edje_object_signal_emit(o, "video_state", "stop");
}

static void
video_obj_signal_jump_cb(void *data, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *ov = data;
   double len;
   double x, y;

   edje_object_part_drag_value_get(o, source, &x, &y);
   len = emotion_object_play_length_get(ov);
   emotion_object_position_set(ov, x * len);
}

static void
video_obj_signal_speed_cb(void *data, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *ov = data;
   double spd;
   double x, y;
   char buf[256];

   edje_object_part_drag_value_get(o, source, &x, &y);
   spd = 255 * y;
   evas_object_color_set(ov, spd, spd, spd, spd);
   snprintf(buf, sizeof(buf), "%.0f", spd);
   edje_object_part_text_set(o, "video_speed_txt", buf);
}

static void
video_obj_signal_frame_move_start_cb(void *data __UNUSED__, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
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
video_obj_signal_frame_move_stop_cb(void *data __UNUSED__, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Frame_Data *fd;

   fd = evas_object_data_get(o, "frame_data");
   fd->moving = 0;
}

static void
video_obj_signal_frame_resize_start_cb(void *data __UNUSED__, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
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
video_obj_signal_frame_resize_stop_cb(void *data __UNUSED__, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Frame_Data *fd;

   fd = evas_object_data_get(o, "frame_data");
   fd->resizing = 0;
}

static void
video_obj_signal_frame_move_cb(void *data __UNUSED__, Evas_Object *o, const char *emission __UNUSED__, const char *source __UNUSED__)
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
init_video_object(const char *module_filename, const char *filename)
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
   if (!emotion_object_file_set(o, filename))
     {
       return;
     }
   emotion_object_last_position_load(o);
   emotion_object_play_set(o, 1);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 320, 240);
   emotion_object_smooth_scale_set(o, 1);
   evas_object_show(o);
/* end basic video setup. all the rest here is just to be fancy */


   video_objs = eina_list_append(video_objs, o);

   emotion_object_size_get(o, &iw, &ih);
   w = iw; h = ih;

   fd = calloc(1, sizeof(Frame_Data));

   oe = edje_object_add(evas);
   evas_object_data_set(oe, "frame_data", fd);
   if (reflex)
     edje_object_file_set(oe, PACKAGE_DATA_DIR"/data/theme.edj", "video_controller/reflex");
   else
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

static Eina_Bool
enter_idle(void *data __UNUSED__)
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
   return EINA_TRUE;
}

static Eina_Bool
check_positions(void *data __UNUSED__)
{
   const Eina_List *lst;
   Evas_Object *o;

   EINA_LIST_FOREACH(video_objs, lst, o)
     video_obj_time_changed(o, evas_object_smart_parent_get(o));

   return !!video_objs;
}

int
main(int argc, char **argv)
{
   int args;
   Eina_Rectangle     geometry = {0, 0, startw, starth};
   char              *engine = NULL;
   char              *backend = NULL;
   int                verbose = 0;
   int                visual = EMOTION_VIS_NONE;
   unsigned char      help = 0;
   unsigned char      engines_listed = 0;
   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_STR(engine),
      ECORE_GETOPT_VALUE_BOOL(engines_listed),
      ECORE_GETOPT_VALUE_PTR_CAST(geometry),
      ECORE_GETOPT_VALUE_STR(backend),
      ECORE_GETOPT_VALUE_INT(visual),
      ECORE_GETOPT_VALUE_INT(verbose),
      ECORE_GETOPT_VALUE_BOOL(reflex),
      ECORE_GETOPT_VALUE_NONE,
      ECORE_GETOPT_VALUE_NONE,
      ECORE_GETOPT_VALUE_NONE,
      ECORE_GETOPT_VALUE_BOOL(help),
      ECORE_GETOPT_VALUE_NONE
    };


   if (!ecore_evas_init())
     return -1;
   if (!edje_init())
     goto shutdown_ecore_evas;

   start_time = ecore_time_get();
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, main_signal_exit, NULL);
   edje_frametime_set(1.0 / 30.0);

   ecore_app_args_set(argc, (const char **)argv);
   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0) goto shutdown_edje;
   else if (help) goto shutdown_edje;
   else if (engines_listed) goto shutdown_edje;
   else if (args == argc)
     {
        printf("must provide at least one file to play!\n");
        goto shutdown_edje;
     }

   if ((geometry.w == 0) || (geometry.h == 0))
     {
        if (geometry.w == 0) geometry.w = 320;
        if (geometry.h == 0) geometry.h = 240;
     }

   printf("evas engine: %s\n", engine ? engine : "<auto>");
   printf("emotion backend: %s\n", backend ? backend : "<auto>");
   printf("vis: %d\n", vis);
   printf("geometry: %d %d %dx%d\n", geometry.x, geometry.y, geometry.w, geometry.h);

   ecore_evas = ecore_evas_new
     (engine, geometry.x, geometry.y, geometry.w, geometry.h, NULL);
   if (!ecore_evas)
     goto shutdown_edje;

   ecore_evas_alpha_set(ecore_evas, EINA_TRUE);

   ecore_evas_callback_delete_request_set(ecore_evas, main_delete_request);
   ecore_evas_callback_resize_set(ecore_evas, main_resize);
   ecore_evas_title_set(ecore_evas, "Evas Media Test Program");
   ecore_evas_name_class_set(ecore_evas, "evas_media_test", "main");
   ecore_evas_show(ecore_evas);
   evas = ecore_evas_get(ecore_evas);
   evas_image_cache_set(evas, 8 * 1024 * 1024);
   evas_font_cache_set(evas, 1 * 1024 * 1024);
   evas_font_path_append(evas, PACKAGE_DATA_DIR"/data/fonts");

   bg_setup();

   for (; args < argc; args++)
     init_video_object(backend, argv[args]);

   ecore_idle_enterer_add(enter_idle, NULL);
   ecore_animator_add(check_positions, NULL);

   ecore_main_loop_begin();

   main_signal_exit(NULL, 0, NULL);
   ecore_evas_free(ecore_evas);
   ecore_evas_shutdown();
   edje_shutdown();

   return 0;

 shutdown_edje:
   edje_shutdown();
 shutdown_ecore_evas:
   ecore_evas_shutdown();

   return -1;
}
