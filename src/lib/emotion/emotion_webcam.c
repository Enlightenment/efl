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

EMOTION_API int EMOTION_WEBCAM_UPDATE = 0;
EMOTION_API int EMOTION_WEBCAM_ADD = 0;
EMOTION_API int EMOTION_WEBCAM_DEL = 0;

typedef struct _Emotion_Webcams Emotion_Webcams;

struct _Emotion_Webcams
{
   Eina_List *webcams;
   Ecore_Idler *idler;
   Eina_List *check_list;
   Eina_Bool init : 1;
};

struct _Emotion_Webcam
{
   EINA_REFCOUNT;

   const char *syspath;
   const char *device;
   const char *name;
   const char *filename;
   Eina_Bool in_list : 1;
};

static Emotion_Webcams *_emotion_webcams = NULL;

static void
emotion_webcam_destroy(Emotion_Webcam *ew)
{
   eina_stringshare_del(ew->syspath);
   eina_stringshare_del(ew->device);
   eina_stringshare_del(ew->name);
   free(ew);
}

#ifdef HAVE_EEZE
static Eeze_Udev_Watch *eeze_watcher = NULL;

static Eina_Bool
_emotion_check_device(Emotion_Webcam *ew)
{
#ifdef HAVE_V4L2
   Emotion_Webcam *check;
   Eina_List *l;
   struct v4l2_capability caps;
   int fd = -1;
#endif

   if (!ew) return EINA_FALSE;
#ifdef HAVE_V4L2
   if (!ew->device) goto on_error;

   fd = open(ew->filename, O_RDONLY);
   if (fd < 0) goto on_error;

   if (ioctl(fd, VIDIOC_QUERYCAP, &caps) == -1) goto on_error;

   // Likely not a webcam
   if (!(caps.capabilities & V4L2_CAP_VIDEO_CAPTURE)) goto on_error;
   if (caps.capabilities &
       (V4L2_CAP_TUNER | V4L2_CAP_RADIO | V4L2_CAP_MODULATOR))
     goto on_error;

   EINA_LIST_FOREACH(_emotion_webcams->webcams, l, check)
     {
        if (check->device == ew->device) goto on_error;
     }
   _emotion_webcams->webcams = eina_list_append(_emotion_webcams->webcams, ew);
   ew->in_list = EINA_TRUE;
   if (fd >= 0) close(fd);
   return EINA_TRUE;

 on_error:
#endif
   INF("'%s' is not a webcam ['%s']", ew->name, strerror(errno));
   emotion_webcam_destroy(ew);
#ifdef HAVE_V4L2
   if (fd >= 0) close(fd);
#endif
   return EINA_FALSE;
}

static Emotion_Webcam *
_emotion_webcam_new(const char *syspath)
{
   Emotion_Webcam *ew;
   const char *device;
   char *local;

   ew = calloc(1, sizeof(Emotion_Webcam));
   if (!ew) return NULL;

   EINA_REFCOUNT_INIT(ew);
   ew->syspath = eina_stringshare_ref(syspath);
   ew->name = eeze_udev_syspath_get_sysattr(syspath, "name");

   device = eeze_udev_syspath_get_property(syspath, "DEVNAME");
   local = alloca(eina_stringshare_strlen(device) + 8);
   snprintf(local, eina_stringshare_strlen(device) + 8, "v4l2://%s", device);
   ew->device = eina_stringshare_add(local);
   eina_stringshare_del(device);
   ew->filename = ew->device + 7;

   return ew;
}

static void
_emotion_webcam_unref(Emotion_Webcam *ew)
{
   EINA_REFCOUNT_UNREF(ew)
     {
        if ((ew->in_list) && (_emotion_webcams))
          {
             _emotion_webcams->webcams =
               eina_list_remove(_emotion_webcams->webcams, ew);
             ew->in_list = EINA_FALSE;
          }
        emotion_webcam_destroy(ew);
     }
}

static void
_emotion_eeze_event_free(void *data EINA_UNUSED, void *ev)
{
   _emotion_webcam_unref(ev);
}

static void
_emotion_webcam_ev_add(const char *syspath)
{
   Emotion_Webcam *ew = _emotion_webcam_new(syspath);
   if (!ew) return;
   if (!_emotion_check_device(ew)) return;
   EINA_REFCOUNT_REF(ew);
   ecore_event_add(EMOTION_WEBCAM_ADD, ew, _emotion_eeze_event_free, NULL);
}

static Eina_Bool
_emotion_process_webcam(void *data)
{
   Emotion_Webcams *webcams = data;
   const char *syspath;

   syspath = eina_list_data_get(webcams->check_list);
   if (!syspath)
     {
        webcams->idler = NULL;
        webcams->init = EINA_TRUE;
        return EINA_FALSE;
     }
   webcams->check_list = eina_list_remove_list(webcams->check_list,
                                               webcams->check_list);
   _emotion_webcam_ev_add(syspath);
   eina_stringshare_del(syspath);
   return EINA_TRUE;
}

static void
_emotion_webcam_remove_cb(void *data EINA_UNUSED, void *ev)
{
   _emotion_webcam_unref(ev);
}

static void
_emotion_eeze_events(const char *syspath, Eeze_Udev_Event ev,
                     void *data EINA_UNUSED,
                     Eeze_Udev_Watch *watcher EINA_UNUSED)
{
   if (ev == EEZE_UDEV_EVENT_REMOVE)
     {
        Emotion_Webcam *ew;
        Eina_List *l;

        EINA_LIST_FOREACH(_emotion_webcams->webcams, l, ew)
          {
             if (ew->syspath == syspath)
               {
                  if (ew->in_list)
                    {
                       _emotion_webcams->webcams =
                         eina_list_remove_list(_emotion_webcams->webcams, l);
                       ew->in_list = EINA_FALSE;
                    }
                  ecore_event_add(EMOTION_WEBCAM_DEL, ew,
                                  _emotion_webcam_remove_cb, NULL);
                  break;
               }
          }
     }
   else if (ev == EEZE_UDEV_EVENT_ADD)
     {
        _emotion_webcam_ev_add(syspath);
     }
   ecore_event_add(EMOTION_WEBCAM_UPDATE, NULL, NULL, NULL);
}

#endif

static void
_emotion_enumerate_all_webcams(void)
{
#ifdef HAVE_EEZE
   Eina_List *devices;
   if (_emotion_webcams->init) return;
   devices = eeze_udev_find_by_type(EEZE_UDEV_TYPE_V4L, NULL);
   _emotion_webcams->check_list = devices;
   _emotion_webcams->idler = ecore_idler_add(_emotion_process_webcam,
                                             _emotion_webcams);
#endif
}


Eina_Bool emotion_webcam_init(void)
{
   EMOTION_WEBCAM_UPDATE = ecore_event_type_new();
   EMOTION_WEBCAM_ADD = ecore_event_type_new();
   EMOTION_WEBCAM_DEL = ecore_event_type_new();

   if (!_emotion_webcams)
     {
        _emotion_webcams = calloc(1, sizeof (Emotion_Webcams));
        EINA_SAFETY_ON_NULL_RETURN_VAL(_emotion_webcams, EINA_FALSE);
     }

#ifdef HAVE_EEZE
   eeze_init();
   eeze_watcher = eeze_udev_watch_add
     (EEZE_UDEV_TYPE_V4L, (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE),
     _emotion_eeze_events, NULL);
#endif

   return EINA_TRUE;
}

void
emotion_webcam_shutdown(void)
{
   Emotion_Webcam *ew;
   const char *syspath;

   ecore_event_type_flush(EMOTION_WEBCAM_UPDATE, EMOTION_WEBCAM_ADD,
                          EMOTION_WEBCAM_DEL);

   if (_emotion_webcams->idler)
     {
        ecore_idler_del(_emotion_webcams->idler);
        _emotion_webcams->idler = NULL;
     }

   EINA_LIST_FREE(_emotion_webcams->check_list, syspath)
     {
        eina_stringshare_del(syspath);
     }

   _emotion_webcams->init = EINA_FALSE;

   EINA_LIST_FREE(_emotion_webcams->webcams, ew)
     {
        ew->in_list = EINA_FALSE;
        // There is currently no way to refcount from the outside, this helps
        // but could lead to some issues
        EINA_REFCOUNT_UNREF(ew)
          {
             emotion_webcam_destroy(ew);
          }
     }
   free(_emotion_webcams);
   _emotion_webcams = NULL;

#ifdef HAVE_EEZE
   eeze_udev_watch_del(eeze_watcher);
   eeze_watcher = NULL;
   eeze_shutdown();
#endif
}

EMOTION_API const Eina_List *
emotion_webcams_get(void)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(_emotion_webcams, NULL);
   _emotion_enumerate_all_webcams();
   return _emotion_webcams->webcams;
}

EMOTION_API const char *
emotion_webcam_name_get(const Emotion_Webcam *ew)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ew, NULL);
   return ew->name;
}

EMOTION_API const char *
emotion_webcam_device_get(const Emotion_Webcam *ew)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ew, NULL);
   return ew->device;
}

EMOTION_API const char *
emotion_webcam_custom_get(const char *device EINA_UNUSED)
{
   return NULL;
}
