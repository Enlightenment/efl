#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

Elm_Config *_elm_config = NULL;
char *_elm_profile = NULL;
static Eet_Data_Descriptor *_config_edd = NULL;
static Ecore_Event_Handler *_prop_change_handler = NULL;
#ifdef HAVE_ELEMENTARY_X
static Ecore_X_Atom _atom_e_scale = 0;
static Ecore_X_Atom _atom_e_finger_size = 0;
static Ecore_X_Atom _atom_e_theme = 0;
#endif

#ifdef HAVE_ELEMENTARY_X
static int
_prop_change(void *data __UNUSED__, int ev_type __UNUSED__, void *ev)
{
   Ecore_X_Event_Window_Property *event = ev;

   if (event->win == ecore_x_window_root_first_get())
     {
	if (event->atom == _atom_e_scale)
	  {
	     unsigned int val = 1000;

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
	else if (event->atom == _atom_e_finger_size)
	  {
	     unsigned int val = 1000;

	     if (ecore_x_window_prop_card32_get(event->win,
						event->atom,
						&val, 1) > 0)
	       {
		  int pfinger_size;

		  pfinger_size = _elm_config->finger_size;
		  _elm_config->finger_size = val;
		  if (pfinger_size != _elm_config->finger_size) _elm_rescale();
	       }
	  }
	else if (event->atom == _atom_e_theme)
	  {
             char *val = NULL;
             
             val = ecore_x_window_prop_string_get(event->win,
                                                  event->atom);
	     eina_stringshare_replace(&_elm_config->theme, val);
	     if (val)
	       {
		  _elm_theme_parse(val);
		  free(val);
		  _elm_rescale();
	       }
	  }
     }
   return 1;
}
#endif

static void
_desc_init(void)
{
   Eet_Data_Descriptor_Class eddc;
   
   EET_EINA_FILE_DATA_DESCRIPTOR_CLASS_SET(&eddc, Elm_Config);
   eddc.func.str_direct_alloc = NULL;
   eddc.func.str_direct_free = NULL;
   
   _config_edd = eet_data_descriptor_file_new(&eddc);
   if (!_config_edd)
     {
        printf("EEEK! eet_data_descriptor_file_new() failed\n");
        return;
     }
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "engine", engine, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "thumbscroll_enable", thumbscroll_enable, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "thumbscroll_threshhold", thumbscroll_threshhold, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "thumbscroll_momentum_threshhold", thumbscroll_momentum_threshhold, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "thumbscroll_friction", thumbscroll_friction, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "thumbscroll_bounce_friction", thumbscroll_bounce_friction, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "page_scroll_friction", page_scroll_friction, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "bring_in_scroll_friction", bring_in_scroll_friction, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "zoom_friction", zoom_friction, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "thumbscroll_bounce_enable", thumbscroll_bounce_enable, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "scale", scale, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "bgpixmap", bgpixmap, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "compositing", compositing, EET_T_INT);
   // EET_DATA_DESCRIPTOR_ADD_LIST(_config_edd, Elm_Config, "font_dirs", font_dirs, sub_edd);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "font_hinting", font_hinting, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "image_cache", image_cache, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "font_cache", font_cache, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "finger_size", finger_size, EET_T_INT);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "fps", fps, EET_T_DOUBLE);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "theme", theme, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(_config_edd, Elm_Config, "modules", modules, EET_T_STRING);
}

static void
_desc_shutdown(void)
{
   if (!_config_edd) return;
   eet_data_descriptor_free(_config_edd);
   _config_edd = NULL;
}

static void
_profile_get(void)
{
   Eet_File *ef = NULL;
   const char *home = NULL;
   char buf[PATH_MAX], *p, *s;
   int len = 0;

   _elm_profile = strdup("default");
   
   // if env var - use profile without question
   s = getenv("ELM_PROFILE");
   if (s)
     {
        free(_elm_profile);
        _elm_profile = strdup(s);
        return;
     }
   
   home = getenv("HOME");
   if (!home) home = "/";
   
   // usser profile
   snprintf(buf, sizeof(buf), "%s/.elementary/config/profile.cfg", home);
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        p = eet_read(ef, "config", &len);
        if (p)
          {
             free(_elm_profile);
             _elm_profile = malloc(len + 1);
             memcpy(_elm_profile, p, len);
             _elm_profile[len] = 0;
             free(p);
          }
        eet_close(ef);
        if (!p) ef = NULL;
     }
   if (ef) return;
   
   // system profile
   snprintf(buf, sizeof(buf), "%s/config/profile.cfg", _elm_data_dir);
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        p = eet_read(ef, "config", &len);
        if (p)
          {
             free(_elm_profile);
             _elm_profile = malloc(len + 1);
             memcpy(_elm_profile, p, len);
             _elm_profile[len] = 0;
             free(p);
          }
        eet_close(ef);
     }
}

void
_config_load(void)
{
   Eet_File *ef = NULL;
   char buf[PATH_MAX];
   const char *home = NULL;
   
   home = getenv("HOME");
   if (!home) home = "/";
   
   // user config
   snprintf(buf, sizeof(buf), "%s/.elementary/config/%s/base.cfg", home, _elm_profile);
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        _elm_config = eet_data_read(ef, _config_edd, "config");
        eet_close(ef);
     }
   if (_elm_config) return;
   
   // system config
   snprintf(buf, sizeof(buf), "%s/config/%s/base.cfg", _elm_data_dir, _elm_profile);
   ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        _elm_config = eet_data_read(ef, _config_edd, "config");
        eet_close(ef);
     }
   if (_elm_config) return;

   // config load fail - defaults
   _elm_config = ELM_NEW(Elm_Config);
   _elm_config->engine = ELM_SOFTWARE_X11;
   _elm_config->thumbscroll_enable = 1;
   _elm_config->thumbscroll_threshhold = 24;
   _elm_config->thumbscroll_momentum_threshhold = 100.0;
   _elm_config->thumbscroll_friction = 1.0;
   _elm_config->thumbscroll_bounce_friction = 0.5;
   _elm_config->page_scroll_friction = 0.5;
   _elm_config->bring_in_scroll_friction = 0.5;
   _elm_config->zoom_friction = 0.5;
   _elm_config->thumbscroll_bounce_enable = 1;
   _elm_config->scale = 1.0;
   _elm_config->bgpixmap = 0;
   _elm_config->font_hinting = 2;
   _elm_config->font_dirs = NULL;
   _elm_config->image_cache = 4096;
   _elm_config->font_cache = 512;
   _elm_config->finger_size = 40;
   _elm_config->compositing = 1;
   _elm_config->fps = 60.0;
   _elm_config->theme = eina_stringshare_add("default");
   _elm_config->modules = NULL;
}

static void
_misc_env_get(void)
{
   char buf[PATH_MAX], *p, *s;
   
   s = getenv("ELM_ENGINE");
   if (s)
     {
        if ((!strcasecmp(s, "x11")) ||
            (!strcasecmp(s, "x")) ||
            (!strcasecmp(s, "software-x11")) ||
            (!strcasecmp(s, "software_x11")))
          _elm_config->engine = ELM_SOFTWARE_X11;
        else if ((!strcasecmp(s, "x11-16")) ||
                 (!strcasecmp(s, "x16")) ||
                 (!strcasecmp(s, "software-16-x11")) ||
                 (!strcasecmp(s, "software_16_x11")))
          _elm_config->engine = ELM_SOFTWARE_16_X11;
        else if ((!strcasecmp(s, "xrender")) ||
                 (!strcasecmp(s, "xr")) ||
                 (!strcasecmp(s, "xrender-x11")) ||
                 (!strcasecmp(s, "xrender_x11")))
          _elm_config->engine = ELM_XRENDER_X11;
        else if ((!strcasecmp(s, "fb")) ||
                 (!strcasecmp(s, "software-fb")) ||
                 (!strcasecmp(s, "software_fb")))
          _elm_config->engine = ELM_SOFTWARE_FB;
        else if ((!strcasecmp(s, "directfb")) ||
                 (!strcasecmp(s, "dfb")))
          _elm_config->engine = ELM_SOFTWARE_DIRECTFB;
        else if ((!strcasecmp(s, "sdl")) ||
                 (!strcasecmp(s, "software-sdl")) ||
                 (!strcasecmp(s, "software_sdl")))
          _elm_config->engine = ELM_SOFTWARE_SDL;
        else if ((!strcasecmp(s, "sdl-16")) ||
                 (!strcasecmp(s, "software-16-sdl")) ||
                 (!strcasecmp(s, "software_16_sdl")))
          _elm_config->engine = ELM_SOFTWARE_16_SDL;
        else if ((!strcasecmp(s, "opengl")) ||
                 (!strcasecmp(s, "gl")) ||
                 (!strcasecmp(s, "opengl-x11")) ||
                 (!strcasecmp(s, "opengl_x11")))
          _elm_config->engine = ELM_OPENGL_X11;
        else if ((!strcasecmp(s, "opengl-sdl")) ||
                 (!strcasecmp(s, "opengl_sdl")) ||
                 (!strcasecmp(s, "gl-sdl")) ||
                 (!strcasecmp(s, "gl_sdl")))
          _elm_config->engine = ELM_OPENGL_SDL;
        else if ((!strcasecmp(s, "gdi")) ||
                 (!strcasecmp(s, "software-gdi")) ||
                 (!strcasecmp(s, "software_gdi")))
          _elm_config->engine = ELM_SOFTWARE_WIN32;
        else if ((!strcasecmp(s, "wince-gdi")) ||
                 (!strcasecmp(s, "software-16-wince-gdi")) ||
                 (!strcasecmp(s, "software_16_wince_gdi")))
          _elm_config->engine = ELM_SOFTWARE_16_WINCE;
     }

   s = getenv("ELM_THUMBSCROLL_ENABLE");
   if (s) _elm_config->thumbscroll_enable = atoi(s);
   s = getenv("ELM_THUMBSCROLL_THRESHOLD");
   if (s) _elm_config->thumbscroll_threshhold = atoi(s);
   // FIXME: floatformat locale issues here 1.0 vs 1,0 - should just be 1.0
   s = getenv("ELM_THUMBSCROLL_MOMENTUM_THRESHOLD");
   if (s) _elm_config->thumbscroll_momentum_threshhold = atof(s);
   s = getenv("ELM_THUMBSCROLL_FRICTION");
   if (s) _elm_config->thumbscroll_friction = atof(s);
   s = getenv("ELM_PAGE_SCROLL_FRICTION");
   if (s) _elm_config->page_scroll_friction = atof(s);
   s = getenv("ELM_BRING_IN_SCROLL_FRICTION");
   if (s) _elm_config->bring_in_scroll_friction = atof(s);
   s = getenv("ELM_ZOOM_FRICTION");
   if (s) _elm_config->zoom_friction = atof(s);

   s = getenv("ELM_THEME");
   if (s) eina_stringshare_replace(&_elm_config->theme, s);
   printf("theme: %s\n", _elm_config->theme);
   _elm_theme_parse(_elm_config->theme);

   s = getenv("ELM_FONT_HINTING");
   if (s)
     {
	if      (!strcasecmp(s, "none"))     _elm_config->font_hinting = 0;
	else if (!strcasecmp(s, "auto"))     _elm_config->font_hinting = 1;
	else if (!strcasecmp(s, "bytecode")) _elm_config->font_hinting = 2;
     }

   s = getenv("ELM_FONT_PATH");
   if (s)
     {
	const char *p, *pp;
	char *buf2;

        EINA_LIST_FREE(_elm_config->font_dirs, p)
          {
             eina_stringshare_del(p);
          }
        
	buf2 = alloca(strlen(s) + 1);
	p = s;
	pp = p;
	for (;;)
	  {
	     if ((*p == ':') || (*p == 0))
	       {
		  int len;

		  len = p - pp;
		  strncpy(buf2, pp, len);
		  buf2[len] = 0;
		  _elm_config->font_dirs = 
                    eina_list_append(_elm_config->font_dirs, 
                                     eina_stringshare_add(buf2));
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

   s = getenv("ELM_IMAGE_CACHE");
   if (s) _elm_config->image_cache = atoi(s);

   s = getenv("ELM_FONT_CACHE");
   if (s) _elm_config->font_cache = atoi(s);

   s = getenv("ELM_SCALE");
   if (s) _elm_config->scale = atof(s);
   
   _elm_config->finger_size = (double)_elm_config->finger_size * _elm_config->scale;
   s = getenv("ELM_FINGER_SIZE");
   if (s) _elm_config->finger_size = atoi(s);

   s = getenv("ELM_FPS");
   if (s) _elm_config->fps = atof(s);
   if (_elm_config->fps < 1.0) _elm_config->fps = 1.0;

   s = getenv("ELM_MODULES");
   if (s) eina_stringshare_replace(&_elm_config->modules, s);
   if (_elm_config->modules) _elm_module_parse(_elm_config->modules);
}

void
_elm_config_init(void)
{
   _desc_init();
   _profile_get();
   _config_load();
   _misc_env_get();

   ecore_animator_frametime_set(1.0 / _elm_config->fps);
   edje_frametime_set(1.0 / _elm_config->fps);
   edje_scale_set(_elm_config->scale);
}

void
_elm_config_sub_init(void)
{
   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11))
     {
#ifdef HAVE_ELEMENTARY_X
	unsigned int val = 1000;

	if (!ecore_x_init(NULL))
	  {
	     ERR("Cannot connect to X11 display. check $DISPLAY variable");
	     exit(1);
	  }
	if (!ecore_x_screen_is_composited(0))
	  _elm_config->compositing = 0;
        
	_atom_e_scale       = ecore_x_atom_get("ENLIGHTENMENT_SCALE");
	_atom_e_finger_size = ecore_x_atom_get("ENLIGHTENMENT_FINGER_SIZE");
	_atom_e_theme       = ecore_x_atom_get("ENLIGHTENMENT_THEME");
        
	ecore_x_event_mask_set(ecore_x_window_root_first_get(),
			       ECORE_X_EVENT_MASK_WINDOW_PROPERTY);
	_prop_change_handler = ecore_event_handler_add
	  (ECORE_X_EVENT_WINDOW_PROPERTY, _prop_change, NULL);
	if (!getenv("ELM_SCALE"))
	  {
	     if (ecore_x_window_prop_card32_get(ecore_x_window_root_first_get(),
						_atom_e_scale,
						&val, 1) > 0)
	       {
		  if (val > 0)
		    {
		       _elm_config->scale = (double)val / 1000.0;
		       // FIXME: hack until e export finger size too
		       if (getenv("ELM_FINGER_SIZE"))
			 _elm_config->finger_size = 40.0 * _elm_config->scale;
                       edje_scale_set(_elm_config->scale);
		    }
	       }
	  }
	if (!getenv("ELM_FINGER_SIZE"))
	  {
	     if (ecore_x_window_prop_card32_get(ecore_x_window_root_first_get(),
						_atom_e_finger_size,
						&val, 1) > 0)
	       {
		  if (val > 0)
		    {
		       _elm_config->finger_size = val;
		    }
	       }
	  }
	if (!getenv("ELM_THEME"))
	  {
             char *s;
             
             s = ecore_x_window_prop_string_get(ecore_x_window_root_first_get(),
                                                _atom_e_theme);
             if (s)
	       {
		  eina_stringshare_replace(&_elm_config->theme, s);
                  _elm_theme_parse(s);
                  free(s);
	       }
	  }
#endif
      }
}

void
_elm_config_shutdown(void)
{
   if ((_elm_config->engine == ELM_SOFTWARE_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_16_X11) ||
       (_elm_config->engine == ELM_XRENDER_X11) ||
       (_elm_config->engine == ELM_OPENGL_X11) ||
       (_elm_config->engine == ELM_SOFTWARE_SDL) ||
       (_elm_config->engine == ELM_SOFTWARE_16_SDL) ||
       (_elm_config->engine == ELM_OPENGL_SDL) ||
       (_elm_config->engine == ELM_SOFTWARE_WIN32) ||
       (_elm_config->engine == ELM_SOFTWARE_16_WINCE))
     {
#ifdef HAVE_ELEMENTARY_X
	ecore_event_handler_del(_prop_change_handler);
	_prop_change_handler = NULL;
#endif
     }
   if (_elm_config)
     {
        const char *fontdir;
        
        EINA_LIST_FREE(_elm_config->font_dirs, fontdir)
          {
             eina_stringshare_del(fontdir);
          }
        if (_elm_config->theme) eina_stringshare_del(_elm_config->theme);
        if (_elm_config->modules) eina_stringshare_del(_elm_config->modules);
        free(_elm_config);
        _elm_config = NULL;
     }
   if (_elm_profile)
     {
        free(_elm_profile);
        _elm_profile = NULL;
     }
   _desc_shutdown();
}
