/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef __EMOTION_GSTREAMER_PIPELINE_H__
#define __EMOTION_GSTREAMER_PIPELINE_H__


#include <gst/gst.h>


gboolean emotion_pipeline_pause           (GstElement *pipeline);

int emotion_pipeline_cdda_build           (void *video, const char * device, unsigned int track);
int emotion_pipeline_file_build           (void *video, const char *file);
int emotion_pipeline_uri_build            (void *video, const char *uri);
int emotion_pipeline_dvd_build            (void *video, const char *device);
int emotion_pipeline_v4l_build            (void *video, const char *device);
int emotion_pipeline_cdda_track_count_get (void *video);

GstElement         *emotion_audio_sink_create         (Emotion_Gstreamer_Video *ev, int index);
Emotion_Video_Sink *emotion_video_sink_new            (Emotion_Gstreamer_Video *ev);
void                emotion_video_sink_free           (Emotion_Gstreamer_Video *ev, Emotion_Video_Sink *vsink);
Emotion_Video_Sink *emotion_visualization_sink_create (Emotion_Gstreamer_Video *ev, Emotion_Audio_Sink *asink);

void                emotion_streams_sinks_get         (Emotion_Gstreamer_Video *ev, GstElement *decoder);

void                emotion_video_sink_fill           (Emotion_Video_Sink *vsink, GstPad *pad, GstCaps *caps);

void                emotion_audio_sink_fill           (Emotion_Audio_Sink *asink, GstPad *pad, GstCaps *caps);

void cb_handoff              (GstElement *fakesrc,
                              GstBuffer  *buffer,
                              GstPad     *pad,
                              gpointer    user_data);

void file_new_decoded_pad_cb (GstElement *decodebin,
                              GstPad     *new_pad,
                              gboolean    last,
                              gpointer    user_data);

const char *emotion_visualization_element_name_get(Emotion_Vis visualisation);


#endif /* __EMOTION_GSTREAMER_PIPELINE_H__ */
