#include <unistd.h>
#include <fcntl.h>

#include <Eina.h>

#include "emotion_private.h"
#include "emotion_gstreamer.h"
#include "Emotion.h"

int _emotion_gstreamer_log_domain = -1;

/* Callbacks to get the eos */
static void _for_each_tag    (GstTagList const* list, gchar const* tag, void *data);
static void _free_metadata   (Emotion_Gstreamer_Metadata *m);

/* Interface */

static unsigned char  em_init                     (Evas_Object     *obj,
                                                   void           **emotion_video,
                                                   Emotion_Module_Options *opt);

static unsigned char  em_file_open                (const char     *file,
                                                   Evas_Object     *obj,
                                                   void            *video);

static void           em_file_close               (void            *video);

static void           em_play                     (void            *video,
                                                   double           pos);

static void           em_stop                     (void            *video);

static void           em_size_get                 (void            *video,
                                                   int             *width,
                                                   int             *height);

static void           em_pos_set                  (void            *video,
                                                   double           pos);


static double         em_len_get                  (void            *video);

static int            em_fps_num_get              (void            *video);

static int            em_fps_den_get              (void            *video);

static double         em_fps_get                  (void            *video);

static double         em_pos_get                  (void            *video);

static void           em_vis_set                  (void            *video,
                                                   Emotion_Vis      vis);

static Emotion_Vis    em_vis_get                  (void            *video);

static Eina_Bool      em_vis_supported            (void            *video,
                                                   Emotion_Vis      vis);

static double         em_ratio_get                (void            *video);

static int            em_video_handled            (void            *video);

static int            em_audio_handled            (void            *video);

static int            em_seekable                 (void            *video);

static void           em_frame_done               (void            *video);

static Emotion_Format em_format_get               (void            *video);

static void           em_video_data_size_get      (void            *video,
                                                   int             *w,
                                                   int             *h);

static int            em_yuv_rows_get             (void            *video,
                                                   int              w,
                                                   int              h,
                                                   unsigned char  **yrows,
                                                   unsigned char  **urows,
                                                   unsigned char  **vrows);

static int            em_bgra_data_get            (void            *video,
                                                   unsigned char  **bgra_data);

static void           em_event_feed               (void            *video,
                                                   int              event);

static void           em_event_mouse_button_feed  (void            *video,
                                                   int              button,
                                                   int              x,
                                                   int              y);

static void           em_event_mouse_move_feed    (void            *video,
                                                   int              x,
                                                   int              y);

static int            em_video_channel_count      (void             *video);

static void           em_video_channel_set        (void             *video,
                                                   int               channel);

static int            em_video_channel_get        (void             *video);

static const char    *em_video_channel_name_get   (void             *video,
                                                   int               channel);

static void           em_video_channel_mute_set   (void             *video,
                                                   int               mute);

static int            em_video_channel_mute_get   (void             *video);

static int            em_audio_channel_count      (void             *video);

static void           em_audio_channel_set        (void             *video,
                                                   int               channel);

static int            em_audio_channel_get        (void             *video);

static const char    *em_audio_channel_name_get   (void             *video,
                                                   int               channel);

static void           em_audio_channel_mute_set   (void             *video,
                                                   int               mute);

static int            em_audio_channel_mute_get   (void             *video);

static void           em_audio_channel_volume_set (void             *video,
                                                   double             vol);

static double         em_audio_channel_volume_get (void             *video);

static int            em_spu_channel_count        (void             *video);

static void           em_spu_channel_set          (void             *video,
                                                   int               channel);

static int            em_spu_channel_get          (void             *video);

static const char    *em_spu_channel_name_get     (void             *video,
                                                   int               channel);

static void           em_spu_channel_mute_set     (void             *video,
                                                   int               mute);

static int            em_spu_channel_mute_get     (void             *video);

static int            em_chapter_count            (void             *video);

static void           em_chapter_set              (void             *video,
                                                   int               chapter);

static int            em_chapter_get              (void             *video);

static const char    *em_chapter_name_get         (void             *video,
                                                   int               chapter);

static void           em_speed_set                (void             *video,
                                                   double            speed);

static double         em_speed_get                (void             *video);

static int            em_eject                    (void             *video);

static const char    *em_meta_get                 (void             *video,
                                                   int               meta);

static GstBusSyncReply _eos_sync_fct(GstBus *bus,
				     GstMessage *message,
				     gpointer data);

/* Module interface */

static Emotion_Video_Module em_module =
{
   em_init, /* init */
   em_shutdown, /* shutdown */
   em_file_open, /* file_open */
   em_file_close, /* file_close */
   em_play, /* play */
   em_stop, /* stop */
   em_size_get, /* size_get */
   em_pos_set, /* pos_set */
   em_len_get, /* len_get */
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
   NULL /* handle */
};

static Emotion_Video_Stream *
emotion_video_stream_new(Emotion_Gstreamer_Video *ev)
{
   Emotion_Video_Stream *vstream;

   if (!ev) return NULL;

   vstream = (Emotion_Video_Stream *)calloc(1, sizeof(Emotion_Video_Stream));
   if (!vstream) return NULL;

   ev->video_streams = eina_list_append(ev->video_streams, vstream);
   if (eina_error_get())
     {
        free(vstream);
        return NULL;
     }
   return vstream;
}

static void
emotion_video_stream_free(Emotion_Gstreamer_Video *ev, Emotion_Video_Stream *vstream)
{
   if (!ev || !vstream) return;

   ev->video_streams = eina_list_remove(ev->video_streams, vstream);
        free(vstream);
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

static unsigned char
em_init(Evas_Object            *obj,
        void                  **emotion_video,
        Emotion_Module_Options *opt __UNUSED__)
{
   Emotion_Gstreamer_Video *ev;
   GError                  *error;

   if (!emotion_video)
     return 0;

   ev = calloc(1, sizeof(Emotion_Gstreamer_Video));
   if (!ev) return 0;

   ev->obj = obj;

   /* Initialization of gstreamer */
   if (!gst_init_check(NULL, NULL, &error))
     goto failure;

   /* Default values */
   ev->ratio = 1.0;
   ev->vis = EMOTION_VIS_NONE;
   ev->volume = 0.8;
   ev->play_started = 0;
   ev->delete_me = EINA_FALSE;
   ev->threads = NULL;

   *emotion_video = ev;

   return 1;

failure:
   free(ev);

   return 0;
}

int
em_shutdown(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Audio_Stream *astream;
   Emotion_Video_Stream *vstream;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev)
     return 0;

   if (ev->threads)
     {
        Ecore_Thread *t;

        EINA_LIST_FREE(ev->threads, t)
          ecore_thread_cancel(t);

        ev->delete_me = EINA_TRUE;
        return 1;
     }

   if (ev->in != ev->out)
     {
        ev->delete_me = EINA_TRUE;
        return 1;
     }

   if (ev->eos_bus)
     {
        gst_object_unref(GST_OBJECT(ev->eos_bus));
        ev->eos_bus = NULL;
     }

   if (ev->pipeline)
     {
       g_object_set(G_OBJECT(ev->sink), "ev", NULL, NULL);
       g_object_set(G_OBJECT(ev->sink), "evas-object", NULL, NULL);
       gst_element_set_state(ev->pipeline, GST_STATE_NULL);
       gst_object_unref(ev->pipeline);

       if (ev->last_buffer)
	 {
            gst_buffer_unref(ev->last_buffer);
            ev->last_buffer = NULL;
	 }

       ev->pipeline = NULL;
       ev->sink = NULL;
     }

   EINA_LIST_FREE(ev->audio_streams, astream)
     free(astream);
   EINA_LIST_FREE(ev->video_streams, vstream)
     free(vstream);

   free(ev);

   return 1;
}


static unsigned char
em_file_open(const char   *file,
             Evas_Object  *obj,
             void         *video)
{
   Emotion_Gstreamer_Video *ev;
   Eina_Strbuf *sbuf = NULL;
   const char *uri;

   ev = (Emotion_Gstreamer_Video *)video;

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
   ev->pipeline = gstreamer_video_sink_new(ev, obj, uri);
   if (sbuf) eina_strbuf_free(sbuf);

   if (!ev->pipeline)
     return EINA_FALSE;

   ev->eos_bus = gst_pipeline_get_bus(GST_PIPELINE(ev->pipeline));
   if (!ev->eos_bus)
     {
        ERR("could not get the bus");
        return EINA_FALSE;
     }

   gst_bus_set_sync_handler(ev->eos_bus, _eos_sync_fct, ev);

   /* Evas Object */
   ev->obj = obj;

   ev->position = 0.0;

   return 1;
}

static void
em_file_close(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Audio_Stream *astream;
   Emotion_Video_Stream *vstream;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev)
     return;

   if (ev->eos_bus)
     {
        gst_object_unref(GST_OBJECT(ev->eos_bus));
        ev->eos_bus = NULL;
     }

   if (ev->threads)
     {
        Ecore_Thread *t;

        EINA_LIST_FREE(ev->threads, t)
          ecore_thread_cancel(t);
     }

   if (ev->pipeline)
     {
        g_object_set(G_OBJECT(ev->sink), "ev", NULL, NULL);
        g_object_set(G_OBJECT(ev->sink), "evas-object", NULL, NULL);
        gst_element_set_state(ev->pipeline, GST_STATE_NULL);
        gst_object_unref(ev->pipeline);
        ev->pipeline = NULL;
        ev->sink = NULL;
     }

   /* we clear the stream lists */
   EINA_LIST_FREE(ev->audio_streams, astream)
     free(astream);
   EINA_LIST_FREE(ev->video_streams, vstream)
     free(vstream);
   ev->pipeline_parsed = EINA_FALSE;
   ev->play_started = 0;

   /* shutdown eos */
   if (ev->metadata)
     {
        _free_metadata(ev->metadata);
        ev->metadata = NULL;
     }
}

static void
em_play(void   *video,
        double  pos __UNUSED__)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev->pipeline) return ;

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

   if (!ev->pipeline) return ;

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
        if (width) *width = vstream->width;
        if (height) *height = vstream->height;

        return ;
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
   gboolean res;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!ev->pipeline) return ;

   if (ev->play)
     res = gst_element_set_state(ev->pipeline, GST_STATE_PAUSED);

   res = gst_element_seek(ev->pipeline, 1.0,
                          GST_FORMAT_TIME,
                          GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH,
                          GST_SEEK_TYPE_SET,
                          (gint64)(pos * (double)GST_SECOND),
                          GST_SEEK_TYPE_NONE, -1);

   if (ev->play)
     res = gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);
}

static double
em_len_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Stream *vstream;
   Emotion_Audio_Stream *astream;
   Eina_List *l;
   GstFormat fmt;
   gint64 val;
   gboolean ret;

   ev = video;
   fmt = GST_FORMAT_TIME;

   if (!ev->pipeline) return 0.0;

   ret = gst_element_query_duration(ev->pipeline, &fmt, &val);
   if (!ret)
     goto fallback;

   if (fmt != GST_FORMAT_TIME)
     {
        DBG("requrested duration in time, but got %s instead.",
            gst_format_get_name(fmt));
        goto fallback;
     }

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
     return vstream->fps_num;

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
     return vstream->fps_den;

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
     return (double)vstream->fps_num / (double)vstream->fps_den;

   return 0.0;
}

static double
em_pos_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   GstFormat fmt;
   gint64 val;
   gboolean ret;

   ev = video;
   fmt = GST_FORMAT_TIME;

   if (!ev->pipeline) return 0.0;

   ret = gst_element_query_position(ev->pipeline, &fmt, &val);
   if (!ret)
     return ev->position;

   if (fmt != GST_FORMAT_TIME)
     {
        ERR("requrested position in time, but got %s instead.",
            gst_format_get_name(fmt));
        return ev->position;
     }

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
em_vis_supported(void *ef __UNUSED__, Emotion_Vis vis)
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
em_seekable(void *video __UNUSED__)
{
   return 1;
}

static void
em_frame_done(void *video __UNUSED__)
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
        switch (vstream->fourcc)
          {
           case GST_MAKE_FOURCC('I', '4', '2', '0'):
              return EMOTION_FORMAT_I420;
           case GST_MAKE_FOURCC('Y', 'V', '1', '2'):
              return EMOTION_FORMAT_YV12;
           case GST_MAKE_FOURCC('Y', 'U', 'Y', '2'):
              return EMOTION_FORMAT_YUY2;
           case GST_MAKE_FOURCC('A', 'R', 'G', 'B'):
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

   if (!_emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE))
     goto on_error;

   vstream = eina_list_nth(ev->video_streams, ev->video_stream_nbr - 1);
   if (vstream)
     {
        *w = vstream->width;
        *h = vstream->height;

        return ;
     }

 on_error:
   *w = 0;
   *h = 0;
}

static int
em_yuv_rows_get(void           *video __UNUSED__,
                int             w __UNUSED__,
                int             h __UNUSED__,
                unsigned char **yrows __UNUSED__,
                unsigned char **urows __UNUSED__,
                unsigned char **vrows __UNUSED__)
{
   return 0;
}

static int
em_bgra_data_get(void *video __UNUSED__, unsigned char **bgra_data __UNUSED__)
{
   return 0;
}

static void
em_event_feed(void *video __UNUSED__, int event __UNUSED__)
{
}

static void
em_event_mouse_button_feed(void *video __UNUSED__, int button __UNUSED__, int x __UNUSED__, int y __UNUSED__)
{
}

static void
em_event_mouse_move_feed(void *video __UNUSED__, int x __UNUSED__, int y __UNUSED__)
{
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
em_video_channel_set(void *video __UNUSED__,
                     int   channel __UNUSED__)
{
#if 0
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (channel < 0) channel = 0;
#endif
   /* FIXME: a faire... */
}

static int
em_video_channel_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_FALSE);

   return ev->video_stream_nbr;
}

static const char *
em_video_channel_name_get(void *video __UNUSED__,
                          int   channel __UNUSED__)
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
em_audio_channel_set(void *video __UNUSED__,
                     int   channel __UNUSED__)
{
#if 0
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (channel < -1) channel = -1;
#endif
   /* FIXME: a faire... */
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
em_audio_channel_name_get(void *video __UNUSED__,
                          int   channel __UNUSED__)
{
   return NULL;
}

#define GST_PLAY_FLAG_AUDIO (1 << 1)

static void
em_audio_channel_mute_set(void *video,
                          int   mute)
{
   Emotion_Gstreamer_Video *ev;
   int flags;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!ev->pipeline) return ;

   ev->audio_mute = mute;

   g_object_set(G_OBJECT(ev->pipeline), "mute", !!mute, NULL);
   /* This code should stop the decoding of only the audio stream, but everything stop :"( */
   /* g_object_get(G_OBJECT(ev->pipeline), "flags", &flags, NULL); */
   /* if (mute) */
   /*   flags &= ~GST_PLAY_FLAG_AUDIO; */
   /* else */
   /*   flags |= GST_PLAY_FLAG_AUDIO; */
   /* g_object_set(G_OBJECT(ev->pipeline), "flags", flags, NULL); */
   /* g_object_get(G_OBJECT(ev->pipeline), "flags", &flags, NULL); */
   /* fprintf(stderr, "flags-n: %x\n", flags); */
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

   if (!ev->pipeline) return ;

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
em_spu_channel_count(void *video __UNUSED__)
{
   return 0;
}

static void
em_spu_channel_set(void *video __UNUSED__, int channel __UNUSED__)
{
}

static int
em_spu_channel_get(void *video __UNUSED__)
{
   return 1;
}

static const char *
em_spu_channel_name_get(void *video __UNUSED__, int channel __UNUSED__)
{
   return NULL;
}

static void
em_spu_channel_mute_set(void *video __UNUSED__, int mute __UNUSED__)
{
}

static int
em_spu_channel_mute_get(void *video __UNUSED__)
{
   return 0;
}

static int
em_chapter_count(void *video __UNUSED__)
{
   return 0;
}

static void
em_chapter_set(void *video __UNUSED__, int chapter __UNUSED__)
{
}

static int
em_chapter_get(void *video __UNUSED__)
{
   return 0;
}

static const char *
em_chapter_name_get(void *video __UNUSED__, int chapter __UNUSED__)
{
   return NULL;
}

static void
em_speed_set(void *video __UNUSED__, double speed __UNUSED__)
{
}

static double
em_speed_get(void *video __UNUSED__)
{
   return 1.0;
}

static int
em_eject(void *video __UNUSED__)
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

static Eina_Bool
module_open(Evas_Object           *obj,
            const Emotion_Video_Module **module,
            void                 **video,
            Emotion_Module_Options *opt)
{
   if (!module)
     return EINA_FALSE;

   if (_emotion_gstreamer_log_domain < 0)
     {
        eina_threads_init();
        eina_log_threads_enable();
        _emotion_gstreamer_log_domain = eina_log_domain_register
          ("emotion-gstreamer", EINA_COLOR_LIGHTCYAN);
        if (_emotion_gstreamer_log_domain < 0)
          {
             EINA_LOG_CRIT("Could not register log domain 'emotion-gstreamer'");
             return EINA_FALSE;
          }
     }

   if (!em_module.init(obj, video, opt))
     return EINA_FALSE;

   eina_threads_init();

   *module = &em_module;
   return EINA_TRUE;
}

static void
module_close(Emotion_Video_Module *module __UNUSED__,
             void                 *video)
{
   em_module.shutdown(video);

   eina_threads_shutdown();
}

Eina_Bool
gstreamer_module_init(void)
{
   GError *error;

   if (!gst_init_check(0, NULL, &error))
     {
        EINA_LOG_CRIT("Could not init GStreamer");
        return EINA_FALSE;
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
        return EINA_FALSE;
     }

   return _emotion_module_register("gstreamer", module_open, module_close);
}

void
gstreamer_module_shutdown(void)
{
   _emotion_module_unregister("gstreamer");

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

#ifdef GST_TAG_CDDA_CDDB_DISCID
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
#endif
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
      default:
         ERR("bus say: %s [%i - %s]",
             GST_MESSAGE_SRC_NAME(msg),
             GST_MESSAGE_TYPE(msg),
	     GST_MESSAGE_TYPE_NAME(msg));
         break;
     }

   emotion_gstreamer_message_free(send);
}

static GstBusSyncReply
_eos_sync_fct(GstBus *bus __UNUSED__, GstMessage *msg, gpointer data)
{
   Emotion_Gstreamer_Video *ev = data;
   Emotion_Gstreamer_Message *send;

   switch (GST_MESSAGE_TYPE(msg))
     {
      case GST_MESSAGE_EOS:
      case GST_MESSAGE_TAG:
      case GST_MESSAGE_ASYNC_DONE:
      case GST_MESSAGE_STREAM_STATUS:
         send = emotion_gstreamer_message_alloc(ev, msg);

         if (send) ecore_main_loop_thread_safe_call_async(_eos_main_fct, send);

         break;

      case GST_MESSAGE_STATE_CHANGED:
        {
           GstState old_state, new_state;

           gst_message_parse_state_changed (msg, &old_state, &new_state, NULL);
           INF("Element %s changed state from %s to %s.",
               GST_OBJECT_NAME(msg->src),
               gst_element_state_get_name(old_state),
               gst_element_state_get_name(new_state));
           break;
        }
      case GST_MESSAGE_ERROR:
	{
           GError *error;
           gchar *debug;

	   gst_message_parse_error(msg, &error, &debug);
	   ERR("WARNING from element %s: %s", GST_OBJECT_NAME(msg->src), error->message);
	   ERR("Debugging info: %s", (debug) ? debug : "none");
	   g_error_free(error);
	   g_free(debug);
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
     gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);

   res = gst_element_get_state(ev->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
   if (!(res == GST_STATE_CHANGE_SUCCESS
         || res == GST_STATE_CHANGE_NO_PREROLL))
     {
        /** NOTE: you need to set: GST_DEBUG_DUMP_DOT_DIR=/tmp EMOTION_ENGINE=gstreamer to save the $EMOTION_GSTREAMER_DOT file in '/tmp' */
        /** then call dot -Tpng -oemotion_pipeline.png /tmp/$TIMESTAMP-$EMOTION_GSTREAMER_DOT.dot */
        if (getenv("EMOTION_GSTREAMER_DOT"))
          GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(ev->pipeline),
                                            GST_DEBUG_GRAPH_SHOW_ALL,
                                            getenv("EMOTION_GSTREAMER_DOT"));

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
        GstStructure *structure;
        GstQuery     *query;
        const GValue *val;
        gchar        *str;

        gdouble length_time = 0.0;
        gint width;
        gint height;
        gint fps_num;
        gint fps_den;
        guint32 fourcc = 0;

        g_signal_emit_by_name(ev->pipeline, "get-video-pad", i, &pad);
        if (!pad)
          continue;

        caps = gst_pad_get_negotiated_caps(pad);
        if (!caps)
          goto unref_pad_v;
        structure = gst_caps_get_structure(caps, 0);
        str = gst_caps_to_string(caps);

        if (!gst_structure_get_int(structure, "width", &width))
          goto unref_caps_v;
        if (!gst_structure_get_int(structure, "height", &height))
          goto unref_caps_v;
        if (!gst_structure_get_fraction(structure, "framerate", &fps_num, &fps_den))
          goto unref_caps_v;

        if (g_str_has_prefix(str, "video/x-raw-yuv"))
          {
             val = gst_structure_get_value(structure, "format");
             fourcc = gst_value_get_fourcc(val);
          }
        else if (g_str_has_prefix(str, "video/x-raw-rgb"))
          fourcc = GST_MAKE_FOURCC('A', 'R', 'G', 'B');
        else
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
        vstream->width = width;
        vstream->height = height;
        vstream->fps_num = fps_num;
        vstream->fps_den = fps_den;
        vstream->fourcc = fourcc;
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
        GstStructure *structure;
        GstQuery     *query;

        gdouble length_time = 0.0;
        gint channels;
        gint samplerate;

        g_signal_emit_by_name(ev->pipeline, "get-audio-pad", i, &pad);
        if (!pad)
          continue;

        caps = gst_pad_get_negotiated_caps(pad);
        if (!caps)
          goto unref_pad_a;
        structure = gst_caps_get_structure(caps, 0);

        if (!gst_structure_get_int(structure, "channels", &channels))
          goto unref_caps_a;
        if (!gst_structure_get_int(structure, "rate", &samplerate))
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
        if (eina_error_get())
          {
             free(astream);
             continue;
          }

        astream->length_time = length_time;
        astream->channels = channels;
        astream->samplerate = samplerate;

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
        vstream->width = 320;
        vstream->height = 200;
        vstream->fps_num = 25;
        vstream->fps_den = 1;
        vstream->fourcc = GST_MAKE_FOURCC('A', 'R', 'G', 'B');

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
       ev->ratio = (double)vstream->width / (double)vstream->height;
       _emotion_frame_resize(ev->obj, vstream->width, vstream->height, ev->ratio);
     }

   {
     /* on recapitule : */
     Emotion_Video_Stream *vstream;
     Emotion_Audio_Stream *astream;

     vstream = eina_list_data_get(ev->video_streams);
     if (vstream)
       {
         DBG("video size=%dx%d, fps=%d/%d, "
             "fourcc=%"GST_FOURCC_FORMAT", length=%"GST_TIME_FORMAT,
             vstream->width, vstream->height, vstream->fps_num, vstream->fps_den,
             GST_FOURCC_ARGS(vstream->fourcc),
             GST_TIME_ARGS((guint64)(vstream->length_time * GST_SECOND)));
       }

     astream = eina_list_data_get(ev->audio_streams);
     if (astream)
       {
         DBG("audio channels=%d, rate=%d, length=%"GST_TIME_FORMAT,
             astream->channels, astream->samplerate,
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
