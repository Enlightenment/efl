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
   int i;
   const char *elm_engine, *elm_scale, *elm_theme, *elm_prefix, *elm_data_dir;
   const char *elm_font_hinting, *elm_font_path, *elm_image_cache;
   const char *elm_font_cache, *elm_finger_size;
   char buf[PATH_MAX];
   
   eet_init();
   ecore_init();
   ecore_app_args_set(argc, (const char **)argv);
   ecore_file_init();
   evas_init();
   edje_init();
   ecore_evas_init(); // FIXME: check errors
   
   ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _elm_signal_exit, NULL);
   
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

   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_WIN32) ||
       (_elm_config->engine == ELM_SOFTWARE_16_WINCE))
     {
#ifdef HAVE_ELEMENTARY_X
	int val = 1000;
	
	ecore_x_init(NULL);
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
                  _elm_config->font_dirs = eina_list_append(_elm_config->font_dirs, buf);
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

   _elm_config->finger_size = 
     (double)_elm_config->finger_size * _elm_config->scale;
   if (elm_finger_size)
     _elm_config->finger_size = 
     (double)(atoi(elm_finger_size)) * _elm_config->scale;
   
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
elm_shutdown(void)
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
        ecore_x_shutdown();
#endif        
     }

   eina_stringshare_del(_elm_data_dir);
   _elm_data_dir = NULL;

   while (_elm_config->font_dirs)
     {
        eina_stringshare_del(_elm_config->font_dirs->data);
        _elm_config->font_dirs = eina_list_remove_list(_elm_config->font_dirs, _elm_config->font_dirs);
     }
   
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
