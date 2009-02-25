#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <dlfcn.h>      /* dlopen,dlclose,etc */

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"


static int _elm_signal_exit(void *data, int ev_type, void *ev);
#ifdef HAVE_ELEMENTARY_X
static int _elm_window_property_change(void *data, int ev_type, void *ev);
#endif
static void _elm_rescale(void);

char *_elm_appname = NULL;
Elm_Config *_elm_config = NULL;
const char *_elm_data_dir = NULL;

static Ecore_Event_Handler *_elm_exit_handler = NULL;
static Ecore_Event_Handler *_elm_event_property_change = NULL;
#ifdef HAVE_ELEMENTARY_X
static Ecore_X_Atom _elm_atom_enlightenment_scale = 0;
#endif

static int
_elm_signal_exit(void *data, int ev_type, void *ev)
{
   elm_exit();
   return 1;
}

#ifdef HAVE_ELEMENTARY_X
static int
_elm_window_property_change(void *data, int ev_type, void *ev)
{
   Ecore_X_Event_Window_Property *event = ev;
   
   if (event->win == ecore_x_window_root_first_get())
     {
        if (event->atom == _elm_atom_enlightenment_scale)
          {
             int val = 1000;
             
             if (ecore_x_window_prop_card32_get(event->win,
                                                event->atom,
                                                &val, 1) > 0)
               {
                  double pscale;
                  
                  pscale = _elm_config->scale;
                  if (val > 0) _elm_config->scale = (double)val / 1000.0;
                  if (pscale != _elm_config->scale) _elm_rescale();
               }
          }
     }
   return 1;
}
#endif

static void
_elm_rescale(void)
{
   edje_scale_set(_elm_config->scale);
   _elm_win_rescale();
}

EAPI void
elm_init(int argc, char **argv)
{
   elm_quicklaunch_init(argc, argv);
   elm_quicklaunch_sub_init(argc, argv);
}

EAPI void
elm_shutdown(void)
{
   elm_quicklaunch_sub_shutdown();
   elm_quicklaunch_shutdown();
}

static const char *elm_engine, *elm_scale, *elm_theme, *elm_prefix, *elm_data_dir;
static const char *elm_font_hinting, *elm_font_path, *elm_image_cache;
static const char *elm_font_cache, *elm_finger_size;

EAPI void
elm_quicklaunch_init(int argc, char **argv)
{
   int i;
   char buf[PATH_MAX];
   
   eet_init();
   ecore_init(); 
   ecore_app_args_set(argc, (const char **)argv);
   ecore_file_init();
   evas_init();
   edje_init();
   ecore_evas_init(); // FIXME: check errors
   
   _elm_exit_handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _elm_signal_exit, NULL);
   
   _elm_appname = strdup(ecore_file_file_get(argv[0]));

   elm_engine = getenv("ELM_ENGINE");
   elm_scale = getenv("ELM_SCALE");
   elm_theme = getenv("ELM_THEME");
   elm_prefix = getenv("ELM_PREFIX");
   elm_data_dir = getenv("ELM_DATA_DIR");
   elm_font_hinting = getenv("ELM_FONT_HINTING");
   elm_font_path = getenv("ELM_FONT_PATH");
   elm_image_cache = getenv("ELM_IMAGE_CACHE");
   elm_font_cache = getenv("ELM_FONT_CACHE");
   elm_finger_size = getenv("ELM_FINGER_SIZE");
   
   if (!_elm_data_dir)
     {
        _elm_data_dir = eina_stringshare_add(elm_data_dir);
     }
   if (!_elm_data_dir)
     {  
        if (elm_prefix)
          {
             snprintf(buf, sizeof(buf), "%s/share/elementary", elm_prefix);
             _elm_data_dir = eina_stringshare_add(buf);
          }
     }
#ifdef HAVE_DLADDR
   if (!_elm_data_dir)
     {
        Dl_info elementary_dl;
        // libelementary.so/../../share/elementary/
        if (dladdr(elm_init, &elementary_dl))
          {
             char *dir, *dir2;
             
             dir = ecore_file_dir_get(elementary_dl.dli_fname);
             if (dir)
               {
                  dir2 = ecore_file_dir_get(dir);
                  if (dir2)
                    {
                       snprintf(buf, sizeof(buf), "%s/share/elementary", dir2);
                       if (ecore_file_is_dir(buf))
                         _elm_data_dir = eina_stringshare_add(buf);
                       free(dir2);
                    }
                  free(dir);
               }
          }
     }
#endif
   if (!_elm_data_dir)
     {
        _elm_data_dir = eina_stringshare_add(PACKAGE_DATA_DIR);
     }
   if (!_elm_data_dir)
     {
        _elm_data_dir = eina_stringshare_add("/");
     }
        
   // FIXME: actually load config
   _elm_config = ELM_NEW(Elm_Config);
   _elm_config->engine = ELM_SOFTWARE_X11;
   _elm_config->thumbscroll_enable = 1;
   _elm_config->thumbscroll_threshhold = 24;
   _elm_config->thumbscroll_momentum_threshhold = 100.0;
   _elm_config->thumbscroll_friction = 1.0;
   _elm_config->scale = 1.0;
   _elm_config->font_hinting = 2;
   _elm_config->font_dirs = NULL;
   _elm_config->image_cache = 4096;
   _elm_config->font_cache = 512;
   _elm_config->finger_size = 40;
   _elm_config->bgpixmap = 0;
   _elm_config->compositing = 1;
   
   if (elm_engine)
     {
        if ((!strcasecmp(elm_engine, "x11")) ||
            (!strcasecmp(elm_engine, "x")) ||
            (!strcasecmp(elm_engine, "software-x11")) ||
            (!strcasecmp(elm_engine, "software_x11")))
          _elm_config->engine = ELM_SOFTWARE_X11;
        else if ((!strcasecmp(elm_engine, "x11-16")) ||
                 (!strcasecmp(elm_engine, "x16")) ||
                 (!strcasecmp(elm_engine, "software-16-x11")) ||
                 (!strcasecmp(elm_engine, "software_16_x11")))
          _elm_config->engine = ELM_SOFTWARE_16_X11;
        else if ((!strcasecmp(elm_engine, "xrender")) ||
                 (!strcasecmp(elm_engine, "xr")) ||
                 (!strcasecmp(elm_engine, "xrender-x11")) ||
                 (!strcasecmp(elm_engine, "xrender_x11")))
          _elm_config->engine = ELM_XRENDER_X11;
        else if ((!strcasecmp(elm_engine, "fb")) ||
                 (!strcasecmp(elm_engine, "software-fb")) ||
                 (!strcasecmp(elm_engine, "software_fb")))
          _elm_config->engine = ELM_SOFTWARE_FB;
        else if ((!strcasecmp(elm_engine, "opengl")) ||
                 (!strcasecmp(elm_engine, "gl")) ||
                 (!strcasecmp(elm_engine, "opengl-x11")) ||
                 (!strcasecmp(elm_engine, "opengl_x11")))
          _elm_config->engine = ELM_OPENGL_X11;
        else if ((!strcasecmp(elm_engine, "ddraw")) ||
                 (!strcasecmp(elm_engine, "software-ddraw")) ||
                 (!strcasecmp(elm_engine, "software_ddraw")))
          _elm_config->engine = ELM_SOFTWARE_WIN32;
        else if ((!strcasecmp(elm_engine, "wince-gdi")) ||
                 (!strcasecmp(elm_engine, "software-16-wince-gdi")) ||
                 (!strcasecmp(elm_engine, "software_16_wince_gdi")))
          _elm_config->engine = ELM_SOFTWARE_16_WINCE;
     }
   
   if (elm_theme)
     _elm_theme_parse(elm_theme);
   else
     _elm_theme_parse("default");
   
   _elm_config->font_hinting = 2;
   if (elm_font_hinting)
     {
        if (!strcasecmp(elm_font_hinting, "none"))
          _elm_config->font_hinting = 0;
        else if (!strcasecmp(elm_font_hinting, "auto"))
          _elm_config->font_hinting = 1;
        else if (!strcasecmp(elm_font_hinting, "bytecode"))
          _elm_config->font_hinting = 2;
     }
   
   if (elm_font_path)
     {
        const char *p, *pp, *s;
        char *buf;
        
        buf = alloca(strlen(elm_font_path) + 1);
        p = elm_font_path;
        pp = p;
        for (;;)
          {
             if ((*p == ':') || (*p == 0))
               {
                  int len;
                  
                  len = p - pp;
                  strncpy(buf, pp, len);
                  buf[len] = 0;
                  _elm_config->font_dirs = eina_list_append(_elm_config->font_dirs, eina_stringshare_add(buf));
                  if (*p == 0) break;
                  p++;
                  pp = p;
               }
             else
               {
                  if (*p == 0) break;
                  p++;
               }
          }
     }
   
   if (elm_image_cache)
     _elm_config->image_cache = atoi(elm_image_cache);

   if (elm_font_cache)
     _elm_config->font_cache = atoi(elm_font_cache);

   /* FIXME: implement quickstart below */
   /* if !quickstart return
    * else
    *  set up fast-start-fifo (in $ELM_FAST_START_FIFO)
    *  sit on blocking read
    *  read 2 bytes == length of data in bytes including nulls
    *  read N bytes (must be < (page_size - 2))
    *  format: exename\0exepath\0[arg1\0][arg2\0][...]
    *  dlopen exepath
    *  dlsym elm_main in exe
    *  if (elm_main ! exists)
    *   ecore_exe exename
    *  else
    *   fork()
    *   [child]
    *    call exit(elm_main());
    *   [parent]
    *    close x fd the nasty way (with close())
    *    ecore_x_shutdown()
    *    ecore_x_init() etc. etc. loop back to blocking on fifo read
    */
}

EAPI void
elm_quicklaunch_sub_init(int argc, char **argv)
{
   ecore_app_args_set(argc, (const char **)argv);
   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11))
     {
#ifdef HAVE_ELEMENTARY_X
	int val = 1000;
	
	if (!ecore_x_init(NULL))
          {
             EINA_ERROR_PERR("elementary: ERROR. Cannot connect to X11 display. check $DISPLAY variable\n");
             exit(1);
          }
	if (!ecore_x_screen_is_composited(0))
	  _elm_config->compositing = 0;
        _elm_atom_enlightenment_scale = ecore_x_atom_get("ENLIGHTENMENT_SCALE");
        ecore_x_event_mask_set(ecore_x_window_root_first_get(),
                               ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
        _elm_event_property_change = ecore_event_handler_add
          (ECORE_X_EVENT_WINDOW_PROPERTY, _elm_window_property_change, NULL);
        /* FIXME if quickstart this happens in child */
	if (ecore_x_window_prop_card32_get(ecore_x_window_root_first_get(),
					   _elm_atom_enlightenment_scale,
					   &val, 1) > 0)
	  {
	     if (val > 0) _elm_config->scale = (double)val / 1000.0;
	  }
#endif        
      }

   if (elm_scale)
     _elm_config->scale = atof(elm_scale);
   
   _elm_config->finger_size = 
     (double)_elm_config->finger_size * _elm_config->scale;
   if (elm_finger_size)
     _elm_config->finger_size = atoi(elm_finger_size);
}

EAPI void
elm_quicklaunch_sub_shutdown(void)
{
   _elm_win_shutdown();
   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_WIN32) ||
       (_elm_config->engine == ELM_SOFTWARE_16_WINCE))
     {
#ifdef HAVE_ELEMENTARY_X
        ecore_event_handler_del(_elm_event_property_change);
        _elm_event_property_change = NULL;
        ecore_x_disconnect();
#endif        
     }
}

EAPI void
elm_quicklaunch_shutdown(void)
{
   eina_stringshare_del(_elm_data_dir);
   _elm_data_dir = NULL;

   while (_elm_config->font_dirs)
     {
        eina_stringshare_del(_elm_config->font_dirs->data);
        _elm_config->font_dirs = eina_list_remove_list(_elm_config->font_dirs, _elm_config->font_dirs);
     }
   
   ecore_event_handler_del(_elm_exit_handler);
   _elm_exit_handler = NULL;
   
   free(_elm_config);
   free(_elm_appname);
   ecore_evas_shutdown();
   edje_shutdown();
   evas_shutdown();
   ecore_file_shutdown();
   ecore_shutdown();
   eet_shutdown();
}

EAPI void
elm_quicklaunch_seed(void)
{
   Evas_Object *win, *bg, *bt;
   
   win = elm_win_add(NULL, "seed", ELM_WIN_BASIC);
   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);
   bt = elm_button_add(win);
   elm_button_label_set(bt, " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789~-_=+\\|]}[{;:'\",<.>/?");
   elm_win_resize_object_add(win, bt);
   ecore_main_loop_iterate();
   evas_object_del(win);
   ecore_main_loop_iterate();
   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11))
     {
#ifdef HAVE_ELEMENTARY_X
        ecore_x_sync();
#endif        
      }
   ecore_main_loop_iterate();
}

static void *qr_handle = NULL;
static int (*qr_main) (int argc, char **argv) = NULL;

EAPI Evas_Bool
elm_quicklaunch_prepare(int argc, char **argv)
{
   char *exe = elm_quicklaunch_exe_path_get(argv[0]);
   if (!exe)
     {
        printf("ERROR: %s does not exist\n", argv[0]);
        return;
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
        if (access(exe2, R_OK | X_OK) == 0)
          {
             free(exe);
             exe = exe2;
          }
        else
          free(exe2);
     }
   qr_handle = dlopen(exe, RTLD_NOW | RTLD_GLOBAL);
   free(exe);
   if (!qr_handle) return 0;
   qr_main = dlsym(qr_handle, "elm_main");
   if (!qr_main)
     {
        dlclose(qr_handle);
        qr_handle = NULL;
        return 0;
     }
   return 1;
}

EAPI Evas_Bool
elm_quicklaunch_fork(int argc, char **argv, char *cwd)
{
   pid_t child;
   int ret;
   int real_argc;
   char **real_argv;
   
   // FIXME:
   // need to accept current environment from elementary_run
   if (!qr_main)
     {
        int i;
        char **args;
        
        child = fork();
        if (child) return 1;
        chdir(cwd);
        args = alloca((argc + 1) * sizeof(char *));
        for (i = 0; i < argc; i++) args[i] = argv[i];
        args[argc] = NULL;
        printf("WARNING: %s not quicklaunch capable\n", argv[0]);
        exit(execvp(argv[0], args));
     }
   child = fork();
   if (child) return 1;
   chdir(cwd);
   ret = qr_main(argc, argv);
   exit(ret);
}

EAPI void
elm_quicklaunch_cleanup(void)
{
   if (qr_handle)
     {
        dlclose(qr_handle);
        qr_handle = NULL;
        qr_main = NULL;
     }
}

EAPI int
elm_quicklaunch_fallback(int argc, char **argv)
{
   int ret;
   elm_quicklaunch_init(argc, argv);
   elm_quicklaunch_sub_init(argc, argv);
   elm_quicklaunch_prepare(argc, argv);
   ret = qr_main(argc, argv);
   exit(ret);
}

EAPI char *
elm_quicklaunch_exe_path_get(const char *exe)
{
   static char *path = NULL;
   static Eina_List *pathlist = NULL;
   Eina_List *l;
   char buf[PATH_MAX];
   if (exe[0] == '/') return strdup(exe);
   if ((exe[0] == '.') && (exe[1] == '/')) return strdup(exe);
   if ((exe[0] == '.') && (exe[1] == '.') && (exe[2] == '/')) return strdup(exe);
   if (!path)
     {
        const char *p, *pp, *s;
        char *buf;
        path = getenv("PATH");
        buf = alloca(strlen(path) + 1);
        p = path;
        pp = p;
        for (;;)
          {
             if ((*p == ':') || (*p == 0))
               {
                  int len;
                  
                  len = p - pp;
                  strncpy(buf, pp, len);
                  buf[len] = 0;
                  pathlist = eina_list_append(pathlist, eina_stringshare_add(buf));
                  if (*p == 0) break;
                  p++;
                  pp = p;
               }
             else
               {
                  if (*p == 0) break;
                  p++;
               }
          }
     }
   for (l = pathlist; l; l = l->next)
     {
        snprintf(buf, sizeof(buf), "%s/%s", l->data, exe);
        if (access(buf, R_OK | X_OK) == 0) return strdup(buf);
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

EAPI void
elm_object_scale_set(Evas_Object *obj, double scale)
{
   return elm_widget_scale_set(obj, scale);
}

EAPI double
elm_object_scale_get(Evas_Object *obj)
{
   return elm_widget_scale_get(obj);
}

EAPI void
elm_object_focus(Evas_Object *obj)
{
   elm_widget_focus_set(obj, 1);
}

EAPI void
elm_coords_finger_size_adjust(int times_w, Evas_Coord *w, int times_h, Evas_Coord *h)
{
   if ((w) && (*w < (_elm_config->finger_size * times_w)))
     *w = _elm_config->finger_size * times_w;
   if ((h) && (*h < (_elm_config->finger_size * times_h)))
     *h = _elm_config->finger_size * times_h;
}
