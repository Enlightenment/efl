#include <gst/gst.h>
#include <Ecore.h>

static int nbr = 0;

static GstElement *_buid_pipeline (gchar *filename, Ecore_Pipe *pipe);

static void new_decoded_pad_cb (GstElement *demuxer,
                                GstPad     *new_pad,
                                gpointer    user_data);

static void handler(void *data, void *buf, unsigned int len)
{
  GstBuffer  *buffer = *((GstBuffer **)buf);

  printf ("handler : %p\n", buffer);
  printf ("frame  : %d %p %lld %p\n", nbr++, data, (long long)GST_BUFFER_DURATION(buffer), buffer);
  gst_buffer_unref (buffer);
}


static void handoff (GstElement* object,
                     GstBuffer* arg0,
                     GstPad* arg1,
                     gpointer user_data)
{
  Ecore_Pipe *pipe;

  pipe = (Ecore_Pipe *)user_data;
  printf ("handoff : %p\n", arg0);
  gst_buffer_ref (arg0);
  ecore_pipe_write(pipe, &arg0, sizeof(arg0));
}

int
main (int argc, char *argv[])
{
  GstElement *pipeline;
  char *filename;
  Ecore_Pipe *pipe;

  gst_init (&argc, &argv);

  if (!ecore_init ())
    {
      gst_deinit ();
      return 0;
    }

  pipe = ecore_pipe_add (handler);
  if (!pipe)
    {
      ecore_shutdown ();
      gst_deinit ();
      return 0;
    }

  if (argc < 2) {
    g_print ("usage: %s file.avi\n", argv[0]);
    ecore_pipe_del (pipe);
    ecore_shutdown ();
    gst_deinit ();
    return 0;
  }
  filename = argv[1];

  pipeline = _buid_pipeline (filename, pipe);
  if (!pipeline) {
    g_print ("Error during the pipeline building\n");
    ecore_pipe_del (pipe);
    ecore_shutdown ();
    gst_deinit ();
    return -1;
  }

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  ecore_main_loop_begin();

  ecore_pipe_del (pipe);
  ecore_shutdown ();
  gst_deinit ();

  return 0;
}

static void
new_decoded_pad_cb (GstElement *demuxer,
                    GstPad     *new_pad,
                    gpointer    user_data)
{
  GstElement *decoder;
  GstPad *pad;
  GstCaps *caps;
  gchar *str;

  caps = gst_pad_get_caps (new_pad);
  str = gst_caps_to_string (caps);

  if (g_str_has_prefix (str, "video/")) {
    decoder = GST_ELEMENT (user_data);

    pad = gst_element_get_pad (decoder, "sink");
    if (GST_PAD_LINK_FAILED (gst_pad_link (new_pad, pad))) {
      g_warning ("Failed to link %s:%s to %s:%s", GST_DEBUG_PAD_NAME (new_pad),
                 GST_DEBUG_PAD_NAME (pad));
    }
  }
  g_free (str);
  gst_caps_unref (caps);
}

static GstElement
_buid_pipeline (gchar *filename, Ecore_Pipe *pipe)
{
  GstElement          *pipeline;
  GstElement          *filesrc;
  GstElement          *demuxer;
  GstElement          *decoder;
  GstElement          *sink;
  GstStateChangeReturn res;

  pipeline = gst_pipeline_new ("pipeline");
  if (!pipeline)
    return NULL;

  filesrc = gst_element_factory_make ("filesrc", "filesrc");
  if (!filesrc) {
    printf ("no filesrc");
    goto failure;
  }
  g_object_set (G_OBJECT (filesrc), "location", filename, NULL);

  demuxer = gst_element_factory_make ("oggdemux", "demuxer");
  if (!demuxer) {
    printf ("no demux");
    goto failure;
  }

  decoder = gst_element_factory_make ("theoradec", "decoder");
  if (!decoder) {
    printf ("no dec");
    goto failure;
  }

  g_signal_connect (demuxer, "pad-added",
                    G_CALLBACK (new_decoded_pad_cb), decoder);

  sink = gst_element_factory_make ("fakesink", "sink");
  if (!sink) {
    printf ("no sink");
    goto failure;
  }
  g_object_set (G_OBJECT (sink), "sync", EINA_TRUE, NULL);
  g_object_set (G_OBJECT (sink), "signal-handoffs", EINA_TRUE, NULL);
  g_signal_connect (sink, "handoff",
                    G_CALLBACK (handoff), pipe);

  gst_bin_add_many (GST_BIN (pipeline),
                    filesrc, demuxer, decoder, sink, NULL);

  if (!gst_element_link (filesrc, demuxer))
    goto failure;
  if (!gst_element_link (decoder, sink))
    goto failure;

  res = gst_element_set_state (pipeline, GST_STATE_PAUSED);
  if (res == GST_STATE_CHANGE_FAILURE)
    goto failure;

  res = gst_element_get_state( pipeline, NULL, NULL, GST_CLOCK_TIME_NONE );
  if (res != GST_STATE_CHANGE_SUCCESS)
    goto failure;

  return pipeline;

 failure:
  gst_object_unref (GST_OBJECT (pipeline));
  return NULL;
}
