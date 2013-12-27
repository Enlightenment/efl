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

static GstBusSyncReply _eos_sync_fct(GstBus *bus,
				     GstMessage *message,
				     gpointer data);

/* Module interface */

static Emotion_Video_Stream *
emotion_video_stream_new(Emotion_Gstreamer_Video *ev)
{
   Emotion_Video_Stream *vstream;

   if (!ev) return NULL;

   vstream = (Emotion_Video_Stream *)calloc(1, sizeof(Emotion_Video_Stream));
   if (!vstream) return NULL;

   ev->video_streams = eina_list_append(ev->video_streams, vstream);
   return vstream;
}

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

static void
em_cleanup(Emotion_Gstreamer_Video *ev)
{
   Emotion_Audio_Stream *astream;
   Emotion_Video_Stream *vstream;

   if (ev->send)
     {
        emotion_gstreamer_buffer_free(ev->send);
        ev->send = NULL;
     }

   if (ev->eos_bus)
     {
        gst_object_unref(GST_OBJECT(ev->eos_bus));
        ev->eos_bus = NULL;
     }

   if (ev->metadata)
     {
        _free_metadata(ev->metadata);
        ev->metadata = NULL;
     }

   if (ev->last_buffer)
     {
        gst_buffer_unref(ev->last_buffer);
        ev->last_buffer = NULL;
     }

   if (!ev->stream)
     {
        evas_object_image_video_surface_set(emotion_object_image_get(ev->obj), NULL);
        ev->stream = EINA_TRUE;
     }

   if (ev->pipeline)
     {
       gstreamer_video_sink_new(ev, ev->obj, NULL);

       g_object_set(G_OBJECT(ev->esink), "ev", NULL, NULL);
       g_object_set(G_OBJECT(ev->esink), "evas-object", NULL, NULL);
       gst_element_set_state(ev->pipeline, GST_STATE_NULL);
       gst_object_unref(ev->pipeline);

       ev->pipeline = NULL;
       ev->sink = NULL;

       ev->src_width = 0;
       ev->src_height = 0;
     }

   EINA_LIST_FREE(ev->audio_streams, astream)
     free(astream);
   EINA_LIST_FREE(ev->video_streams, vstream)
     free(vstream);
}

static void
em_del(void *video)
{
   Emotion_Gstreamer_Video *ev = video;

   if (ev->threads)
     {
        Ecore_Thread *t;

        EINA_LIST_FREE(ev->threads, t)
          ecore_thread_cancel(t);

        ev->delete_me = EINA_TRUE;
        return;
     }

   if (ev->in != ev->out)
     {
        ev->delete_me = EINA_TRUE;
        return;
     }

   em_cleanup(ev);

   free(ev);
}

static Eina_Bool
em_file_open(void *video,
             const char   *file)
{
   Emotion_Gstreamer_Video *ev = video;
   Eina_Strbuf *sbuf = NULL;
   const char *uri;

   if (!file) return EINA_FALSE;
   if (strstr(file, "://") == NULL)
     {
        sbuf = eina_strbuf_new();
        eina_strbuf_append(sbuf, "file://");
        if (strncmp(file, "./", 2) == 0)
          file += 2;
	if (strstr(file, ":/") != NULL)
	  { /* We absolutely need file:///C:/ under Windows, so adding it here */
             eina_strbuf_append(sbuf, "/");
	  }
	else if (*file != '/')
          {
             char tmp[PATH_MAX];

             if (getcwd(tmp, PATH_MAX))
               {
                  eina_strbuf_append(sbuf, tmp);
                  eina_strbuf_append(sbuf, "/");
               }
          }
        eina_strbuf_append(sbuf, file);
     }

   ev->play_started = 0;
   ev->pipeline_parsed = 0;

   uri = sbuf ? eina_strbuf_string_get(sbuf) : file;
   DBG("setting file to '%s'", uri);
   ev->pipeline = gstreamer_video_sink_new(ev, ev->obj, uri);
   if (sbuf) eina_strbuf_free(sbuf);

   if (!ev->pipeline)
     return EINA_FALSE;

   ev->eos_bus = gst_pipeline_get_bus(GST_PIPELINE(ev->pipeline));
   if (!ev->eos_bus)
     {
        ERR("could not get the bus");
        return EINA_FALSE;
     }

   gst_bus_set_sync_handler(ev->eos_bus, _eos_sync_fct, ev, NULL);

   ev->position = 0.0;

   return 1;
}

static void
em_file_close(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev)
     return;

   if (ev->threads)
     {
        Ecore_Thread *t;

        EINA_LIST_FREE(ev->threads, t)
          ecore_thread_cancel(t);
     }

   em_cleanup(ev);

   ev->pipeline_parsed = EINA_FALSE;
   ev->play_started = 0;
}

static void
em_play(void   *video,
        double  pos EINA_UNUSED)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev->pipeline) return;

   if (ev->pipeline_parsed)
     gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);
   ev->play = 1;
   ev->play_started = 1;
}

static void
em_stop(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!ev->pipeline) return;

   if (ev->pipeline_parsed)
     gst_element_set_state(ev->pipeline, GST_STATE_PAUSED);
   ev->play = 0;
}

static void
em_size_get(void  *video,
            int   *width,
            int   *height)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Stream      *vstream;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!_emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE))
     goto on_error;

   vstream = eina_list_nth(ev->video_streams, ev->video_stream_nbr - 1);
   if (vstream)
     {
        if (width) *width = vstream->info.width;
        if (height) *height = vstream->info.height;

        return;
     }

 on_error:
   if (width) *width = 0;
   if (height) *height = 0;
}

static void
em_pos_set(void   *video,
           double  pos)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!ev->pipeline) return;

   if (ev->play)
     gst_element_set_state(ev->pipeline, GST_STATE_PAUSED);

   gst_element_seek(ev->pipeline, 1.0,
                          GST_FORMAT_TIME,
                          GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH,
                          GST_SEEK_TYPE_SET,
                          (gint64)(pos * (double)GST_SECOND),
                          GST_SEEK_TYPE_NONE, -1);

   if (ev->play)
     gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);
}

static double
em_len_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Stream *vstream;
   Emotion_Audio_Stream *astream;
   Eina_List *l;
   gint64 val;
   gboolean ret;

   ev = video;

   if (!ev->pipeline) return 0.0;

   ret = gst_element_query_duration(ev->pipeline, GST_FORMAT_TIME, &val);
   if (!ret)
     goto fallback;

   if (val <= 0.0)
     goto fallback;

   return val / 1000000000.0;

 fallback:
   if (!_emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE))
     return 0.0;

   EINA_LIST_FOREACH(ev->audio_streams, l, astream)
     if (astream->length_time >= 0)
       return astream->length_time;

   EINA_LIST_FOREACH(ev->video_streams, l, vstream)
     if (vstream->length_time >= 0)
       return vstream->length_time;

   return 0.0;
}

static double
em_buffer_size_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   GstQuery *query;
   gboolean busy;
   gint percent;

   ev = video;

   if (!ev->pipeline) return 0.0;

   query = gst_query_new_buffering(GST_FORMAT_DEFAULT);
   if (gst_element_query(ev->pipeline, query))
     gst_query_parse_buffering_percent(query, &busy, &percent);
   else
     percent = 100;

   gst_query_unref(query);
   return ((float)(percent)) / 100.0;
}

static int
em_fps_num_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Stream      *vstream;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!_emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE))
     return 0;

   vstream = eina_list_nth(ev->video_streams, ev->video_stream_nbr - 1);
   if (vstream)
     return vstream->info.fps_n;

   return 0;
}

static int
em_fps_den_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Stream      *vstream;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!_emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE))
     return 1;

   vstream = eina_list_nth(ev->video_streams, ev->video_stream_nbr - 1);
   if (vstream)
     return vstream->info.fps_d;

   return 1;
}

static double
em_fps_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Stream      *vstream;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!_emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE))
     return 0.0;

   vstream = eina_list_nth(ev->video_streams, ev->video_stream_nbr - 1);
   if (vstream)
     return (double)vstream->info.fps_n / (double)vstream->info.fps_d;

   return 0.0;
}

static double
em_pos_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   gint64 val;
   gboolean ret;

   ev = video;

   if (!ev->pipeline) return 0.0;

   ret = gst_element_query_position(ev->pipeline, GST_FORMAT_TIME, &val);
   if (!ret)
     return ev->position;

   ev->position = val / 1000000000.0;
   return ev->position;
}

static void
em_vis_set(void *video,
           Emotion_Vis vis)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   ev->vis = vis;
}

static Emotion_Vis
em_vis_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

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
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ev->ratio;
}

static int
em_video_handled(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE);

   if (!eina_list_count(ev->video_streams))
     return 0;

   return 1;
}

static int
em_audio_handled(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE);

   if (!eina_list_count(ev->audio_streams))
     return 0;

   return 1;
}

static int
em_seekable(void *video EINA_UNUSED)
{
   return 1;
}

static void
em_frame_done(void *video EINA_UNUSED)
{
}

static Emotion_Format
em_format_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Stream    *vstream;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!_emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE))
     return EMOTION_FORMAT_NONE;

   vstream = eina_list_nth(ev->video_streams, ev->video_stream_nbr - 1);
   if (vstream)
     {
        switch (vstream->info.finfo->format)
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
     }
   return EMOTION_FORMAT_NONE;
}

static void
em_video_data_size_get(void *video, int *w, int *h)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Stream    *vstream;

   ev = (Emotion_Gstreamer_Video *)video;

   if (ev->pipeline && (!ev->video_stream_nbr || !ev->video_streams))
     if (!_emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE))
       goto on_error;

   vstream = eina_list_nth(ev->video_streams, ev->video_stream_nbr - 1);
   if (vstream)
     {
        *w = vstream->info.width;
        *h = vstream->info.height;

        return;
     }

 on_error:
   *w = 0;
   *h = 0;
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
   Emotion_Gstreamer_Video *ev;
   GstNavigationCommand command;

   ev = (Emotion_Gstreamer_Video *)video;

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
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   /* FIXME */
   gst_navigation_send_mouse_event (GST_NAVIGATION (ev->pipeline), "mouse-button-press", button, x, y);
   gst_navigation_send_mouse_event (GST_NAVIGATION (ev->pipeline), "mouse-button-release", button, x, y);
}

static void
em_event_mouse_move_feed(void *video, int x, int y)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   gst_navigation_send_mouse_event (GST_NAVIGATION (ev->pipeline), "mouse-move", 0, x, y);
}

/* Video channels */
static int
em_video_channel_count(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE);

   return eina_list_count(ev->video_streams);
}

static void
em_video_channel_set(void *video,
                     int   channel)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE);

   if (channel < 0) channel = -1;

   if (ev->pipeline)
     g_object_set (ev->pipeline, "current-video", channel, NULL);
}

static int
em_video_channel_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE);

   return ev->video_stream_nbr;
}

static void
em_video_subtitle_file_set(void *video EINA_UNUSED,
                           const char *filepath EINA_UNUSED)
{
   DBG("video_subtitle_file_set not implemented for gstreamer yet.");
}

static const char *
em_video_subtitle_file_get(void *video EINA_UNUSED)
{
   DBG("video_subtitle_file_get not implemented for gstreamer yet.");
   return NULL;
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
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   ev->video_mute = mute;
}

static int
em_video_channel_mute_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ev->video_mute;
}

/* Audio channels */

static int
em_audio_channel_count(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE);

   return eina_list_count(ev->audio_streams);
}

static void
em_audio_channel_set(void *video,
                     int   channel)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE);

   if (channel < 0) channel = -1;

   if (ev->pipeline)
     g_object_set (ev->pipeline, "current-audio", channel, NULL);
}

static int
em_audio_channel_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE);

   return ev->audio_stream_nbr;
}

static const char *
em_audio_channel_name_get(void *video EINA_UNUSED,
                          int   channel EINA_UNUSED)
{
   return NULL;
}

#define GST_PLAY_FLAG_AUDIO (1 << 1)

static void
em_audio_channel_mute_set(void *video,
                          int   mute)
{
   /* NOTE: at first I wanted to completly shutdown the audio path on mute,
      but that's not possible as the audio sink could be the clock source
      for the pipeline (at least that's the case on some of the hardware
      I have been tested emotion on.
    */
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!ev->pipeline) return;

   ev->audio_mute = mute;

   g_object_set(G_OBJECT(ev->pipeline), "mute", !!mute, NULL);
}

static int
em_audio_channel_mute_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ev->audio_mute;
}

static void
em_audio_channel_volume_set(void  *video,
                            double vol)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!ev->pipeline) return;

   if (vol < 0.0)
     vol = 0.0;
   if (vol > 1.0)
     vol = 1.0;
   ev->volume = vol;
   g_object_set(G_OBJECT(ev->pipeline), "volume", vol, NULL);
}

static double
em_audio_channel_volume_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ev->volume;
}

/* spu stuff */

static int
em_spu_channel_count(void *video EINA_UNUSED)
{
   return 0;
}

static void
em_spu_channel_set(void *video EINA_UNUSED, int channel EINA_UNUSED)
{
}

static int
em_spu_channel_get(void *video EINA_UNUSED)
{
   return 1;
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
   Emotion_Gstreamer_Video *ev;
   const char *str = NULL;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!ev || !ev->metadata) return NULL;
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
   Emotion_Gstreamer_Video *ev;

   ev = calloc(1, sizeof(Emotion_Gstreamer_Video));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ev, NULL);

   ev->api = api;
   ev->obj = obj;

   /* Default values */
   ev->ratio = 1.0;
   ev->vis = EMOTION_VIS_NONE;
   ev->volume = 0.8;
   ev->play_started = 0;
   ev->delete_me = EINA_FALSE;
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
   Emotion_Gstreamer_Video *ev;
   int i;
   int count;


   ev = (Emotion_Gstreamer_Video*)data;

   if (!ev || !ev->metadata) return;

   /* FIXME: Should use the GStreamer tag merging functions */
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

static Eina_Bool
_video_size_get(GstElement *elem, int *width, int *height)
{
   GstIterator *itr = NULL;
   GstCaps *caps;
   GValue v = G_VALUE_INIT;
   GstPad *pad;
   Eina_Bool ret = EINA_FALSE;
   Eina_Bool done = EINA_FALSE;
   GstVideoInfo info;

   itr = gst_element_iterate_src_pads(elem);

   while (!done && !ret)
     {
        switch(gst_iterator_next(itr, &v))
          {
             case GST_ITERATOR_OK:
               pad = GST_PAD(g_value_get_object(&v));
               caps = gst_pad_get_current_caps(pad);
               if (gst_video_info_from_caps(&info, caps))
                 {
                    *width = info.width;
                    *height = info.height;
                    ret = EINA_TRUE;
                    done = EINA_TRUE;
                 }
               gst_caps_unref(caps);
               g_value_reset(&v);
               break;
             case GST_ITERATOR_RESYNC:
               gst_iterator_resync(itr);
               done = ret = EINA_FALSE;
               break;
             case GST_ITERATOR_ERROR:
             case GST_ITERATOR_DONE:
               done = TRUE;
               break;
          }
     }
   g_value_unset(&v);
   gst_iterator_free(itr);

   return ret;
}

static void
_main_frame_resize(void *data)
{
   Emotion_Gstreamer_Video *ev = data;
   double ratio;

   ratio = (double)ev->src_width / (double)ev->src_height;
   _emotion_frame_resize(ev->obj, ev->src_width, ev->src_height, ratio);
   _emotion_pending_ecore_end();
}

static void
_no_more_pads(GstElement *decodebin, gpointer data)
{
   GstIterator *itr = NULL;
   Emotion_Gstreamer_Video *ev = data;
   GValue v = G_VALUE_INIT;
   GstElement *elem;
   Eina_Bool done = EINA_FALSE;

   itr = gst_bin_iterate_elements(GST_BIN(decodebin));

   while (!done)
     {
        switch(gst_iterator_next(itr, &v))
          {
             case GST_ITERATOR_OK:
               elem = GST_ELEMENT(g_value_get_object(&v));
               if(_video_size_get(elem, &ev->src_width, &ev->src_height))
                 {
                    _emotion_pending_ecore_begin();
                    ecore_main_loop_thread_safe_call_async(_main_frame_resize, ev);
                    g_value_reset(&v);
                    done = EINA_TRUE;
                    break;
                 }
               g_value_reset(&v);
               break;
             case GST_ITERATOR_RESYNC:
               gst_iterator_resync(itr);
               done = EINA_FALSE;
               break;
             case GST_ITERATOR_ERROR:
             case GST_ITERATOR_DONE:
               done = TRUE;
               break;
          }
     }
   g_value_unset(&v);
   gst_iterator_free(itr);
}

static void
_eos_main_fct(void *data)
{
   Emotion_Gstreamer_Message *send;
   Emotion_Gstreamer_Video *ev;
   GstMessage              *msg;

   send = data;
   ev = send->ev;
   msg = send->msg;

   if (ev->play_started && !ev->delete_me)
     {
        _emotion_playback_started(ev->obj);
        ev->play_started = 0;
     }

   switch (GST_MESSAGE_TYPE(msg))
     {
      case GST_MESSAGE_EOS:
         if (!ev->delete_me)
           {
              ev->play = 0;
              _emotion_decode_stop(ev->obj);
              _emotion_playback_finished(ev->obj);
           }
         break;
      case GST_MESSAGE_TAG:
         if (!ev->delete_me)
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
           }
         break;
      case GST_MESSAGE_ASYNC_DONE:
         if (!ev->delete_me) _emotion_seek_done(ev->obj);
         break;
      case GST_MESSAGE_STREAM_STATUS:
         break;
      case GST_MESSAGE_STATE_CHANGED:
         if (!ev->delete_me)
           {
              /* FIXME: This is conceptionally broken */
              if (!g_signal_handlers_disconnect_by_func(msg->src, _no_more_pads, ev))
                 g_signal_connect(msg->src, "no-more-pads", G_CALLBACK(_no_more_pads), ev);
           }
         break;
      case GST_MESSAGE_ERROR:
         em_cleanup(ev);
         break;
      default:
         ERR("bus say: %s [%i - %s]",
             GST_MESSAGE_SRC_NAME(msg),
             GST_MESSAGE_TYPE(msg),
	     GST_MESSAGE_TYPE_NAME(msg));
         break;
     }

   emotion_gstreamer_message_free(send);
   _emotion_pending_ecore_end();
}

static GstBusSyncReply
_eos_sync_fct(GstBus *bus EINA_UNUSED, GstMessage *msg, gpointer data)
{
   Emotion_Gstreamer_Video *ev = data;
   Emotion_Gstreamer_Message *send;

   switch (GST_MESSAGE_TYPE(msg))
     {
      case GST_MESSAGE_EOS:
      case GST_MESSAGE_TAG:
      case GST_MESSAGE_ASYNC_DONE:
      case GST_MESSAGE_STREAM_STATUS:
         INF("bus say: %s [%i - %s]",
             GST_MESSAGE_SRC_NAME(msg),
             GST_MESSAGE_TYPE(msg),
	     GST_MESSAGE_TYPE_NAME(msg));
         send = emotion_gstreamer_message_alloc(ev, msg);

        if (send)
          {
             _emotion_pending_ecore_begin();
             ecore_main_loop_thread_safe_call_async(_eos_main_fct, send);
          }

         break;

      case GST_MESSAGE_STATE_CHANGED:
        {
           GstState old_state, new_state;

           gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
           INF("Element %s changed state from %s to %s.",
               GST_OBJECT_NAME(msg->src),
               gst_element_state_get_name(old_state),
               gst_element_state_get_name(new_state));

           /* FIXME: This is broken */
           if (!strncmp(GST_OBJECT_NAME(msg->src), "decodebin", 9) && new_state == GST_STATE_READY)
             {
                send = emotion_gstreamer_message_alloc(ev, msg);

                if (send)
                  {
                     _emotion_pending_ecore_begin();
                     ecore_main_loop_thread_safe_call_async(_eos_main_fct, send);
                  }
             }
           break;
        }
      case GST_MESSAGE_ERROR:
	{
           GError *error;
           gchar *debug;

	   gst_message_parse_error(msg, &error, &debug);
	   ERR("ERROR from element %s: %s", GST_OBJECT_NAME(msg->src), error->message);
	   ERR("Debugging info: %s", (debug) ? debug : "none");
	   g_error_free(error);
	   g_free(debug);

           send = emotion_gstreamer_message_alloc(ev, msg);

           if (send)
             {
                _emotion_pending_ecore_begin();
                ecore_main_loop_thread_safe_call_async(_eos_main_fct, send);
             }
	   break;
	}
      case GST_MESSAGE_WARNING:
        {
           GError *error;
           gchar *debug;

           gst_message_parse_warning(msg, &error, &debug);
           WRN("WARNING from element %s: %s", GST_OBJECT_NAME(msg->src), error->message);
           WRN("Debugging info: %s", (debug) ? debug : "none");
           g_error_free(error);
           g_free(debug);
           break;
        }
      default:
         WRN("bus say: %s [%i - %s]",
             GST_MESSAGE_SRC_NAME(msg),
             GST_MESSAGE_TYPE(msg),
	     GST_MESSAGE_TYPE_NAME(msg));
         break;
     }

   gst_message_unref(msg);

   return GST_BUS_DROP;
}

Eina_Bool
_emotion_gstreamer_video_pipeline_parse(Emotion_Gstreamer_Video *ev,
                                        Eina_Bool force)
{
   gboolean res;
   int i;

   if (ev->pipeline_parsed)
     return EINA_TRUE;

   if (force && ev->threads)
     {
        Ecore_Thread *t;

        EINA_LIST_FREE(ev->threads, t)
          ecore_thread_cancel(t);
     }

   if (ev->threads)
     return EINA_FALSE;

   res = gst_element_get_state(ev->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
   if (res == GST_STATE_CHANGE_NO_PREROLL)
     {
       gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);

       res = gst_element_get_state(ev->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
     }

   /** NOTE: you need to set: GST_DEBUG_DUMP_DOT_DIR=/tmp EMOTION_ENGINE=gstreamer to save the $EMOTION_GSTREAMER_DOT file in '/tmp' */
   /** then call dot -Tpng -oemotion_pipeline.png /tmp/$TIMESTAMP-$EMOTION_GSTREAMER_DOT.dot */
   if (getenv("EMOTION_GSTREAMER_DOT"))
     GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(ev->pipeline),
				       GST_DEBUG_GRAPH_SHOW_ALL,
				       getenv("EMOTION_GSTREAMER_DOT"));

   if (!(res == GST_STATE_CHANGE_SUCCESS
         || res == GST_STATE_CHANGE_NO_PREROLL))
     {
        ERR("Unable to get GST_CLOCK_TIME_NONE.");
        return EINA_FALSE;
     }

   g_object_get(G_OBJECT(ev->pipeline),
                "n-audio", &ev->audio_stream_nbr,
                "n-video", &ev->video_stream_nbr,
                NULL);

   if ((ev->video_stream_nbr == 0) && (ev->audio_stream_nbr == 0))
     {
        ERR("No audio nor video stream found");
        return EINA_FALSE;
     }

   /* video stream */
   for (i = 0; i < ev->video_stream_nbr; i++)
     {
        Emotion_Video_Stream *vstream;
        GstPad       *pad = NULL;
        GstCaps      *caps;
        GstQuery     *query;

        gdouble length_time = 0.0;
        GstVideoInfo info;

        g_signal_emit_by_name(ev->pipeline, "get-video-pad", i, &pad);
        if (!pad)
          continue;

        caps = gst_pad_get_current_caps(pad);
        if (!caps)
          goto unref_pad_v;

        if (!gst_video_info_from_caps(&info, caps))
          goto unref_caps_v;

        query = gst_query_new_duration(GST_FORMAT_TIME);
        if (gst_pad_peer_query(pad, query))
          {
             gint64 t;

             gst_query_parse_duration(query, NULL, &t);
             length_time = (double)t / (double)GST_SECOND;
          }
        else
          goto unref_query_v;

        vstream = emotion_video_stream_new(ev);
        if (!vstream) goto unref_query_v;

        vstream->length_time = length_time;
        vstream->info = info;
        vstream->index = i;

     unref_query_v:
        gst_query_unref(query);
     unref_caps_v:
        gst_caps_unref(caps);
     unref_pad_v:
        gst_object_unref(pad);
     }

   /* Audio streams */
   for (i = 0; i < ev->audio_stream_nbr; i++)
     {
        Emotion_Audio_Stream *astream;
        GstPad       *pad;
        GstCaps      *caps;
        GstAudioInfo  info;
        GstQuery     *query;

        gdouble length_time = 0.0;

        g_signal_emit_by_name(ev->pipeline, "get-audio-pad", i, &pad);
        if (!pad)
          continue;

        caps = gst_pad_get_current_caps(pad);
        if (!caps)
          goto unref_pad_a;

        if (!gst_audio_info_from_caps(&info, caps))
          goto unref_caps_a;

        query = gst_query_new_duration(GST_FORMAT_TIME);
        if (gst_pad_peer_query(pad, query))
          {
             gint64 t;

             gst_query_parse_duration(query, NULL, &t);
             length_time = (double)t / (double)GST_SECOND;
          }
        else
          goto unref_query_a;

        astream = calloc(1, sizeof(Emotion_Audio_Stream));
        if (!astream) continue;
        ev->audio_streams = eina_list_append(ev->audio_streams, astream);

        astream->length_time = length_time;
        astream->info = info;

     unref_query_a:
        gst_query_unref(query);
     unref_caps_a:
        gst_caps_unref(caps);
     unref_pad_a:
        gst_object_unref(pad);
     }

   /* Visualization sink */
   if (ev->video_stream_nbr == 0)
     {
        GstElement *vis = NULL;
        Emotion_Video_Stream *vstream;
        Emotion_Audio_Stream *astream;
        gint flags;
        const char *vis_name;

        if (!(vis_name = emotion_visualization_element_name_get(ev->vis)))
          {
             WRN("pb vis name %d", ev->vis);
             goto finalize;
          }

        astream = eina_list_data_get(ev->audio_streams);

        vis = gst_element_factory_make(vis_name, "vissink");
        vstream = emotion_video_stream_new(ev);
        if (!vstream)
          goto finalize;
        else
          DBG("could not create visualization stream");

        vstream->length_time = astream->length_time;
        gst_video_info_init (&vstream->info);
        gst_video_info_set_format (&vstream->info, 320, 200, GST_VIDEO_FORMAT_ARGB);
        vstream->info.fps_n = 25;
        vstream->info.fps_d = 1;

        g_object_set(G_OBJECT(ev->pipeline), "vis-plugin", vis, NULL);
        g_object_get(G_OBJECT(ev->pipeline), "flags", &flags, NULL);
        flags |= 0x00000008;
        g_object_set(G_OBJECT(ev->pipeline), "flags", flags, NULL);
     }

 finalize:

   ev->video_stream_nbr = eina_list_count(ev->video_streams);
   ev->audio_stream_nbr = eina_list_count(ev->audio_streams);

   if (ev->video_stream_nbr == 1)
     {
       Emotion_Video_Stream *vstream;

       vstream = eina_list_data_get(ev->video_streams);
       ev->ratio = (double)vstream->info.width / (double)vstream->info.height;
       ev->ratio *= (double)vstream->info.par_n / (double)vstream->info.par_d;
       _emotion_frame_resize(ev->obj, vstream->info.width, vstream->info.height, ev->ratio);
     }

   {
     /* on recapitule : */
     Emotion_Video_Stream *vstream;
     Emotion_Audio_Stream *astream;

     vstream = eina_list_data_get(ev->video_streams);
     if (vstream)
       {
         DBG("video size=%dx%d, fps=%d/%d, "
             "format=%s, length=%"GST_TIME_FORMAT,
             vstream->info.width, vstream->info.height, vstream->info.fps_n, vstream->info.fps_d,
             gst_video_format_to_string(vstream->info.finfo->format),
             GST_TIME_ARGS((guint64)(vstream->length_time * GST_SECOND)));
       }

     astream = eina_list_data_get(ev->audio_streams);
     if (astream)
       {
         DBG("audio channels=%d, rate=%d, length=%"GST_TIME_FORMAT,
             astream->info.channels, astream->info.rate,
             GST_TIME_ARGS((guint64)(astream->length_time * GST_SECOND)));
       }
   }

   if (ev->metadata)
     _free_metadata(ev->metadata);
   ev->metadata = calloc(1, sizeof(Emotion_Gstreamer_Metadata));

   ev->pipeline_parsed = EINA_TRUE;

   em_audio_channel_volume_set(ev, ev->volume);
   em_audio_channel_mute_set(ev, ev->audio_mute);

   if (ev->play_started)
     {
        _emotion_playback_started(ev->obj);
        ev->play_started = 0;
     }

   _emotion_open_done(ev->obj);

   return EINA_TRUE;
}
