#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Emotion.h"
#include "emotion_private.h"
#include "emotion_gstreamer.h"


static Emotion_Gstreamer_Video *em_v;

static int id_new_pad;
static int id_no_more_pad;
static int count = 0;
static int video_streams_count = 0;
static int audio_streams_count = 0;


static void _em_set_pipeline_info (Emotion_Gstreamer_Video *ev);

/* Callbacks to get stream information */
static void cb_end_of_stream (GstElement *thread,
			      gpointer    data);

static gboolean cb_idle_eos (gpointer data);

static void cb_thread_error (GstElement *thread,
			     GstElement *source,
			     GError     *error,
			     gchar      *debug,
			     gpointer    data);

static void cb_new_pad (GstElement *decodebin,
			GstPad     *pad,
			gboolean    last,
			gpointer    data);

static void cb_no_more_pad (GstElement *decodebin,
			    gpointer    data);

static void cb_caps_video_set (GObject    *obj,
			       GParamSpec *pspec,
			       gpointer    data);

static void cb_caps_audio_set (GObject    *obj,
			       GParamSpec *pspec,
			       gpointer    data);

/* Callbacks to display the frame content */

static void cb_handoff (GstElement *fakesrc,
			GstBuffer  *buffer,
			GstPad     *pad,
			gpointer    user_data);

static int   _em_fd_ev_active(void *data, Ecore_Fd_Handler *fdh);

/* Interface */

static unsigned char em_init(Evas_Object *obj,
			     void       **emotion_video);
static int em_shutdown(void *video);
static unsigned char em_file_open(const char  *file,
				  Evas_Object *obj,
				  void        *video);
static void em_file_close(void *video);
static void em_play(void *video,
		    double pos);
static void em_stop(void *video);
static void em_size_get(void *video,
			int  *width,
			int  *height);
static void em_pos_set(void  *video,
		       double pos);
static double em_len_get(void *video);
static double em_fps_get(void *video);
static double em_pos_get(void *video);
static double em_ratio_get(void *video);

static int em_seekable(void *video);
static void em_frame_done(void *video);
static Emotion_Format em_format_get(void *video);
static void em_video_data_size_get(void *video, int *w, int *h);
static int em_yuv_rows_get(void *video, int w, int h, unsigned char **yrows, unsigned char **urows, unsigned char **vrows);
static int em_bgra_data_get(void *video, unsigned char **bgra_data);
static void em_event_feed(void *video, int event);
static void em_event_mouse_button_feed(void *video, int button, int x, int y);
static void em_event_mouse_move_feed(void *video, int x, int y);

static int em_video_channel_count(void *video);
static void em_video_channel_set(void *video,
				 int   channel);
static int em_video_channel_get(void *video);
static const char *em_video_channel_name_get(void *video,
					     int   channel);
static void em_video_channel_mute_set(void *video,
				      int   mute);
static int em_video_channel_mute_get(void *video);

static int em_audio_channel_count(void *video);
static void em_audio_channel_set(void *video,
				 int   channel);
static int em_audio_channel_get(void *video);
static const char *em_audio_channel_name_get(void *video,
					     int   channel);
static void em_audio_channel_mute_set(void *video,
				      int   mute);
static int em_audio_channel_mute_get(void *video);
static void em_audio_channel_volume_set(void  *video,
					double vol);
static double em_audio_channel_volume_get(void *video);

static int em_spu_channel_count(void *video);
static void em_spu_channel_set(void *video, int channel);
static int em_spu_channel_get(void *video);
static const char *em_spu_channel_name_get(void *video, int channel);
static void em_spu_channel_mute_set(void *video, int mute);
static int em_spu_channel_mute_get(void *video);

static int em_chapter_count(void *video);
static void em_chapter_set(void *video, int chapter);
static int em_chapter_get(void *video);
static const char *em_chapter_name_get(void *video, int chapter);
static void em_speed_set(void *video, double speed);
static double em_speed_get(void *video);
static int em_eject(void *video);
static const char *em_meta_get(void *video, int meta);


static unsigned char
em_init(Evas_Object  *obj,
	void        **emotion_video)
{
   Emotion_Gstreamer_Video *ev;
   int fds[2];

   if (!emotion_video)
      return 0;

   printf ("Init gstreamer...\n");

   ev = calloc(1, sizeof(Emotion_Gstreamer_Video));
   if (!ev) return 0;
   ev->obj = obj;
   ev->obj_data = NULL;

   em_v = ev;

   /* Initialization of gstreamer */
   gst_init (NULL, NULL);

   /* Gstreamer pipeline */
   ev->pipeline = gst_thread_new ("pipeline");
   g_signal_connect (ev->pipeline, "eos", G_CALLBACK (cb_end_of_stream), ev);
   g_signal_connect (ev->pipeline, "error", G_CALLBACK (cb_thread_error), ev);

   /* We allocate the sinks lists */
   ev->video_sinks = ecore_list_new ();
   ev->audio_sinks = ecore_list_new ();

   *emotion_video = ev;

   /* Default values */
   ev->width = 1;
   ev->height = 1;
   ev->ratio = 1.0;

   ev->position = 0;

   /* Create the file descriptors */
   if (pipe(fds) == 0)
     {
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

   return 1;
}

static int
em_shutdown(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   gst_element_set_state (ev->pipeline, GST_STATE_NULL);
   gst_object_unref (GST_OBJECT (ev->pipeline));

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
   GstElement              *filesrc;
   GstElement              *decoder;

   ev = (Emotion_Gstreamer_Video *)video;
   printf ("Open file gstreamer...\n");

   /* Evas Object */
   ev->obj = obj;

   /* Gstreamer elements */
   filesrc = gst_element_factory_make ("filesrc", "source");
   g_object_set (G_OBJECT (filesrc), "location", file, NULL);
   decoder = gst_element_factory_make ("decodebin", "decoder");
   gst_bin_add_many (GST_BIN (ev->pipeline), filesrc, decoder, NULL);
   gst_element_link (filesrc, decoder);

   gst_element_set_state (ev->pipeline, GST_STATE_READY);

   /* Set the callback to get the sinks */
   id_new_pad = g_signal_connect (G_OBJECT (decoder),
				  "new-decoded-pad",
				  G_CALLBACK (cb_new_pad), ev);
   id_no_more_pad = g_signal_connect (G_OBJECT (decoder),
				      "no-more-pads",
				      G_CALLBACK (cb_no_more_pad), ev);
   /* Get the sinks */
   printf ("get sinks\n");
   gst_element_set_state (ev->pipeline, GST_STATE_PLAYING);

   /* we set the streams to the first frame */
/*    ev->get_poslen = 0; */
/*    ev->seek_to = 0; */

   return 1;
}

static void
em_file_close(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev)
     return;


   printf("EX pause end...\n");
   if (!emotion_object_play_get(ev->obj))
     {
	printf("  ... unpause\n");
	/* FIXME: do something ? */
     }
   printf("EX stop\n");
   if (ev->pipeline)
     gst_element_set_state (ev->pipeline, GST_STATE_READY);
}

static void
em_play(void   *video,
	double  pos)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   gst_element_set_state (ev->pipeline, GST_STATE_PLAYING);
}

static void
em_stop(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   gst_element_set_state (ev->pipeline, GST_STATE_PAUSED);
   ev->play = 0;
}

static void
em_size_get(void  *video,
	    int   *width,
	    int   *height)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (width) *width = ev->width;
   if (height) *height = ev->height;
}

static void
em_pos_set(void   *video,
	   double  pos)
{
   Emotion_Gstreamer_Video *ev;
   GstElement              *vsink = NULL;
   GstElement              *asink = NULL;
   GstSeekType              type;
   guint64                  time;

   ev = (Emotion_Gstreamer_Video *)video;

   if (ev->seek_to_pos == pos) return;

   type = (GstSeekType)(GST_FORMAT_TIME     |
			GST_SEEK_METHOD_SET |
			GST_SEEK_FLAG_FLUSH);

   if (ecore_list_current (ev->video_sinks))
     vsink = ((Emotion_Video_Sink *)(ecore_list_current (ev->video_sinks)))->sink;
   if (ecore_list_current (ev->audio_sinks))
     asink = ((Emotion_Audio_Sink *)(ecore_list_current (ev->audio_sinks)))->sink;


   time = (guint64)(floor (pos));

   if (vsink)
     gst_element_seek(vsink, type, time);
   if (asink)
     gst_element_seek(asink, type, time);
}

static double
em_len_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vs;

   ev = (Emotion_Gstreamer_Video *)video;

   if (!ecore_list_is_empty(ev->video_sinks))
     {
       vs = (Emotion_Video_Sink *)ecore_list_current(ev->video_sinks);
       return vs->length_time;
     }

   return 0;
}

static double
em_fps_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vs;

   ev = (Emotion_Gstreamer_Video *)video;

   /* FIXME: Maybe we can just get the fps of the current stream */
   if (!ecore_list_is_empty(ev->video_sinks))
     {
       vs = (Emotion_Video_Sink *)ecore_list_current(ev->video_sinks);
       return vs->framerate;
     }

   return 0;
}

static double
em_pos_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 0;//ev->pos;
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

   return 0;
}

static int
em_audio_handled(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 0;
}

static int
em_seekable(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 0;
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

   ev = (Emotion_Gstreamer_Video *)video;

   return  EMOTION_YV12;
}

static void
em_video_data_size_get(void *video, int *w, int *h)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   *w= ev->width;
   *h = ev->height;
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
       for (i = 0; i < h; i++)
	  yrows[i] = &ev->obj_data[i * w];

       for (i = 0; i < (h / 2); i++)
	 urows[i] = &ev->obj_data[h * w + i * (w / 2) ];

       for (i = 0; i < (h / 2); i++)
	 vrows[i] = &ev->obj_data[h * w + h * (w /4) + i * (w / 2)];

       return 1;
     }

   return 0;
}

static int
em_bgra_data_get(void *video, unsigned char **bgra_data)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return 1;
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

   return ecore_list_index(ev->video_sinks);
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

   return ecore_list_index(ev->audio_sinks);
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

   ev = (Emotion_Gstreamer_Video *)video;

   ev->audio_mute = mute;
   /* FIXME: a faire ... */
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

   if (vol < 0.0)
     vol = 0.0;
   if (vol > 100.0)
     vol = 100.0;
   g_object_set (G_OBJECT (ev->pipeline), "volume",
		 vol / 100.0, NULL);
}

static double
em_audio_channel_volume_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   double                   vol;

   ev = (Emotion_Gstreamer_Video *)video;

   g_object_get (G_OBJECT (ev->pipeline), "volume", &vol, NULL);

   return vol*100.0;
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

   ev = (Emotion_Gstreamer_Video *)video;
   return NULL;
}

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
   em_fps_get, /* fps_get */
   em_pos_get, /* pos_get */
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

#if 0
void
em_debug(Emotion_Xine_Video *ev)
{
   int has_chapters = 0;
   int max_spu = 0;
   int max_audio = 0;
   int video_channels = 0;
   int video_streams = 0;
   int video_seekable = 0;
   char *title;
   char *comment;
   char *artist;
   char *genre;
   char *album;
   char *year;
   char *cdindex_discid;
   int video_channel = 0;
   int audio_channel = 0;
   int spu_channel = 0;
   int video_ratio = 0;
   int audio_mode = 0;

//   return;
   has_chapters = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_HAS_CHAPTERS);
   max_spu = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_MAX_SPU_CHANNEL);
   max_audio = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_MAX_AUDIO_CHANNEL);
   video_channels = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_CHANNELS);
   video_streams = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_STREAMS);
   video_seekable = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_SEEKABLE);
   title = xine_get_meta_info(ev->stream, XINE_META_INFO_TITLE);
   comment = xine_get_meta_info(ev->stream, XINE_META_INFO_COMMENT);
   artist = xine_get_meta_info(ev->stream, XINE_META_INFO_ARTIST);
   genre = xine_get_meta_info(ev->stream, XINE_META_INFO_GENRE);
   album = xine_get_meta_info(ev->stream, XINE_META_INFO_ALBUM);
   year = xine_get_meta_info(ev->stream, XINE_META_INFO_YEAR);
   cdindex_discid = xine_get_meta_info(ev->stream, XINE_META_INFO_CDINDEX_DISCID);
   video_channel = xine_get_param(ev->stream, XINE_PARAM_VIDEO_CHANNEL);
   audio_channel = xine_get_param(ev->stream, XINE_PARAM_AUDIO_CHANNEL_LOGICAL);
   spu_channel = xine_get_param(ev->stream, XINE_PARAM_SPU_CHANNEL);
   video_ratio = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_VIDEO_RATIO);
   audio_mode = xine_get_stream_info(ev->stream, XINE_STREAM_INFO_AUDIO_MODE);
   printf("has_chapters = %i\n", has_chapters);
   printf("max_spu = %i\n", max_spu);
   printf("max_audio = %i\n", max_audio);
   printf("video_channels = %i\n", video_channels);
   printf("video_streams = %i\n", video_streams);
   printf("video_seekable = %i\n", video_seekable);
   printf("title = %s\n", title);
   printf("comment = %s\n", comment);
   printf("artist = %s\n", artist);
   printf("genre = %s\n", genre);
   printf("album = %s\n", album);
   printf("year = %s\n", year);
   printf("cdindex_discid = %s\n", cdindex_discid);
   printf("video_channel = %i\n", video_channel);
   printf("audio_channel = %i\n", audio_channel);
   printf("spu_channels = %i\n", spu_channel);
   printf("video_ratio = %i\n", video_ratio);
   printf("audio_mode = %i\n", audio_mode);
     {
	int i;

	for (i = 0; i <= max_audio; i++)
	  {
	     char lang[XINE_LANG_MAX + 1];

	     lang[0] = 0;
	     printf("  AUDIO %i = ", i);
	     if (xine_get_audio_lang(ev->stream, i, lang))
	       printf("%s\n", lang);
	     else
	       printf("NONE\n");
	  }
	for (i = 0; i <= max_spu; i++)
	  {
	     char lang[XINE_LANG_MAX + 1];

	     lang[0] = 0;
	     printf("  SPU %i = ", i);
	     if (xine_get_spu_lang(ev->stream, i, lang))
	       printf("%s\n", lang);
	     else
	       printf("NONE\n");
	  }
     }
}
#endif

static void
_em_set_pipeline_info (Emotion_Gstreamer_Video *ev)
{
   Emotion_Video_Sink      *video_sink;
   GstElement              *misc;

   video_sink = (Emotion_Video_Sink *)ecore_list_goto_first (ev->video_sinks);
   ev->width = video_sink->width;
   ev->height = video_sink->height;
   ev->ratio = (double)ev->width / (double)ev->height;

  _emotion_frame_new(ev->obj);
  _emotion_frame_resize(ev->obj,
			ev->width, ev->height, ev->ratio);

  /* we set the streams to the first frame */
  ev->get_poslen = 0;
  ev->seek_to = 0;

  /* we add hand-offs signal to display the video */
  misc = video_sink->sink;

  g_object_set (G_OBJECT (misc ),"signal-handoffs", TRUE, NULL);
  g_signal_connect (misc, "handoff", G_CALLBACK (cb_handoff),ev);
}


/*
 * Callbacks
 */

static void
cb_end_of_stream (GstElement *thread,
		  gpointer    data)
{
  Emotion_Gstreamer_Video* ev;

  ev = (Emotion_Gstreamer_Video *)data;
  printf ("Have eos in thread %p\n", g_thread_self ());
  g_idle_add ((GSourceFunc) cb_idle_eos, data);
}

static gboolean
cb_idle_eos (gpointer data)
{
  Emotion_Gstreamer_Video* ev;

  ev = (Emotion_Gstreamer_Video *)data;

  printf ("Have idle-func in thread %p\n", g_thread_self ());

  _emotion_playback_finished(ev->obj);

  return 0;
}

static void
cb_thread_error (GstElement *thread,
		 GstElement *source,
		 GError     *error,
		 gchar      *debug,
		 gpointer    data)
{
  printf ("Error in thread %p: %s\n", g_thread_self (), error->message);
  g_idle_add ((GSourceFunc) cb_idle_eos, NULL);
}


/*
 * Add a sink to the pipeline when it is found by the decodebin
 * element.
 */
void
cb_new_pad (GstElement *decodebin,
	    GstPad     *pad,
	    gboolean    last,
	    gpointer    data)
{
   Emotion_Gstreamer_Video *ev;
   GstCaps                 *caps;
   GstStructure            *str;
   const gchar             *mimetype;

   ev = (Emotion_Gstreamer_Video *)data;

   caps = gst_pad_get_caps (pad);
   str = gst_caps_get_structure (caps, 0);
   mimetype = gst_structure_get_name (str);

   /* test */
   printf ("\nNew Pad : %s\n", gst_structure_to_string (str));
   /* end test */

  if (g_str_has_prefix (mimetype, "audio/"))
    {
      Emotion_Audio_Sink *asink;

      GstElement         *audio_thread;
      GstElement         *audioqueue;
      GstElement         *conv;
      GstElement         *scale;
      GstPad             *audiopad;

      char                buf[1024];
      char               *namebin;

      asink = (Emotion_Audio_Sink *)malloc (sizeof (Emotion_Audio_Sink));
      if (!asink)
	return;
      if (!ecore_list_append(ev->audio_sinks, asink))
	{
	  free (asink);
	  return;
	}

      sprintf (buf, "audio_thread%d", ecore_list_nodes (ev->audio_sinks));
      namebin = strdup (buf);
      audio_thread = gst_thread_new (namebin);
      free (namebin);
      audioqueue = gst_element_factory_make ("queue", namebin);
      conv = gst_element_factory_make ("audioconvert", NULL);
      audiopad = gst_element_get_pad (conv, "sink");
      scale = gst_element_factory_make ("audioscale", NULL);
      if (audio_streams_count == 0)
        asink->sink = gst_element_factory_make ("alsasink", NULL);
      else
        asink->sink = gst_element_factory_make ("fakesink", NULL);
      gst_bin_add_many (GST_BIN (audio_thread),
			audioqueue, conv, scale, asink->sink, NULL);
      gst_element_link_many (audioqueue, conv, scale, asink->sink, NULL);


      /* Ghost pads for the thread */
      gst_element_add_ghost_pad (audio_thread,
				 gst_element_get_pad (audioqueue,
						      "sink"),
				 "sink");

      gst_pad_link (pad,
		    gst_element_get_pad (audioqueue, "sink"));

      /* We add the thread in the pipeline */
      gst_bin_add (GST_BIN (ev->pipeline), audio_thread);
      gst_bin_sync_children_state (GST_BIN (ev->pipeline));

/*       audiopad = gst_element_get_pad (ai->sink, "sink"); */
      audiopad = gst_element_get_pad (gst_pad_get_real_parent (pad),
				      "src");
      g_signal_connect (audiopad, "notify::caps",
			G_CALLBACK (cb_caps_audio_set), asink);

      audio_streams_count++;
    }
  else
    {
      if (g_str_has_prefix (mimetype, "video/"))
	{
	  Emotion_Video_Sink *vsink;

	  GstElement         *video_thread;
	  GstElement         *videoqueue;
	  GstElement         *vcs;
	  GstPad             *videopad;

	  char                buf[1024];
	  char               *name;

	  vsink = (Emotion_Video_Sink *)malloc (sizeof (Emotion_Video_Sink));
	  if (!ev->video_sinks)
	    printf ("DIABOLIC !!!\n");
	  if (!vsink)
	    return;
	  if (!ecore_list_append(ev->video_sinks, vsink))
	    {
	      free (vsink);
	      return;
	    }
	  printf ("video, sink added\n");

	  sprintf (buf, "video_thread%d", ecore_list_nodes (ev->video_sinks));
	  name = strdup (buf);
	  video_thread = gst_thread_new (name);
	  free (name);
	  videoqueue = gst_element_factory_make ("queue", NULL);
	  vcs = gst_element_factory_make ("ffmpegcolorspace", NULL);
	  videopad = gst_element_get_pad (vcs, "sink");
	  vsink->sink = gst_element_factory_make ("fakesink", NULL);
          g_object_set (G_OBJECT (vsink->sink), "sync", TRUE, NULL);
	  gst_bin_add_many (GST_BIN (video_thread),
			    videoqueue, vcs, vsink->sink, NULL);
	  gst_element_link_many (videoqueue, vcs, vsink->sink, NULL);


	  /* Ghost pads for the thread */
	  gst_element_add_ghost_pad (video_thread,
				     gst_element_get_pad (videoqueue,
							  "sink"),
				     "sink");

	  gst_pad_link (pad,
			gst_element_get_pad (videoqueue, "sink"));

	  /* We add the thread in the pipeline */
	  gst_bin_add (GST_BIN (ev->pipeline), video_thread);
	  gst_bin_sync_children_state (GST_BIN (ev->pipeline));

	  /*       audiopad = gst_element_get_pad (ai->sink, "sink");
	  */
	  videopad = gst_element_get_pad (gst_pad_get_real_parent (pad),
					  "src");
	  g_signal_connect (videopad, "notify::caps",
			    G_CALLBACK (cb_caps_video_set), vsink);

	  video_streams_count++;
	}
      else
	{
	  printf ("Unknown stream type\n");
	}
    }
}

/*
 * Stop the pipeline when there is no more streams to find.
 */
void
cb_no_more_pad (GstElement *decodebin,
		gpointer    data)
{
   printf ("no more pads\n");

   if (g_signal_handler_is_connected (G_OBJECT (decodebin), id_new_pad))
     g_signal_handler_disconnect (G_OBJECT (decodebin), id_new_pad);
   if (g_signal_handler_is_connected (G_OBJECT (decodebin), id_no_more_pad))
     g_signal_handler_disconnect (G_OBJECT (decodebin), id_no_more_pad);
}

/*
 * Fill the informations for each video streams
 */
void
cb_caps_video_set (GObject    *obj,
		   GParamSpec *pspec,
		   gpointer    data)
{
  GstStructure       *str;
  GstPad             *pad = GST_PAD (obj);
  Emotion_Video_Sink *vsink;

  if (!GST_PAD_CAPS (pad))
    return;

  vsink = (Emotion_Video_Sink *)data;

  str = gst_caps_get_structure (GST_PAD_CAPS (pad), 0);
  if (str)
    {
      gdouble       framerate;
      const GValue *val;
      gint          width;
      gint          height;

      if ((gst_structure_get_int (str, "width", &width)) &&
	  (gst_structure_get_int (str, "height", &height)))
	{
	  vsink->width = (unsigned int)width;
	  vsink->height = (unsigned int)height;
	}

      if (gst_structure_get_double (str, "framerate", &framerate))
	vsink->framerate = framerate;

      val = gst_structure_get_value (str, "pixel-aspect-ratio");
      if (val)
	{
	  vsink->par_num = (unsigned int)gst_value_get_fraction_numerator (val);
	  vsink->par_den = (unsigned int)gst_value_get_fraction_denominator (val);
	}
      printf ("width        : %d\n", width);
      printf ("height       : %d\n", height);
      printf ("frame rate   : %f\n", framerate);

      {
        GstFormat   fmt;
        gint64      len;
        GstElement *sink;

        sink = vsink->sink;
	fmt = GST_FORMAT_TIME;
	if (gst_element_query (sink, GST_QUERY_TOTAL, &fmt, &len))
	  {
	     vsink->length_time = (unsigned long long)len;

	     fmt = GST_FORMAT_DEFAULT;
	     if (gst_element_query (sink, GST_QUERY_TOTAL, &fmt, &len))
	        vsink->length_frames = (unsigned long long)len;
	     else
	        vsink->length_frames = vsink->framerate *
		  (double)vsink->length_time / 1000000000.0;

             printf ("frame count  : %lld\n", vsink->length_frames);
             count ++;
             printf ("video, count : %d, vsc : %d asc : %d\n",
                     count, video_streams_count, audio_streams_count);
	     if (count == video_streams_count + audio_streams_count)
	       {
                 _em_set_pipeline_info (em_v);
	       }
	  }
      }
    }
}

/*
 * Fill the informations for each audio streams
 */
void
cb_caps_audio_set (GObject    *obj,
		   GParamSpec *pspec,
		   gpointer    data)
{
  GstStructure       *str;
  GstPad             *pad = GST_PAD (obj);
  Emotion_Audio_Sink *asink;

  if (!GST_PAD_CAPS (pad))
    return;

  asink = (Emotion_Audio_Sink *)data;

  str = gst_caps_get_structure (GST_PAD_CAPS (pad), 0);
  if (str)
    {
      gint channels;
      gint samplerate;

      if (gst_structure_get_int (str, "channels", &channels))
	asink->channels = (unsigned int)channels;

      if (gst_structure_get_int (str, "rate", &samplerate))
	asink->samplerate = (unsigned int)samplerate;

      printf ("channels     : %d\n", channels);
      printf ("sample rate  : %d\n", samplerate);

      {
        GstFormat   fmt;
        gint64      len;
        GstElement *sink;

        sink = asink->sink;
        fmt = GST_FORMAT_TIME;
        if (gst_element_query (sink, GST_QUERY_TOTAL, &fmt, &len))
          {
            asink->length_time = (unsigned long long)len;

            fmt = GST_FORMAT_DEFAULT;
            if (gst_element_query (sink, GST_QUERY_TOTAL, &fmt, &len))
              asink->length_samples = (unsigned long long)len;
            else
              asink->length_samples = asink->samplerate *
                (double)asink->length_time / 1000000000.0;

            printf ("sample count : %lld\n", asink->length_samples);
            count ++;
            printf ("audio, count : %d, vsc : %d asc : %d\n",
                    count, video_streams_count, audio_streams_count);
            if (count == video_streams_count + audio_streams_count)
              {
                _em_set_pipeline_info (em_v);
              }
          }
      }
    }
}

/* Send the video frame to the evas object */
static void
cb_handoff (GstElement *fakesrc,
	    GstBuffer  *buffer,
	    GstPad     *pad,
	    gpointer    user_data)
{
   void *buf[2];

   Emotion_Gstreamer_Video *ev = ( Emotion_Gstreamer_Video *) user_data;


  if (!ev->obj_data)
    ev->obj_data = (void*) malloc (GST_BUFFER_SIZE(buffer) * sizeof(void));

  memcpy ( ev->obj_data, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));
  buf[0] = GST_BUFFER_DATA(buffer);
  buf[1] = buffer;
  write(ev->fd_ev_write, buf, sizeof(buf));

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
          frame_data = buf[0];
          buffer = buf[1];
          _emotion_frame_new(ev->obj);
          len = ((Emotion_Video_Sink *)ecore_list_goto_first(ev->video_sinks))->length_time;
          _emotion_video_pos_update(ev->obj, ev->position, len);

        }
    }
   return 1;
}
