#ifndef __EMOTION_GSTREAMER_H__
#define __EMOTION_GSTREAMER_H__

#include <Ecore_Data.h>

#include <gst/gst.h>

typedef struct _Emotion_Video_Sink Emotion_Video_Sink;

struct _Emotion_Video_Sink
{
  GstElement        *sink;
  unsigned long long length_time;
  unsigned long long length_frames;
  unsigned int       width;
  unsigned int       height;
  unsigned int       par_num;
  unsigned int       par_den;
  double             framerate;
};

typedef struct _Emotion_Audio_Sink Emotion_Audio_Sink;

struct _Emotion_Audio_Sink
{
  GstElement        *sink;
  unsigned long long length_time;
  unsigned long long length_samples;
  unsigned int       channels;
  unsigned int       samplerate;
};

typedef struct _Emotion_Gstreamer_Video Emotion_Gstreamer_Video;

struct _Emotion_Gstreamer_Video
{
  /* Gstreamer elements */
  GstElement *pipeline;
  
  /* Sinks */
  Ecore_List *video_sinks;
  Ecore_List *audio_sinks;

  /* Evas object */
  Evas_Object   *obj;
  unsigned char *obj_data;

  /* Characteristics */
  int          position;
  int          width;
  int          height;
  double       ratio;

  volatile int    seek_to;
  volatile int    get_poslen;
  volatile double seek_to_pos;

  int                       fd_ev_read;
  int                       fd_ev_write;
  Ecore_Fd_Handler         *fd_ev_handler;


  unsigned char play : 1;
  unsigned char video_mute : 1;
  unsigned char audio_mute : 1;
};

unsigned char         module_open  (Evas_Object           *obj,
				    Emotion_Video_Module **module,
				    void                 **video);

void                  module_close (Emotion_Video_Module *module,
				    void                 *video);


#endif /* __EMOTION_GSTREAMER_H__ */
