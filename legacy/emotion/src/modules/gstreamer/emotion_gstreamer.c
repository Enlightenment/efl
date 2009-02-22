/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <unistd.h>
#include <fcntl.h>

#include "Emotion.h"
#include "emotion_private.h"
#include "emotion_gstreamer.h"
#include "emotion_gstreamer_pipeline.h"


/* Callbacks to get the eos */
static int  _eos_timer_fct   (void *data);
static void _em_buffer_read(void *data, void *buffer, unsigned int nbyte);
static void _for_each_tag    (GstTagList const* list, gchar const* tag, void *data);
static void _free_metadata   (Emotion_Gstreamer_Metadata *m);

/* Interface */

static unsigned char  em_init                     (Evas_Object     *obj,
                                                   void           **emotion_video,
						   Emotion_Module_Options *opt);

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


static double         em_len_get                  (void            *video);

static int            em_fps_num_get              (void            *video);

static int            em_fps_den_get              (void            *video);

static double         em_fps_get                  (void            *video);

static double         em_pos_get                  (void            *video);

static void           em_vis_set                  (void            *video,
                                                   Emotion_Vis      vis);

static Emotion_Vis    em_vis_get                  (void            *video);

static Evas_Bool      em_vis_supported            (void            *video,
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

static unsigned char
em_init(Evas_Object  *obj,
	void        **emotion_video,
	Emotion_Module_Options *opt)
{
   Emotion_Gstreamer_Video *ev;
   GError                  *error;
   int                      fds[2];

   if (!emotion_video)
     return 0;

   ev = calloc(1, sizeof(Emotion_Gstreamer_Video));
   if (!ev) return 0;

   ev->obj = obj;
   ev->obj_data = NULL;

   /* Initialization of gstreamer */
   if (!gst_init_check(NULL, NULL, &error))
     goto failure_gstreamer;

   /* We allocate the sinks lists */
   ev->video_sinks = ecore_list_new();
   if (!ev->video_sinks)
     goto failure_video_sinks;
   ecore_list_free_cb_set(ev->video_sinks, ECORE_FREE_CB(free));
   ev->audio_sinks = ecore_list_new();
   if (!ev->audio_sinks)
     goto failure_audio_sinks;
   ecore_list_free_cb_set(ev->audio_sinks, ECORE_FREE_CB(free));

   *emotion_video = ev;

   /* Default values */
   ev->ratio = 1.0;
   ev->video_sink_nbr = 0;
   ev->audio_sink_nbr = 0;
   ev->vis = EMOTION_VIS_NONE;
   ev->volume = 0.8;

   /* Create the file descriptors */
   ev->pipe = ecore_pipe_add (_em_buffer_read, ev);
   if (!ev->pipe)
     goto failure_pipe;

   return 1;

failure_pipe:
   ecore_list_destroy(ev->audio_sinks);
failure_audio_sinks:
   ecore_list_destroy(ev->video_sinks);
failure_video_sinks:
failure_gstreamer:
   free(ev);

   return 0;
}

static int
em_shutdown(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev)
     return 0;

   ecore_pipe_del(ev->pipe);

   /* FIXME: and the evas object ? */
   if (ev->obj_data) free(ev->obj_data);

   ecore_list_destroy(ev->video_sinks);
   ecore_list_destroy(ev->audio_sinks);

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

   ev->pipeline = gst_pipeline_new("pipeline");
   if (!ev->pipeline)
     return 0;

   ev->eos_bus = gst_pipeline_get_bus(GST_PIPELINE(ev->pipeline));
   if (!ev->eos_bus)
     {
	gst_object_unref(ev->pipeline);
	return 0;
     }

   /* Evas Object */
   ev->obj = obj;

   /* CD Audio */
   if (strstr(file, "cdda://"))
     {
	const char  *device = NULL;
	unsigned int track = 1;

	device = file + strlen("cdda://");
	if (device[0] == '/')
	  {
	     char *tmp;

	     if ((tmp = strchr(device, '?')) || (tmp = strchr(device, '#')))
	       {
		  sscanf(tmp + 1, "%d", &track);
		  tmp[0] = '\0';
	       }
	  }
	else
	  {
	     device = NULL;
	     sscanf(file, "cdda://%d", &track);
	  }
	fprintf(stderr, "[Emotion] [gst] build CD Audio pipeline\n");
	if (!(emotion_pipeline_cdda_build(ev, device, track)))
	  {
	     fprintf(stderr, "[Emotion] [gst] error while building CD Audio pipeline\n");
	     gst_object_unref(ev->pipeline);
	     return 0;
	  }
     }
   /* Dvd */
   else if (strstr(file, "dvd://"))
     {

	fprintf(stderr, "[Emotion] [gst] build DVD pipeline\n");
	if (!(emotion_pipeline_dvd_build(ev, NULL)))
	  {
	     fprintf(stderr, "[Emotion] [gst] error while building DVD pipeline\n");
	     gst_object_unref(ev->pipeline);
	     return 0;
	  }
     }
   /* http */
   else if (strstr(file, "http://"))
     {
	fprintf(stderr, "[Emotion] [gst] build URI pipeline\n");
	if (!(emotion_pipeline_uri_build(ev, file)))
	  {
	     fprintf(stderr, "[Emotion] [gst] error while building URI pipeline\n");
	     gst_object_unref(ev->pipeline);
	     return 0;
	  }
     }
   /* v4l */
   else if (strstr(file, "v4l://"))
     {
	fprintf(stderr, "[Emotion] [gst] build V4L pipeline\n");
	if (!(emotion_pipeline_v4l_build(ev, file)))
	  {
	     fprintf(stderr, "[Emotion] [gst] error while building V4L pipeline\n");
	     gst_object_unref(ev->pipeline);
	     return 0;
	  }
     }
   /* Normal media file */
   else
     {
	const char *filename;

	filename = strstr(file, "file://")
		   ? file + strlen("file://")
		   : file;

	fprintf(stderr, "[Emotion] [gst] build file pipeline\n");
	if (!(emotion_pipeline_file_build(ev, filename)))
	  {
	     fprintf(stderr, "[Emotion] [gst] error while building File pipeline\n");
	     gst_object_unref(ev->pipeline);
	     return 0;
	  }
     }

   ev->position = 0.0;

     {
	/* on recapitule : */
	Emotion_Video_Sink *vsink;
	Emotion_Audio_Sink *asink;

	vsink = (Emotion_Video_Sink *)ecore_list_first_goto(ev->video_sinks);
	if (vsink)
	  {
	     fprintf(stderr, "video : \n");
	     fprintf(stderr, "  size   : %dx%d\n", vsink->width, vsink->height);
	     fprintf(stderr, "  fps    : %d/%d\n", vsink->fps_num, vsink->fps_den);
	     fprintf(stderr, "  fourcc : %" GST_FOURCC_FORMAT "\n", GST_FOURCC_ARGS(vsink->fourcc));
	     fprintf(stderr, "  length : %" GST_TIME_FORMAT "\n\n",
		     GST_TIME_ARGS((guint64)(vsink->length_time * GST_SECOND)));
	  }

	asink = (Emotion_Audio_Sink *)ecore_list_first_goto(ev->audio_sinks);
	if (asink)
	  {
	     fprintf(stderr, "audio : \n");
	     fprintf(stderr, "  chan   : %d\n", asink->channels);
	     fprintf(stderr, "  rate   : %d\n", asink->samplerate);
	     fprintf(stderr, "  length : %" GST_TIME_FORMAT "\n\n",
		     GST_TIME_ARGS((guint64)(asink->length_time * GST_SECOND)));
	  }
     }

   if (ev->metadata)
     _free_metadata(ev->metadata);
   ev->metadata = calloc(1, sizeof(Emotion_Gstreamer_Metadata));

  em_audio_channel_volume_set(ev, ev->volume);

   _eos_timer_fct(ev);

   return 1;
}

static void
em_file_close(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev)
     return;

   /* we clear the sink lists */
   ecore_list_clear(ev->video_sinks);
   ecore_list_clear(ev->audio_sinks);

   /* shutdown eos */
   if (ev->eos_timer)
     {
	ecore_timer_del(ev->eos_timer);
	ev->eos_timer = NULL;
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

   if (ev->pipeline)
     {
	gst_element_set_state(ev->pipeline, GST_STATE_NULL);
	gst_object_unref(ev->pipeline);
	ev->pipeline = NULL;
     }
}

static void
em_play(void   *video,
	double  pos)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);
   ev->play = 1;

   /* eos */
   ev->eos_timer = ecore_timer_add(0.1, _eos_timer_fct, ev);
}

static void
em_stop(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   /* shutdown eos */
   if (ev->eos_timer)
     {
	ecore_timer_del(ev->eos_timer);
	ev->eos_timer = NULL;
     }

   gst_element_set_state(ev->pipeline, GST_STATE_PAUSED);
   ev->play = 0;
}

static void
em_size_get(void  *video,
	    int   *width,
	    int   *height)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_index_goto(ev->video_sinks, ev->video_sink_nbr);
   if (vsink)
     {
	if (width) *width = vsink->width;
	if (height) *height = vsink->height;
     }
   else
     {
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

   vsink = (Emotion_Video_Sink *)ecore_list_index_goto(ev->video_sinks, ev->video_sink_nbr);
   asink = (Emotion_Audio_Sink *)ecore_list_index_goto(ev->video_sinks, ev->audio_sink_nbr);

   if (vsink)
     {
	gst_element_seek(vsink->sink, 1.0,
			 GST_FORMAT_TIME,
			 GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH,
			 GST_SEEK_TYPE_SET,
			 (gint64)(pos * (double)GST_SECOND),
			 GST_SEEK_TYPE_NONE, -1);
     }
   if (asink)
     {
	gst_element_seek(asink->sink, 1.0,
			 GST_FORMAT_TIME,
			 GST_SEEK_FLAG_ACCURATE | GST_SEEK_FLAG_FLUSH,
			 GST_SEEK_TYPE_SET,
			 (gint64)(pos * (double)GST_SECOND),
			 GST_SEEK_TYPE_NONE, -1);
     }
}

static double
em_len_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink *vsink;
   Emotion_Audio_Sink *asink;
   GstFormat fmt;
   gint64 val;
   gboolean ret;

   ev = video;
   fmt = GST_FORMAT_TIME;
   ret = gst_element_query_duration(ev->pipeline, &fmt, &val);
   if (!ret)
     goto fallback;

   if (fmt != GST_FORMAT_TIME)
     {
	fprintf(stderr, "requrested duration in time, but got %s instead.",
		gst_format_get_name(fmt));
	goto fallback;
     }

   if (val <= 0.0)
     goto fallback;

   return val / 1000000000.0;

 fallback:
   ecore_list_first_goto(ev->audio_sinks);
   while ((asink = ecore_list_next(ev->audio_sinks)) != NULL)
     if (asink->length_time >= 0)
       return asink->length_time;

   ecore_list_first_goto(ev->video_sinks);
   while ((vsink = ecore_list_next(ev->video_sinks)) != NULL)
     if (vsink->length_time >= 0)
       return vsink->length_time;

   return 0.0;
}

static int
em_fps_num_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_index_goto(ev->video_sinks, ev->video_sink_nbr);
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

   vsink = (Emotion_Video_Sink *)ecore_list_index_goto(ev->video_sinks, ev->video_sink_nbr);
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

   vsink = (Emotion_Video_Sink *)ecore_list_index_goto(ev->video_sinks, ev->video_sink_nbr);
   if (vsink)
     return (double)vsink->fps_num / (double)vsink->fps_den;

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
   ret = gst_element_query_position(ev->pipeline, &fmt, &val);
   if (!ret)
     return ev->position;

   if (fmt != GST_FORMAT_TIME)
     {
	fprintf(stderr, "requrested position in time, but got %s instead.",
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

   if (ev->vis == vis) return;
   ev->vis = vis;
}

static Emotion_Vis
em_vis_get(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   return ev->vis;
}

static Evas_Bool
em_vis_supported(void *ef, Emotion_Vis vis)
{
   const char *name;
   GstElementFactory *factory;

   if (vis == EMOTION_VIS_NONE)
     return 1;

   name = emotion_visualization_element_name_get(vis);
   if (!name)
     return 0;

   factory = gst_element_factory_find(name);
   if (!factory)
     return 0;

   gst_object_unref(factory);
   return 1;
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

   if (ecore_list_empty_is(ev->video_sinks))
     return 0;

   return 1;
}

static int
em_audio_handled(void *video)
{
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;

   if (ecore_list_empty_is(ev->audio_sinks))
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
em_format_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_index_goto(ev->video_sinks, ev->video_sink_nbr);
   if (vsink)
     {
	switch (vsink->fourcc)
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
   Emotion_Video_Sink      *vsink;

   ev = (Emotion_Gstreamer_Video *)video;

   vsink = (Emotion_Video_Sink *)ecore_list_index_goto(ev->video_sinks, ev->video_sink_nbr);
   if (vsink)
     {
	*w = vsink->width;
	*h = vsink->height;
     }
   else
     {
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
	if (em_format_get(video) == EMOTION_FORMAT_I420)
	  {
	     for (i = 0; i < h; i++)
	       yrows[i] = &ev->obj_data[i * w];

	     for (i = 0; i < (h / 2); i++)
	       urows[i] = &ev->obj_data[h * w + i * (w / 2)];

	     for (i = 0; i < (h / 2); i++)
	       vrows[i] = &ev->obj_data[h * w + h * (w /4) + i * (w / 2)];
	  }
	else if (em_format_get(video) == EMOTION_FORMAT_YV12)
	  {
	     for (i = 0; i < h; i++)
	       yrows[i] = &ev->obj_data[i * w];

	     for (i = 0; i < (h / 2); i++)
	       vrows[i] = &ev->obj_data[h * w + i * (w / 2)];

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

   if (ev->obj_data && em_format_get(video) == EMOTION_FORMAT_BGRA)
     {
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

   return ecore_list_count(ev->video_sinks);
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

   return ecore_list_count(ev->audio_sinks);
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
   volume = gst_bin_get_by_name(GST_BIN(ev->pipeline), "volume");
   if (!volume) return;

   if (mute)
     g_object_set(G_OBJECT(volume), "volume", 0.0, NULL);
   else
     g_object_set(G_OBJECT(volume), "volume", ev->volume, NULL);

   gst_object_unref(volume);
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
   volume = gst_bin_get_by_name(GST_BIN(ev->pipeline), "volume");
   if (!volume) return;
   g_object_set(G_OBJECT(volume), "volume", vol, NULL);
   gst_object_unref(volume);
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
   const char *str;

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

EAPI unsigned char
module_open(Evas_Object           *obj,
	    Emotion_Video_Module **module,
	    void                 **video,
	    Emotion_Module_Options *opt)
{
   if (!module)
     return 0;

   if (!em_module.init(obj, video, opt))
     return 0;

   *module = &em_module;
   return 1;
}

EAPI void
module_close(Emotion_Video_Module *module,
	     void                 *video)
{
   em_module.shutdown(video);
}

static void
_for_each_tag(GstTagList const* list,
		    gchar const* tag,
		    void *data)
{
   Emotion_Gstreamer_Video *ev;
   const GValue *val;
   int i;
   int count;


   ev = (Emotion_Gstreamer_Video*)data;

   if (!ev || !ev->metadata) return;

   count = gst_tag_list_get_tag_size(list, tag);
   val = gst_tag_list_get_value_index(list, tag, 0);

   for (i = 0; i < count; i++)
     {
	if (!strcmp(tag, GST_TAG_TITLE))
	  {
	     char *str;
	     if (ev->metadata->title) g_free(ev->metadata->title);
	     if (gst_tag_list_get_string(list, GST_TAG_TITLE, &str))
	       ev->metadata->title = str;
	     else
	       ev->metadata->title = NULL;
	     break;
	  }
	if (!strcmp(tag, GST_TAG_ALBUM))
	  {
	     gchar *str;
	     if (ev->metadata->album) g_free(ev->metadata->album);
	     if (gst_tag_list_get_string(list, GST_TAG_ALBUM, &str))
	       ev->metadata->album = str;
	     else
	       ev->metadata->album = NULL;
	     break;
	  }
	if (!strcmp(tag, GST_TAG_ARTIST))
	  {
	     gchar *str;
	     if (ev->metadata->artist) g_free( ev->metadata->artist);
	     if (gst_tag_list_get_string(list, GST_TAG_ARTIST, &str))
	       ev->metadata->artist = str;
	     else
	       ev->metadata->artist = NULL;
	     break;
	  }
	if (!strcmp(tag, GST_TAG_GENRE))
	  {
	     gchar *str;
	     if (ev->metadata->genre) g_free( ev->metadata->genre);
	     if (gst_tag_list_get_string(list, GST_TAG_GENRE, &str))
	       ev->metadata->genre = str;
	     else
	       ev->metadata->genre = NULL;
	     break;
	  }
	if (!strcmp(tag, GST_TAG_COMMENT))
	  {
	     gchar *str;
	     if (ev->metadata->comment) g_free(ev->metadata->comment);
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
	     if (ev->metadata->year) g_free(ev->metadata->year);
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
	     if (ev->metadata->count) g_free( ev->metadata->count);
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
	     if (ev->metadata->disc_id) g_free(ev->metadata->disc_id);
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

  if (m->title)   g_free(m->title);
  if (m->album)   g_free(m->album);
  if (m->artist)  g_free(m->artist);
  if (m->genre)   g_free(m->genre);
  if (m->comment) g_free(m->comment);
  if (m->year)    g_free(m->year);
  if (m->count)   g_free(m->count);
  if (m->disc_id) g_free(m->disc_id);

  free(m);
}

static void
_em_buffer_read(void *data, void *buf, unsigned int nbyte)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Sink      *vsink;
   GstBuffer               *buffer;

   ev = (Emotion_Gstreamer_Video *)data;
   buffer = *((GstBuffer **)buf);
   _emotion_frame_new(ev->obj);
   vsink = (Emotion_Video_Sink *)ecore_list_index_goto(ev->video_sinks, ev->video_sink_nbr);
   if (vsink) _emotion_video_pos_update(ev->obj, ev->position, vsink->length_time);
}

static int
_eos_timer_fct(void *data)
{
   Emotion_Gstreamer_Video *ev;
   GstMessage              *msg;

   ev = (Emotion_Gstreamer_Video *)data;
   while ((msg = gst_bus_poll(ev->eos_bus, GST_MESSAGE_ERROR | GST_MESSAGE_EOS | GST_MESSAGE_TAG, 0)))
     {
	switch (GST_MESSAGE_TYPE(msg))
	  {
	   case GST_MESSAGE_ERROR:
		{
		   gchar *debug;
		   GError *err;

		   gst_message_parse_error(msg, &err, &debug);
		   g_free(debug);

		   g_print("Error: %s\n", err->message);
		   g_error_free(err);

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
	   case GST_MESSAGE_TAG:
		{
		   GstTagList *new_tags;
		   gst_message_parse_tag(msg, &new_tags);
		   if (new_tags)
		     {
			gst_tag_list_foreach(new_tags, (GstTagForeachFunc)_for_each_tag, ev);
			gst_tag_list_free(new_tags);
		     }
		   break;
		}
	   default:
	      break;
	  }
	gst_message_unref(msg);
     }
   return 1;
}
