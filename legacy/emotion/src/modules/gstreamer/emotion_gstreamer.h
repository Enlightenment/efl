#ifndef __EMOTION_GSTREAMER_H__
#define __EMOTION_GSTREAMER_H__


#include <Evas.h>
#include <Ecore.h>

#define HTTP_STREAM 0
#define RTSP_STREAM 1
#include <glib.h>
#include <gst/gst.h>
#include <glib-object.h>
#include <gst/video/gstvideosink.h>
#include <gst/video/video.h>

#include "emotion_private.h"

typedef struct _EvasVideoSinkPrivate EvasVideoSinkPrivate;
typedef struct _EvasVideoSink        EvasVideoSink;
typedef struct _EvasVideoSinkClass   EvasVideoSinkClass;
typedef struct _Emotion_Gstreamer_Video Emotion_Gstreamer_Video;
typedef struct _Emotion_Audio_Stream Emotion_Audio_Stream;
typedef struct _Emotion_Gstreamer_Metadata Emotion_Gstreamer_Metadata;
typedef struct _Emotion_Gstreamer_Buffer Emotion_Gstreamer_Buffer;
typedef struct _Emotion_Gstreamer_Message Emotion_Gstreamer_Message;
typedef struct _Emotion_Video_Stream Emotion_Video_Stream;

struct _Emotion_Video_Stream
{
   gdouble     length_time;
   gint        width;
   gint        height;
   gint        fps_num;
   gint        fps_den;
   guint32     fourcc;
   int         index;
};

struct _Emotion_Audio_Stream
{
   gdouble     length_time;
   gint        channels;
   gint        samplerate;
};

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

struct _Emotion_Gstreamer_Video
{
   /* Gstreamer elements */
   GstElement       *pipeline;

   /* eos */
   GstBus           *eos_bus;

   /* Strams */
   Eina_List        *video_streams;
   Eina_List        *audio_streams;

   int               video_stream_nbr;
   int               audio_stream_nbr;

   /* Evas object */
   Evas_Object      *obj;

   /* Characteristics of stream */
   double            position;
   double            ratio;
   double            volume;

   volatile int      seek_to;
   volatile int      get_poslen;

   Emotion_Gstreamer_Metadata *metadata;

   Emotion_Vis       vis;

   unsigned char     play         : 1;
   unsigned char     play_started : 1;
   unsigned char     video_mute   : 1;
   unsigned char     audio_mute   : 1;
};

struct _EvasVideoSink {
    /*< private >*/
    GstVideoSink parent;
    EvasVideoSinkPrivate *priv;
};

struct _EvasVideoSinkClass {
    /*< private >*/
    GstVideoSinkClass parent_class;
};

struct _EvasVideoSinkPrivate {
   EINA_REFCOUNT;

   Evas_Object *o;

   int width;
   int height;
   Evas_Colorspace eformat;
   GstVideoFormat gformat;

   GMutex* buffer_mutex;
   GCond* data_cond;

    /* We need to keep a copy of the last inserted buffer as evas doesn't copy YUV data around */
   GstBuffer *last_buffer;

   // If this is TRUE all processing should finish ASAP
   // This is necessary because there could be a race between
   // unlock() and render(), where unlock() wins, signals the
   // GCond, then render() tries to render a frame although
   // everything else isn't running anymore. This will lead
   // to deadlocks because render() holds the stream lock.
   //
   // Protected by the buffer mutex
   Eina_Bool unlocked : 1;
};

struct _Emotion_Gstreamer_Buffer
{
   EvasVideoSinkPrivate *sink;

   GstBuffer *frame;

   Eina_Bool preroll : 1;
};

struct _Emotion_Gstreamer_Message
{
   Emotion_Gstreamer_Video *ev;

   GstMessage *msg;
};

extern int _emotion_gstreamer_log_domain;
#define DBG(...) EINA_LOG_DOM_DBG(_emotion_gstreamer_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_emotion_gstreamer_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_emotion_gstreamer_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_emotion_gstreamer_log_domain, __VA_ARGS__)
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_emotion_gstreamer_log_domain, __VA_ARGS__)

#define EVAS_TYPE_VIDEO_SINK evas_video_sink_get_type()

#define EVAS_VIDEO_SINK(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    EVAS_TYPE_VIDEO_SINK, EvasVideoSink))

#define EVAS_VIDEO_SINK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), \
    EVAS_TYPE_VIDEO_SINK, EvasVideoSinkClass))

#define EVAS_IS_VIDEO_SINK(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
    EVAS_TYPE_VIDEO_SINK))

#define EVAS_IS_VIDEO_SINK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), \
    EVAS_TYPE_VIDEO_SINK))

#define EVAS_VIDEO_SINK_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), \
    EVAS_TYPE_VIDEO_SINK, EvasVideoSinkClass))

GstElement *gstreamer_video_sink_new(Emotion_Gstreamer_Video *ev,
                                     Evas_Object *obj,
                                     const char *uri);

gboolean    gstreamer_plugin_init(GstPlugin *plugin);

Emotion_Gstreamer_Buffer *emotion_gstreamer_buffer_alloc(EvasVideoSinkPrivate *sink,
                                                         GstBuffer *buffer,
                                                         Eina_Bool preroll);
void emotion_gstreamer_buffer_free(Emotion_Gstreamer_Buffer *send);

Emotion_Gstreamer_Message *emotion_gstreamer_message_alloc(Emotion_Gstreamer_Video *ev,
                                                           GstMessage *msg);
void emotion_gstreamer_message_free(Emotion_Gstreamer_Message *send);

#endif /* __EMOTION_GSTREAMER_H__ */
