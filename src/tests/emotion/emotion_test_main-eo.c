#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT
#endif

#include <Eo.h>
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
   "(C) 2011-2014 Enlightenment",
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
      ECORE_GETOPT_STORE_TRUE('w', "webcams", "show all the available v4l streams"),
      ECORE_GETOPT_STORE_TRUE('r', "reflex", "show video reflex effect"),
      ECORE_GETOPT_STORE_TRUE('l', "loop", "restart the video when end reached"),
      ECORE_GETOPT_STORE_TRUE('p', "position", "start the video from last know position"),
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

static void bg_key_down(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED);


static Evas_Object *o_bg = NULL;

static Ecore_Evas  *ecore_evas = NULL;
static Evas        *evas       = NULL;
static int          startw     = 800;
static int          starth     = 600;

static Eina_List   *video_objs = NULL;
static Emotion_Vis  vis        = EMOTION_VIS_NONE;
static unsigned char reflex    = 0;
static unsigned char loop      = 0;
static unsigned char last_position_load  = 0;
static const char  *theme_file = NULL;

static void
bg_resize(Evas_Coord w, Evas_Coord h)
{
   efl_gfx_size_set(o_bg, w, h);
}

static void
main_resize(Ecore_Evas *ee)
{
   Evas_Coord w, h;

   evas_output_viewport_get(ecore_evas_get(ee), NULL, NULL, &w, &h);
   bg_resize(w, h);
}

static Eina_Bool
main_signal_exit(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev EINA_UNUSED)
{
   Evas_Object *o;

   ecore_main_loop_quit();
   EINA_LIST_FREE(video_objs, o)
     {
        emotion_object_last_position_save(o);
        efl_del(o);
     }
   return EINA_TRUE;
}

static void
main_delete_request(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static void
bg_setup(void)
{
   o_bg = efl_add(EDJE_OBJECT_CLASS, evas);
   efl_file_set(o_bg, theme_file, "background");
   efl_gfx_position_set(o_bg, 0, 0);
   efl_gfx_size_set(o_bg, startw, starth);
   efl_gfx_stack_layer_set(o_bg, -999);
   efl_gfx_visible_set(o_bg, EINA_TRUE);
   evas_object_focus_set(o_bg, EINA_TRUE);
   efl_event_callback_add(o_bg, EFL_EVENT_KEY_DOWN, bg_key_down, NULL);
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
bg_key_down(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Key *ev = event->info;
   Eina_List *l;
   Evas_Object *o;
   const char *keyname = efl_input_key_name_get(ev);

   if (!keyname) return;

   if      (!strcmp(keyname, "Escape"))
     ecore_main_loop_quit();
   else if (!strcmp(keyname, "Up"))
     broadcast_event(EMOTION_EVENT_UP);
   else if (!strcmp(keyname, "Down"))
     broadcast_event(EMOTION_EVENT_DOWN);
   else if (!strcmp(keyname, "Left"))
     broadcast_event(EMOTION_EVENT_LEFT);
   else if (!strcmp(keyname, "Right"))
     broadcast_event(EMOTION_EVENT_RIGHT);
   else if (!strcmp(keyname, "Return"))
     broadcast_event(EMOTION_EVENT_SELECT);
   else if (!strcmp(keyname, "m"))
     broadcast_event(EMOTION_EVENT_MENU1);
   else if (!strcmp(keyname, "Prior"))
     broadcast_event(EMOTION_EVENT_PREV);
   else if (!strcmp(keyname, "Next"))
     broadcast_event(EMOTION_EVENT_NEXT);
   else if (!strcmp(keyname, "0"))
     broadcast_event(EMOTION_EVENT_0);
   else if (!strcmp(keyname, "1"))
     broadcast_event(EMOTION_EVENT_1);
   else if (!strcmp(keyname, "2"))
     broadcast_event(EMOTION_EVENT_2);
   else if (!strcmp(keyname, "3"))
     broadcast_event(EMOTION_EVENT_3);
   else if (!strcmp(keyname, "4"))
     broadcast_event(EMOTION_EVENT_4);
   else if (!strcmp(keyname, "5"))
     broadcast_event(EMOTION_EVENT_5);
   else if (!strcmp(keyname, "6"))
     broadcast_event(EMOTION_EVENT_6);
   else if (!strcmp(keyname, "7"))
     broadcast_event(EMOTION_EVENT_7);
   else if (!strcmp(keyname, "8"))
     broadcast_event(EMOTION_EVENT_8);
   else if (!strcmp(keyname, "9"))
     broadcast_event(EMOTION_EVENT_9);
   else if (!strcmp(keyname, "-"))
     broadcast_event(EMOTION_EVENT_10);
   else if (!strcmp(keyname, "v"))
     {
        EINA_LIST_FOREACH(video_objs, l, o)
          {
             if (emotion_object_video_mute_get(o))
               emotion_object_video_mute_set(o, EINA_FALSE);
             else
               emotion_object_video_mute_set(o, EINA_TRUE);
          }
     }
   else if (!strcmp(keyname, "a"))
     {
        EINA_LIST_FOREACH(video_objs, l, o)
          {
            if (emotion_object_audio_mute_get(o))
              {
                 emotion_object_audio_mute_set(o, EINA_FALSE);
                 printf("unmute\n");
              }
            else
              {
                 emotion_object_audio_mute_set(o, EINA_TRUE);
                 printf("mute\n");
              }
          }
     }
   else if (!strcmp(keyname, "i"))
     {
        EINA_LIST_FOREACH(video_objs, l, o)
          {
             printf("audio channels: %i\n", emotion_object_audio_channel_count(o));
             printf("video channels: %i\n", emotion_object_video_channel_count(o));
             printf("spu channels: %i\n", emotion_object_spu_channel_count(o));
             printf("seekable: %i\n", emotion_object_seekable_get(o));
          }
     }
   else if (!strcmp(keyname, "f"))
     {
       if (!ecore_evas_fullscreen_get(ecore_evas))
         ecore_evas_fullscreen_set(ecore_evas, EINA_TRUE);
       else
         ecore_evas_fullscreen_set(ecore_evas, EINA_FALSE);
     }
   else if (!strcmp(keyname, "d"))
     {
        if (!ecore_evas_avoid_damage_get(ecore_evas))
          ecore_evas_avoid_damage_set(ecore_evas, EINA_TRUE);
        else
          ecore_evas_avoid_damage_set(ecore_evas, EINA_FALSE);
     }
   else if (!strcmp(keyname, "s"))
     {
        if (!ecore_evas_shaped_get(ecore_evas))
          {
             ecore_evas_shaped_set(ecore_evas, EINA_TRUE);
             evas_object_hide(o_bg);
          }
        else
          {
             ecore_evas_shaped_set(ecore_evas, EINA_FALSE);
             evas_object_show(o_bg);
          }
     }
   else if (!strcmp(keyname, "b"))
     {
        if (!ecore_evas_borderless_get(ecore_evas))
          ecore_evas_borderless_set(ecore_evas, EINA_TRUE);
        else
          ecore_evas_borderless_set(ecore_evas, EINA_FALSE);
     }
   else if (!strcmp(keyname, "q"))
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
   else if (!strcmp(keyname, "z"))
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
        printf("UNHANDLED: %s\n", keyname);
     }
}

static void
_oe_free_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   free(data);
}

static void
video_obj_time_changed(Evas_Object *obj, Evas_Object *edje)
{
   double pos, len, scale;
   char buf[256];
   int ph, pm, ps, pf, lh, lm, ls;

   pos = emotion_object_position_get(obj);
   len = emotion_object_play_length_get(obj);
   scale = (len > 0.0) ? pos / len : 0.0;
   efl_ui_drag_value_set(efl_part(edje, "video_progress"), scale, 0.0);

   lh = len / 3600;
   lm = len / 60 - (lh * 60);
   ls = len - (lh * 3600) - (lm * 60);

   ph = pos / 3600;
   pm = pos / 60 - (ph * 60);
   ps = pos - (ph * 3600) - (pm * 60);

   pf = pos * 100 - (ps * 100) - (pm * 60 * 100) - (ph * 60 * 60 * 100);

   snprintf(buf, sizeof(buf), "%i:%02i:%02i.%02i / %i:%02i:%02i",
            ph, pm, ps, pf, lh, lm, ls);
   efl_text_set(efl_part(edje, "video_progress_txt"), buf);
}

static void
video_obj_frame_decode_cb(void *data, const Efl_Event *event)
{
   video_obj_time_changed(event->object, data);

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
video_obj_frame_resize_cb(void *data, const Efl_Event *event)
{
   Evas_Object *oe;
   int iw, ih;
   Evas_Coord w, h;
   double ratio;

   oe = data;
   emotion_object_size_get(event->object, &iw, &ih);
   ratio = emotion_object_ratio_get(event->object);
   printf("HANDLE %ix%i @ %3.3f\n", iw, ih, ratio);
   if (ratio > 0.0) iw = (ih * ratio) + 0.5;
   evas_object_size_hint_min_set(event->object, iw, ih);
   edje_object_part_swallow(oe, "video_swallow", event->object);
   edje_object_size_min_calc(oe, &w, &h);
   efl_gfx_size_set(oe, w, h);
   evas_object_size_hint_min_set(event->object, 0, 0);
   edje_object_part_swallow(oe, "video_swallow", event->object);
}

static void
video_obj_length_change_cb(void *data, const Efl_Event *event)
{
   printf("len change!\n");
   video_obj_time_changed(event->object, data);
}

static void
video_obj_position_update_cb(void *data, const Efl_Event *event)
{
   printf("pos up!\n");
   video_obj_time_changed(event->object, data);
}

static void
video_obj_stopped_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   printf("video stopped!!!\n");
   if (loop)
     {
        emotion_object_position_set(event->object, 0.0);
        emotion_object_play_set(event->object, EINA_TRUE);
     }
}

static void
video_obj_channels_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   printf("channels changed: [AUD %i][VID %i][SPU %i]\n",
          emotion_object_audio_channel_count(event->object),
          emotion_object_video_channel_count(event->object),
          emotion_object_spu_channel_count(event->object));
}

static void
 video_obj_title_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   printf("video title to: \"%s\"\n", emotion_object_title_get(event->object));
}

static void
video_obj_progress_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   printf("progress: \"%s\" %3.3f\n",
          emotion_object_progress_info_get(event->object),
          emotion_object_progress_status_get(event->object));
}

static void
video_obj_ref_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   printf("video ref to: \"%s\" %i\n",
          emotion_object_ref_file_get(event->object),
          emotion_object_ref_num_get(event->object));
}

static void
video_obj_button_num_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   printf("video spu buttons to: %i\n",
          emotion_object_spu_button_count_get(event->object));
}

static void
video_obj_button_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   printf("video selected spu button: %i\n",
          emotion_object_spu_button_get(event->object));
}

static void
video_obj_signal_play_cb(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Evas_Object *ov = data;
   emotion_object_play_set(ov, EINA_TRUE);
   efl_canvas_layout_signal_emit(o, "video_state", "play");
}

static void
video_obj_signal_pause_cb(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Evas_Object *ov = data;
   emotion_object_play_set(ov, EINA_FALSE);
   efl_canvas_layout_signal_emit(o, "video_state", "pause");
}

static void
video_obj_signal_stop_cb(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Evas_Object *ov = data;
   emotion_object_play_set(ov, EINA_FALSE);
   emotion_object_position_set(ov, 0.0);
   efl_canvas_layout_signal_emit(o, "video_state", "stop");
}

static void
video_obj_signal_jump_cb(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source)
{
   Evas_Object *ov = data;
   double len;
   double x, y;

   efl_ui_drag_value_get(efl_part(o, source), &x, &y);
   len = emotion_object_play_length_get(ov);
   emotion_object_position_set(ov, x * len);
}

static void
video_obj_signal_alpha_cb(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source)
{
   Evas_Object *ov = data;
   double alpha;
   double x, y;
   char buf[256];

   efl_ui_drag_value_get(efl_part(o, source), &x, &y);
   alpha = 255 * y;
   efl_gfx_color_set(ov, alpha, alpha, alpha, alpha);
   snprintf(buf, sizeof(buf), "alpha %.0f", alpha);
   efl_text_set(efl_part(o, "video_alpha_txt"), buf);
}

static void
video_obj_signal_vol_cb(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source)
{
   Evas_Object *ov = data;
   double vol;
   char buf[256];

   efl_ui_drag_value_get(efl_part(o, source), NULL, &vol);
   emotion_object_audio_volume_set(ov, vol);
   snprintf(buf, sizeof(buf), "vol %.2f", vol);
   efl_text_set(efl_part(o, "video_volume_txt"), buf);
}

static void
video_obj_signal_frame_move_start_cb(void *data EINA_UNUSED, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Frame_Data *fd;
   Evas_Coord x, y;

   fd = efl_key_data_get(o, "frame_data");
   if (!fd) return;
   fd->moving = 1;
   evas_canvas_pointer_canvas_xy_get(evas_object_evas_get(o), &x, &y);
   fd->x = x;
   fd->y = y;
   efl_gfx_stack_raise(o);
}

static void
video_obj_signal_frame_move_stop_cb(void *data EINA_UNUSED, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Frame_Data *fd;

   fd = efl_key_data_get(o, "frame_data");
   if (!fd) return;
   fd->moving = 0;
}

static void
video_obj_signal_frame_resize_start_cb(void *data EINA_UNUSED, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Frame_Data *fd;
   Evas_Coord x, y;

   fd = efl_key_data_get(o, "frame_data");
   if (!fd) return;
   fd->resizing = 1;
   evas_canvas_pointer_canvas_xy_get(evas_object_evas_get(o), &x, &y);
   fd->x = x;
   fd->y = y;
   efl_gfx_stack_raise(o);
}

static void
video_obj_signal_frame_resize_stop_cb(void *data EINA_UNUSED, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Frame_Data *fd;

   fd = efl_key_data_get(o, "frame_data");
   if (!fd) return;
   fd->resizing = 0;
}

static void
video_obj_signal_frame_move_cb(void *data EINA_UNUSED, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   Frame_Data *fd;

   fd = efl_key_data_get(o, "frame_data");
   if (!fd) return;
   if (fd->moving)
     {
        Evas_Coord x, y, ox, oy;

        evas_canvas_pointer_canvas_xy_get(evas_object_evas_get(o), &x, &y);
        efl_gfx_position_get(o, &ox, &oy);
        efl_gfx_position_set(o, ox + (x - fd->x), oy + (y - fd->y));
        fd->x = x;
        fd->y = y;
     }
   else if (fd->resizing)
     {
        Evas_Coord x, y, ow, oh;

        evas_canvas_pointer_canvas_xy_get(evas_object_evas_get(o), &x, &y);
        efl_gfx_size_get(o, &ow, &oh);
        evas_object_resize(o, ow + (x - fd->x), oh + (y - fd->y));
        fd->x = x;
        fd->y = y;
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(emotion_object_test_callbacks,
       { EFL_CANVAS_VIDEO_EVENT_FRAME_DECODE, video_obj_frame_decode_cb },
       { EFL_CANVAS_VIDEO_EVENT_FRAME_RESIZE, video_obj_frame_resize_cb },
       { EFL_CANVAS_VIDEO_EVENT_LENGTH_CHANGE, video_obj_length_change_cb },
       { EFL_CANVAS_VIDEO_EVENT_POSITION_CHANGE, video_obj_position_update_cb },
       { EFL_CANVAS_VIDEO_EVENT_PLAYBACK_STOP, video_obj_stopped_cb },
       { EFL_CANVAS_VIDEO_EVENT_CHANNELS_CHANGE, video_obj_channels_cb },
       { EFL_CANVAS_VIDEO_EVENT_TITLE_CHANGE, video_obj_title_cb },
       { EFL_CANVAS_VIDEO_EVENT_PROGRESS_CHANGE, video_obj_progress_cb },
       { EFL_CANVAS_VIDEO_EVENT_REF_CHANGE, video_obj_ref_cb },
       { EFL_CANVAS_VIDEO_EVENT_BUTTON_NUM_CHANGE, video_obj_button_num_cb },
       { EFL_CANVAS_VIDEO_EVENT_BUTTON_CHANGE, video_obj_button_cb }
);

static void
init_video_object(const char *module_filename, const char *filename)
{
   Evas_Object *o, *oe;
   Evas_Coord w, h, offset;
   Frame_Data *fd;

   /* basic video object setup */
   o = emotion_object_add(evas);
   if ((module_filename) && (!emotion_object_init(o, module_filename)))
     return;
   emotion_object_vis_set(o, vis);
   if (!emotion_object_file_set(o, filename))
     return;
   if (last_position_load)
     emotion_object_last_position_load(o);
   emotion_object_play_set(o, 1);
   emotion_object_audio_volume_set(o, 0.5);
   evas_object_move(o, 0, 0);
   evas_object_resize(o, 320, 240);
   emotion_object_smooth_scale_set(o, 1);
   evas_object_show(o);

   video_objs = eina_list_append(video_objs, o);
   /* end basic video setup. all the rest here is just to be fancy */


   fd = calloc(1, sizeof(Frame_Data));
   if (!fd) exit(1);

   oe = efl_add(EDJE_OBJECT_CLASS, evas);
   efl_event_callback_add(oe, EFL_EVENT_DEL, _oe_free_cb, fd);
   efl_key_data_set(oe, "frame_data", fd);
   efl_file_set(oe, theme_file, reflex ? "video_controller/reflex" : "video_controller");
   edje_object_part_swallow(oe, "video_swallow", o);

   offset = 20 * (eina_list_count(video_objs) - 1);
   efl_gfx_position_set(oe, offset, offset);
   edje_object_size_min_calc(oe, &w, &h);
   efl_gfx_size_set(oe, w, h);

   efl_event_callback_array_add(o, emotion_object_test_callbacks(), oe);

   efl_canvas_layout_signal_callback_add(oe, "video_control", "play", video_obj_signal_play_cb, o);
   efl_canvas_layout_signal_callback_add(oe, "video_control", "pause", video_obj_signal_pause_cb, o);
   efl_canvas_layout_signal_callback_add(oe, "video_control", "stop", video_obj_signal_stop_cb, o);
   efl_canvas_layout_signal_callback_add(oe, "drag", "video_progress", video_obj_signal_jump_cb, o);
   efl_canvas_layout_signal_callback_add(oe, "drag", "video_alpha", video_obj_signal_alpha_cb, o);
   efl_canvas_layout_signal_callback_add(oe, "drag", "video_volume", video_obj_signal_vol_cb, o);
   efl_canvas_layout_signal_callback_add(oe, "frame_move", "start", video_obj_signal_frame_move_start_cb, oe);
   efl_canvas_layout_signal_callback_add(oe, "frame_move", "stop", video_obj_signal_frame_move_stop_cb, oe);
   efl_canvas_layout_signal_callback_add(oe, "frame_resize", "start", video_obj_signal_frame_resize_start_cb, oe);
   efl_canvas_layout_signal_callback_add(oe, "frame_resize", "stop", video_obj_signal_frame_resize_stop_cb, oe);
   efl_canvas_layout_signal_callback_add(oe, "mouse, move", "*", video_obj_signal_frame_move_cb, oe);
   efl_ui_drag_value_set(efl_part(oe, "video_alpha"), 0.0, 1.0);
   efl_text_set(efl_part(oe, "video_alpha_txt"), "alpha 255");
   efl_ui_drag_value_set(efl_part(oe, "video_volume"), 0.0, 0.5);
   efl_text_set(efl_part(oe, "video_volume_txt"), "vol 0.50");
   efl_canvas_layout_signal_emit(oe, "video_state", "play");
   efl_gfx_visible_set(oe, EINA_TRUE);
}

int
main(int argc, char **argv)
{
   int args;
   Eina_Rectangle     geometry = {0, 0, startw, starth};
   char              *engine = NULL;
   char              *backend = NULL;
   Eina_Bool          webcams = EINA_FALSE;
   int                visual = EMOTION_VIS_NONE;
   unsigned char      help = 0;
   unsigned char      engines_listed = 0;
   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_STR(engine),
      ECORE_GETOPT_VALUE_BOOL(engines_listed),
      ECORE_GETOPT_VALUE_PTR_CAST(geometry),
      ECORE_GETOPT_VALUE_STR(backend),
      ECORE_GETOPT_VALUE_INT(visual),
      ECORE_GETOPT_VALUE_BOOL(webcams),
      ECORE_GETOPT_VALUE_BOOL(reflex),
      ECORE_GETOPT_VALUE_BOOL(loop),
      ECORE_GETOPT_VALUE_BOOL(last_position_load),
      ECORE_GETOPT_VALUE_NONE,
      ECORE_GETOPT_VALUE_NONE,
      ECORE_GETOPT_VALUE_NONE,
      ECORE_GETOPT_VALUE_BOOL(help),
      ECORE_GETOPT_VALUE_NONE
    };

   // init ecore_evas
   if (!ecore_evas_init())
     return -1;

   // init edje
   if (!edje_init())
     goto shutdown_ecore_evas;
   edje_frametime_set(1.0 / 30.0);

   // search the theme file
   struct stat st;
   if (stat(PACKAGE_BUILD_DIR"/src/tests/emotion/data/theme.edj", &st) == 0)
      theme_file = PACKAGE_BUILD_DIR"/src/tests/emotion/data/theme.edj";
   else if (stat(PACKAGE_DATA_DIR"/data/theme.edj", &st) == 0)
      theme_file = PACKAGE_DATA_DIR"/data/theme.edj";
   else
     {
        printf("Cannot find the theme file\n");
        goto shutdown_edje;
     }
   printf("theme file: %s\n", theme_file);

   // parse command line arguments
   ecore_app_args_set(argc, (const char **)argv);
   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0) goto shutdown_edje;
   else if (help) goto shutdown_edje;
   else if (engines_listed) goto shutdown_edje;
   else if ((args == argc) && (!webcams))
     {
        printf("must provide at least one file to play!\n");
        goto shutdown_edje;
     }
   if (geometry.w == 0) geometry.w = 320;
   if (geometry.h == 0) geometry.h = 240;
   vis = visual;

   printf("evas engine: %s\n", engine ? engine : "<auto>");
   printf("emotion backend: %s\n", backend ? backend : "<auto>");
   printf("vis: %d\n", vis);
   printf("geometry: %d %d %dx%d\n", geometry.x, geometry.y, geometry.w, geometry.h);

   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, main_signal_exit, NULL);

   // create the ecore_evas window
   ecore_evas = ecore_evas_new(engine, geometry.x, geometry.y,
                               geometry.w, geometry.h, NULL);
   if (!ecore_evas) goto shutdown_edje;
   ecore_evas_callback_delete_request_set(ecore_evas, main_delete_request);
   ecore_evas_callback_resize_set(ecore_evas, main_resize);
   ecore_evas_title_set(ecore_evas, "Evas Media Test Program");
   ecore_evas_name_class_set(ecore_evas, "evas_media_test", "main");
   ecore_evas_show(ecore_evas);
   evas = ecore_evas_get(ecore_evas);
   evas_image_cache_set(evas, 8 * 1024 * 1024);
   evas_font_cache_set(evas, 1 * 1024 * 1024);

   // init emotion
   emotion_init();

   // create the checkboard background edje object
   bg_setup();

   // open files and webcams
   for (; args < argc; args++)
     init_video_object(backend, argv[args]);

   if (webcams)
     {
        const Eina_List *wl, *l;
        Emotion_Webcam *webcam;

        wl = emotion_webcams_get();
        EINA_LIST_FOREACH(wl, l, webcam)
          {
             printf("Playing stream: '%s' url: '%s'\n",
                     emotion_webcam_name_get(webcam),
                     emotion_webcam_device_get(webcam));
             init_video_object(backend, emotion_webcam_device_get(webcam));
          }
     }

   // start the main loop
   ecore_main_loop_begin();

   // shutdown
   main_signal_exit(NULL, 0, NULL);

   emotion_shutdown();
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
