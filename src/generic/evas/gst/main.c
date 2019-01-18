#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_SHM_OPEN
# include <sys/mman.h>
#endif
#include <fcntl.h>
#include <unistd.h>

#include <gst/gst.h>

#include <Eina.h>

#include "shmfile.h"
#include "timeout.h"

#define DATA32  unsigned int

//#define GST_DBG

#ifdef GST_DBG
#define D(fmt, args...) fprintf(stderr, fmt, ## args)
#else
#define D(fmt, args...)
#endif

#ifdef WORDS_BIGENDIAN
# define CAPS "video/x-raw,format=ARGB"
#else
# define CAPS "video/x-raw,format=BGRA"
#endif

static GstElement *pipeline = NULL;
static GstElement *sink = NULL;
static gint64      duration = -1;

int   width = 0;
int   height = 0;
void *data = NULL;


static Eina_Bool
_gst_init(const char *filename)
{
   GstPad              *pad;
   GstCaps             *caps;
   GstStructure        *structure;
   gchar               *descr;
   gchar               *uri;
   GError              *error = NULL;
   GstFormat            format;
   GstStateChangeReturn ret;
//   int                  vidstr = 0;

   if (!filename || !*filename)
     return EINA_FALSE;

   if (!gst_init_check(NULL, NULL, &error))
     return EINA_FALSE;

   if ((*filename == '/') || (*filename == '~'))
     {
        uri = g_filename_to_uri(filename, NULL, NULL);
        if (!uri)
          {
             D("could not create new uri from %s", filename);
             goto unref_pipeline;
          }
     }
   else
     uri = strdup(filename);

   D("Setting file %s\n", uri);

   descr = g_strdup_printf("uridecodebin uri=%s ! typefind ! videoconvert ! " 
      " appsink name=sink caps=\"" CAPS "\"", uri);
   pipeline = gst_parse_launch(descr, &error);
   free(uri);

   if (error != NULL)
     {
        D("could not construct pipeline: %s\n", error->message);
        g_error_free (error);
        goto gst_shutdown;
     }
/* needs gst 1.0+
 * also only works on playbin objects!!! this is a uridecodebin!
   g_object_get(G_OBJECT(pipeline),
                "n-video", &vidstr,
                NULL);
   if (vidstr <= 0)
     {
        D("no video stream\n");
        goto gst_shutdown;
     }
*/
   sink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");

   ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
   switch (ret)
     {
     case GST_STATE_CHANGE_FAILURE:
        D("failed to play the file\n");
        goto unref_pipeline;
     case GST_STATE_CHANGE_NO_PREROLL:
        D("live sources not supported yet\n");
        goto unref_pipeline;
     default:
        break;
     }

   ret = gst_element_get_state((pipeline), NULL, NULL, GST_CLOCK_TIME_NONE);
   if (ret == GST_STATE_CHANGE_FAILURE)
     {
        D("could not complete pause\n");
        goto unref_pipeline;
     }

   format = GST_FORMAT_TIME;
   gst_element_query_duration (pipeline, format, &duration);
   if (duration == -1)
     {
        fprintf(stderr, "duration fetch err\n");
        D("could not retrieve the duration, set it to 1s\n");
        duration = 1 * GST_SECOND;
     }

   pad = gst_element_get_static_pad(sink, "sink");
   if (!pad)
     {
        D("could not retrieve the sink pad\n");
        goto unref_pipeline;
     }

   caps = gst_pad_get_current_caps(pad);
   if (!caps)
     goto unref_pad;

   structure = gst_caps_get_structure(caps, 0);

   if (!gst_structure_get_int(structure, "width", &width))
     goto unref_caps;
   if (!gst_structure_get_int(structure, "height", &height))
     goto unref_caps;

   gst_caps_unref(caps);
   gst_object_unref(pad);

   return EINA_TRUE;

 unref_caps:
   gst_caps_unref(caps);
 unref_pad:
   gst_object_unref(pad);
 unref_pipeline:
   gst_element_set_state (pipeline, GST_STATE_NULL);
   gst_object_unref(pipeline);
 gst_shutdown:
   gst_deinit();

   return EINA_FALSE;
}

static void
_gst_shutdown()
{
   gst_element_set_state (pipeline, GST_STATE_NULL);
   gst_object_unref(pipeline);
   gst_deinit();
}

static void
_gst_load_image(int size_w EINA_UNUSED, int size_h EINA_UNUSED, double pos)
{
   GstBuffer *buffer;
   GstMapInfo info;
   GstSample *sample;

   D("load image\n");
   if (pos >= 0.0)
     gst_element_seek_simple(pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                             pos);
   else
     gst_element_seek_simple(pipeline, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                             duration / 2);
   g_signal_emit_by_name(sink, "pull-preroll", &sample, NULL);

   shm_alloc(width * height * sizeof(DATA32));
   if (!shm_addr) return;
   data = shm_addr;

   buffer = gst_sample_get_buffer (sample);
   gst_buffer_map (buffer, &info, GST_MAP_READ);
   D("load image: %p %d\n", info.data, info.size);

   memcpy(data, info.data, info.size);

   gst_buffer_unmap(buffer, &info);
}

int
main(int argc, char **argv)
{
   char *file, *p;
   int i, numonly;
   int size_w = 0, size_h = 0;
   int head_only = 0;
   long long pos = -1.0;

   if (argc < 2) return -1;
   // file is ALWAYS first arg, other options come after
   file = argv[1];
   for (i = 2; i < argc; i++)
     {
        if      (!strcmp(argv[i], "-head"))
           // asked to only load header, not body/data
           head_only = 1;
        else if (!strcmp(argv[i], "-key"))
          {
             i++;
             numonly = 1;
             for (p = argv[i]; *p; p++)
               {
                  if ((*p < '0') || (*p > '9'))
                    {
                       numonly = 0;
                       break;
                    }
               }
             if (numonly) pos = atoll(argv[i]) * 1000000;
             i++;
          }
        else if (!strcmp(argv[i], "-opt-scale-down-by"))
          { // not used by ps loader
             i++;
             // int scale_down = atoi(argv[i]);
          }
        else if (!strcmp(argv[i], "-opt-dpi"))
          {
             i++;
          }
        else if (!strcmp(argv[i], "-opt-size"))
          { // not used by ps loader
             i++;
             size_w = atoi(argv[i]);
             i++;
             size_h = atoi(argv[i]);
          }
     }

   timeout_init(10);

   D("_gst_init_file\n");

   if (!_gst_init(file))
     return -1;
   D("_gst_init done\n");

   if ((pos >= 0) && (pos > duration)) return -1;

   if (!head_only)
     {
        _gst_load_image(size_w, size_h, pos);
     }

   D("size...: %ix%i\n", width, height);
   D("alpha..: 0\n");

   printf("size %i %i\n", width, height);
   printf("alpha 0\n");

   if (!head_only)
     {
        if (shm_fd >= 0)
          {
             printf("shmfile %s\n", shmfile);
          }
        else
          {
             // could also to "tmpfile %s\n" like shmfile but just
             // a mmaped tmp file on the system
             printf("data\n");
             if (fwrite(data, width * height * sizeof(DATA32), 1, stdout) != 1)
               {
                  shm_free();
                  return -1;
               }
          }
        shm_free();
     }
   else
     printf("done\n");

   _gst_shutdown();
   fflush(stdout);
   return 0;
}
