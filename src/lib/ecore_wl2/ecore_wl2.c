#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef NEED_RUN_IN_TREE
# include "../../static_libs/buildsystem/buildsystem.h"
#endif

#include "ecore_wl2_private.h"

/* local variables */
static int _ecore_wl2_init_count = 0;

/* external variables */
Eina_Bool no_session_recovery;
int _ecore_wl2_log_dom = -1;

/* public API variables */
EAPI int ECORE_WL2_EVENT_CONNECT = 0;
EAPI int ECORE_WL2_EVENT_DISCONNECT = 0;
EAPI int ECORE_WL2_EVENT_GLOBAL_ADDED = 0;
EAPI int ECORE_WL2_EVENT_GLOBAL_REMOVED = 0;
EAPI int ECORE_WL2_EVENT_FOCUS_IN = 0;
EAPI int ECORE_WL2_EVENT_FOCUS_OUT = 0;
EAPI int ECORE_WL2_EVENT_DND_ENTER = 0;
EAPI int ECORE_WL2_EVENT_DND_LEAVE = 0;
EAPI int ECORE_WL2_EVENT_DND_MOTION = 0;
EAPI int ECORE_WL2_EVENT_DND_DROP = 0;
EAPI int ECORE_WL2_EVENT_DND_END = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_END = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_DROP = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_ACTION = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_TARGET = 0;
EAPI int ECORE_WL2_EVENT_DATA_SOURCE_SEND = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_CONFIGURE = 0;
EAPI int ECORE_WL2_EVENT_SYNC_DONE = 0;
EAPI int ECORE_WL2_EVENT_OFFER_DATA_READY = 0;
EAPI int ECORE_WL2_EVENT_SEAT_NAME_CHANGED = 0;
EAPI int ECORE_WL2_EVENT_SEAT_CAPABILITIES_CHANGED = 0;
EAPI int ECORE_WL2_EVENT_DEVICE_ADDED = 0;
EAPI int ECORE_WL2_EVENT_DEVICE_REMOVED = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_CONFIGURE_COMPLETE = 0;
EAPI int ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED = 0;
EAPI int ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED = 0;
EAPI int ECORE_WL2_EVENT_SEAT_SELECTION = 0;
EAPI int ECORE_WL2_EVENT_OUTPUT_TRANSFORM = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE_DONE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_REQUEST = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_DONE = 0;
EAPI int ECORE_WL2_EVENT_AUX_HINT_ALLOWED = 0;
EAPI int ECORE_WL2_EVENT_AUX_HINT_SUPPORTED = 0;
EAPI int ECORE_WL2_EVENT_AUX_MESSAGE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_SHOW = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_HIDE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ACTIVATE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_DEACTIVATE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_ICONIFY_STATE_CHANGE = 0;
EAPI int ECORE_WL2_EVENT_WINDOW_OFFSCREEN = 0;

EAPI int _ecore_wl2_event_window_www = -1;
EAPI int _ecore_wl2_event_window_www_drag = -1;

static Eina_Array *supplied_modules = NULL;
static Eina_Array *local_modules = NULL;

static Eina_Bool
_ecore_wl2_surface_modules_init(void)
{
   const char *mod_dir;

#ifdef NEED_RUN_IN_TREE
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        char path[PATH_MAX];
        //when running in tree we are ignoring all the settings
        //and just load the intree module that we have build
        if (bs_mod_get(path, sizeof(path), "ecore_wl2/engines","dmabuf"))
          {
             Eina_Module *local_module = eina_module_new(path);
             EINA_SAFETY_ON_NULL_RETURN_VAL(local_module, EINA_FALSE);

             if (!eina_module_load(local_module))
               {
                  ERR("Cannot load module %s", path);
                  eina_module_free(local_module);
                  local_module = NULL;
                  return EINA_FALSE;
               }
             eina_module_free(local_module);
             local_module = NULL;
             return EINA_TRUE;
          }
     }
#endif
   supplied_modules = eina_module_arch_list_get(NULL,
                                                PACKAGE_LIB_DIR"/ecore_wl2/engines",
                                                MODULE_ARCH);
   eina_module_list_load(supplied_modules);

   mod_dir = getenv("ECORE_WL2_SURFACE_MODULE_DIR");
   if (mod_dir)
     {
        local_modules = eina_module_list_get(NULL, mod_dir,
                                             EINA_TRUE, NULL, NULL);
        eina_module_list_load(local_modules);
     }

   if (!supplied_modules && !local_modules)
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_ecore_wl2_surface_modules_unload(void)
{
   eina_module_list_unload(supplied_modules);
   eina_module_list_unload(local_modules);
}

/* public API functions */
EAPI int
ecore_wl2_init(void)
{
   if (++_ecore_wl2_init_count != 1) return _ecore_wl2_init_count;

   /* try to initialize Eina */
   if (!eina_init()) return --_ecore_wl2_init_count;

   /* try to create Eina logging domain */
   _ecore_wl2_log_dom =
     eina_log_domain_register("ecore_wl2", ECORE_WL2_DEFAULT_LOG_COLOR);
   if (_ecore_wl2_log_dom < 0)
     {
        EINA_LOG_ERR("Cannot create a log domain for Ecore Wl2");
        goto eina_err;
     }

   /* try to initialize Ecore */
   if (!ecore_init())
     {
        ERR("Could not initialize Ecore");
        goto ecore_err;
     }

   /* try to initialize Ecore_Event */
   if (!ecore_event_init())
     {
        ERR("Could not initialize Ecore_Event");
        goto ecore_event_err;
     }

   if (!_ecore_wl2_surface_modules_init())
     {
        ERR("Could not load surface modules");
        goto module_load_err;
     }

   /* handle creating new Ecore_Wl2 event types */
   ECORE_WL2_EVENT_CONNECT = ecore_event_type_new();
   ECORE_WL2_EVENT_DISCONNECT = ecore_event_type_new();
   ECORE_WL2_EVENT_GLOBAL_ADDED = ecore_event_type_new();
   ECORE_WL2_EVENT_GLOBAL_REMOVED = ecore_event_type_new();
   ECORE_WL2_EVENT_FOCUS_IN = ecore_event_type_new();
   ECORE_WL2_EVENT_FOCUS_OUT = ecore_event_type_new();
   ECORE_WL2_EVENT_DND_ENTER = ecore_event_type_new();
   ECORE_WL2_EVENT_DND_LEAVE = ecore_event_type_new();
   ECORE_WL2_EVENT_DND_MOTION = ecore_event_type_new();
   ECORE_WL2_EVENT_DND_DROP = ecore_event_type_new();
   ECORE_WL2_EVENT_DND_END = ecore_event_type_new();
   ECORE_WL2_EVENT_DATA_SOURCE_END = ecore_event_type_new();
   ECORE_WL2_EVENT_DATA_SOURCE_DROP = ecore_event_type_new();
   ECORE_WL2_EVENT_DATA_SOURCE_ACTION = ecore_event_type_new();
   ECORE_WL2_EVENT_DATA_SOURCE_TARGET = ecore_event_type_new();
   ECORE_WL2_EVENT_DATA_SOURCE_SEND = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_CONFIGURE = ecore_event_type_new();
   ECORE_WL2_EVENT_SYNC_DONE = ecore_event_type_new();
   ECORE_WL2_EVENT_OFFER_DATA_READY = ecore_event_type_new();
   ECORE_WL2_EVENT_SEAT_NAME_CHANGED = ecore_event_type_new();
   ECORE_WL2_EVENT_SEAT_CAPABILITIES_CHANGED = ecore_event_type_new();
   ECORE_WL2_EVENT_DEVICE_ADDED = ecore_event_type_new();
   ECORE_WL2_EVENT_DEVICE_REMOVED = ecore_event_type_new();
   _ecore_wl2_event_window_www = ecore_event_type_new();
   _ecore_wl2_event_window_www_drag = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_CONFIGURE_COMPLETE = ecore_event_type_new();
   ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED = ecore_event_type_new();
   ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED = ecore_event_type_new();
   ECORE_WL2_EVENT_SEAT_SELECTION = ecore_event_type_new();
   ECORE_WL2_EVENT_OUTPUT_TRANSFORM = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_ROTATE = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE_DONE = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_REQUEST = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_DONE = ecore_event_type_new();
   ECORE_WL2_EVENT_AUX_HINT_ALLOWED = ecore_event_type_new();
   ECORE_WL2_EVENT_AUX_HINT_SUPPORTED = ecore_event_type_new();
   ECORE_WL2_EVENT_AUX_MESSAGE = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_SHOW = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_HIDE = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_ACTIVATE = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_DEACTIVATE = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_ICONIFY_STATE_CHANGE = ecore_event_type_new();
   ECORE_WL2_EVENT_WINDOW_OFFSCREEN = ecore_event_type_new();

   if (!no_session_recovery)
     no_session_recovery = !!getenv("EFL_NO_WAYLAND_SESSION_RECOVERY");

   return _ecore_wl2_init_count;

module_load_err:
   ecore_event_shutdown();

ecore_event_err:
   ecore_shutdown();

ecore_err:
   eina_log_domain_unregister(_ecore_wl2_log_dom);
   _ecore_wl2_log_dom = -1;

eina_err:
   eina_shutdown();
   return --_ecore_wl2_init_count;
}

EAPI int
ecore_wl2_shutdown(void)
{
   if (_ecore_wl2_init_count < 1)
     {
        ERR("Ecore_Wl2 shutdown called without Ecore_Wl2 Init");
        return 0;
     }

   if (--_ecore_wl2_init_count != 0) return _ecore_wl2_init_count;

   /* reset events */
   ecore_event_type_flush(ECORE_WL2_EVENT_CONNECT,
                          ECORE_WL2_EVENT_DISCONNECT,
                          ECORE_WL2_EVENT_GLOBAL_ADDED,
                          ECORE_WL2_EVENT_GLOBAL_REMOVED,
                          ECORE_WL2_EVENT_FOCUS_IN,
                          ECORE_WL2_EVENT_FOCUS_OUT,
                          ECORE_WL2_EVENT_DND_ENTER,
                          ECORE_WL2_EVENT_DND_LEAVE,
                          ECORE_WL2_EVENT_DND_MOTION,
                          ECORE_WL2_EVENT_DND_DROP,
                          ECORE_WL2_EVENT_DND_END,
                          ECORE_WL2_EVENT_DATA_SOURCE_END,
                          ECORE_WL2_EVENT_DATA_SOURCE_DROP,
                          ECORE_WL2_EVENT_DATA_SOURCE_ACTION,
                          ECORE_WL2_EVENT_DATA_SOURCE_TARGET,
                          ECORE_WL2_EVENT_DATA_SOURCE_SEND,
                          ECORE_WL2_EVENT_WINDOW_CONFIGURE,
                          ECORE_WL2_EVENT_SYNC_DONE,
                          ECORE_WL2_EVENT_OFFER_DATA_READY,
                          ECORE_WL2_EVENT_SEAT_NAME_CHANGED,
                          ECORE_WL2_EVENT_SEAT_CAPABILITIES_CHANGED,
                          ECORE_WL2_EVENT_DEVICE_ADDED,
                          ECORE_WL2_EVENT_DEVICE_REMOVED,
                          ECORE_WL2_EVENT_WINDOW_CONFIGURE_COMPLETE,
                          ECORE_WL2_EVENT_SEAT_KEYMAP_CHANGED,
                          ECORE_WL2_EVENT_SEAT_KEYBOARD_REPEAT_CHANGED,
                          ECORE_WL2_EVENT_SEAT_SELECTION,
                          ECORE_WL2_EVENT_OUTPUT_TRANSFORM,
                          ECORE_WL2_EVENT_WINDOW_ROTATE,
                          ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE,
                          ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_PREPARE_DONE,
                          ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_REQUEST,
                          ECORE_WL2_EVENT_WINDOW_ROTATION_CHANGE_DONE,
                          ECORE_WL2_EVENT_AUX_HINT_ALLOWED,
                          ECORE_WL2_EVENT_AUX_HINT_SUPPORTED,
                          ECORE_WL2_EVENT_AUX_MESSAGE,
                          ECORE_WL2_EVENT_WINDOW_SHOW,
                          ECORE_WL2_EVENT_WINDOW_HIDE,
                          ECORE_WL2_EVENT_WINDOW_ACTIVATE,
                          ECORE_WL2_EVENT_WINDOW_DEACTIVATE,
                          ECORE_WL2_EVENT_WINDOW_ICONIFY_STATE_CHANGE,
                          ECORE_WL2_EVENT_WINDOW_OFFSCREEN);

   /* shutdown Ecore_Event */
   ecore_event_shutdown();

   /* shutdown Ecore */
   ecore_shutdown();

   /* unregister logging domain */
   eina_log_domain_unregister(_ecore_wl2_log_dom);
   _ecore_wl2_log_dom = -1;

   _ecore_wl2_surface_modules_unload();

   /* shutdown eina */
   eina_shutdown();

   return _ecore_wl2_init_count;
}

EAPI void
ecore_wl2_session_recovery_disable(void)
{
   no_session_recovery = EINA_TRUE;
}
