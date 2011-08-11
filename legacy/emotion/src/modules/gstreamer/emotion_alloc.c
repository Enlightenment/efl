#include <glib.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideosink.h>

#include <Ecore.h>

#include "emotion_gstreamer.h"

Emotion_Gstreamer_Buffer *
emotion_gstreamer_buffer_alloc(EvasVideoSinkPrivate *sink,
			       GstBuffer *buffer,
			       Eina_Bool preroll)
{
   Emotion_Gstreamer_Buffer *send;

   send = malloc(sizeof (Emotion_Gstreamer_Buffer));
   if (!send) return NULL;

   send->sink = sink;
   send->frame = gst_buffer_ref(buffer);
   send->preroll = preroll;

   return send;
}

void
emotion_gstreamer_buffer_free(Emotion_Gstreamer_Buffer *send)
{
   gst_buffer_unref(send->frame);
   free(send);
}

Emotion_Gstreamer_Message *
emotion_gstreamer_message_alloc(Emotion_Gstreamer_Video *ev,
				GstMessage *msg)
{
   Emotion_Gstreamer_Message *send;

   send = malloc(sizeof (Emotion_Gstreamer_Message));
   if (!send) return NULL;

   ev->out++;
   send->ev = ev;
   send->msg = gst_message_ref(msg);

   return send;
}

void
emotion_gstreamer_message_free(Emotion_Gstreamer_Message *send)
{
   send->ev->in++;

   if (send->ev->in == send->ev->out && send->ev->delete_me)
     em_shutdown(send->ev);

   gst_message_unref(send->msg);
   free(send);
}
