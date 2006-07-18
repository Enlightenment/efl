#ifndef __EMOTION_GSTREAMER_PIPELINE_H__
#define __EMOTION_GSTREAMER_PIPELINE_H__


#include <gst/gst.h>


gboolean emotion_pipeline_pause (GstElement *pipeline);

int emotion_pipeline_cdda_build (void *video, const char * device, unsigned int track);
int emotion_pipeline_file_build (void *video, const char *file);
int emotion_pipeline_dvd_build (void *video, const char *device);

int emotion_pipeline_cdda_track_count_get(void *video);


#endif /* __EMOTION_GSTREAMER_PIPELINE_H__ */
