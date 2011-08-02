#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <errno.h>

#ifndef _MSC_VER
# include <unistd.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Input.h"

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

int _ecore_evas_log_dom = -1;
static int _ecore_evas_init_count = 0;
static Ecore_Fd_Handler *_ecore_evas_async_events_fd = NULL;
static Eina_Bool _ecore_evas_async_events_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);

static Ecore_Idle_Enterer *ecore_evas_idle_enterer = NULL;
static Ecore_Evas *ecore_evases = NULL;
static int _ecore_evas_fps_debug = 0;

static Eina_Bool
_ecore_evas_idle_enter(void *data __UNUSED__)
{
   Ecore_Evas *ee;
   double t1 = 0.0;
   double t2 = 0.0;
   int rend = 0;

   if (!ecore_evases) return ECORE_CALLBACK_RENEW;
   if (_ecore_evas_fps_debug)
     {
        t1 = ecore_time_get();
     }
   EINA_INLIST_FOREACH(ecore_evases, ee)
     {
        if (!ee->manual_render)
          {
             if (ee->engine.func->fn_render)
               rend |= ee->engine.func->fn_render(ee);
          }
     }
   if (_ecore_evas_fps_debug)
     {
        t2 = ecore_time_get();
        if (rend)
          _ecore_evas_fps_debug_rendertime_add(t2 - t1);
     }
   return ECORE_CALLBACK_RENEW;
}

/**
 * Query if a particular renginering engine target has support
 * @param  engine The engine to check support for
 * @return 1 if the particular engine is supported, 0 if it is not
 *
 * Query if engine @param engine is supported by ecore_evas. 1 is returned if
 * it is, and 0 is returned if it is not supported.
 */
EAPI int
ecore_evas_engine_type_supported_get(Ecore_Evas_Engine_Type engine)
{
   switch (engine)
     {
      case ECORE_EVAS_ENGINE_SOFTWARE_BUFFER:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_SOFTWARE_XLIB:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_XLIB
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_XRENDER_X11:
        return 0;
      case ECORE_EVAS_ENGINE_OPENGL_X11:
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_SOFTWARE_XCB:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_XCB
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_XRENDER_XCB:
        return 0;
      case ECORE_EVAS_ENGINE_SOFTWARE_GDI:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_SOFTWARE_DDRAW:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_DIRECT3D:
#ifdef BUILD_ECORE_EVAS_DIRECT3D
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_OPENGL_GLEW:
#ifdef BUILD_ECORE_EVAS_OPENGL_GLEW
        return 1;
#else
        return 0;
#endif
     case ECORE_EVAS_ENGINE_SOFTWARE_SDL:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_SDL
        return 1;
#else
        return 0;
#endif
     case ECORE_EVAS_ENGINE_OPENGL_SDL:
#ifdef BUILD_ECORE_EVAS_OPENGL_SDL
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_DIRECTFB:
#ifdef BUILD_ECORE_EVAS_DIRECTFB
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_SOFTWARE_FB:
#ifdef BUILD_ECORE_EVAS_FB
        return 1;
#else
        return 0;
#endif

          case ECORE_EVAS_ENGINE_SOFTWARE_8_X11:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_8_X11
        return 1;
#else
        return 0;
#endif

      case ECORE_EVAS_ENGINE_SOFTWARE_16_X11:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_X11
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_SOFTWARE_16_DDRAW:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_DDRAW
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_SOFTWARE_16_WINCE:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE
        return 1;
#else
        return 0;
#endif
      case ECORE_EVAS_ENGINE_COCOA:
#ifdef BUILD_ECORE_EVAS_COCOA
         return 1;
#else
         return 0;
#endif
      default:
        return 0;
        break;
     };
}

/**
 * Init the Evas system.
 * @return greater than 0 on success, 0 on failure
 *
 * Set up the Evas wrapper system. Init Evas and Ecore libraries.
 */
EAPI int
ecore_evas_init(void)
{
   int fd;

   if (++_ecore_evas_init_count != 1)
     return _ecore_evas_init_count;

   if (!evas_init())
     return --_ecore_evas_init_count;

   if (!ecore_init())
     goto shutdown_evas;

   _ecore_evas_log_dom = eina_log_domain_register
     ("ecore_evas", ECORE_EVAS_DEFAULT_LOG_COLOR);
   if(_ecore_evas_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for Ecore_Evas.");
        goto shutdown_ecore;
     }

   fd = evas_async_events_fd_get();
   if (fd > 0)
     _ecore_evas_async_events_fd = ecore_main_fd_handler_add(fd,
                                                             ECORE_FD_READ,
                                                             _ecore_evas_async_events_fd_handler, NULL,
                                                             NULL, NULL);

   ecore_evas_idle_enterer =
     ecore_idle_enterer_add(_ecore_evas_idle_enter, NULL);
   if (getenv("ECORE_EVAS_FPS_DEBUG")) _ecore_evas_fps_debug = 1;
   if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_init();

   return _ecore_evas_init_count;

 shutdown_ecore:
   ecore_shutdown();
 shutdown_evas:
   evas_shutdown();

   return --_ecore_evas_init_count;
}

/**
 * Shut down the Evas system.
 * @return 0 if ecore evas is fully shut down, or > 0 if it still needs to be shut down
 *
 * This closes the Evas wrapper system down. Shut down Evas and Ecore libraries.
 */
EAPI int
ecore_evas_shutdown(void)
{
   if (--_ecore_evas_init_count != 0)
     return _ecore_evas_init_count;

   while (ecore_evases) _ecore_evas_free(ecore_evases);

   if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_shutdown();
   ecore_idle_enterer_del(ecore_evas_idle_enterer);
   ecore_evas_idle_enterer = NULL;

#ifdef BUILD_ECORE_EVAS_X11
   while (_ecore_evas_x_shutdown());
#endif
#ifdef BUILD_ECORE_EVAS_WIN32
   while (_ecore_evas_win32_shutdown());
#endif
#ifdef BUILD_ECORE_EVAS_FB
   while (_ecore_evas_fb_shutdown());
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   while (_ecore_evas_buffer_shutdown());
#endif
#ifdef BUILD_ECORE_EVAS_DIRECTFB
   while (_ecore_evas_directfb_shutdown());
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE
   while (_ecore_evas_wince_shutdown());
#endif
   if (_ecore_evas_async_events_fd)
     ecore_main_fd_handler_del(_ecore_evas_async_events_fd);

   eina_log_domain_unregister(_ecore_evas_log_dom);
   _ecore_evas_log_dom = -1;
   ecore_shutdown();
   evas_shutdown();

   return _ecore_evas_init_count;
}

Eina_Bool _ecore_evas_app_comp_sync = 1;

EAPI void
ecore_evas_app_comp_sync_set(Eina_Bool do_sync)
{
   _ecore_evas_app_comp_sync = do_sync;
}

EAPI Eina_Bool
ecore_evas_app_comp_sync_get(void)
{
   return _ecore_evas_app_comp_sync;
}

struct ecore_evas_engine {
   const char *name;
   Ecore_Evas *(*constructor)(int x, int y, int w, int h, const char *extra_options);
};

/* inline is just to avoid need to ifdef around it */
static inline const char *
_ecore_evas_parse_extra_options_str(const char *extra_options, const char *key, char **value)
{
   int len = strlen(key);

   while (extra_options)
     {
        const char *p;

        if (strncmp(extra_options, key, len) != 0)
          {
             extra_options = strchr(extra_options, ';');
             if (extra_options)
               extra_options++;
             continue;
          }

        extra_options += len;
        p = strchr(extra_options, ';');
        if (p)
          {
             len = p - extra_options;
             *value = malloc(len + 1);
             memcpy(*value, extra_options, len);
             (*value)[len] = '\0';
             extra_options = p + 1;
          }
        else
          {
             *value = strdup(extra_options);
             extra_options = NULL;
          }
     }
   return extra_options;
}

/* inline is just to avoid need to ifdef around it */
static inline const char *
_ecore_evas_parse_extra_options_uint(const char *extra_options, const char *key, unsigned int *value)
{
   int len = strlen(key);

   while (extra_options)
     {
        const char *p;

        if (strncmp(extra_options, key, len) != 0)
          {
             extra_options = strchr(extra_options, ';');
             if (extra_options)
               extra_options++;
             continue;
          }

        extra_options += len;
        *value = strtol(extra_options, NULL, 0);

        p = strchr(extra_options, ';');
        if (p)
          extra_options = p + 1;
        else
          extra_options = NULL;
     }
   return extra_options;
}

/* inline is just to avoid need to ifdef around it */
static inline const char *
_ecore_evas_parse_extra_options_x(const char *extra_options, char **disp_name, unsigned int *parent)
{
   _ecore_evas_parse_extra_options_str(extra_options, "display=", disp_name);
   _ecore_evas_parse_extra_options_uint(extra_options, "parent=", parent);
   return extra_options;
}

#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
static Ecore_Evas *
_ecore_evas_constructor_software_x11(int x, int y, int w, int h, const char *extra_options)
{
   unsigned int parent = 0;
   char *disp_name = NULL;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_x(extra_options, &disp_name, &parent);
   ee = ecore_evas_software_x11_new(disp_name, parent, x, y, w, h);
   free(disp_name);

   return ee;
}
#endif

#ifdef BUILD_ECORE_EVAS_COCOA
static Ecore_Evas *
_ecore_evas_constructor_cocoa(int x, int y, int w, int h, const char *extra_options)
{
   char *name = NULL;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_str(extra_options, "name=", &name);
   ee = ecore_evas_cocoa_new(name, w, h);
   free(name);

   if (ee) ecore_evas_move(ee, x, y);
   return ee;
}
#endif

#ifdef BUILD_ECORE_EVAS_OPENGL_X11
static Ecore_Evas *
_ecore_evas_constructor_opengl_x11(int x, int y, int w, int h, const char *extra_options)
{
   Ecore_X_Window parent = 0;
   char *disp_name = NULL;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_x(extra_options, &disp_name, &parent);
   ee = ecore_evas_gl_x11_new(disp_name, parent, x, y, w, h);
   free(disp_name);

   return ee;
}
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_8_X11
static Ecore_Evas *
_ecore_evas_constructor_software_8_x11(int x, int y, int w, int h, const char *extra_options)
{
   Ecore_X_Window parent = 0;
   char *disp_name = NULL;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_x(extra_options, &disp_name, &parent);
   ee = ecore_evas_software_x11_8_new(disp_name, parent, x, y, w, h);
   free(disp_name);

   return ee;
}
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_X11
static Ecore_Evas *
_ecore_evas_constructor_software_16_x11(int x, int y, int w, int h, const char *extra_options)
{
   Ecore_X_Window parent = 0;
   char *disp_name = NULL;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_x(extra_options, &disp_name, &parent);
   ee = ecore_evas_software_x11_16_new(disp_name, parent, x, y, w, h);
   free(disp_name);

   return ee;
}
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_SDL
static Ecore_Evas *
_ecore_evas_constructor_sdl(int x __UNUSED__, int y __UNUSED__, int w, int h, const char *extra_options)
{
   Ecore_Evas *ee;
   unsigned int fullscreen = 0, hwsurface = 0, noframe = 0, alpha = 0;
   char *name = NULL;

   _ecore_evas_parse_extra_options_str(extra_options, "name=", &name);
   _ecore_evas_parse_extra_options_uint(extra_options, "fullscreen=", &fullscreen);
   _ecore_evas_parse_extra_options_uint(extra_options, "hwsurface=", &hwsurface);
   _ecore_evas_parse_extra_options_uint(extra_options, "alpha=", &alpha);

   ee = ecore_evas_sdl_new(name, w, h, fullscreen, hwsurface, noframe, alpha);
   free(name);

   return ee;
}

static Ecore_Evas *
_ecore_evas_constructor_sdl16(int x __UNUSED__, int y __UNUSED__, int w, int h, const char *extra_options)
{
   Ecore_Evas *ee;
   unsigned int fullscreen = 0, hwsurface = 0, noframe = 0, alpha = 0;
   char *name = NULL;

   _ecore_evas_parse_extra_options_str(extra_options, "name=", &name);
   _ecore_evas_parse_extra_options_uint(extra_options, "fullscreen=", &fullscreen);
   _ecore_evas_parse_extra_options_uint(extra_options, "hwsurface=", &hwsurface);
   _ecore_evas_parse_extra_options_uint(extra_options, "alpha=", &alpha);

   ee = ecore_evas_sdl16_new(name, w, h, fullscreen, hwsurface, noframe, alpha);
   free(name);

   return ee;
}
#endif

#ifdef BUILD_ECORE_EVAS_OPENGL_SDL
static Ecore_Evas *
_ecore_evas_constructor_opengl_sdl(int x __UNUSED__, int y __UNUSED__, int w, int h, const char *extra_options)
{
   Ecore_Evas *ee;
   unsigned int fullscreen = 0, noframe = 0;
   char *name = NULL;

   _ecore_evas_parse_extra_options_str(extra_options, "name=", &name);
   _ecore_evas_parse_extra_options_uint(extra_options, "fullscreen=", &fullscreen);
   _ecore_evas_parse_extra_options_uint(extra_options, "noframe=", &noframe);

   ee = ecore_evas_gl_sdl_new(name, w, h, fullscreen, noframe);
   free(name);

   return ee;
}
#endif

#ifdef BUILD_ECORE_EVAS_DIRECTFB
static Ecore_Evas *
_ecore_evas_constructor_directfb(int x, int y, int w, int h, const char *extra_options)
{
   Ecore_Evas *ee;
   char *disp_name = NULL;
   unsigned int windowed = 1;

   _ecore_evas_parse_extra_options_str(extra_options, "display=", &disp_name);
   _ecore_evas_parse_extra_options_uint(extra_options, "windowed=", &windowed);

   ee = ecore_evas_directfb_new(disp_name, windowed, x, y, w, h);
   free(disp_name);

   return ee;
}
#endif

#ifdef BUILD_ECORE_EVAS_FB
static Ecore_Evas *
_ecore_evas_constructor_fb(int x __UNUSED__, int y __UNUSED__, int w, int h, const char *extra_options)
{
   Ecore_Evas *ee;
   char *disp_name = NULL;
   unsigned int rotation = 0;

   _ecore_evas_parse_extra_options_str(extra_options, "display=", &disp_name);
   _ecore_evas_parse_extra_options_uint(extra_options, "rotation=", &rotation);

   ee = ecore_evas_fb_new(disp_name, rotation, w, h);
   free(disp_name);

   return ee;
}
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
static Ecore_Evas *
_ecore_evas_constructor_software_gdi(int x, int y, int w, int h, const char *extra_options)
{
   return ecore_evas_software_gdi_new(NULL, x, y, w, h);
}
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
static Ecore_Evas *
_ecore_evas_constructor_software_ddraw(int x, int y, int w, int h, const char *extra_options)
{
   return ecore_evas_software_ddraw_new(NULL, x, y, w, h);
}
#endif

#ifdef BUILD_ECORE_EVAS_DIRECT3D
static Ecore_Evas *
_ecore_evas_constructor_direct3d(int x, int y, int w, int h, const char *extra_options)
{
   return ecore_evas_direct3d_new(NULL, x, y, w, h);
}
#endif

#ifdef BUILD_ECORE_EVAS_OPENGL_GLEW
static Ecore_Evas *
_ecore_evas_constructor_opengl_glew(int x, int y, int w, int h, const char *extra_options)
{
   return ecore_evas_gl_glew_new(NULL, x, y, w, h);
}
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_DDRAW
static Ecore_Evas *
_ecore_evas_constructor_software_16_ddraw(int x, int y, int w, int h, const char *extra_options)
{
   return ecore_evas_software_16_ddraw_new(NULL, x, y, w, h);
}
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE
static Ecore_Evas *
_ecore_evas_constructor_software_16_wince(int x, int y, int w, int h, const char *extra_options __UNUSED__)
{
   return ecore_evas_software_wince_new(NULL, x, y, w, h);
}

static Ecore_Evas *
_ecore_evas_constructor_software_16_wince_fb(int x, int y, int w, int h, const char *extra_options __UNUSED__)
{
   return ecore_evas_software_wince_fb_new(NULL, x, y, w, h);
}

static Ecore_Evas *
_ecore_evas_constructor_software_16_wince_gapi(int x, int y, int w, int h, const char *extra_options __UNUSED__)
{
   return ecore_evas_software_wince_gapi_new(NULL, x, y, w, h);
}

static Ecore_Evas *
_ecore_evas_constructor_software_16_wince_gdi(int x, int y, int w, int h, const char *extra_options __UNUSED__)
{
   return ecore_evas_software_wince_gdi_new(NULL, x, y, w, h);
}
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
static Ecore_Evas *
_ecore_evas_constructor_buffer(int x __UNUSED__, int y __UNUSED__, int w, int h, const char *extra_options __UNUSED__)
{
   return ecore_evas_buffer_new(w, h);
}
#endif

/* note: keep sorted by priority, highest first */
static const struct ecore_evas_engine _engines[] = {
  /* unix */
#ifdef BUILD_ECORE_EVAS_SOFTWARE_X11
  {"software_x11", _ecore_evas_constructor_software_x11},
#endif
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
  {"opengl_x11", _ecore_evas_constructor_opengl_x11},
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_8_X11
  {"software_8_x11", _ecore_evas_constructor_software_8_x11},
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_X11
  {"software_16_x11", _ecore_evas_constructor_software_16_x11},
#endif
#ifdef BUILD_ECORE_EVAS_DIRECTFB
  {"directfb", _ecore_evas_constructor_directfb},
#endif
#ifdef BUILD_ECORE_EVAS_FB
  {"fb", _ecore_evas_constructor_fb},
#endif

  /* windows */
#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
  {"software_gdi", _ecore_evas_constructor_software_gdi},
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
  {"software_ddraw", _ecore_evas_constructor_software_ddraw},
#endif
#ifdef BUILD_ECORE_EVAS_DIRECT3D
  {"direct3d", _ecore_evas_constructor_direct3d},
#endif
#ifdef BUILD_ECORE_EVAS_OPENGL_GLEW
  {"opengl_glew", _ecore_evas_constructor_opengl_glew},
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_DDRAW
  {"software_16_ddraw", _ecore_evas_constructor_software_16_ddraw},
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE
  {"software_16_wince", _ecore_evas_constructor_software_16_wince},
  {"software_16_wince_fb", _ecore_evas_constructor_software_16_wince_fb},
  {"software_16_wince_gapi", _ecore_evas_constructor_software_16_wince_gapi},
  {"software_16_wince_gdi", _ecore_evas_constructor_software_16_wince_gdi},
#endif

  /* Apple */
#ifdef BUILD_ECORE_EVAS_COCOA
  {"cocoa", _ecore_evas_constructor_cocoa},
#endif

  /* Last chance to have a window */
#ifdef BUILD_ECORE_EVAS_OPENGL_SDL
  {"opengl_sdl", _ecore_evas_constructor_opengl_sdl},
#endif

#ifdef BUILD_ECORE_EVAS_SOFTWARE_SDL
  {"sdl", _ecore_evas_constructor_sdl},
  {"software_16_sdl", _ecore_evas_constructor_sdl16},
#endif

  /* independent */
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
  {"buffer", _ecore_evas_constructor_buffer},
#endif
  {NULL, NULL}
};

/**
 * Returns a list of supported engines names.
 *
 * @return newly allocated list with engines names. Engines names
 * strings are internal and should be considered constants, do not
 * free them, to avoid problems use ecore_evas_engines_free()
 */
EAPI Eina_List *
ecore_evas_engines_get(void)
{
   const struct ecore_evas_engine *itr;
   Eina_List *lst = NULL;

   for (itr = _engines; itr->name; itr++)
     lst = eina_list_append(lst, itr->name);

   return lst;
}

/**
 * Free list returned by ecore_evas_engines_get()
 */
EAPI void
ecore_evas_engines_free(Eina_List *engines)
{
   eina_list_free(engines);
}

static Ecore_Evas *
_ecore_evas_new_auto_discover(int x, int y, int w, int h, const char *extra_options)
{
   const struct ecore_evas_engine *itr;

   DBG("auto discover engine");

   for (itr = _engines; itr->constructor; itr++)
     {
        Ecore_Evas *ee = itr->constructor(x, y, w, h, extra_options);
        if (ee)
          {
             INF("auto discovered '%s'", itr->name);
             return ee;
          }
     }

   WRN("could not auto discover.");
   return NULL;
}

/**
 * Creates a new Ecore_Evas based on engine name and common parameters.
 *
 * @param engine_name engine name as returned by
 *        ecore_evas_engines_get() or NULL to use environment variable
 *        ECORE_EVAS_ENGINE, that can be undefined and in this case
 *        this call will try to find the first working engine.
 * @param x horizontal position of window (not supported in all engines)
 * @param y vertical position of window (not supported in all engines)
 * @param w width of window
 * @param h height of window
 * @param extra_options string with extra parameter, dependent on engines
 *        or NULL. String is usually in the form: 'key1=value1;key2=value2'.
 *        Pay attention that when getting that from shell commands, most
 *        consider ';' as the command terminator, so you need to escape
 *        it or use quotes.
 *
 * @return Ecore_Evas instance or NULL if creation failed.
 */
EAPI Ecore_Evas *
ecore_evas_new(const char *engine_name, int x, int y, int w, int h, const char *extra_options)
{
   const struct ecore_evas_engine *itr;

   if (!engine_name)
     {
        engine_name = getenv("ECORE_EVAS_ENGINE");
        if (engine_name)
          DBG("no engine_name provided, using ECORE_EVAS_ENGINE='%s'",
              engine_name);
     }
   if (!engine_name)
     return _ecore_evas_new_auto_discover(x, y, w, h, extra_options);

   for (itr = _engines; itr->name; itr++)
     if (strcmp(itr->name, engine_name) == 0)
       {
          INF("using engine '%s', extra_options=%s",
              engine_name, extra_options ? extra_options : "(null)");
          return itr->constructor(x, y, w, h, extra_options);
       }

   WRN("unknown engine '%s'", engine_name);
   return NULL;
}

/**
 * Get the engine name used by this engine.
 *
 * should return one of the values in ecore_evas_engines_get(), usually
 * acceptable by ecore_evas_new().
 */
EAPI const char *
ecore_evas_engine_name_get(const Ecore_Evas *ee)
{
   if (!ee)
     return NULL;
   return ee->driver;
}

/**
 * Return the Ecore_Evas for this Evas
 *
 * @param e The Evas to get the Ecore_Evas from
 * @return The Ecore_Evas that holds this Evas, or NULL if not hold by one.
 */
EAPI Ecore_Evas *
ecore_evas_ecore_evas_get(const Evas *e)
{
   Ecore_Evas *ee = evas_data_attach_get(e);
   if (!ee) return NULL;
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, "ecore_evas_ecore_evas_get");
        return NULL;
     }
   return ee;
}

/**
 * Free an Ecore_Evas
 * @param ee The Ecore_Evas to free
 *
 * This frees up any memory used by the Ecore_Evas.
 */
EAPI void
ecore_evas_free(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_free");
        return;
     }
   _ecore_evas_free(ee);
   return;
}

/**
 * Retrieve user data associated with an Ecore_Evas.
 * @param ee The Ecore_Evas to retrieve the user data from.
 * @param key The key which the user data to be retrieved is associated with.
 *
 * This function retrieves user specific data that has been stored within an
 * Ecore_Evas structure with ecore_evas_data_set().
 *
 * @returns NULL on error or no data found, A pointer to the user data on
 *     success.
 *
 * @see ecore_evas_data_set
 */
EAPI void *
ecore_evas_data_get(const Ecore_Evas *ee, const char *key)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_data_get");
        return NULL;
     }

   if (!key) return NULL;
   if (!ee->data) return NULL;

   return eina_hash_find(ee->data, key);
}

/**
 * Store user data in an Ecore_Evas structure.
 *
 * @param ee The Ecore_Evas to store the user data in.
 * @param key A unique string to associate the user data against. Cannot
 * be NULL.
 * @param data A pointer to the user data to store.
 *
 * This function associates the @p data with a @p key which is stored by
 * the Ecore_Evas @p ee. Be aware that a call to ecore_evas_free() will
 * not free any memory for the associated user data, this is the responsibility
 * of the caller.
 *
 * @see ecore_evas_free
 */
EAPI void
ecore_evas_data_set(Ecore_Evas *ee, const char *key, const void *data)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_data_set");
        return;
     }

   if (!key) return;

   if (ee->data)
     eina_hash_del(ee->data, key, NULL);
   if (data)
     {
       if (!ee->data)
         ee->data = eina_hash_string_superfast_new(NULL);
       eina_hash_add(ee->data, key, data);
     }
}

#define IFC(_ee, _fn)  if (_ee->engine.func->_fn) {_ee->engine.func->_fn
#define IFE            return;}

EAPI void
ecore_evas_callback_resize_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_resize_set");
        return;
     }
   IFC(ee, fn_callback_resize_set) (ee, func);
   IFE;
   ee->func.fn_resize = func;
}

EAPI void
ecore_evas_callback_move_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_move_set");
        return;
     }
   IFC(ee, fn_callback_move_set) (ee, func);
   IFE;
   ee->func.fn_move = func;
}

EAPI void
ecore_evas_callback_show_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_show_set");
        return;
     }
   IFC(ee, fn_callback_show_set) (ee, func);
   IFE;
   ee->func.fn_show = func;
}

EAPI void
ecore_evas_callback_hide_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_hide_set");
        return;
     }
   IFC(ee, fn_callback_hide_set) (ee, func);
   IFE;
   ee->func.fn_hide = func;
}

EAPI void
ecore_evas_callback_delete_request_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_delete_request_set");
        return;
     }
   IFC(ee, fn_callback_delete_request_set) (ee, func);
   IFE;
   ee->func.fn_delete_request = func;
}

EAPI void
ecore_evas_callback_destroy_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_destroy_set");
        return;
     }
   IFC(ee, fn_callback_destroy_set) (ee, func);
   IFE;
   ee->func.fn_destroy = func;
}

EAPI void
ecore_evas_callback_focus_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_focus_in_set");
        return;
     }
   IFC(ee, fn_callback_focus_in_set) (ee, func);
   IFE;
   ee->func.fn_focus_in = func;
}

EAPI void
ecore_evas_callback_focus_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_focus_out_set");
        return;
     }
   IFC(ee, fn_callback_focus_out_set) (ee, func);
   IFE;
   ee->func.fn_focus_out = func;
}

EAPI void
ecore_evas_callback_sticky_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_sticky_set");
        return;
     }
   IFC(ee, fn_callback_sticky_set) (ee, func);
   IFE;
   ee->func.fn_sticky = func;
}

EAPI void
ecore_evas_callback_unsticky_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_unsticky_set");
        return;
     }
   IFC(ee, fn_callback_unsticky_set) (ee, func);
   IFE;
   ee->func.fn_unsticky = func;
}

EAPI void
ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_mouse_in_set");
        return;
     }
   IFC(ee, fn_callback_mouse_in_set) (ee, func);
   IFE;
   ee->func.fn_mouse_in = func;
}

EAPI void
ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_mouse_out_set");
        return;
     }
   IFC(ee, fn_callback_mouse_out_set) (ee, func);
   IFE;
   ee->func.fn_mouse_out = func;
}

EAPI void
ecore_evas_callback_pre_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_pre_render_set");
        return;
     }
   IFC(ee, fn_callback_pre_render_set) (ee, func);
   IFE;
   ee->func.fn_pre_render = func;
}

EAPI void
ecore_evas_callback_post_render_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_post_render_set");
        return;
     }
   IFC(ee, fn_callback_post_render_set) (ee, func);
   IFE;
   ee->func.fn_post_render = func;
}

EAPI void
ecore_evas_callback_pre_free_set(Ecore_Evas *ee, void (*func) (Ecore_Evas *ee))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_pre_free_set");
        return;
     }
   ee->func.fn_pre_free = func;
}

/**
 * Get an Ecore_Evas's Evas
 * @param ee The Ecore_Evas whose Evas you wish to get
 * @return The Evas wrapped by @p ee
 *
 * This function returns the Evas contained within @p ee.
 */
EAPI Evas *
ecore_evas_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_get");
        return NULL;
     }
   return ee->evas;
}

/**
 * Move an Ecore_Evas
 * @param ee The Ecore_Evas to move
 * @param x The x coordinate to move to
 * @param y The y coordinate to move to
 *
 * This moves @p ee to the screen coordinates (@p x, @p y)
 */
EAPI void
ecore_evas_move(Ecore_Evas *ee, int x, int y)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_move");
        return;
     }
   if (ee->prop.fullscreen) return;
   IFC(ee, fn_move) (ee, x, y);
   IFE;
}

/**
 * Provide Managed move co-ordinates for an Ecore_Evas
 * @param ee The Ecore_Evas to move
 * @param x The x coordinate to set as the managed location
 * @param y The y coordinate to set as the managed location
 *
 * This sets the managed geometry position of the @p ee to (@p x, @p y)
 */
EAPI void
ecore_evas_managed_move(Ecore_Evas *ee, int x, int y)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_move");
        return;
     }
   IFC(ee, fn_managed_move) (ee, x, y);
   IFE;
}

/**
 * Resize an Ecore_Evas
 * @param ee The Ecore_Evas to move
 * @param w The w coordinate to resize to
 * @param h The h coordinate to resize to
 *
 * This resizes @p ee to @p w x @p h
 */
EAPI void
ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_resize");
        return;
     }
   if (ee->prop.fullscreen) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        IFC(ee, fn_resize) (ee, h, w);
        IFE;
     }
   else
     {
        IFC(ee, fn_resize) (ee, w, h);
        IFE;
     }
}

/**
 * Resize an Ecore_Evas
 * @param ee The Ecore_Evas to move
 * @param x The x coordinate to move to
 * @param y The y coordinate to move to
 * @param w The w coordinate to resize to
 * @param h The h coordinate to resize to
 *
 * This moves @p ee to the screen coordinates (@p x, @p y) and  resizes
 * it to @p w x @p h.
 *
 */
EAPI void
ecore_evas_move_resize(Ecore_Evas *ee, int x, int y, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_move_resize");
        return;
     }
   if (ee->prop.fullscreen) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        IFC(ee, fn_move_resize) (ee, x, y, h, w);
        IFE;
     }
   else
     {
        IFC(ee, fn_move_resize) (ee, x, y, w, h);
        IFE;
     }
}

/**
 * Get the geometry of an Ecore_Evas
 * @param ee The Ecore_Evas whose geometry y
 * @param x A pointer to an int to place the x coordinate in
 * @param y A pointer to an int to place the y coordinate in
 * @param w A pointer to an int to place the w size in
 * @param h A pointer to an int to place the h size in
 *
 * This function takes four pointers to (already allocated) ints, and places
 * the geometry of @p ee in them.
 *
 * @code
 * int x, y, w, h;
 * ecore_evas_geometry_get(ee, &x, &y, &w, &h);
 * @endcode
 *
 */
EAPI void
ecore_evas_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_geometry_get");
        return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        if (x) *x = ee->x;
        if (y) *y = ee->y;
        if (w) *w = ee->h;
        if (h) *h = ee->w;
     }
   else
     {
        if (x) *x = ee->x;
        if (y) *y = ee->y;
        if (w) *w = ee->w;
        if (h) *h = ee->h;
     }
}

/**
 * Set the rotation of an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas
 * @param rot the angle (in degrees) of rotation.
 *
 * The allowed values of @p rot depend on the engine being used. Most only
 * allow multiples of 90.
 */
EAPI void
ecore_evas_rotation_set(Ecore_Evas *ee, int rot)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_rotation_set");
        return;
     }
   rot = rot % 360;
   while (rot < 0) rot += 360;
   while (rot >= 360) rot -= 360;
   IFC(ee, fn_rotation_set) (ee, rot, 0);
   /* make sure everything gets redrawn */
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
   IFE;
}

/**
 * Set the rotation of an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas
 * @param rot the angle (in degrees) of rotation.
 *
 * The allowed values of @p rot depend on the engine being used. Most only
 * allow multiples of 90.
 */
EAPI void
ecore_evas_rotation_with_resize_set(Ecore_Evas *ee, int rot)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_rotation_set");
        return;
     }
   rot = rot % 360;
   while (rot < 0) rot += 360;
   while (rot >= 360) rot -= 360;
   IFC(ee, fn_rotation_set) (ee, rot, 1);
   /* make sure everything gets redrawn */
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
   IFE;
}

/**
 * Set the rotation of an Ecore_Evas' window
 *
 * @param ee The Ecore_Evas
 * @return the angle (in degrees) of rotation.
 *
 */
EAPI int
ecore_evas_rotation_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_rotation_get");
        return 0;
     }
   return ee->rotation;
}

/**
 * Set whether an Ecore_Evas is shaped or not.
 * @param ee The Ecore_Evas to shape
 * @param shaped EINA_TRUE to shape, EINA_FALSE to not
 *
 * This function allows one to make an Ecore_Evas shaped to the contents of the
 * evas. If @p shaped is EINA_TRUE, @p ee will be transparent in parts of the evas that
 * contain no objects. If @p shaped is EINA_FALSE, then @p ee will be rectangular, and
 * and parts with no data will show random framebuffer artifacting. For
 * non-shaped Ecore_Evases, it is recommend to cover the entire evas with a
 * background object.
 */
EAPI void
ecore_evas_shaped_set(Ecore_Evas *ee, Eina_Bool shaped)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_shaped_set");
        return;
     }
   IFC(ee, fn_shaped_set) (ee, shaped);
   IFE;
}

/**
 * Query whether an Ecore_Evas is shaped or not.
 * @param ee The Ecore_Evas to query.
 * @return EINA_TRUE if shaped, EINA_FALSE if not.
 *
 * This function returns EINA_TRUE if @p ee is shaped, and EINA_FALSE if not.
 */
EAPI Eina_Bool
ecore_evas_shaped_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_shaped_get");
        return EINA_FALSE;
     }
   return ee->shaped ? EINA_TRUE : EINA_FALSE;
}

/**
 * Set whether an Ecore_Evas has an alpha channel or not.
 * @param ee The Ecore_Evas to shape
 * @param alpha EINA_TRUE to enable the alpha channel, EINA_FALSE to disable it
 *
 * This function allows you to make an Ecore_Evas translucent using an
 * alpha channel. See ecore_evas_shaped_set() for details. The difference
 * between a shaped window and a window with an alpha channel is that an
 * alpha channel supports multiple levels of transpararency, as opposed to
 * the 1 bit transparency of a shaped window (a pixel is either opaque, or
 * it's transparent).
 */
EAPI void
ecore_evas_alpha_set(Ecore_Evas *ee, Eina_Bool alpha)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_alpha_set");
        return;
     }
   IFC(ee, fn_alpha_set) (ee, alpha);
   IFE;
}

/**
 * Query whether an Ecore_Evas has an alpha channel.
 * @param ee The Ecore_Evas to query.
 * @return EINA_TRUE if ee has an alpha channel, EINA_FALSE if it does not.
 *
 * This function returns EINA_TRUE if @p ee has an alpha channel, and EINA_FALSE if
 * it does not.
 */
EAPI Eina_Bool
ecore_evas_alpha_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_alpha_get");
        return EINA_FALSE;
     }
   return ee->alpha ? EINA_TRUE : EINA_FALSE;
}

/**
 * Set whether an Ecore_Evas has an transparent window or not.
 * @param ee The Ecore_Evas to shape
 * @param transparent EINA_TRUE to enable the transparent window, EINA_FALSE to disable it
 *
 * This function allows you to make an Ecore_Evas translucent using an
 * alpha channel. See ecore_evas_shaped_set() for details. The difference
 * between a shaped window and a window with an alpha channel is that an
 * alpha channel supports multiple levels of transpararency, as opposed to
 * the 1 bit transparency of a shaped window (a pixel is either opaque, or
 * it's transparent).
 */
EAPI void
ecore_evas_transparent_set(Ecore_Evas *ee, Eina_Bool transparent)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_transparent_set");
        return;
     }
   IFC(ee, fn_transparent_set) (ee, transparent);
   IFE;
}

/**
 * Query whether an Ecore_Evas has an alpha channel.
 * @param ee The Ecore_Evas to query.
 * @return EINA_TRUE if ee has an alpha channel, EINA_FALSE if it does not.
 *
 * This function returns EINA_TRUE if @p ee has an alpha channel, and EINA_FALSE if
 * it does not.
 */
EAPI Eina_Bool
ecore_evas_transparent_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_transparent_get");
        return EINA_FALSE;
     }
   return ee->transparent ? EINA_TRUE : 0;
}

/**
 * Show an Ecore_Evas' window
 * @param ee The Ecore_Evas to show.
 *
 * This function makes @p ee visible.
 */
EAPI void
ecore_evas_show(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_show");
        return;
     }
   IFC(ee, fn_show) (ee);
   IFE;
}

/**
 * Hide an Ecore_Evas' window
 * @param ee The Ecore_Evas to show.
 *
 * This function makes @p ee hidden.
 */
EAPI void
ecore_evas_hide(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_hide");
        return;
     }
   IFC(ee, fn_hide) (ee);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is visible or not.
 * @param ee The Ecore_Evas to query.
 * @return 1 if visible, 0 if not.
 *
 * This function queries @p ee and returns 1 if it is visible, and 0 if not.
 */
EAPI int
ecore_evas_visibility_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_visibility_get");
        return 0;
     }
   return ee->visible ? 1:0;
}

/**
 * Raise and Ecore_Evas' window.
 * @param ee The Ecore_Evas to raise.
 *
 * This functions raises the Ecore_Evas to the front.
 */
EAPI void
ecore_evas_raise(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_raise");
        return;
     }
   IFC(ee, fn_raise) (ee);
   IFE;
}

/**
 * Lower an Ecore_Evas' window.
 * @param ee The Ecore_Evas to raise.
 *
 * This functions lowers the Ecore_Evas to the back.
 */
EAPI void
ecore_evas_lower(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_lower");
        return;
     }
   IFC(ee, fn_lower) (ee);
   IFE;
}

/**
 * Activate (set focus to, via the window manager) an Ecore_Evas' window.
 * @param ee The Ecore_Evas to activate.
 *
 * This functions activates the Ecore_Evas.
 */
EAPI void
ecore_evas_activate(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_activate");
        return;
     }
   IFC(ee, fn_activate) (ee);
   IFE;
}

/**
 * Set the title of an Ecore_Evas' window
 * @param ee The Ecore_Evas whose title you wish to set.
 * @param t The title
 *
 * This function sets the title of @p ee to @p t.
 */
EAPI void
ecore_evas_title_set(Ecore_Evas *ee, const char *t)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_title_set");
        return;
     }
   IFC(ee, fn_title_set) (ee, t);
   IFE;
}

/**
 * Get the title of an Ecore_Evas' window
 * @param ee The Ecore_Evas whose title you wish to get.
 * @return The title of @p ee.
 *
 * This function returns the title of @p ee.
 */
EAPI const char *
ecore_evas_title_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_title_get");
        return NULL;
     }
   return ee->prop.title;
}

/**
 * Set the name and class of an Ecore_Evas' window
 * @param ee the Ecore_Evas
 * @param n the name
 * @param c the class
 *
 * This function sets the name of @p ee to @p n, and its class to @p c.
 */
EAPI void
ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_name_class_set");
        return;
     }
   IFC(ee, fn_name_class_set) (ee, n, c);
   IFE;
}

/**
 * Get the name and class of an Ecore_Evas' window
 * @p ee The Ecore_Evas to query
 * @p n A pointer to a string to place the name in.
 * @p c A pointer to a string to place the class in.
 *
 * This function gets puts the name of @p ee into @p n, and its class into
 * @p c.
 */
EAPI void
ecore_evas_name_class_get(const Ecore_Evas *ee, const char **n, const char **c)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_name_class_get");
        return;
     }
   if (n) *n = ee->prop.name;
   if (c) *c = ee->prop.clas;
}

EAPI void
ecore_evas_size_min_set(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_size_min_set");
        return;
     }
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        IFC(ee, fn_size_min_set) (ee, h, w);
        IFE;
     }
   else
     {
        IFC(ee, fn_size_min_set) (ee, w, h);
        IFE;
     }
}

EAPI void
ecore_evas_size_min_get(const Ecore_Evas *ee, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_size_min_get");
        return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        if (w) *w = ee->prop.min.h;
        if (h) *h = ee->prop.min.w;
     }
   else
     {
        if (w) *w = ee->prop.min.w;
        if (h) *h = ee->prop.min.h;
     }
}

EAPI void
ecore_evas_size_max_set(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_size_max_set");
        return;
     }
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        IFC(ee, fn_size_max_set) (ee, h, w);
        IFE;
     }
   else
     {
        IFC(ee, fn_size_max_set) (ee, w, h);
        IFE;
     }
}

EAPI void
ecore_evas_size_max_get(const Ecore_Evas *ee, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_size_max_get");
        return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        if (w) *w = ee->prop.max.h;
        if (h) *h = ee->prop.max.w;
     }
   else
     {
        if (w) *w = ee->prop.max.w;
        if (h) *h = ee->prop.max.h;
     }
}

EAPI void
ecore_evas_size_base_set(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_size_base_set");
        return;
     }
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        IFC(ee, fn_size_base_set) (ee, h, w);
        IFE;
     }
   else
     {
        IFC(ee, fn_size_base_set) (ee, w, h);
        IFE;
     }
}

EAPI void
ecore_evas_size_base_get(const Ecore_Evas *ee, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_size_base_get");
        return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        if (w) *w = ee->prop.base.h;
        if (h) *h = ee->prop.base.w;
     }
   else
     {
        if (w) *w = ee->prop.base.w;
        if (h) *h = ee->prop.base.h;
     }
}

EAPI void
ecore_evas_size_step_set(Ecore_Evas *ee, int w, int h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_size_step_set");
        return;
     }
   if (w < 0) w = 0;
   if (h < 0) h = 0;
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        IFC(ee, fn_size_step_set) (ee, h, w);
        IFE;
     }
   else
     {
        IFC(ee, fn_size_step_set) (ee, w, h);
        IFE;
     }
}

EAPI void
ecore_evas_size_step_get(const Ecore_Evas *ee, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_size_step_get");
        return;
     }
   if ((ee->rotation == 90) || (ee->rotation == 270))
     {
        if (w) *w = ee->prop.step.h;
        if (h) *h = ee->prop.step.w;
     }
   else
     {
        if (w) *w = ee->prop.step.w;
        if (h) *h = ee->prop.step.h;
     }
}

EAPI void
ecore_evas_cursor_set(Ecore_Evas *ee, const char *file, int layer, int hot_x, int hot_y)
{
   Evas_Object  *obj = NULL;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_cursor_set");
        return;
     }

   if (file)
     {
        int x, y;

        obj = evas_object_image_add(ee->evas);
        evas_object_image_file_set(obj, file, NULL);
        evas_object_image_size_get(obj, &x, &y);
        evas_object_resize(obj, x, y);
        evas_object_image_fill_set(obj, 0, 0, x, y);
     }

   IFC(ee, fn_object_cursor_set) (ee, obj, layer, hot_x, hot_y);
   IFE;
}

EAPI void
ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_cursor_set");
        return;
     }
   IFC(ee, fn_object_cursor_set) (ee, obj, layer, hot_x, hot_y);
   IFE;
}

EAPI void
ecore_evas_cursor_get(const Ecore_Evas *ee, Evas_Object **obj, int *layer, int *hot_x, int *hot_y)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_cursor_get");
        return;
     }
   if (obj) *obj = ee->prop.cursor.object;
   if (layer) *layer = ee->prop.cursor.layer;
   if (hot_x) *hot_x = ee->prop.cursor.hot.x;
   if (hot_y) *hot_y = ee->prop.cursor.hot.y;
}

/**
 * Set the layer of an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param layer The layer to put @p ee on.
 *
 * This function moves @p ee to the layer @p layer.
 */
EAPI void
ecore_evas_layer_set(Ecore_Evas *ee, int layer)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_layer_set");
        return;
     }
   IFC(ee, fn_layer_set) (ee, layer);
   IFE;
}

/**
 * Get the layer of an Ecore_Evas' window
 * @param ee The Ecore_Evas to set
 * @return the layer @p ee's window is on.
 *
 */
EAPI int
ecore_evas_layer_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_layer_get");
        return 0;
     }
   return ee->prop.layer;
}

/**
 * Set the focus of an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE for focus, EINA_FALSE to defocus.
 *
 * This function focuses @p ee if @p on is EINA_TRUE, or defocuses @p ee if @p on is EINA_FALSE.
 */
EAPI void
ecore_evas_focus_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_focus_set");
        return;
     }
   IFC(ee, fn_focus_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is focused or not
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee if focused, EINA_FALSE if not.
 *
 */
EAPI Eina_Bool
ecore_evas_focus_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_focus_get");
        return EINA_FALSE;
     }
   return ee->prop.focused ? EINA_TRUE : EINA_FALSE;
}

/**
 * Iconify or uniconify an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE to iconify, EINA_FALSE to uniconify.
 *
 * This function iconifies @p ee if @p on is EINA_TRUE, or uniconifies @p ee if @p on
 * is EINA_FALSE.
 */
EAPI void
ecore_evas_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_iconified_set");
        return;
     }
   IFC(ee, fn_iconified_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is iconified or not
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee is iconified, EINA_FALSE if not.
 *
 */
EAPI Eina_Bool
ecore_evas_iconified_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_iconified_get");
        return EINA_FALSE;
     }
   return ee->prop.iconified ? EINA_TRUE : EINA_FALSE;
}

/**
 * Set whether an Ecore_Evas' window is borderless or not
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE for borderless, EINA_FALSE for bordered.
 *
 * This function makes @p ee borderless if @p on is EINA_TRUE, or bordered if @p on
 * is EINA_FALSE.
 */
EAPI void
ecore_evas_borderless_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_borderless_set");
        return;
     }
   IFC(ee, fn_borderless_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is borderless or not
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee is borderless, EINA_FALSE if not.
 *
 */
EAPI Eina_Bool
ecore_evas_borderless_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_borderless_get");
        return EINA_FALSE;
     }
   return ee->prop.borderless ? EINA_TRUE : EINA_FALSE;
}

/**
 * Tell the WM whether or not to ignore an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE to ignore, EINA_FALSE to not.
 *
 * This function causes the window manager to ignore @p ee if @p on is EINA_TRUE,
 * or not ignore @p ee if @p on is EINA_FALSE.
 */
EAPI void
ecore_evas_override_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_override_set");
        return;
     }
   IFC(ee, fn_override_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is overridden or not
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee is overridden, EINA_FALSE if not.
 *
 */
EAPI Eina_Bool
ecore_evas_override_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_override_get");
        return EINA_FALSE;
     }
   return ee->prop.override ? EINA_TRUE : EINA_FALSE;
}

/**
 * Maximize (or unmaximize) an Ecore_Evas' window
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE to maximize, EINA_FALSE to unmaximize.
 *
 * This function maximizes @p ee if @p on is EINA_TRUE, or unmaximizes @p ee
 * if @p on is EINA_FALSE.
 */
EAPI void
ecore_evas_maximized_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_maximized_set");
        return;
     }
   IFC(ee, fn_maximized_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is maximized or not
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee is maximized, EINA_FALSE if not.
 *
 */
EAPI Eina_Bool
ecore_evas_maximized_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_maximized_get");
        return EINA_FALSE;
     }
   return ee->prop.maximized ? EINA_TRUE : EINA_FALSE;
}

/**
 * Set whether or not an Ecore_Evas' window is fullscreen
 * @param ee The Ecore_Evas
 * @param on EINA_TRUE fullscreen, EINA_FALSE not.
 *
 * This function causes @p ee to be fullscreen if @p on is EINA_TRUE,
 * or not if @p on is EINA_FALSE.
 */
EAPI void
ecore_evas_fullscreen_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_fullscreen_set");
        return;
     }
   IFC(ee, fn_fullscreen_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window is fullscreen or not
 * @param ee The Ecore_Evas to set
 * @return EINA_TRUE if @p ee is fullscreen, EINA_FALSE if not.
 *
 */
EAPI Eina_Bool
ecore_evas_fullscreen_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_fullscreen_get");
        return EINA_FALSE;
     }
   return ee->prop.fullscreen ? EINA_TRUE : EINA_FALSE;
}

/**
 * Set whether or not an Ecore_Evas' window should avoid damage
 *
 * @param ee The Ecore_Evas
 * @param The type of the damage management
 *
 * This function causes @p ee to be drawn to a pixmap to avoid recalculations.
 * On expose events it will copy from the pixmap to the window.
 */
EAPI void
ecore_evas_avoid_damage_set(Ecore_Evas *ee, Ecore_Evas_Avoid_Damage_Type on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_avoid_damage_set");
        return;
     }
   IFC(ee, fn_avoid_damage_set) (ee, on);
   IFE;
}

/**
 * Query whether an Ecore_Evas' window avoids damage or not
 * @param ee The Ecore_Evas to set
 * @return The type of the damage management
 *
 */
EAPI Ecore_Evas_Avoid_Damage_Type
ecore_evas_avoid_damage_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_avoid_damage_get");
        return ECORE_EVAS_AVOID_DAMAGE_NONE;
     }
   return ee->prop.avoid_damage;
}

/**
 * Set the withdrawn state of an Ecore_Evas' window.
 * @param ee The Ecore_Evas whose window's withdrawn state is set.
 * @param withdrawn The Ecore_Evas window's new withdrawn state.
 *
 */
EAPI void
ecore_evas_withdrawn_set(Ecore_Evas *ee, Eina_Bool withdrawn)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_withdrawn_set");
        return;
     }

   IFC(ee, fn_withdrawn_set) (ee, withdrawn);
   IFE;
}

/**
 * Returns the withdrawn state of an Ecore_Evas' window.
 * @param ee The Ecore_Evas whose window's withdrawn state is returned.
 * @return The Ecore_Evas window's withdrawn state.
 *
 */
EAPI Eina_Bool
ecore_evas_withdrawn_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_withdrawn_get");
        return EINA_FALSE;
     } else
     return ee->prop.withdrawn ? EINA_TRUE : EINA_FALSE;
}

/**
 * Set the sticky state of an Ecore_Evas window.
 *
 * @param ee The Ecore_Evas whose window's sticky state is set.
 * @param sticky The Ecore_Evas window's new sticky state.
 *
 */
EAPI void
ecore_evas_sticky_set(Ecore_Evas *ee, Eina_Bool sticky)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_sticky_set");
        return;
     }

   IFC(ee, fn_sticky_set) (ee, sticky);
   IFE;
}

/**
 * Returns the sticky state of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose window's sticky state is returned.
 * @return The Ecore_Evas window's sticky state.
 *
 */
EAPI Eina_Bool
ecore_evas_sticky_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_sticky_get");
        return EINA_FALSE;
     } else
     return ee->prop.sticky ? EINA_TRUE : EINA_FALSE;
}

/**
 * Set if this evas should ignore events
 *
 * @param ee The Ecore_Evas whose window's to ignore events.
 * @param ignore The Ecore_Evas new ignore state.
 *
 */
EAPI void
ecore_evas_ignore_events_set(Ecore_Evas *ee, Eina_Bool ignore)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_ignore_events_set");
        return;
     }

   IFC(ee, fn_ignore_events_set) (ee, ignore);
   IFE;
}

/**
 * Returns the ignore state of an Ecore_Evas' window.
 *
 * @param ee The Ecore_Evas whose window's ignore events state is returned.
 * @return The Ecore_Evas window's ignore state.
 *
 */
EAPI Eina_Bool
ecore_evas_ignore_events_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_ignore_events_get");
        return EINA_FALSE;
     }
   return ee->ignore_events ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_manual_render_set(Ecore_Evas *ee, Eina_Bool manual_render)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_manual_render_set");
        return;
     }
   ee->manual_render = manual_render;
}

EAPI Eina_Bool
ecore_evas_manual_render_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_manual_render_get");
        return EINA_FALSE;
     }
   return ee->manual_render ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_manual_render(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_manual_render");
        return;
     }
   if (ee->engine.func->fn_render)
     ee->engine.func->fn_render(ee);
}

EAPI void
ecore_evas_comp_sync_set(Ecore_Evas *ee, Eina_Bool do_sync)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_comp_sync_set");
        return;
     }
   ee->no_comp_sync = !do_sync;
}

EAPI Eina_Bool
ecore_evas_comp_sync_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_comp_sync_get");
        return EINA_FALSE;
     }
   return !ee->no_comp_sync;
}

EAPI Ecore_Window
ecore_evas_window_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
   {
      ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
         "ecore_evas_window_get");
      return 0;
   }

   return ee->prop.window;
}

/**
 * Get whole screen geometry associated with this Ecore_Evas.
 *
 * @param ee The Ecore_Evas whose window's to query container screen geometry.
 * @param x where to return the horizontal offset value. May be NULL.
 * @param y where to return the vertical offset value. May be NULL.
 * @param w where to return the width value. May be NULL.
 * @param h where to return the height value. May be NULL.
 *
 * @since 1.1
 */
EAPI void
ecore_evas_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_screen_geometry_get");
        return;
     }

   IFC(ee, fn_screen_geometry_get) (ee, x, y, w, h);
   IFE;
}

/* fps debug calls - for debugging how much time your app actually spends */
/* rendering graphics... :) */

static int _ecore_evas_fps_debug_init_count = 0;
static int _ecore_evas_fps_debug_fd = -1;
unsigned int *_ecore_evas_fps_rendertime_mmap = NULL;

void
_ecore_evas_fps_debug_init(void)
{
   char buf[4096];
   const char *tmp;

   _ecore_evas_fps_debug_init_count++;
   if (_ecore_evas_fps_debug_init_count > 1) return;

#ifndef HAVE_EVIL
   tmp = "/tmp";
#else
   tmp = evil_tmpdir_get ();
#endif /* HAVE_EVIL */
   snprintf(buf, sizeof(buf), "%s/.ecore_evas_fps_debug-%i", tmp, (int)getpid());
   _ecore_evas_fps_debug_fd = open(buf, O_CREAT | O_TRUNC | O_RDWR, 0644);
   if (_ecore_evas_fps_debug_fd < 0)
     {
        unlink(buf);
        _ecore_evas_fps_debug_fd = open(buf, O_CREAT | O_TRUNC | O_RDWR, 0644);
     }
   if (_ecore_evas_fps_debug_fd >= 0)
     {
        unsigned int zero = 0;
        char *buf = (char *)&zero;
        ssize_t todo = sizeof(unsigned int);

        while (todo > 0)
          {
             ssize_t r = write(_ecore_evas_fps_debug_fd, buf, todo);
             if (r > 0)
               {
                  todo -= r;
                  buf += r;
               }
             else if ((r < 0) && (errno == EINTR))
               continue;
             else
               {
                  ERR("could not write to file '%s' fd %d: %s",
                      buf, _ecore_evas_fps_debug_fd, strerror(errno));
                  close(_ecore_evas_fps_debug_fd);
                  _ecore_evas_fps_debug_fd = -1;
                  return;
               }
          }
        _ecore_evas_fps_rendertime_mmap = mmap(NULL, sizeof(unsigned int),
                                               PROT_READ | PROT_WRITE,
                                               MAP_SHARED,
                                               _ecore_evas_fps_debug_fd, 0);
        if (_ecore_evas_fps_rendertime_mmap == MAP_FAILED)
          _ecore_evas_fps_rendertime_mmap = NULL;
     }
}

void
_ecore_evas_fps_debug_shutdown(void)
{
   _ecore_evas_fps_debug_init_count--;
   if (_ecore_evas_fps_debug_init_count > 0) return;
   if (_ecore_evas_fps_debug_fd >= 0)
     {
        char buf[4096];

        snprintf(buf, sizeof(buf), "/tmp/.ecore_evas_fps_debug-%i", (int)getpid());
        unlink(buf);
        if (_ecore_evas_fps_rendertime_mmap)
          {
             munmap(_ecore_evas_fps_rendertime_mmap, sizeof(int));
             _ecore_evas_fps_rendertime_mmap = NULL;
          }
        close(_ecore_evas_fps_debug_fd);
        _ecore_evas_fps_debug_fd = -1;
     }
}

void
_ecore_evas_fps_debug_rendertime_add(double t)
{
   static double rtime = 0.0;
   static double rlapse = 0.0;
   static int frames = 0;
   static int flapse = 0;
   double tim;

   tim = ecore_time_get();
   rtime += t;
   frames++;
   if (rlapse == 0.0)
     {
        rlapse = tim;
        flapse = frames;
     }
   else if ((tim - rlapse) >= 0.5)
     {
        printf("FRAME: %i, FPS: %3.1f, RTIME %3.0f%%\n",
               frames,
               (frames - flapse) / (tim - rlapse),
               (100.0 * rtime) / (tim - rlapse)
               );
        rlapse = tim;
        flapse = frames;
        rtime = 0.0;
     }
}

void
_ecore_evas_register(Ecore_Evas *ee)
{
   ee->registered = 1;
   ecore_evases = (Ecore_Evas *)eina_inlist_prepend
     (EINA_INLIST_GET(ecore_evases), EINA_INLIST_GET(ee));
}

void
_ecore_evas_free(Ecore_Evas *ee)
{
   if (ee->func.fn_pre_free) ee->func.fn_pre_free(ee);
   while (ee->sub_ecore_evas)
     {
        _ecore_evas_free(ee->sub_ecore_evas->data);
     }
   if (ee->data) eina_hash_free(ee->data);
   ee->data = NULL;
   if (ee->name) free(ee->name);
   ee->name = NULL;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (ee->prop.name) free(ee->prop.name);
   ee->prop.name = NULL;
   if (ee->prop.clas) free(ee->prop.clas);
   ee->prop.clas = NULL;
   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);
   ee->prop.cursor.object = NULL;
   if (ee->evas) evas_free(ee->evas);
   ee->evas = NULL;
   ECORE_MAGIC_SET(ee, ECORE_MAGIC_NONE);
   ee->driver = NULL;
   if (ee->engine.idle_flush_timer)
     ecore_timer_del(ee->engine.idle_flush_timer);
   if (ee->engine.func->fn_free) ee->engine.func->fn_free(ee);
   if (ee->registered)
     {
        ecore_evases = (Ecore_Evas *)eina_inlist_remove
          (EINA_INLIST_GET(ecore_evases), EINA_INLIST_GET(ee));
     }
   free(ee);
}

static Eina_Bool
_ecore_evas_cb_idle_flush(void *data)
{
   Ecore_Evas *ee;

   ee = (Ecore_Evas *)data;
   evas_render_idle_flush(ee->evas);
   ee->engine.idle_flush_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_ecore_evas_async_events_fd_handler(void *data __UNUSED__, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   evas_async_events_process();

   return ECORE_CALLBACK_RENEW;
}

void
_ecore_evas_idle_timeout_update(Ecore_Evas *ee)
{
   if (ee->engine.idle_flush_timer)
     ecore_timer_del(ee->engine.idle_flush_timer);
   ee->engine.idle_flush_timer = ecore_timer_add(IDLE_FLUSH_TIME,
                                                 _ecore_evas_cb_idle_flush,
                                                 ee);
}

void
_ecore_evas_mouse_move_process(Ecore_Evas *ee, int x, int y, unsigned int timestamp)
{
   ee->mouse.x = x;
   ee->mouse.y = y;
   if (ee->prop.cursor.object)
     {
        evas_object_show(ee->prop.cursor.object);
        if (ee->rotation == 0)
          evas_object_move(ee->prop.cursor.object,
                           x - ee->prop.cursor.hot.x,
                           y - ee->prop.cursor.hot.y);
        else if (ee->rotation == 90)
          evas_object_move(ee->prop.cursor.object,
                           ee->h - y - 1 - ee->prop.cursor.hot.x,
                           x - ee->prop.cursor.hot.y);
        else if (ee->rotation == 180)
          evas_object_move(ee->prop.cursor.object,
                           ee->w - x - 1 - ee->prop.cursor.hot.x,
                           ee->h - y - 1 - ee->prop.cursor.hot.y);
        else if (ee->rotation == 270)
          evas_object_move(ee->prop.cursor.object,
                           y - ee->prop.cursor.hot.x,
                           ee->w - x - 1 - ee->prop.cursor.hot.y);
     }
   if (ee->rotation == 0)
     evas_event_feed_mouse_move(ee->evas, x, y, timestamp, NULL);
   else if (ee->rotation == 90)
     evas_event_feed_mouse_move(ee->evas, ee->h - y - 1, x, timestamp, NULL);
   else if (ee->rotation == 180)
     evas_event_feed_mouse_move(ee->evas, ee->w - x - 1, ee->h - y - 1, timestamp, NULL);
   else if (ee->rotation == 270)
     evas_event_feed_mouse_move(ee->evas, y, ee->w - x - 1, timestamp, NULL);
}

void
_ecore_evas_mouse_multi_move_process(Ecore_Evas *ee, int device,
                                     int x, int y,
                                     double radius,
                                     double radius_x, double radius_y,
                                     double pressure,
                                     double angle,
                                     double mx, double my,
                                     unsigned int timestamp)
{
   if (ee->rotation == 0)
      evas_event_feed_multi_move(ee->evas, device,
                                 x, y,
                                 radius,
                                 radius_x, radius_y,
                                 pressure,
                                 angle - ee->rotation,
                                 mx, my,
                                 timestamp, NULL);
   else if (ee->rotation == 90)
      evas_event_feed_multi_move(ee->evas, device,
                                 ee->h - y - 1, x,
                                 radius,
                                 radius_y, radius_x,
                                 pressure,
                                 angle - ee->rotation,
                                 ee->h - my - 1, mx,
                                 timestamp, NULL);
   else if (ee->rotation == 180)
      evas_event_feed_multi_move(ee->evas, device,
                                 ee->w - x - 1, ee->h - y - 1,
                                 radius,
                                 radius_x, radius_y,
                                 pressure,
                                 angle - ee->rotation,
                                 ee->w - mx - 1, ee->h - my - 1,
                                 timestamp, NULL);
   else if (ee->rotation == 270)
      evas_event_feed_multi_move(ee->evas, device,
                                 y, ee->w - x - 1,
                                 radius,
                                 radius_y, radius_x,
                                 pressure,
                                 angle - ee->rotation,
                                 my, ee->w - mx - 1,
                                 timestamp, NULL);
}

void
_ecore_evas_mouse_multi_down_process(Ecore_Evas *ee, int device,
                                     int x, int y,
                                     double radius,
                                     double radius_x, double radius_y,
                                     double pressure,
                                     double angle,
                                     double mx, double my,
                                     Evas_Button_Flags flags,
                                     unsigned int timestamp)
{
   if (ee->rotation == 0)
      evas_event_feed_multi_down(ee->evas, device,
                                 x, y,
                                 radius,
                                 radius_x, radius_y,
                                 pressure,
                                 angle - ee->rotation,
                                 mx, my,
                                 flags, timestamp, NULL);
   else if (ee->rotation == 90)
      evas_event_feed_multi_down(ee->evas, device,
                                 ee->h - y - 1, x,
                                 radius,
                                 radius_y, radius_x,
                                 pressure,
                                 angle - ee->rotation,
                                 ee->h - my - 1, mx,
                                 flags, timestamp, NULL);
   else if (ee->rotation == 180)
      evas_event_feed_multi_down(ee->evas, device,
                                 ee->w - x - 1, ee->h - y - 1,
                                 radius,
                                 radius_x, radius_y,
                                 pressure,
                                 angle - ee->rotation,
                                 ee->w - mx - 1, ee->h - my - 1,
                                 flags, timestamp, NULL);
   else if (ee->rotation == 270)
      evas_event_feed_multi_down(ee->evas, device,
                                 y, ee->w - x - 1,
                                 radius,
                                 radius_y, radius_x,
                                 pressure,
                                 angle - ee->rotation,
                                 my, ee->w - mx - 1,
                                 flags, timestamp, NULL);
}

void
_ecore_evas_mouse_multi_up_process(Ecore_Evas *ee, int device,
                                   int x, int y,
                                   double radius,
                                   double radius_x, double radius_y,
                                   double pressure,
                                   double angle,
                                   double mx, double my,
                                   Evas_Button_Flags flags,
                                   unsigned int timestamp)
{
   if (ee->rotation == 0)
      evas_event_feed_multi_up(ee->evas, device,
                               x, y,
                               radius,
                               radius_x, radius_y,
                               pressure,
                               angle - ee->rotation,
                               mx, my,
                               flags, timestamp, NULL);
   else if (ee->rotation == 90)
      evas_event_feed_multi_up(ee->evas, device,
                               ee->h - y - 1, x,
                               radius,
                               radius_y, radius_x,
                               pressure,
                               angle - ee->rotation,
                               ee->h - my - 1, mx,
                               flags, timestamp, NULL);
   else if (ee->rotation == 180)
      evas_event_feed_multi_up(ee->evas, device,
                               ee->w - x - 1, ee->h - y - 1,
                               radius,
                               radius_x, radius_y,
                               pressure,
                               angle - ee->rotation,
                               ee->w - mx - 1, ee->h - my - 1,
                               flags, timestamp, NULL);
   else if (ee->rotation == 270)
      evas_event_feed_multi_up(ee->evas, device,
                               y, ee->w - x - 1,
                               radius,
                               radius_y, radius_x,
                               pressure,
                               angle - ee->rotation,
                               my, ee->w - mx - 1,
                               flags, timestamp, NULL);
}

/**
 * Get a list of all the ecore_evases.
 *
 * The returned list of ecore evases is only valid until the canvases are
 * destroyed (and should not be cached for instance).
 * The list can be free by just deleting the list.
 *
 * @return A list of ecore_evases.
 */
EAPI Eina_List *
ecore_evas_ecore_evas_list_get(void)
{
   Ecore_Evas *ee;
   Eina_List *l = NULL;

   EINA_INLIST_FOREACH(ecore_evases, ee)
     {
        l = eina_list_append(l, ee);
     }

   return l;
}
