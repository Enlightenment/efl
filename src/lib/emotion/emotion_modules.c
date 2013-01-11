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

static Eina_Hash *_emotion_backends = NULL;
static Eina_Array *_emotion_modules = NULL;

static void
_emotion_modules_load(void)
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

   snprintf(buf, sizeof(buf), "%s/emotion/modules", eina_prefix_lib_get(_emotion_pfx));
   _emotion_modules = eina_module_arch_list_get(_emotion_modules, buf, MODULE_ARCH);
}

Eina_Bool
emotion_modules_init(void)
{
   int static_modules = 0;

   _emotion_backends = eina_hash_string_small_new(free);
   EINA_SAFETY_ON_NULL_RETURN_VAL(_emotion_backends, EINA_FALSE);

   _emotion_modules_load();

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

   return EINA_TRUE;
}

void
emotion_modules_shutdown(void)
{
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
}

EAPI Eina_Bool
_emotion_module_register(const char *name, Emotion_Module_Open mod_open, Emotion_Module_Close mod_close)
{
   Eina_Emotion_Plugins *plugin;

   plugin = malloc(sizeof (Eina_Emotion_Plugins));
   EINA_SAFETY_ON_NULL_RETURN_VAL(plugin, EINA_FALSE);

   plugin->open = mod_open;
   plugin->close = mod_close;

   INF("register module=%s, open=%p, close=%p", name, mod_open, mod_close);
   return eina_hash_add(_emotion_backends, name, plugin);
}

EAPI Eina_Bool
_emotion_module_unregister(const char *name)
{
   INF("unregister module=%s", name);
   return eina_hash_del_by_key(_emotion_backends, name);
}

struct _Emotion_Engine_Instance
{
   Eina_Emotion_Plugins *plugin;
   Emotion_Video_Module *api;
   Evas_Object *obj;
   char *name;
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


static const char *_backend_priority[] = {
  "gstreamer",
  "xine",
  "generic"
};

Emotion_Engine_Instance *
emotion_engine_instance_new(const char *name, Evas_Object *obj, Emotion_Module_Options *opts)
{
   // TODO: rewrite
   Eina_Emotion_Plugins *plugin;
   unsigned int i = 0;
   Emotion_Video_Module *mod = NULL;
   void *data = NULL;

   if (!_emotion_backends)
     {
        ERR("No backend loaded");
        return NULL;
     }

   if (!name && getenv("EMOTION_ENGINE"))
     {
        name = getenv("EMOTION_ENGINE");
        DBG("using EMOTION_ENGINE=%s", name);
     }

   /* FIXME: Always look for a working backend. */
 retry:
   if (!name || i > 0)
     name = _backend_priority[i++];

   plugin = eina_hash_find(_emotion_backends, name);
   DBG("try engine=%s, plugin=%p", name, plugin);
   if (!plugin)
     {
        if (i != 0 && i < (sizeof (_backend_priority) / sizeof (char*)))
          goto retry;

        ERR("No backend loaded");
        return NULL;
     }

   if (plugin->open(obj, (const Emotion_Video_Module **) &mod, &data, opts))
     {
        Emotion_Engine_Instance *inst = calloc(1, sizeof(Emotion_Engine_Instance));
        INF("opened %s, mod=%p, video=%p", name, mod, data);
        inst->plugin = plugin;
        inst->api = mod;
        inst->obj = obj;
        inst->data = data;
        inst->name = strdup(name);
        return inst;
     }

   if (i != 0 && i < (sizeof (_backend_priority) / sizeof (char*)))
     goto retry;

   ERR("Unable to load module: %s", name);

   return NULL;
}

void
emotion_engine_instance_del(Emotion_Engine_Instance *inst)
{
   EINA_SAFETY_ON_NULL_RETURN(inst);
   inst->plugin->close(inst->api, inst->data); // TODO: weird api
}

Eina_Bool
emotion_engine_instance_name_equal(const Emotion_Engine_Instance *inst, const char *name)
{
   /* these are valid, no safety macros here */
   if (!name) return EINA_FALSE;
   if (!inst) return EINA_FALSE;
   return strcmp(name, inst->name) == 0;
}

void *
emotion_engine_instance_data_get(const Emotion_Engine_Instance *inst)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(inst, NULL);
   return inst->data;
}

unsigned char
emotion_engine_instance_file_open(Emotion_Engine_Instance *inst, const char *file)
{
   EMOTION_ENGINE_INSTANCE_CHECK(inst, file_open, EINA_FALSE);
   return inst->api->file_open(file, inst->obj, inst->data); // TODO: weird api
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
