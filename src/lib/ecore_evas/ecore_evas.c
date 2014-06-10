#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef _MSC_VER
# include <unistd.h>
#endif

#if defined(HAVE_SYS_MMAN_H) || defined(HAVE_EVIL)
# include <sys/mman.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"
#include "ecore_evas_x11.h"
#include "ecore_evas_wayland.h"
#include "ecore_evas_extn.h"
#include "ecore_evas_win32.h"

EAPI Eina_Bool _ecore_evas_app_comp_sync = EINA_FALSE;
EAPI int _ecore_evas_log_dom = -1;
static int _ecore_evas_init_count = 0;
static Ecore_Fd_Handler *_ecore_evas_async_events_fd = NULL;
static Eina_Bool _ecore_evas_async_events_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);

static Ecore_Idle_Enterer *ecore_evas_idle_enterer = NULL;
static Ecore_Evas *ecore_evases = NULL;
static int _ecore_evas_fps_debug = 0;

#define RENDER_SYNC 1

#ifdef RENDER_SYNC
static Ecore_Animator *ecore_evas_animator = NULL;
static Eina_Bool ecore_evas_animator_ticked = EINA_FALSE;
static Eina_Bool ecore_evas_first = EINA_TRUE;
static int overtick = 0;

static Eina_Bool
_ecore_evas_animator(void *data EINA_UNUSED)
{
   ecore_evas_animator_ticked = EINA_TRUE;
   overtick--;
   if (overtick == 0)
     {
        ecore_evas_animator = NULL;
        return EINA_FALSE;
     }
   return EINA_TRUE;
}
#endif

static Eina_Bool
_ecore_evas_idle_enter(void *data EINA_UNUSED)
{
   Ecore_Evas *ee;
   double t1 = 0.0;
   double t2 = 0.0;
   int rend = 0;
#ifdef ECORE_EVAS_ASYNC_RENDER_DEBUG
   double now = ecore_loop_time_get();
#endif

   if (!ecore_evases) return ECORE_CALLBACK_RENEW;

#ifdef RENDER_SYNC
   if (!ecore_evas_first)
     {
        if ((!ecore_evas_animator_ticked) &&
            (!ecore_main_loop_animator_ticked_get()))
          {
             if (!ecore_evas_animator)
               {
                  overtick = 1;
                  ecore_evas_animator = ecore_animator_add(_ecore_evas_animator, NULL);
               }
             return ECORE_CALLBACK_RENEW;
          }
        ecore_evas_animator_ticked = EINA_FALSE;
     }
   ecore_evas_first = EINA_FALSE;
#endif

   if (_ecore_evas_fps_debug)
     {
        t1 = ecore_time_get();
     }
   EINA_INLIST_FOREACH(ecore_evases, ee)
     {
#ifdef ECORE_EVAS_ASYNC_RENDER_DEBUG
        if ((ee->in_async_render) && (now - ee->async_render_start > 2.0))
          {
             ERR("stuck async render: time=%f, ee=%p, engine=%s, geometry=(%d, %d, %d, %d), visible=%hhu, shaped=%hhu, alpha=%hhu, transparent=%hhu",
                 now - ee->async_render_start, ee, ee->driver, ee->x, ee->y, ee->w, ee->h, ee->visible, ee->shaped, ee->alpha, ee->transparent);

             ERR("delayed.avoid_damage=%hhu", ee->delayed.avoid_damage);
             ERR("delayed.resize_shape=%hhu", ee->delayed.resize_shape);
             ERR("delayed.shaped=%hhu", ee->delayed.shaped);
             ERR("delayed.shaped_changed=%hhu", ee->delayed.shaped_changed);
             ERR("delayed.alpha=%hhu", ee->delayed.alpha);
             ERR("delayed.alpha_changed=%hhu", ee->delayed.alpha_changed);
             ERR("delayed.transparent=%hhu", ee->delayed.transparent);
             ERR("delayed.transparent_changed=%hhu", ee->delayed.transparent_changed);
             ERR("delayed.rotation=%d", ee->delayed.rotation);
             ERR("delayed.rotation_resize=%d", ee->delayed.rotation_resize);
             ERR("delayed.rotation_changed=%d", ee->delayed.rotation_changed);

             ERR("reset in_async_render of ee=%p", ee);
             ee->in_async_render = EINA_FALSE;
             ee->async_render_start = 0.0;

          }
        else if ((!ee->in_async_render) && (ee->async_render_start > 0.0))
          {
             DBG("--- async render %f ee=%p [%s] (%d, %d, %d, %d) visible=%hhu shaped=%hhu alpha=%hhu transparent=%hhu",
                 now, ee, ee->driver, ee->x, ee->y, ee->w, ee->h, ee->visible, ee->shaped, ee->alpha, ee->transparent);
             ee->async_render_start = 0.0;
          }
#endif

        if (!ee->manual_render)
          {
             if (ee->engine.func->fn_render)
               rend |= ee->engine.func->fn_render(ee);
          }
#ifdef ECORE_EVAS_ASYNC_RENDER_DEBUG
        if ((ee->in_async_render) && (ee->async_render_start <= 0.0))
          {
             DBG("+++ async render %f ee=%p [%s] (%d, %d, %d, %d) visible=%hhu shaped=%hhu alpha=%hhu transparent=%hhu",
                 now, ee, ee->driver, ee->x, ee->y, ee->w, ee->h, ee->visible, ee->shaped, ee->alpha, ee->transparent);
             ee->async_render_start = now;
          }
#endif
     }
   if (_ecore_evas_fps_debug)
     {
        t2 = ecore_time_get();
        if (rend)
          _ecore_evas_fps_debug_rendertime_add(t2 - t1);
     }
   return ECORE_CALLBACK_RENEW;
}

EAPI Ecore_Evas_Interface *
_ecore_evas_interface_get(const Ecore_Evas *ee, const char *iname)
{
   Eina_List *l;
   Ecore_Evas_Interface *i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ee, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iname, NULL);

   EINA_LIST_FOREACH(ee->engine.ifaces, l, i)
     {
	if (!strcmp(i->name, iname))
	  return i;
     }

   CRI("Ecore_Evas %p (engine: %s) does not have interface '%s'",
        ee, ee->driver, iname);

   return NULL;
}

/**
 * Query if a particular rendering engine target has support
 * @param  engine The engine to check support for
 * @return 1 if the particular engine is supported, 0 if it is not
 *
 * Query if engine @param engine is supported by ecore_evas. 1 is returned if
 * it is, and 0 is returned if it is not supported.
 */
EAPI int
ecore_evas_engine_type_supported_get(Ecore_Evas_Engine_Type engine)
{
   /* It should be done reading the availables engines */

   switch (engine)
     {
      case ECORE_EVAS_ENGINE_SOFTWARE_BUFFER:
        return EINA_TRUE;

      case ECORE_EVAS_ENGINE_SOFTWARE_XLIB:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_XLIB
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
      case ECORE_EVAS_ENGINE_XRENDER_X11:
        return EINA_FALSE;
      case ECORE_EVAS_ENGINE_OPENGL_X11:
#ifdef BUILD_ECORE_EVAS_OPENGL_X11
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
      case ECORE_EVAS_ENGINE_SOFTWARE_XCB:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_XCB
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
      case ECORE_EVAS_ENGINE_XRENDER_XCB:
        return EINA_FALSE;
      case ECORE_EVAS_ENGINE_SOFTWARE_GDI:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
      case ECORE_EVAS_ENGINE_SOFTWARE_DDRAW:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
     case ECORE_EVAS_ENGINE_SOFTWARE_SDL:
#ifdef BUILD_ECORE_EVAS_SOFTWARE_SDL
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
     case ECORE_EVAS_ENGINE_OPENGL_SDL:
#ifdef BUILD_ECORE_EVAS_OPENGL_SDL
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
      case ECORE_EVAS_ENGINE_DIRECTFB:
        return EINA_FALSE;
      case ECORE_EVAS_ENGINE_SOFTWARE_FB:
#ifdef BUILD_ECORE_EVAS_FB
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif

      case ECORE_EVAS_ENGINE_SOFTWARE_8_X11:
        return EINA_FALSE;
      case ECORE_EVAS_ENGINE_SOFTWARE_16_X11:
        return EINA_FALSE;
      case ECORE_EVAS_ENGINE_SOFTWARE_16_DDRAW:
        return EINA_FALSE;
      case ECORE_EVAS_ENGINE_SOFTWARE_16_WINCE:
        return EINA_FALSE;
      case ECORE_EVAS_ENGINE_DIRECT3D:
        return EINA_FALSE;
      case ECORE_EVAS_ENGINE_OPENGL_GLEW:
        return EINA_FALSE;

      case ECORE_EVAS_ENGINE_OPENGL_COCOA:
#ifdef BUILD_ECORE_EVAS_OPENGL_COCOA
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
      case ECORE_EVAS_ENGINE_EWS:
#ifdef BUILD_ECORE_EVAS_EWS
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
     case ECORE_EVAS_ENGINE_PSL1GHT:
#ifdef BUILD_ECORE_EVAS_PSL1GHT
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
     case ECORE_EVAS_ENGINE_WAYLAND_SHM:
#ifdef BUILD_ECORE_EVAS_WAYLAND_SHM
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
     case ECORE_EVAS_ENGINE_WAYLAND_EGL:
#ifdef BUILD_ECORE_EVAS_WAYLAND_EGL
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif
     case ECORE_EVAS_ENGINE_DRM:
#ifdef BUILD_ECORE_EVAS_DRM
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif

      default:
        return EINA_FALSE;
     };
}

static void
_ecore_evas_fork_cb(void *data EINA_UNUSED)
{
   int fd;
   
   if (_ecore_evas_async_events_fd)
     ecore_main_fd_handler_del(_ecore_evas_async_events_fd);
   fd = evas_async_events_fd_get();
   if (fd >= 0)
     _ecore_evas_async_events_fd = 
     ecore_main_fd_handler_add(fd, ECORE_FD_READ,
                               _ecore_evas_async_events_fd_handler, NULL,
                               NULL, NULL);
}

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

   ecore_fork_reset_callback_add(_ecore_evas_fork_cb, NULL);
   fd = evas_async_events_fd_get();
   if (fd >= 0)
     _ecore_evas_async_events_fd = 
     ecore_main_fd_handler_add(fd, ECORE_FD_READ,
                               _ecore_evas_async_events_fd_handler, NULL,
                               NULL, NULL);

   ecore_evas_idle_enterer =
     ecore_idle_enterer_add(_ecore_evas_idle_enter, NULL);
   if (getenv("ECORE_EVAS_FPS_DEBUG")) _ecore_evas_fps_debug = 1;
   if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_init();

#ifdef BUILD_ECORE_EVAS_EWS
   _ecore_evas_ews_events_init();
#endif

   _ecore_evas_extn_init();

   _ecore_evas_engine_init();

   eina_log_timing(_ecore_evas_log_dom,
		   EINA_LOG_STATE_STOP,
		   EINA_LOG_STATE_INIT);

   if (getenv("ECORE_EVAS_COMP_NOSYNC"))
     _ecore_evas_app_comp_sync = EINA_FALSE;
   else if (getenv("ECORE_EVAS_COMP_SYNC"))
     _ecore_evas_app_comp_sync = EINA_TRUE;
   return _ecore_evas_init_count;

 shutdown_ecore:
   ecore_shutdown();
 shutdown_evas:
   evas_shutdown();

   return --_ecore_evas_init_count;
}

EAPI int
ecore_evas_shutdown(void)
{
   if (--_ecore_evas_init_count != 0)
     return _ecore_evas_init_count;

   eina_log_timing(_ecore_evas_log_dom,
		   EINA_LOG_STATE_START,
		   EINA_LOG_STATE_SHUTDOWN);

   while (ecore_evases) _ecore_evas_free(ecore_evases);

   if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_shutdown();
   ecore_idle_enterer_del(ecore_evas_idle_enterer);
   ecore_evas_idle_enterer = NULL;
#ifdef RENDER_SYNC
   if (ecore_evas_animator) ecore_animator_del(ecore_evas_animator);
   ecore_evas_animator = NULL;
#endif

   _ecore_evas_extn_shutdown();

#ifdef BUILD_ECORE_EVAS_EWS
   while (_ecore_evas_ews_shutdown());
#endif
   _ecore_evas_engine_shutdown();
   if (_ecore_evas_async_events_fd)
     ecore_main_fd_handler_del(_ecore_evas_async_events_fd);
   
   ecore_fork_reset_callback_del(_ecore_evas_fork_cb, NULL);

   eina_log_domain_unregister(_ecore_evas_log_dom);
   _ecore_evas_log_dom = -1;
   ecore_shutdown();
   evas_shutdown();

   return _ecore_evas_init_count;
}

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

static Ecore_Evas *
_ecore_evas_constructor_cocoa(int x, int y, int w, int h, const char *extra_options)
{
   char *name = NULL;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_str(extra_options, "name=", &name);
   ee = ecore_evas_cocoa_new(NULL, x, y, w, h);
   free(name);

   if (ee) ecore_evas_move(ee, x, y);
   return ee;
}

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

static Ecore_Evas *
_ecore_evas_constructor_sdl(int x EINA_UNUSED, int y EINA_UNUSED, int w, int h, const char *extra_options)
{
   Ecore_Evas *ee;
   unsigned int fullscreen = 0, hwsurface = 0, noframe = 0, alpha = 0;
   char *name = NULL;

   _ecore_evas_parse_extra_options_str(extra_options, "name=", &name);
   _ecore_evas_parse_extra_options_uint(extra_options, "fullscreen=", &fullscreen);
   _ecore_evas_parse_extra_options_uint(extra_options, "hwsurface=", &hwsurface);
   _ecore_evas_parse_extra_options_uint(extra_options, "noframe=", &noframe);
   _ecore_evas_parse_extra_options_uint(extra_options, "alpha=", &alpha);

   ee = ecore_evas_sdl_new(name, w, h, fullscreen, hwsurface, noframe, alpha);
   free(name);

   return ee;
}
static Ecore_Evas *
_ecore_evas_constructor_opengl_sdl(int x EINA_UNUSED, int y EINA_UNUSED, int w, int h, const char *extra_options)
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

static Ecore_Evas *
_ecore_evas_constructor_fb(int x EINA_UNUSED, int y EINA_UNUSED, int w, int h, const char *extra_options)
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

static Ecore_Evas *
_ecore_evas_constructor_psl1ght(int x EINA_UNUSED, int y EINA_UNUSED, int w, int h, const char *extra_options)
{
   Ecore_Evas *ee;
   char *name = NULL;

   _ecore_evas_parse_extra_options_str(extra_options, "name=", &name);
   ee = ecore_evas_psl1ght_new(name, w, h);
   free(name);

   if (ee) ecore_evas_move(ee, x, y);
   return ee;
}

static Ecore_Evas *
_ecore_evas_constructor_wayland_shm(int x, int y, int w, int h, const char *extra_options)
{
   char *disp_name = NULL;
   unsigned int frame = 1, parent = 0;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_str(extra_options, "display=", &disp_name);
   _ecore_evas_parse_extra_options_uint(extra_options, "frame=", &frame);
   _ecore_evas_parse_extra_options_uint(extra_options, "parent=", &parent);
   ee = ecore_evas_wayland_shm_new(disp_name, parent, x, y, w, h, frame);
   free(disp_name);

   return ee;
}

static Ecore_Evas *
_ecore_evas_constructor_wayland_egl(int x, int y, int w, int h, const char *extra_options)
{
   char *disp_name = NULL;
   unsigned int frame = 1, parent = 0;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_str(extra_options, "display=", &disp_name);
   _ecore_evas_parse_extra_options_uint(extra_options, "frame=", &frame);
   _ecore_evas_parse_extra_options_uint(extra_options, "parent=", &parent);
   ee = ecore_evas_wayland_egl_new(disp_name, parent, x, y, w, h, frame);
   free(disp_name);

   return ee;
}

static Ecore_Evas *
_ecore_evas_constructor_drm(int x, int y, int w, int h, const char *extra_options)
{
   char *device = NULL;
   unsigned int parent = 0;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_str(extra_options, "device=", &device);
   _ecore_evas_parse_extra_options_uint(extra_options, "parent=", &parent);
   ee = ecore_evas_drm_new(device, parent, x, y, w, h);
   free(device);

   return ee;
}

static Ecore_Evas *
_ecore_evas_constructor_software_gdi(int x, int y, int w, int h,
				     const char *extra_options EINA_UNUSED)
{
   return ecore_evas_software_gdi_new(NULL, x, y, w, h);
}

static Ecore_Evas *
_ecore_evas_constructor_software_ddraw(int x, int y, int w, int h,
				       const char *extra_options EINA_UNUSED)
{
   return ecore_evas_software_ddraw_new(NULL, x, y, w, h);
}

static Ecore_Evas *
_ecore_evas_constructor_direct3d(int x, int y, int w, int h,
				 const char *extra_options EINA_UNUSED)
{
   return ecore_evas_direct3d_new(NULL, x, y, w, h);
}

static Ecore_Evas *
_ecore_evas_constructor_opengl_glew(int x, int y, int w, int h,
				    const char *extra_options EINA_UNUSED)
{
   return ecore_evas_gl_glew_new(NULL, x, y, w, h);
}

static Ecore_Evas *
_ecore_evas_constructor_buffer(int x EINA_UNUSED, int y EINA_UNUSED, int w, int h, const char *extra_options EINA_UNUSED)
{
   return ecore_evas_buffer_new(w, h);
}

#ifdef BUILD_ECORE_EVAS_EWS
static Ecore_Evas *
_ecore_evas_constructor_ews(int x, int y, int w, int h, const char *extra_options EINA_UNUSED)
{
   return ecore_evas_ews_new(x, y, w, h);
}
#endif

/* note: keep sorted by priority, highest first */
static const struct ecore_evas_engine _engines[] = {
  /* unix */
  {"software_x11", _ecore_evas_constructor_software_x11},
  {"opengl_x11", _ecore_evas_constructor_opengl_x11},
  {"fb", _ecore_evas_constructor_fb},
  {"software_gdi", _ecore_evas_constructor_software_gdi},
  {"software_ddraw", _ecore_evas_constructor_software_ddraw},
  {"direct3d", _ecore_evas_constructor_direct3d},
  {"opengl_glew", _ecore_evas_constructor_opengl_glew},
  {"opengl_cocoa", _ecore_evas_constructor_cocoa},
  {"psl1ght", _ecore_evas_constructor_psl1ght},
  {"wayland_shm", _ecore_evas_constructor_wayland_shm},
  {"wayland_egl", _ecore_evas_constructor_wayland_egl},
  {"drm", _ecore_evas_constructor_drm},
  {"opengl_sdl", _ecore_evas_constructor_opengl_sdl},
  {"sdl", _ecore_evas_constructor_sdl},
  {"buffer", _ecore_evas_constructor_buffer},
#ifdef BUILD_ECORE_EVAS_EWS
  {"ews", _ecore_evas_constructor_ews},
#endif
  {NULL, NULL}
};

EAPI Eina_List *
ecore_evas_engines_get(void)
{
   return eina_list_clone(_ecore_evas_available_engines_get());
}

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

EAPI const char *
ecore_evas_engine_name_get(const Ecore_Evas *ee)
{
   if (!ee)
     return NULL;
   return ee->driver;
}

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

EAPI void
ecore_evas_free(Ecore_Evas *ee)
{
   if (!ee) return;
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_free");
        return;
     }
   _ecore_evas_free(ee);
   return;
}

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

EAPI Evas *
ecore_evas_object_evas_get(Evas_Object *obj)
{
   Ecore_Evas *ee;

   ee = evas_object_data_get(obj, "Ecore_Evas");
   if (!ee) return NULL;

   return ecore_evas_get(ee);
}

EAPI Ecore_Evas *
ecore_evas_object_ecore_evas_get(Evas_Object *obj)
{
   return evas_object_data_get(obj, "Ecore_Evas");
}

#define IFC(_ee, _fn)  if (_ee->engine.func->_fn) {_ee->engine.func->_fn
#define IFE            return;}

EAPI void
ecore_evas_callback_resize_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_move_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_show_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_hide_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_destroy_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_focus_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_focus_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_sticky_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_unsticky_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_pre_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_post_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
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
ecore_evas_callback_pre_free_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_pre_free_set");
        return;
     }
   ee->func.fn_pre_free = func;
}

EAPI void
ecore_evas_callback_state_change_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_callback_state_change_set");
        return;
     }
   ee->func.fn_state_change = func;
}

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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        IFC(ee, fn_resize) (ee, w, h);
        IFE;
     }
   else
     {
        IFC(ee, fn_resize) (ee, h, w);
        IFE;
     }
}

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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        IFC(ee, fn_move_resize) (ee, x, y, w, h);
        IFE;
     }
   else
     {
        IFC(ee, fn_move_resize) (ee, x, y, h, w);
        IFE;
     }
}

EAPI void
ecore_evas_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_geometry_get");
        return;
     }
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        if (x) *x = ee->x;
        if (y) *y = ee->y;
        if (w) *w = ee->w;
        if (h) *h = ee->h;
     }
   else
     {
        if (x) *x = ee->x;
        if (y) *y = ee->y;
        if (w) *w = ee->h;
        if (h) *h = ee->w;
     }
}

EAPI void
ecore_evas_request_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_request_geometry_get");
        return;
     }
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        if (x) *x = ee->req.x;
        if (y) *y = ee->req.y;
        if (w) *w = ee->req.w;
        if (h) *h = ee->req.h;
     }
   else
     {
        if (x) *x = ee->req.x;
        if (y) *y = ee->req.y;
        if (w) *w = ee->req.h;
        if (h) *h = ee->req.w;
     }
}

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
   IFC(ee, fn_rotation_set) (ee, rot, 0);
   /* make sure everything gets redrawn */
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
   IFE;
}

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
   IFC(ee, fn_rotation_set) (ee, rot, 1);
   /* make sure everything gets redrawn */
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
   IFE;
}

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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        IFC(ee, fn_size_min_set) (ee, w, h);
        IFE;
     }
   else
     {
        IFC(ee, fn_size_min_set) (ee, h, w);
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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        if (w) *w = ee->prop.min.w;
        if (h) *h = ee->prop.min.h;
     }
   else
     {
        if (w) *w = ee->prop.min.h;
        if (h) *h = ee->prop.min.w;
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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        IFC(ee, fn_size_max_set) (ee, w, h);
        IFE;
     }
   else
     {
        IFC(ee, fn_size_max_set) (ee, h, w);
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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        if (w) *w = ee->prop.max.w;
        if (h) *h = ee->prop.max.h;
     }
   else
     {
        if (w) *w = ee->prop.max.h;
        if (h) *h = ee->prop.max.w;
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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        IFC(ee, fn_size_base_set) (ee, w, h);
        IFE;
     }
   else
     {
        IFC(ee, fn_size_base_set) (ee, h, w);
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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        if (w) *w = ee->prop.base.w;
        if (h) *h = ee->prop.base.h;
     }
   else
     {
        if (w) *w = ee->prop.base.h;
        if (h) *h = ee->prop.base.w;
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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        IFC(ee, fn_size_step_set) (ee, w, h);
        IFE;
     }
   else
     {
        IFC(ee, fn_size_step_set) (ee, h, w);
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
   if (ECORE_EVAS_PORTRAIT(ee))
     {
        if (w) *w = ee->prop.step.w;
        if (h) *h = ee->prop.step.h;
     }
   else
     {
        if (w) *w = ee->prop.step.h;
        if (h) *h = ee->prop.step.w;
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

EAPI Evas_Object *
ecore_evas_cursor_unset(Ecore_Evas *ee)
{
   Evas_Object *obj;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_cursor_unset");
        return NULL;
     }
   obj = ee->prop.cursor.object;
   evas_object_hide(obj);
   ee->prop.cursor.object = NULL;

   return obj;
}

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

EAPI Eina_Bool
ecore_evas_window_profile_supported_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_window_profile_supported_get");
        return EINA_FALSE;
     }
   return ee->profile_supported ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_window_profile_set(Ecore_Evas *ee, const char *profile)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_window_profile_set");
        return;
     }
   IFC(ee, fn_profile_set) (ee, profile);
   IFE;
}

EAPI const char *
ecore_evas_window_profile_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_window_profile_get");
        return NULL;
     }
   return ee->prop.profile.name;
}

EAPI void
ecore_evas_window_available_profiles_set(Ecore_Evas *ee, const char **profiles, const unsigned int count)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_window_available_profiles_set");
        return;
     }
   IFC(ee, fn_profiles_set) (ee, profiles, count);
   IFE;
}

EAPI Eina_Bool
ecore_evas_window_available_profiles_get(Ecore_Evas *ee, char ***profiles, unsigned int *count)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_window_available_profiles_get");
        return EINA_FALSE;
     }

   if ((ee->prop.profile.available_list) &&
       (ee->prop.profile.count >= 1))
     {
        if (profiles) *profiles = ee->prop.profile.available_list;
        if (count) *count = ee->prop.profile.count;
        return EINA_TRUE;
     }
   else
     return EINA_FALSE;
}

EAPI Eina_Bool
ecore_evas_wm_rotation_supported_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_wm_rotation_supported_get");
        return EINA_FALSE;
     }
   return ee->prop.wm_rot.supported;
}

EAPI void
ecore_evas_wm_rotation_preferred_rotation_set(Ecore_Evas *ee, int rotation)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_wm_rotation_preferred_rotation_set");
        return;
     }
   if (rotation != -1)
     {
        if (ee->prop.wm_rot.available_rots)
          {
             Eina_Bool found = EINA_FALSE;
             unsigned int i;
             for (i = 0; i < ee->prop.wm_rot.count; i++)
               {
                  if (ee->prop.wm_rot.available_rots[i] == rotation)
                    {
                       found = EINA_TRUE;
                       break;
                    }
               }
             if (!found) return;
          }
     }
   IFC(ee, fn_wm_rot_preferred_rotation_set) (ee, rotation);
   IFE;
}

EAPI int
ecore_evas_wm_rotation_preferred_rotation_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_wm_rotation_preferred_rotation_get");
        return -1;
     }
   return ee->prop.wm_rot.preferred_rot;
}

EAPI void
ecore_evas_wm_rotation_available_rotations_set(Ecore_Evas *ee, const int *rotations, unsigned int count)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_wm_rotation_available_rotations_set");
        return;
     }
   IFC(ee, fn_wm_rot_available_rotations_set) (ee, rotations, count);
   IFE;
}

EAPI Eina_Bool
ecore_evas_wm_rotation_available_rotations_get(const Ecore_Evas *ee, int **rotations, unsigned int *count)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_wm_rotation_available_rotations_get");
        return EINA_FALSE;
     }
   if ((!rotations) || (!count))
     return EINA_FALSE;

   if ((!ee->prop.wm_rot.available_rots) || (ee->prop.wm_rot.count == 0))
     return EINA_FALSE;

   *rotations = calloc(ee->prop.wm_rot.count, sizeof(int));
   if (!*rotations) return EINA_FALSE;

   memcpy(*rotations, ee->prop.wm_rot.available_rots, sizeof(int) * ee->prop.wm_rot.count);
   *count = ee->prop.wm_rot.count;

   return EINA_TRUE;
}

EAPI void
ecore_evas_wm_rotation_manual_rotation_done_set(Ecore_Evas *ee, Eina_Bool set)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_wm_rotation_manual_rotation_done_set");
        return;
     }

   if (!ee->prop.wm_rot.app_set)
     {
        return;
     }

   IFC(ee, fn_wm_rot_manual_rotation_done_set) (ee, set);
   IFE;
}

EAPI Eina_Bool
ecore_evas_wm_rotation_manual_rotation_done_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_wm_rotation_manual_rotation_done_get");
        return EINA_FALSE;
     }

   if (!ee->prop.wm_rot.app_set)
     {
        return EINA_FALSE;
     }

   return ee->prop.wm_rot.manual_mode.set;
}

EAPI void
ecore_evas_wm_rotation_manual_rotation_done(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_wm_rotation_manual_rotation_done");
        return;
     }

   if (!ee->prop.wm_rot.app_set)
     {
        return;
     }

   IFC(ee, fn_wm_rot_manual_rotation_done) (ee);
   IFE;
}

EAPI const Eina_List *
ecore_evas_aux_hints_supported_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_aux_hints_supported_get");
        return NULL;
     }
   return ee->prop.aux_hint.supported_list;
}

EAPI Eina_List *
ecore_evas_aux_hints_allowed_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_aux_hints_allowed_get");
        return NULL;
     }

   Eina_List *list = NULL, *ll;
   Ecore_Evas_Aux_Hint *aux;
   EINA_LIST_FOREACH(ee->prop.aux_hint.hints, ll, aux)
     {
        if ((aux->allowed) && !(aux->notified))
          {
             list = eina_list_append(list, (void*)(uintptr_t)aux->id);
          }
     }

   return list;
}

EAPI int
ecore_evas_aux_hint_add(Ecore_Evas *ee, const char *hint, const char *val)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_aux_hint_add");
        return -1;
     }

   Eina_List *ll;
   char *supported_hint;
   EINA_LIST_FOREACH(ee->prop.aux_hint.supported_list, ll, supported_hint)
     {
        if (!strncmp(supported_hint, hint, strlen(hint)))
          {
             Ecore_Evas_Aux_Hint *aux= (Ecore_Evas_Aux_Hint *)calloc(1, sizeof(Ecore_Evas_Aux_Hint));
             if (aux)
               {
                  aux->id = ee->prop.aux_hint.id;
                  aux->hint = eina_stringshare_add(hint);
                  aux->val = eina_stringshare_add(val);

                  ee->prop.aux_hint.hints = eina_list_append(ee->prop.aux_hint.hints, aux);

                  Eina_Strbuf *buf = _ecore_evas_aux_hints_string_get(ee);
                  if (buf)
                    {
                       if (ee->engine.func->fn_aux_hints_set)
                         ee->engine.func->fn_aux_hints_set(ee, eina_strbuf_string_get(buf));

                       eina_strbuf_free(buf);

                       ee->prop.aux_hint.id++;

                       return aux->id;
                    }

                  eina_stringshare_del(aux->hint);
                  eina_stringshare_del(aux->val);
                  free(aux);
               }
             break;
          }
     }

   return -1;
}

EAPI Eina_Bool
ecore_evas_aux_hint_del(Ecore_Evas *ee, const int id)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_aux_hint_del");
        return EINA_FALSE;
     }

   Eina_List *ll;
   Ecore_Evas_Aux_Hint *aux;
   EINA_LIST_FOREACH(ee->prop.aux_hint.hints, ll, aux)
     {
        if (id == aux->id)
          {
             ee->prop.aux_hint.hints = eina_list_remove(ee->prop.aux_hint.hints, aux);

             eina_stringshare_del(aux->hint);
             eina_stringshare_del(aux->val);
             free(aux);

             Eina_Strbuf *buf = _ecore_evas_aux_hints_string_get(ee);
             if (buf)
               {
                  if (ee->engine.func->fn_aux_hints_set)
                    ee->engine.func->fn_aux_hints_set(ee, eina_strbuf_string_get(buf));

                  eina_strbuf_free(buf);

                  return EINA_TRUE;
               }
             break;
          }
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_evas_aux_hint_val_set(Ecore_Evas *ee, const int id, const char *val)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_aux_hint_val_set");
        return EINA_FALSE;
     }

   Eina_List *ll;
   Ecore_Evas_Aux_Hint *aux;
   EINA_LIST_FOREACH(ee->prop.aux_hint.hints, ll, aux)
     {
        if (id == aux->id)
          {
             eina_stringshare_del(aux->val);
             aux->val = eina_stringshare_add(val);
			 aux->allowed = 0;
			 aux->notified = 0;

             Eina_Strbuf *buf = _ecore_evas_aux_hints_string_get(ee);
             if (buf)
               {
                  if (ee->engine.func->fn_aux_hints_set)
                    ee->engine.func->fn_aux_hints_set(ee, eina_strbuf_string_get(buf));

                  eina_strbuf_free(buf);

                  return EINA_TRUE;
               }
             break;
          }
     }

   return EINA_TRUE;
}

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

EAPI Eina_Bool
ecore_evas_withdrawn_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_withdrawn_get");
        return EINA_FALSE;
     }
   return ee->prop.withdrawn ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_sticky_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_sticky_set");
        return;
     }

   IFC(ee, fn_sticky_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_sticky_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_sticky_get");
        return EINA_FALSE;
     }
   return ee->prop.sticky ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_window_group_set(Ecore_Evas *ee, const Ecore_Evas *ee_group)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return;
     }

   IFC(ee, fn_window_group_set) (ee, ee_group);
   IFE;
}

EAPI const Ecore_Evas *
ecore_evas_window_group_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return NULL;
     }
   return ee->prop.group_ee;
}

EAPI void
ecore_evas_aspect_set(Ecore_Evas *ee, double aspect)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return;
     }

   IFC(ee, fn_aspect_set) (ee, aspect);
   IFE;
}

EAPI double
ecore_evas_aspect_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return EINA_FALSE;
     }
   return ee->prop.aspect;
}

EAPI void
ecore_evas_urgent_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return;
     }

   IFC(ee, fn_urgent_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_urgent_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return EINA_FALSE;
     }
   return ee->prop.urgent ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_modal_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return;
     }

   IFC(ee, fn_modal_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_modal_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return EINA_FALSE;
     }
   return ee->prop.modal ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_demand_attention_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return;
     }

   IFC(ee, fn_demands_attention_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_demand_attention_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return EINA_FALSE;
     }
   return ee->prop.demand_attention ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_focus_skip_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return;
     }

   IFC(ee, fn_focus_skip_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_focus_skip_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "XXX");
        return EINA_FALSE;
     }
   return ee->prop.focus_skip ? EINA_TRUE : EINA_FALSE;
}

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
ecore_evas_msg_parent_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_msg_parent_send");
        return;
     }
   DBG("Msg(to parent): ee=%p msg_domain=%d msg_id=%d size=%d", ee, msg_domain, msg_id, size);
   IFC(ee, fn_msg_parent_send) (ee, msg_domain, msg_id, data, size);
   IFE;
}

EAPI void
ecore_evas_msg_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_msg_send");
        return;
     }
   DBG("Msg: ee=%p msg_domain=%d msg_id=%d size=%d", ee, msg_domain, msg_id, size);
   IFC(ee, fn_msg_send) (ee, msg_domain, msg_id, data, size);
   IFE;
}

EAPI void
ecore_evas_callback_msg_parent_handle_set(Ecore_Evas *ee, void (*func_parent_handle)(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_msg_parent_handle");
        return;
     }
   DBG("Msg Parent handle: ee=%p", ee);
   ee->func.fn_msg_parent_handle = func_parent_handle;
}

EAPI void
ecore_evas_callback_msg_handle_set(Ecore_Evas *ee, void (*func_handle)(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size))
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_msg_handle");
        return;
     }
   DBG("Msg handle: ee=%p", ee);
   ee->func.fn_msg_handle = func_handle;
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

EAPI void
ecore_evas_screen_dpi_get(const Ecore_Evas *ee, int *xdpi, int *ydpi)
{
   if (xdpi) *xdpi = 0;
   if (ydpi) *ydpi = 0;
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_screen_geometry_get");
        return;
     }

   IFC(ee, fn_screen_dpi_get) (ee, xdpi, ydpi);
   IFE;
}

EAPI void
ecore_evas_draw_frame_set(Ecore_Evas *ee, Eina_Bool draw_frame)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, "ecore_evas_draw_frame_set");
        return;
     }
   ee->prop.draw_frame = draw_frame;
}

EAPI Eina_Bool
ecore_evas_draw_frame_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, "ecore_evas_draw_frame_get");
        return EINA_FALSE;
     }
   return ee->prop.draw_frame;
}

EAPI void 
ecore_evas_pointer_xy_get(const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, "ecore_evas_pointer_xy_get");
        return;
     }
   IFC(ee, fn_pointer_xy_get) (ee, x, y);
   IFE;
}

EAPI Eina_Bool 
ecore_evas_pointer_warp(const Ecore_Evas *ee, Evas_Coord x, Evas_Coord y)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, "ecore_evas_pointer_warp");
        return EINA_FALSE;
     }

   if (ee->engine.func->fn_pointer_warp)
     return ee->engine.func->fn_pointer_warp(ee, x, y);

   return EINA_FALSE;
}

EAPI void *
ecore_evas_pixmap_visual_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, "ecore_evas_pixmap_visual_get");
        return NULL;
     }

   if (!strcmp(ee->driver, "software_x11"))
     {
        Ecore_Evas_Interface_Software_X11 *iface;
        iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
        EINA_SAFETY_ON_NULL_RETURN_VAL(iface, NULL);

        if (iface->pixmap_visual_get)
          return iface->pixmap_visual_get(ee);
     }
   else if (!strcmp(ee->driver, "opengl_x11"))
     {
        Ecore_Evas_Interface_Gl_X11 *iface;
        iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
        EINA_SAFETY_ON_NULL_RETURN_VAL(iface, NULL);

        if (iface->pixmap_visual_get)
          return iface->pixmap_visual_get(ee);
     }

   return NULL;
}

EAPI unsigned long 
ecore_evas_pixmap_colormap_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, "ecore_evas_pixmap_colormap_get");
        return 0;
     }

   if (!strcmp(ee->driver, "software_x11"))
     {
        Ecore_Evas_Interface_Software_X11 *iface;
        iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
        EINA_SAFETY_ON_NULL_RETURN_VAL(iface, 0);

        if (iface->pixmap_colormap_get)
          return iface->pixmap_colormap_get(ee);
     }
   else if (!strcmp(ee->driver, "opengl_x11"))
     {
        Ecore_Evas_Interface_Gl_X11 *iface;
        iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
        EINA_SAFETY_ON_NULL_RETURN_VAL(iface, 0);

        if (iface->pixmap_colormap_get)
          return iface->pixmap_colormap_get(ee);
     }

   return 0;
}

EAPI int 
ecore_evas_pixmap_depth_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, "ecore_evas_pixmap_depth_get");
        return 0;
     }

   if (!strcmp(ee->driver, "software_x11"))
     {
        Ecore_Evas_Interface_Software_X11 *iface;
        iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
        EINA_SAFETY_ON_NULL_RETURN_VAL(iface, 0);

        if (iface->pixmap_depth_get)
          return iface->pixmap_depth_get(ee);
     }
   else if (!strcmp(ee->driver, "opengl_x11"))
     {
        Ecore_Evas_Interface_Gl_X11 *iface;
        iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
        EINA_SAFETY_ON_NULL_RETURN_VAL(iface, 0);

        if (iface->pixmap_depth_get)
          return iface->pixmap_depth_get(ee);
     }

   return 0;
}

/* fps debug calls - for debugging how much time your app actually spends */
/* rendering graphics... :) */

static int _ecore_evas_fps_debug_init_count = 0;
static int _ecore_evas_fps_debug_fd = -1;
unsigned int *_ecore_evas_fps_rendertime_mmap = NULL;

EAPI void
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
        char *buf2 = (char *)&zero;
        ssize_t todo = sizeof(unsigned int);

        while (todo > 0)
          {
             ssize_t r = write(_ecore_evas_fps_debug_fd, buf2, todo);
             if (r > 0)
               {
                  todo -= r;
                  buf2 += r;
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

EAPI void
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

EAPI void
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

EAPI void
_ecore_evas_register(Ecore_Evas *ee)
{
   ee->registered = 1;
   ecore_evases = (Ecore_Evas *)eina_inlist_prepend
     (EINA_INLIST_GET(ecore_evases), EINA_INLIST_GET(ee));
}

EAPI void
_ecore_evas_ref(Ecore_Evas *ee)
{
   ee->refcount++;
}

EAPI void
_ecore_evas_unref(Ecore_Evas *ee)
{
   ee->refcount--;
   if (ee->refcount == 0)
     {
        if (ee->deleted) _ecore_evas_free(ee);
     }
   else if (ee->refcount < -1)
     ERR("Ecore_Evas %p->refcount=%d < 0", ee, ee->refcount);
}

EAPI void
_ecore_evas_free(Ecore_Evas *ee)
{
   Ecore_Evas_Interface *iface;

   ee->deleted = EINA_TRUE;
   if (ee->refcount > 0) return;

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
   _ecore_evas_window_profile_free(ee);
   ee->prop.profile.name = NULL;
   _ecore_evas_window_available_profiles_free(ee);
   ee->prop.profile.available_list = NULL;
   if (ee->prop.wm_rot.available_rots) free(ee->prop.wm_rot.available_rots);
   ee->prop.wm_rot.available_rots = NULL;
   if (ee->prop.wm_rot.manual_mode.timer)
     ecore_timer_del(ee->prop.wm_rot.manual_mode.timer);
   _ecore_evas_aux_hint_free(ee);
   ee->prop.wm_rot.manual_mode.timer = NULL;
   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);
   ee->prop.cursor.object = NULL;
   if (ee->evas) evas_free(ee->evas);
   ee->evas = NULL;
   ECORE_MAGIC_SET(ee, ECORE_MAGIC_NONE);
   ee->driver = NULL;
   if (ee->engine.idle_flush_timer)
     ecore_timer_del(ee->engine.idle_flush_timer);
   ee->engine.idle_flush_timer = NULL;
   if (ee->engine.func->fn_free) ee->engine.func->fn_free(ee);
   if (ee->registered)
     {
        ecore_evases = (Ecore_Evas *)eina_inlist_remove
          (EINA_INLIST_GET(ecore_evases), EINA_INLIST_GET(ee));
     }

   EINA_LIST_FREE(ee->engine.ifaces, iface)
     free(iface);

   ee->engine.ifaces = NULL;
   free(ee);
}

static Eina_Bool
_ecore_evas_cb_idle_flush(void *data)
{
   Ecore_Evas *ee = data;

   ee->engine.idle_flush_timer = NULL;
   evas_render_idle_flush(ee->evas);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_ecore_evas_async_events_fd_handler(void *data EINA_UNUSED, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   evas_async_events_process();

   return ECORE_CALLBACK_RENEW;
}

EAPI void
_ecore_evas_idle_timeout_update(Ecore_Evas *ee)
{
   if (ee->engine.idle_flush_timer)
     ecore_timer_del(ee->engine.idle_flush_timer);
   ee->engine.idle_flush_timer = 
     ecore_timer_add(IDLE_FLUSH_TIME, _ecore_evas_cb_idle_flush, ee);
}

EAPI void
_ecore_evas_mouse_move_process(Ecore_Evas *ee, int x, int y, unsigned int timestamp)
{
   int fx, fy, fw, fh;
   ee->mouse.x = x;
   ee->mouse.y = y;

   evas_output_framespace_get(ee->evas, &fx, &fy, &fw, &fh);

   if (ee->prop.cursor.object)
     {
        evas_object_show(ee->prop.cursor.object);
        if (ee->rotation == 0)
          evas_object_move(ee->prop.cursor.object,
                           x - fx - ee->prop.cursor.hot.x,
                           y - fy - ee->prop.cursor.hot.y);
        else if (ee->rotation == 90)
          evas_object_move(ee->prop.cursor.object,
                           ee->h + fw - y - fx - 1 - ee->prop.cursor.hot.x,
                           x - fy - ee->prop.cursor.hot.y);
        else if (ee->rotation == 180)
          evas_object_move(ee->prop.cursor.object,
                           ee->w + fw - x - fx - 1 - ee->prop.cursor.hot.x,
                           ee->h + fh - y - fy - 1 - ee->prop.cursor.hot.y);
        else if (ee->rotation == 270)
          evas_object_move(ee->prop.cursor.object,
                           y - fx - ee->prop.cursor.hot.x,
                           ee->w + fh - x - fy - 1 - ee->prop.cursor.hot.y);
     }
   if (ee->rotation == 0)
     evas_event_input_mouse_move(ee->evas, x, y, timestamp, NULL);
   else if (ee->rotation == 90)
     evas_event_input_mouse_move(ee->evas, ee->h + fw - y - 1, x, timestamp, NULL);
   else if (ee->rotation == 180)
     evas_event_input_mouse_move(ee->evas, ee->w + fw - x - 1, ee->h + fh - y - 1, timestamp, NULL);
   else if (ee->rotation == 270)
     evas_event_input_mouse_move(ee->evas, y, ee->w + fh - x - 1, timestamp, NULL);
}

EAPI void
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
     evas_event_input_multi_move(ee->evas, device,
                                 x, y,
                                 radius,
                                 radius_x, radius_y,
                                 pressure,
                                 angle - ee->rotation,
                                 mx, my,
                                 timestamp, NULL);
   else if (ee->rotation == 90)
     evas_event_input_multi_move(ee->evas, device,
                                 ee->h - y - 1, x,
                                 radius,
                                 radius_y, radius_x,
                                 pressure,
                                 angle - ee->rotation,
                                 ee->h - my - 1, mx,
                                 timestamp, NULL);
   else if (ee->rotation == 180)
     evas_event_input_multi_move(ee->evas, device,
                                 ee->w - x - 1, ee->h - y - 1,
                                 radius,
                                 radius_x, radius_y,
                                 pressure,
                                 angle - ee->rotation,
                                 ee->w - mx - 1, ee->h - my - 1,
                                 timestamp, NULL);
   else if (ee->rotation == 270)
     evas_event_input_multi_move(ee->evas, device,
                                 y, ee->w - x - 1,
                                 radius,
                                 radius_y, radius_x,
                                 pressure,
                                 angle - ee->rotation,
                                 my, ee->w - mx - 1,
                                 timestamp, NULL);
}

EAPI void
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
     evas_event_input_multi_down(ee->evas, device,
                                 x, y,
                                 radius,
                                 radius_x, radius_y,
                                 pressure,
                                 angle - ee->rotation,
                                 mx, my,
                                 flags, timestamp, NULL);
   else if (ee->rotation == 90)
     evas_event_input_multi_down(ee->evas, device,
                                 ee->h - y - 1, x,
                                 radius,
                                 radius_y, radius_x,
                                 pressure,
                                 angle - ee->rotation,
                                 ee->h - my - 1, mx,
                                 flags, timestamp, NULL);
   else if (ee->rotation == 180)
     evas_event_input_multi_down(ee->evas, device,
                                 ee->w - x - 1, ee->h - y - 1,
                                 radius,
                                 radius_x, radius_y,
                                 pressure,
                                 angle - ee->rotation,
                                 ee->w - mx - 1, ee->h - my - 1,
                                 flags, timestamp, NULL);
   else if (ee->rotation == 270)
     evas_event_input_multi_down(ee->evas, device,
                                 y, ee->w - x - 1,
                                 radius,
                                 radius_y, radius_x,
                                 pressure,
                                 angle - ee->rotation,
                                 my, ee->w - mx - 1,
                                 flags, timestamp, NULL);
}

EAPI void
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
     evas_event_input_multi_up(ee->evas, device,
                               x, y,
                               radius,
                               radius_x, radius_y,
                               pressure,
                               angle - ee->rotation,
                               mx, my,
                               flags, timestamp, NULL);
   else if (ee->rotation == 90)
     evas_event_input_multi_up(ee->evas, device,
                               ee->h - y - 1, x,
                               radius,
                               radius_y, radius_x,
                               pressure,
                               angle - ee->rotation,
                               ee->h - my - 1, mx,
                               flags, timestamp, NULL);
   else if (ee->rotation == 180)
     evas_event_input_multi_up(ee->evas, device,
                               ee->w - x - 1, ee->h - y - 1,
                               radius,
                               radius_x, radius_y,
                               pressure,
                               angle - ee->rotation,
                               ee->w - mx - 1, ee->h - my - 1,
                               flags, timestamp, NULL);
   else if (ee->rotation == 270)
     evas_event_input_multi_up(ee->evas, device,
                               y, ee->w - x - 1,
                               radius,
                               radius_y, radius_x,
                               pressure,
                               angle - ee->rotation,
                               my, ee->w - mx - 1,
                               flags, timestamp, NULL);
}

EAPI void
_ecore_evas_window_profile_free(Ecore_Evas *ee)
{
   if (ee->prop.profile.name)
     eina_stringshare_del(ee->prop.profile.name);
}

EAPI void
_ecore_evas_window_available_profiles_free(Ecore_Evas *ee)
{
   if (ee->prop.profile.available_list)
     {
        int i;
        for (i = 0; i < ee->prop.profile.count; i++)
          {
             if (ee->prop.profile.available_list[i])
               {
                  eina_stringshare_del(ee->prop.profile.available_list[i]);
                  ee->prop.profile.available_list[i] = NULL;
               }
          }
        free(ee->prop.profile.available_list);
     }
}

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

EAPI Eina_List *
ecore_evas_sub_ecore_evas_list_get(const Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
     {
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS,
                         "ecore_evas_sub_ecore_evas_list_get");
        return NULL;
     }
   return ee->sub_ecore_evas;
}

EAPI void
ecore_evas_input_event_register(Ecore_Evas *ee)
{
   ecore_event_window_register((Ecore_Window)ee, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
}

EAPI void
ecore_evas_input_event_unregister(Ecore_Evas *ee)
{
   ecore_event_window_unregister((Ecore_Window)ee);
}

EAPI Eina_Strbuf *
_ecore_evas_aux_hints_string_get(Ecore_Evas *ee)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   if (buf)
     {
        if (eina_list_count(ee->prop.aux_hint.hints) > 0)
          {
             Eina_List *l;
             Ecore_Evas_Aux_Hint *aux;
             int i = 0;

             EINA_LIST_FOREACH(ee->prop.aux_hint.hints, l, aux)
               {
                  /* add delimiter */
                  if (i > 0) eina_strbuf_append_char(buf, ',');
                  eina_strbuf_append_printf(buf, "%d:%s:%s", aux->id, aux->hint, aux->val);
                  i++;
               }
          }
     }
   return buf;
}

void
_ecore_evas_aux_hint_free(Ecore_Evas *ee)
{
   char *hint;
   EINA_LIST_FREE(ee->prop.aux_hint.supported_list, hint)
     {
        eina_stringshare_del(hint);
     }

   Ecore_Evas_Aux_Hint *aux;
   EINA_LIST_FREE(ee->prop.aux_hint.hints, aux)
     {
        eina_stringshare_del(aux->hint);
        eina_stringshare_del(aux->val);
        free(aux);
     }
}

/**
 * @brief Create Ecore_Evas using fb backend.
 * @param disp_name The name of the display to be used.
 * @param rotation The rotation to be used.
 * @param w The width of the Ecore_Evas to be created.
 * @param h The height of the Ecore_Evas to be created.
 * @return The new Ecore_Evas.
 */
EAPI Ecore_Evas *
ecore_evas_fb_new(const char *disp_name, int rotation, int w, int h)
{
   Ecore_Evas *(*new)(const char *, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("fb");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_fb_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(disp_name, rotation, w, h);
}

/**
 * @brief Create Ecore_Evas using software x11.
 * @note If ecore is not compiled with support to x11 then nothing is done and NULL is returned.
 * @param disp_name The name of the Ecore_Evas to be created.
 * @param parent The parent of the Ecore_Evas to be created.
 * @param x The X coordinate to be used.
 * @param y The Y coordinate to be used.
 * @param w The width of the Ecore_Evas to be created.
 * @param h The height of the Ecore_Evas to be created.
 * @return A handle to the created Ecore_Evas.
 */
EAPI Ecore_Evas *
ecore_evas_software_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h)
{
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_software_x11_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(disp_name, parent, x, y, w, h);
}

/**
 * @brief Get the window from Ecore_Evas using software x11.
 * @note If ecore is not compiled with support for x11 or if @p ee was not
 * created with ecore_evas_software_x11_new() then nothing is done and
 * 0 is returned.
 * @param ee The Ecore_Evas from which to get the window.
 * @return The window of type Ecore_X_Window.
 */
EAPI Ecore_X_Window
ecore_evas_software_x11_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Software_X11 *iface;
   iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, 0);

   return iface->window_get(ee);
}

/**
 * @brief Set the direct_resize of Ecore_Evas using software x11.
 * @note If ecore is not compiled with support to x11 then nothing is done.
 * @param ee The Ecore_Evas in which to set direct resize.
 * @param on Enables the resize of Ecore_Evas if equals EINA_TRUE, disables if equals EINA_FALSE.
 */
EAPI void
ecore_evas_software_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Interface_Software_X11 *iface;
   iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->resize_set(ee, on);
}

/**
 * @brief Gets if the Ecore_Evas is being directly resized using software x11.
 * @note If ecore is not compiled with support to x11 then nothing is done and EINA_FALSE is returned.
 * @param ee The Ecore_Evas from which to get direct resize.
 * @return EINA_TRUE if the resize was managed directly, otherwise return EINA_FALSE.
 */
EAPI Eina_Bool
ecore_evas_software_x11_direct_resize_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Software_X11 *iface;
   iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);

   return iface->resize_get(ee);
}

/**
 * @brief Add extra window on Ecore_Evas using software x11.
 * @note If ecore is not compiled with support to x11 then nothing is done.
 * @param ee The Ecore_Evas on which to add the window.
 * @param win The window to be added at the Ecore_Evas.
 */
EAPI void
ecore_evas_software_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
   Ecore_Evas_Interface_Software_X11 *iface;
   iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->extra_event_window_add(ee, win);
}

/**
 * @brief Create a new Ecore_Evas which does not contain an XWindow. It will 
 * only contain an XPixmap to render to
 * 
 * @since 1.8
 */
EAPI Ecore_Evas *
ecore_evas_software_x11_pixmap_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h)
{
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_software_x11_pixmap_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(disp_name, parent, x, y, w, h);
}

/**
 * @brief Returns the underlying Ecore_X_Pixmap used in the Ecore_Evas
 * 
 * @param ee The Ecore_Evas whose pixmap is desired.
 * @return The underlying Ecore_X_Pixmap
 * 
 * @warning Support for this depends on the underlying windowing system.
 * 
 * @since 1.8
 */
EAPI Ecore_X_Pixmap 
ecore_evas_software_x11_pixmap_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Software_X11 *iface;
   iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, 0);

   if (iface->pixmap_get)
     return iface->pixmap_get(ee);

   return 0;
}

/**
 * @brief Create Ecore_Evas using opengl x11.
 * @note If ecore is not compiled with support to x11 then nothing is done and NULL is returned.
 * @param disp_name The name of the display of the Ecore_Evas to be created.
 * @param parent The parent of the Ecore_Evas to be created.
 * @param x The X coordinate to be used.
 * @param y The Y coordinate to be used.
 * @param w The width of the Ecore_Evas to be created.
 * @param h The height of the Ecore_Evas to be created.
 * @return The new Ecore_Evas.
 */
EAPI Ecore_Evas *
ecore_evas_gl_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h)
{
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_gl_x11_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(disp_name, parent, x, y, w, h);
}

EAPI Ecore_Evas *
ecore_evas_gl_x11_options_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h, const int *opt)
{
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int, const int*);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_gl_x11_options_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(disp_name, parent, x, y, w, h, opt);
}

/**
 * @brief Create a new Ecore_Evas which does not contain an XWindow. It will 
 * only contain an XPixmap to render to
 * 
 * @since 1.8
 */
EAPI Ecore_Evas *
ecore_evas_gl_x11_pixmap_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h)
{
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_gl_x11_pixmap_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(disp_name, parent, x, y, w, h);
}

/**
 * @brief Returns the underlying Ecore_X_Pixmap used in the Ecore_Evas
 * 
 * @param ee The Ecore_Evas whose pixmap is desired.
 * @return The underlying Ecore_X_Pixmap
 * 
 * @warning Support for this depends on the underlying windowing system.
 * 
 * @since 1.8
 */
EAPI Ecore_X_Pixmap 
ecore_evas_gl_x11_pixmap_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, 0);

   if (iface->pixmap_get)
     return iface->pixmap_get(ee);

   return 0;
}

/**
 * @brief Get the window from Ecore_Evas using opengl x11.
 * @note If ecore is not compiled with support for x11 or if @p ee was not
 * created with ecore_evas_gl_x11_new() then nothing is done and
 * 0 is returned.
 * @param ee The Ecore_Evas from which to get the window.
 * @return The window of type Ecore_X_Window of Ecore_Evas.
 */
EAPI Ecore_X_Window
ecore_evas_gl_x11_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, 0);

   return iface->window_get(ee);
}

/**
 * @brief Set direct_resize for Ecore_Evas using opengl x11.
 * @note If ecore is not compiled with support to x11 then nothing is done.
 * @param ee The Ecore_Evas in which to set direct resize.
 * @param on Enables the resize of Ecore_Evas if equals EINA_TRUE, disables if equals EINA_FALSE.
 */
EAPI void
ecore_evas_gl_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->resize_set(ee, on);
}

/**
 * @brief Gets if the Ecore_Evas is being directly resized using opengl x11.
 * @note If ecore is not compiled with support to x11 then nothing is done and EINA_FALSE is returned.
 * @param ee The Ecore_Evas from which to get direct resize.
 * @return EINA_TRUE if the resize was managed directly, otherwise return EINA_FALSE.
 */
EAPI Eina_Bool
ecore_evas_gl_x11_direct_resize_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);

   return iface->resize_get(ee);
}

/**
 * @brief Add extra window on Ecore_Evas using opengl x11.
 * @note If ecore is not compiled with support to x11 then nothing is done.
 * @param ee The Ecore_Evas for which to add the window.
 * @param win The window to be added at the Ecore_Evas.
 */
EAPI void
ecore_evas_gl_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->extra_event_window_add(ee, win);
}

/**
 * @brief Set the functions to be used before and after the swap callback.
 * @note If ecore is not compiled with support to x11 then nothing is done and the function is returned.
 * @param ee The Ecore_Evas for which to set the swap callback.
 * @param data The data for which to set the swap callback.
 * @param pre_cb The function to be called before the callback.
 * @param post_cb The function to be called after the callback.
 */
EAPI void
ecore_evas_gl_x11_pre_post_swap_callback_set(const Ecore_Evas *ee, void *data, void (*pre_cb) (void *data, Evas *e), void (*post_cb) (void *data, Evas *e))
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->pre_post_swap_callback_set(ee, data, pre_cb, post_cb);
}

EAPI void
ecore_evas_x11_leader_set(Ecore_Evas *ee, Ecore_X_Window win)
{
   Ecore_Evas_Interface_X11 *iface;
   iface = (Ecore_Evas_Interface_X11 *)_ecore_evas_interface_get(ee, "x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->leader_set(ee, win);
}

EAPI Ecore_X_Window
ecore_evas_x11_leader_get(Ecore_Evas *ee)
{
   Ecore_Evas_Interface_X11 *iface;
   iface = (Ecore_Evas_Interface_X11 *)_ecore_evas_interface_get(ee, "x11");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, 0);

   return iface->leader_get(ee);
}

EAPI void
ecore_evas_x11_leader_default_set(Ecore_Evas *ee)
{
   Ecore_Evas_Interface_X11 *iface;
   iface = (Ecore_Evas_Interface_X11 *)_ecore_evas_interface_get(ee, "x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->leader_default_set(ee);
}

EAPI void
ecore_evas_x11_shape_input_rectangle_set(Ecore_Evas *ee, int x, int y, int w, int h)
{
   Ecore_Evas_Interface_X11 *iface;
   iface = (Ecore_Evas_Interface_X11 *)_ecore_evas_interface_get(ee, "x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->shape_input_rectangle_set(ee, x, y, w, h);
}

EAPI void
ecore_evas_x11_shape_input_rectangle_add(Ecore_Evas *ee, int x, int y, int w, int h)
{
   Ecore_Evas_Interface_X11 *iface;
   iface = (Ecore_Evas_Interface_X11 *)_ecore_evas_interface_get(ee, "x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->shape_input_rectangle_add(ee, x, y, w, h);
}

EAPI void
ecore_evas_x11_shape_input_rectangle_subtract(Ecore_Evas *ee, int x, int y, int w, int h)
{
   Ecore_Evas_Interface_X11 *iface;
   iface = (Ecore_Evas_Interface_X11 *)_ecore_evas_interface_get(ee, "x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->shape_input_rectangle_subtract(ee, x, y, w, h);
}

EAPI void
ecore_evas_x11_shape_input_empty(Ecore_Evas *ee)
{
   Ecore_Evas_Interface_X11 *iface;
   iface = (Ecore_Evas_Interface_X11 *)_ecore_evas_interface_get(ee, "x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->shape_input_empty(ee);
}

EAPI void
ecore_evas_x11_shape_input_reset(Ecore_Evas *ee)
{
   Ecore_Evas_Interface_X11 *iface;
   iface = (Ecore_Evas_Interface_X11 *)_ecore_evas_interface_get(ee, "x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->shape_input_reset(ee);
}

EAPI void
ecore_evas_x11_shape_input_apply(Ecore_Evas *ee)
{
   Ecore_Evas_Interface_X11 *iface;
   iface = (Ecore_Evas_Interface_X11 *)_ecore_evas_interface_get(ee, "x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->shape_input_apply(ee);
}

EAPI Ecore_Evas *
ecore_evas_extn_socket_new(int w, int h)
{
   Ecore_Evas *(*new)(int, int);
   Eina_Module *m = _ecore_evas_engine_load("extn");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_extn_socket_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(w, h);
}

EAPI Eina_Bool
ecore_evas_extn_socket_listen(Ecore_Evas *ee, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   Ecore_Evas_Interface_Extn *iface;
   iface = (Ecore_Evas_Interface_Extn *)_ecore_evas_interface_get(ee, "extn");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);

   return iface->listen(ee, svcname, svcnum, svcsys);
}

EAPI void
ecore_evas_extn_plug_object_data_lock(Evas_Object *obj)
{
   Ecore_Evas_Interface_Extn *iface;
   Ecore_Evas *ee;

   ee = ecore_evas_object_ecore_evas_get(obj);
   EINA_SAFETY_ON_NULL_RETURN(ee);

   iface = (Ecore_Evas_Interface_Extn *)_ecore_evas_interface_get(ee, "extn");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->data_lock(ee);
}

EAPI void
ecore_evas_extn_plug_object_data_unlock(Evas_Object *obj)
{
   Ecore_Evas_Interface_Extn *iface;
   Ecore_Evas *ee;

   ee = ecore_evas_object_ecore_evas_get(obj);
   EINA_SAFETY_ON_NULL_RETURN(ee);

   iface = (Ecore_Evas_Interface_Extn *)_ecore_evas_interface_get(ee, "extn");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->data_unlock(ee);
}

EAPI Evas_Object *
ecore_evas_extn_plug_new(Ecore_Evas *ee_target)
{
   Evas_Object *(*new)(Ecore_Evas *);
   Eina_Module *m = _ecore_evas_engine_load("extn");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_extn_plug_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(ee_target);
}

EAPI Eina_Bool
ecore_evas_extn_plug_connect(Evas_Object *obj, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   Ecore_Evas_Interface_Extn *iface;
   Ecore_Evas *ee;

   ee = ecore_evas_object_ecore_evas_get(obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ee, EINA_FALSE);

   iface = (Ecore_Evas_Interface_Extn *)_ecore_evas_interface_get(ee, "extn");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);

   return iface->connect(ee, svcname, svcnum, svcsys);
}

EAPI Ecore_Evas *
ecore_evas_sdl_new(const char* name, int w, int h, int fullscreen,
		   int hwsurface, int noframe, int alpha)
{
   Ecore_Evas *(*new)(const char *, int, int, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("sdl");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_sdl_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(name, w, h, fullscreen, hwsurface, noframe, alpha);
}

EAPI Ecore_Evas *
ecore_evas_sdl16_new(const char* name, int w, int h, int fullscreen,
		     int hwsurface, int noframe, int alpha)
{
   Ecore_Evas *(*new)(const char *, int, int, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("sdl");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_sdl16_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(name, w, h, fullscreen, hwsurface, noframe, alpha);
}

EAPI Ecore_Evas *
ecore_evas_gl_sdl_new(const char* name, int w, int h, int fullscreen, int noframe)
{
   Ecore_Evas *(*new)(const char *, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("sdl");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_gl_sdl_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(name, w, h, fullscreen, noframe);
}

EAPI Ecore_Evas *
ecore_evas_wayland_shm_new(const char *disp_name, unsigned int parent,
			   int x, int y, int w, int h, Eina_Bool frame)
{
   Ecore_Evas *(*new)(const char *, unsigned int, int, int, int, int, Eina_Bool);
   Eina_Module *m = _ecore_evas_engine_load("wayland");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_wayland_shm_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(disp_name, parent, x, y, w, h, frame);
}

EAPI Ecore_Evas *
ecore_evas_wayland_egl_new(const char *disp_name, unsigned int parent,
			   int x, int y, int w, int h, Eina_Bool frame)
{
   Ecore_Evas *(*new)(const char *, unsigned int, int, int, int, int, Eina_Bool);
   Eina_Module *m = _ecore_evas_engine_load("wayland");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_wayland_egl_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(disp_name, parent, x, y, w, h, frame);
}

EAPI void
ecore_evas_wayland_resize(Ecore_Evas *ee, int location)
{
   Ecore_Evas_Interface_Wayland *iface;
   iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get(ee, "wayland");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->resize(ee, location);
}

EAPI void
ecore_evas_wayland_move(Ecore_Evas *ee, int x, int y)
{
   Ecore_Evas_Interface_Wayland *iface;
   iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get(ee, "wayland");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->move(ee, x, y);
}

EAPI void
ecore_evas_wayland_pointer_set(Ecore_Evas *ee, int hot_x, int hot_y)
{
   Ecore_Evas_Interface_Wayland *iface;
   iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get(ee, "wayland");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->pointer_set(ee, hot_x, hot_y);
}

EAPI void
ecore_evas_wayland_type_set(Ecore_Evas *ee, int type)
{
   Ecore_Evas_Interface_Wayland *iface;
   iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get(ee, "wayland");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->type_set(ee, type);
}

EAPI Ecore_Wl_Window *
ecore_evas_wayland_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Wayland *iface;
   iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get(ee, "wayland");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, NULL);

   return iface->window_get(ee);
}

EAPI Ecore_Evas *
ecore_evas_drm_new(const char *disp_name, unsigned int parent,
                   int x, int y, int w, int h)
{
   Ecore_Evas *(*new)(const char *, unsigned int, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("drm");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_drm_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(disp_name, parent, x, y, w, h);
}

EAPI Ecore_Evas *
ecore_evas_software_gdi_new(Ecore_Win32_Window *parent,
			    int                 x,
			    int                 y,
			    int                 width,
			    int                 height)
{
   Ecore_Evas *(*new)(Ecore_Win32_Window *, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("win32");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_software_gdi_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(parent, x, y, width, height);
}

EAPI Ecore_Evas *
ecore_evas_software_ddraw_new(Ecore_Win32_Window *parent,
			      int                 x,
			      int                 y,
			      int                 width,
			      int                 height)
{
   Ecore_Evas *(*new)(Ecore_Win32_Window *, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("win32");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_software_ddraw_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(parent, x, y, width, height);
}

EAPI Ecore_Win32_Window *
ecore_evas_win32_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Win32 *iface;
   iface = (Ecore_Evas_Interface_Win32 *)_ecore_evas_interface_get(ee, "win32");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, NULL);

   return iface->window_get(ee);
}

EAPI Ecore_Evas *
ecore_evas_cocoa_new(Ecore_Cocoa_Window *parent, int x, int y, int w, int h)
{
   Ecore_Evas *(*new)(Ecore_Cocoa_Window *, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("cocoa");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_cocoa_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(parent, x, y, w, h);
}

EAPI Ecore_Evas *
ecore_evas_psl1ght_new(const char* name, int w, int h)
{
   Ecore_Evas *(*new)(const char*, int, int);
   Eina_Module *m = _ecore_evas_engine_load("psl1ght");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_psl1ght_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   return new(name, w, h);
}
