#ifndef __EMOTION_GSTREAMER_H__
#define __EMOTION_GSTREAMER_H__


#include <Ecore_Data.h>

#include <gst/gst.h>


typedef struct _Emotion_Video_Sink Emotion_Video_Sink;

struct _Emotion_Video_Sink
{
  GstElement *sink;
  gdouble     length_time;
  gint        width;
  gint        height;
  gint        fps_num;
  gint        fps_den;
  guint32     fourcc;
};

typedef struct _Emotion_Audio_Sink Emotion_Audio_Sink;

struct _Emotion_Audio_Sink
{
  GstElement *sink;
  gdouble     length_time;
  gint        channels;
  gint        samplerate;
};

typedef struct _Emotion_Gstreamer_Video Emotion_Gstreamer_Video;

struct _Emotion_Gstreamer_Video
{
  /* Gstreamer elements */
  GstElement       *pipeline;
  
  /* Sinks */
  Ecore_List       *video_sinks;
  Ecore_List       *audio_sinks;

  int               video_sink_nbr;
  int               audio_sink_nbr;

  /* Evas object */
  Evas_Object      *obj;
  unsigned char    *obj_data;

  /* Characteristics of stream */
  int               position;
  double            ratio;

  volatile int      seek_to;
  volatile int      get_poslen;
  volatile double   seek_to_pos;

  int               fd_ev_read;
  int               fd_ev_write;
  Ecore_Fd_Handler *fd_ev_handler;


  unsigned char     play : 1;
  unsigned char     video_mute : 1;
  unsigned char     audio_mute : 1;
};

unsigned char module_open  (Evas_Object           *obj,
                            Emotion_Video_Module **module,
                            void                 **video);

void          module_close (Emotion_Video_Module  *module,
                            void                  *video);


#endif /* __EMOTION_GSTREAMER_H__ */
