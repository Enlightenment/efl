/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "emotion_gstreamer.h"
#include "emotion_gstreamer_pipeline.h"


static Emotion_Audio_Sink *_emotion_audio_sink_new  (Emotion_Gstreamer_Video *ev);
static void                _emotion_audio_sink_free (Emotion_Gstreamer_Video *ev, Emotion_Audio_Sink *asink);

int
emotion_pipeline_cdda_build(void *video, const char * device, unsigned int track)
{
   GstElement              *cdiocddasrc;
   Emotion_Video_Sink      *vsink;
   Emotion_Audio_Sink      *asink;
   Emotion_Gstreamer_Video *ev;
   /*    GstFormat                format; */
   /*    gint64                  tracks_count; */

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev) return 0;

   cdiocddasrc = gst_element_factory_make("cdiocddasrc", "src");
   if (!cdiocddasrc)
     {
	g_print("cdiocddasrc element missing. Install it.\n");
	goto failure_cdiocddasrc;
     }

   if (device)
     g_object_set(G_OBJECT(cdiocddasrc), "device", device, NULL);

   g_object_set(G_OBJECT(cdiocddasrc), "track", track, NULL);

   asink = _emotion_audio_sink_new(ev);
   if (!asink)
     goto failure_emotion_sink;

   asink->sink = emotion_audio_sink_create(ev,  1);
   if (!asink->sink)
     goto failure_gstreamer_sink;

   gst_bin_add_many((GST_BIN(ev->pipeline)), cdiocddasrc, asink->sink, NULL);

   if (!gst_element_link(cdiocddasrc, asink->sink))
     goto failure_link;

   vsink = emotion_visualization_sink_create(ev, asink);
   if (!vsink) goto failure_link;

   if (!emotion_pipeline_pause(ev->pipeline))
     goto failure_gstreamer_pause;

     {
	GstQuery *query;
	GstPad   *pad;
	GstCaps  *caps;
	GstStructure *structure;

	/* should always be found */
	pad = gst_element_get_pad(cdiocddasrc, "src");

	caps = gst_pad_get_caps(pad);
	structure = gst_caps_get_structure(GST_CAPS(caps), 0);

	gst_structure_get_int(structure, "channels", &asink->channels);
	gst_structure_get_int(structure, "rate", &asink->samplerate);

	gst_caps_unref(caps);

	query = gst_query_new_duration(GST_FORMAT_TIME);
	if (gst_pad_query(pad, query))
	  {
	     gint64 time;

	     gst_query_parse_duration(query, NULL, &time);
	     asink->length_time = (double)time / (double)GST_SECOND;
	     vsink->length_time = asink->length_time;
	  }
	gst_query_unref(query);
	gst_object_unref(GST_OBJECT(pad));
     }

   return 1;

failure_gstreamer_pause:
   emotion_video_sink_free(ev, vsink);
failure_link:
   gst_bin_remove(GST_BIN(ev->pipeline), asink->sink);
failure_gstreamer_sink:
   _emotion_audio_sink_free(ev, asink);
failure_emotion_sink:
   gst_bin_remove(GST_BIN(ev->pipeline), cdiocddasrc);
failure_cdiocddasrc:

   return 0;
}

static Emotion_Audio_Sink *
_emotion_audio_sink_new(Emotion_Gstreamer_Video *ev)
{
   Emotion_Audio_Sink *asink;

   if (!ev) return NULL;

   asink = (Emotion_Audio_Sink *)malloc(sizeof(Emotion_Audio_Sink));
   if (!asink) return NULL;

   ev->audio_sinks = eina_list_append(ev->audio_sinks, asink);
   if (!eina_list_data_find(ev->audio_sinks, asink))
     {
	free(asink);
	return NULL;
     }
   return asink;
}

static void
_emotion_audio_sink_free(Emotion_Gstreamer_Video *ev, Emotion_Audio_Sink *asink)
{
   if (!ev || !asink) return;

   ev->audio_sinks = eina_list_remove(ev->audio_sinks, asink);
	free(asink);
}
