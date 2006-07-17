#include <unistd.h>
#include <fcntl.h>

#include "Emotion.h"
#include "emotion_private.h"
#include "emotion_gstreamer.h"
#include "emotion_gstreamer_pipeline.h"


/* Callbacks to get the eos */
static int _eos_timer_fct (void *data);

static int   _em_fd_ev_active(void *data, Ecore_Fd_Handler *fdh);


/* Interface */

static unsigned char  em_init                     (Evas_Object     *obj,
                                                   void           **emotion_video);

static int            em_shutdown                 (void           *video);

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

static void           em_vis_set                  (void            *video,
                                                   Emotion_Vis      vis);

static double         em_len_get                  (void            *video);

static int            em_fps_num_get              (void            *video);

static int            em_fps_den_get              (void            *video);

static double         em_fps_get                  (void            *video);

static double         em_pos_get                  (void            *video);

static Emotion_Vis    em_vis_get                  (void            *video);

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
   em_vis_set, /* vis_set */
   em_len_get, /* len_get */
   em_fps_num_get, /* fps_num_get */
   em_fps_den_get, /* fps_den_get */
   em_fps_get, /* fps_get */
   em_pos_get, /* pos_get */
   em_vis_get, /* vis_get */
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
   em_meta_get /* meta_get */
};

static unsigned char
em_init(Evas_Object  *obj,
	void        **emotion_video)
{
   Emotion_Gstreamer_Video *ev;
   GError                  *error;
   int                      fds[2];

   if (!emotion_video)
      return 0;

   printf ("Init gstreamer...\n");

   ev = calloc(1, sizeof(Emotion_Gstreamer_Video));
   if (!ev) return 0;

   ev->obj = obj;
   ev->obj_data = NULL;

   /* Initialization of gstreamer */
   if (!gst_init_check (NULL, NULL, &error))
     goto failure_gstreamer;

   ev->pipeline = gst_pipeline_new ("pipeline");
   if (!ev->pipeline)
     goto failure_pipeline;

   ev->eos_bus = gst_pipeline_get_bus (GST_PIPELINE (ev->pipeline));
   if (!ev->eos_bus)
     goto failure_bus;

   /* We allocate the sinks lists */
   ev->video_sinks = ecore_list_new ();
   if (!ev->video_sinks)
     goto failure_video_sinks;
   ecore_list_set_free_cb(ev->video_sinks, ECORE_FREE_CB(free));
   ev->audio_sinks = ecore_list_new ();
   if (!ev->audio_sinks)
     goto failure_audio_sinks;
   ecore_list_set_free_cb(ev->audio_sinks, ECORE_FREE_CB(free));

   *emotion_video = ev;

   /* Default values */
   ev->ratio = 1.0;
   ev->video_sink_nbr = 0;
   ev->audio_sink_nbr = 0;
   ev->vis = EMOTION_VIS_GOOM;

   /* Create the file descriptors */
   if (pipe(fds) == 0) {
      ev->fd_ev_read = fds[0];
      ev->fd_ev_write = fds[1];
      fcntl(ev->fd_ev_read, F_SETFL, O_NONBLOCK);
      ev->fd_ev_handler = ecore_main_fd_handler_add(ev->fd_ev_read,
                                                    ECORE_FD_READ,
                                                    _em_fd_ev_active,
                                                    ev,
                                                    NULL, NULL);
      ecore_main_fd_handler_active_set(ev->fd_ev_handler, ECORE_FD_READ);
   }
   else
     goto failure_pipe;

   return 1;

 failure_pipe:
   ecore_list_destroy (ev->audio_sinks);
 failure_audio_sinks:
   ecore_list_destroy (ev->video_sinks);
 failure_video_sinks:
   gst_object_unref (GST_OBJECT (ev->eos_bus));
 failure_bus:
   /* this call is not really necessary */
   gst_element_set_state (ev->pipeline, GST_STATE_NULL);
   gst_object_unref (GST_OBJECT (ev->pipeline));
 failure_pipeline:
   gst_deinit ();
 failure_gstreamer:
   free (ev);

   return 0;
}

static int
em_shutdown(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev)
     return 0;

   gst_element_set_state (ev->pipeline, GST_STATE_NULL);
   gst_object_unref (GST_OBJECT (ev->pipeline));
   gst_object_unref (GST_OBJECT (ev->eos_bus));
   gst_deinit ();

   ecore_list_destroy (ev->video_sinks);
   ecore_list_destroy (ev->audio_sinks);

   /* FIXME: and the evas object ? */

   ecore_main_fd_handler_del(ev->fd_ev_handler);
   close(ev->fd_ev_write);
   close(ev->fd_ev_read);

   free(ev);

   return 1;
}

static unsigned char
em_file_open(const char   *file,
	     Evas_Object  *obj,
	     void         *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   /* Evas Object */
   ev->obj = obj;

   /* CD Audio */
   if (strstr (file,"cdda://")) {
      const char  *device = NULL;
      unsigned int track = 1;

      device = file + strlen ("cdda://");
      if (device[0] == '/') {
         char *tmp;

         if ((tmp = strchr (device, '?')) || (tmp = strchr (device, '#'))) {
            sscanf (tmp + 1,"%d", &track);
            tmp[0] = '\0';
         }
      }
      else {
         device = NULL;
         sscanf (file,"cdda://%d", &track);
      }
      printf ("build cdda pipeline\n");
      if (!(emotion_pipeline_cdda_build (ev, device, track))) {
        printf ("error building CA Audio pipeline\n");
        return 0;
      }
   }
   /* Normal media file */
   else {
      const char *filename;

      filename = strstr (file,"file://")
        ? file + strlen ("file://")
        : file;

      printf ("build file pipeline \n");
      if (!(emotion_pipeline_file_build (ev, filename))) {
        printf ("error building File pipeline\n");
        return 0;
      }
   }

   ev->position = 0.0;

   {
      /* on recapitule : */
     Emotion_Video_Sink *vsink;
     Emotion_Audio_Sink *asink;

     vsink = (Emotion_Video_Sink *)ecore_list_goto_first (ev->video_sinks);
     if (vsink) {
        g_print ("video : \n");
        g_print ("  size   : %dx%d\n", vsink->width, vsink->height);
        g_print ("  fps    : %d/%d\n", vsink->fps_num, vsink->fps_den);
        g_print ("  fourcc : %" GST_FOURCC_FORMAT "\n", GST_FOURCC_ARGS (vsink->fourcc));
        g_print ("  length : %" GST_TIME_FORMAT "\n\n",
                 GST_TIME_ARGS ((guint64)(vsink->length_time * GST_SECOND)));
     }

     asink = (Emotion_Audio_Sink *)ecore_list_goto_first (ev->audio_sinks);
     if (asink) {
        g_print ("audio : \n");
        g_print ("  chan   : %d\n", asink->channels);
        g_print ("  rate   : %d\n", asink->samplerate);
        g_print ("  length : %" GST_TIME_FORMAT "\n\n",
                 GST_TIME_ARGS ((guint64)(asink->length_time * GST_SECOND)));
     }
   }

   return 1;
}

static void
em_file_close(void *video)
{
   Emotion_Gstreamer_Video *ev;
   GstIterator             *iter;
   gpointer                 data;
   gboolean                 done;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev)
     return;

   printf("EX pause end...\n");
   if (!emotion_object_play_get(ev->obj))
     {
	printf("  ... unpause\n");
        emotion_pipeline_pause (ev->pipeline);
     }

   printf("EX stop\n");
   gst_element_set_state (ev->pipeline, GST_STATE_READY);

   /* we remove all the elements in the pipeline */
   iter = gst_bin_iterate_elements (GST_BIN (ev->pipeline));
   done = FALSE;
   while (!done) {
     switch (gst_iterator_next (iter, &data)) {
     case GST_ITERATOR_OK: {
       GstElement *element;

       element = GST_ELEMENT (data);
       if (element) {
         gst_bin_remove (GST_BIN (ev->pipeline), element);
       }
       break;
     }
     case GST_ITERATOR_RESYNC: {
       GstElement *element;

       element = GST_ELEMENT (data);
       if (element) {
         gst_bin_remove (GST_BIN (ev->pipeline), element);
       }
       gst_iterator_resync (iter);
       break;
     }
     case GST_ITERATOR_ERROR:
       printf("error iter\n");
       done = TRUE;
       break;
     case GST_ITERATOR_DONE:
       done = TRUE;
       break;
     }
   }
   gst_iterator_free (iter);

   /* we clear the sink lists */
   ecore_list_clear (ev->video_sinks);
   ecore_list_clear (ev->audio_sinks);

   /* shutdown eos */
   if (ev->eos_timer) {
     ecore_timer_del (ev->eos_timer);
     ev->eos_timer = NULL;
   }
}

static void
em_play(void   *video,
	double  pos)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   gst_element_set_state (ev->pipeline, GST_STATE_PLAYING);
   ev->play = 1;

   /* eos */
   ev->eos_timer = ecore_timer_add (0.1, _eos_timer_fct, ev);
}

static void
em_stop(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   gst_element_set_state (ev->pipeline, GST_STATE_PAUSED);
   ev->play = 0;

   /* shutdown eos */
   if (ev->eos_timer) {
     ecore_timer_del (ev->eos_timer);
     ev->eos_timer = NULL;
   }
}

static void
em_size_get(void  *video,
	    int   *width,
	    int   *height)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_goto_index (ev->video_sinks, ev->video_sink_nbr);
   if (vsink) {
      if (width) *width = vsink->width;
      if (height) *height = vsink->height;
   }
   else {
      if (width) *width = 0;
      if (height) *height = 0;
   }
}

static void
em_pos_set(void   *video,
	   double  pos)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;
   Emotion_Audio_Sink      *asink;

   ev = (Emotion_Gstreamer_Video *)video;

   if (ev->seek_to_pos == pos) return;

   vsink = (Emotion_Video_Sink *)ecore_list_goto_index (ev->video_sinks, ev->video_sink_nbr);
   asink = (Emotion_Audio_Sink *)ecore_list_goto_index (ev->video_sinks, ev->audio_sink_nbr);

   if (vsink) {
      gst_element_seek(vsink->sink, 1.0,
                       GST_FORMAT_TIME,
                       GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH,
                       GST_SEEK_TYPE_SET,
                       (gint64)(pos * (double)GST_SECOND),
                       GST_SEEK_TYPE_NONE,
                       -1);
   }
   if (asink) {
      gst_element_seek(asink->sink, 1.0,
                       GST_FORMAT_TIME,
                       GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH,
                       GST_SEEK_TYPE_SET,
                       (gint64)(pos * (double)GST_SECOND),
                       GST_SEEK_TYPE_NONE,
                       -1);
   }
   ev->seek_to_pos = pos;
}

static void
em_vis_set(void       *video,
	   Emotion_Vis vis)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (ev->vis == vis) return;
   ev->vis = vis;
}

static double
em_len_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_goto_index (ev->video_sinks, ev->video_sink_nbr);
   if (vsink)
      return (double)vsink->length_time;

   return 0.0;
}

static int
em_fps_num_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_goto_index (ev->video_sinks, ev->video_sink_nbr);
   if (vsink)
      return vsink->fps_num;

   return 0;
}

static int
em_fps_den_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_goto_index (ev->video_sinks, ev->video_sink_nbr);
   if (vsink)
      return vsink->fps_den;

   return 1;
}

static double
em_fps_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_goto_index (ev->video_sinks, ev->video_sink_nbr);
   if (vsink)
      return (double)vsink->fps_num / (double)vsink->fps_den;

   return 0.0;
}

static double
em_pos_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ev->position;
}

static Emotion_Vis
em_vis_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ev->vis;
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

   if (ecore_list_is_empty (ev->video_sinks))
     return 0;

   return 1;
}

static int
em_audio_handled(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (ecore_list_is_empty (ev->audio_sinks))
     return 0;

   return 1;
}

static int
em_seekable(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 1;
}

static void
em_frame_done(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
}

static Emotion_Format
em_format_get (void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_goto_index (ev->video_sinks, ev->video_sink_nbr);
   if (vsink) {
      switch (vsink->fourcc) {
      case GST_MAKE_FOURCC ('I','4','2','0'):
         return EMOTION_FORMAT_I420;
      case GST_MAKE_FOURCC ('Y','V','1','2'):
         return EMOTION_FORMAT_YV12;
      case GST_MAKE_FOURCC ('Y','U','Y','2'):
         return EMOTION_FORMAT_YUY2;
      case GST_MAKE_FOURCC ('A','R','G','B'):
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
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_goto_index (ev->video_sinks, ev->video_sink_nbr);
   if (vsink) {
      *w = vsink->width;
      *h = vsink->height;
   }
   else {
      *w = 0;
      *h = 0;
   }
}

static int
em_yuv_rows_get(void           *video,
		int             w,
		int             h,
		unsigned char **yrows,
		unsigned char **urows,
		unsigned char **vrows)
{
   Emotion_Gstreamer_Video *ev;
   int                      i;

   ev = (Emotion_Gstreamer_Video *)video;

   if (ev->obj_data)
     {
       if (em_format_get(video) == EMOTION_FORMAT_I420) {
         for (i = 0; i < h; i++)
           yrows[i] = &ev->obj_data[i * w];

         for (i = 0; i < (h / 2); i++)
           urows[i] = &ev->obj_data[h * w + i * (w / 2) ];

         for (i = 0; i < (h / 2); i++)
           vrows[i] = &ev->obj_data[h * w + h * (w /4) + i * (w / 2)];
       }
       else if (em_format_get(video) == EMOTION_FORMAT_YV12) {
         for (i = 0; i < h; i++)
           yrows[i] = &ev->obj_data[i * w];

         for (i = 0; i < (h / 2); i++)
           vrows[i] = &ev->obj_data[h * w + i * (w / 2) ];

         for (i = 0; i < (h / 2); i++)
           urows[i] = &ev->obj_data[h * w + h * (w /4) + i * (w / 2)];
       }
       else
         return 0;

       return 1;
     }

   return 0;
}

static int
em_bgra_data_get(void *video, unsigned char **bgra_data)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (ev->obj_data && em_format_get(video) == EMOTION_FORMAT_BGRA) {
      *bgra_data = ev->obj_data;
      return 1;
   }
   return 0;
}

static void
em_event_feed(void *video, int event)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
}

static void
em_event_mouse_button_feed(void *video, int button, int x, int y)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
}

static void
em_event_mouse_move_feed(void *video, int x, int y)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
}

/* Video channels */
static int
em_video_channel_count(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ecore_list_nodes(ev->video_sinks);
}

static void
em_video_channel_set(void *video,
		     int   channel)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (channel < 0) channel = 0;
   /* FIXME: a faire... */
}

static int
em_video_channel_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ev->video_sink_nbr;
}

static const char *
em_video_channel_name_get(void *video,
			  int   channel)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

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

   return ecore_list_nodes(ev->audio_sinks);
}

static void
em_audio_channel_set(void *video,
		     int   channel)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (channel < -1) channel = -1;
   /* FIXME: a faire... */
}

static int
em_audio_channel_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ev->audio_sink_nbr;
}

static const char *
em_audio_channel_name_get(void *video,
			  int   channel)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return NULL;
}

static void
em_audio_channel_mute_set(void *video,
			  int   mute)
{
   Emotion_Gstreamer_Video *ev;
   GstElement              *volume;

   ev = (Emotion_Gstreamer_Video *)video;

   if (ev->audio_mute == mute)
     return;

   ev->audio_mute = mute;
   volume = gst_bin_get_by_name (GST_BIN (ev->pipeline), "volume");
   if (!volume) return;

   if (mute)
      g_object_set (G_OBJECT (volume), "volume", 0.0, NULL);
   else
      g_object_set (G_OBJECT (volume), "volume", ev->volume * 10.0, NULL);

   gst_object_unref (volume);
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
   GstElement              *volume;

   ev = (Emotion_Gstreamer_Video *)video;

   if (vol < 0.0)
     vol = 0.0;
   if (vol > 1.0)
     vol = 1.0;
   ev->volume = vol;
   volume = gst_bin_get_by_name (GST_BIN (ev->pipeline), "volume");
   if (!volume) return;
   g_object_set (G_OBJECT (volume), "volume",
		 vol * 10.0, NULL);
   gst_object_unref (volume);
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
em_spu_channel_count(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 0;
}

static void
em_spu_channel_set(void *video, int channel)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
}

static int
em_spu_channel_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 1;
}

static const char *
em_spu_channel_name_get(void *video, int channel)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   return NULL;
}

static void
em_spu_channel_mute_set(void *video, int mute)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
}

static int
em_spu_channel_mute_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 0;
}

static int
em_chapter_count(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   return 0;
}

static void
em_chapter_set(void *video, int chapter)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
}

static int
em_chapter_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 0;
}

static const char *
em_chapter_name_get(void *video, int chapter)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return NULL;
}

static void
em_speed_set(void *video, double speed)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
}

static double
em_speed_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 1.0;
}

static int
em_eject(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 1;
}

static const char *
em_meta_get(void *video, int meta)
{
   Emotion_Gstreamer_Video *ev;
   GstBus                  *bus;
   gchar                   *str = NULL;
   gboolean                 done;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev) return NULL;

   done = FALSE;
   bus = gst_element_get_bus (ev->pipeline);
   if (!bus) return NULL;

   while (!done) {
      GstMessage *message;

      message = gst_bus_pop (bus);
      if (message == NULL)
        /* All messages read, we're done */
         break;

      switch (GST_MESSAGE_TYPE (message)) {
      case GST_MESSAGE_ERROR:
      case GST_MESSAGE_EOS:
         gst_message_unref (message);
         break;
      case GST_MESSAGE_TAG: {
         GstTagList *new_tags;

         gst_message_parse_tag (message, &new_tags);

         switch (meta) {
         case META_TRACK_TITLE:
            gst_tag_list_get_string (new_tags, GST_TAG_TITLE, &str);
            if (str) done = TRUE;
            break;
         case META_TRACK_ARTIST:
            gst_tag_list_get_string (new_tags, GST_TAG_ARTIST, &str);
            if (str) done = TRUE;
            break;
         case META_TRACK_GENRE:
            gst_tag_list_get_string (new_tags, GST_TAG_GENRE, &str);
            if (str) done = TRUE;
            break;
         case META_TRACK_COMMENT:
            gst_tag_list_get_string (new_tags, GST_TAG_COMMENT, &str);
            if (str) done = TRUE;
            break;
         case META_TRACK_ALBUM:
            gst_tag_list_get_string (new_tags, GST_TAG_ALBUM, &str);
            if (str) done = TRUE;
            break;
         case META_TRACK_YEAR: {
            const GValue *date;

            date = gst_tag_list_get_value_index (new_tags, GST_TAG_DATE, 0);
            if (date)
               str = g_strdup_value_contents (date);
            if (str) done = TRUE;
            break;
         }
         case META_TRACK_DISCID:
#ifdef GST_TAG_CDDA_CDDB_DISCID
            gst_tag_list_get_string (new_tags, GST_TAG_CDDA_CDDB_DISCID, &str);
#endif
            if (str) done = TRUE;
            break;
         case META_TRACK_COUNT: {
            int track_count;

            track_count = emotion_pipeline_cdda_track_count_get (video);
            if (track_count > 0) {
               char buf[64];

               g_snprintf (buf, 64, "%d", track_count);
               str = g_strdup (buf);
               done = TRUE;
            }
            break;
         }
         }
         break;
      }
      default:
         break;
      }
      gst_message_unref (message);
   }

   gst_object_unref (GST_OBJECT (bus));

   return str;
}

unsigned char
module_open(Evas_Object           *obj,
	    Emotion_Video_Module **module,
	    void                 **video)
{
   if (!module)
      return 0;

   if (!em_module.init(obj, video))
      return 0;

   *module = &em_module;
   return 1;
}

void
module_close(Emotion_Video_Module *module,
	     void                 *video)
{
   em_module.shutdown(video);
}

static int
_em_fd_ev_active(void *data, Ecore_Fd_Handler *fdh)
{
   int fd;
   int len;
   void *buf[1];
   unsigned char *frame_data;
   Emotion_Gstreamer_Video *ev;
   GstBuffer  *buffer;

   ev = data;
   fd = ecore_main_fd_handler_fd_get(fdh);

   while ((len = read(fd, buf, sizeof(buf))) > 0)
     {
        if (len == sizeof(buf))
          {
             Emotion_Video_Sink *vsink;

             frame_data = buf[0];
             buffer = buf[1];
             _emotion_frame_new(ev->obj);
             vsink = (Emotion_Video_Sink *)ecore_list_goto_index (ev->video_sinks, ev->video_sink_nbr);
             _emotion_video_pos_update(ev->obj, ev->position, vsink->length_time);
          }
     }
   return 1;
}

int _eos_timer_fct (void *data)
{
   Emotion_Gstreamer_Video *ev;
   GstMessage              *msg;

   ev = (Emotion_Gstreamer_Video *)data;
   while ((msg = gst_bus_poll (ev->eos_bus, GST_MESSAGE_ERROR | GST_MESSAGE_EOS, 0))) {
     switch (GST_MESSAGE_TYPE(msg)) {
     case GST_MESSAGE_ERROR: {
       gchar *debug;
       GError *err;

       gst_message_parse_error (msg, &err, &debug);
       g_free (debug);

       g_print ("Error: %s\n", err->message);
       g_error_free (err);

       break;
     }
     case GST_MESSAGE_EOS:
       if (ev->eos_timer)
         {
           ecore_timer_del(ev->eos_timer);
           ev->eos_timer = NULL;
         }
       ev->play = 0;
       _emotion_decode_stop(ev->obj);
       _emotion_playback_finished(ev->obj);
       break;
     default:
       break;
     }
     gst_message_unref (msg);
   }
   return 1;
}
