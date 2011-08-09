#include "emotion_private.h"
#include "Emotion.h"

#ifdef EMOTION_HAVE_EEZE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
# include <sys/ioctl.h>
# ifdef HAVE_V4L2
#  include <linux/videodev2.h>
# endif

# include <Ecore.h>
# include <Eeze.h>
#endif

EAPI int EMOTION_WEBCAM_UPDATE = 0;

struct ext_match_s
{
   unsigned int length;
   const char *extension;
};

#define MATCHING(Ext)                           \
  { sizeof (Ext), Ext }

static const struct ext_match_s matchs[] =
{ /* map extensions to know if it's a emotion playable content for good first-guess tries */
   MATCHING(".264"),
   MATCHING(".3g2"),
   MATCHING(".3gp"),
   MATCHING(".3gp2"),
   MATCHING(".3gpp"),
   MATCHING(".3gpp2"),
   MATCHING(".3p2"),
   MATCHING(".asf"),
   MATCHING(".avi"),
   MATCHING(".bdm"),
   MATCHING(".bdmv"),
   MATCHING(".clpi"),
   MATCHING(".clp"),
   MATCHING(".fla"),
   MATCHING(".flv"),
   MATCHING(".m1v"),
   MATCHING(".m2v"),
   MATCHING(".m2t"),
   MATCHING(".m4v"),
   MATCHING(".mkv"),
   MATCHING(".mov"),
   MATCHING(".mp2"),
   MATCHING(".mp2ts"),
   MATCHING(".mp4"),
   MATCHING(".mpe"),
   MATCHING(".mpeg"),
   MATCHING(".mpg"),
   MATCHING(".mpl"),
   MATCHING(".mpls"),
   MATCHING(".mts"),
   MATCHING(".mxf"),
   MATCHING(".nut"),
   MATCHING(".nuv"),
   MATCHING(".ogg"),
   MATCHING(".ogm"),
   MATCHING(".ogv"),
   MATCHING(".rm"),
   MATCHING(".rmj"),
   MATCHING(".rmm"),
   MATCHING(".rms"),
   MATCHING(".rmx"),
   MATCHING(".rmvb"),
   MATCHING(".swf"),
   MATCHING(".ts"),
   MATCHING(".weba"),
   MATCHING(".webm"),
   MATCHING(".wmv")
};

Eina_Bool
_emotion_object_extension_can_play_generic_get(const void *data __UNUSED__, const char *file)
{
   unsigned int length;
   unsigned int i;

   length = eina_stringshare_strlen(file) + 1;
   if (length < 5) return EINA_FALSE;

   for (i = 0; i < sizeof (matchs) / sizeof (struct ext_match_s); ++i)
     {
        if (matchs[i].length > length) continue;

        if (!strcasecmp(matchs[i].extension,
                        file + length - matchs[i].length))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
emotion_object_extension_may_play_fast_get(const char *file)
{
   if (!file) return EINA_FALSE;
   return _emotion_object_extension_can_play_generic_get(NULL, file);
}

EAPI Eina_Bool
emotion_object_extension_may_play_get(const char *file)
{
   const char *tmp;
   Eina_Bool result;

   if (!file) return EINA_FALSE;
   tmp = eina_stringshare_add(file);
   result = emotion_object_extension_may_play_fast_get(tmp);
   eina_stringshare_del(tmp);

   return result;
}

#ifdef EMOTION_HAVE_EEZE
struct _Emotion_Webcam
{
   EINA_REFCOUNT;

   const char *syspath;
   const char *device;
   const char *name;

   const char *filename;
};

static int _emotion_webcams_count = 0;
static Eina_List *_emotion_webcams = NULL;
static Eeze_Udev_Watch *eeze_watcher = NULL;

static void
emotion_webcam_destroy(Emotion_Webcam *ew)
{
   eina_stringshare_del(ew->syspath);
   eina_stringshare_del(ew->device);
   eina_stringshare_del(ew->name);
   free(ew);
}

static void
_emotion_check_device(Emotion_Webcam *ew)
{
#ifdef HAVE_V4L2
   Emotion_Webcam *check;
   Eina_List *l;
   struct v4l2_capability caps;
   int fd;
#endif

   if (!ew) return ;
#ifdef HAVE_V4L2
   if (!ew->device) goto on_error;

   fd = open(ew->filename, O_RDONLY);
   if (fd < 0) goto on_error;

   if (ioctl(fd, VIDIOC_QUERYCAP, &caps) == -1) goto on_error;

   /* Likely not a webcam */
   if (!caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) goto on_error;
   if (caps.capabilities & V4L2_CAP_TUNER
       || caps.capabilities & V4L2_CAP_RADIO
       || caps.capabilities & V4L2_CAP_MODULATOR)
     goto on_error;

   EINA_LIST_FOREACH(_emotion_webcams, l, check)
     if (check->device == ew->device)
       goto on_error;

   _emotion_webcams = eina_list_append(_emotion_webcams, ew);

   EINA_REFCOUNT_INIT(ew);

   return ;

 on_error:
#endif
   fprintf(stderr, "'%s' is not a webcam ['%s']\n", ew->name, strerror(errno));
   eina_stringshare_del(ew->syspath);
   eina_stringshare_del(ew->device);
   eina_stringshare_del(ew->name);
   free(ew);
}

static Emotion_Webcam *
_emotion_webcam_new(const char *syspath)
{
   Emotion_Webcam *test;
   const char *device;
   char *local;

   test = malloc(sizeof (Emotion_Webcam));
   if (!test) return NULL;

   test->syspath = eina_stringshare_ref(syspath);
   test->name = eeze_udev_syspath_get_sysattr(syspath, "name");

   device = eeze_udev_syspath_get_property(syspath, "DEVNAME");
   local = alloca(eina_stringshare_strlen(device) + 8);
   snprintf(local, eina_stringshare_strlen(device) + 8, "v4l2://%s", device);
   test->device = eina_stringshare_add(local);
   eina_stringshare_del(device);
   test->filename = test->device + 7;

   return test;
}

static void
_emotion_enumerate_all_webcams(void)
{
   Eina_List *devices;
   const char *syspath;

   devices = eeze_udev_find_by_type(EEZE_UDEV_TYPE_V4L, NULL);

   EINA_LIST_FREE(devices, syspath)
     {
        Emotion_Webcam *test;

        test = _emotion_webcam_new(syspath);
        if (test) _emotion_check_device(test);

        eina_stringshare_del(syspath);
     }
}

static void
_emotion_eeze_events(const char *syspath,
                     Eeze_Udev_Event ev,
                     void *data,
                     Eeze_Udev_Watch *watcher)
{
   if (ev == EEZE_UDEV_EVENT_REMOVE)
     {
        Emotion_Webcam *check;
	Eina_List *l;

        EINA_LIST_FOREACH(_emotion_webcams, l, check)
          if (check->syspath == syspath)
            {
               _emotion_webcams = eina_list_remove_list(_emotion_webcams, l);
               EINA_REFCOUNT_UNREF(check)
                 emotion_webcam_destroy(check);
               break ;
            }
     }
   else if (ev == EEZE_UDEV_EVENT_ADD)
     {
        Emotion_Webcam *test;

        test = _emotion_webcam_new(syspath);
        if (test) _emotion_check_device(test);
     }
   ecore_event_add(EMOTION_WEBCAM_UPDATE, NULL, NULL, NULL);
}

#endif

EAPI Eina_Bool
emotion_init(void)
{
#ifdef EMOTION_HAVE_EEZE
   if (_emotion_webcams_count++) return EINA_TRUE;

   EMOTION_WEBCAM_UPDATE = ecore_event_type_new();

   eeze_init();

   _emotion_enumerate_all_webcams();

   eeze_watcher = eeze_udev_watch_add(EEZE_UDEV_TYPE_V4L,
                                      (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE),
                                      _emotion_eeze_events, NULL);

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
emotion_shutdown(void)
{
#ifdef EMOTION_HAVE_EEZE
   if (--_emotion_webcams_count) return EINA_TRUE;

   eeze_udev_watch_del(eeze_watcher);
   eeze_watcher = NULL;

   eeze_shutdown();

   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

EAPI const Eina_List *
emotion_webcams_get(void)
{
#ifdef EMOTION_HAVE_EEZE
   return _emotion_webcams;
#else
   return NULL;
#endif
}

EAPI const char *
emotion_webcam_name_get(const Emotion_Webcam *ew)
{
   if (!ew) return NULL;

#ifdef EMOTION_HAVE_EEZE
   return ew->name;
#else
   return NULL;
#endif
}

EAPI const char *
emotion_webcam_device_get(const Emotion_Webcam *ew)
{
   if (!ew) return NULL;

#ifdef EMOTION_HAVE_EEZE
   return ew->device;
#else
   return NULL;
#endif
}
