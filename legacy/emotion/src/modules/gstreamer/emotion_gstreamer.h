#ifndef __EMOTION_GSTREAMER_H__
#define __EMOTION_GSTREAMER_H__


#include <Evas.h>
#include <Ecore.h>

#define HTTP_STREAM 0
#define RTSP_STREAM 1
#include <gst/gst.h>

#include "emotion_private.h"


typedef struct _Emotion_Video_Stream Emotion_Video_Stream;

struct _Emotion_Video_Stream
{
   gdouble     length_time;
   gint        width;
   gint        height;
   gint        fps_num;
   gint        fps_den;
   guint32     fourcc;
};

typedef struct _Emotion_Audio_Stream Emotion_Audio_Stream;

struct _Emotion_Audio_Stream
{
   gdouble     length_time;
   gint        channels;
   gint        samplerate;
};

typedef struct _Emotion_Gstreamer_Metadata Emotion_Gstreamer_Metadata;

struct _Emotion_Gstreamer_Metadata
{
   char *title;
   char *album;
   char *artist;
   char *genre;
   char *comment;
   char *year;
   char *count;
   char *disc_id;
};


typedef struct _Emotion_Gstreamer_Video Emotion_Gstreamer_Video;

struct _Emotion_Gstreamer_Video
{
   /* Gstreamer elements */
   GstElement       *pipeline;

   /* eos */
   GstBus           *eos_bus;
   Ecore_Timer      *eos_timer;

   /* Strams */
   Eina_List        *video_streams;
   Eina_List        *audio_streams;

   int               video_stream_nbr;
   int               audio_stream_nbr;

   /* Evas object */
   Evas_Object      *obj;
   unsigned char    *obj_data;

   /* Characteristics of stream */
   double            position;
   double            ratio;
   double            volume;

   volatile int      seek_to;
   volatile int      get_poslen;

   Ecore_Pipe       *pipe;

   Emotion_Gstreamer_Metadata *metadata;

   Emotion_Vis       vis;

   unsigned char     play         : 1;
   unsigned char     play_started : 1;
   unsigned char     video_mute   : 1;
   unsigned char     audio_mute   : 1;
};

extern int _emotion_gstreamer_log_domain;
#define DBG(...) EINA_LOG_DOM_DBG(_emotion_gstreamer_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_emotion_gstreamer_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_emotion_gstreamer_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_emotion_gstreamer_log_domain, __VA_ARGS__)
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_emotion_gstreamer_log_domain, __VA_ARGS__)

#endif /* __EMOTION_GSTREAMER_H__ */
