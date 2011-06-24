#include <glib.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideosink.h>

#include <Ecore.h>

#include "emotion_gstreamer.h"

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK, GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS(GST_VIDEO_CAPS_BGRx ";" GST_VIDEO_CAPS_BGR ";" GST_VIDEO_CAPS_BGRA ";" GST_VIDEO_CAPS_YUV("{I420,YV12}")));


GST_DEBUG_CATEGORY_STATIC(evas_video_sink_debug);
#define GST_CAT_DEFAULT evas_video_sink_debug

enum {
  REPAINT_REQUESTED,
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_EVAS_OBJECT,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_LAST,
};

static guint evas_video_sink_signals[LAST_SIGNAL] = { 0, };

struct _EvasVideoSinkPrivate {
   Evas_Object *o;
   Ecore_Pipe *p;

   int width;
   int height;
   gboolean update_size;
   GstVideoFormat format;

   GMutex* buffer_mutex;
   GCond* data_cond;

   // If this is TRUE all processing should finish ASAP
   // This is necessary because there could be a race between
   // unlock() and render(), where unlock() wins, signals the
   // GCond, then render() tries to render a frame although
   // everything else isn't running anymore. This will lead
   // to deadlocks because render() holds the stream lock.
   //
   // Protected by the buffer mutex
   Eina_Bool unlocked : 1;
   Eina_Bool preroll : 1;
};

#define _do_init(bla)                                   \
  GST_DEBUG_CATEGORY_INIT(evas_video_sink_debug,        \
                          "emotion-sink",		\
                          0,                            \
                          "emotion video sink")

GST_BOILERPLATE_FULL(EvasVideoSink,
                     evas_video_sink,
                     GstVideoSink,
                     GST_TYPE_VIDEO_SINK,
                     _do_init);


static void unlock_buffer_mutex(EvasVideoSinkPrivate* priv);

static void evas_video_sink_render_handler(void *data, void *buf, unsigned int len);

static void
evas_video_sink_base_init(gpointer g_class)
{
   GstElementClass* element_class;

   element_class = GST_ELEMENT_CLASS(g_class);
   gst_element_class_add_pad_template(element_class, gst_static_pad_template_get(&sinktemplate));
   gst_element_class_set_details_simple(element_class, "Evas video sink",
                                        "Sink/Video", "Sends video data from a GStreamer pipeline to an Evas object",
                                        "Vincent Torri <vtorri@univ-evry.fr>");
}

static void
evas_video_sink_init(EvasVideoSink* sink, EvasVideoSinkClass* klass __UNUSED__)
{
   EvasVideoSinkPrivate* priv;

   sink->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE(sink, EVAS_TYPE_VIDEO_SINK, EvasVideoSinkPrivate);
   priv->o = NULL;
   priv->p = ecore_pipe_add(evas_video_sink_render_handler, sink);
   priv->width = 0;
   priv->height = 0;
   priv->update_size = TRUE;
   priv->format = GST_VIDEO_FORMAT_UNKNOWN;
   priv->data_cond = g_cond_new();
   priv->buffer_mutex = g_mutex_new();
   priv->preroll = EINA_FALSE;
   priv->unlocked = EINA_FALSE;
}


/**** Object methods ****/

static void
evas_video_sink_set_property(GObject * object, guint prop_id,
                             const GValue * value, GParamSpec * pspec)
{
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;

   sink = EVAS_VIDEO_SINK (object);
   priv = sink->priv;

   switch (prop_id) {
    case PROP_EVAS_OBJECT:
       g_mutex_lock(priv->buffer_mutex);
       priv->o = g_value_get_pointer (value);
       g_mutex_unlock(priv->buffer_mutex);
       break;
    default:
       G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
       break;
   }
}

static void
evas_video_sink_get_property(GObject * object, guint prop_id,
                             GValue * value, GParamSpec * pspec)
{
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;

   sink = EVAS_VIDEO_SINK (object);
   priv = sink->priv;

   switch (prop_id) {
    case PROP_EVAS_OBJECT:
       g_mutex_lock(priv->buffer_mutex);
       g_value_set_pointer (value, priv->o);
       g_mutex_unlock(priv->buffer_mutex);
       break;
    case PROP_WIDTH:
       g_mutex_lock(priv->buffer_mutex);
       g_value_set_int(value, priv->width);
       g_mutex_unlock(priv->buffer_mutex);
       break;
    case PROP_HEIGHT:
       g_mutex_lock(priv->buffer_mutex);
       g_value_set_int (value, priv->height);
       g_mutex_unlock(priv->buffer_mutex);
       break;
    default:
       G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
       break;
   }
}

static void
evas_video_sink_dispose(GObject* object)
{
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;

   sink = EVAS_VIDEO_SINK(object);
   priv = sink->priv;

   if (priv->buffer_mutex) {
      g_mutex_free(priv->buffer_mutex);
      priv->buffer_mutex = 0;
   }

   if (priv->data_cond) {
      g_cond_free(priv->data_cond);
      priv->data_cond = 0;
   }

   if (priv->p) {
      ecore_pipe_del(priv->p);
      priv->p = NULL;
   }

   G_OBJECT_CLASS(parent_class)->dispose(object);
}


/**** BaseSink methods ****/

gboolean evas_video_sink_set_caps(GstBaseSink *bsink, GstCaps *caps)
{
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;
   int width;
   int height;

   sink = EVAS_VIDEO_SINK(bsink);
   priv = sink->priv;

   if (G_UNLIKELY(!gst_video_format_parse_caps(caps, &priv->format, &width, &height))) {
      return FALSE;
   }

   if ((width != priv->width) || (height != priv->height))
     {
        priv->width = width;
        priv->height = height;
        priv->update_size = TRUE;
     }

   printf("format :");
   switch (priv->format)
     {
      case GST_VIDEO_FORMAT_I420:
         evas_object_image_size_set(priv->o, priv->width, priv->height);
         evas_object_image_fill_set(priv->o, 0, 0, priv->width, priv->height);
         evas_object_image_colorspace_set(priv->o, EVAS_COLORSPACE_YCBCR422P601_PL);
         evas_object_image_alpha_set(priv->o, 0);
         printf ("I420\n");
         break;
      case GST_VIDEO_FORMAT_YV12:
         evas_object_image_size_set(priv->o, priv->width, priv->height);
         evas_object_image_fill_set(priv->o, 0, 0, priv->width, priv->height);
         evas_object_image_colorspace_set(priv->o, EVAS_COLORSPACE_YCBCR422P601_PL);
         evas_object_image_alpha_set(priv->o, 0);
         printf ("YV12\n");
         break;
      case GST_VIDEO_FORMAT_BGR:
         printf ("BGR\n");
         break;
      case GST_VIDEO_FORMAT_BGRx:
         printf ("BGRx\n");
         break;
      case GST_VIDEO_FORMAT_BGRA:
         printf ("BGRA\n");
         break;
      default:
         printf ("unsupported : %d\n", priv->format);
         return FALSE;
     }

   return TRUE;
}

static gboolean
evas_video_sink_start(GstBaseSink* base_sink)
{
   EvasVideoSinkPrivate* priv;
   gboolean res = TRUE;

   priv = EVAS_VIDEO_SINK(base_sink)->priv;
   g_mutex_lock(priv->buffer_mutex);
   if (!priv->o)
     res = FALSE;
   else
     {
        if (!priv->p)
          res = FALSE;
        else
          {
             priv->unlocked = EINA_FALSE;
          }
     }
   g_mutex_unlock(priv->buffer_mutex);
   return res;
}

static gboolean
evas_video_sink_stop(GstBaseSink* base_sink)
{
   EvasVideoSinkPrivate* priv = EVAS_VIDEO_SINK(base_sink)->priv;

   unlock_buffer_mutex(priv);
   return TRUE;
}

static gboolean
evas_video_sink_unlock(GstBaseSink* object)
{
   EvasVideoSink* sink;

   sink = EVAS_VIDEO_SINK(object);

   unlock_buffer_mutex(sink->priv);

   return GST_CALL_PARENT_WITH_DEFAULT(GST_BASE_SINK_CLASS, unlock,
                                       (object), TRUE);
}

static gboolean
evas_video_sink_unlock_stop(GstBaseSink* object)
{
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;

   sink = EVAS_VIDEO_SINK(object);
   priv = sink->priv;

   g_mutex_lock(priv->buffer_mutex);
   priv->unlocked = FALSE;
   g_mutex_unlock(priv->buffer_mutex);

   return GST_CALL_PARENT_WITH_DEFAULT(GST_BASE_SINK_CLASS, unlock_stop,
                                       (object), TRUE);
}

static GstFlowReturn
evas_video_sink_preroll(GstBaseSink* bsink, GstBuffer* buffer)
{
   GstBuffer *send;
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;

   sink = EVAS_VIDEO_SINK(bsink);
   priv = sink->priv;

   send = gst_buffer_ref(buffer);

   priv->preroll = EINA_TRUE;

   ecore_pipe_write(priv->p, &send, sizeof(buffer));
   return GST_FLOW_OK;
}

static GstFlowReturn
evas_video_sink_render(GstBaseSink* bsink, GstBuffer* buffer)
{
   GstBuffer *send;
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;
   Eina_Bool ret;

   sink = EVAS_VIDEO_SINK(bsink);
   priv = sink->priv;

   g_mutex_lock(priv->buffer_mutex);

   if (priv->unlocked) {
      g_mutex_unlock(priv->buffer_mutex);
      return GST_FLOW_OK;
   }

   priv->preroll = EINA_FALSE;

   send = gst_buffer_ref(buffer);
   ret = ecore_pipe_write(priv->p, &send, sizeof(buffer));
   if (!ret)
     return GST_FLOW_ERROR;

   g_cond_wait(priv->data_cond, priv->buffer_mutex);
   g_mutex_unlock(priv->buffer_mutex);

   return GST_FLOW_OK;
}

static void evas_video_sink_render_handler(void *data,
                                           void *buf,
                                           unsigned int len)
{
   Emotion_Gstreamer_Video *ev;
   Emotion_Video_Stream *vstream;
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;
   GstBuffer* buffer;
   unsigned char *evas_data;
   const guint8 *gst_data;
   GstQuery *query;
   GstFormat fmt = GST_FORMAT_TIME;
   gint64 pos;

   sink = (EvasVideoSink *)data;
   priv = sink->priv;

   buffer = *((GstBuffer **)buf);

   if (priv->unlocked)
     goto exit_point;

   gst_data = GST_BUFFER_DATA(buffer);
   if (!gst_data) goto exit_point;
   if (priv->update_size)
     {
        evas_object_image_size_set(priv->o, priv->width, priv->height);
        evas_object_image_fill_set(priv->o, 0, 0, priv->width, priv->height);
        priv->update_size = FALSE;
     }

   evas_data = (unsigned char *)evas_object_image_data_get(priv->o, 1);

   // Evas's BGRA has pre-multiplied alpha while GStreamer's doesn't.
   // Here we convert to Evas's BGRA.
   if (priv->format == GST_VIDEO_FORMAT_BGR) {
      unsigned char *evas_tmp;
      int x;
      int y;

      evas_tmp = evas_data;
      /* FIXME: could this be optimized ? */
      for (x = 0; x < priv->height; x++) {
         for (y = 0; y < priv->width; y++) {
            evas_tmp[0] = gst_data[0];
            evas_tmp[1] = gst_data[1];
            evas_tmp[2] = gst_data[2];
            evas_tmp[3] = 255;
            gst_data += 3;
            evas_tmp += 4;
         }
      }
   }

   // Evas's BGRA has pre-multiplied alpha while GStreamer's doesn't.
   // Here we convert to Evas's BGRA.
   if (priv->format == GST_VIDEO_FORMAT_BGRx) {
      unsigned char *evas_tmp;
      int x;
      int y;

      evas_tmp = evas_data;
      /* FIXME: could this be optimized ? */
      for (x = 0; x < priv->height; x++) {
         for (y = 0; y < priv->width; y++) {
            evas_tmp[0] = gst_data[0];
            evas_tmp[1] = gst_data[1];
            evas_tmp[2] = gst_data[2];
            evas_tmp[3] = 255;
            gst_data += 4;
            evas_tmp += 4;
         }
      }
   }

   // Evas's BGRA has pre-multiplied alpha while GStreamer's doesn't.
   // Here we convert to Evas's BGRA.
   if (priv->format == GST_VIDEO_FORMAT_BGRA) {
      unsigned char *evas_tmp;
      int x;
      int y;
      unsigned char alpha;

      evas_tmp = evas_data;
      /* FIXME: could this be optimized ? */
      for (x = 0; x < priv->height; x++) {
         for (y = 0; y < priv->width; y++) {
            alpha = gst_data[3];
            evas_tmp[0] = (gst_data[0] * alpha) / 255;
            evas_tmp[1] = (gst_data[1] * alpha) / 255;
            evas_tmp[2] = (gst_data[2] * alpha) / 255;
            evas_tmp[3] = alpha;
            gst_data += 4;
            evas_tmp += 4;
         }
      }
   }

   if (priv->format == GST_VIDEO_FORMAT_I420) {
      int i;
      unsigned char **rows;

      evas_object_image_pixels_dirty_set(priv->o, 1);
      rows = (unsigned char **)evas_data;

      for (i = 0; i < priv->height; i++)
        rows[i] = &gst_data[i * priv->width];

      rows += priv->height;
      for (i = 0; i < (priv->height / 2); i++)
        rows[i] = &gst_data[priv->height * priv->width + i * (priv->width / 2)];

      rows += priv->height / 2;
      for (i = 0; i < (priv->height / 2); i++)
        rows[i] = &gst_data[priv->height * priv->width + priv->height * (priv->width /4) + i * (priv->width / 2)];
   }

   if (priv->format == GST_VIDEO_FORMAT_YV12) {
      int i;
      unsigned char **rows;

      evas_object_image_pixels_dirty_set(priv->o, 1);

      rows = (unsigned char **)evas_data;

      for (i = 0; i < priv->height; i++)
        rows[i] = &gst_data[i * priv->width];

      rows += priv->height;
      for (i = 0; i < (priv->height / 2); i++)
        rows[i] = &gst_data[priv->height * priv->width + priv->height * (priv->width /4) + i * (priv->width / 2)];

      rows += priv->height / 2;
      for (i = 0; i < (priv->height / 2); i++)
        rows[i] = &gst_data[priv->height * priv->width + i * (priv->width / 2)];
   }

   evas_object_image_data_update_add(priv->o, 0, 0, priv->width, priv->height);
   evas_object_image_data_set(priv->o, evas_data);
   evas_object_image_pixels_dirty_set(priv->o, 0);

   ev = evas_object_data_get(priv->o, "_emotion_gstreamer_video");
   _emotion_frame_new(ev->obj);

   vstream = eina_list_nth(ev->video_streams, ev->video_stream_nbr - 1);

   gst_element_query_position(ev->pipeline, &fmt, &pos);
   ev->position = (double)pos / (double)GST_SECOND;

   vstream->width = priv->width;
   vstream->height = priv->height;
   ev->ratio = (double) priv->width / (double) priv->height;

   _emotion_video_pos_update(ev->obj, ev->position, vstream->length_time);
   _emotion_frame_resize(ev->obj, priv->width, priv->height, ev->ratio);

 exit_point:
   gst_buffer_unref(buffer);

   if (priv->preroll) return ;

   g_mutex_lock(priv->buffer_mutex);

   if (priv->unlocked) {
      g_mutex_unlock(priv->buffer_mutex);
      return;
   }

   g_cond_signal(priv->data_cond);
   g_mutex_unlock(priv->buffer_mutex);
}

static void
unlock_buffer_mutex(EvasVideoSinkPrivate* priv)
{
   g_mutex_lock(priv->buffer_mutex);

   priv->unlocked = EINA_TRUE;
   g_cond_signal(priv->data_cond);
   g_mutex_unlock(priv->buffer_mutex);
}

static void
marshal_VOID__MINIOBJECT(GClosure * closure, GValue * return_value __UNUSED__,
                         guint n_param_values, const GValue * param_values,
                         gpointer invocation_hint __UNUSED__, gpointer marshal_data)
{
   typedef void (*marshalfunc_VOID__MINIOBJECT) (gpointer obj, gpointer arg1, gpointer data2);
   marshalfunc_VOID__MINIOBJECT callback;
   GCClosure *cc;
   gpointer data1, data2;

   cc = (GCClosure *) closure;

   g_return_if_fail(n_param_values == 2);

   if (G_CCLOSURE_SWAP_DATA(closure)) {
      data1 = closure->data;
      data2 = g_value_peek_pointer(param_values + 0);
   } else {
      data1 = g_value_peek_pointer(param_values + 0);
      data2 = closure->data;
   }
   callback = (marshalfunc_VOID__MINIOBJECT) (marshal_data ? marshal_data : cc->callback);

   callback(data1, gst_value_get_mini_object(param_values + 1), data2);
}

static void
evas_video_sink_class_init(EvasVideoSinkClass* klass)
{
   GObjectClass* gobject_class;
   GstBaseSinkClass* gstbase_sink_class;

   gobject_class = G_OBJECT_CLASS(klass);
   gstbase_sink_class = GST_BASE_SINK_CLASS(klass);

   g_type_class_add_private(klass, sizeof(EvasVideoSinkPrivate));

   gobject_class->set_property = evas_video_sink_set_property;
   gobject_class->get_property = evas_video_sink_get_property;

   g_object_class_install_property (gobject_class, PROP_EVAS_OBJECT,
                                    g_param_spec_pointer ("evas-object", "Evas Object",
                                                          "The Evas object where the display of the video will be done",
                                                          G_PARAM_READWRITE));

   g_object_class_install_property (gobject_class, PROP_WIDTH,
                                    g_param_spec_int ("width", "Width",
                                                      "The width of the video",
                                                      0, 65536, 0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

   g_object_class_install_property (gobject_class, PROP_HEIGHT,
                                    g_param_spec_int ("height", "Height",
                                                      "The height of the video",
                                                      0, 65536, 0, G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

   gobject_class->dispose = evas_video_sink_dispose;

   gstbase_sink_class->set_caps = evas_video_sink_set_caps;
   gstbase_sink_class->stop = evas_video_sink_stop;
   gstbase_sink_class->start = evas_video_sink_start;
   gstbase_sink_class->unlock = evas_video_sink_unlock;
   gstbase_sink_class->unlock_stop = evas_video_sink_unlock_stop;
   gstbase_sink_class->render = evas_video_sink_render;
   gstbase_sink_class->preroll = evas_video_sink_preroll;

   evas_video_sink_signals[REPAINT_REQUESTED] = g_signal_new("repaint-requested",
                                                             G_TYPE_FROM_CLASS(klass),
                                                             (GSignalFlags)(G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
                                                             0,
                                                             0,
                                                             0,
                                                             marshal_VOID__MINIOBJECT,
                                                             G_TYPE_NONE, 1, GST_TYPE_BUFFER);
}

gboolean
gstreamer_plugin_init (GstPlugin * plugin)
{
   return gst_element_register (plugin,
                                "emotion-sink",
                                GST_RANK_NONE,
                                EVAS_TYPE_VIDEO_SINK);
}

GstElement *
gstreamer_video_sink_new(Emotion_Gstreamer_Video *ev,
			 Evas_Object *o,
			 const char *uri)
{
   GstElement *playbin;
   GstElement *sink;
   Evas_Object *obj;
   GstStateChangeReturn res;

   obj = _emotion_image_get(o);
   if (!obj)
     {
        ERR("Not Evas_Object specified");
        return NULL;
     }

   playbin = gst_element_factory_make("playbin2", "playbin");
   if (!playbin)
     {
        ERR("Unable to create 'playbin' GstElement.");
        return NULL;
     }

   sink = gst_element_factory_make("emotion-sink", "sink");
   if (!sink)
     {
        ERR("Unable to create 'emotion-sink' GstElement.");
        goto unref_pipeline;
     }

   g_object_set(G_OBJECT(playbin), "video-sink", sink, NULL);
   g_object_set(G_OBJECT(playbin), "uri", uri, NULL);
   g_object_set(G_OBJECT(sink), "evas-object", obj, NULL);

   res = gst_element_set_state(playbin, GST_STATE_PAUSED);
   if (res == GST_STATE_CHANGE_FAILURE)
     {
        ERR("Unable to set GST_STATE_PAUSED.");
        goto unref_pipeline;
     }

   res = gst_element_get_state(playbin, NULL, NULL, GST_CLOCK_TIME_NONE);
   if (res != GST_STATE_CHANGE_SUCCESS)
     {
        ERR("Unable to get GST_CLOCK_TIME_NONE.");
        goto unref_pipeline;
     }

   evas_object_data_set(obj, "_emotion_gstreamer_video", ev);

   return playbin;

 unref_pipeline:
   gst_object_unref(playbin);
   return NULL;
}
