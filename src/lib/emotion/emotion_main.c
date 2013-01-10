#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <stdio.h>

#ifdef HAVE_EEZE
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
#include <Eet.h>

#include "Emotion.h"
#include "emotion_private.h"

#ifdef EMOTION_STATIC_BUILD_XINE
Eina_Bool xine_module_init(void);
void      xine_module_shutdown(void);
#endif
#ifdef EMOTION_STATIC_BUILD_GSTREAMER
Eina_Bool gstreamer_module_init(void);
void      gstreamer_module_shutdown(void);
#endif
#ifdef EMOTION_STATIC_BUILD_GENERIC
Eina_Bool generic_module_init(void);
void      generic_module_shutdown(void);
#endif

static Emotion_Version _version = { VMAJ, VMIN, VMIC, VREV };
static int emotion_pending_objects = 0;
EAPI Emotion_Version *emotion_version = &_version;

static Eina_Prefix *pfx = NULL;
Eina_Hash *_emotion_backends = NULL;
Eina_Array *_emotion_modules = NULL;
int _emotion_log_domain = -1;

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
_emotion_object_extension_can_play_generic_get(const void *data EINA_UNUSED, const char *file)
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
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   return _emotion_object_extension_can_play_generic_get(NULL, file);
}

EAPI Eina_Bool
emotion_object_extension_may_play_get(const char *file)
{
   const char *tmp;
   Eina_Bool result;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
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

static int _emotion_init_count = 0;
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
   EINA_LOG_ERR("'%s' is not a webcam ['%s']", ew->name, strerror(errno));
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

static void
_emotion_modules_init(void)
{
   char buf[PATH_MAX];
   char *path;

   if (getenv("EFL_RUN_IN_TREE"))
     {
        struct stat st;
        snprintf(buf, sizeof(buf), "%s/src/modules/emotion",
                 PACKAGE_BUILD_DIR);
        if (stat(buf, &st) == 0)
          {
             const char *built_modules[] = {
#ifdef EMOTION_BUILD_GSTREAMER
               "gstreamer",
#endif
#ifdef EMOTION_BUILD_XINE
               "xine",
#endif
               NULL
             };
             const char **itr;
             for (itr = built_modules; *itr != NULL; itr++)
               {
                  snprintf(buf, sizeof(buf),
                           "%s/src/modules/emotion/%s/.libs",
                           PACKAGE_BUILD_DIR, *itr);
                  _emotion_modules = eina_module_list_get(_emotion_modules, buf,
                                                          EINA_FALSE, NULL, NULL);
               }

             return;
          }
     }

   path = eina_module_environment_path_get("EMOTION_MODULES_DIR",
                                           "/emotion/modules");
   if (path)
     {
        _emotion_modules = eina_module_arch_list_get(_emotion_modules, path, MODULE_ARCH);
        free(path);
     }

   path = eina_module_environment_path_get("HOME", "/.emotion");
   if (path)
     {
        _emotion_modules = eina_module_arch_list_get(_emotion_modules, path, MODULE_ARCH);
        free(path);
     }

   snprintf(buf, sizeof(buf), "%s/emotion/modules", eina_prefix_lib_get(pfx));
   _emotion_modules = eina_module_arch_list_get(_emotion_modules, buf, MODULE_ARCH);
}

EAPI Eina_Bool
emotion_init(void)
{
   char buffer[PATH_MAX];
   int static_modules = 0;

   if (_emotion_init_count > 0)
     {
        _emotion_init_count++;
        return EINA_TRUE;
     }

   eina_init();

   _emotion_log_domain = eina_log_domain_register("emotion", EINA_COLOR_LIGHTCYAN);
   if (_emotion_log_domain < 0)
     {
        EINA_LOG_CRIT("Could not register log domain 'emotion'");
        eina_shutdown();
        return EINA_FALSE;
     }

   pfx = eina_prefix_new(NULL, emotion_init,
                         "EMOTION", "emotion", "checkme",
                         PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
   EINA_SAFETY_ON_NULL_GOTO(pfx, error);

   _emotion_backends = eina_hash_string_small_new(free);
   EINA_SAFETY_ON_NULL_GOTO(_emotion_backends, error_hash);

   _emotion_modules_init();

   /* Init static module */
#ifdef EMOTION_STATIC_BUILD_XINE
   static_modules += xine_module_init();
#endif
#ifdef EMOTION_STATIC_BUILD_GSTREAMER
   static_modules += gstreamer_module_init();
#endif
#ifdef EMOTION_STATIC_BUILD_GENERIC
   static_modules += generic_module_init();
#endif

   if ((!_emotion_modules) && (!static_modules))
     WRN("No emotion modules found!");
   else if (_emotion_modules)
     eina_module_list_load(_emotion_modules);

   ecore_init();
   eet_init();

   snprintf(buffer, sizeof(buffer), "%s/emotion.cfg", eina_prefix_data_get(pfx));
   _emotion_webcams_file = eet_open(buffer, EET_FILE_MODE_READ);
   if (_emotion_webcams_file)
     {
        Eet_Data_Descriptor *edd = _emotion_webcams_data();

        _emotion_webcams = eet_data_read(_emotion_webcams_file, edd, "config");

        INF("Loaded config %p from eet: %s", _emotion_webcams_file, buffer);
        eet_data_descriptor_free(_webcams_edd); _webcams_edd = NULL;
        eet_data_descriptor_free(_webcam_edd); _webcam_edd = NULL;
     }

   if (!_emotion_webcams)
     {
        DBG("No config at %s, create empty", buffer);
        _emotion_webcams = calloc(1, sizeof (Emotion_Webcams));
        EINA_SAFETY_ON_NULL_GOTO(_emotion_webcams, error_webcams_alloc);
     }

#ifdef HAVE_EEZE
   EMOTION_WEBCAM_UPDATE = ecore_event_type_new();

   eeze_init();

   _emotion_enumerate_all_webcams();

   eeze_watcher = eeze_udev_watch_add(EEZE_UDEV_TYPE_V4L,
                                      (EEZE_UDEV_EVENT_ADD | EEZE_UDEV_EVENT_REMOVE),
                                      _emotion_eeze_events, NULL);
#endif

   _emotion_init_count = 1;
   return EINA_TRUE;

 error_webcams_alloc:

#ifdef EMOTION_STATIC_BUILD_XINE
   xine_module_shutdown();
#endif
#ifdef EMOTION_STATIC_BUILD_GSTREAMER
   gstreamer_module_shutdown();
#endif
#ifdef EMOTION_STATIC_BUILD_GENERIC
   generic_module_shutdown();
#endif

   if (_emotion_modules)
     {
        eina_module_list_free(_emotion_modules);
        eina_array_free(_emotion_modules);
        _emotion_modules = NULL;
     }

   eina_hash_free(_emotion_backends);
   _emotion_backends = NULL;

 error_hash:
   eina_prefix_free(pfx);
   pfx = NULL;

 error:
   eina_log_domain_unregister(_emotion_log_domain);
   _emotion_log_domain = -1;

   eina_shutdown();
   return EINA_FALSE;
}

EAPI Eina_Bool
emotion_shutdown(void)
{
   Emotion_Webcam *ew;
   double start;

   if (_emotion_init_count <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in emotion shutdown.");
        return EINA_FALSE;
     }
   if (--_emotion_init_count) return EINA_TRUE;

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

#ifdef HAVE_EEZE
   eeze_udev_watch_del(eeze_watcher);
   eeze_watcher = NULL;

   eeze_shutdown();
#endif

   start = ecore_time_get();
   while (emotion_pending_objects && ecore_time_get() - start < 0.5)
     ecore_main_loop_iterate();

   if (emotion_pending_objects)
     {
        EINA_LOG_ERR("There is still %i Emotion pipeline running", emotion_pending_objects);
     }

   eet_shutdown();
   ecore_shutdown();

#ifdef EMOTION_STATIC_BUILD_XINE
   xine_module_shutdown();
#endif
#ifdef EMOTION_STATIC_BUILD_GSTREAMER
   gstreamer_module_shutdown();
#endif
#ifdef EMOTION_STATIC_BUILD_GENERIC
   generic_module_shutdown();
#endif

   if (_emotion_modules)
     {
        eina_module_list_free(_emotion_modules);
        eina_array_free(_emotion_modules);
        _emotion_modules = NULL;
     }

   eina_hash_free(_emotion_backends);
   _emotion_backends = NULL;

   eina_prefix_free(pfx);
   pfx = NULL;

   eina_log_domain_unregister(_emotion_log_domain);
   _emotion_log_domain = -1;

   eina_shutdown();

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

EAPI void
_emotion_pending_object_ref(void)
{
   emotion_pending_objects++;
}

EAPI void
_emotion_pending_object_unref(void)
{
   emotion_pending_objects--;
}
