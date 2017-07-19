#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "emotion_gstreamer.h"

Emotion_Gstreamer_Buffer *
emotion_gstreamer_buffer_alloc(EmotionVideoSink *sink,
                               GstBuffer *buffer,
                               GstVideoInfo *info,
                               Evas_Colorspace eformat,
                               int eheight,
                               Evas_Video_Convert_Cb func)
{
   Emotion_Gstreamer_Buffer *send;

   if (!sink->priv->emotion_object) return NULL;

   send = calloc(1, sizeof(Emotion_Gstreamer_Buffer));
   if (!send) return NULL;

   send->sink = gst_object_ref(sink);
   send->frame = gst_buffer_ref(buffer);
   send->info = *info;
   if (gst_video_frame_map(&(send->vframe), info, buffer, GST_MAP_READ))
     send->vfmapped = EINA_TRUE;
   else
     send->vfmapped = EINA_FALSE;
   send->eformat = eformat;
   send->eheight = eheight;
   send->func = func;
   return send;
}

void
emotion_gstreamer_buffer_free(Emotion_Gstreamer_Buffer *send)
{
   gst_object_unref(send->sink);
   gst_buffer_replace(&send->frame, NULL);
   free(send);
}

Emotion_Gstreamer_Message *
emotion_gstreamer_message_alloc(Emotion_Gstreamer *ev,
                                GstMessage *msg)
{
   Emotion_Gstreamer_Message *send;

   if (!ev) return NULL;

   send = malloc(sizeof (Emotion_Gstreamer_Message));
   if (!send) return NULL;

   send->ev = emotion_gstreamer_ref(ev);
   send->msg = gst_message_ref(msg);

   return send;
}

void
emotion_gstreamer_message_free(Emotion_Gstreamer_Message *send)
{
   emotion_gstreamer_unref(send->ev);
   gst_message_unref(send->msg);
   free(send);
}
