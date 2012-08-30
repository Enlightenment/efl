#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_FORK
#include <dlfcn.h> /* dlopen,dlclose,etc */
#endif

#ifdef HAVE_CRT_EXTERNS_H
# include <crt_externs.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef HAVE_EMOTION
# include <Emotion.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

#define SEMI_BROKEN_QUICKLAUNCH 1

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
   printf("ELEMENTARY FINGER WAGGLE!!!!!!!!!!\n"
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
_elm_signal_exit(void *data  __UNUSED__,
                 int ev_type __UNUSED__,
                 void *ev    __UNUSED__)
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

#if HAVE_EMOTION
   emotion_init();
   _emotion_inited = EINA_TRUE;
#endif
}

void
_elm_emotion_shutdown(void)
{
   if (!_emotion_inited) return ;

#if HAVE_EMOTION
   emotion_shutdown();
   _emotion_inited = EINA_FALSE;
#endif
}

static void *app_mainfunc = NULL;
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
        caps = alloca(strlen(app_domain) + 1);
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
   if (app_domain) eina_stringshare_del(app_domain);
   if (app_checkfile) eina_stringshare_del(app_checkfile);
   if (app_compile_bin_dir) eina_stringshare_del(app_compile_bin_dir);
   if (app_compile_lib_dir) eina_stringshare_del(app_compile_lib_dir);
   if (app_compile_data_dir) eina_stringshare_del(app_compile_data_dir);
   if (app_compile_locale_dir) eina_stringshare_del(app_compile_locale_dir);
   if (app_prefix_dir) eina_stringshare_del(app_prefix_dir);
   if (app_bin_dir) eina_stringshare_del(app_bin_dir);
   if (app_lib_dir) eina_stringshare_del(app_lib_dir);
   if (app_data_dir) eina_stringshare_del(app_data_dir);
   if (app_locale_dir) eina_stringshare_del(app_locale_dir);
   app_mainfunc = NULL;
   app_domain = NULL;
   app_checkfile = NULL;
   app_compile_bin_dir = NULL;
   app_compile_lib_dir = NULL;
   app_compile_data_dir = NULL;
   app_compile_locale_dir = NULL;
   app_prefix_dir = NULL;
   app_bin_dir = NULL;
   app_lib_dir = NULL;
   app_data_dir = NULL;
   app_locale_dir = NULL;
   app_pfx = NULL;
}

EAPI int
elm_init(int    argc,
         char **argv)
{
   elm_quicklaunch_init(argc, argv);
   if (_elm_init_count > 1) return _elm_init_count;
   elm_quicklaunch_sub_init(argc, argv);
   _prefix_shutdown();
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
   _elm_win_shutdown();
   while (_elm_win_deferred_free) ecore_main_loop_iterate();
// wrningz :(
//   _prefix_shutdown();
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

#ifdef ELM_EDBUS
static int _elm_need_e_dbus = 0;
#endif
EAPI Eina_Bool
elm_need_e_dbus(void)
{
#ifdef ELM_EDBUS
   if (_elm_need_e_dbus++) return EINA_TRUE;
   e_dbus_init();
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif
}

static void
_elm_unneed_e_dbus(void)
{
#ifdef ELM_EDBUS
   if (--_elm_need_e_dbus) return;

   _elm_need_e_dbus = 0;
   e_dbus_shutdown();
#endif
}

#ifdef ELM_EFREET
static int _elm_need_efreet = 0;
#endif
EAPI Eina_Bool
elm_need_efreet(void)
{
#ifdef ELM_EFREET
   if (_elm_need_efreet++) return EINA_TRUE;
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
#else
   return EINA_FALSE;
#endif
}

static void
_elm_unneed_efreet(void)
{
#ifdef ELM_EFREET
   if (--_elm_need_efreet) return;

   _elm_need_efreet = 0;
   efreet_trash_shutdown();
   efreet_mime_shutdown();
   efreet_shutdown();
#endif
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
   _elm_init_count++;
   _elm_ql_init_count++;
   if (_elm_init_count > 1) return _elm_ql_init_count;
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
   
   if (argv) _elm_appname = strdup(ecore_file_file_get(argv[0]));

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
#ifdef HAVE_ELEMENTARY_ECORE_IMF
        ecore_imf_init();
#endif
#ifdef HAVE_ELEMENTARY_ECORE_CON
        ecore_con_init();
        ecore_con_url_init();
#endif
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
        _elm_ews_wm_shutdown();
#ifdef HAVE_ELEMENTARY_ECORE_CON
        ecore_con_url_shutdown();
        ecore_con_shutdown();
#endif
#ifdef HAVE_ELEMENTARY_ECORE_IMF
        ecore_imf_shutdown();
#endif
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
            ENGINE_COMPARE(ELM_SOFTWARE_16_WINCE) ||
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
   if (pfx) eina_prefix_free(pfx);
   pfx = NULL;
   eina_stringshare_del(_elm_data_dir);
   _elm_data_dir = NULL;
   eina_stringshare_del(_elm_lib_dir);
   _elm_lib_dir = NULL;

   free(_elm_appname);
   _elm_appname = NULL;

   _elm_config_shutdown();

   ecore_event_handler_del(_elm_exit_handler);
   _elm_exit_handler = NULL;

   _elm_theme_shutdown();
   _elm_unneed_efreet();
   _elm_unneed_e_dbus();
   _elm_unneed_ethumb();
   _elm_unneed_web();
   ecore_file_shutdown();

#ifdef HAVE_ELEMENTARY_EMAP
   emap_shutdown();
#endif
#ifdef HAVE_EMOTION
   _elm_emotion_shutdown();
#endif

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
elm_quicklaunch_prepare(int argc __UNUSED__,
                        char   **argv)
{
#ifdef HAVE_FORK
   char *exe = elm_quicklaunch_exe_path_get(argv[0]);
   if (!exe)
     {
        ERR("requested quicklaunch binary '%s' does not exist\n", argv[0]);
        return EINA_FALSE;
     }
   else
     {
        char *exe2, *p;
        char *exename;

        exe2 = malloc(strlen(exe) + 1 + 10);
        strcpy(exe2, exe);
        p = strrchr(exe2, '/');
        if (p) p++;
        else p = exe2;
        exename = alloca(strlen(p) + 1);
        strcpy(exename, p);
        *p = 0;
        strcat(p, "../lib/");
        strcat(p, exename);
        strcat(p, ".so");
        if (!access(exe2, R_OK | X_OK))
          {
             free(exe);
             exe = exe2;
          }
        else
          free(exe2);
     }
   qr_handle = dlopen(exe, RTLD_NOW | RTLD_GLOBAL);
   if (!qr_handle)
     {
        fprintf(stderr, "dlerr: %s\n", dlerror());
        WRN("dlopen('%s') failed: %s", exe, dlerror());
        free(exe);
        return EINA_FALSE;
     }
   INF("dlopen('%s') = %p", exe, qr_handle);
   qr_main = dlsym(qr_handle, "elm_main");
   INF("dlsym(%p, 'elm_main') = %p", qr_handle, qr_main);
   if (!qr_main)
     {
        WRN("not quicklauncher capable: no elm_main in '%s'", exe);
        dlclose(qr_handle);
        qr_handle = NULL;
        free(exe);
        return EINA_FALSE;
     }
   free(exe);
   return EINA_TRUE;
#else
   return EINA_FALSE;
   (void)argv;
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

   if (quicklaunch_on)
     {
        if (_elm_appname) free(_elm_appname);
        _elm_appname = NULL;
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
# ifdef HAVE_ELEMENTARY_ECORE_IMF
        ecore_imf_init();
# endif
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
   elm_quicklaunch_init(argc, argv);
   elm_quicklaunch_sub_init(argc, argv);
   elm_quicklaunch_prepare(argc, argv);
   ret = qr_main(argc, argv);
   exit(ret);
   return ret;
}

EAPI char *
elm_quicklaunch_exe_path_get(const char *exe)
{
   static char *path = NULL;
   static Eina_List *pathlist = NULL;
   const char *pathitr;
   const Eina_List *l;
   char buf[PATH_MAX];
   if (exe[0] == '/') return strdup(exe);
   if ((exe[0] == '.') && (exe[1] == '/')) return strdup(exe);
   if ((exe[0] == '.') && (exe[1] == '.') && (exe[2] == '/')) return strdup(exe);
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
   elm_widget_text_part_set(obj, part, label);
}

EAPI const char *
elm_object_part_text_get(const Evas_Object *obj, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_text_part_get(obj, part);
}

EAPI void
elm_object_domain_translatable_text_part_set(Evas_Object *obj, const char *part, const char *domain, const char *text)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);
   elm_widget_domain_translatable_text_part_set(obj, part, domain, text);
}

EAPI const char *
elm_object_translatable_text_part_get(const Evas_Object *obj, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   return elm_widget_translatable_text_part_get(obj, part);
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
   return;
   (void)top;
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
   return;
   (void)top;
   (void)file;
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
elm_object_item_access_info_set(Elm_Object_Item *it, const char *txt)
{
   _elm_widget_item_access_info_set((Elm_Widget_Item *)it, txt);
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
