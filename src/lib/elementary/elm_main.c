#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifndef _WIN32
# include <dlfcn.h> /* dlopen,dlclose,etc */
#endif

#ifdef HAVE_CRT_EXTERNS_H
# include <crt_externs.h>
#endif

#ifdef _WIN32
# include <Evil.h>
#endif

#include <Emotion.h>

#include <Elementary.h>

#include "eina_internal.h"
#include "ecore_internal.h"

#include "elm_priv.h"
#include "elm_interface_scrollable.h"

//we need those for legacy compatible code
#include "elm_genlist.eo.h"
#include "elm_gengrid.eo.h"

#define SEMI_BROKEN_QUICKLAUNCH 1

#ifdef __CYGWIN__
# define LIBEXT ".dll"
#else
# define LIBEXT ".so"
#endif

Eina_Bool _use_build_config;

static Elm_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Elm_Version *elm_version = &_version;

void
_efl_ui_focus_manager_redirect_events_del(Efl_Ui_Focus_Manager *manager, Eo *obj)
{
   efl_event_callback_forwarder_del(manager, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, obj);
   efl_event_callback_forwarder_del(manager, EFL_UI_FOCUS_MANAGER_EVENT_REDIRECT_CHANGED, obj);
   efl_event_callback_forwarder_del(manager, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED , obj);
   efl_event_callback_forwarder_del(manager, EFL_UI_FOCUS_MANAGER_EVENT_COORDS_DIRTY, obj);
}

void
_efl_ui_focus_manager_redirect_events_add(Efl_Ui_Focus_Manager *manager, Eo *obj)
{
   efl_event_callback_forwarder_add(manager, EFL_UI_FOCUS_MANAGER_EVENT_FLUSH_PRE, obj);
   efl_event_callback_forwarder_add(manager, EFL_UI_FOCUS_MANAGER_EVENT_REDIRECT_CHANGED, obj);
   efl_event_callback_forwarder_add(manager, EFL_UI_FOCUS_MANAGER_EVENT_FOCUSED , obj);
   efl_event_callback_forwarder_add(manager, EFL_UI_FOCUS_MANAGER_EVENT_COORDS_DIRTY, obj);
}

Eina_Bool
_elm_dangerous_call_check(const char *call)
{
   char buf[256];
   const char *eval;

   snprintf(buf, sizeof(buf), "%i.%i.%i.%i", VMAJ, VMIN, VMIC, VREV);
   eval = getenv("ELM_NO_FINGER_WAGGLING");
   if ((eval) && (!strcmp(eval, buf)))
     return 0;
   ERR("ELEMENTARY FINGER WAGGLE!!!!!!!!!!\n"
       "\n"
       "  %s() used.\n"
       "PLEASE see the API documentation for this function. This call\n"
       "should almost never be used. Only in very special cases.\n"
       "\n"
       "To remove this warning please set the environment variable:\n"
       "  ELM_NO_FINGER_WAGGLING\n"
       "To the value of the Elementary version + revision number. e.g.:\n"
       "  1.2.5.40295\n"
       "\n"
       ,
       call);
   return 1;
}

static Eina_Bool _elm_signal_exit(void *data,
                                  int   ev_type,
                                  void *ev);

static Eina_Prefix *pfx = NULL;
char *_elm_appname = NULL;
const char *_elm_data_dir = NULL;
const char *_elm_lib_dir = NULL;
int _elm_log_dom = -1;

EAPI int ELM_EVENT_POLICY_CHANGED = 0;
EAPI int ELM_EVENT_PROCESS_BACKGROUND = 0;
EAPI int ELM_EVENT_PROCESS_FOREGROUND = 0;

static int _elm_init_count = 0;
static int _elm_sub_init_count = 0;
static int _elm_ql_init_count = 0;
static Eina_Bool _elm_prefs_initted = EINA_FALSE;
static int _elm_policies[ELM_POLICY_LAST];
static Ecore_Event_Handler *_elm_exit_handler = NULL;
static Eina_Bool quicklaunch_on = 0;

static Eina_Bool
_elm_signal_exit(void *data  EINA_UNUSED,
                 int ev_type EINA_UNUSED,
                 void *ev    EINA_UNUSED)
{
   elm_exit();
   return ECORE_CALLBACK_PASS_ON;
}

void
_elm_rescale(void)
{
   edje_scale_set(_elm_config->scale);
   _elm_win_rescale(NULL, EINA_FALSE);
   _elm_ews_wm_rescale(NULL, EINA_FALSE);
}

static void *app_mainfunc = NULL;
static const char *app_name = NULL;
static const char *app_desktop_entry = NULL;
static const char *app_domain = NULL;
static const char *app_checkfile = NULL;

static const char *app_compile_bin_dir = NULL;
static const char *app_compile_lib_dir = NULL;
static const char *app_compile_data_dir = NULL;
static const char *app_compile_locale_dir = NULL;
static const char *app_prefix_dir = NULL;
static const char *app_bin_dir = NULL;
static const char *app_lib_dir = NULL;
static const char *app_data_dir = NULL;
static const char *app_locale_dir = NULL;
static double app_base_scale = 1.0;

static Eina_Prefix *app_pfx = NULL;

static Ecore_Event_Handler *system_handlers[2] = { NULL, NULL };

static void
_prefix_check(void)
{
   int argc = 0;
   char **argv = NULL;
   const char *dirs[4] = { NULL, NULL, NULL, NULL };
   char *caps = NULL, *p1, *p2;
   char buf[PATH_MAX];

   if (app_pfx) return;
   if (!app_domain) return;

   ecore_app_args_get(&argc, &argv);
   if (argc < 1) return;

   dirs[0] = app_compile_bin_dir;
   dirs[1] = app_compile_lib_dir;
   dirs[2] = app_compile_data_dir;
   dirs[3] = app_compile_locale_dir;

   if (!dirs[0]) dirs[0] = "/usr/local/bin";
   if (!dirs[1]) dirs[1] = "/usr/local/lib";
   if (!dirs[2])
     {
        snprintf(buf, sizeof(buf), "/usr/local/share/%s", app_domain);
        dirs[2] = buf;
     }
   if (!dirs[3]) dirs[3] = dirs[2];

   if (app_domain)
     {
        caps = alloca(eina_stringshare_strlen(app_domain) + 1);
        for (p1 = (char *)app_domain, p2 = caps; *p1; p1++, p2++)
           *p2 = toupper(*p1);
        *p2 = 0;
     }
   app_pfx = eina_prefix_new(argv[0], app_mainfunc, caps, app_domain,
                             app_checkfile, dirs[0], dirs[1], dirs[2], dirs[3]);

   eina_vpath_interface_app_set(app_domain, app_pfx);
}

static void
_prefix_shutdown(void)
{
   if (app_pfx) eina_prefix_free(app_pfx);
   ELM_SAFE_FREE(app_domain, eina_stringshare_del);
   ELM_SAFE_FREE(app_checkfile, eina_stringshare_del);
   ELM_SAFE_FREE(app_compile_bin_dir, eina_stringshare_del);
   ELM_SAFE_FREE(app_compile_lib_dir, eina_stringshare_del);
   ELM_SAFE_FREE(app_compile_data_dir, eina_stringshare_del);
   ELM_SAFE_FREE(app_compile_locale_dir, eina_stringshare_del);
   app_mainfunc = NULL;
   app_prefix_dir = NULL;
   app_bin_dir = NULL;
   app_lib_dir = NULL;
   app_data_dir = NULL;
   app_locale_dir = NULL;
   app_pfx = NULL;
}

static struct {
     Eina_Module *handle;
     void (*init)(void);
     void (*shutdown)(void);
     Eina_Bool (*app_connect)(const char *appname);
     Eina_Bool is_init;
} _clouseau_old_info;

static struct {
     Eina_Module *handle;
     Eina_Bool (*init)(void);
     Eina_Bool (*shutdown)(void);
     Eina_Bool is_init;
} _clouseau_info;

#define _CLOUSEAU_OLD_LOAD_SYMBOL(cls_struct, sym) \
   do \
     { \
        if ((cls_struct).handle) \
          (cls_struct).sym = eina_module_symbol_get((cls_struct).handle, "clouseau_" #sym); \
        if (!(cls_struct).sym) \
          { \
             WRN("Failed loading symbol '%s' from the clouseau library.", "clouseau_" #sym); \
             if ((cls_struct).handle) eina_module_free((cls_struct).handle); \
             (cls_struct).handle = NULL; \
          } \
     } \
   while (0)

#define _CLOUSEAU_LOAD_SYMBOL(cls_struct, sym) \
   do \
     { \
        if ((cls_struct).handle) \
          (cls_struct).sym = eina_module_symbol_get((cls_struct).handle, "clouseau_debug_" #sym); \
        if (!(cls_struct).sym) \
          { \
             WRN("Failed loading symbol '%s' from the clouseau library.", "clouseau_debug_" #sym); \
             if ((cls_struct).handle) eina_module_free((cls_struct).handle); \
             (cls_struct).handle = NULL; \
             return EINA_FALSE; \
          } \
     } \
   while (0)

static void
_elm_old_clouseau_unload()
{
   if (_clouseau_old_info.is_init)
     {
        if (_clouseau_old_info.shutdown)
          {
             _clouseau_old_info.shutdown();
          }
        if (_clouseau_old_info.handle)
          {
             eina_module_free(_clouseau_old_info.handle);
             _clouseau_old_info.handle = NULL;
          }
        _clouseau_old_info.is_init = EINA_FALSE;
     }
}

static void
_elm_clouseau_unload()
{
   if (_clouseau_info.is_init)
     {
        if (_clouseau_info.shutdown)
          {
             _clouseau_info.shutdown();
          }
        if (_clouseau_info.handle)
          {
             eina_module_free(_clouseau_info.handle);
             _clouseau_info.handle = NULL;
          }
        _clouseau_info.is_init = EINA_FALSE;
     }
}

Eina_Bool
_elm_old_clouseau_reload()
{
   if (!_elm_config->clouseau_enable)
     {
        _elm_old_clouseau_unload();
        return EINA_TRUE;
     }

   if (!_clouseau_old_info.is_init)
     {
        _clouseau_old_info.handle = eina_module_new(
              PACKAGE_LIB_DIR "/libclouseau" LIBEXT);
        if (!_clouseau_old_info.handle || !eina_module_load(_clouseau_old_info.handle))
          {
             WRN("Failed loading the clouseau_old library.");
             if (_clouseau_old_info.handle) eina_module_free(_clouseau_old_info.handle);
             _clouseau_old_info.handle = NULL;
          }

        _CLOUSEAU_OLD_LOAD_SYMBOL(_clouseau_old_info, init);
        _CLOUSEAU_OLD_LOAD_SYMBOL(_clouseau_old_info, shutdown);
        _CLOUSEAU_OLD_LOAD_SYMBOL(_clouseau_old_info, app_connect);

        if (_clouseau_old_info.handle)
          {
             _clouseau_old_info.init();
             if (!_clouseau_old_info.app_connect(elm_app_name_get()))
               {
                  ERR("Failed connecting to the clouseau server.");
               }
             _clouseau_old_info.is_init = EINA_TRUE;
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_elm_clouseau_load()
{
   if (!_clouseau_info.is_init)
     {
        _clouseau_info.handle = eina_module_new(
              PACKAGE_LIB_DIR "/libclouseau_debug" LIBEXT);
        if (!_clouseau_info.handle || !eina_module_load(_clouseau_info.handle))
          {
             WRN("Failed loading the clouseau library.");
             if (_clouseau_info.handle) eina_module_free(_clouseau_info.handle);
             _clouseau_info.handle = NULL;
             return EINA_FALSE;
          }

        _CLOUSEAU_LOAD_SYMBOL(_clouseau_info, init);
        _CLOUSEAU_LOAD_SYMBOL(_clouseau_info, shutdown);

        if (_clouseau_info.handle)
          {
             _clouseau_info.init();
             _clouseau_info.is_init = EINA_TRUE;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_sys_memory_changed(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Ecore_Memory_State state = ecore_memory_state_get();

   if (state != ECORE_MEMORY_STATE_LOW)
     return ECORE_CALLBACK_PASS_ON;

   elm_cache_all_flush();
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_sys_lang_changed(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   char *lang;

   lang = getenv("LANG");
   if (!lang)
     lang = getenv("LC_MESSAGES");
   if (!lang)
     lang = getenv("LC_ALL");

   if (lang)
     elm_language_set(lang);
   else
     ERR("Language not set in environment");

   return ECORE_CALLBACK_PASS_ON;
}

// This is necessary to keep backward compatibility
static const char *bcargv[] = { "exe" };

EAPI int
elm_init(int argc, char **argv)
{
   _elm_init_count++;
   if (_elm_init_count > 1) return _elm_init_count;
   elm_quicklaunch_init(argc, argv ? argv : (char**) bcargv);
   elm_quicklaunch_sub_init(argc, argv ? argv : (char**) bcargv);

   _prefix_shutdown();

   _elm_clouseau_load();

   system_handlers[0] =
     ecore_event_handler_add(ECORE_EVENT_MEMORY_STATE, _sys_memory_changed, NULL);
   system_handlers[1] =
     ecore_event_handler_add(ECORE_EVENT_LOCALE_CHANGED, _sys_lang_changed, NULL);

   if (_elm_config->atspi_mode != ELM_ATSPI_MODE_OFF)
     _elm_atspi_bridge_init();
   if (!_elm_config->web_backend)
     _elm_config->web_backend = eina_stringshare_add("none");
   if (!_elm_web_init(_elm_config->web_backend))
     _elm_config->web_backend = eina_stringshare_add("none");
   _elm_code_parse_setup();

   // For backward compability, EFL startup time and ELM startup time are made
   // identical. It is fine to do it here as we are finishing initialisation
   // and the startup time should have been accounted earlier.
   if (_elm_startup_time >= 0)
     if (_efl_startup_time <= 0)
       _efl_startup_time = _elm_startup_time;
   _elm_startup_time = _efl_startup_time;

   return _elm_init_count;
}

EAPI int
elm_shutdown(void)
{
   if (_elm_init_count <= 0)
     return 0;

   _elm_init_count--;
   if (_elm_init_count > 0) return _elm_init_count;

   ecore_event_handler_del(system_handlers[0]);
   ecore_event_handler_del(system_handlers[1]);

   _elm_win_shutdown();
   _elm_atspi_bridge_shutdown();

   while (_elm_win_deferred_free) ecore_main_loop_iterate();

   _elm_clouseau_unload();
   _elm_old_clouseau_unload();
// wrningz :(
//   _prefix_shutdown();
   ELM_SAFE_FREE(app_name, eina_stringshare_del);
   ELM_SAFE_FREE(app_desktop_entry, eina_stringshare_del);

   elm_quicklaunch_sub_shutdown();
   elm_quicklaunch_shutdown();
   return _elm_init_count;
}

EAPI void
elm_app_info_set(void *mainfunc, const char *dom, const char *checkfile)
{
   app_mainfunc = mainfunc;
   eina_stringshare_replace(&app_domain, dom);
   eina_stringshare_replace(&app_checkfile, checkfile);
}

EAPI void
elm_app_name_set(const char *name)
{
   eina_stringshare_replace(&app_name, name);
}

EAPI void
elm_app_desktop_entry_set(const char *path)
{
   eina_stringshare_replace(&app_desktop_entry, path);
}

EAPI void
elm_app_compile_bin_dir_set(const char *dir)
{
   eina_stringshare_replace(&app_compile_bin_dir, dir);
}

EAPI void
elm_app_compile_lib_dir_set(const char *dir)
{
   eina_stringshare_replace(&app_compile_lib_dir, dir);
}

EAPI void
elm_app_compile_data_dir_set(const char *dir)
{
   eina_stringshare_replace(&app_compile_data_dir, dir);
}

EAPI void
elm_app_compile_locale_set(const char *dir)
{
   eina_stringshare_replace(&app_compile_locale_dir, dir);
}

EAPI const char *
elm_app_name_get(void)
{
   if (app_name) return app_name;

   return "";
}

EAPI const char *
elm_app_desktop_entry_get(void)
{
   if (app_desktop_entry) return app_desktop_entry;

   return "";
}

EAPI const char *
elm_app_prefix_dir_get(void)
{
   if (app_prefix_dir) return app_prefix_dir;
   _prefix_check();
   if (!app_pfx) return "";
   app_prefix_dir = eina_prefix_get(app_pfx);
   return app_prefix_dir;
}

EAPI const char *
elm_app_bin_dir_get(void)
{
   if (app_bin_dir) return app_bin_dir;
   _prefix_check();
   if (!app_pfx) return "";
   app_bin_dir = eina_prefix_bin_get(app_pfx);
   return app_bin_dir;
}

EAPI const char *
elm_app_lib_dir_get(void)
{
   if (app_lib_dir) return app_lib_dir;
   _prefix_check();
   if (!app_pfx) return "";
   app_lib_dir = eina_prefix_lib_get(app_pfx);
   return app_lib_dir;
}

EAPI const char *
elm_app_data_dir_get(void)
{
   if (app_data_dir) return app_data_dir;
   _prefix_check();
   if (!app_pfx) return "";
   app_data_dir = eina_prefix_data_get(app_pfx);
   return app_data_dir;
}

EAPI const char *
elm_app_locale_dir_get(void)
{
   if (app_locale_dir) return app_locale_dir;
   _prefix_check();
   if (!app_pfx) return "";
   app_locale_dir = eina_prefix_locale_get(app_pfx);
   return app_locale_dir;
}

EAPI void
elm_app_base_scale_set(double base_scale)
{
   if (base_scale < 0.0) return;
   if (fabs(base_scale) < DBL_EPSILON) return;
   app_base_scale = base_scale;
}

EAPI double
elm_app_base_scale_get(void)
{
   if (app_base_scale > 0.0) return app_base_scale;
   return 1.0;
}

static Eina_Bool _elm_need_e_dbus = EINA_FALSE;
static void *e_dbus_handle = NULL;

EAPI Eina_Bool
elm_need_e_dbus(void)
{
   int (*init_func)(void) = NULL;

   if (_elm_need_e_dbus) return EINA_TRUE;
   /* We use RTLD_NOLOAD when available, so we are sure to use the 'libeldbus' that was linked to the binary */
#ifndef RTLD_NOLOAD
# define RTLD_NOLOAD RTLD_GLOBAL
#endif
   if (!e_dbus_handle)
     e_dbus_handle = dlopen("libedbus.so", RTLD_LAZY | RTLD_NOLOAD);
   if (!e_dbus_handle)
     e_dbus_handle = dlopen("libedbus.so.1", RTLD_LAZY | RTLD_NOLOAD);
   if (!e_dbus_handle) return EINA_FALSE;
   init_func = dlsym(e_dbus_handle, "e_dbus_init");
   if (!init_func) return EINA_FALSE;
   _elm_need_e_dbus = EINA_TRUE;
   init_func();
   return EINA_TRUE;
}

static void
_elm_unneed_e_dbus(void)
{
   int (*shutdown_func)(void) = NULL;

   if (!_elm_need_e_dbus) return;
   shutdown_func = dlsym(e_dbus_handle, "e_dbus_shutdown");
   if (!shutdown_func) return;
   _elm_need_e_dbus = EINA_FALSE;
   shutdown_func();

   dlclose(e_dbus_handle);
   e_dbus_handle = NULL;
}

static Eina_Bool _elm_need_eldbus = EINA_FALSE;
EAPI Eina_Bool
elm_need_eldbus(void)
{
   if (_elm_need_eldbus) return EINA_TRUE;
   _elm_need_eldbus = EINA_TRUE;
   eldbus_init();
   return EINA_TRUE;
}

static void
_elm_unneed_eldbus(void)
{
   if (!_elm_need_eldbus) return;
   _elm_need_eldbus = EINA_FALSE;
   eldbus_shutdown();
}

#ifdef ELM_ELOCATION
static Eina_Bool _elm_need_elocation = EINA_FALSE;
#endif
EAPI Eina_Bool
elm_need_elocation(void)
{
#ifdef ELM_ELOCATION
   if (_elm_need_elocation) return EINA_TRUE;
   _elm_need_elocation = EINA_TRUE;
   elocation_init();
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

static void
_elm_unneed_elocation(void)
{
#ifdef ELM_ELOCATION
   if (!_elm_need_elocation) return;
   _elm_need_elocation = EINA_FALSE;
   eldbus_shutdown();
#endif
}

static Eina_Bool _elm_need_efreet = EINA_FALSE;

EAPI Eina_Bool
elm_need_efreet(void)
{
   if (_elm_need_efreet) return EINA_TRUE;
   if (!efreet_init()) return EINA_FALSE;
   if (!efreet_mime_init())
     {
        efreet_shutdown();
        return EINA_FALSE;
     }
   if (!efreet_trash_init())
     {
        efreet_mime_shutdown();
        efreet_shutdown();
        return EINA_FALSE;
     }
   _elm_need_efreet = EINA_TRUE;
   /*
     {
        Eina_List **list;

        list = efreet_icon_extra_list_get();
        if (list)
          {
             e_user_dir_concat_static(buf, "icons");
             *list = eina_list_prepend(*list, (void *)eina_stringshare_add(buf));
             e_prefix_data_concat_static(buf, "data/icons");
             *list = eina_list_prepend(*list, (void *)eina_stringshare_add(buf));
          }
     }
    */
   return EINA_TRUE;
}

static void
_elm_unneed_efreet(void)
{
   if (!_elm_need_efreet) return;
   _elm_need_efreet = EINA_FALSE;
   efreet_trash_shutdown();
   efreet_mime_shutdown();
   efreet_shutdown();
}

EAPI void
elm_quicklaunch_mode_set(Eina_Bool ql_on)
{
   quicklaunch_on = ql_on;
}

EAPI Eina_Bool
elm_quicklaunch_mode_get(void)
{
   return quicklaunch_on;
}

EAPI int
elm_quicklaunch_init(int    argc EINA_UNUSED,
                     char **argv)
{
   _elm_ql_init_count++;
   if (_elm_ql_init_count > 1) return _elm_ql_init_count;
   _use_build_config = !!getenv("EFL_RUN_IN_TREE");
   eina_init();
   _elm_log_dom = eina_log_domain_register("elementary", EINA_COLOR_LIGHTBLUE);
   if (!_elm_log_dom)
     {
        EINA_LOG_ERR("could not register elementary log domain.");
        _elm_log_dom = EINA_LOG_DOMAIN_GLOBAL;
     }

   eet_init();
   ecore_init();
   ecore_event_init();
   edje_init();
   eio_init();

#ifdef HAVE_ELEMENTARY_EMAP
   emap_init();
#endif

   memset(_elm_policies, 0, sizeof(_elm_policies));
   ELM_EVENT_POLICY_CHANGED = ecore_event_type_new();
   ELM_EVENT_PROCESS_BACKGROUND = ecore_event_type_new();
   ELM_EVENT_PROCESS_FOREGROUND = ecore_event_type_new();

   if (!ecore_file_init())
     ERR("Elementary cannot init ecore_file");

   _elm_exit_handler =
     ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _elm_signal_exit, NULL);

   if (argv)
     {
        _elm_appname = strdup(ecore_file_file_get(argv[0]));
        elm_app_name_set(_elm_appname);
     }

   pfx = eina_prefix_new(argv ? argv[0] : NULL, elm_quicklaunch_init,
                         "ELM", "elementary", "config/profile.cfg",
                         PACKAGE_LIB_DIR, /* don't have a bin dir currently */
                         PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR,
                         LOCALE_DIR);
   if (pfx)
     {
        if (_use_build_config)
          _elm_data_dir = eina_stringshare_add(PACKAGE_BUILD_DIR "/data/elementary");
        else
          _elm_data_dir = eina_stringshare_add(eina_prefix_data_get(pfx));
        _elm_lib_dir = eina_stringshare_add(eina_prefix_lib_get(pfx));
     }
   if (!_elm_data_dir) _elm_data_dir = eina_stringshare_add("/");
   if (!_elm_lib_dir) _elm_lib_dir = eina_stringshare_add("/");

   eina_log_timing(_elm_log_dom, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);

   if (quicklaunch_on)
     _elm_init_count++;
   return _elm_ql_init_count;
}

EAPI int
elm_quicklaunch_sub_init(int    argc,
                         char **argv)
{
   _elm_sub_init_count++;
   if (_elm_sub_init_count > 1) return _elm_sub_init_count;
   _elm_config_init();

   if (!quicklaunch_on)
     {
        ecore_evas_init(); // FIXME: check errors
        elm_color_class_init();
        _elm_module_init();
        _elm_config_sub_init();
        ecore_imf_init();
        ecore_con_init();
        ecore_con_url_init();
        _elm_prefs_initted = _elm_prefs_init();
        _elm_ews_wm_init();

        ecore_init_ex(argc, argv);
     }
   return _elm_sub_init_count;
}

EAPI int
elm_quicklaunch_sub_shutdown(void)
{
   _elm_sub_init_count--;
   if (_elm_sub_init_count > 0) return _elm_sub_init_count;
   if (!quicklaunch_on)
     {
        ecore_shutdown_ex();

        _elm_win_shutdown();
        _elm_ews_wm_shutdown();
        ecore_con_url_shutdown();
        ecore_con_shutdown();
        ecore_imf_shutdown();
        edje_shutdown();
        ecore_evas_shutdown();
        _elm_config_sub_shutdown();
        _elm_module_shutdown();
        if (_elm_prefs_initted)
          _elm_prefs_shutdown();
        elm_color_class_shutdown();
     }

   ecore_main_loop_iterate();
   _elm_config_shutdown();

   return _elm_sub_init_count;
}

EAPI int
elm_quicklaunch_shutdown(void)
{
   _elm_ql_init_count--;
   if (_elm_ql_init_count > 0) return _elm_ql_init_count;

   eina_log_timing(_elm_log_dom, EINA_LOG_STATE_STOP, EINA_LOG_STATE_SHUTDOWN);

   ecore_event_type_flush(ELM_EVENT_POLICY_CHANGED,
                          ELM_EVENT_PROCESS_BACKGROUND,
                          ELM_EVENT_PROCESS_FOREGROUND);

   if (pfx) eina_prefix_free(pfx);
   pfx = NULL;
   ELM_SAFE_FREE(_elm_data_dir, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_lib_dir, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_appname, free);

   ELM_SAFE_FREE(_elm_exit_handler, ecore_event_handler_del);

   _elm_theme_shutdown();
   _elm_unneed_systray();
   _elm_unneed_sys_notify();
   _elm_unneed_efreet();
   _elm_unneed_e_dbus();
   _elm_unneed_eldbus();
   _elm_unneed_elocation();
   _elm_unneed_ethumb();
   _elm_unneed_web();

#ifdef HAVE_ELEMENTARY_EMAP
   emap_shutdown();
#endif

   ecore_file_shutdown();
   eio_shutdown();
   ecore_event_shutdown();
   ecore_shutdown();
   eet_shutdown();

   if ((_elm_log_dom > -1) && (_elm_log_dom != EINA_LOG_DOMAIN_GLOBAL))
     {
        eina_log_domain_unregister(_elm_log_dom);
        _elm_log_dom = -1;
     }

   eina_shutdown();
   return _elm_ql_init_count;
}

EAPI void
elm_quicklaunch_seed(void)
{
#ifndef SEMI_BROKEN_QUICKLAUNCH
   if (quicklaunch_on)
     {
        Evas_Object *win, *bg, *bt;

        win = elm_win_add(NULL, "seed", ELM_WIN_BASIC);
        bg = elm_bg_add(win);
        elm_win_resize_object_add(win, bg);
        evas_object_show(bg);
        bt = elm_button_add(win);
        elm_object_text_set(bt, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~-_=+\\|]}[{;:'\",<.>/?");
        elm_win_resize_object_add(win, bt);
        ecore_main_loop_iterate();
        evas_object_del(win);
        ecore_main_loop_iterate();
# ifdef HAVE_ELEMENTARY_X
        if (ecore_x_display_get()) ecore_x_sync();
# endif
        ecore_main_loop_iterate();
     }
#endif
}

#ifdef HAVE_FORK
static void *qr_handle = NULL;
#endif
static int (*qr_main)(int    argc,
                      char **argv) = NULL;
static void (*qre_main)(void *data,
                        const Efl_Event *ev) = NULL;
static void (*qre_pause)(void *data,
                        const Efl_Event *ev) = NULL;
static void (*qre_resume)(void *data,
                        const Efl_Event *ev) = NULL;
static void (*qre_terminate)(void *data,
                        const Efl_Event *ev) = NULL;

EFL_CALLBACKS_ARRAY_DEFINE(_qre_main_ex,
                           { EFL_LOOP_EVENT_ARGUMENTS, qre_main },
                           { EFL_APP_EVENT_PAUSE, qre_pause },
                           { EFL_APP_EVENT_RESUME, qre_resume },
                           { EFL_EVENT_DEL, qre_terminate });

EAPI Eina_Bool
elm_quicklaunch_prepare(int    argc,
                        char **argv,
                        const char *cwd)
{
#ifdef HAVE_FORK
   char *exe, *exe2, *p;
   char *exename;

   if (argc <= 0 || argv == NULL) return EINA_FALSE;

   exe = elm_quicklaunch_exe_path_get(argv[0], cwd);
   if (!exe)
     {
        ERR("requested quicklaunch binary '%s' does not exist\n", argv[0]);
        return EINA_FALSE;
     }

   exe2 = malloc(strlen(exe) + 1 + 7 + strlen(LIBEXT));
   strcpy(exe2, exe);
   p = strrchr(exe2, '/');
   if (p) p++;
   else p = exe2;
   exename = alloca(strlen(p) + 1);
   strcpy(exename, p);
   *p = 0;
   strcat(p, "../lib/");
   strcat(p, exename);
   strcat(p, LIBEXT);
   if (access(exe2, R_OK | X_OK) != 0)
     ELM_SAFE_FREE(exe2, free);

   /* Try linking to executable first. Works with PIE files. */
   qr_handle = dlopen(exe, RTLD_NOW | RTLD_GLOBAL);
   if (qr_handle)
     {
        INF("dlopen('%s') = %p", exe, qr_handle);
        qr_main = dlsym(qr_handle, "elm_main");
        if (qr_main)
          {
             INF("dlsym(%p, 'elm_main') = %p", qr_handle, qr_main);
             free(exe2);
             free(exe);
             return EINA_TRUE;
          }
        dlclose(qr_handle);
        qr_handle = NULL;
     }

   if (!exe2)
     {
        WRN("not quicklauncher capable: '%s'", exe);
        free(exe);
        return EINA_FALSE;
     }
   free(exe);

   /* Open companion .so file.
    * Support for legacy quicklaunch apps with separate library.
    */
   qr_handle = dlopen(exe2, RTLD_NOW | RTLD_GLOBAL);
   if (!qr_handle)
     {
        WRN("dlopen('%s') failed: %s", exe2, dlerror());
        free(exe2);
        return EINA_FALSE;
     }
   INF("dlopen('%s') = %p", exe2, qr_handle);
   qr_main = dlsym(qr_handle, "elm_main");
   INF("dlsym(%p, 'elm_main') = %p", qr_handle, qr_main);
   if (!qr_main)
     {
        WRN("not quicklauncher capable: no elm_main in '%s'", exe2);
        dlclose(qr_handle);
        qr_handle = NULL;
        free(exe2);
        return EINA_FALSE;
     }
   free(exe2);
   return EINA_TRUE;
#else
   (void)argc;
   (void)argv;
   (void)cwd;
   return EINA_FALSE;
#endif
}

EAPI Eina_Bool
efl_quicklaunch_prepare(int    argc,
                        char **argv,
                        const char *cwd)
{
#ifdef HAVE_FORK
   char *exe, *exe2;

   if (argc <= 0 || argv == NULL) return EINA_FALSE;

   exe = elm_quicklaunch_exe_path_get(argv[0], cwd);
   if (!exe)
     {
        ERR("requested quicklaunch binary '%s' does not exist\n", argv[0]);
        return EINA_FALSE;
     }

   exe2 = eina_file_path_sanitize(exe);

   ELM_SAFE_FREE(exe, free);

   /* Try linking to executable first. Works with PIE files. */
   qr_handle = dlopen(exe2, RTLD_NOW | RTLD_GLOBAL);
   if (!qr_handle)
     {
        ERR("dlopen('%s') failed: %s", exe2, dlerror());
        free(exe2);
        return EINA_FALSE;
     }

   INF("dlopen('%s') = %p", exe2, qr_handle);
   qre_main = dlsym(qr_handle, "efl_main");
   INF("dlsym(%p, 'efl_main') = %p", qr_handle, qre_main);
   qre_pause = dlsym(qr_handle, "efl_pause");
   qre_resume = dlsym(qr_handle, "efl_resume");
   qre_terminate = dlsym(qr_handle, "efl_terminate");
   if (qre_main)
     {
        free(exe2);
        return EINA_TRUE;
     }

   WRN("not quicklauncher capable: no efl_main in '%s'", exe2);
   dlclose(qr_handle);
   qr_handle = NULL;
   free(exe2);

   return EINA_FALSE;
#else
   (void)argc;
   (void)argv;
   (void)cwd;
   return EINA_FALSE;
#endif
}

EAPI int
elm_quicklaunch_fork(int    argc,
                     char **argv,
                     char  *cwd,
                     void (*postfork_func) (void *data),
                     void  *postfork_data)
{
#ifdef HAVE_FORK
   pid_t child;
   int ret;

   if (!qr_main && !qre_main)
     {
        int i;
        char **args;

        WRN("No main function found.");
        child = fork();
        if (child > 0) return child;
        else if (child < 0)
          {
             perror("could not fork");
             return 0;
          }
        setsid();
        if (chdir(cwd) != 0) perror("could not chdir");
        args = alloca((argc + 1) * sizeof(char *));
        for (i = 0; i < argc; i++) args[i] = argv[i];
        args[argc] = NULL;
        WRN("%s not quicklaunch capable, fallback...", argv[0]);
        execvp(argv[0], args);
        ERR("failed to execute '%s': %s", argv[0], strerror(errno));
        exit(-1);
     }
   INF("Main function found (legacy: %p, efl: %p)",
       qr_main, qre_main);
   child = fork();
   if (child > 0) return child;
   else if (child < 0)
     {
        perror("could not fork");
        return 0;
     }
   ecore_app_args_set(argc, (const char**)argv);
   if (postfork_func) postfork_func(postfork_data);

   eina_main_loop_define();
   ecore_fork_reset();

   if (quicklaunch_on)
     {
        ELM_SAFE_FREE(_elm_appname, free);
        if ((argv) && (argv[0]))
          _elm_appname = strdup(ecore_file_file_get(argv[0]));

#ifdef SEMI_BROKEN_QUICKLAUNCH
        evas_init();
        _elm_module_init();
        _elm_config_sub_init();
        ecore_evas_init(); // FIXME: check errors
        ecore_imf_init();
#endif
     }

   if (setsid() < 0) perror("could not setsid");
   if (chdir(cwd) != 0) perror("could not chdir");
   if (_elm_config->atspi_mode != ELM_ATSPI_MODE_OFF)
     _elm_atspi_bridge_init();

   if (qre_main)
     {
        if (qre_pause && qre_resume && qre_terminate)
          {
             efl_event_callback_array_add(efl_main_loop_get(), _qre_main_ex(), NULL);
          }
        else
          {
             efl_event_callback_add(efl_main_loop_get(), EFL_LOOP_EVENT_ARGUMENTS, qre_main, NULL);
          }

        ecore_init_ex(argc, argv);

        ret = efl_loop_exit_code_process(efl_loop_begin(efl_main_loop_get()));

        ecore_shutdown_ex();

        elm_shutdown();
        exit(ret);
     }
   else
     {
        ecore_init_ex(argc, argv);

        ret = qr_main(argc, argv);

        ecore_shutdown_ex();
        exit(ret);
     }

   return 1;
#else
   return 0;
   (void)argc;
   (void)argv;
   (void)cwd;
   (void)postfork_func;
   (void)postfork_data;
#endif
}

EAPI void
elm_quicklaunch_cleanup(void)
{
#ifdef HAVE_FORK
   if (qr_handle)
     {
        dlclose(qr_handle);
        qr_handle = NULL;
        qr_main = NULL;
     }
#endif
}

EAPI int
elm_quicklaunch_fallback(int    argc,
                         char **argv)
{
   int ret;
   char cwd[PATH_MAX];
   elm_quicklaunch_init(argc, argv);
   elm_quicklaunch_sub_init(argc, argv);
   elm_quicklaunch_prepare(argc, argv, getcwd(cwd, sizeof(cwd)));
   ret = qr_main(argc, argv);
   exit(ret);
   return ret;
}

EAPI char *
elm_quicklaunch_exe_path_get(const char *exe, const char *cwd)
{
   static char *path = NULL;
   static Eina_List *pathlist = NULL;
   const char *pathitr;
   const Eina_List *l;
   char buf[PATH_MAX];
   if (exe[0] == '/') return strdup(exe);
   if (cwd)
     pathlist = eina_list_append(pathlist, eina_stringshare_add(cwd));
   else
     {
        if ((exe[0] == '.') && (exe[1] == '/')) return strdup(exe);
        if ((exe[0] == '.') && (exe[1] == '.') && (exe[2] == '/')) return strdup(exe);
     }
   if (!path)
     {
        const char *p, *pp;
        char *buf2;
        path = getenv("PATH");
        buf2 = alloca(strlen(path) + 1);
        p = path;
        pp = p;
        for (;; )
          {
             if ((*p == ':') || (!*p))
               {
                  int len;

                  len = p - pp;
                  strncpy(buf2, pp, len);
                  buf2[len] = 0;
                  pathlist = eina_list_append(pathlist, eina_stringshare_add(buf2));
                  if (!*p) break;
                  p++;
                  pp = p;
               }
             else
               {
                  if (!*p) break;
                  p++;
               }
          }
     }
   EINA_LIST_FOREACH(pathlist, l, pathitr)
     {
        snprintf(buf, sizeof(buf), "%s/%s", pathitr, exe);
        if (!access(buf, R_OK | X_OK)) return strdup(buf);
     }
   return NULL;
}

EAPI void
elm_run(void)
{
   ecore_main_loop_begin();
}

static void
_on_terminate(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   Eina_List *l, *l_next;
   Evas_Object *win;

   EINA_LIST_FOREACH_SAFE(_elm_win_list, l, l_next, win)
     evas_object_del(win);
}

EAPI void
elm_exit(void)
{
   efl_exit(0);
}

//FIXME: Use Elm_Policy Parameter when 2.0 is released.
EAPI Eina_Bool
elm_policy_set(unsigned int policy,
               int          value)
{
   Elm_Event_Policy_Changed *ev;

   if (policy >= ELM_POLICY_LAST)
     return EINA_FALSE;

   if (value == _elm_policies[policy])
     return EINA_TRUE;

   if (policy == ELM_POLICY_EXIT)
     {
        if (value == ELM_POLICY_EXIT_WINDOWS_DEL)
          {
             efl_event_callback_add(efl_main_loop_get(), EFL_APP_EVENT_TERMINATE,
                                    _on_terminate, NULL);
          }
        else
          {
             efl_event_callback_del(efl_main_loop_get(), EFL_APP_EVENT_TERMINATE,
                                    _on_terminate, NULL);
          }
     }

   /* TODO: validate policy? */

   ev = malloc(sizeof(*ev));
   ev->policy = policy;
   ev->new_value = value;
   ev->old_value = _elm_policies[policy];

   _elm_policies[policy] = value;

   ecore_event_add(ELM_EVENT_POLICY_CHANGED, ev, NULL, NULL);

   return EINA_TRUE;
}

//FIXME: Use Elm_Policy Parameter when 2.0 is released.
EAPI int
elm_policy_get(unsigned int policy)
{
   if (policy >= ELM_POLICY_LAST)
     return 0;
   return _elm_policies[policy];
}

EAPI void
elm_language_set(const char *lang)
{
   setlocale(LC_ALL, lang);
   evas_language_reinit();
   _elm_win_translate();
   edje_language_set(lang);
}

EAPI Eina_Bool
elm_object_mirrored_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return efl_ui_mirrored_get(obj);
}

EAPI void
elm_object_mirrored_set(Evas_Object *obj, Eina_Bool mirrored)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   efl_ui_mirrored_set(obj, mirrored);
}

EAPI Eina_Bool
elm_object_mirrored_automatic_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return efl_ui_mirrored_automatic_get(obj);
}

EAPI void
elm_object_mirrored_automatic_set(Evas_Object *obj, Eina_Bool automatic)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   efl_ui_mirrored_automatic_set(obj, automatic);
}

/**
 * @}
 */

EAPI void
elm_object_scale_set(Evas_Object *obj,
                     double       scale)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   efl_gfx_entity_scale_set(obj, scale);
}

EAPI double
elm_object_scale_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, 0.0);
   return efl_gfx_entity_scale_get(obj);
}

EAPI void
elm_object_part_text_set(Evas_Object *obj, const char *part, const char *label)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_part_text_set(obj, part, label);
}

EAPI const char *
elm_object_part_text_get(const Evas_Object *obj, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_part_text_get(obj, part);
}

EAPI void
elm_object_domain_translatable_part_text_set(Evas_Object *obj, const char *part, const char *domain, const char *text)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   if (elm_widget_is_legacy(obj))
     {
        if (!part)
          part = efl_ui_widget_default_text_part_get(obj);
        else if (efl_isa(obj, EFL_UI_LAYOUT_OBJECT_CLASS))
           _elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE);

        elm_widget_part_translatable_text_set(obj, part, text, domain);
     }
   else
     {
        if (!part)
           efl_ui_translatable_text_set(obj, text, domain);
        else
           efl_ui_translatable_text_set(efl_part(obj, part), text, domain);
     }
}

EAPI const char *
elm_object_translatable_part_text_get(const Evas_Object *obj, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   if (elm_widget_is_legacy(obj))
     {
        if (!part)
          part = efl_ui_widget_default_text_part_get(obj);
        else if (efl_isa(obj, EFL_UI_LAYOUT_OBJECT_CLASS))
           _elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE);

        return elm_widget_part_translatable_text_get(obj, part, NULL);
     }
   else
     {
        if (!part)
          return efl_ui_translatable_text_get(obj, NULL);
        else
          return efl_ui_translatable_text_get(efl_part(obj, part), NULL);
     }
}

EAPI void
elm_object_domain_part_text_translatable_set(Evas_Object *obj, const char *part, const char *domain, Eina_Bool translatable)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_domain_part_text_translatable_set(obj, part, domain, translatable);
}

EINA_DEPRECATED EAPI void
elm_object_domain_translatable_text_part_set(Evas_Object *obj, const char *part, const char *domain, const char *text)
{
   elm_object_domain_translatable_part_text_set(obj, part, domain, text);
}

EINA_DEPRECATED EAPI const char *
elm_object_translatable_text_part_get(const Evas_Object *obj, const char *part)
{
   return elm_object_translatable_part_text_get(obj, part);
}

EAPI void
elm_object_part_content_set(Evas_Object *obj, const char *part, Evas_Object *content)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_content_part_set(obj, part, content);
}

EAPI Evas_Object *
elm_object_part_content_get(const Evas_Object *obj, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_content_part_get(obj, part);
}

EAPI Evas_Object *
elm_object_part_content_unset(Evas_Object *obj, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_content_part_unset(obj, part);
}

EAPI Eina_Bool
elm_object_style_set(Evas_Object *obj,
                     const char  *style)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   if (elm_widget_style_set(obj, style))
     return EINA_TRUE;
   return EINA_FALSE;
}

EAPI Eina_Bool
elm_object_focus_highlight_style_set(Evas_Object *obj,
                                     const char  *style)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_focus_highlight_style_set(obj, style);
}

EAPI const char *
elm_object_focus_highlight_style_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_focus_highlight_style_get(obj);
}

EAPI const char *
elm_object_style_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_style_get(obj);
}

EAPI void
elm_object_disabled_set(Evas_Object *obj,
                        Eina_Bool    disabled)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_disabled_set(obj, disabled);
}

EAPI Eina_Bool
elm_object_disabled_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_disabled_get(obj);
}

EAPI void
elm_cache_all_flush(void)
{
   const Eina_List *l;
   Evas_Object *obj;

   edje_file_cache_flush();
   edje_collection_cache_flush();
   eet_clearcache();
   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     {
        Evas *e = evas_object_evas_get(obj);
        evas_image_cache_flush(e);
        evas_font_cache_flush(e);
// this is up for debate if we should dump as well
//        evas_render_dump(e);
     }
}

EAPI void
elm_object_focus_allow_set(Evas_Object *obj,
                           Eina_Bool    enable)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_can_focus_set(obj, enable);
}

EAPI Eina_Bool
elm_object_focus_allow_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return (elm_widget_can_focus_get(obj)) || (elm_widget_child_can_focus_get(obj));
}


EAPI void
elm_object_tree_focus_allow_set(Evas_Object *obj,
                                Eina_Bool    tree_focusable)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_tree_unfocusable_set(obj, !tree_focusable);
}

EAPI Eina_Bool
elm_object_tree_focus_allow_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return !elm_widget_tree_unfocusable_get(obj);
}

EAPI void
elm_object_focus_move_policy_set(Evas_Object *obj,
                                 Elm_Focus_Move_Policy policy)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   efl_ui_widget_focus_move_policy_set(obj, policy);
}

EAPI Elm_Focus_Move_Policy
elm_object_focus_move_policy_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return efl_ui_widget_focus_move_policy_get(obj);
}

EAPI Eina_Bool
elm_object_focus_move_policy_automatic_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return efl_ui_widget_focus_move_policy_automatic_get(obj);
}

EAPI void
elm_object_focus_move_policy_automatic_set(Evas_Object *obj, Eina_Bool automatic)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   return efl_ui_widget_focus_move_policy_automatic_set(obj, automatic);
}

EAPI void
elm_object_scroll_hold_push(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scroll_hold_push(obj);
}

EAPI void
elm_object_scroll_hold_pop(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scroll_hold_pop(obj);
}

EAPI int
elm_object_scroll_hold_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, 0);
   return elm_widget_scroll_hold_get(obj);
}

EAPI void
elm_object_scroll_freeze_push(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scroll_freeze_push(obj);
}

EAPI void
elm_object_scroll_freeze_pop(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scroll_freeze_pop(obj);
}

EAPI int
elm_object_scroll_freeze_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, 0);
   return elm_widget_scroll_freeze_get(obj);
}

EAPI void
elm_object_scroll_lock_x_set(Evas_Object *obj,
                             Eina_Bool    lock)
{
   Efl_Ui_Scroll_Block block;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   block = elm_widget_scroll_lock_get(obj);
   if (lock) block |= EFL_UI_SCROLL_BLOCK_HORIZONTAL;
   else block &= ~EFL_UI_SCROLL_BLOCK_HORIZONTAL;
   elm_widget_scroll_lock_set(obj, block);
}

EAPI void
elm_object_scroll_lock_y_set(Evas_Object *obj,
                             Eina_Bool    lock)
{
   Efl_Ui_Scroll_Block block;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   block = elm_widget_scroll_lock_get(obj);
   if (lock) block |= EFL_UI_SCROLL_BLOCK_VERTICAL;
   else block &= ~EFL_UI_SCROLL_BLOCK_VERTICAL;
   elm_widget_scroll_lock_set(obj, block);
}

EAPI Eina_Bool
elm_object_scroll_lock_x_get(const Evas_Object *obj)
{
   Efl_Ui_Scroll_Block block;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   block = elm_widget_scroll_lock_get(obj);
   return !!(block & EFL_UI_SCROLL_BLOCK_HORIZONTAL);
}

EAPI Eina_Bool
elm_object_scroll_lock_y_get(const Evas_Object *obj)
{
   Efl_Ui_Scroll_Block block;

   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   block = elm_widget_scroll_lock_get(obj);
   return !!(block & EFL_UI_SCROLL_BLOCK_VERTICAL);
}

EAPI void
elm_object_scroll_item_loop_enabled_set(Evas_Object *obj,
                                        Eina_Bool   enable)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   if (!efl_isa(obj, ELM_INTERFACE_SCROLLABLE_MIXIN)) return;
   elm_interface_scrollable_item_loop_enabled_set(obj, enable);
}

EAPI Eina_Bool
elm_object_scroll_item_loop_enabled_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   if (!efl_isa(obj, ELM_INTERFACE_SCROLLABLE_MIXIN)) return EINA_FALSE;
   return elm_interface_scrollable_item_loop_enabled_get(obj);
}

EAPI Eina_Bool
elm_object_widget_check(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_is(obj);
}

EAPI Evas_Object *
elm_object_parent_widget_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_parent_widget_get(obj);
}

EAPI Evas_Object *
elm_object_top_widget_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_top_get(obj);
}

EAPI const char *
elm_object_widget_type_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_type_get(obj);
}

EAPI void
elm_object_signal_emit(Evas_Object *obj,
                       const char  *emission,
                       const char  *source)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_signal_emit(obj, emission, source);
}

EAPI void
elm_object_signal_callback_add(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func, void *data)
{
    EINA_SAFETY_ON_NULL_RETURN(obj);
    EINA_SAFETY_ON_NULL_RETURN(func);
    elm_widget_signal_callback_add(obj, emission, source, func, data);
}

EAPI void *
elm_object_signal_callback_del(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);
    return elm_widget_signal_callback_del(obj, emission, source, func);
}

EAPI void
elm_object_event_callback_add(Evas_Object *obj, Elm_Event_Cb func, const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(func);
   elm_widget_event_callback_add(obj, func, data);
}

EAPI void *
elm_object_event_callback_del(Evas_Object *obj, Elm_Event_Cb func, const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(func, NULL);
   return elm_widget_event_callback_del(obj, func, data);
}

EAPI void
elm_object_tree_dump(const Evas_Object *top)
{
#ifdef ELM_DEBUG
   elm_widget_tree_dump(top);
#else
   (void)top;
   return;
#endif
}

EAPI void
elm_object_tree_dot_dump(const Evas_Object *top,
                         const char        *file)
{
#ifdef ELM_DEBUG
   FILE *f = fopen(file, "wb");
   elm_widget_tree_dot_dump(top, f);
   fclose(f);
#else
   (void)top;
   (void)file;
   return;
#endif
}

EAPI void
elm_coords_finger_size_adjust(int times_w,
                              Evas_Coord *w,
                              int times_h,
                              Evas_Coord *h)
{
   if ((w) && (*w < (elm_config_finger_size_get() * times_w)))
     *w = elm_config_finger_size_get() * times_w;
   if ((h) && (*h < (elm_config_finger_size_get() * times_h)))
     *h = elm_config_finger_size_get() * times_h;
}

EAPI void
elm_object_access_info_set(Evas_Object *obj, const char *txt)
{
   elm_widget_access_info_set(obj, txt);
}

EAPI const char *
elm_object_access_info_get(Evas_Object *obj)
{
   return elm_widget_access_info_get(obj);
}

EAPI Evas_Object *
elm_object_name_find(const Evas_Object *obj, const char *name, int recurse)
{
   return elm_widget_name_find(obj, name, recurse);
}

EAPI void
elm_object_orientation_mode_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   Efl_Ui_Widget_Orientation_Mode mode =
         disabled ? EFL_UI_WIDGET_ORIENTATION_MODE_DISABLED
                  : EFL_UI_WIDGET_ORIENTATION_MODE_DEFAULT;
   efl_ui_widget_orientation_mode_set(obj, mode);
}

EAPI Eina_Bool
elm_object_orientation_mode_disabled_get(const Evas_Object *obj)
{
   return efl_ui_widget_orientation_mode_get(obj) == EFL_UI_WIDGET_ORIENTATION_MODE_DISABLED;
}

EAPI Elm_Object_Item *
elm_object_focused_item_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return efl_ui_widget_focused_item_get(obj);
}

EAPI void
elm_object_focus_region_show_mode_set(Evas_Object *obj, Elm_Focus_Region_Show_Mode mode)
{
   elm_widget_focus_region_show_mode_set(obj, mode);
}

EAPI Elm_Focus_Region_Show_Mode
elm_object_focus_region_show_mode_get(const Evas_Object *obj)
{
   return elm_widget_focus_region_show_mode_get(obj);
}

static void
_item_noref(void *data EINA_UNUSED, const Efl_Event *ev)
{
   if (!efl_parent_get(ev->object)) return ;
   efl_del(ev->object);
}

EAPI void
elm_object_item_del(Eo *obj)
{
   Elm_Widget_Item_Data *item;

   if (efl_ref_count(obj) == 1)
     {
        // Noref already, die little item !
        efl_del(obj);
        return ;
     }

   item = efl_data_scope_safe_get(obj, ELM_WIDGET_ITEM_CLASS);
   if (!item) return ;
   efl_event_callback_add(obj, EFL_EVENT_NOREF, _item_noref, NULL);
   item->on_deletion = EINA_TRUE;
}
