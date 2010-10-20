#ifndef __EMOTION_GSTREAMER_PIPELINE_H__
#define __EMOTION_GSTREAMER_PIPELINE_H__


#include <gst/gst.h>


gboolean emotion_pipeline_pause           (GstElement *pipeline);

Eina_Bool _emotion_pipeline_build(Emotion_Gstreamer_Video *ev, const char *file);
int emotion_pipeline_cdda_track_count_get (void *video);

GstElement         *emotion_audio_stream_create         (Emotion_Gstreamer_Video *ev, int index);
Emotion_Video_Stream *emotion_video_stream_new            (Emotion_Gstreamer_Video *ev);
void                emotion_video_stream_free           (Emotion_Gstreamer_Video *ev, Emotion_Video_Stream *vstream);

void cb_handoff              (GstElement *fakesrc,
                              GstBuffer  *buffer,
                              GstPad     *pad,
                              gpointer    user_data);

const char *emotion_visualization_element_name_get(Emotion_Vis visualisation);


#endif /* __EMOTION_GSTREAMER_PIPELINE_H__ */
