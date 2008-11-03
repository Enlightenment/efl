/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <stdlib.h>
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#ifdef HAVE_EVIL
# include <Evil.h>
#endif
#include "ecore_private.h"
#include "Ecore.h"

struct _Ecore_Pipe
{
   ECORE_MAGIC;
   int                fd_read;
   int                fd_write;
   Ecore_Fd_Handler  *fd_handler;
   void             (*handler) (void *data, void *buffer, int nbyte);
   const void        *data;
};

/*
 * On Windows, pipe() is implemented with sockets.
 * Contrary to Linux, Windows uses different functions
 * for sockets and fd's: write() is for fd's and send
 * is for sockets. So I need to put some win32 code
 * here. I can' think of a solution where the win32
 * code is in Evil and not here.
 */

#ifdef _WIN32

# include <winsock2.h>

# define e_write(fd, buffer, size) send((fd), (char *)(buffer), size, 0)
# define e_read(fd, buffer, size)  recv((fd), (char *)(buffer), size, 0)

#else

# include <unistd.h>
# include <fcntl.h>

# define e_write(fd, buffer, size) write((fd), buffer, size)
# define e_read(fd, buffer, size)  read((fd), buffer, size)
# define e_pipe(mod)               pipe(mod)

#endif /* _WIN32 */


static int _ecore_pipe_read(void             *data,
                            Ecore_Fd_Handler *fd_handler);

/**
 * @defgroup Ecore_Pipe_Group Pipe wrapper
 *
 * These functions wrap the pipe / write / read functions to
 * easily integrate a loop that is in its own thread to the ecore
 * main loop.
 *
 * The ecore_pipe_add() function creates file descriptors (sockets on
 * Windows) and attach an handle to the ecore main loop. That handle is
 * called when data is read in the pipe. To write data in the pipe,
 * just call ecore_pipe_write(). When you are done, just call
 * ecore_pipe_del().
 *
 * Here is an example that uses the pipe wrapper with a Gstreamer
 * pipeline. For each decoded frame in the Gstreamer thread, a handle
 * is called in the ecore thread.
 *
 * @code#include <gst/gst.h>
 * #include <Ecore.h>
 *
 * static int nbr = 0;
 *
 * static GstElement *_buid_pipeline (gchar *filename, Ecore_Pipe *pipe);
 *
 * static void new_decoded_pad_cb (GstElement *demuxer,
 *                                 GstPad     *new_pad,
 *                                 gpointer    user_data);
 *
 * static void handler(void *data, void *buffer, int nbyte)
 * {
 *   GstBuffer  *gbuffer;
 *
 *   printf ("handler : %p\n", data);
 *   gbuffer = buffer;
 *   printf ("frame  : %d %p %lld %p\n", nbr++, data, (long long)GST_BUFFER_DURATION(gbuffer), gbuffer);
 *   gst_buffer_unref (gbuffer);
 * }
 *
 *
 * static void handoff (GstElement* object,
 *                      GstBuffer* arg0,
 *                      GstPad* arg1,
 *                      gpointer user_data)
 * {
 *   Ecore_Pipe *pipe;
 *
 *   pipe = (Ecore_Pipe *)user_data;
 *   printf ("handoff : %p\n", arg0);
 *   gst_buffer_ref (arg0);
 *   ecore_pipe_write(pipe, arg0);
 * }
 *
 * int
 * main (int argc, char *argv[])
 * {
 *   GstElement *pipeline;
 *   char *filename;
 *   Ecore_Pipe *pipe;
 *
 *   gst_init (&argc, &argv);
 *
 *   if (!ecore_init ())
 *     {
 *       gst_deinit ();
 *       return 0;
 *     }
 *
 *   pipe = ecore_pipe_add (handler);
 *   if (!pipe)
 *     {
 *       ecore_shutdown ();
 *       gst_deinit ();
 *       return 0;
 *     }
 *
 *   if (argc < 2) {
 *     g_print ("usage: %s file.avi\n", argv[0]);
 *     ecore_pipe_del (pipe);
 *     ecore_shutdown ();
 *     gst_deinit ();
 *     return 0;
 *   }
 *   filename = argv[1];
 *
 *   pipeline = _buid_pipeline (filename, pipe);
 *   if (!pipeline) {
 *     g_print ("Error during the pipeline building\n");
 *     ecore_pipe_free (pipe);
 *     ecore_shutdown ();
 *     gst_deinit ();
 *     return -1;
 *   }
 *
 *   gst_element_set_state (pipeline, GST_STATE_PLAYING);
 *
 *   ecore_main_loop_begin();
 *
 *   ecore_pipe_free (pipe);
 *   ecore_shutdown ();
 *   gst_deinit ();
 *
 *   return 0;
 * }
 *
 * static void
 * new_decoded_pad_cb (GstElement *demuxer,
 *                     GstPad     *new_pad,
 *                     gpointer    user_data)
 * {
 *   GstElement *decoder;
 *   GstPad *pad;
 *   GstCaps *caps;
 *   gchar *str;
 *
 *   caps = gst_pad_get_caps (new_pad);
 *   str = gst_caps_to_string (caps);
 *
 *   if (g_str_has_prefix (str, "video/")) {
 *     decoder = GST_ELEMENT (user_data);
 *
 *     pad = gst_element_get_pad (decoder, "sink");
 *     if (GST_PAD_LINK_FAILED (gst_pad_link (new_pad, pad))) {
 *       g_warning ("Failed to link %s:%s to %s:%s", GST_DEBUG_PAD_NAME (new_pad),
 *                  GST_DEBUG_PAD_NAME (pad));
 *     }
 *   }
 *   g_free (str);
 *   gst_caps_unref (caps);
 * }
 *
 * static GstElement *
 * _buid_pipeline (gchar *filename, Ecore_Pipe *pipe)
 * {
 *   GstElement          *pipeline;
 *   GstElement          *filesrc;
 *   GstElement          *demuxer;
 *   GstElement          *decoder;
 *   GstElement          *sink;
 *   GstStateChangeReturn res;
 *
 *   pipeline = gst_pipeline_new ("pipeline");
 *   if (!pipeline)
 *     return NULL;
 *
 *   filesrc = gst_element_factory_make ("filesrc", "filesrc");
 *   if (!filesrc) {
 *     printf ("no filesrc");
 *     goto failure;
 *   }
 *   g_object_set (G_OBJECT (filesrc), "location", filename, NULL);
 *
 *   demuxer = gst_element_factory_make ("oggdemux", "demuxer");
 *   if (!demuxer) {
 *     printf ("no demux");
 *     goto failure;
 *   }
 *
 *   decoder = gst_element_factory_make ("theoradec", "decoder");
 *   if (!decoder) {
 *     printf ("no dec");
 *     goto failure;
 *   }
 *
 *   g_signal_connect (demuxer, "pad-added",
 *                     G_CALLBACK (new_decoded_pad_cb), decoder);
 *
 *   sink = gst_element_factory_make ("fakesink", "sink");
 *   if (!sink) {
 *     printf ("no sink");
 *     goto failure;
 *   }
 *   g_object_set (G_OBJECT (sink), "sync", TRUE, NULL);
 *   g_object_set (G_OBJECT (sink), "signal-handoffs", TRUE, NULL);
 *   g_signal_connect (sink, "handoff",
 *                     G_CALLBACK (handoff), pipe);
 *
 *   gst_bin_add_many (GST_BIN (pipeline),
 *                     filesrc, demuxer, decoder, sink, NULL);
 *
 *   if (!gst_element_link (filesrc, demuxer))
 *     goto failure;
 *   if (!gst_element_link (decoder, sink))
 *     goto failure;
 *
 *   res = gst_element_set_state (pipeline, GST_STATE_PAUSED);
 *   if (res == GST_STATE_CHANGE_FAILURE)
 *     goto failure;
 *
 *   res = gst_element_get_state( pipeline, NULL, NULL, GST_CLOCK_TIME_NONE );
 *   if (res != GST_STATE_CHANGE_SUCCESS)
 *     goto failure;
 *
 *   return pipeline;
 *
 *  failure:
 *   gst_object_unref (GST_OBJECT (pipeline));
 *   return NULL;
 * }
 * @endcode
 */


/**
 * Create two file descriptors (sockets on Windows). Add
 * a callback that will be called when the file descriptor that
 * is listened receives data. An event is also put in the event
 * queue when data is received.
 *
 * @param handler The handler called when data is received.
 * @param data    Data to pass to @p handler when it is called.
 * @return        A newly created Ecore_Pipe object if successful.
 *                @c NULL otherwise.
 * @ingroup Ecore_Pipe_Group
 */
EAPI Ecore_Pipe *
ecore_pipe_add(void (*handler) (void *data, void *buffer, int nbyte),
	       const void *data)
{
   Ecore_Pipe       *p;
   Ecore_Fd_Handler *fd_handler;
   int               fds[2];
   
   if (!handler)
     return NULL;
   p = (Ecore_Pipe *)malloc(sizeof (Ecore_Pipe));
   if (!p)
     return NULL;
   
   if (e_pipe(fds))
     {
        free(p);
        return NULL;
     }
   
   ECORE_MAGIC_SET(p, ECORE_MAGIC_PIPE);
   p->fd_read = fds[0];
   p->fd_write = fds[1];
   p->handler = handler;
   p->data = data;
   
#ifndef _WIN32
   fcntl(p->fd_read, F_SETFL, O_NONBLOCK);
#endif /* _WIN32 */
   p->fd_handler = ecore_main_fd_handler_add(p->fd_read,
					     ECORE_FD_READ,
					     _ecore_pipe_read,
					     p,
					     NULL, NULL);

   return p;
}

/**
 * Free an Ecore_Pipe object created with ecore_pipe_add().
 *
 * @param p The Ecore_Pipe object to be freed.
 * @ingroup Ecore_Pipe_Group
 */
EAPI void *
ecore_pipe_del(Ecore_Pipe *p)
{
   void *data;

   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
	ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE,
			 "ecore_pipe_del");
	return NULL;
     }
   ecore_main_fd_handler_del(p->fd_handler);
   close(p->fd_read);
   close(p->fd_write);
   data = (void *)p->data;
   free (p);
   return data;
}

/**
 * Write on the file descriptor the data passed as parameter.
 *
 * @param p      The Ecore_Pipe object.
 * @param data   The data to write into the pipe.
 * @param nbytes The size of the @p data in bytes
 * @ingroup Ecore_Pipe_Group
 */
EAPI void
ecore_pipe_write(Ecore_Pipe *p, const void *buffer, int nbytes)
{
   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
	ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE,
			 "ecore_pipe_write");
	return;
     }
   /* first write the len into the pipe */
   e_write(p->fd_write, &nbytes, sizeof(nbytes));
   
   /* and now pass the data to the pipe */
   e_write(p->fd_write, buffer, nbytes);
}

/* Private function */

static int
_ecore_pipe_read(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Pipe  *p;
   unsigned int len;
   void        *passed_data;
   
   p = (Ecore_Pipe *)data;
   
   /* read the len of the passed data */
   e_read(p->fd_read, &len, sizeof(len));
   
   /* and read the passed data */
   passed_data = malloc(len);
   e_read(p->fd_read, passed_data, len);
   
   p->handler(p->data, passed_data, len);
   free(passed_data);
   
   return ECORE_CALLBACK_RENEW;
}
