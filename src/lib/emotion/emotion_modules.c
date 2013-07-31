#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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

typedef struct _Emotion_Engine_Registry_Entry
{
   const Emotion_Engine *engine;
   int priority;
} Emotion_Engine_Registry_Entry;

static Eina_List *_emotion_engine_registry = NULL;
static Eina_Array *_emotion_modules = NULL;
static Eina_Bool _emotion_modules_loaded = EINA_FALSE;

static void
_emotion_engine_registry_entry_free(Emotion_Engine_Registry_Entry *re)
{
   free(re);
}

static int
_emotion_engine_registry_entry_cmp(const void *pa, const void *pb)
{
   const Emotion_Engine_Registry_Entry *a = pa, *b = pb;
   int r = b->priority - a->priority;

   if (r == 0)
     r = b->engine->priority - a->engine->priority;

   if (r == 0)
     /* guarantee some order to ease debug */
     r = strcmp(b->engine->name, a->engine->name);

   return r;
}

static void
_emotion_modules_load(void)
{
   char buf[PATH_MAX];
   char *path;

   if (_emotion_modules_loaded) return;
   _emotion_modules_loaded = EINA_TRUE;

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

             goto load;
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

   snprintf(buf, sizeof(buf), "%s/emotion/modules", eina_prefix_lib_get(_emotion_pfx));
   _emotion_modules = eina_module_arch_list_get(_emotion_modules, buf, MODULE_ARCH);

 load:
   if (_emotion_modules)
     eina_module_list_load(_emotion_modules);

   if (!_emotion_engine_registry)
     ERR("Couldn't find any emotion engine.");
}

Eina_Bool
emotion_modules_init(void)
{
#ifdef EMOTION_STATIC_BUILD_XINE
   xine_module_init();
#endif
#ifdef EMOTION_STATIC_BUILD_GSTREAMER
   gstreamer_module_init();
#endif
#ifdef EMOTION_STATIC_BUILD_GENERIC
   generic_module_init();
#endif

   return EINA_TRUE;
}

void
emotion_modules_shutdown(void)
{
   Emotion_Engine_Registry_Entry *re;

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

   EINA_LIST_FREE(_emotion_engine_registry, re)
     {
        WRN("Engine was not unregistered: %p", re->engine);
        _emotion_engine_registry_entry_free(re);
     }

   _emotion_modules_loaded = EINA_FALSE;
}

EAPI Eina_Bool
_emotion_module_register(const Emotion_Engine *api)
{
   Emotion_Engine_Registry_Entry *re;

   EINA_SAFETY_ON_NULL_RETURN_VAL(api, EINA_FALSE);

   if (api->version != EMOTION_ENGINE_API_VERSION)
     {
        ERR("Module '%p' uses api version=%u while %u was expected",
            api, api->version, EMOTION_ENGINE_API_VERSION);
        return EINA_FALSE;
     }

   EINA_SAFETY_ON_NULL_RETURN_VAL(api->name, EINA_FALSE);

   INF("register name=%s, version=%u, priority=%d, api=%p",
       api->name, api->version, api->priority, api);

   re = calloc(1, sizeof(Emotion_Engine_Registry_Entry));
   EINA_SAFETY_ON_NULL_RETURN_VAL(re, EINA_FALSE);

   re->engine = api;
   re->priority = api->priority; // TODO: use user-priority from file as weel.

   _emotion_engine_registry = eina_list_sorted_insert
     (_emotion_engine_registry, _emotion_engine_registry_entry_cmp, re);

   return EINA_TRUE;
}

EAPI Eina_Bool
_emotion_module_unregister(const Emotion_Engine *api)
{
   Eina_List *n;
   Emotion_Engine_Registry_Entry *re;

   EINA_SAFETY_ON_NULL_RETURN_VAL(api, EINA_FALSE);
   if (api->version != EMOTION_ENGINE_API_VERSION)
     {
        ERR("Module '%p' uses api version=%u while %u was expected",
            api, api->version, EMOTION_ENGINE_API_VERSION);
        return EINA_FALSE;
     }

   INF("unregister name=%s, api=%p", api->name, api);

   EINA_LIST_FOREACH(_emotion_engine_registry, n, re)
     {
        if (re->engine == api)
          {
             _emotion_engine_registry_entry_free(re);
             _emotion_engine_registry = eina_list_remove_list
               (_emotion_engine_registry, n);
             return EINA_TRUE;
          }
     }

   ERR("module not registered name=%s, api=%p", api->name, api);
   return EINA_FALSE;
}

struct _Emotion_Engine_Instance
{
   const Emotion_Engine *api;
   Evas_Object *obj;
   void *data;
};

#define EMOTION_ENGINE_INSTANCE_CHECK(inst, meth, ...)  \
  do                                                    \
    {                                                   \
       if (!inst)                                       \
         {                                              \
            DBG("no instance to call "#meth);           \
            return __VA_ARGS__;                         \
         }                                              \
       if (!inst->api->meth)                            \
         {                                              \
            DBG("no "#meth" in instance=%p", inst);     \
            return __VA_ARGS__;                         \
         }                                              \
    }                                                   \
  while (0)

#define EMOTION_ENGINE_INSTANCE_CALL(inst, meth, ...)   \
  do                                                    \
    {                                                   \
       EMOTION_ENGINE_INSTANCE_CHECK(inst, meth);       \
       inst->api->meth(inst->data, ## __VA_ARGS__);     \
    }                                                   \
  while (0)

#define EMOTION_ENGINE_INSTANCE_CALL_RET(inst, meth, retval, ...)       \
  do                                                                    \
    {                                                                   \
       EMOTION_ENGINE_INSTANCE_CHECK(inst, meth, retval);               \
       return inst->api->meth(inst->data, ## __VA_ARGS__);              \
    }                                                                   \
  while (0)


static const Emotion_Engine *
_emotion_engine_registry_find(const char *name)
{
   const Eina_List *n;
   const Emotion_Engine_Registry_Entry *re;
   EINA_LIST_FOREACH(_emotion_engine_registry, n, re)
     {
        if (strcmp(re->engine->name, name) == 0)
          return re->engine;
     }
   return NULL;
}

static Emotion_Engine_Instance *
_emotion_engine_instance_new(const Emotion_Engine *engine, Evas_Object *obj, void *data)
{
   Emotion_Engine_Instance *inst = calloc(1, sizeof(Emotion_Engine_Instance));
   EINA_SAFETY_ON_NULL_GOTO(inst, error);
   inst->api = engine;
   inst->obj = obj;
   inst->data = data;
   return inst;

 error:
   engine->del(data);
   return NULL;
}

Emotion_Engine_Instance *
emotion_engine_instance_new(const char *name, Evas_Object *obj, Emotion_Module_Options *opts)
{
   const Eina_List *n;
   const Emotion_Engine_Registry_Entry *re;
   const Emotion_Engine *engine;
   void *data;

   _emotion_modules_load();

   if ((!name) && getenv("EMOTION_ENGINE"))
     {
        name = getenv("EMOTION_ENGINE");
        DBG("using EMOTION_ENGINE=%s", name);
     }

   if (name)
     {
        engine = _emotion_engine_registry_find(name);
        if (!engine)
          ERR("Couldn't find requested engine: %s. Try fallback", name);
        else
          {
             data = engine->add(engine, obj, opts);
             if (data)
               {
                  INF("Using requested engine %s, data=%p", name, data);
                  return _emotion_engine_instance_new(engine, obj, data);
               }

             ERR("Requested engine '%s' could not be used. Try fallback", name);
          }
     }

   EINA_LIST_FOREACH(_emotion_engine_registry, n, re)
     {
        engine = re->engine;
        DBG("Trying engine %s, priority=%d (%d)",
            engine->name, re->priority, engine->priority);

        data = engine->add(engine, obj, opts);
        if (data)
          {
             INF("Using fallback engine %s, data=%p", engine->name, data);
             return _emotion_engine_instance_new(engine, obj, data);
          }
     }

   ERR("No engine worked");
   return NULL;
}

void
emotion_engine_instance_del(Emotion_Engine_Instance *inst)
{
   EINA_SAFETY_ON_NULL_RETURN(inst);
   inst->api->del(inst->data);
}

Eina_Bool
emotion_engine_instance_name_equal(const Emotion_Engine_Instance *inst, const char *name)
{
   /* these are valid, no safety macros here */
   if (!name) return EINA_FALSE;
   if (!inst) return EINA_FALSE;
   return strcmp(name, inst->api->name) == 0;
}

void *
emotion_engine_instance_data_get(const Emotion_Engine_Instance *inst)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(inst, NULL);
   return inst->data;
}

Eina_Bool
emotion_engine_instance_file_open(Emotion_Engine_Instance *inst, const char *file)
{
   EMOTION_ENGINE_INSTANCE_CHECK(inst, file_open, EINA_FALSE);
   return inst->api->file_open(inst->data, file);
}

void
emotion_engine_instance_file_close(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, file_close);
}

void
emotion_engine_instance_play(Emotion_Engine_Instance *inst, double pos)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, play, pos);
}

void
emotion_engine_instance_stop(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, stop);
}

void
emotion_engine_instance_size_get(const Emotion_Engine_Instance *inst, int *w, int *h)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, size_get, w, h);
}

void
emotion_engine_instance_pos_set(Emotion_Engine_Instance *inst, double pos)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, pos_set, pos);
}

double
emotion_engine_instance_len_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, len_get, 0.0);
}

double
emotion_engine_instance_buffer_size_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, buffer_size_get, 1.0);
}

int
emotion_engine_instance_fps_num_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, fps_num_get, 1);
}

int
emotion_engine_instance_fps_den_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, fps_den_get, 1);
}

double
emotion_engine_instance_fps_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, fps_get, 1.0);
}

double
emotion_engine_instance_pos_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, pos_get, 0.0);
}

void
emotion_engine_instance_vis_set(Emotion_Engine_Instance *inst, Emotion_Vis vis)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, vis_set, vis);
}

Emotion_Vis
emotion_engine_instance_vis_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, vis_get, EMOTION_VIS_NONE);
}

Eina_Bool
emotion_engine_instance_vis_supported(Emotion_Engine_Instance *inst, Emotion_Vis vis)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, vis_supported, EINA_FALSE, vis);
}

double
emotion_engine_instance_ratio_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, ratio_get, 0.0);
}

Eina_Bool
emotion_engine_instance_video_handled(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, video_handled, EINA_FALSE);
}

Eina_Bool
emotion_engine_instance_audio_handled(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, audio_handled, EINA_FALSE);
}

Eina_Bool
emotion_engine_instance_seekable(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, seekable, EINA_FALSE);
}

void
emotion_engine_instance_frame_done(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, frame_done);
}

Emotion_Format
emotion_engine_instance_format_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, format_get, EMOTION_FORMAT_NONE);
}

void
emotion_engine_instance_video_data_size_get(const Emotion_Engine_Instance *inst, int *w, int *h)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, video_data_size_get, w, h);
}

Eina_Bool
emotion_engine_instance_yuv_rows_get(const Emotion_Engine_Instance *inst, int w, int h, unsigned char **yrows, unsigned char **urows, unsigned char **vrows)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, yuv_rows_get, EINA_FALSE, w, h, yrows, urows, vrows);
}

Eina_Bool
emotion_engine_instance_bgra_data_get(const Emotion_Engine_Instance *inst, unsigned char **bgra_data)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, bgra_data_get, EINA_FALSE, bgra_data);
}

void
emotion_engine_instance_event_feed(Emotion_Engine_Instance *inst, int event)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, event_feed, event);
}

void
emotion_engine_instance_event_mouse_button_feed(Emotion_Engine_Instance *inst, int button, int x, int y)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, event_mouse_button_feed, button, x, y);
}

void
emotion_engine_instance_event_mouse_move_feed(Emotion_Engine_Instance *inst, int x, int y)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, event_mouse_move_feed, x, y);
}

int
emotion_engine_instance_video_channel_count(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, video_channel_count, 0);
}

void
emotion_engine_instance_video_channel_set(Emotion_Engine_Instance *inst, int channel)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, video_channel_set, channel);
}

int
emotion_engine_instance_video_channel_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, video_channel_get, 0);
}

void
emotion_engine_instance_video_subtitle_file_set(Emotion_Engine_Instance *inst, const char *filepath)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, video_subtitle_file_set, filepath);
}

const char *
emotion_engine_instance_video_subtitle_file_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, video_subtitle_file_get, NULL);
}

const char *
emotion_engine_instance_video_channel_name_get(const Emotion_Engine_Instance *inst, int channel)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, video_channel_name_get, NULL, channel);
}

void
emotion_engine_instance_video_channel_mute_set(Emotion_Engine_Instance *inst, Eina_Bool mute)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, video_channel_mute_set, mute);
}

Eina_Bool
emotion_engine_instance_video_channel_mute_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, video_channel_mute_get, EINA_FALSE);
}

int
emotion_engine_instance_audio_channel_count(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, audio_channel_count, 0);
}

void
emotion_engine_instance_audio_channel_set(Emotion_Engine_Instance *inst, int channel)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, audio_channel_set, channel);
}

int
emotion_engine_instance_audio_channel_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, audio_channel_get, 0);
}

const char *
emotion_engine_instance_audio_channel_name_get(const Emotion_Engine_Instance *inst, int channel)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, audio_channel_name_get, NULL, channel);
}

void
emotion_engine_instance_audio_channel_mute_set(Emotion_Engine_Instance *inst, Eina_Bool mute)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, audio_channel_mute_set, mute);
}

Eina_Bool
emotion_engine_instance_audio_channel_mute_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, audio_channel_mute_get, EINA_FALSE);
}

void
emotion_engine_instance_audio_channel_volume_set(Emotion_Engine_Instance *inst, double vol)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, audio_channel_volume_set, vol);
}

double
emotion_engine_instance_audio_channel_volume_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, audio_channel_volume_get, 0.0);
}

int
emotion_engine_instance_spu_channel_count(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, spu_channel_count, 0);
}

void
emotion_engine_instance_spu_channel_set(Emotion_Engine_Instance *inst, int channel)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, spu_channel_set, channel);
}

int
emotion_engine_instance_spu_channel_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, spu_channel_get, 0);
}

const char *
emotion_engine_instance_spu_channel_name_get(const Emotion_Engine_Instance *inst, int channel)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, spu_channel_name_get, NULL, channel);
}

void
emotion_engine_instance_spu_channel_mute_set(Emotion_Engine_Instance *inst, Eina_Bool mute)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, spu_channel_mute_set, mute);
}

Eina_Bool
emotion_engine_instance_spu_channel_mute_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, spu_channel_mute_get, EINA_FALSE);
}

int
emotion_engine_instance_chapter_count(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, chapter_count, 0);
}

void
emotion_engine_instance_chapter_set(Emotion_Engine_Instance *inst, int chapter)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, chapter_set, chapter);
}

int
emotion_engine_instance_chapter_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, chapter_get, 0);
}

const char *
emotion_engine_instance_chapter_name_get(const Emotion_Engine_Instance *inst, int chapter)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, chapter_name_get, NULL, chapter);
}

void
emotion_engine_instance_speed_set(Emotion_Engine_Instance *inst, double speed)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, speed_set, speed);
}

double
emotion_engine_instance_speed_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, speed_get, 0.0);
}

Eina_Bool
emotion_engine_instance_eject(Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, eject, EINA_FALSE);
}

const char *
emotion_engine_instance_meta_get(const Emotion_Engine_Instance *inst, int meta)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, meta_get, NULL, meta);
}

void
emotion_engine_instance_priority_set(Emotion_Engine_Instance *inst, Eina_Bool priority)
{
   EMOTION_ENGINE_INSTANCE_CALL(inst, priority_set, priority);
}

Eina_Bool
emotion_engine_instance_priority_get(const Emotion_Engine_Instance *inst)
{
   EMOTION_ENGINE_INSTANCE_CALL_RET(inst, priority_get, EINA_FALSE);
}
