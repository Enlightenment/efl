#include <Ecore.h>

#include "emotion_gstreamer.h"

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK, GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS(GST_VIDEO_CAPS_YUV("{ I420, YV12, YUY2, NV12, ST12, TM12 }") ";"
                                                                                   GST_VIDEO_CAPS_BGRx ";" GST_VIDEO_CAPS_BGR ";" GST_VIDEO_CAPS_BGRA));

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
  PROP_EV,
  PROP_LAST
};

static guint evas_video_sink_signals[LAST_SIGNAL] = { 0, };

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
static void evas_video_sink_main_render(void *data);
static void evas_video_sink_samsung_main_render(void *data);

static void
_evas_video_bgrx_step(unsigned char *evas_data, const unsigned char *gst_data,
                      unsigned int w, unsigned int h __UNUSED__, unsigned int output_height, unsigned int step)
{
   unsigned int x;
   unsigned int y;

   for (y = 0; y < output_height; ++y)
     {
        for (x = 0; x < w; x++)
          {
             evas_data[0] = gst_data[0];
             evas_data[1] = gst_data[1];
             evas_data[2] = gst_data[2];
             evas_data[3] = 255;
             gst_data += step;
             evas_data += 4;
          }
     }
}

static void
_evas_video_bgr(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   _evas_video_bgrx_step(evas_data, gst_data, w, h, output_height, 3);
}

static void
_evas_video_bgrx(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   _evas_video_bgrx_step(evas_data, gst_data, w, h, output_height, 4);
}

static void
_evas_video_bgra(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h __UNUSED__, unsigned int output_height)
{
   unsigned int x;
   unsigned int y;

   for (y = 0; y < output_height; ++y)
     {
        unsigned char alpha;

        for (x = 0; x < w; ++x)
          {
             alpha = gst_data[3];
             evas_data[0] = (gst_data[0] * alpha) / 255;
             evas_data[1] = (gst_data[1] * alpha) / 255;
             evas_data[2] = (gst_data[2] * alpha) / 255;
             evas_data[3] = alpha;
             gst_data += 4;
             evas_data += 4;
          }
     }
}

static void
_evas_video_i420(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < rh; i++)
     rows[i] = &gst_data[i * w];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * w + j * (w / 2)];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * w + h * (w / 4) + j * (w / 2)];
   WRN("I420: %i (max: %i for height %i, real height = %i) = %p", i, 2 * h, h, rh, evas_data);
}

static void
_evas_video_yv12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < rh; i++)
     rows[i] = &gst_data[i * w];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * w + h * (w / 4) + j * (w / 2)];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * w + j * (w / 2)];
   WRN("YV12: %i (max: %i for height %i, real height = %i) = %p", i, 2 * h, h, rh, evas_data);
}

static void
_evas_video_yuy2(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h __UNUSED__, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < output_height; i++)
     rows[i] = &gst_data[i * w * 2];
}

static void
_evas_video_nv12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < rh; i++)
     rows[i] = &gst_data[i * w];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &gst_data[h * w + j * w];
}

static void
_evas_video_mt12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height __UNUSED__)
{
   const unsigned char **rows;
   unsigned int i;
   unsigned int j;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < (h / 32) / 2; i++)
     rows[i] = &gst_data[i * w * 2 * 32];

   if ((h / 32) % 2)
     {
        rows[i] = &gst_data[i * w * 2 * 32];
        i++;
     }

   for (j = 0; j < ((h / 2) / 32) / 2; ++j, ++i)
     rows[i] = &gst_data[h * w + j * (w / 2) * 2 * 16];
}

static void
_evas_video_st12_multiplane(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height __UNUSED__)
{
   const GstMultiPlaneImageBuffer *mp_buf = (const GstMultiPlaneImageBuffer *) gst_data;
   const unsigned char **rows;
   unsigned int i;
   unsigned int j;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < (h / 32) / 2; i++)
     rows[i] = mp_buf->uaddr[0] + i * w * 2 * 32;
   if ((h / 32) % 2)
     {
        rows[i] = mp_buf->uaddr[0] + i * w * 2 * 32;
        i++;
     }

   for (j = 0; j < ((h / 2) / 16) / 2; j++, i++)
     rows[i] = mp_buf->uaddr[1] + j * w * 2 * 16;
   if (((h / 2) / 16) % 2)
     rows[i] = mp_buf->uaddr[0] + j * w * 2 * 16;
}

static void
_evas_video_st12(unsigned char *evas_data, const unsigned char *gst_data, unsigned int w, unsigned int h, unsigned int output_height __UNUSED__)
{
   const SCMN_IMGB *imgb = (const SCMN_IMGB *) gst_data;
   const unsigned char **rows;
   unsigned int i, j;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < (h / 32) / 2; i++)
     rows[i] = imgb->uaddr[0] + i * w * 2 * 32;
   if ((h / 32) % 2)
     {
        rows[i] = imgb->uaddr[0] + i * w * 2 * 32;
        i++;
     }

   for (j = 0; j < ((h / 2) / 16) / 2; j++, i++)
     rows[i] = imgb->uaddr[1] + j * w * 2 * 16;
   if (((h / 2) / 16) % 2)
     rows[i] = imgb->uaddr[1] + j * w * 2 * 16;
}

static const struct {
   guint32 fourcc;
   Evas_Colorspace eformat;
   Evas_Video_Convert_Cb func;
   Eina_Bool force_height;
} colorspace_fourcc_convertion[] = {
  { GST_MAKE_FOURCC('I', '4', '2', '0'), EVAS_COLORSPACE_YCBCR422P601_PL, _evas_video_i420, EINA_TRUE },
  { GST_MAKE_FOURCC('Y', 'V', '1', '2'), EVAS_COLORSPACE_YCBCR422P601_PL, _evas_video_yv12, EINA_TRUE },
  { GST_MAKE_FOURCC('Y', 'U', 'Y', '2'), EVAS_COLORSPACE_YCBCR422601_PL, _evas_video_yuy2, EINA_FALSE },
  { GST_MAKE_FOURCC('N', 'V', '1', '2'), EVAS_COLORSPACE_YCBCR420NV12601_PL, _evas_video_nv12, EINA_TRUE },
  { GST_MAKE_FOURCC('T', 'M', '1', '2'), EVAS_COLORSPACE_YCBCR420TM12601_PL, _evas_video_mt12, EINA_TRUE }
};

static const struct {
   GstVideoFormat format;
   Evas_Colorspace eformat;
   Evas_Video_Convert_Cb func;
} colorspace_format_convertion[] = {
  { GST_VIDEO_FORMAT_BGR, EVAS_COLORSPACE_ARGB8888, _evas_video_bgr },
  { GST_VIDEO_FORMAT_BGRx, EVAS_COLORSPACE_ARGB8888, _evas_video_bgrx },
  { GST_VIDEO_FORMAT_BGRA, EVAS_COLORSPACE_ARGB8888, _evas_video_bgra }
};

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

   INF("sink init");
   sink->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE(sink, EVAS_TYPE_VIDEO_SINK, EvasVideoSinkPrivate);
   priv->o = NULL;
   priv->last_buffer = NULL;
   priv->width = 0;
   priv->height = 0;
   priv->func = NULL;
   priv->eformat = EVAS_COLORSPACE_ARGB8888;
   priv->samsung = EINA_FALSE;
   eina_lock_new(&priv->m);
   eina_condition_new(&priv->c, &priv->m);
   priv->unlocked = EINA_FALSE;
}

/**** Object methods ****/
static void
_cleanup_priv(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   EvasVideoSinkPrivate* priv;

   priv = data;

   eina_lock_take(&priv->m);
   if (priv->o == obj)
     priv->o = NULL;
   eina_lock_release(&priv->m);
}

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
       eina_lock_take(&priv->m);
       evas_object_event_callback_del(priv->o, EVAS_CALLBACK_FREE, _cleanup_priv);
       priv->o = g_value_get_pointer (value);
       INF("sink set Evas_Object %p.", priv->o);
       evas_object_event_callback_add(priv->o, EVAS_CALLBACK_FREE, _cleanup_priv, priv);
       eina_lock_release(&priv->m);
       break;
    case PROP_EV:
       INF("sink set ev.");
       eina_lock_take(&priv->m);
       priv->ev = g_value_get_pointer (value);
       eina_lock_release(&priv->m);
       break;
    default:
       G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
       ERR("invalid property");
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
       INF("sink get property.");
       eina_lock_take(&priv->m);
       g_value_set_pointer(value, priv->o);
       eina_lock_release(&priv->m);
       break;
    case PROP_WIDTH:
       INF("sink get width.");
       eina_lock_take(&priv->m);
       g_value_set_int(value, priv->width);
       eina_lock_release(&priv->m);
       break;
    case PROP_HEIGHT:
       INF("sink get height.");
       eina_lock_take(&priv->m);
       g_value_set_int (value, priv->height);
       eina_lock_release(&priv->m);
       break;
    case PROP_EV:
       INF("sink get ev.");
       eina_lock_take(&priv->m);
       g_value_set_pointer (value, priv->ev);
       eina_lock_release(&priv->m);
       break;
    default:
       G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
       ERR("invalide property");
       break;
   }
}

static void
evas_video_sink_dispose(GObject* object)
{
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;

   INF("dispose.");

   sink = EVAS_VIDEO_SINK(object);
   priv = sink->priv;

   eina_lock_free(&priv->m);
   eina_condition_free(&priv->c);

   if (priv->last_buffer) {
      gst_buffer_unref(priv->last_buffer);
      priv->last_buffer = NULL;
   }

   G_OBJECT_CLASS(parent_class)->dispose(object);
}


/**** BaseSink methods ****/

gboolean evas_video_sink_set_caps(GstBaseSink *bsink, GstCaps *caps)
{
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;
   GstStructure *structure;
   GstVideoFormat format;
   guint32 fourcc;
   unsigned int i;

   sink = EVAS_VIDEO_SINK(bsink);
   priv = sink->priv;

   structure = gst_caps_get_structure(caps, 0);

   if (gst_structure_get_int(structure, "width", (int*) &priv->width)
       && gst_structure_get_int(structure, "height", (int*) &priv->height)
       && gst_structure_get_fourcc(structure, "format", &fourcc))
     {
        priv->source_height = priv->height;

        for (i = 0; i < sizeof (colorspace_fourcc_convertion) / sizeof (colorspace_fourcc_convertion[0]); ++i)
          if (fourcc == colorspace_fourcc_convertion[i].fourcc)
            {
               priv->eformat = colorspace_fourcc_convertion[i].eformat;
               priv->func = colorspace_fourcc_convertion[i].func;
               if (colorspace_fourcc_convertion[i].force_height)
                 {
                    priv->height = (priv->height >> 1) << 1;
                 }
               return TRUE;
            }

        if (fourcc == GST_MAKE_FOURCC('S', 'T', '1', '2'))
          {
             priv->eformat = EVAS_COLORSPACE_YCBCR420TM12601_PL;
             priv->samsung = EINA_TRUE;
             priv->func = NULL;
	     return TRUE;
          }
     }

   INF("fallback code !");
   if (!gst_video_format_parse_caps(caps, &format, (int*) &priv->width, (int*) &priv->height))
     {
        ERR("Unable to parse caps.");
        return FALSE;
     }

   priv->source_height = priv->height;

   for (i = 0; i < sizeof (colorspace_format_convertion) / sizeof (colorspace_format_convertion[0]); ++i)
     if (format == colorspace_format_convertion[i].format)
       {
          priv->eformat = colorspace_format_convertion[i].eformat;
          priv->func = colorspace_format_convertion[i].func;
          return TRUE;
       }

   ERR("unsupported : %d\n", format);
   return FALSE;
}

static gboolean
evas_video_sink_start(GstBaseSink* base_sink)
{
   EvasVideoSinkPrivate* priv;
   gboolean res = TRUE;

   INF("sink start");

   priv = EVAS_VIDEO_SINK(base_sink)->priv;
   eina_lock_take(&priv->m);
   if (!priv->o)
     res = FALSE;
   else
     priv->unlocked = EINA_FALSE;
   eina_lock_release(&priv->m);
   return res;
}

static gboolean
evas_video_sink_stop(GstBaseSink* base_sink)
{
   EvasVideoSinkPrivate* priv = EVAS_VIDEO_SINK(base_sink)->priv;

   INF("sink stop");

   unlock_buffer_mutex(priv);
   return TRUE;
}

static gboolean
evas_video_sink_unlock(GstBaseSink* object)
{
   EvasVideoSink* sink;

   INF("sink unlock");

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

   INF("sink unlock stop");

   eina_lock_take(&priv->m);
   priv->unlocked = FALSE;
   eina_lock_release(&priv->m);

   return GST_CALL_PARENT_WITH_DEFAULT(GST_BASE_SINK_CLASS, unlock_stop,
                                       (object), TRUE);
}

static GstFlowReturn
evas_video_sink_preroll(GstBaseSink* bsink, GstBuffer* buffer)
{
   Emotion_Gstreamer_Buffer *send;
   EvasVideoSinkPrivate *priv;
   EvasVideoSink *sink;

   INF("sink preroll %p [%i]", GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));

   sink = EVAS_VIDEO_SINK(bsink);
   priv = sink->priv;

   if (GST_BUFFER_SIZE(buffer) <= 0 && !priv->samsung)
     {
        WRN("empty buffer");
        return GST_FLOW_OK;
     }

   send = emotion_gstreamer_buffer_alloc(priv, buffer, EINA_TRUE);

   if (send)
     {
        if (priv->samsung)
          {
             if (!priv->func)
               {
                  GstStructure *structure;
                  GstCaps *caps;
                  gboolean is_multiplane = FALSE;

                  caps = GST_BUFFER_CAPS(buffer);
                  structure = gst_caps_get_structure (caps, 0);
                  gst_structure_get_boolean(structure, "multiplane", &is_multiplane);

                  if (is_multiplane)
                    priv->func = _evas_video_st12_multiplane;
                  else
                    priv->func = _evas_video_st12;
               }

             ecore_main_loop_thread_safe_call_async(evas_video_sink_samsung_main_render, send);
          }
        else
          ecore_main_loop_thread_safe_call_async(evas_video_sink_main_render, send);
     }

   return GST_FLOW_OK;
}

static GstFlowReturn
evas_video_sink_render(GstBaseSink* bsink, GstBuffer* buffer)
{
   Emotion_Gstreamer_Buffer *send;
   EvasVideoSinkPrivate *priv;
   EvasVideoSink *sink;

   INF("sink render %p [%i]", GST_BUFFER_DATA(buffer), GST_BUFFER_SIZE(buffer));

   sink = EVAS_VIDEO_SINK(bsink);
   priv = sink->priv;

   eina_lock_take(&priv->m);

   if (priv->unlocked) {
      ERR("LOCKED");
      eina_lock_release(&priv->m);
      return GST_FLOW_OK;
   }

   send = emotion_gstreamer_buffer_alloc(priv, buffer, EINA_FALSE);
   if (!send) {
      eina_lock_release(&priv->m);
      return GST_FLOW_ERROR;
   }

   if (priv->samsung)
     {
        if (!priv->func)
          {
             GstStructure *structure;
             GstCaps *caps;
             gboolean is_multiplane = FALSE;

             caps = GST_BUFFER_CAPS(buffer);
             structure = gst_caps_get_structure (caps, 0);
             gst_structure_get_boolean(structure, "multiplane", &is_multiplane);

             if (is_multiplane)
               priv->func = _evas_video_st12_multiplane;
             else
               priv->func = _evas_video_st12;
          }

        ecore_main_loop_thread_safe_call_async(evas_video_sink_samsung_main_render, send);
     }
   else
     ecore_main_loop_thread_safe_call_async(evas_video_sink_main_render, send);

   eina_condition_wait(&priv->c);
   eina_lock_release(&priv->m);

   return GST_FLOW_OK;
}

static void
evas_video_sink_samsung_main_render(void *data)
{
   Emotion_Gstreamer_Buffer *send;
   Emotion_Video_Stream *vstream;
   EvasVideoSinkPrivate* priv;
   GstBuffer* buffer;
   unsigned char *evas_data;
   const guint8 *gst_data;
   GstFormat fmt = GST_FORMAT_TIME;
   gint64 pos;
   Eina_Bool preroll;
   int stride, elevation;
   Evas_Coord w, h;

   send = data;

   if (!send) goto exit_point;

   priv = send->sink;
   buffer = send->frame;
   preroll = send->preroll;

   if (!priv || !priv->o || priv->unlocked || !send->ev)
     goto exit_point;

   _emotion_gstreamer_video_pipeline_parse(send->ev, EINA_TRUE);

   /* Getting stride to compute the right size and then fill the object properly */
   /* Y => [0] and UV in [1] */
   if (priv->func == _evas_video_st12_multiplane)
     {
        const GstMultiPlaneImageBuffer *mp_buf = (const GstMultiPlaneImageBuffer *) buffer;

        stride = mp_buf->stride[0];
        elevation = mp_buf->elevation[0];
        priv->width = mp_buf->width[0];
        priv->height = mp_buf->height[0];

        gst_data = (const guint8 *) mp_buf;
     }
   else
     {
        const SCMN_IMGB *imgb = (const SCMN_IMGB *) GST_BUFFER_MALLOCDATA(buffer);

        stride = imgb->stride[0];
        elevation = imgb->elevation[0];
        priv->width = imgb->width[0];
        priv->height = imgb->height[0];

        gst_data = (const guint8 *) imgb;
     }

   INF("sink main render [%i, %i] - [%i, %i]", priv->width, priv->height, stride, elevation);

   evas_object_image_alpha_set(priv->o, 0);
   evas_object_image_colorspace_set(priv->o, priv->eformat);
   evas_object_image_size_set(priv->o, stride, elevation);
   evas_object_geometry_get(priv->o, NULL, NULL, &w, &h);
   evas_object_image_fill_set(priv->o, 0, 0, stride * w / priv->width, elevation * h / priv->height);

   evas_data = evas_object_image_data_get(priv->o, 1);

   if (priv->func)
     priv->func(evas_data, gst_data, stride, elevation, elevation);
   else
     WRN("No way to decode %x colorspace !", priv->eformat);

   evas_object_image_data_set(priv->o, evas_data);
   evas_object_image_data_update_add(priv->o, 0, 0, priv->width, priv->height);
   evas_object_image_pixels_dirty_set(priv->o, 0);

   _emotion_frame_new(send->ev->obj);

   vstream = eina_list_nth(send->ev->video_streams, send->ev->video_stream_nbr - 1);

   gst_element_query_position(send->ev->pipeline, &fmt, &pos);
   send->ev->position = (double)pos / (double)GST_SECOND;

   vstream->width = priv->width;
   vstream->height = priv->height;
   send->ev->ratio = (double) priv->width / (double) priv->height;

   _emotion_video_pos_update(send->ev->obj, send->ev->position, vstream->length_time);
   _emotion_frame_resize(send->ev->obj, priv->width, priv->height, send->ev->ratio);

   if (priv->last_buffer) gst_buffer_unref(priv->last_buffer);
   priv->last_buffer = gst_buffer_ref(buffer);

 exit_point:
   emotion_gstreamer_buffer_free(send);

   if (preroll || !priv->o || !send->ev) return ;

   eina_lock_take(&priv->m);
   if (!priv->unlocked)
     eina_condition_signal(&priv->c);

   eina_lock_release(&priv->m);
}

static void
evas_video_sink_main_render(void *data)
{
   Emotion_Gstreamer_Buffer *send;
   Emotion_Gstreamer_Video *ev = NULL;
   Emotion_Video_Stream *vstream;
   EvasVideoSinkPrivate* priv;
   GstBuffer* buffer;
   unsigned char *evas_data;
   GstFormat fmt = GST_FORMAT_TIME;
   gint64 pos;
   Eina_Bool preroll;

   send = data;

   if (!send) goto exit_point;

   priv = send->sink;
   buffer = send->frame;
   preroll = send->preroll;
   ev = send->ev;

   if (!priv || !priv->o || priv->unlocked || !ev)
     goto exit_point;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_TRUE);

   INF("sink main render [%i, %i] (source height: %i)", priv->width, priv->height, priv->source_height);

   evas_object_image_alpha_set(priv->o, 0);
   evas_object_image_colorspace_set(priv->o, priv->eformat);
   evas_object_image_size_set(priv->o, priv->width, priv->height);

   evas_data = evas_object_image_data_get(priv->o, 1);

   if (priv->func)
     priv->func(evas_data, GST_BUFFER_DATA(buffer), priv->width, priv->source_height, priv->height);
   else
     WRN("No way to decode %x colorspace !", priv->eformat);

   evas_object_image_data_set(priv->o, evas_data);
   evas_object_image_data_update_add(priv->o, 0, 0, priv->width, priv->height);
   evas_object_image_pixels_dirty_set(priv->o, 0);

   _emotion_frame_new(ev->obj);

   vstream = eina_list_nth(ev->video_streams, ev->video_stream_nbr - 1);

   gst_element_query_position(ev->pipeline, &fmt, &pos);
   ev->position = (double)pos / (double)GST_SECOND;

   vstream->width = priv->width;
   vstream->height = priv->height;
   ev->ratio = (double) priv->width / (double) priv->height;

   _emotion_video_pos_update(ev->obj, ev->position, vstream->length_time);
   _emotion_frame_resize(ev->obj, priv->width, priv->height, ev->ratio);

   if (priv->last_buffer) gst_buffer_unref(priv->last_buffer);
   priv->last_buffer = gst_buffer_ref(buffer);

 exit_point:
   emotion_gstreamer_buffer_free(send);

   if (preroll || !priv->o || !ev) return ;

   eina_lock_take(&priv->m);
   if (!priv->unlocked)
     eina_condition_signal(&priv->c);

   eina_lock_release(&priv->m);
}

static void
unlock_buffer_mutex(EvasVideoSinkPrivate* priv)
{
   eina_lock_take(&priv->m);
   priv->unlocked = EINA_TRUE;

   eina_condition_signal(&priv->c);
   eina_lock_release(&priv->m);
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
   g_object_class_install_property (gobject_class, PROP_EV,
                                    g_param_spec_pointer ("ev", "Emotion_Gstreamer_Video",
                                                          "THe internal data of the emotion object",
                                                          G_PARAM_READWRITE));

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

static void
_emotion_gstreamer_pause(void *data, Ecore_Thread *thread)
{
   Emotion_Gstreamer_Video *ev = data;

   if (ecore_thread_check(thread) || !ev->pipeline) return ;

   gst_element_set_state(ev->pipeline, GST_STATE_PAUSED);
}

static void
_emotion_gstreamer_cancel(void *data, Ecore_Thread *thread)
{
   Emotion_Gstreamer_Video *ev = data;

   ev->threads = eina_list_remove(ev->threads, thread);

   if (getenv("EMOTION_GSTREAMER_DOT")) GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(ev->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, getenv("EMOTION_GSTREAMER_DOT"));

   if (ev->in == ev->out && ev->threads == NULL && ev->delete_me)
     em_shutdown(ev);
}

static void
_emotion_gstreamer_end(void *data, Ecore_Thread *thread)
{
   Emotion_Gstreamer_Video *ev = data;

   ev->threads = eina_list_remove(ev->threads, thread);

   if (ev->play)
     {
        gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);
        ev->play_started = 1;
     }

   if (getenv("EMOTION_GSTREAMER_DOT")) GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(ev->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, getenv("EMOTION_GSTREAMER_DOT"));

   if (ev->in == ev->out && ev->threads == NULL && ev->delete_me)
     em_shutdown(ev);
   else
     _emotion_gstreamer_video_pipeline_parse(data, EINA_TRUE);
}

GstElement *
gstreamer_video_sink_new(Emotion_Gstreamer_Video *ev,
			 Evas_Object *o,
			 const char *uri)
{
   GstElement *playbin;
   GstElement *sink;
   Evas_Object *obj;
   int flags;

   obj = emotion_object_image_get(o);
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

#define GST_PLAY_FLAG_NATIVE_VIDEO  (1 << 6)
#define GST_PLAY_FLAG_DOWNLOAD      (1 << 7)
#define GST_PLAY_FLAG_BUFFERING     (1 << 8)

   g_object_set(G_OBJECT(sink), "evas-object", obj, NULL);
   g_object_set(G_OBJECT(sink), "ev", ev, NULL);

   evas_object_image_pixels_get_callback_set(obj, NULL, NULL);

   g_object_get(G_OBJECT(playbin), "flags", &flags, NULL);
   g_object_set(G_OBJECT(playbin), "flags", flags | GST_PLAY_FLAG_NATIVE_VIDEO | GST_PLAY_FLAG_DOWNLOAD | GST_PLAY_FLAG_BUFFERING, NULL);
   g_object_set(G_OBJECT(playbin), "video-sink", sink, NULL);
   g_object_set(G_OBJECT(playbin), "uri", uri, NULL);

   ev->pipeline = playbin;
   ev->sink = sink;
   ev->threads = eina_list_append(ev->threads,
                                  ecore_thread_run(_emotion_gstreamer_pause,
                                                   _emotion_gstreamer_end,
                                                   _emotion_gstreamer_cancel,
                                                   ev));

   /** NOTE: you need to set: GST_DEBUG_DUMP_DOT_DIR=/tmp EMOTION_ENGINE=gstreamer to save the $EMOTION_GSTREAMER_DOT file in '/tmp' */
   /** then call dot -Tpng -oemotion_pipeline.png /tmp/$TIMESTAMP-$EMOTION_GSTREAMER_DOT.dot */
   if (getenv("EMOTION_GSTREAMER_DOT")) GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(playbin), GST_DEBUG_GRAPH_SHOW_ALL, getenv("EMOTION_GSTREAMER_DOT"));

   return playbin;

 unref_pipeline:
   gst_object_unref(playbin);
   return NULL;
}
