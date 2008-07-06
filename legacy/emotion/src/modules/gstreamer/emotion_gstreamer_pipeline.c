/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <unistd.h>
#include <fcntl.h>

#include "emotion_private.h"
#include "emotion_gstreamer.h"
#include "emotion_gstreamer_pipeline.h"


gboolean
emotion_pipeline_pause (GstElement *pipeline)
{
   GstStateChangeReturn res;

   res = gst_element_set_state ((pipeline), GST_STATE_PAUSED);
   if (res == GST_STATE_CHANGE_FAILURE) {
     g_print ("Emotion-Gstreamer ERROR: could not pause\n");
     return 0;
   }

   res = gst_element_get_state ((pipeline), NULL, NULL, GST_CLOCK_TIME_NONE);
   if (res != GST_STATE_CHANGE_SUCCESS) {
     g_print ("Emotion-Gstreamer ERROR: could not complete pause\n");
     return 0;
   }

   return 1;
}

/* Send the video frame to the evas object */
void
cb_handoff (GstElement *fakesrc,
	    GstBuffer  *buffer,
	    GstPad     *pad,
	    gpointer    user_data)
{
   GstQuery *query;
   void *buf[2];

   Emotion_Gstreamer_Video *ev = ( Emotion_Gstreamer_Video *) user_data;
   if (!ev)
     return;

   if (!ev->video_mute) {
      if (!ev->obj_data)
         ev->obj_data = (void*) malloc (GST_BUFFER_SIZE(buffer) * sizeof(void));

      memcpy ( ev->obj_data, GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));
      buf[0] = GST_BUFFER_DATA(buffer);
      buf[1] = buffer;
      write(ev->fd_ev_write, buf, sizeof(buf));
   }
   else {
     Emotion_Audio_Sink *asink;
     asink = (Emotion_Audio_Sink *)ecore_list_index_goto (ev->audio_sinks, ev->audio_sink_nbr);
     _emotion_video_pos_update(ev->obj, ev->position, asink->length_time);
   }

   query = gst_query_new_position (GST_FORMAT_TIME);
   if (gst_pad_query (gst_pad_get_peer (pad), query)) {
      gint64 position;

      gst_query_parse_position (query, NULL, &position);
      ev->position = (double)position / (double)GST_SECOND;
   }
   gst_query_unref (query);
}

void
file_new_decoded_pad_cb (GstElement *decodebin,
                         GstPad     *new_pad,
                         gboolean    last,
                         gpointer    user_data)
{
   Emotion_Gstreamer_Video *ev;
   GstCaps *caps;
   gchar   *str;

   ev = (Emotion_Gstreamer_Video *)user_data;
   caps = gst_pad_get_caps (new_pad);
   str = gst_caps_to_string (caps);
   /* video stream */
   if (g_str_has_prefix (str, "video/")) {
      Emotion_Video_Sink *vsink;
      GstElement         *queue;
      GstPad             *videopad;

      vsink = (Emotion_Video_Sink *)malloc (sizeof (Emotion_Video_Sink));
      if (!vsink) return;
      if (!ecore_list_append (ev->video_sinks, vsink)) {
         free(vsink);
         return;
      }

      queue = gst_element_factory_make ("queue", NULL);
      vsink->sink = gst_element_factory_make ("fakesink", "videosink");
      gst_bin_add_many (GST_BIN (ev->pipeline), queue, vsink->sink, NULL);
      gst_element_link (queue, vsink->sink);
      videopad = gst_element_get_pad (queue, "sink");
      gst_pad_link (new_pad, videopad);
      gst_object_unref (videopad);
      if (ecore_list_count(ev->video_sinks) == 1) {
         ev->ratio = (double)vsink->width / (double)vsink->height;
      }
      gst_element_set_state (queue, GST_STATE_PAUSED);
      gst_element_set_state (vsink->sink, GST_STATE_PAUSED);
   }
   /* audio stream */
   else if (g_str_has_prefix (str, "audio/")) {
      Emotion_Audio_Sink *asink;
      GstPad             *audiopad;

      asink = (Emotion_Audio_Sink *)malloc (sizeof (Emotion_Audio_Sink));
      if (!asink) return;
      if (!ecore_list_append (ev->audio_sinks, asink)) {
         free(asink);
         return;
      }

      asink->sink = emotion_audio_sink_create (ev, ecore_list_index (ev->audio_sinks));
      gst_bin_add (GST_BIN (ev->pipeline), asink->sink);
      audiopad = gst_element_get_pad (asink->sink, "sink");
      gst_pad_link(new_pad, audiopad);
      gst_element_set_state (asink->sink, GST_STATE_PAUSED);
   }

   free(str);
}

Emotion_Video_Sink *
emotion_video_sink_new (Emotion_Gstreamer_Video *ev)
{
   Emotion_Video_Sink *vsink;

   if (!ev) return NULL;

   vsink = (Emotion_Video_Sink *)malloc (sizeof (Emotion_Video_Sink));
   if (!vsink) return NULL;

   if (!ecore_list_append (ev->video_sinks, vsink)) {
     free (vsink);
     return NULL;
   }
   return vsink;
}

void
emotion_video_sink_free (Emotion_Gstreamer_Video *ev, Emotion_Video_Sink *vsink)
{
   if (!ev || !vsink) return;

   if (ecore_list_goto (ev->video_sinks, vsink)) {
      ecore_list_remove (ev->video_sinks);
      free (vsink);
   }
}

Emotion_Video_Sink *
emotion_visualization_sink_create (Emotion_Gstreamer_Video *ev, Emotion_Audio_Sink *asink)
{
   Emotion_Video_Sink *vsink;

   if (!ev) return NULL;

   vsink = emotion_video_sink_new (ev);
   if (!vsink) return NULL;

   vsink->sink = gst_bin_get_by_name (GST_BIN (asink->sink), "vissink1");
   if (!vsink->sink) {
      emotion_video_sink_free (ev, vsink);
      return NULL;
   }
   vsink->width = 320;
   vsink->height = 200;
   ev->ratio = (double)vsink->width / (double)vsink->height;
   vsink->fps_num = 25;
   vsink->fps_den = 1;
   vsink->fourcc = GST_MAKE_FOURCC ('A','R','G','B');
   vsink->length_time = asink->length_time;

   g_object_set (G_OBJECT (vsink->sink), "sync", TRUE, NULL);
   g_object_set (G_OBJECT (vsink->sink), "signal-handoffs", TRUE, NULL);
   g_signal_connect (G_OBJECT (vsink->sink),
                     "handoff",
                     G_CALLBACK (cb_handoff), ev);
   return vsink;
}

int
emotion_pipeline_cdda_track_count_get(void *video)
{
   Emotion_Gstreamer_Video *ev;
   GstBus                  *bus;
   guint                    tracks_count = 0;
   gboolean                 done;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev) return tracks_count;

   done = FALSE;
   bus = gst_element_get_bus (ev->pipeline);
   if (!bus) return tracks_count;

   while (!done) {
      GstMessage *message;

      message = gst_bus_pop (bus);
      if (message == NULL)
        /* All messages read, we're done */
         break;

      switch (GST_MESSAGE_TYPE (message)) {
      case GST_MESSAGE_TAG: {
         GstTagList *tags;

         gst_message_parse_tag (message, &tags);

         gst_tag_list_get_uint (tags, GST_TAG_TRACK_COUNT, &tracks_count);
         if (tracks_count) done = TRUE;
         break;
      }
      case GST_MESSAGE_ERROR:
      default:
         break;
      }
      gst_message_unref (message);
   }

   gst_object_unref (GST_OBJECT (bus));

   return tracks_count;
}

GstElement *
emotion_audio_sink_create (Emotion_Gstreamer_Video *ev, int index)
{
   gchar       buf[128];
   GstElement *bin;
   GstElement *audiobin;
   GstElement *visbin = NULL;
   GstElement *tee;
   GstPad     *teepad;
   GstPad     *binpad;

   /* audio sink */
   bin = gst_bin_new (NULL);
   if (!bin) return NULL;

   g_snprintf (buf, 128, "tee%d", index);
   tee = gst_element_factory_make ("tee", buf);

   /* audio part */
   {
     GstElement *queue;
     GstElement *conv;
     GstElement *resample;
     GstElement *volume;
     GstElement *sink;
     GstPad     *audiopad;
     double      vol;

     audiobin = gst_bin_new (NULL);

     queue = gst_element_factory_make ("queue", NULL);
     conv = gst_element_factory_make ("audioconvert", NULL);
     resample = gst_element_factory_make ("audioresample", NULL);
     volume = gst_element_factory_make ("volume", "volume");
     g_object_get (G_OBJECT (volume), "volume", &vol, NULL);
     ev->volume = vol / 10.0;

     if (index == 1)
       sink = gst_element_factory_make ("autoaudiosink", NULL);
     else
       sink = gst_element_factory_make ("fakesink", NULL);

     gst_bin_add_many (GST_BIN (audiobin),
                       queue, conv, resample, volume, sink, NULL);
     gst_element_link_many (queue, conv, resample, volume, sink, NULL);

     audiopad = gst_element_get_pad (queue, "sink");
     gst_element_add_pad (audiobin, gst_ghost_pad_new ("sink", audiopad));
     gst_object_unref (audiopad);
   }

   /* visualisation part */
   {
     GstElement *vis = NULL;
     char       *vis_name;

     switch (ev->vis) {
     case EMOTION_VIS_GOOM:
       vis_name = "goom";
       break;
     case EMOTION_VIS_LIBVISUAL_BUMPSCOPE:
       vis_name = "libvisual_bumpscope";
       break;
     case EMOTION_VIS_LIBVISUAL_CORONA:
       vis_name = "libvisual_corona";
       break;
     case EMOTION_VIS_LIBVISUAL_DANCING_PARTICLES:
       vis_name = "libvisual_dancingparticles";
       break;
     case EMOTION_VIS_LIBVISUAL_GDKPIXBUF:
       vis_name = "libvisual_gdkpixbuf";
       break;
     case EMOTION_VIS_LIBVISUAL_G_FORCE:
       vis_name = "libvisual_G-Force";
       break;
     case EMOTION_VIS_LIBVISUAL_GOOM:
       vis_name = "libvisual_goom";
       break;
     case EMOTION_VIS_LIBVISUAL_INFINITE:
       vis_name = "libvisual_infinite";
       break;
     case EMOTION_VIS_LIBVISUAL_JAKDAW:
       vis_name = "libvisual_jakdaw";
       break;
     case EMOTION_VIS_LIBVISUAL_JESS:
       vis_name = "libvisual_jess";
       break;
     case EMOTION_VIS_LIBVISUAL_LV_ANALYSER:
       vis_name = "libvisual_lv_analyzer";
       break;
     case EMOTION_VIS_LIBVISUAL_LV_FLOWER:
       vis_name = "libvisual_lv_flower";
       break;
     case EMOTION_VIS_LIBVISUAL_LV_GLTEST:
       vis_name = "libvisual_lv_gltest";
       break;
     case EMOTION_VIS_LIBVISUAL_LV_SCOPE:
       vis_name = "libvisual_lv_scope";
       break;
     case EMOTION_VIS_LIBVISUAL_MADSPIN:
       vis_name = "libvisual_madspin";
       break;
     case EMOTION_VIS_LIBVISUAL_NEBULUS:
       vis_name = "libvisual_nebulus";
       break;
     case EMOTION_VIS_LIBVISUAL_OINKSIE:
       vis_name = "libvisual_oinksie";
       break;
     case EMOTION_VIS_LIBVISUAL_PLASMA:
       vis_name = "libvisual_plazma";
       break;
     default:
       vis_name = "goom";
       break;
     }

     g_snprintf (buf, 128, "vis%d", index);
     if ((vis = gst_element_factory_make (vis_name, buf))) {
       GstElement *queue;
       GstElement *conv;
       GstElement *cspace;
       GstElement *sink;
       GstPad     *vispad;
       GstCaps    *caps;

       g_snprintf (buf, 128, "visbin%d", index);
       visbin = gst_bin_new (buf);

       queue = gst_element_factory_make ("queue", NULL);
       conv = gst_element_factory_make ("audioconvert", NULL);
       cspace = gst_element_factory_make ("ffmpegcolorspace", NULL);
       g_snprintf (buf, 128, "vissink%d", index);
       sink = gst_element_factory_make ("fakesink", buf);

       gst_bin_add_many (GST_BIN (visbin),
                         queue, conv, vis, cspace, sink, NULL);
       gst_element_link_many (queue, conv, vis, cspace, NULL);
       caps = gst_caps_new_simple ("video/x-raw-rgb",
                                   "bpp", G_TYPE_INT, 32,
                                   "width", G_TYPE_INT, 320,
                                   "height", G_TYPE_INT, 200,
                                   NULL);
       gst_element_link_filtered (cspace, sink, caps);

       vispad = gst_element_get_pad (queue, "sink");
       gst_element_add_pad (visbin, gst_ghost_pad_new ("sink", vispad));
       gst_object_unref (vispad);
     }
   }

   gst_bin_add_many (GST_BIN (bin), tee, audiobin, NULL);
   if (visbin)
     gst_bin_add (GST_BIN (bin), visbin);

   binpad = gst_element_get_pad (audiobin, "sink");
   teepad = gst_element_get_request_pad (tee, "src%d");
   gst_pad_link (teepad, binpad);
   gst_object_unref (teepad);
   gst_object_unref (binpad);

   if (visbin) {
      binpad = gst_element_get_pad (visbin, "sink");
      teepad = gst_element_get_request_pad (tee, "src%d");
      gst_pad_link (teepad, binpad);
      gst_object_unref (teepad);
      gst_object_unref (binpad);
   }

   teepad = gst_element_get_pad (tee, "sink");
   gst_element_add_pad (bin, gst_ghost_pad_new ("sink", teepad));
   gst_object_unref (teepad);

   return bin;
}

void
emotion_streams_sinks_get (Emotion_Gstreamer_Video *ev, GstElement *decoder)
{
   GstIterator *it;
   gpointer     data;

   ecore_list_first_goto (ev->video_sinks);
   ecore_list_first_goto (ev->audio_sinks);

   it = gst_element_iterate_src_pads (decoder);
   while (gst_iterator_next (it, &data) == GST_ITERATOR_OK) {
      GstPad  *pad;
      GstCaps *caps;
      gchar   *str;

      pad = GST_PAD (data);

      caps = gst_pad_get_caps (pad);
      str = gst_caps_to_string (caps);
      g_print ("caps !! %s\n", str);

      /* video stream */
      if (g_str_has_prefix (str, "video/")) {
         Emotion_Video_Sink *vsink;

         vsink = (Emotion_Video_Sink *)ecore_list_next (ev->video_sinks);

         emotion_video_sink_fill (vsink, pad, caps);
      }
      /* audio stream */
      else if (g_str_has_prefix (str, "audio/")) {
         Emotion_Audio_Sink *asink;
         gint                index;

         asink = (Emotion_Audio_Sink *)ecore_list_next (ev->audio_sinks);

         emotion_audio_sink_fill (asink, pad, caps);

         index = ecore_list_index (ev->audio_sinks);

         if (ecore_list_count (ev->video_sinks) == 0) {
            if (index == 1) {
               Emotion_Video_Sink *vsink;

               vsink = emotion_visualization_sink_create (ev, asink);
               if (!vsink) goto finalize;
            }
         }
         else {
            gchar       buf[128];
            GstElement *visbin;

            g_snprintf (buf, 128, "visbin%d", index);
            visbin = gst_bin_get_by_name (GST_BIN (ev->pipeline), buf);
            if (visbin) {
               GstPad *srcpad;
               GstPad *sinkpad;

               sinkpad = gst_element_get_pad (visbin, "sink");
               srcpad = gst_pad_get_peer (sinkpad);
               gst_pad_unlink (srcpad, sinkpad);

               gst_object_unref (srcpad);
               gst_object_unref (sinkpad);
            }
         }
      }
   finalize:
      gst_caps_unref (caps);
      g_free (str);
      gst_object_unref (pad);
   }
   gst_iterator_free (it);
}

void
emotion_video_sink_fill (Emotion_Video_Sink *vsink, GstPad *pad, GstCaps *caps)
{
   GstStructure *structure;
   GstQuery     *query;
   const GValue *val;
   gchar        *str;

   structure = gst_caps_get_structure (caps, 0);
   str = gst_caps_to_string (caps);

   gst_structure_get_int (structure, "width", &vsink->width);
   gst_structure_get_int (structure, "height", &vsink->height);

   vsink->fps_num = 1;
   vsink->fps_den = 1;
   val = gst_structure_get_value (structure, "framerate");
   if (val) {
      vsink->fps_num = gst_value_get_fraction_numerator (val);
      vsink->fps_den = gst_value_get_fraction_denominator (val);
   }
   if (g_str_has_prefix(str, "video/x-raw-yuv")) {
      val = gst_structure_get_value (structure, "format");
      vsink->fourcc = gst_value_get_fourcc (val);
   }
   else if (g_str_has_prefix(str, "video/x-raw-rgb"))
     vsink->fourcc = GST_MAKE_FOURCC ('A','R','G','B');
   else
     vsink->fourcc = 0;

   query = gst_query_new_duration (GST_FORMAT_TIME);
   if (gst_pad_query (pad, query)) {
      gint64 time;

      gst_query_parse_duration (query, NULL, &time);
      vsink->length_time = (double)time / (double)GST_SECOND;
   }
   g_free (str);
   gst_query_unref (query);
}

void
emotion_audio_sink_fill (Emotion_Audio_Sink *asink, GstPad *pad, GstCaps *caps)
{
   GstStructure *structure;
   GstQuery     *query;

   structure = gst_caps_get_structure (caps, 0);

   gst_structure_get_int (structure, "channels", &asink->channels);
   gst_structure_get_int (structure, "rate", &asink->samplerate);

   query = gst_query_new_duration (GST_FORMAT_TIME);
   if (gst_pad_query (pad, query)) {
      gint64 time;

      gst_query_parse_duration (query, NULL, &time);
      asink->length_time = (double)time / (double)GST_SECOND;
   }
   gst_query_unref (query);
 }
