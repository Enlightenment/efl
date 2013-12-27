#ifndef __EMOTION_GSTREAMER_H__
#define __EMOTION_GSTREAMER_H__

#include "emotion_modules.h"

#include <unistd.h>
#include <fcntl.h>

#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>

#include <glib.h>
#include <gst/gst.h>
#include <glib-object.h>
#include <gst/video/gstvideosink.h>
#include <gst/video/video.h>
#include <gst/audio/audio.h>
#include <gst/tag/tag.h>

typedef void (*Evas_Video_Convert_Cb)(unsigned char *evas_data,
                                      const unsigned char *gst_data,
                                      unsigned int w,
                                      unsigned int h,
                                      unsigned int output_height);

typedef struct _EvasVideoSinkPrivate EvasVideoSinkPrivate;
typedef struct _EvasVideoSink        EvasVideoSink;
typedef struct _EvasVideoSinkClass   EvasVideoSinkClass;
typedef struct _Emotion_Gstreamer_Video Emotion_Gstreamer_Video;
typedef struct _Emotion_Gstreamer_Metadata Emotion_Gstreamer_Metadata;
typedef struct _Emotion_Gstreamer_Buffer Emotion_Gstreamer_Buffer;
typedef struct _Emotion_Gstreamer_Message Emotion_Gstreamer_Message;

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
   const Emotion_Engine *api;

   /* Gstreamer elements */
   GstElement       *pipeline;
   GstElement       *sink;
   GstElement       *esink;
   GstElement       *convert;

   Eina_List        *threads;

   /* eos */
   GstBus           *eos_bus;

    /* We need to keep a copy of the last inserted buffer as evas doesn't copy YUV data around */
   GstBuffer        *last_buffer;

   /* Evas object */
   Evas_Object      *obj;

   /* Characteristics of stream */
   double            position;
   double            volume;

   volatile int      seek_to;
   volatile int      get_poslen;

   Emotion_Gstreamer_Metadata *metadata;

   const char       *uri;

   Emotion_Gstreamer_Buffer *send;

   EvasVideoSinkPrivate *sink_data;

   Emotion_Vis       vis;

   int               in;
   int               out;

   int frames;
   int flapse;
   double rtime;
   double rlapse;

   struct
   {
      double         width;
      double         height;
   } fill;

   Eina_Bool         play         : 1;
   Eina_Bool         play_started : 1;
   Eina_Bool         video_mute   : 1;
   Eina_Bool         audio_mute   : 1;
   Eina_Bool         pipeline_parsed : 1;
   Eina_Bool         delete_me    : 1;
   Eina_Bool         kill_buffer  : 1;
   Eina_Bool         stream       : 1;
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
   Evas_Object *o;

   Emotion_Gstreamer_Video *ev;

   Evas_Video_Convert_Cb func;

   GstVideoInfo info;
   unsigned int eheight;
   Evas_Colorspace eformat;

   Eina_Lock m;
   Eina_Condition c;

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
   Emotion_Gstreamer_Video *ev;
   EvasVideoSinkPrivate *sink;

   GstBuffer *frame;

   Eina_Bool preroll : 1;
   Eina_Bool force : 1;
};

struct _Emotion_Gstreamer_Message
{
   Emotion_Gstreamer_Video *ev;

   GstMessage *msg;
};

extern int _emotion_gstreamer_log_domain;
extern Eina_Bool debug_fps;

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_emotion_gstreamer_log_domain, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_emotion_gstreamer_log_domain, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_emotion_gstreamer_log_domain, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_emotion_gstreamer_log_domain, __VA_ARGS__)

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_emotion_gstreamer_log_domain, __VA_ARGS__)

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
Eina_Bool _emotion_gstreamer_video_pipeline_parse(Emotion_Gstreamer_Video *ev,
                                                  Eina_Bool force);

typedef struct _ColorSpace_Format_Convertion ColorSpace_Format_Convertion;

struct _ColorSpace_Format_Convertion
{
   const char *name;
   GstVideoFormat format;
   Evas_Colorspace eformat;
   Evas_Video_Convert_Cb func;
   Eina_Bool force_height;
};

extern const ColorSpace_Format_Convertion colorspace_format_convertion[];

/* From gst-plugins-base/gst/playback */
typedef enum {
  GST_PLAY_FLAG_VIDEO         = (1 << 0),
  GST_PLAY_FLAG_AUDIO         = (1 << 1),
  GST_PLAY_FLAG_TEXT          = (1 << 2),
  GST_PLAY_FLAG_VIS           = (1 << 3),
  GST_PLAY_FLAG_SOFT_VOLUME   = (1 << 4),
  GST_PLAY_FLAG_NATIVE_AUDIO  = (1 << 5),
  GST_PLAY_FLAG_NATIVE_VIDEO  = (1 << 6),
  GST_PLAY_FLAG_DOWNLOAD      = (1 << 7),
  GST_PLAY_FLAG_BUFFERING     = (1 << 8),
  GST_PLAY_FLAG_DEINTERLACE   = (1 << 9),
  GST_PLAY_FLAG_SOFT_COLORBALANCE = (1 << 10)
} GstPlayFlags;

#endif /* __EMOTION_GSTREAMER_H__ */
