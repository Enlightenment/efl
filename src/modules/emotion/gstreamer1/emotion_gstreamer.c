#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "emotion_gstreamer.h"

int _emotion_gstreamer_log_domain = -1;
Eina_Bool debug_fps = EINA_FALSE;

static int _emotion_init_count = 0;

/* Callbacks to get the eos */
static void _for_each_tag    (GstTagList const* list, gchar const* tag, void *data);
static void _free_metadata   (Emotion_Gstreamer_Metadata *m);

static GstElement * _create_pipeline (Emotion_Gstreamer *ev, Evas_Object *o, const char *uri, const char *suburi);

static GstBusSyncReply _bus_sync_handler(GstBus *bus,
                                         GstMessage *message,
                                         gpointer data);

static void em_audio_channel_volume_set(void *video, double vol);
static void em_audio_channel_mute_set(void *video, int mute);

/* Module interface */

static const char *
emotion_visualization_element_name_get(Emotion_Vis visualisation)
{
   switch (visualisation)
     {
      case EMOTION_VIS_NONE:
         return NULL;
      case EMOTION_VIS_GOOM:
         return "goom";
      case EMOTION_VIS_LIBVISUAL_BUMPSCOPE:
         return "libvisual_bumpscope";
      case EMOTION_VIS_LIBVISUAL_CORONA:
         return "libvisual_corona";
      case EMOTION_VIS_LIBVISUAL_DANCING_PARTICLES:
         return "libvisual_dancingparticles";
      case EMOTION_VIS_LIBVISUAL_GDKPIXBUF:
         return "libvisual_gdkpixbuf";
      case EMOTION_VIS_LIBVISUAL_G_FORCE:
         return "libvisual_G-Force";
      case EMOTION_VIS_LIBVISUAL_GOOM:
         return "libvisual_goom";
      case EMOTION_VIS_LIBVISUAL_INFINITE:
         return "libvisual_infinite";
      case EMOTION_VIS_LIBVISUAL_JAKDAW:
         return "libvisual_jakdaw";
      case EMOTION_VIS_LIBVISUAL_JESS:
         return "libvisual_jess";
      case EMOTION_VIS_LIBVISUAL_LV_ANALYSER:
         return "libvisual_lv_analyzer";
      case EMOTION_VIS_LIBVISUAL_LV_FLOWER:
         return "libvisual_lv_flower";
      case EMOTION_VIS_LIBVISUAL_LV_GLTEST:
         return "libvisual_lv_gltest";
      case EMOTION_VIS_LIBVISUAL_LV_SCOPE:
         return "libvisual_lv_scope";
      case EMOTION_VIS_LIBVISUAL_MADSPIN:
         return "libvisual_madspin";
      case EMOTION_VIS_LIBVISUAL_NEBULUS:
         return "libvisual_nebulus";
      case EMOTION_VIS_LIBVISUAL_OINKSIE:
         return "libvisual_oinksie";
      case EMOTION_VIS_LIBVISUAL_PLASMA:
         return "libvisual_plazma";
      default:
         return "goom";
     }
}

Emotion_Gstreamer *
emotion_gstreamer_ref(Emotion_Gstreamer *ev)
{
  g_atomic_int_inc (&ev->ref_count);
  return ev;
}

void
emotion_gstreamer_unref(Emotion_Gstreamer *ev)
{
  if (g_atomic_int_dec_and_test(&ev->ref_count))
    {
       if (ev->subtitle)
         {
            eina_stringshare_del(ev->subtitle);
            ev->subtitle = NULL;
         }
       free(ev);
    }
}

static Eina_Bool
em_file_open(void *video,
             const char *file)
{
   Emotion_Gstreamer *ev = video;
   char *uri;
   char *suburi = NULL;

   if (!file) return EINA_FALSE;

   if (gst_uri_is_valid(file)) uri = strdup(file);
   else uri = gst_filename_to_uri(file, NULL);
   if (!uri) return EINA_FALSE;

   ev->shutdown = EINA_FALSE;
   ev->ready = EINA_FALSE;
   ev->live = EINA_FALSE;
   ev->buffering = EINA_FALSE;

   DBG("setting file to '%s'", uri);

   if (ev->subtitle)
     {
        if (gst_uri_is_valid(ev->subtitle)) suburi = strdup(ev->subtitle);
        else suburi = gst_filename_to_uri(ev->subtitle, NULL);
     }
   ev->pipeline = _create_pipeline(ev, ev->obj, uri, suburi);
   g_free(uri);

   if (!ev->pipeline)
     return EINA_FALSE;

   em_audio_channel_volume_set(ev, ev->volume);
   em_audio_channel_mute_set(ev, ev->audio_mute);

   ev->position = 0.0;

   return EINA_TRUE;
}

static void
em_file_close(void *video)
{
   Emotion_Gstreamer *ev = video;
   Eina_List *l;

   ev->shutdown = EINA_TRUE;

   if (ev->threads)
     {
        Ecore_Thread *t;

        EINA_LIST_FOREACH(ev->threads, l, t)
          {
             ecore_thread_cancel(t);
          }
     }

   if (ev->pipeline)
     {
       if (ev->audio_buffer_probe)
         {
            gst_pad_remove_probe(ev->audio_buffer_probe_pad, ev->audio_buffer_probe);
            gst_object_unref(ev->audio_buffer_probe_pad);
            ev->audio_buffer_probe_pad = NULL;
            ev->audio_buffer_probe = 0;
         }

       gst_element_set_state(ev->pipeline, GST_STATE_NULL);
       g_object_set(G_OBJECT(ev->vsink), "emotion-object", NULL, NULL);
       gst_object_unref(ev->pipeline);

       ev->pipeline = NULL;
       ev->vsink = NULL;
     }

   if (ev->metadata)
     {
        _free_metadata(ev->metadata);
        ev->metadata = NULL;
     }

   ev->ready = EINA_FALSE;
}

static void
em_del(void *video)
{
   Emotion_Gstreamer *ev = video;

   em_file_close(ev);

   emotion_gstreamer_unref(ev);
}

static void
em_play(void   *video,
        double  pos EINA_UNUSED)
{
   Emotion_Gstreamer *ev = video;

   if (!ev->pipeline) return;

   if (ev->ready && !ev->buffering)
     gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);
   ev->play = EINA_TRUE;
}

static void
em_stop(void *video)
{
   Emotion_Gstreamer *ev = video;

   if (!ev->pipeline) return;

   if (ev->ready)
     gst_element_set_state(ev->pipeline, GST_STATE_PAUSED);
   ev->play = EINA_FALSE;
}

static void
em_size_get(void  *video,
            int   *width,
            int   *height)
{
   Emotion_Gstreamer *ev = video;
   gint cur;
   GstPad *pad;
   GstCaps *caps;
   GstVideoInfo info;

   if (width) *width = 0;
   if (height) *height = 0;

   if (!ev->ready)
     return;

   g_object_get(ev->pipeline, "current-video", &cur, NULL);
   g_signal_emit_by_name (ev->pipeline, "get-video-pad", cur, &pad);
   if (!pad)
     return;

   caps = gst_pad_get_current_caps(pad);
   gst_object_unref(pad);
   if (!caps)
     return;

   gst_video_info_from_caps (&info, caps);
   if (width) *width = info.width;
   if (height) *height = info.height;
   gst_caps_unref(caps);
}

static void
em_pos_set(void   *video,
           double  pos)
{
   Emotion_Gstreamer *ev = video;

   if (!ev->ready) return;

   gst_element_seek(ev->pipeline, 1.0,
                          GST_FORMAT_TIME,
                          GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH,
                          GST_SEEK_TYPE_SET,
                          (gint64)(pos * (double)GST_SECOND),
                          GST_SEEK_TYPE_NONE, -1);
}

static double
em_len_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint64 val;
   gboolean ret;

   if (!ev->ready)
     return 0.0;

   ret = gst_element_query_duration(ev->pipeline, GST_FORMAT_TIME, &val);
   if (!ret || val == -1)
     return 0.0;

   return val / 1000000000.0;
}

static double
em_buffer_size_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   GstQuery *query;
   gboolean busy;
   gint percent;

   if (!ev->ready) return 0.0;

   query = gst_query_new_buffering(GST_FORMAT_DEFAULT);
   if (gst_element_query(ev->pipeline, query))
     gst_query_parse_buffering_percent(query, &busy, &percent);
   else
     percent = 100;

   gst_query_unref(query);
   return ((float)(percent)) / 100.0;
}

static Eina_Bool
_em_fps_get(Emotion_Gstreamer *ev, int *n, int *d)
{
   gint cur;
   GstPad *pad;
   GstCaps *caps;
   GstVideoInfo info;
   Eina_Bool ret = EINA_FALSE;

   if (n) *n = 0;
   if (d) *d = 1;

   if (!ev->ready)
     goto on_error;

   g_object_get(ev->pipeline, "current-video", &cur, NULL);
   g_signal_emit_by_name (ev->pipeline, "get-video-pad", cur, &pad);
   if (!pad)
     goto on_error;

   caps = gst_pad_get_current_caps(pad);
   gst_object_unref(pad);
   if (!caps)
     goto on_error;

   gst_video_info_from_caps (&info, caps);
   if (n) *n = info.fps_n;
   if (d) *d = info.fps_d;
   gst_caps_unref(caps);
   ret = EINA_TRUE;

 on_error:

   return ret;
}

static int
em_fps_num_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   int num;

   _em_fps_get(ev, &num, NULL);
   
   return num;
}

static int
em_fps_den_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   int den;

   _em_fps_get(ev, NULL, &den);
   
   return den;
}

static double
em_fps_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   int num, den;

   if (!ev->ready)
     return 0.0;

   _em_fps_get(ev, &num, &den);

   return (double)num / (double)den;
}

static double
em_pos_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint64 val;
   gboolean ret;

   if (!ev->ready) return 0.0;

   ret = gst_element_query_position(ev->pipeline, GST_FORMAT_TIME, &val);
   if (!ret || val == -1)
     return ev->position;

   ev->position = val / 1000000000.0;
   return ev->position;
}

static void
em_vis_set(void *video,
           Emotion_Vis vis)
{
   Emotion_Gstreamer *ev = video;

   ev->vis = vis;
}

static Emotion_Vis
em_vis_get(void *video)
{
   Emotion_Gstreamer *ev = video;

   return ev->vis;
}

static Eina_Bool
em_vis_supported(void *ef EINA_UNUSED, Emotion_Vis vis)
{
   const char *name;
   GstElementFactory *factory;

   if (vis == EMOTION_VIS_NONE)
     return EINA_TRUE;

   name = emotion_visualization_element_name_get(vis);
   if (!name)
     return EINA_FALSE;

   factory = gst_element_factory_find(name);
   if (!factory)
     return EINA_FALSE;

   gst_object_unref(factory);
   return EINA_TRUE;
}

static double
em_ratio_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint cur;
   GstPad *pad;
   GstCaps *caps;
   GstVideoInfo info;

   info.par_n = info.par_d = 1;

   if (!ev->ready)
     goto on_error;

   g_object_get(ev->pipeline, "current-video", &cur, NULL);
   g_signal_emit_by_name (ev->pipeline, "get-video-pad", cur, &pad);
   if (!pad)
     goto on_error;

   caps = gst_pad_get_current_caps(pad);
   gst_object_unref(pad);
   if (!caps)
     goto on_error;

   gst_video_info_from_caps (&info, caps);
   gst_caps_unref(caps);

 on_error:

   return (double)info.par_n / (double)info.par_d;
}

static int em_audio_channel_count(void *video);
static int em_video_channel_count(void *video);

static int
em_video_handled(void *video)
{
   Emotion_Gstreamer *ev = video;

   return em_video_channel_count(ev) > 0 ? 1 : 0;
}

static int
em_audio_handled(void *video)
{
   Emotion_Gstreamer *ev = video;

   return em_audio_channel_count(ev) > 0 ? 1 : 0;
}

static int
em_seekable(void *video)
{
   Emotion_Gstreamer *ev = video;
   GstQuery *query;
   int ret = 0;
   gboolean seekable;

   if (!ev->ready) return ret;

   query = gst_query_new_seeking(GST_FORMAT_TIME);
   if (!gst_element_query(ev->pipeline, query))
     goto on_error;

   gst_query_parse_seeking(query, NULL, &seekable, NULL, NULL);
   if (!seekable)
     goto on_error;

   ret = 1;

on_error:
   gst_query_unref(query);

   return ret;
}

static void
em_frame_done(void *video EINA_UNUSED)
{
}

static Emotion_Format
em_format_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint cur;
   GstPad *pad;
   GstCaps *caps;
   GstVideoInfo info;
   Emotion_Format format = EMOTION_FORMAT_NONE;

   if (!ev->ready)
     goto on_error;

   g_object_get(ev->pipeline, "current-video", &cur, NULL);
   g_signal_emit_by_name (ev->pipeline, "get-video-pad", cur, &pad);
   if (!pad)
     goto on_error;

   caps = gst_pad_get_current_caps(pad);
   gst_object_unref(pad);
   if (!caps)
     goto on_error;

   gst_video_info_from_caps (&info, caps);
   gst_caps_unref(caps);

   switch (info.finfo->format)
     {
      case GST_VIDEO_FORMAT_I420:
         return EMOTION_FORMAT_I420;
      case GST_VIDEO_FORMAT_YV12:
         return EMOTION_FORMAT_YV12;
      case GST_VIDEO_FORMAT_YUY2:
         return EMOTION_FORMAT_YUY2;
      case GST_VIDEO_FORMAT_ARGB:
         /* FIXME: This will be wrong for big endian archs */
         return EMOTION_FORMAT_BGRA;
      default:
         return EMOTION_FORMAT_NONE;
     }

 on_error:

   return format;
}

static void
em_video_data_size_get(void *video, int *w, int *h)
{
   em_size_get(video, w, h);
}

static int
em_yuv_rows_get(void           *video EINA_UNUSED,
                int             w EINA_UNUSED,
                int             h EINA_UNUSED,
                unsigned char **yrows EINA_UNUSED,
                unsigned char **urows EINA_UNUSED,
                unsigned char **vrows EINA_UNUSED)
{
   return 0;
}

static int
em_bgra_data_get(void *video EINA_UNUSED, unsigned char **bgra_data EINA_UNUSED)
{
   return 0;
}

static void
em_event_feed(void *video, int event)
{
   Emotion_Gstreamer *ev = video;
   GstNavigationCommand command;

   if (!ev->ready) return;

   switch (event)
     {
      case EMOTION_EVENT_MENU1:
	command = GST_NAVIGATION_COMMAND_MENU1;
	break;
      case EMOTION_EVENT_MENU2:
	command = GST_NAVIGATION_COMMAND_MENU2;
	break;
      case EMOTION_EVENT_MENU3:
	command = GST_NAVIGATION_COMMAND_MENU3;
	break;
      case EMOTION_EVENT_MENU4:
	command = GST_NAVIGATION_COMMAND_MENU4;
	break;
      case EMOTION_EVENT_MENU5:
	command = GST_NAVIGATION_COMMAND_MENU5;
	break;
      case EMOTION_EVENT_MENU6:
	command = GST_NAVIGATION_COMMAND_MENU6;
	break;
      case EMOTION_EVENT_MENU7:
	command = GST_NAVIGATION_COMMAND_MENU7;
	break;
      case EMOTION_EVENT_UP:
	command = GST_NAVIGATION_COMMAND_UP;
	break;
      case EMOTION_EVENT_DOWN:
	command = GST_NAVIGATION_COMMAND_DOWN;
	break;
      case EMOTION_EVENT_LEFT:
	command = GST_NAVIGATION_COMMAND_LEFT;
	break;
      case EMOTION_EVENT_RIGHT:
	command = GST_NAVIGATION_COMMAND_RIGHT;
	break;
      case EMOTION_EVENT_SELECT:
	command = GST_NAVIGATION_COMMAND_ACTIVATE;
	break;
      case EMOTION_EVENT_NEXT:
        /* FIXME */
	command = GST_NAVIGATION_COMMAND_RIGHT;
	break;
      case EMOTION_EVENT_PREV:
        /* FIXME */
	command = GST_NAVIGATION_COMMAND_LEFT;
	break;
      case EMOTION_EVENT_ANGLE_NEXT:
	command = GST_NAVIGATION_COMMAND_NEXT_ANGLE;
	break;
      case EMOTION_EVENT_ANGLE_PREV:
	command = GST_NAVIGATION_COMMAND_PREV_ANGLE;
	break;
      case EMOTION_EVENT_FORCE:
        /* FIXME */
	command = GST_NAVIGATION_COMMAND_ACTIVATE;
	break;
      case EMOTION_EVENT_0:
      case EMOTION_EVENT_1:
      case EMOTION_EVENT_2:
      case EMOTION_EVENT_3:
      case EMOTION_EVENT_4:
      case EMOTION_EVENT_5:
      case EMOTION_EVENT_6:
      case EMOTION_EVENT_7:
      case EMOTION_EVENT_8:
      case EMOTION_EVENT_9:
      case EMOTION_EVENT_10:
      default:
	return;
	break;
     }

  gst_navigation_send_command (GST_NAVIGATION (ev->pipeline), command);
}

static void
em_event_mouse_button_feed(void *video, int button, int x, int y)
{
   Emotion_Gstreamer *ev = video;

   if (!ev->ready) return;

   /* FIXME */
   gst_navigation_send_mouse_event (GST_NAVIGATION (ev->pipeline), "mouse-button-press", button, x, y);
   gst_navigation_send_mouse_event (GST_NAVIGATION (ev->pipeline), "mouse-button-release", button, x, y);
}

static void
em_event_mouse_move_feed(void *video, int x, int y)
{
   Emotion_Gstreamer *ev = video;

   if (!ev->ready) return;

   gst_navigation_send_mouse_event (GST_NAVIGATION (ev->pipeline), "mouse-move", 0, x, y);
}

/* Video channels */
static int
em_video_channel_count(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint n;

   if (!ev->ready) return 0;

   g_object_get(ev->pipeline, "n-video", &n, NULL);

   return n;
}

static void
em_video_channel_set(void *video,
                     int   channel)
{
   Emotion_Gstreamer *ev = video;

   if (!ev->ready) return;

   if (channel < 0) channel = -1;

   g_object_set (ev->pipeline, "current-video", channel, NULL);
}

static int
em_video_channel_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint cur;

   if (!ev->ready) return -1;

   g_object_get(ev->pipeline, "current-video", &cur, NULL);

   return cur;
}

static void
em_video_subtitle_file_set(void *video,
                           const char *filepath)
{
   Emotion_Gstreamer *ev = video;

   eina_stringshare_replace(&(ev->subtitle), filepath);
}

static const char *
em_video_subtitle_file_get(void *video)
{
   Emotion_Gstreamer *ev = video;

   return ev->subtitle;
}

static const char *
em_video_channel_name_get(void *video EINA_UNUSED,
                          int   channel EINA_UNUSED)
{
   return NULL;
}

static void
em_video_channel_mute_set(void *video,
                          int   mute)
{
   Emotion_Gstreamer *ev = video;

   ev->video_mute = mute;
}

static int
em_video_channel_mute_get(void *video)
{
   Emotion_Gstreamer *ev = video;

   return ev->video_mute;
}

/* Audio channels */

static int
em_audio_channel_count(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint n;

   if (!ev->ready) return 0;

   g_object_get(ev->pipeline, "n-audio", &n, NULL);

   return n;
}

static void
em_audio_channel_set(void *video,
                     int   channel)
{
   Emotion_Gstreamer *ev = video;

   if (!ev->ready) return;

   if (channel < 0) channel = -1;

   g_object_set (ev->pipeline, "current-audio", channel, NULL);
}

static int
em_audio_channel_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint cur;

   if (!ev->ready) return -1;

   g_object_get(ev->pipeline, "current-audio", &cur, NULL);

   return cur;
}

static const char *
em_audio_channel_name_get(void *video EINA_UNUSED,
                          int   channel EINA_UNUSED)
{
   return NULL;
}

static void
em_audio_channel_mute_set(void *video,
                          int   mute)
{
   Emotion_Gstreamer *ev = video;

   ev->audio_mute = !!mute;

   if (!ev->pipeline) return;

   g_object_set(G_OBJECT(ev->pipeline), "mute", !!mute, NULL);
}

static int
em_audio_channel_mute_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   gboolean mute;

   if (!ev->pipeline)
     return ev->audio_mute;

   g_object_get(ev->pipeline, "mute", &mute, NULL);

   return !!mute;
}

static void
em_audio_channel_volume_set(void  *video,
                            double vol)
{
   Emotion_Gstreamer *ev = video;

   if (vol < 0.0)
     vol = 0.0;
   ev->volume = vol;

   if (!ev->pipeline) return;

   g_object_set(G_OBJECT(ev->pipeline), "volume", vol, NULL);
}

static double
em_audio_channel_volume_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   gdouble vol;

   if (!ev->pipeline)
     return ev->volume;

   g_object_get(ev->pipeline, "volume", &vol, NULL);

   return vol;
}

/* spu stuff */

static int
em_spu_channel_count(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint n;

   if (!ev->ready) return 0;

   g_object_get(ev->pipeline, "n-text", &n, NULL);

   return n;
}

static void
em_spu_channel_set(void *video, int channel)
{
   Emotion_Gstreamer *ev = video;

   if (!ev->ready) return;

   if (channel < 0) channel = -1;

   g_object_set (ev->pipeline, "current-text", channel, NULL);
}

static int
em_spu_channel_get(void *video)
{
   Emotion_Gstreamer *ev = video;
   gint cur;

   if (!ev->ready) return -1;

   g_object_get(ev->pipeline, "current-text", &cur, NULL);

   return cur;
}

static const char *
em_spu_channel_name_get(void *video EINA_UNUSED, int channel EINA_UNUSED)
{
   return NULL;
}

static void
em_spu_channel_mute_set(void *video EINA_UNUSED, int mute EINA_UNUSED)
{
}

static int
em_spu_channel_mute_get(void *video EINA_UNUSED)
{
   return 0;
}

static int
em_chapter_count(void *video EINA_UNUSED)
{
   return 0;
}

static void
em_chapter_set(void *video EINA_UNUSED, int chapter EINA_UNUSED)
{
}

static int
em_chapter_get(void *video EINA_UNUSED)
{
   return 0;
}

static const char *
em_chapter_name_get(void *video EINA_UNUSED, int chapter EINA_UNUSED)
{
   return NULL;
}

static void
em_speed_set(void *video EINA_UNUSED, double speed EINA_UNUSED)
{
}

static double
em_speed_get(void *video EINA_UNUSED)
{
   return 1.0;
}

static int
em_eject(void *video EINA_UNUSED)
{
   return 1;
}

static const char *
em_meta_get(void *video, int meta)
{
   Emotion_Gstreamer *ev = video;
   const char *str = NULL;

   if (!ev->metadata) return NULL;

   switch (meta)
     {
      case META_TRACK_TITLE:
         str = ev->metadata->title;
         break;
      case META_TRACK_ARTIST:
         str = ev->metadata->artist;
         break;
      case  META_TRACK_ALBUM:
         str = ev->metadata->album;
         break;
      case META_TRACK_YEAR:
         str = ev->metadata->year;
         break;
      case META_TRACK_GENRE:
         str = ev->metadata->genre;
         break;
      case META_TRACK_COMMENT:
         str = ev->metadata->comment;
         break;
      case META_TRACK_DISCID:
         str = ev->metadata->disc_id;
         break;
      default:
         break;
     }

   return str;
}

static void *
em_add(const Emotion_Engine *api,
       Evas_Object *obj,
       const Emotion_Module_Options *opt EINA_UNUSED)
{
   Emotion_Gstreamer *ev;

   ev = calloc(1, sizeof(Emotion_Gstreamer));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ev, NULL);

   ev->api = api;
   ev->obj = obj;

   ev->ref_count = 1;

   /* Default values */
   ev->vis = EMOTION_VIS_NONE;
   ev->volume = 0.8;
   ev->ready = EINA_FALSE;
   ev->shutdown = EINA_FALSE;
   ev->threads = NULL;

   return ev;
}

static const Emotion_Engine em_engine =
{
   EMOTION_ENGINE_API_VERSION,
   EMOTION_ENGINE_PRIORITY_DEFAULT,
   "gstreamer1",
   em_add, /* add */
   em_del, /* del */
   em_file_open, /* file_open */
   em_file_close, /* file_close */
   em_play, /* play */
   em_stop, /* stop */
   em_size_get, /* size_get */
   em_pos_set, /* pos_set */
   em_len_get, /* len_get */
   em_buffer_size_get, /* buffer_size_get */
   em_fps_num_get, /* fps_num_get */
   em_fps_den_get, /* fps_den_get */
   em_fps_get, /* fps_get */
   em_pos_get, /* pos_get */
   em_vis_set, /* vis_set */
   em_vis_get, /* vis_get */
   em_vis_supported, /* vis_supported */
   em_ratio_get, /* ratio_get */
   em_video_handled, /* video_handled */
   em_audio_handled, /* audio_handled */
   em_seekable, /* seekable */
   em_frame_done, /* frame_done */
   em_format_get, /* format_get */
   em_video_data_size_get, /* video_data_size_get */
   em_yuv_rows_get, /* yuv_rows_get */
   em_bgra_data_get, /* bgra_data_get */
   em_event_feed, /* event_feed */
   em_event_mouse_button_feed, /* event_mouse_button_feed */
   em_event_mouse_move_feed, /* event_mouse_move_feed */
   em_video_channel_count, /* video_channel_count */
   em_video_channel_set, /* video_channel_set */
   em_video_channel_get, /* video_channel_get */
   em_video_subtitle_file_set, /* video_subtitle_file_set */
   em_video_subtitle_file_get, /* video_subtitle_file_get */
   em_video_channel_name_get, /* video_channel_name_get */
   em_video_channel_mute_set, /* video_channel_mute_set */
   em_video_channel_mute_get, /* video_channel_mute_get */
   em_audio_channel_count, /* audio_channel_count */
   em_audio_channel_set, /* audio_channel_set */
   em_audio_channel_get, /* audio_channel_get */
   em_audio_channel_name_get, /* audio_channel_name_get */
   em_audio_channel_mute_set, /* audio_channel_mute_set */
   em_audio_channel_mute_get, /* audio_channel_mute_get */
   em_audio_channel_volume_set, /* audio_channel_volume_set */
   em_audio_channel_volume_get, /* audio_channel_volume_get */
   em_spu_channel_count, /* spu_channel_count */
   em_spu_channel_set, /* spu_channel_set */
   em_spu_channel_get, /* spu_channel_get */
   em_spu_channel_name_get, /* spu_channel_name_get */
   em_spu_channel_mute_set, /* spu_channel_mute_set */
   em_spu_channel_mute_get, /* spu_channel_mute_get */
   em_chapter_count, /* chapter_count */
   em_chapter_set, /* chapter_set */
   em_chapter_get, /* chapter_get */
   em_chapter_name_get, /* chapter_name_get */
   em_speed_set, /* speed_set */
   em_speed_get, /* speed_get */
   em_eject, /* eject */
   em_meta_get, /* meta_get */
   NULL, /* priority_set */
   NULL /* priority_get */
};

Eina_Bool
gstreamer_module_init(void)
{
   GError *error;

   if (_emotion_init_count > 0)
     {
        _emotion_pending_ecore_begin();
        return EINA_TRUE;
     }

   if (getenv("EMOTION_FPS_DEBUG")) debug_fps = EINA_TRUE;

   eina_threads_init();
   eina_log_threads_enable();
   _emotion_gstreamer_log_domain = eina_log_domain_register
     ("emotion-gstreamer", EINA_COLOR_LIGHTCYAN);
   if (_emotion_gstreamer_log_domain < 0)
     {
        EINA_LOG_CRIT("Could not register log domain 'emotion-gstreamer'");
        return EINA_FALSE;
     }

   if (!gst_init_check(0, NULL, &error))
     {
        EINA_LOG_CRIT("Could not init GStreamer");
        goto error_gst_init;
     }

   if (gst_plugin_register_static(GST_VERSION_MAJOR, GST_VERSION_MINOR,
                                  "emotion-sink",
                                  "video sink plugin for Emotion",
                                  gstreamer_plugin_init,
                                  VERSION,
                                  "LGPL",
                                  "Enlightenment",
                                  PACKAGE,
                                  "http://www.enlightenment.org/") == FALSE)
     {
        EINA_LOG_CRIT("Could not load static gstreamer video sink for Emotion.");
        goto error_gst_plugin;
     }

   if (!_emotion_module_register(&em_engine))
     {
        ERR("Could not register module %p", &em_engine);
        goto error_register;
     }

   _emotion_init_count = 1;
   return EINA_TRUE;

 error_register:
 error_gst_plugin:

   gst_deinit();

 error_gst_init:
   eina_log_domain_unregister(_emotion_gstreamer_log_domain);
   _emotion_gstreamer_log_domain = -1;

   return EINA_FALSE;
}

void
gstreamer_module_shutdown(void)
{
   if (_emotion_init_count > 1)
     {
        _emotion_init_count--;
        return;
     }
   else if (_emotion_init_count == 0)
     {
        EINA_LOG_ERR("too many gstreamer_module_shutdown()");
        return;
     }
   _emotion_init_count = 0;

   _emotion_module_unregister(&em_engine);

   eina_log_domain_unregister(_emotion_gstreamer_log_domain);
   _emotion_gstreamer_log_domain = -1;

   gst_deinit();
}

#ifndef EMOTION_STATIC_BUILD_GSTREAMER

EINA_MODULE_INIT(gstreamer_module_init);
EINA_MODULE_SHUTDOWN(gstreamer_module_shutdown);

#endif

static void
_for_each_tag(GstTagList const* list,
                    gchar const* tag,
                    void *data)
{
   Emotion_Gstreamer *ev;
   int i;
   int count;


   ev = (Emotion_Gstreamer*)data;

   if (!ev || !ev->metadata) return;

   count = gst_tag_list_get_tag_size(list, tag);

   for (i = 0; i < count; i++)
     {
        if (!strcmp(tag, GST_TAG_TITLE))
          {
             char *str;
             g_free(ev->metadata->title);
             if (gst_tag_list_get_string(list, GST_TAG_TITLE, &str))
               ev->metadata->title = str;
             else
               ev->metadata->title = NULL;
             break;
          }
        if (!strcmp(tag, GST_TAG_ALBUM))
          {
             gchar *str;
             g_free(ev->metadata->album);
             if (gst_tag_list_get_string(list, GST_TAG_ALBUM, &str))
               ev->metadata->album = str;
             else
               ev->metadata->album = NULL;
             break;
          }
        if (!strcmp(tag, GST_TAG_ARTIST))
          {
             gchar *str;
             g_free(ev->metadata->artist);
             if (gst_tag_list_get_string(list, GST_TAG_ARTIST, &str))
               ev->metadata->artist = str;
             else
               ev->metadata->artist = NULL;
             break;
          }
        if (!strcmp(tag, GST_TAG_GENRE))
          {
             gchar *str;
             g_free(ev->metadata->genre);
             if (gst_tag_list_get_string(list, GST_TAG_GENRE, &str))
               ev->metadata->genre = str;
             else
               ev->metadata->genre = NULL;
             break;
          }
        if (!strcmp(tag, GST_TAG_COMMENT))
          {
             gchar *str;
             g_free(ev->metadata->comment);
             if (gst_tag_list_get_string(list, GST_TAG_COMMENT, &str))
               ev->metadata->comment = str;
             else
               ev->metadata->comment = NULL;
             break;
          }
        if (!strcmp(tag, GST_TAG_DATE))
          {
             gchar *str;
             const GValue *date;
             g_free(ev->metadata->year);
             date = gst_tag_list_get_value_index(list, GST_TAG_DATE, 0);
             if (date)
               str = g_strdup_value_contents(date);
             else
               str = NULL;
             ev->metadata->year = str;
             break;
          }

        if (!strcmp(tag, GST_TAG_DATE_TIME))
          {
             gchar *str;
             const GValue *date;
             g_free(ev->metadata->year);
             date = gst_tag_list_get_value_index(list, GST_TAG_DATE_TIME, 0);
             if (date)
               str = g_strdup_value_contents(date);
             else
               str = NULL;
             ev->metadata->year = str;
             break;
          }

        if (!strcmp(tag, GST_TAG_TRACK_NUMBER))
          {
             gchar *str;
             const GValue *track;
             g_free(ev->metadata->count);
             track = gst_tag_list_get_value_index(list, GST_TAG_TRACK_NUMBER, 0);
             if (track)
               str = g_strdup_value_contents(track);
             else
               str = NULL;
             ev->metadata->count = str;
             break;
          }

        if (!strcmp(tag, GST_TAG_CDDA_CDDB_DISCID))
          {
             gchar *str;
             const GValue *discid;
             g_free(ev->metadata->disc_id);
             discid = gst_tag_list_get_value_index(list, GST_TAG_CDDA_CDDB_DISCID, 0);
             if (discid)
               str = g_strdup_value_contents(discid);
             else
               str = NULL;
             ev->metadata->disc_id = str;
             break;
          }
     }

}

static void
_free_metadata(Emotion_Gstreamer_Metadata *m)
{
  if (!m) return;

  g_free(m->title);
  g_free(m->album);
  g_free(m->artist);
  g_free(m->genre);
  g_free(m->comment);
  g_free(m->year);
  g_free(m->count);
  g_free(m->disc_id);

  free(m);
}

static void
audio_buffer_probe_main(void *data)
{
   Emotion_Gstreamer *ev = data;

   if (!ev->shutdown)
     _emotion_frame_new(ev->obj);

   g_atomic_int_set(&ev->audio_buffer_probe_pending, 0);

   emotion_gstreamer_unref(ev);
   _emotion_pending_ecore_end();
}

static GstPadProbeReturn
audio_buffer_probe(GstPad *pad EINA_UNUSED, GstPadProbeInfo *info EINA_UNUSED, gpointer user_data)
{
   Emotion_Gstreamer *ev = user_data;

   /* Don't call too many of these */
   if (!g_atomic_int_compare_and_exchange(&ev->audio_buffer_probe_pending, 0, 1))
     return GST_PAD_PROBE_OK;

   _emotion_pending_ecore_begin();
   ecore_main_loop_thread_safe_call_async(audio_buffer_probe_main, emotion_gstreamer_ref(ev));

   return GST_PAD_PROBE_OK;
}

static void
_bus_main_handler(void *data)
{
   Emotion_Gstreamer_Message *send;
   Emotion_Gstreamer *ev;
   GstMessage              *msg;

   send = data;
   ev = send->ev;
   msg = send->msg;

   /* Just exit immediately if we're shutting down */
   if (ev->shutdown)
     {
        emotion_gstreamer_message_free(send);
        _emotion_pending_ecore_end();
        return;
     }

   switch (GST_MESSAGE_TYPE(msg))
     {
      case GST_MESSAGE_EOS:
         ev->play = EINA_FALSE;
         _emotion_decode_stop(ev->obj);
         _emotion_playback_finished(ev->obj);
         break;
      case GST_MESSAGE_TAG:
        {
           GstTagList *new_tags;
           gst_message_parse_tag(msg, &new_tags);
           if (new_tags)
             {
                gst_tag_list_foreach(new_tags,
                                     (GstTagForeachFunc)_for_each_tag,
                                     ev);
                gst_tag_list_free(new_tags);
             }
           break;
        }
      case GST_MESSAGE_ASYNC_DONE:
         _emotion_seek_done(ev->obj);
         break;
      case GST_MESSAGE_STATE_CHANGED:
        {
           GstState old_state, new_state;

           gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
           INF("Element %s changed state from %s to %s.",
               GST_OBJECT_NAME(msg->src),
               gst_element_state_get_name(old_state),
               gst_element_state_get_name(new_state));

           if (GST_MESSAGE_SRC(msg) == GST_OBJECT(ev->pipeline) && new_state >= GST_STATE_PAUSED && !ev->ready)
             {
                gint n_audio, n_video;

                ev->ready = EINA_TRUE;
                
                g_object_get(G_OBJECT(ev->pipeline),
                  "n-audio", &n_audio,
                  "n-video", &n_video,
                  NULL);

                if (n_audio == 0 && n_video == 0)
                  ERR("No audio nor video stream found");
                
                if (n_audio > 0 && n_video == 0)
                  {
                     GstElement *vis = NULL;
                     gint flags;
                     const char *vis_name;
                     
                     if ((vis_name = emotion_visualization_element_name_get(ev->vis)))
                       {                       
                          vis = gst_element_factory_make(vis_name, "vis");
                          g_object_set(G_OBJECT(ev->pipeline), "vis-plugin", vis, NULL);
                          g_object_get(G_OBJECT(ev->pipeline), "flags", &flags, NULL);
                          flags |= GST_PLAY_FLAG_VIS;
                          g_object_set(G_OBJECT(ev->pipeline), "flags", flags, NULL);
                       }
                     else
                       {
                          GstElement *audio_sink;

                          g_object_get(ev->pipeline, "audio-sink", &audio_sink, NULL);
                          ev->audio_buffer_probe_pad = gst_element_get_static_pad(audio_sink, "sink");
                          ev->audio_buffer_probe = gst_pad_add_probe(ev->audio_buffer_probe_pad,
                                                                     GST_PAD_PROBE_TYPE_BUFFER | GST_PAD_PROBE_TYPE_BUFFER_LIST,
                                                                     audio_buffer_probe,
                                                                     ev,
                                                                     NULL);
                          gst_object_unref(audio_sink);
                       }
                  }

                if (n_audio > 0 || n_video > 0)
                  {
                     /** NOTE: you need to set: GST_DEBUG_DUMP_DOT_DIR=/tmp EMOTION_ENGINE=gstreamer to save the $EMOTION_GSTREAMER_DOT file in '/tmp' */
                     /** then call dot -Tpng -oemotion_pipeline.png /tmp/$TIMESTAMP-$EMOTION_GSTREAMER_DOT.dot */
                     
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
                     if (getuid() == geteuid())
#endif
                       {
                          if (getenv("EMOTION_GSTREAMER_DOT"))
                            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(ev->pipeline),
                                                              GST_DEBUG_GRAPH_SHOW_ALL,
                                                              getenv("EMOTION_GSTREAMER_DOT"));
                       }
                       
                     _emotion_open_done(ev->obj);
                     _emotion_playback_started(ev->obj);
                  }
             }
           break;
        }
      case GST_MESSAGE_ERROR:
        {
           GError *err = NULL;
           gchar *name, *debug = NULL;
           
           name = gst_object_get_path_string (msg->src);
           gst_message_parse_error (msg, &err, &debug);
           
           ERR("ERROR: from element %s: %s\nAdditional debug info:\n%s", name, err->message, debug);
           
           g_error_free (err);
           g_free (debug);
           g_free (name);

           gst_element_set_state(ev->pipeline, GST_STATE_NULL);

           ev->play = EINA_FALSE;
           _emotion_decode_stop(ev->obj);
           _emotion_playback_finished(ev->obj);

           break;
        }
      case GST_MESSAGE_WARNING:
        {
           GError *err = NULL;
           gchar *name, *debug = NULL;
           
           name = gst_object_get_path_string (msg->src);
           gst_message_parse_warning (msg, &err, &debug);
           
           WRN("WARNING: from element %s: %s\nAdditional debug info:\n%s", name, err->message, debug);
           
           g_error_free (err);
           g_free (debug);
           g_free (name);

           break;
        }
      case GST_MESSAGE_BUFFERING:
        {
           gint percent = 0;
           
           /* If the stream is live, we do not care about buffering. */
           if (ev->live)
             {
                ev->buffering = FALSE;
                break;
             }
           
           gst_message_parse_buffering (msg, &percent);

           /* Wait until buffering is complete before start/resume playing */
           if (percent < 100)
             gst_element_set_state (ev->pipeline, GST_STATE_PAUSED);
           else if (ev->play)
             gst_element_set_state (ev->pipeline, GST_STATE_PLAYING);

           ev->buffering = (percent < 100);

           break;
        }
      case GST_MESSAGE_CLOCK_LOST:
        {
           gst_element_set_state (ev->pipeline, GST_STATE_PAUSED);
           gst_element_set_state (ev->pipeline, GST_STATE_PLAYING);
           break;
        }
      default:
         break;
     }

   emotion_gstreamer_message_free(send);
   _emotion_pending_ecore_end();
}

static GstBusSyncReply
_bus_sync_handler(GstBus *bus EINA_UNUSED, GstMessage *msg, gpointer data)
{
   Emotion_Gstreamer *ev = data;
   Emotion_Gstreamer_Message *send;

   INF("Message %s from %s",
       GST_MESSAGE_TYPE_NAME(msg),
       GST_MESSAGE_SRC_NAME(msg));
   
   send = emotion_gstreamer_message_alloc(ev, msg);
   
   if (send)
     {
        _emotion_pending_ecore_begin();
        ecore_main_loop_thread_safe_call_async(_bus_main_handler, send);
     }

   gst_message_unref(msg);

   return GST_BUS_DROP;
}

static void
_emotion_gstreamer_pause(void *data, Ecore_Thread *thread)
{
   Emotion_Gstreamer *ev = data;
   gboolean res;

   if (ecore_thread_check(thread) || !ev->pipeline) return;

   gst_element_set_state(ev->pipeline, GST_STATE_PAUSED);
   res = gst_element_get_state(ev->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
   if (res == GST_STATE_CHANGE_NO_PREROLL)
     {
        ev->live = EINA_TRUE;
        gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);
	gst_element_get_state(ev->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
     }
}

static void
_emotion_gstreamer_cancel(void *data, Ecore_Thread *thread)
{
   Emotion_Gstreamer *ev = data;

   ev->threads = eina_list_remove(ev->threads, thread);

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        if (getenv("EMOTION_GSTREAMER_DOT")) GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(ev->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, getenv("EMOTION_GSTREAMER_DOT"));
     }

   emotion_gstreamer_unref(ev);
}

static void
_emotion_gstreamer_end(void *data, Ecore_Thread *thread)
{
   Emotion_Gstreamer *ev = data;

   ev->threads = eina_list_remove(ev->threads, thread);

   if (ev->play && !ev->buffering)
     {
        gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);
     }

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        if (getenv("EMOTION_GSTREAMER_DOT")) GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(ev->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, getenv("EMOTION_GSTREAMER_DOT"));
     }

   emotion_gstreamer_unref(ev);
}

static GstElement *
_create_pipeline(Emotion_Gstreamer *ev,
                 Evas_Object *o,
                 const char *uri,
                 const char *suburi)
{
   GstElement *playbin;
   GstElement *vsink;
   GstBus *bus;
   int flags;

   if (!uri)
     return NULL;

   playbin = gst_element_factory_make("playbin", "playbin");
   if (!playbin)
     {
        ERR("Unable to create 'playbin' GstElement.");
        return NULL;
     }

   vsink = gst_element_factory_make("emotion-sink", "sink");
   if (!vsink)
     {
        ERR("Unable to create 'emotion-sink' GstElement.");
        goto unref_pipeline;
     }

   g_object_set(G_OBJECT(vsink), "emotion-object", o, NULL);

   g_object_get(G_OBJECT(playbin), "flags", &flags, NULL);
   g_object_set(G_OBJECT(playbin), "flags", flags | GST_PLAY_FLAG_DOWNLOAD, NULL);
   g_object_set(G_OBJECT(playbin), "video-sink", vsink, NULL);
   g_object_set(G_OBJECT(playbin), "uri", uri, NULL);
   if (suburi)
     {
        g_object_set(G_OBJECT(playbin), "suburi", suburi, NULL);
        g_object_set(G_OBJECT(playbin), "subtitle-font-desc", "Sans, 10", NULL);
     }

   bus = gst_element_get_bus(playbin);
   gst_bus_set_sync_handler(bus, _bus_sync_handler, ev, NULL);
   gst_object_unref(bus);

   ev->pipeline = playbin;
   ev->vsink = vsink;

   ev->metadata = calloc(1, sizeof(Emotion_Gstreamer_Metadata));

   ev->threads = eina_list_append(ev->threads,
                                  ecore_thread_run(_emotion_gstreamer_pause,
                                                   _emotion_gstreamer_end,
                                                   _emotion_gstreamer_cancel,
                                                   emotion_gstreamer_ref(ev)));

   /** NOTE: you need to set: GST_DEBUG_DUMP_DOT_DIR=/tmp EMOTION_ENGINE=gstreamer to save the $EMOTION_GSTREAMER_DOT file in '/tmp' */
   /** then call dot -Tpng -oemotion_pipeline.png /tmp/$TIMESTAMP-$EMOTION_GSTREAMER_DOT.dot */
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        if (getenv("EMOTION_GSTREAMER_DOT")) GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(playbin), GST_DEBUG_GRAPH_SHOW_ALL, getenv("EMOTION_GSTREAMER_DOT"));
     }

   return playbin;

 unref_pipeline:
   gst_object_unref(vsink);
   gst_object_unref(playbin);
   return NULL;
}

