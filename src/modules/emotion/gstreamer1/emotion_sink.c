#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "emotion_gstreamer.h"

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK, GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS(GST_VIDEO_CAPS_MAKE("{ I420, YV12, YUY2, NV12, BGRx, BGR, BGRA }"))); 

GST_DEBUG_CATEGORY_STATIC(evas_video_sink_debug);
#define GST_CAT_DEFAULT evas_video_sink_debug

enum {
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

#define _do_init                                        \
  GST_DEBUG_CATEGORY_INIT(evas_video_sink_debug,        \
                          "emotion-sink",		\
                          0,                            \
                          "emotion video sink")

#define parent_class evas_video_sink_parent_class
G_DEFINE_TYPE_WITH_CODE(EvasVideoSink,
                     evas_video_sink,
                     GST_TYPE_VIDEO_SINK,
                     _do_init);


static void unlock_buffer_mutex(EvasVideoSinkPrivate* priv);
static void evas_video_sink_main_render(void *data);

static void
evas_video_sink_init(EvasVideoSink* sink)
{
   EvasVideoSinkPrivate* priv;

   INF("sink init");
   sink->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE(sink, EVAS_TYPE_VIDEO_SINK, EvasVideoSinkPrivate);
   priv->o = NULL;
   priv->info.width = 0;
   priv->info.height = 0;
   priv->eheight = 0;
   priv->func = NULL;
   priv->eformat = EVAS_COLORSPACE_ARGB8888;
   eina_lock_new(&priv->m);
   eina_condition_new(&priv->c, &priv->m);
   priv->unlocked = EINA_FALSE;
}

/**** Object methods ****/
static void
_cleanup_priv(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
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
       if (priv->o)
         evas_object_event_callback_del(priv->o, EVAS_CALLBACK_DEL, _cleanup_priv);
       priv->o = g_value_get_pointer (value);
       INF("sink set Evas_Object %p.", priv->o);
       if (priv->o)
         evas_object_event_callback_add(priv->o, EVAS_CALLBACK_DEL, _cleanup_priv, priv);
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
       g_value_set_int(value, priv->info.width);
       eina_lock_release(&priv->m);
       break;
    case PROP_HEIGHT:
       INF("sink get height.");
       eina_lock_take(&priv->m);
       g_value_set_int (value, priv->eheight);
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

   G_OBJECT_CLASS(parent_class)->dispose(object);
}


/**** BaseSink methods ****/

gboolean evas_video_sink_set_caps(GstBaseSink *bsink, GstCaps *caps)
{
   EvasVideoSink* sink;
   EvasVideoSinkPrivate* priv;
   GstVideoInfo info;
   unsigned int i;

   sink = EVAS_VIDEO_SINK(bsink);
   priv = sink->priv;

   if (!gst_video_info_from_caps(&info, caps))
     {
        ERR("Unable to parse caps.");
        return FALSE;
     }

   priv->info = info;
   priv->eheight = info.height;

   for (i = 0; colorspace_format_convertion[i].name != NULL; ++i)
     if (info.finfo->format == colorspace_format_convertion[i].format)
       {
          DBG("Found '%s'", colorspace_format_convertion[i].name);
          priv->eformat = colorspace_format_convertion[i].eformat;
          priv->func = colorspace_format_convertion[i].func;
          if (colorspace_format_convertion[i].force_height)
            {
               priv->eheight = (priv->eheight >> 1) << 1;
            }
          if (priv->ev)
            priv->ev->kill_buffer = EINA_TRUE;
          return TRUE;
       }

   ERR("unsupported : %s\n", gst_video_format_to_string(info.finfo->format));
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

   INF("sink preroll %p [%" G_GSIZE_FORMAT "]", buffer, gst_buffer_get_size(buffer));

   sink = EVAS_VIDEO_SINK(bsink);
   priv = sink->priv;

   if (gst_buffer_get_size(buffer) <= 0)
     {
        WRN("empty buffer");
        return GST_FLOW_OK;
     }

   send = emotion_gstreamer_buffer_alloc(priv, buffer, EINA_TRUE);

   if (send)
     {
        _emotion_pending_ecore_begin();
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

   INF("sink render %p", buffer);

   sink = EVAS_VIDEO_SINK(bsink);
   priv = sink->priv;

   eina_lock_take(&priv->m);

   if (priv->unlocked) {
      ERR("LOCKED");
      eina_lock_release(&priv->m);
      return GST_FLOW_FLUSHING;
   }

   send = emotion_gstreamer_buffer_alloc(priv, buffer, EINA_FALSE);
   if (!send) {
      eina_lock_release(&priv->m);
      return GST_FLOW_ERROR;
   }

   _emotion_pending_ecore_begin();
   ecore_main_loop_thread_safe_call_async(evas_video_sink_main_render, send);

   eina_condition_wait(&priv->c);
   eina_lock_release(&priv->m);

   return GST_FLOW_OK;
}

static void
_update_emotion_fps(Emotion_Gstreamer_Video *ev)
{
   double tim;

   if (!debug_fps) return;

   tim = ecore_time_get();
   ev->frames++;

   if (ev->rlapse == 0.0)
     {
        ev->rlapse = tim;
        ev->flapse = ev->frames;
     }
   else if ((tim - ev->rlapse) >= 0.5)
     {
        printf("FRAME: %i, FPS: %3.1f\n",
               ev->frames,
               (ev->frames - ev->flapse) / (tim - ev->rlapse));
        ev->rlapse = tim;
        ev->flapse = ev->frames;
     }
}

static void
evas_video_sink_main_render(void *data)
{
   Emotion_Gstreamer_Buffer *send;
   Emotion_Gstreamer_Video *ev = NULL;
   EvasVideoSinkPrivate *priv = NULL;
   GstBuffer *buffer;
   GstMapInfo map;
   unsigned char *evas_data;
   Eina_Bool preroll = EINA_FALSE;
   double ratio;

   send = data;

   if (!send) goto exit_point;

   priv = send->sink;
   buffer = send->frame;
   preroll = send->preroll;
   ev = send->ev;

   /* frame after cleanup */
   if (!preroll && !ev->last_buffer)
     {
        priv = NULL;
        goto exit_point;
     }

   if (!priv || !priv->o || priv->unlocked)
     goto exit_point;

   if (ev->send && send != ev->send)
     {
        emotion_gstreamer_buffer_free(ev->send);
        ev->send = NULL;
     }

   if (!ev->stream && !send->force)
     {
        ev->send = send;
        _emotion_frame_new(ev->obj);
        evas_object_image_data_update_add(priv->o, 0, 0, priv->info.width, priv->eheight);
        goto exit_stream;
     }

   if (!gst_buffer_map(buffer, &map, GST_MAP_READ))
     goto exit_stream;

   INF("sink main render [%i, %i] (source height: %i)", priv->info.width, priv->eheight, priv->info.height);

   evas_object_image_alpha_set(priv->o, 0);
   evas_object_image_colorspace_set(priv->o, priv->eformat);
   evas_object_image_size_set(priv->o, priv->info.width, priv->eheight);

   evas_data = evas_object_image_data_get(priv->o, 1);

   if (priv->func)
     priv->func(evas_data, map.data, priv->info.width, priv->info.height, priv->eheight);
   else
     WRN("No way to decode %x colorspace !", priv->eformat);

   gst_buffer_unmap(buffer, &map);

   evas_object_image_data_set(priv->o, evas_data);
   evas_object_image_data_update_add(priv->o, 0, 0, priv->info.width, priv->eheight);
   evas_object_image_pixels_dirty_set(priv->o, 0);

   _update_emotion_fps(ev);

   ratio = (double) priv->info.width / (double) priv->eheight;
   ratio *= (double) priv->info.par_n / (double) priv->info.par_d;

   _emotion_frame_resize(ev->obj, priv->info.width, priv->eheight, ratio);

   buffer = gst_buffer_ref(buffer);
   if (ev->last_buffer) gst_buffer_unref(ev->last_buffer);
   ev->last_buffer = buffer;

   _emotion_gstreamer_video_pipeline_parse(ev, EINA_TRUE);

   if (!preroll && ev->play_started)
     {
        _emotion_playback_started(ev->obj);
        ev->play_started = 0;
     }

   if (!send->force)
     {
        _emotion_frame_new(ev->obj);
     }

 exit_point:
   if (send) emotion_gstreamer_buffer_free(send);

 exit_stream:
   if (priv)
     {
        if (preroll || !priv->o)
          {
             _emotion_pending_ecore_end();
             return;
          }
        
        if (!priv->unlocked)
          eina_condition_signal(&priv->c);
     }
   _emotion_pending_ecore_end();
}

static void
unlock_buffer_mutex(EvasVideoSinkPrivate* priv)
{
   priv->unlocked = EINA_TRUE;

   eina_condition_signal(&priv->c);
}

static void
evas_video_sink_class_init(EvasVideoSinkClass* klass)
{
   GObjectClass* gobject_class;
   GstElementClass* gstelement_class;
   GstBaseSinkClass* gstbase_sink_class;

   gobject_class = G_OBJECT_CLASS(klass);
   gstelement_class = GST_ELEMENT_CLASS(klass);
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
                                                          "The internal data of the emotion object",
                                                          G_PARAM_READWRITE));

   gobject_class->dispose = evas_video_sink_dispose;

   gst_element_class_add_pad_template(gstelement_class, gst_static_pad_template_get(&sinktemplate));
   gst_element_class_set_static_metadata(gstelement_class, "Evas video sink",
                                        "Sink/Video", "Sends video data from a GStreamer pipeline to an Evas object",
                                        "Vincent Torri <vtorri@univ-evry.fr>");

   gstbase_sink_class->set_caps = evas_video_sink_set_caps;
   gstbase_sink_class->stop = evas_video_sink_stop;
   gstbase_sink_class->start = evas_video_sink_start;
   gstbase_sink_class->unlock = evas_video_sink_unlock;
   gstbase_sink_class->unlock_stop = evas_video_sink_unlock_stop;
   gstbase_sink_class->render = evas_video_sink_render;
   gstbase_sink_class->preroll = evas_video_sink_preroll;
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
   gboolean res;

   if (ecore_thread_check(thread) || !ev->pipeline) return;

   gst_element_set_state(ev->pipeline, GST_STATE_PAUSED);
   res = gst_element_get_state(ev->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
   if (res == GST_STATE_CHANGE_NO_PREROLL)
     {
        gst_element_set_state(ev->pipeline, GST_STATE_PLAYING);
	gst_element_get_state(ev->pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
     }
}

static void
_emotion_gstreamer_cancel(void *data, Ecore_Thread *thread)
{
   Emotion_Gstreamer_Video *ev = data;

   ev->threads = eina_list_remove(ev->threads, thread);

   if (getenv("EMOTION_GSTREAMER_DOT")) GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(ev->pipeline), GST_DEBUG_GRAPH_SHOW_ALL, getenv("EMOTION_GSTREAMER_DOT"));

   if (ev->in == ev->out && ev->delete_me)
     ev->api->del(ev);
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

   if (ev->in == ev->out && ev->delete_me)
     ev->api->del(ev);
   else
     _emotion_gstreamer_video_pipeline_parse(data, EINA_TRUE);
}

static void
_main_frame_resize(void *data)
{
   Emotion_Gstreamer_Video *ev;
   gint cur;
   GstPad *pad;
   GstCaps *caps;
   GstVideoInfo info;
   double ratio;

   ev = (Emotion_Gstreamer_Video *)data;

   g_object_get(ev->pipeline, "current-video", &cur, NULL);
   g_signal_emit_by_name (ev->pipeline, "get-video-pad", cur, &pad);
   if (!pad)
     goto on_error;

   caps = gst_pad_get_current_caps(pad);
   gst_object_unref(pad);
   if (!caps)
     goto on_error;

   gst_video_info_from_caps (&info, caps);
   gst_caps_unref(caps);

   ratio = (double)info.width / (double)info.height;
   ratio *= (double)info.par_n / (double)info.par_d;

   _emotion_frame_resize(ev->obj, info.width, info.height, ratio);

 on_error:
   _emotion_pending_ecore_end();
}

static void
_video_changed(GstElement *playbin EINA_UNUSED, gpointer data)
{
   Emotion_Gstreamer_Video *ev = data;

   _emotion_pending_ecore_begin();
   ecore_main_loop_thread_safe_call_async(_main_frame_resize, ev);
}

GstElement *
gstreamer_video_sink_new(Emotion_Gstreamer_Video *ev,
			 Evas_Object *o,
			 const char *uri)
{
   GstElement *playbin;
   GstElement *bin = NULL;
   GstElement *esink = NULL;
   GstElement *queue = NULL;
   Evas_Object *obj;
   GstPad *pad;
   int flags;
   const char *launch;

   obj = emotion_object_image_get(o);
   if (!obj)
     {
//        ERR("Not Evas_Object specified");
        return NULL;
     }

   if (!uri)
     return NULL;

   launch = emotion_webcam_custom_get(uri);
   if (launch)
     {
        GError *error = NULL;

        playbin = gst_parse_bin_from_description(launch, 1, &error);
        if (!playbin)
          {
             ERR("Unable to setup command : '%s' got error '%s'.", launch, error->message);
             g_error_free(error);
             return NULL;
          }
        if (error)
          {
             WRN("got recoverable error '%s' for command : '%s'.", error->message, launch);
             g_error_free(error);
          }
     }
   else
     {
        playbin = gst_element_factory_make("playbin", "playbin");
        if (!playbin)
          {
             ERR("Unable to create 'playbin' GstElement.");
             return NULL;
          }
        g_signal_connect(playbin, "video-changed", G_CALLBACK(_video_changed), ev);
     }

   bin = gst_bin_new(NULL);
   if (!bin)
     {
       ERR("Unable to create GstBin !");
       goto unref_pipeline;
     }

   esink = gst_element_factory_make("emotion-sink", "sink");
   if (!esink)
     {
        ERR("Unable to create 'emotion-sink' GstElement.");
        goto unref_pipeline;
     }

   g_object_set(G_OBJECT(esink), "evas-object", obj, NULL);
   g_object_set(G_OBJECT(esink), "ev", ev, NULL);

   evas_object_image_pixels_get_callback_set(obj, NULL, NULL);

   /* We need queue to force each video sink to be in its own thread */
   queue = gst_element_factory_make("queue", "equeue");
   if (!queue)
     {
        ERR("Unable to create 'queue' GstElement.");
        goto unref_pipeline;
     }

   gst_bin_add_many(GST_BIN(bin), queue, esink, NULL);
   gst_element_link_many(queue, esink, NULL);

   /* link both sink to GstTee */
   pad = gst_element_get_static_pad(queue, "sink");
   gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad));
   gst_object_unref(pad);

   if (launch)
     {
        g_object_set(G_OBJECT(playbin), "sink", bin, NULL);
     }
   else
     {
        g_object_get(G_OBJECT(playbin), "flags", &flags, NULL);
        g_object_set(G_OBJECT(playbin), "flags", flags | GST_PLAY_FLAG_NATIVE_VIDEO | GST_PLAY_FLAG_DOWNLOAD | GST_PLAY_FLAG_NATIVE_AUDIO, NULL);
        g_object_set(G_OBJECT(playbin), "video-sink", bin, NULL);
        g_object_set(G_OBJECT(playbin), "uri", uri, NULL);
     }

   evas_object_image_pixels_get_callback_set(obj, NULL, NULL);

   ev->stream = EINA_TRUE;

   eina_stringshare_replace(&ev->uri, uri);
   ev->pipeline = playbin;
   ev->sink = bin;
   ev->esink = esink;
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
   gst_object_unref(esink);
   gst_object_unref(bin);
   gst_object_unref(playbin);
   return NULL;
}
