#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <dlfcn.h> /* dlopen,dlclose,etc */

#ifdef HAVE_CRT_EXTERNS_H
# include <crt_externs.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Emotion.h>

#include <Elementary.h>
#include "elm_priv.h"

#define SEMI_BROKEN_QUICKLAUNCH 1

#ifdef __CYGWIN__
# define LIBEXT ".dll"
#else
# define LIBEXT ".so"
#endif

static Elm_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Elm_Version *elm_version = &_version;

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

static int _elm_init_count = 0;
static int _elm_sub_init_count = 0;
static int _elm_ql_init_count = 0;
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

static Eina_Bool _emotion_inited = EINA_FALSE;

void
_elm_emotion_init(void)
{
   if (_emotion_inited) return ;

   emotion_init();
   _emotion_inited = EINA_TRUE;
}

void
_elm_emotion_shutdown(void)
{
   if (!_emotion_inited) return ;

   emotion_shutdown();
   _emotion_inited = EINA_FALSE;
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
} _clouseau_info;

#define _CLOUSEAU_LOAD_SYMBOL(cls_struct, sym) \
   do \
     { \
        (cls_struct).sym = eina_module_symbol_get((cls_struct).handle, "clouseau_" #sym); \
        if (!(cls_struct).sym) \
          { \
             WRN("Failed loading symbol '%s' from the clouseau library.", "clouseau_" #sym); \
             eina_module_free((cls_struct).handle); \
             (cls_struct).handle = NULL; \
             return EINA_FALSE; \
          } \
     } \
   while (0)

static void
_elm_clouseau_unload()
{
   if (!_clouseau_info.is_init)
      return;

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

Eina_Bool
_elm_clouseau_reload()
{
   if (!_elm_config->clouseau_enable)
     {
        _elm_clouseau_unload();
        return EINA_TRUE;
     }

   if (_clouseau_info.is_init)
      return EINA_TRUE;

   _clouseau_info.handle = eina_module_new(
         PACKAGE_LIB_DIR "/libclouseau" LIBEXT);
   if (!eina_module_load(_clouseau_info.handle))
     {
        WRN("Failed loading the clouseau library.");
        eina_module_free(_clouseau_info.handle);
        _clouseau_info.handle = NULL;
        return EINA_FALSE;
     }

   _CLOUSEAU_LOAD_SYMBOL(_clouseau_info, init);
   _CLOUSEAU_LOAD_SYMBOL(_clouseau_info, shutdown);
   _CLOUSEAU_LOAD_SYMBOL(_clouseau_info, app_connect);

   _clouseau_info.init();
   if (!_clouseau_info.app_connect(elm_app_name_get()))
     {
        ERR("Failed connecting to the clouseau server.");
     }

   _clouseau_info.is_init = EINA_TRUE;

   return EINA_TRUE;
}

Eina_Bool _sys_memory_changed(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Ecore_Memory_State state = ecore_memory_state_get();

   if (state != ECORE_MEMORY_STATE_LOW)
     return ECORE_CALLBACK_PASS_ON;

   elm_cache_all_flush();
   return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool _sys_lang_changed(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
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

EAPI int
elm_init(int    argc,
         char **argv)
{
   _elm_init_count++;
   if (_elm_init_count > 1) return _elm_init_count;
   elm_quicklaunch_init(argc, argv);
   elm_quicklaunch_sub_init(argc, argv);
   _prefix_shutdown();

   system_handlers[0] = ecore_event_handler_add(ECORE_EVENT_MEMORY_STATE, _sys_memory_changed, NULL);
   system_handlers[1] = ecore_event_handler_add(ECORE_EVENT_LOCALE_CHANGED, _sys_lang_changed, NULL);

   _elm_atspi_bridge_init();

   return _elm_init_count;
}

EAPI int
elm_shutdown(void)
{
   if (_elm_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   _elm_init_count--;
   if (_elm_init_count > 0) return _elm_init_count;

   ecore_event_handler_del(system_handlers[0]);
   ecore_event_handler_del(system_handlers[1]);

   _elm_win_shutdown();
   _elm_atspi_bridge_shutdown();

   while (_elm_win_deferred_free) ecore_main_loop_iterate();

   _elm_clouseau_unload();
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
   if (!e_dbus_handle) e_dbus_handle = dlopen("libeldbus.so", RTLD_LAZY | RTLD_NOLOAD);
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
   _elm_need_efreet = EINA_TRUE;
   efreet_init();
   efreet_mime_init();
   efreet_trash_init();
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
elm_quicklaunch_init(int    argc,
                     char **argv)
{
   _elm_ql_init_count++;
   if (_elm_ql_init_count > 1) return _elm_ql_init_count;
   eina_init();
   _elm_log_dom = eina_log_domain_register("elementary", EINA_COLOR_LIGHTBLUE);
   if (!_elm_log_dom)
     {
        EINA_LOG_ERR("could not register elementary log domain.");
        _elm_log_dom = EINA_LOG_DOMAIN_GLOBAL;
     }

   eet_init();
   ecore_init();

#ifdef HAVE_ELEMENTARY_EMAP
   emap_init();
#endif
   ecore_app_args_set(argc, (const char **)argv);

   memset(_elm_policies, 0, sizeof(_elm_policies));
   if (!ELM_EVENT_POLICY_CHANGED)
     ELM_EVENT_POLICY_CHANGED = ecore_event_type_new();

   ecore_file_init();

   _elm_exit_handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _elm_signal_exit, NULL);

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
        _elm_data_dir = eina_stringshare_add(eina_prefix_data_get(pfx));
        _elm_lib_dir = eina_stringshare_add(eina_prefix_lib_get(pfx));
     }
   if (!_elm_data_dir) _elm_data_dir = eina_stringshare_add("/");
   if (!_elm_lib_dir) _elm_lib_dir = eina_stringshare_add("/");

   eina_log_timing(_elm_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

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
   if (quicklaunch_on)
     {
        _elm_config_init();
#ifdef SEMI_BROKEN_QUICKLAUNCH
        return _elm_sub_init_count;
#endif
     }

   if (!quicklaunch_on)
     {
        ecore_app_args_set(argc, (const char **)argv);
        evas_init();
        edje_init();
        _elm_module_init();
        _elm_config_init();
        _elm_config_sub_init();
        ecore_evas_init(); // FIXME: check errors
        ecore_imf_init();
        ecore_con_init();
        ecore_con_url_init();
        _elm_prefs_init();
        _elm_ews_wm_init();
     }
   return _elm_sub_init_count;
}

EAPI int
elm_quicklaunch_sub_shutdown(void)
{
   _elm_sub_init_count--;
   if (_elm_sub_init_count > 0) return _elm_sub_init_count;
   if (quicklaunch_on)
     {
#ifdef SEMI_BROKEN_QUICKLAUNCH
        return _elm_sub_init_count;
#endif
     }
   if (!quicklaunch_on)
     {
        _elm_win_shutdown();
        _elm_module_shutdown();
        _elm_prefs_shutdown();
        _elm_ews_wm_shutdown();
        ecore_con_url_shutdown();
        ecore_con_shutdown();
        ecore_imf_shutdown();
        ecore_evas_shutdown();
        _elm_config_sub_shutdown();
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
            ENGINE_COMPARE(ELM_XRENDER_X11) ||
            ENGINE_COMPARE(ELM_OPENGL_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_SDL) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_SDL) ||
            ENGINE_COMPARE(ELM_OPENGL_SDL) ||
            ENGINE_COMPARE(ELM_OPENGL_COCOA) ||
            ENGINE_COMPARE(ELM_SOFTWARE_WIN32) ||
            ENGINE_COMPARE(ELM_EWS))
#undef ENGINE_COMPARE
          evas_cserve_disconnect();
        edje_shutdown();
        evas_shutdown();
     }
   return _elm_sub_init_count;
}

EAPI int
elm_quicklaunch_shutdown(void)
{
   _elm_ql_init_count--;
   if (_elm_ql_init_count > 0) return _elm_ql_init_count;

   eina_log_timing(_elm_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_SHUTDOWN);

   if (pfx) eina_prefix_free(pfx);
   pfx = NULL;
   ELM_SAFE_FREE(_elm_data_dir, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_lib_dir, eina_stringshare_del);
   ELM_SAFE_FREE(_elm_appname, free);

   _elm_config_shutdown();

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
   ecore_file_shutdown();

#ifdef HAVE_ELEMENTARY_EMAP
   emap_shutdown();
#endif
   _elm_emotion_shutdown();

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
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
            ENGINE_COMPARE(ELM_XRENDER_X11) ||
            ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
          {
# ifdef HAVE_ELEMENTARY_X
             ecore_x_sync();
# endif
          }
        ecore_main_loop_iterate();
     }
#endif
}

#ifdef HAVE_FORK
static void *qr_handle = NULL;
#endif
static int (*qr_main)(int    argc,
                      char **argv) = NULL;

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
        fprintf(stderr, "dlerr: %s\n", dlerror());
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
elm_quicklaunch_fork(int    argc,
                     char **argv,
                     char  *cwd,
                     void (postfork_func) (void *data),
                     void  *postfork_data)
{
#ifdef HAVE_FORK
   pid_t child;
   int ret;

   if (!qr_main)
     {
        int i;
        char **args;

        child = fork();
        if (child > 0) return EINA_TRUE;
        else if (child < 0)
          {
             perror("could not fork");
             return EINA_FALSE;
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
   child = fork();
   if (child > 0) return EINA_TRUE;
   else if (child < 0)
     {
        perror("could not fork");
        return EINA_FALSE;
     }
   if (postfork_func) postfork_func(postfork_data);

   ecore_fork_reset();
   eina_main_loop_define();

   if (quicklaunch_on)
     {
        ELM_SAFE_FREE(_elm_appname, free);
        if ((argv) && (argv[0]))
          _elm_appname = strdup(ecore_file_file_get(argv[0]));

#ifdef SEMI_BROKEN_QUICKLAUNCH
        ecore_app_args_set(argc, (const char **)argv);
        evas_init();
        edje_init();
        _elm_module_init();
        _elm_config_sub_init();
#define ENGINE_COMPARE(name) (!strcmp(_elm_config->engine, name))
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
            ENGINE_COMPARE(ELM_XRENDER_X11) ||
            ENGINE_COMPARE(ELM_OPENGL_X11))
#undef ENGINE_COMPARE
          {
# ifdef HAVE_ELEMENTARY_X
             ecore_x_init(NULL);
# endif
          }
        ecore_evas_init(); // FIXME: check errors
        ecore_imf_init();
#endif
     }

   setsid();
   if (chdir(cwd) != 0) perror("could not chdir");
   ecore_app_args_set(argc, (const char **)argv);
   ret = qr_main(argc, argv);
   exit(ret);
   return EINA_TRUE;
#else
   return EINA_FALSE;
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

EAPI void
elm_exit(void)
{
   ecore_main_loop_quit();

   if (elm_policy_get(ELM_POLICY_EXIT) == ELM_POLICY_EXIT_WINDOWS_DEL)
     {
        Eina_List *l, *l_next;
        Evas_Object *win;

        EINA_LIST_FOREACH_SAFE(_elm_win_list, l, l_next, win)
          evas_object_del(win);
     }
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
   _elm_win_translate();
}

EAPI Eina_Bool
elm_object_mirrored_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_mirrored_get(obj);
}

EAPI void
elm_object_mirrored_set(Evas_Object *obj, Eina_Bool mirrored)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_mirrored_set(obj, mirrored);
}

EAPI Eina_Bool
elm_object_mirrored_automatic_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_mirrored_automatic_get(obj);
}

EAPI void
elm_object_mirrored_automatic_set(Evas_Object *obj, Eina_Bool automatic)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_mirrored_automatic_set(obj, automatic);
}

/**
 * @}
 */

EAPI void
elm_object_scale_set(Evas_Object *obj,
                     double       scale)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_scale_set(obj, scale);
}

EAPI double
elm_object_scale_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, 0.0);
   return elm_widget_scale_get(obj);
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
   elm_widget_domain_translatable_part_text_set(obj, part, domain, text);
}

EAPI const char *
elm_object_translatable_part_text_get(const Evas_Object *obj, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_translatable_part_text_get(obj, part);
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
   return elm_widget_style_set(obj, style);
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
        evas_render_dump(e);
     }
}

EAPI Eina_Bool
elm_object_focus_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_focus_get(obj);
}

EAPI void
elm_object_focus_set(Evas_Object *obj,
                     Eina_Bool    focus)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);

   if (elm_widget_is(obj))
     {
        const char *type;

        if (focus == elm_widget_focus_get(obj)) return;

        // ugly, but, special case for inlined windows
        type = evas_object_type_get(obj);
        if ((type) && (!strcmp(type, "elm_win")))
          {
             Evas_Object *inlined = elm_win_inlined_image_object_get(obj);

             if (inlined)
               {
                  evas_object_focus_set(inlined, focus);
                  return;
               }
          }
        if (focus)
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
        else
          elm_widget_focused_object_clear(obj);
     }
   else
     {
        evas_object_focus_set(obj, focus);
     }
}

EAPI void
elm_object_focus_allow_set(Evas_Object *obj,
                           Eina_Bool    enable)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_can_focus_set(obj, enable);
/*FIXME: According to the elm_object_focus_allow_get(), child_can_focus field
of the parent should be updated. Otherwise, the checking of it's child focus allow states should not be in elm_object_focus_allow_get() */
}

EAPI Eina_Bool
elm_object_focus_allow_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return (elm_widget_can_focus_get(obj)) || (elm_widget_child_can_focus_get(obj));
}

EAPI void
elm_object_focus_custom_chain_set(Evas_Object *obj,
                                  Eina_List   *objs)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_custom_chain_set(obj, objs);
}

EAPI void
elm_object_focus_custom_chain_unset(Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_custom_chain_unset(obj);
}

EAPI const Eina_List *
elm_object_focus_custom_chain_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_focus_custom_chain_get(obj);
}

EAPI void
elm_object_focus_custom_chain_append(Evas_Object *obj,
                                     Evas_Object *child,
                                     Evas_Object *relative_child)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_custom_chain_append(obj, child, relative_child);
}

EAPI void
elm_object_focus_custom_chain_prepend(Evas_Object *obj,
                                      Evas_Object *child,
                                      Evas_Object *relative_child)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_custom_chain_prepend(obj, child, relative_child);
}

EINA_DEPRECATED EAPI void
elm_object_focus_cycle(Evas_Object        *obj,
                       Elm_Focus_Direction dir)
{
   elm_object_focus_next(obj, dir);
}

EAPI void
elm_object_focus_next(Evas_Object        *obj,
                      Elm_Focus_Direction dir)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_cycle(obj, dir);
}

EAPI Evas_Object *
elm_object_focus_next_object_get(const Evas_Object  *obj,
                                 Elm_Focus_Direction dir)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_focus_next_object_get(obj, dir);
}

EAPI void
elm_object_focus_next_object_set(Evas_Object        *obj,
                                 Evas_Object        *next,
                                 Elm_Focus_Direction dir)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_focus_next_object_set(obj, next, dir);
}

EAPI Evas_Object *
elm_object_focused_object_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_focused_object_get(obj);
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
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_drag_lock_x_set(obj, lock);
}

EAPI void
elm_object_scroll_lock_y_set(Evas_Object *obj,
                             Eina_Bool    lock)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_drag_lock_y_set(obj, lock);
}

EAPI Eina_Bool
elm_object_scroll_lock_x_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_drag_lock_x_get(obj);
}

EAPI Eina_Bool
elm_object_scroll_lock_y_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_drag_lock_y_get(obj);
}

EAPI void
elm_object_scroll_item_loop_enabled_set(Evas_Object *obj,
                                        Eina_Bool   enable)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_item_loop_enabled_set(obj, enable);
}

EAPI Eina_Bool
elm_object_scroll_item_loop_enabled_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   return elm_widget_item_loop_enabled_get(obj);
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

EAPI Evas_Object *
elm_object_item_widget_get(const Elm_Object_Item *it)
{
   return elm_widget_item_widget_get(it);
}

EAPI void
elm_object_item_part_content_set(Elm_Object_Item *it,
                                 const char *part,
                                 Evas_Object *content)
{
   _elm_widget_item_part_content_set((Elm_Widget_Item *)it, part, content);
}

EAPI Evas_Object *
elm_object_item_part_content_get(const Elm_Object_Item *it,
                                 const char *part)
{
   return _elm_widget_item_part_content_get((Elm_Widget_Item *)it, part);
}

EAPI Evas_Object *
elm_object_item_part_content_unset(Elm_Object_Item *it, const char *part)
{
   return _elm_widget_item_part_content_unset((Elm_Widget_Item *)it, part);
}

EAPI void
elm_object_item_part_text_set(Elm_Object_Item *it,
                              const char *part,
                              const char *label)
{
   _elm_widget_item_part_text_set((Elm_Widget_Item *)it, part, label);
}

EAPI const char *
elm_object_item_part_text_get(const Elm_Object_Item *it, const char *part)
{
   return _elm_widget_item_part_text_get((Elm_Widget_Item *)it, part);
}

EAPI void
elm_object_item_domain_translatable_part_text_set(Elm_Object_Item *it, const char *part, const char *domain, const char *text)
{
   _elm_widget_item_domain_translatable_part_text_set((Elm_Widget_Item *)it, part, domain, text);
}

EAPI const char *
elm_object_item_translatable_part_text_get(const Elm_Object_Item *it, const char *part)
{
   return _elm_widget_item_translatable_part_text_get((Elm_Widget_Item *)it, part);
}

EAPI void
elm_object_item_domain_part_text_translatable_set(Elm_Object_Item *it, const char *part, const char *domain, Eina_Bool translatable)
{
   _elm_widget_item_domain_part_text_translatable_set((Elm_Widget_Item *)it, part, domain, translatable);
}

EAPI void
elm_object_access_info_set(Evas_Object *obj, const char *txt)
{
   elm_widget_access_info_set(obj, txt);
}

EAPI Evas_Object *
elm_object_name_find(const Evas_Object *obj, const char *name, int recurse)
{
   return elm_widget_name_find(obj, name, recurse);
}

EAPI void
elm_object_orientation_mode_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   elm_widget_orientation_mode_disabled_set(obj, disabled);
}

EAPI Eina_Bool
elm_object_orientation_mode_disabled_get(const Evas_Object *obj)
{
   return elm_widget_orientation_mode_disabled_get(obj);
}

EAPI Elm_Object_Item *
elm_object_focused_item_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_focused_item_get(obj);
}

EAPI void
elm_object_item_access_info_set(Elm_Object_Item *it, const char *txt)
{
   _elm_widget_item_access_info_set((Elm_Widget_Item *)it, txt);
}

EAPI Evas_Object *
elm_object_item_access_register(Elm_Object_Item *item)
{
   Elm_Widget_Item *it;

   it = (Elm_Widget_Item *)item;

   _elm_access_widget_item_register(it);

   if (it) return it->access_obj;
   return NULL;
}

EAPI void
elm_object_item_access_unregister(Elm_Object_Item *item)
{
   _elm_access_widget_item_unregister((Elm_Widget_Item *)item);
}

EAPI Evas_Object *
elm_object_item_access_object_get(const Elm_Object_Item *item)
{
   if (!item) return NULL;

   return ((Elm_Widget_Item *)item)->access_obj;
}

EAPI void
elm_object_item_access_order_set(Elm_Object_Item *item, Eina_List *objs)
{
   _elm_access_widget_item_access_order_set((Elm_Widget_Item *)item, objs);
}

EAPI const Eina_List *
elm_object_item_access_order_get(const Elm_Object_Item *item)
{
   return _elm_access_widget_item_access_order_get((Elm_Widget_Item *)item);
}

EAPI void
elm_object_item_access_order_unset(Elm_Object_Item *item)
{
   _elm_access_widget_item_access_order_unset((Elm_Widget_Item *)item);
}

EAPI void *
elm_object_item_data_get(const Elm_Object_Item *it)
{
   return elm_widget_item_data_get(it);
}

EAPI void
elm_object_item_data_set(Elm_Object_Item *it, void *data)
{
   elm_widget_item_data_set(it, data);
}

EAPI void
elm_object_item_signal_emit(Elm_Object_Item *it, const char *emission, const char *source)
{
   _elm_widget_item_signal_emit((Elm_Widget_Item *)it, emission, source);
}

EAPI void
elm_object_item_signal_callback_add(Elm_Object_Item *it, const char *emission, const char *source, Elm_Object_Item_Signal_Cb func, void *data)
{
   _elm_widget_item_signal_callback_add((Elm_Widget_Item *)it, emission, source, (Elm_Widget_Item_Signal_Cb) func, data);
}

EAPI void *
elm_object_item_signal_callback_del(Elm_Object_Item *it, const char *emission, const char *source, Elm_Object_Item_Signal_Cb func)
{
   return _elm_widget_item_signal_callback_del((Elm_Widget_Item *)it, emission, source, (Elm_Widget_Item_Signal_Cb) func);
}

EAPI void
elm_object_item_style_set(Elm_Object_Item *it, const char *style)
{
   elm_widget_item_style_set(it, style);
}

EAPI const char *
elm_object_item_style_get(Elm_Object_Item *it)
{
   return elm_widget_item_style_get(it);
}

EAPI void elm_object_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled)
{
   _elm_widget_item_disabled_set((Elm_Widget_Item *)it, disabled);
}

EAPI Eina_Bool elm_object_item_disabled_get(const Elm_Object_Item *it)
{
   return _elm_widget_item_disabled_get((Elm_Widget_Item *)it);
}

EAPI void elm_object_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb del_cb)
{
   _elm_widget_item_del_cb_set((Elm_Widget_Item *)it, del_cb);
}

EAPI void elm_object_item_del(Elm_Object_Item *it)
{
   if (!it) return;
   _elm_widget_item_del((Elm_Widget_Item *)it);
}

EAPI void
elm_object_item_tooltip_text_set(Elm_Object_Item *it, const char *text)
{
   elm_widget_item_tooltip_text_set(it, text);
}

EAPI void
elm_object_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb)
{
   elm_widget_item_tooltip_content_cb_set(it, func, data, del_cb);
}

EAPI void
elm_object_item_tooltip_unset(Elm_Object_Item *it)
{
   elm_widget_item_tooltip_unset(it);
}

EAPI Eina_Bool
elm_object_item_tooltip_window_mode_set(Elm_Object_Item *it, Eina_Bool disable)
{
   return elm_widget_item_tooltip_window_mode_set(it, disable);
}

EAPI Eina_Bool
elm_object_item_tooltip_window_mode_get(const Elm_Object_Item *it)
{
   return elm_widget_item_tooltip_window_mode_get(it);
}

EAPI void
elm_object_item_tooltip_style_set(Elm_Object_Item *it, const char *style)
{
   elm_widget_item_tooltip_style_set(it, style);
}

EAPI const char *
elm_object_item_tooltip_style_get(const Elm_Object_Item *it)
{
   return elm_widget_item_tooltip_style_get(it);
}

EAPI void
elm_object_item_cursor_set(Elm_Object_Item *it, const char *cursor)
{
   elm_widget_item_cursor_set(it, cursor);
}

EAPI const char *
elm_object_item_cursor_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_get(it);
}

EAPI void
elm_object_item_cursor_unset(Elm_Object_Item *it)
{
   elm_widget_item_cursor_unset(it);
}

EAPI void
elm_object_item_cursor_style_set(Elm_Object_Item *it, const char *style)
{
   elm_widget_item_cursor_style_set(it, style);
}

EAPI const char *
elm_object_item_cursor_style_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_style_get(it);
}

EAPI void
elm_object_item_cursor_engine_only_set(Elm_Object_Item *it, Eina_Bool engine_only)
{
   elm_widget_item_cursor_engine_only_set(it, engine_only);
}

EAPI Eina_Bool
elm_object_item_cursor_engine_only_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_engine_only_get(it);
}

EAPI Evas_Object *
elm_object_item_track(Elm_Object_Item *it)
{
   return elm_widget_item_track((Elm_Widget_Item *)it);
}

void
elm_object_item_untrack(Elm_Object_Item *it)
{
   elm_widget_item_untrack((Elm_Widget_Item *)it);
}

int
elm_object_item_track_get(const Elm_Object_Item *it)
{
   return elm_widget_item_track_get((Elm_Widget_Item *)it);
}

EAPI void
elm_object_item_focus_set(Elm_Object_Item *item, Eina_Bool focused)
{
   elm_widget_item_focus_set(item, focused);
}

EAPI Eina_Bool
elm_object_item_focus_get(const Elm_Object_Item *item)
{
   return elm_widget_item_focus_get(item);
}
