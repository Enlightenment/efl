/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "emotion_gstreamer.h"
#include "emotion_gstreamer_pipeline.h"

int
emotion_pipeline_v4l_build(void *video, const char *device)
{
   GstElement *v4l2src, *cspace, *queue, *sink;
   Emotion_Video_Sink *vsink;
   GstCaps *caps;
   Emotion_Gstreamer_Video *ev;
   char dev[128];
   int devno;

   ev = (Emotion_Gstreamer_Video *)video;
   if (!ev) return 0;

   v4l2src = gst_element_factory_make("v4l2src", "v4l2src");
   cspace = gst_element_factory_make("ffmpegcolorspace", "cspace");
   queue = gst_element_factory_make("queue", "queue");
   sink = gst_element_factory_make("fakesink", "sink");

   if ((!v4l2src) || (!cspace) || (!queue) || (!sink))
     goto failure;

   if (sscanf(device, "v4l://%d", &devno) != 1)
     devno = 0;

   snprintf(dev, sizeof(dev), "/dev/video%d", devno);
   g_object_set (v4l2src, "device", dev, NULL);

   gst_bin_add_many(GST_BIN(ev->pipeline), v4l2src, cspace, queue, sink, NULL);

   caps = gst_caps_new_simple("video/x-raw-yuv",
                              "width", G_TYPE_INT, 320,
                              "height", G_TYPE_INT, 240,
                              NULL);
   if (!gst_element_link_filtered(v4l2src, cspace, caps))
     {
        gst_caps_unref(caps);
        goto failure;
     }
   gst_caps_unref(caps);

   caps = gst_caps_new_simple("video/x-raw-rgb",
                              "bpp", G_TYPE_INT, 32,
                              "width", G_TYPE_INT, 320,
                              "height", G_TYPE_INT, 240,
                              NULL);
   if (!gst_element_link_filtered(cspace, queue, caps))
     {
       gst_caps_unref(caps);
       goto failure;
     }
   gst_caps_unref(caps);

   gst_element_link(queue, sink);

   vsink = emotion_video_sink_new(ev);
   if(!vsink) goto failure;
   vsink->sink = sink;
   vsink->width=320;
   vsink->height=240;
   vsink->fourcc = GST_MAKE_FOURCC ('A', 'R', 'G', 'B');

   g_object_set(G_OBJECT(vsink->sink), "sync", FALSE, NULL);
   g_object_set(G_OBJECT(vsink->sink), "signal-handoffs", TRUE, NULL);
   g_signal_connect(G_OBJECT(vsink->sink),
		    "handoff",
		    G_CALLBACK(cb_handoff), ev);

   return 1;

failure:
   if(v4l2src)
     gst_object_unref(v4l2src);
   if(cspace)
     gst_object_unref(cspace);
   if(queue)
     gst_object_unref(queue);
   if(sink)
     gst_object_unref(sink);

   return 0;
}
