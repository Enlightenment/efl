/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "emotion_gstreamer.h"
#include "emotion_gstreamer_pipeline.h"

int
emotion_pipeline_uri_build(void *video, const char *uri)
{
   GstElement              *src;
   GstElement              *decodebin;
   Emotion_Gstreamer_Video *ev;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev) return 0;

   if (gst_uri_protocol_is_supported(GST_URI_SRC, uri))
     goto failure_src;
   src = gst_element_make_from_uri(GST_URI_SRC, uri, "src");
   if (!src)
     goto failure_src;
   g_object_set(G_OBJECT(src), "location", uri, NULL);

   decodebin = gst_element_factory_make("decodebin", "decodebin");
   if (!decodebin)
     goto failure_decodebin;
   g_signal_connect(decodebin, "new-decoded-pad",
		    G_CALLBACK(file_new_decoded_pad_cb), ev);

   gst_bin_add_many(GST_BIN(ev->pipeline), src, decodebin, NULL);
   if (!gst_element_link(src, decodebin))
     goto failure_link;

   if (!emotion_pipeline_pause(ev->pipeline))
     goto failure_gstreamer_pause;

   emotion_streams_sinks_get(ev, decodebin);

   /* The first vsink is a valid Emotion_Video_Sink * */
   /* If no video stream is found, it's a visualisation sink */
     {
	Emotion_Video_Sink *vsink;

	vsink = (Emotion_Video_Sink *)eina_list_data_get(ev->video_sinks);
	if (vsink && vsink->sink)
	  {
	     g_object_set(G_OBJECT(vsink->sink), "sync", TRUE, NULL);
	     g_object_set(G_OBJECT(vsink->sink), "signal-handoffs", TRUE, NULL);
	     g_signal_connect(G_OBJECT(vsink->sink),
			      "handoff",
			      G_CALLBACK(cb_handoff), ev);
	  }
     }

   return 1;

failure_gstreamer_pause:
failure_link:
   gst_element_set_state(ev->pipeline, GST_STATE_NULL);
   gst_bin_remove(GST_BIN(ev->pipeline), decodebin);
failure_decodebin:
   gst_bin_remove(GST_BIN(ev->pipeline), src);
failure_src:

   return 0;
}
