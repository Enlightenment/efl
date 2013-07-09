#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_EEZE
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# ifdef HAVE_V4L2
#  include <sys/ioctl.h>
#  include <linux/videodev2.h>
# endif
# include <Eeze.h>
#endif

#include <Ecore.h>

#include "emotion_private.h"

EAPI int EMOTION_WEBCAM_UPDATE = 0;

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

static Eet_Data_Descriptor *_webcam_edd;
static Eet_Data_Descriptor *_webcams_edd;

static Emotion_Webcams *_emotion_webcams = NULL;

static Eet_Data_Descriptor *
_emotion_webcams_edds_new(void)
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

static void
_emotion_webcams_edds_free(void)
{
   eet_data_descriptor_free(_webcams_edd);
   _webcams_edd = NULL;

   eet_data_descriptor_free(_webcam_edd);
   _webcam_edd = NULL;
}

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

#ifdef HAVE_EEZE
static Eeze_Udev_Watch *eeze_watcher = NULL;

static void
_emotion_check_device(Emotion_Webcam *ew)
{
#ifdef HAVE_V4L2
   Emotion_Webcam *check;
   Eina_List *l;
   struct v4l2_capability caps;
   int fd = -1;
#endif

   if (!ew) return;
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

   if (fd > 0) close(fd);

   return;

 on_error:
#endif
   EINA_LOG_ERR("'%s' is not a webcam ['%s']", ew->name, strerror(errno));
   eina_stringshare_del(ew->syspath);
   eina_stringshare_del(ew->device);
   eina_stringshare_del(ew->name);
   free(ew);
#ifdef HAVE_V4L2
   if (fd > 0) close(fd);
#endif
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
                     void *data EINA_UNUSED,
                     Eeze_Udev_Watch *watcher EINA_UNUSED)
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

Eina_Bool emotion_webcam_init(void)
{
   EMOTION_WEBCAM_UPDATE = ecore_event_type_new();

   eet_init();
   _emotion_webcams_edds_new();

   if (!_emotion_webcams)
     {
        _emotion_webcams = calloc(1, sizeof (Emotion_Webcams));
        EINA_SAFETY_ON_NULL_RETURN_VAL(_emotion_webcams, EINA_FALSE);
     }

#ifdef HAVE_EEZE
   eeze_init();

   _emotion_enumerate_all_webcams();

   eeze_watcher = eeze_udev_watch_add(EEZE_UDEV_TYPE_V4L,
                                      (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE),
                                      _emotion_eeze_events, NULL);
#endif

   return EINA_TRUE;
}

void
emotion_webcam_shutdown(void)
{
   Emotion_Webcam *ew;

   EINA_LIST_FREE(_emotion_webcams->webcams, ew)
     {
        /* There is currently no way to refcount from the outside, this help, but could lead to some issue */
        EINA_REFCOUNT_UNREF(ew)
          emotion_webcam_destroy(ew);
     }
   free(_emotion_webcams);
   _emotion_webcams = NULL;

#ifdef HAVE_EEZE
   eeze_udev_watch_del(eeze_watcher);
   eeze_watcher = NULL;

   eeze_shutdown();
#endif

   _emotion_webcams_edds_free();
   eet_shutdown();
}

Eina_Bool
emotion_webcam_config_load(Eet_File *ef)
{
   Emotion_Webcams *emotion_webcams = NULL;

   if (ef)
     {
        emotion_webcams = eet_data_read(ef, _webcams_edd, "config");
        INF("Loaded config %p from eet %s", _emotion_webcams, eet_file_get(ef));
     }

   if (emotion_webcams)
     {
        if (_emotion_webcams)
          {
             emotion_webcam_shutdown();
             _emotion_webcams = emotion_webcams;
             emotion_webcam_init();
          }
        else
          _emotion_webcams = emotion_webcams;
     }

   if (!_emotion_webcams)
     {
        DBG("No config, create empty");
        _emotion_webcams = calloc(1, sizeof (Emotion_Webcams));
        EINA_SAFETY_ON_NULL_RETURN_VAL(_emotion_webcams, EINA_FALSE);
     }

   return EINA_TRUE;
}

EAPI const Eina_List *
emotion_webcams_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_emotion_webcams, NULL);
   return _emotion_webcams->webcams;
}

EAPI const char *
emotion_webcam_name_get(const Emotion_Webcam *ew)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ew, NULL);
   return ew->name;
}

EAPI const char *
emotion_webcam_device_get(const Emotion_Webcam *ew)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ew, NULL);
   return ew->device;
}

EAPI const char *
emotion_webcam_custom_get(const char *device)
{
   const Emotion_Webcam *ew;
   const Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(_emotion_webcams, NULL);

   EINA_LIST_FOREACH(_emotion_webcams->webcams, l, ew)
     if (ew->device && strcmp(device, ew->device) == 0)
       return ew->custom;

   return NULL;
}
