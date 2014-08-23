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
#include <gst/video/navigation.h>
#include <gst/audio/audio.h>
#include <gst/tag/tag.h>

typedef void (*Evas_Video_Convert_Cb)(unsigned char *evas_data,
                                      const unsigned char *gst_data,
                                      unsigned int w,
                                      unsigned int h,
                                      unsigned int output_height);

typedef struct _EmotionVideoSinkPrivate EmotionVideoSinkPrivate;
typedef struct _EmotionVideoSink        EmotionVideoSink;
typedef struct _EmotionVideoSinkClass   EmotionVideoSinkClass;
typedef struct _Emotion_Gstreamer Emotion_Gstreamer;
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

struct _Emotion_Gstreamer
{
   const Emotion_Engine *api;

   volatile int     ref_count;

   const char       *subtitle;
   /* Gstreamer elements */
   GstElement       *pipeline;
   GstElement       *vsink;

   Eina_List        *threads;

   /* Evas object */
   Evas_Object      *obj;

   gulong            audio_buffer_probe;
   GstPad           *audio_buffer_probe_pad;
   gint              audio_buffer_probe_pending;

   /* Characteristics of stream */
   double            position;
   double            volume;

   Emotion_Gstreamer_Metadata *metadata;

   Emotion_Vis       vis;

   Eina_Bool         play         : 1;
   Eina_Bool         video_mute   : 1;
   Eina_Bool         audio_mute   : 1;
   Eina_Bool         ready        : 1;
   Eina_Bool         live         : 1;
   Eina_Bool         buffering    : 1;
   Eina_Bool         shutdown     : 1;
};

struct _EmotionVideoSink {
    /*< private >*/
    GstVideoSink parent;
    EmotionVideoSinkPrivate *priv;
};

struct _EmotionVideoSinkClass {
    /*< private >*/
    GstVideoSinkClass parent_class;
};

struct _EmotionVideoSinkPrivate {
   Evas_Object *emotion_object;
   Evas_Object *evas_object;

   GstVideoInfo info;
   unsigned int eheight;
   Evas_Colorspace eformat;
   Evas_Video_Convert_Cb func;

   Eina_Lock m;
   Eina_Condition c;

   Emotion_Gstreamer_Buffer *send;

    /* We need to keep a copy of the last inserted buffer as evas doesn't copy YUV data around */
   GstBuffer        *last_buffer;

   int frames;
   int flapse;
   double rtime;
   double rlapse;

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
   EmotionVideoSink *sink;

   GstBuffer *frame;

   GstVideoInfo info;
   Evas_Colorspace eformat;
   int eheight;
   Evas_Video_Convert_Cb func;
};

struct _Emotion_Gstreamer_Message
{
   Emotion_Gstreamer *ev;

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

#define EMOTION_TYPE_VIDEO_SINK emotion_video_sink_get_type()

#define EMOTION_VIDEO_SINK(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    EMOTION_TYPE_VIDEO_SINK, EmotionVideoSink))

#define EMOTION_VIDEO_SINK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), \
    EMOTION_TYPE_VIDEO_SINK, EmotionVideoSinkClass))

#define EMOTION_IS_VIDEO_SINK(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
    EMOTION_TYPE_VIDEO_SINK))

#define EMOTION_IS_VIDEO_SINK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), \
    EMOTION_TYPE_VIDEO_SINK))

#define EMOTION_VIDEO_SINK_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), \
    EMOTION_TYPE_VIDEO_SINK, EmotionVideoSinkClass))

gboolean    gstreamer_plugin_init(GstPlugin *plugin);

Emotion_Gstreamer_Buffer *emotion_gstreamer_buffer_alloc(EmotionVideoSink *sink,
                                                         GstBuffer *buffer,
                                                         GstVideoInfo *info,
                                                         Evas_Colorspace eformat,
                                                         int eheight,
                                                         Evas_Video_Convert_Cb func);
void emotion_gstreamer_buffer_free(Emotion_Gstreamer_Buffer *send);

Emotion_Gstreamer_Message *emotion_gstreamer_message_alloc(Emotion_Gstreamer *ev,
                                                           GstMessage *msg);
void emotion_gstreamer_message_free(Emotion_Gstreamer_Message *send);

Emotion_Gstreamer * emotion_gstreamer_ref (Emotion_Gstreamer *ev);
void emotion_gstreamer_unref (Emotion_Gstreamer *ev);

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
