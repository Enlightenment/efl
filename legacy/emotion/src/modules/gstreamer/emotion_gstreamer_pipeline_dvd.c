/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "emotion_gstreamer.h"
#include "emotion_gstreamer_pipeline.h"


static void dvd_pad_added_cb (GstElement *dvddemuxer,
                              GObject    *new_pad,
                              gpointer    user_data);

static void dvd_no_more_pads_cb (GstElement *dvddemuxer,
                                 gpointer    user_data);

static int no_more_pads = 0;


int
emotion_pipeline_dvd_build (void *video, const char *device)
{
   GstElement              *dvdreadsrc;
   GstElement              *dvddemux;
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev) return 0;

   dvdreadsrc = gst_element_factory_make ("dvdreadsrc", "src");
   if (!dvdreadsrc)
     goto failure_dvdreadsrc;
   if (device)
     g_object_set (G_OBJECT (dvdreadsrc), "device", device, NULL);

   dvddemux = gst_element_factory_make ("dvddemux", "dvddemux");
   if (!dvddemux)
     goto failure_dvddemux;
   g_signal_connect (dvddemux, "pad-added",
                     G_CALLBACK (dvd_pad_added_cb), ev);
   g_signal_connect (dvddemux, "no-more-pads",
                     G_CALLBACK (dvd_no_more_pads_cb), ev);

   gst_bin_add_many (GST_BIN (ev->pipeline), dvdreadsrc, dvddemux, NULL);
   if (!gst_element_link (dvdreadsrc, dvddemux))
     goto failure_link;

   if (!emotion_pipeline_pause (ev->pipeline))
     goto failure_gstreamer_pause;

   while (no_more_pads == 0) {
   g_print ("toto\n");}
   no_more_pads = 0;

   /* We get the informations of streams */
   ecore_list_first_goto (ev->video_sinks);
   ecore_list_first_goto (ev->audio_sinks);

   {
      GstIterator *it;
      gpointer     data;

      it = gst_element_iterate_src_pads (dvddemux);
      while (gst_iterator_next (it, &data) == GST_ITERATOR_OK) {
         GstPad  *pad;
         GstCaps *caps;
         gchar   *str;

         pad = GST_PAD (data);

         caps = gst_pad_get_caps (pad);
         str = gst_caps_to_string (caps);
         g_print ("caps !! %s\n", str);
         /* video stream */
         if (g_str_has_prefix (str, "video/mpeg")) {
            Emotion_Video_Sink *vsink;
            GstPad             *sink_pad;
            GstCaps            *sink_caps;

            vsink = (Emotion_Video_Sink *)ecore_list_next (ev->video_sinks);
            sink_pad = gst_element_get_pad (gst_bin_get_by_name (GST_BIN (ev->pipeline), "mpeg2dec"), "src");
            sink_caps = gst_pad_get_caps (sink_pad);
            str = gst_caps_to_string (sink_caps);
            g_print (" ** caps v !! %s\n", str);

            emotion_video_sink_fill (vsink, sink_pad, sink_caps);

            gst_caps_unref (sink_caps);
            gst_object_unref (sink_pad);
         }
         /* audio stream */
         else if (g_str_has_prefix (str, "audio/")) {
            Emotion_Audio_Sink *asink;
            GstPad             *sink_pad;
            GstCaps            *sink_caps;

            asink = (Emotion_Audio_Sink *)ecore_list_next (ev->audio_sinks);
            sink_pad = gst_element_get_pad (gst_bin_get_by_name (GST_BIN (ev->pipeline), "a52dec"), "src");
            sink_caps = gst_pad_get_caps (sink_pad);

            emotion_audio_sink_fill (asink, sink_pad, sink_caps);
         }
         gst_caps_unref (caps);
         g_free (str);
         gst_object_unref (pad);
      }
      gst_iterator_free (it);
   }

   /* The first vsink is a valid Emotion_Video_Sink * */
   /* If no video stream is found, it's a visualisation sink */
   {
      Emotion_Video_Sink *vsink;

      vsink = (Emotion_Video_Sink *)ecore_list_first_goto (ev->video_sinks);
      if (vsink && vsink->sink) {
         g_object_set (G_OBJECT (vsink->sink), "sync", TRUE, NULL);
         g_object_set (G_OBJECT (vsink->sink), "signal-handoffs", TRUE, NULL);
         g_signal_connect (G_OBJECT (vsink->sink),
                           "handoff",
                           G_CALLBACK (cb_handoff), ev);
      }
   }

   return 1;

 failure_gstreamer_pause:
 failure_link:
   gst_element_set_state (ev->pipeline, GST_STATE_NULL);
   gst_bin_remove (GST_BIN (ev->pipeline), dvddemux);
 failure_dvddemux:
   gst_bin_remove (GST_BIN (ev->pipeline), dvdreadsrc);
 failure_dvdreadsrc:

   return 0;
}

static void
dvd_pad_added_cb (GstElement *dvddemuxer,
                  GObject    *new_pad,
                  gpointer    user_data)
{
   Emotion_Gstreamer_Video *ev;
   GstCaps                 *caps;
   gchar                   *str;

   ev = (Emotion_Gstreamer_Video *)user_data;
   caps = gst_pad_get_caps (GST_PAD (new_pad));
   str = gst_caps_to_string (caps);
   /* video stream */
   if (g_str_has_prefix (str, "video/mpeg")) {
      Emotion_Video_Sink *vsink;
      GstElement         *queue;
      GstElement         *decoder;
      GstPad             *videopad;

      vsink = (Emotion_Video_Sink *)malloc (sizeof (Emotion_Video_Sink));
      if (!vsink) return;
      if (!ecore_list_append (ev->video_sinks, vsink)) {
         free(vsink);
         return;
      }

      queue = gst_element_factory_make ("queue", NULL);
      decoder = gst_element_factory_make ("mpeg2dec", "mpeg2dec");
      vsink->sink = gst_element_factory_make ("fakesink", "videosink");
      gst_bin_add_many (GST_BIN (ev->pipeline), queue, decoder, vsink->sink, NULL);
      gst_element_link (queue, decoder);
      gst_element_link (decoder, vsink->sink);
      videopad = gst_element_get_pad (queue, "sink");
      gst_pad_link (GST_PAD (new_pad), videopad);
      gst_object_unref (videopad);
      if (ecore_list_count(ev->video_sinks) == 1) {
         ev->ratio = (double)vsink->width / (double)vsink->height;
      }
      gst_element_set_state (queue, GST_STATE_PAUSED);
      gst_element_set_state (decoder, GST_STATE_PAUSED);
      gst_element_set_state (vsink->sink, GST_STATE_PAUSED);
   }
   /* audio stream */
   else if (g_str_has_prefix (str, "audio/")) {
     Emotion_Audio_Sink *asink;
     GstElement         *queue;
     GstElement         *decoder;
     GstElement         *conv;
     GstElement         *resample;
     GstElement         *volume;
     GstPad             *audiopad;
     double              vol;

      asink = (Emotion_Audio_Sink *)malloc (sizeof (Emotion_Audio_Sink));
      if (!asink) return;
      if (!ecore_list_append (ev->audio_sinks, asink)) {
         free(asink);
         return;
      }

     queue = gst_element_factory_make ("queue", NULL);
     decoder = gst_element_factory_make ("a52dec", "a52dec");
     conv = gst_element_factory_make ("audioconvert", NULL);
     resample = gst_element_factory_make ("audioresample", NULL);
     volume = gst_element_factory_make ("volume", "volume");
     g_object_get (G_OBJECT (volume), "volume", &vol, NULL);
     ev->volume = vol / 10.0;

     /* FIXME: must manage several audio streams */
     asink->sink = gst_element_factory_make ("fakesink", NULL);

     gst_bin_add_many (GST_BIN (ev->pipeline),
                       queue, decoder, conv, resample, volume, asink->sink, NULL);
     gst_element_link_many (queue, decoder, conv, resample, volume, asink->sink, NULL);

     audiopad = gst_element_get_pad (queue, "sink");
     gst_pad_link (GST_PAD (new_pad), audiopad);
     gst_object_unref (audiopad);

     gst_element_set_state (queue, GST_STATE_PAUSED);
     gst_element_set_state (decoder, GST_STATE_PAUSED);
     gst_element_set_state (conv, GST_STATE_PAUSED);
     gst_element_set_state (resample, GST_STATE_PAUSED);
     gst_element_set_state (volume, GST_STATE_PAUSED);
     gst_element_set_state (asink->sink, GST_STATE_PAUSED);
   }
}

static void
dvd_no_more_pads_cb (GstElement *dvddemuxer,
                     gpointer    user_data)
{
  no_more_pads = 1;
}
