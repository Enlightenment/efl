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

#include <Eet.h>

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

typedef struct _Emotion_Webcams Emotion_Webcams;

struct _Emotion_Webcams
{
   Eina_List *webcams;
};

struct _Emotion_Webcam
{
   EINA_REFCOUNT;

   const char *syspath;
   const char *device;
   const char *name;

   const char *custom;

   const char *filename;
};

static int _emotion_webcams_count = 0;
static Eet_Data_Descriptor *_webcam_edd;
static Eet_Data_Descriptor *_webcams_edd;

static Emotion_Webcams *_emotion_webcams = NULL;
static Eet_File *_emotion_webcams_file = NULL;

static Eet_Data_Descriptor *
_emotion_webcams_data(void)
{
   Eet_Data_Descriptor_Class eddc;

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Emotion_Webcam);
   _webcam_edd = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_webcam_edd, Emotion_Webcam, "device", device, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_webcam_edd, Emotion_Webcam, "name", name, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_webcam_edd, Emotion_Webcam, "custom", custom, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_webcam_edd, Emotion_Webcam, "filename", filename, EET_T_STRING);

   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Emotion_Webcams);
   _webcams_edd = eet_data_descriptor_file_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_webcams_edd, Emotion_Webcams, "webcams", webcams, _webcam_edd);

   return _webcams_edd;
}

#ifdef EMOTION_HAVE_EEZE
static Eeze_Udev_Watch *eeze_watcher = NULL;

static void
emotion_webcam_destroy(Emotion_Webcam *ew)
{
   if (!ew->custom)
     {
        eina_stringshare_del(ew->syspath);
        eina_stringshare_del(ew->device);
        eina_stringshare_del(ew->name);
     }
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

   EINA_LIST_FOREACH(_emotion_webcams->webcams, l, check)
     if (check->device == ew->device)
       goto on_error;

   _emotion_webcams->webcams = eina_list_append(_emotion_webcams->webcams, ew);

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

   test->custom = NULL;
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
                     void *data __UNUSED__,
                     Eeze_Udev_Watch *watcher __UNUSED__)
{
   if (ev == EEZE_UDEV_EVENT_REMOVE)
     {
        Emotion_Webcam *check;
	Eina_List *l;

        EINA_LIST_FOREACH(_emotion_webcams->webcams, l, check)
          if (check->syspath == syspath)
            {
               _emotion_webcams->webcams = eina_list_remove_list(_emotion_webcams->webcams, l);
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
   char buffer[4096];

   if (_emotion_webcams_count++) return EINA_TRUE;

   snprintf(buffer, 4096, "%s/emotion.cfg", PACKAGE_DATA_DIR);
   _emotion_webcams_file = eet_open(buffer, EET_FILE_MODE_READ);
   if (_emotion_webcams_file)
     {
        Eet_Data_Descriptor *edd;

        edd = _emotion_webcams_data();

        _emotion_webcams = eet_data_read(_emotion_webcams_file, edd, "config");

        eet_data_descriptor_free(_webcams_edd); _webcams_edd = NULL;
        eet_data_descriptor_free(_webcam_edd); _webcam_edd = NULL;
     }

   if (!_emotion_webcams)
     {
        _emotion_webcams = calloc(1, sizeof (Emotion_Webcams));
        if (!_emotion_webcams) return EINA_FALSE;
     }

#ifdef EMOTION_HAVE_EEZE
   EMOTION_WEBCAM_UPDATE = ecore_event_type_new();

   eeze_init();

   _emotion_enumerate_all_webcams();

   eeze_watcher = eeze_udev_watch_add(EEZE_UDEV_TYPE_V4L,
                                      (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE),
                                      _emotion_eeze_events, NULL);
#endif

   return EINA_TRUE;
}

EAPI Eina_Bool
emotion_shutdown(void)
{
   Emotion_Webcam *ew;

   if (--_emotion_webcams_count) return EINA_TRUE;

   EINA_LIST_FREE(_emotion_webcams->webcams, ew)
     {
        /* There is currently no way to refcount from the outside, this help, but could lead to some issue */
        EINA_REFCOUNT_UNREF(ew)
          emotion_webcam_destroy(ew);
     }
   free(_emotion_webcams);
   _emotion_webcams = NULL;

   if (_emotion_webcams_file)
     {
        /* As long as there is no one reference any pointer, you are safe */
        eet_close(_emotion_webcams_file);
        _emotion_webcams_file = NULL;
     }

#ifdef EMOTION_HAVE_EEZE
   eeze_udev_watch_del(eeze_watcher);
   eeze_watcher = NULL;

   eeze_shutdown();
#endif

   return EINA_TRUE;
}

EAPI const Eina_List *
emotion_webcams_get(void)
{
   return _emotion_webcams->webcams;
}

EAPI const char *
emotion_webcam_name_get(const Emotion_Webcam *ew)
{
   if (!ew) return NULL;

   return ew->name;
}

EAPI const char *
emotion_webcam_device_get(const Emotion_Webcam *ew)
{
   if (!ew) return NULL;

   return ew->device;
}

EAPI const char *
emotion_webcam_custom_get(const char *device)
{
   const Emotion_Webcam *ew;
   const Eina_List *l;

   EINA_LIST_FOREACH(_emotion_webcams->webcams, l, ew)
     if (ew->device && strcmp(device, ew->device) == 0)
       return ew->custom;

   return NULL;
}
