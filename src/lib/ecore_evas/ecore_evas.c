#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define ECORE_EVAS_INTERNAL
#define EFL_INPUT_EVENT_PROTECTED

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if defined(HAVE_SYS_MMAN_H) || defined(_WIN32)
# include <sys/mman.h>
#endif

#ifdef _WIN32
# include <Evil.h>
#endif

#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>
#include <Evas_Internal.h>

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"
#include "ecore_evas_x11.h"
#include "ecore_evas_wayland.h"
#include "ecore_evas_cocoa.h"
#include "ecore_evas_extn.h"
#include "ecore_evas_win32.h"

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#ifndef O_BINARY
# define O_BINARY 0
#endif

#define ECORE_EVAS_CHECK(ee, ...) \
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS)) \
     { \
        ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, __FUNCTION__); \
        return __VA_ARGS__; \
     }

#define ECORE_EVAS_CHECK_GOTO(_ee, _label) \
  if (!ECORE_MAGIC_CHECK(_ee, ECORE_MAGIC_EVAS)) \
    { \
       ECORE_MAGIC_FAIL(_ee, ECORE_MAGIC_EVAS, __FUNCTION__); \
       goto _label; \
    }

EAPI Eina_Bool _ecore_evas_app_comp_sync = EINA_FALSE;
EAPI int _ecore_evas_log_dom = -1;
static int _ecore_evas_init_count = 0;

static Ecore_Idle_Exiter *ecore_evas_idle_exiter = NULL;
static Ecore_Idle_Enterer *ecore_evas_idle_enterer = NULL;
static Ecore_Evas *ecore_evases = NULL;
static int _ecore_evas_fps_debug = 0;

static const Efl_Event_Description *_event_description_get(Efl_Pointer_Action action);

//RENDER_SYNC
static int _ecore_evas_render_sync = 1;

static void
_ecore_evas_focus_out_dispatch(Ecore_Evas *ee, Efl_Input_Device *seat)
{
   evas_canvas_seat_focus_out(ee->evas, seat);
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
   if (ee->func.fn_focus_device_out) ee->func.fn_focus_device_out(ee, seat);
}

static void
_ecore_evas_device_del_cb(void *data, const Efl_Event *ev)
{
   Ecore_Evas *ee = data;

   ee->prop.focused_by = eina_list_remove(ee->prop.focused_by, ev->object);
   _ecore_evas_focus_out_dispatch(ee, ev->object);
}

static void
_ecore_evas_mouse_out_dispatch(Ecore_Evas *ee, Efl_Input_Device *mouse)
{
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   if (ee->func.fn_device_mouse_out) ee->func.fn_device_mouse_out(ee, mouse);
}

static void
_ecore_evas_mouse_del_cb(void *data, const Efl_Event *ev)
{
   Ecore_Evas *ee = data;

   ee->mice_in = eina_list_remove(ee->mice_in, ev->object);
   _ecore_evas_mouse_out_dispatch(ee, ev->object);
}

static void
_ecore_evas_animator(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Ecore_Evas *ee = data;

   ee->animator_ticked = EINA_TRUE;

   efl_event_callback_del(ee->evas, EFL_EVENT_ANIMATOR_TICK, _ecore_evas_animator, ee);
   ee->animator_registered = EINA_FALSE;
}

static Eina_Bool
_ecore_evas_changes_get(Ecore_Evas *ee)
{
   Eina_List *l;

   if (evas_changed_get(ee->evas)) return EINA_TRUE;
   EINA_LIST_FOREACH(ee->sub_ecore_evas, l, ee)
     {
        if (evas_changed_get(ee->evas)) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_ecore_evas_idle_exiter(void *data EINA_UNUSED)
{
   Ecore_Evas *ee;

   EINA_INLIST_FOREACH(ecore_evases, ee)
     ee->animator_ran = EINA_FALSE;

   return ECORE_CALLBACK_RENEW;
}

EAPI void
ecore_evas_render_wait(Ecore_Evas *ee)
{
   if (ee->in_async_render) evas_sync(ee->evas);
}

EAPI Eina_Bool
ecore_evas_render(Ecore_Evas *ee)
{
   Eina_Bool rend = EINA_FALSE;

   if (ee->in_async_render)
     {
        DBG("ee=%p is rendering, skip.", ee);
        return EINA_FALSE;
     }

   if (ee->engine.func->fn_prepare)
     if (!ee->engine.func->fn_prepare(ee))
       return EINA_FALSE;

   ecore_evas_render_prepare(ee);

   if (!ee->visible || ee->draw_block)
     {
        evas_norender(ee->evas);
     }
   else if (ee->can_async_render && !ee->manual_render)
     {
        rend |= !!evas_render_async(ee->evas);
        if (rend) ee->in_async_render = 1;
     }
   else
     {
        Eina_List *updates;

        updates = evas_render_updates(ee->evas);
        rend |= !!updates;
        evas_render_updates_free(updates);
     }

   return rend;
}

static void
_evas_evas_buffer_rendered(void *data, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee = data;

   _ecore_evas_idle_timeout_update(ee);

   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
   ee->in_async_render = 0;
}

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

   if (_ecore_evas_fps_debug)
     {
        t1 = ecore_time_get();
     }
   EINA_INLIST_FOREACH(ecore_evases, ee)
     {
        if (ee->draw_block) continue;

        if (ee->manual_render)
          {
             if (ee->engine.func->fn_evas_changed)
               ee->engine.func->fn_evas_changed(ee, EINA_FALSE);
             continue;
          }
        if (_ecore_evas_render_sync)
          {
             if (!ee->first_frame)
               {
                  if ((!ee->animator_ticked) &&
                      (!ee->animator_ran))
                    {
                       if (_ecore_evas_changes_get(ee))
                         {
                            if (!ee->animator_registered)
                              {
                                 efl_event_callback_add(ee->evas, EFL_EVENT_ANIMATOR_TICK, _ecore_evas_animator, ee);
                                 ee->animator_registered = EINA_TRUE;
                              }
                         }
                       continue ;
                    }
                  ee->animator_ticked = EINA_FALSE;
               }
             ee->first_frame = EINA_FALSE;
          }

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

        Eina_Bool change = EINA_FALSE;

        if (ee->engine.func->fn_render)
          {
             change = ee->engine.func->fn_render(ee);
          }
        else
          {
             change = ecore_evas_render(ee);
          }
        rend |= change;
         /*
          * Some engines that generate their own ticks based on hardware
          * events need to know that render has been considered, and
          * whether it will actually generate a new image or not
          */
        if (ee->engine.func->fn_evas_changed)
          ee->engine.func->fn_evas_changed(ee, change);

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

static void
_ecore_evas_object_cursor_del(void *data, Evas *e EINA_UNUSED,
                              Evas_Object *obj EINA_UNUSED,
                              void *event_info EINA_UNUSED)
{
   Ecore_Evas_Cursor *cursor = data;
   cursor->object = NULL;
}

static void
_ecore_evas_cursor_element_del(Ecore_Evas_Cursor *cursor)
{
   if (cursor->object)
     {
        evas_object_event_callback_del_full(cursor->object, EVAS_CALLBACK_DEL,
                                            _ecore_evas_object_cursor_del,
                                            cursor);
        evas_object_del(cursor->object);
     }
   free(cursor);
}

static void
_ecore_evas_cursor_add(Ecore_Evas *ee, Efl_Input_Device *dev)
{
   Ecore_Evas_Cursor *cursor;
   Eo *seat;

   seat = dev;
   if (evas_device_class_get(dev) != EVAS_DEVICE_CLASS_SEAT)
     seat = efl_input_device_seat_get(dev);
   if (eina_hash_find(ee->prop.cursors, &seat)) return;
   cursor = calloc(1, sizeof(Ecore_Evas_Cursor));
   EINA_SAFETY_ON_NULL_RETURN(cursor);
   eina_hash_add(ee->prop.cursors, &seat, cursor);
   if (seat != evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT)) return;
   if (ee->prop.cursor_cache.object)
     {
        ecore_evas_object_cursor_device_set(ee, dev,
                                            ee->prop.cursor_cache.object,
                                            ee->prop.cursor_cache.layer,
                                            ee->prop.cursor_cache.hot.x,
                                            ee->prop.cursor_cache.hot.y);
        memset(&ee->prop.cursor_cache, 0, sizeof(Ecore_Evas_Cursor));
     }
}

static inline Eina_Bool
_is_pointer(Eo *dev)
{
   Efl_Input_Device_Type c = efl_input_device_type_get(dev);
   return (c == EFL_INPUT_DEVICE_TYPE_MOUSE) ||
            (c == EFL_INPUT_DEVICE_TYPE_PEN) ||
            (c == EFL_INPUT_DEVICE_TYPE_TOUCH) ||
            (c == EFL_INPUT_DEVICE_TYPE_WAND);
}

static void
_ecore_evas_dev_added_or_removed(void *data, const Efl_Event *event)
{
   Ecore_Evas *ee = data;

   if (event->desc == EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED)
     {
        if (_is_pointer(event->info))
          _ecore_evas_cursor_add(ee, event->info);
        else if (event->info == evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT))
          {
             if (ee->prop.focused)
               _ecore_evas_focus_device_set(ee, event->info, 1);
          }
     }
   else
     {
        Eina_Iterator *it;
        Eina_Bool found = EINA_FALSE;
        Eo *dev, *seat = efl_input_device_seat_get(event->info);

        it = efl_input_device_children_iterate(seat);
        EINA_ITERATOR_FOREACH(it, dev)
          if ((dev != event->info) && _is_pointer(dev))
            {
               found = EINA_TRUE;
               break;
            }
        eina_iterator_free(it);
        if (!found)
          eina_hash_del_by_key(ee->prop.cursors, &seat);
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_evas_device_cbs,
                           { EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED, _ecore_evas_dev_added_or_removed },
                           { EFL_CANVAS_SCENE_EVENT_DEVICE_REMOVED, _ecore_evas_dev_added_or_removed });
Eina_Bool
_ecore_evas_cursors_init(Ecore_Evas *ee)
{
   const Eina_List *devs, *l;
   Efl_Input_Device *dev;

   if (!ee) return EINA_FALSE;
   ee->prop.cursors = eina_hash_pointer_new(EINA_FREE_CB(_ecore_evas_cursor_element_del));
   EINA_SAFETY_ON_NULL_RETURN_VAL(ee->prop.cursors, EINA_FALSE);

   devs = evas_device_list(ee->evas, NULL);

   EINA_LIST_FOREACH(devs, l, dev)
     {
        if (_is_pointer(dev))
          _ecore_evas_cursor_add(ee, dev);
     }

   efl_event_callback_array_priority_add(ee->evas, _ecore_evas_device_cbs(),
                                         EFL_CALLBACK_PRIORITY_BEFORE, ee);

   return EINA_TRUE;
}

static Ecore_Evas_Interface *
_ecore_evas_interface_get_internal(const Ecore_Evas *ee, const char *iname, Eina_Bool cri)
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

   if (cri)
     CRI("Ecore_Evas %p (engine: %s) does not have interface '%s'",
          ee, ee->driver, iname);

   return NULL;
}

EAPI Ecore_Evas_Interface *
_ecore_evas_interface_get(const Ecore_Evas *ee, const char *iname)
{
   return _ecore_evas_interface_get_internal(ee, iname, 1);
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
      case ECORE_EVAS_ENGINE_SOFTWARE_XCB: /* @deprecated */
        return EINA_FALSE;
      case ECORE_EVAS_ENGINE_XRENDER_XCB: /* @deprecated */
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
        return EINA_FALSE;
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
     case ECORE_EVAS_ENGINE_OPENGL_DRM:
#ifdef BUILD_ECORE_EVAS_GL_DRM
        return EINA_TRUE;
#else
        return EINA_FALSE;
#endif

      default:
        return EINA_FALSE;
     };
}

EAPI int
ecore_evas_init(void)
{
   if (++_ecore_evas_init_count != 1)
     return _ecore_evas_init_count;

   if (!ecore_init())
     goto shutdown_evas;

   if (!evas_init())
     return --_ecore_evas_init_count;

   _ecore_evas_log_dom = eina_log_domain_register
     ("ecore_evas", ECORE_EVAS_DEFAULT_LOG_COLOR);
   if(_ecore_evas_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for Ecore_Evas.");
        goto shutdown_ecore;
     }

   ecore_evas_idle_enterer =
     ecore_idle_enterer_add(_ecore_evas_idle_enter, NULL);
   ecore_evas_idle_exiter =
     ecore_idle_exiter_add(_ecore_evas_idle_exiter, NULL);
   if (getenv("ECORE_EVAS_FPS_DEBUG")) _ecore_evas_fps_debug = 1;
   if (getenv("ECORE_EVAS_RENDER_NOSYNC")) _ecore_evas_render_sync = 0;
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

#ifdef BUILD_ECORE_EVAS_EWS
   _ecore_evas_ews_events_flush();
#endif

   while (ecore_evases) _ecore_evas_free(ecore_evases);

   if (_ecore_evas_fps_debug) _ecore_evas_fps_debug_shutdown();
   ecore_idle_enterer_del(ecore_evas_idle_enterer);
   ecore_evas_idle_enterer = NULL;
   ecore_idle_exiter_del(ecore_evas_idle_exiter);
   ecore_evas_idle_exiter = NULL;

   _ecore_evas_extn_shutdown();

#ifdef BUILD_ECORE_EVAS_EWS
   while (_ecore_evas_ews_shutdown());
#endif
   _ecore_evas_engine_shutdown();

   eina_log_domain_unregister(_ecore_evas_log_dom);
   _ecore_evas_log_dom = -1;
   evas_shutdown();
   ecore_shutdown();

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
_ecore_evas_constructor_psl1ght(int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED, const char *extra_options EINA_UNUSED)
{
   return NULL;
}

static Ecore_Evas *
_ecore_evas_constructor_wayland_shm(int x, int y, int w, int h, const char *extra_options)
{
   char *disp_name = NULL;
   unsigned int frame = 0, parent = 0;
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
   unsigned int frame = 0, parent = 0;
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
_ecore_evas_constructor_opengl_drm(int x, int y, int w, int h, const char *extra_options)
{
   char *device = NULL;
   unsigned int parent = 0;
   Ecore_Evas *ee;

   _ecore_evas_parse_extra_options_str(extra_options, "device=", &device);
   _ecore_evas_parse_extra_options_uint(extra_options, "parent=", &parent);
   ee = ecore_evas_gl_drm_new(device, parent, x, y, w, h);
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
  {"gl_drm", _ecore_evas_constructor_opengl_drm},
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
   ECORE_EVAS_CHECK(ee, NULL);
   return ee;
}

EAPI void
ecore_evas_free(Ecore_Evas *ee)
{
   if (!ee) return;
   ECORE_EVAS_CHECK(ee);
   _ecore_evas_free(ee);
   return;
}

EAPI void *
ecore_evas_data_get(const Ecore_Evas *ee, const char *key)
{
   ECORE_EVAS_CHECK(ee, NULL);

   if (!key) return NULL;
   if (!ee->data) return NULL;

   return eina_hash_find(ee->data, key);
}

EAPI void
ecore_evas_data_set(Ecore_Evas *ee, const char *key, const void *data)
{
   ECORE_EVAS_CHECK(ee);

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
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_resize_set) (ee, func);
   IFE;
   ee->func.fn_resize = func;
}

EAPI void
ecore_evas_callback_move_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_move_set) (ee, func);
   IFE;
   ee->func.fn_move = func;
}

EAPI void
ecore_evas_callback_show_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_show_set) (ee, func);
   IFE;
   ee->func.fn_show = func;
}

EAPI void
ecore_evas_callback_hide_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_hide_set) (ee, func);
   IFE;
   ee->func.fn_hide = func;
}

EAPI void
ecore_evas_callback_delete_request_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_delete_request_set) (ee, func);
   IFE;
   ee->func.fn_delete_request = func;
}

EAPI void
ecore_evas_callback_destroy_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_destroy_set) (ee, func);
   IFE;
   ee->func.fn_destroy = func;
}

EAPI void
ecore_evas_callback_focus_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_focus_in_set) (ee, func);
   IFE;
   ee->func.fn_focus_in = func;
}

EAPI void
ecore_evas_callback_focus_device_in_set(Ecore_Evas *ee,
                                        Ecore_Evas_Focus_Device_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_focus_device_in_set) (ee, func);
   IFE;
   ee->func.fn_focus_device_in = func;
}

EAPI void
ecore_evas_callback_focus_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_focus_out_set) (ee, func);
   IFE;
   ee->func.fn_focus_out = func;
}

EAPI void
ecore_evas_callback_focus_device_out_set(Ecore_Evas *ee,
                                         Ecore_Evas_Focus_Device_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_focus_device_out_set) (ee, func);
   IFE;
   ee->func.fn_focus_device_out = func;
}

EAPI void
ecore_evas_callback_sticky_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_sticky_set) (ee, func);
   IFE;
   ee->func.fn_sticky = func;
}

EAPI void
ecore_evas_callback_unsticky_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_unsticky_set) (ee, func);
   IFE;
   ee->func.fn_unsticky = func;
}

EAPI void
ecore_evas_callback_mouse_in_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_mouse_in_set) (ee, func);
   IFE;
   ee->func.fn_mouse_in = func;
}

EAPI void
ecore_evas_callback_mouse_out_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_mouse_out_set) (ee, func);
   IFE;
   ee->func.fn_mouse_out = func;
}

EAPI void
ecore_evas_callback_pre_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_pre_render_set) (ee, func);
   IFE;
   ee->func.fn_pre_render = func;
}

EAPI void
ecore_evas_callback_post_render_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_post_render_set) (ee, func);
   IFE;
   ee->func.fn_post_render = func;
}

EAPI void
ecore_evas_callback_pre_free_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   ee->func.fn_pre_free = func;
}

EAPI void
ecore_evas_callback_state_change_set(Ecore_Evas *ee, Ecore_Evas_Event_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   ee->func.fn_state_change = func;
}

EAPI Evas *
ecore_evas_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, NULL);
   return ee->evas;
}

EAPI void
ecore_evas_move(Ecore_Evas *ee, int x, int y)
{
   ECORE_EVAS_CHECK(ee);
   if (ee->prop.fullscreen) return;
   IFC(ee, fn_move) (ee, x, y);
   IFE;
}

EAPI void
ecore_evas_managed_move(Ecore_Evas *ee, int x, int y)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_managed_move) (ee, x, y);
   IFE;
}

EAPI void
ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee, 0);
   return ee->rotation;
}

EAPI void
ecore_evas_shaped_set(Ecore_Evas *ee, Eina_Bool shaped)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_shaped_set) (ee, shaped);
   IFE;
}

EAPI Eina_Bool
ecore_evas_shaped_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->shaped ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_alpha_set(Ecore_Evas *ee, Eina_Bool alpha)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_alpha_set) (ee, alpha);
   IFE;
}

EAPI Eina_Bool
ecore_evas_alpha_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->alpha ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_transparent_set(Ecore_Evas *ee, Eina_Bool transparent)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_transparent_set) (ee, transparent);
   IFE;
}

EAPI Eina_Bool
ecore_evas_transparent_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->transparent ? EINA_TRUE : 0;
}

EAPI void
ecore_evas_show(Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_show) (ee);
   IFE;
}

EAPI void
ecore_evas_hide(Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_hide) (ee);
   IFE;
}

 EAPI int
ecore_evas_visibility_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, 0);
   return ee->visible ? 1:0;
}

EAPI void
ecore_evas_raise(Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_raise) (ee);
   IFE;
}

EAPI void
ecore_evas_lower(Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_lower) (ee);
   IFE;
}

EAPI void
ecore_evas_activate(Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_activate) (ee);
   IFE;
}

EAPI void
ecore_evas_title_set(Ecore_Evas *ee, const char *t)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_title_set) (ee, t);
   IFE;
}

EAPI const char *
ecore_evas_title_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, NULL);
   return ee->prop.title;
}

EAPI void
ecore_evas_name_class_set(Ecore_Evas *ee, const char *n, const char *c)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_name_class_set) (ee, n, c);
   IFE;
}

EAPI void
ecore_evas_name_class_get(const Ecore_Evas *ee, const char **n, const char **c)
{
   if (n) *n = NULL;
   if (c) *c = NULL;
   ECORE_EVAS_CHECK(ee);
   if (n) *n = ee->prop.name;
   if (c) *c = ee->prop.clas;
}

EAPI void
ecore_evas_size_min_set(Ecore_Evas *ee, int w, int h)
{
   ECORE_EVAS_CHECK(ee);
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
   if (w) *w = 0;
   if (h) *h = 0;
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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
   if (w) *w = 0;
   if (h) *h = 0;
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee);
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

EAPI Evas_Object *
_ecore_evas_default_cursor_image_get(Ecore_Evas *ee)
{
   Efl_Input_Device *pointer;
   Ecore_Evas_Cursor *cursor;

   pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT);
   cursor = eina_hash_find(ee->prop.cursors, &pointer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cursor, NULL);
   return cursor->object;
}

EAPI void
_ecore_evas_default_cursor_hide(Ecore_Evas *ee)
{
   Efl_Input_Device *pointer;
   Ecore_Evas_Cursor *cursor;

   pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT);
   cursor = eina_hash_find(ee->prop.cursors, &pointer);
   EINA_SAFETY_ON_NULL_RETURN(cursor);
   if (cursor->object)
     evas_object_hide(cursor->object);
}

static void
_ecore_evas_object_cursor_device_set(Ecore_Evas *ee, Efl_Input_Device *pointer,
                                     Evas_Object *obj, int layer,
                                     int hot_x, int hot_y)
{
   Ecore_Evas_Cursor *cursor;
   int x, y;
   Evas_Object *old;
   Efl_Input_Device *dpointer;

   ECORE_EVAS_CHECK(ee);

   dpointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT);
   if (pointer)
     {
        if (efl_input_device_type_get(pointer) != EFL_INPUT_DEVICE_TYPE_SEAT)
          pointer = efl_input_device_seat_get(pointer);
        if (efl_input_device_type_get(pointer) != EFL_INPUT_DEVICE_TYPE_SEAT)
          {
             ERR("Could not find seat");
             return;
          }
     }
   else
     {
        pointer = dpointer;
        if (!pointer)
          {
             ee->prop.cursor_cache.object = obj;
             ee->prop.cursor_cache.layer = layer;
             ee->prop.cursor_cache.hot.x = hot_x;
             ee->prop.cursor_cache.hot.y = hot_y;
             return;
          }
     }
   if (pointer == dpointer)
     memset(&ee->prop.cursor_cache, 0, sizeof(Ecore_Evas_Cursor));

   if (obj && ee->engine.func->fn_object_cursor_set)
     ee->engine.func->fn_object_cursor_set(ee, obj, layer, hot_x, hot_y);
   else if (!obj && ee->engine.func->fn_object_cursor_unset)
     ee->engine.func->fn_object_cursor_unset(ee);

   cursor = eina_hash_find(ee->prop.cursors, &pointer);
   EINA_SAFETY_ON_NULL_RETURN(cursor);
   old = cursor->object;
   if (!obj)
     {
        cursor->object = NULL;
        cursor->layer = 0;
        cursor->hot.x = 0;
        cursor->hot.y = 0;
        goto end;
     }

   cursor->object = obj;
   cursor->layer = layer;
   cursor->hot.x = hot_x;
   cursor->hot.y = hot_y;

   evas_pointer_output_xy_get(ee->evas, &x, &y);

   if (obj != old)
     {
        evas_object_layer_set(cursor->object, cursor->layer);
        evas_object_pass_events_set(cursor->object, 1);
        if (evas_pointer_inside_get(ee->evas))
          evas_object_show(cursor->object);
        evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                       _ecore_evas_object_cursor_del, cursor);
     }

   evas_object_move(cursor->object, x - cursor->hot.x,
                    y - cursor->hot.y);

end:
   if ((old) && (obj != old))
     {
        evas_object_event_callback_del_full
          (old, EVAS_CALLBACK_DEL, _ecore_evas_object_cursor_del, cursor);
        evas_object_del(old);
     }
}

EAPI void
ecore_evas_cursor_set(Ecore_Evas *ee, const char *file,
                      int layer, int hot_x, int hot_y)
{
   Evas_Object *obj = NULL;

   ECORE_EVAS_CHECK(ee);

   if (file)
     {
        int x, y;

        obj = evas_object_image_add(ee->evas);
        evas_object_image_file_set(obj, file, NULL);
        evas_object_image_size_get(obj, &x, &y);
        evas_object_resize(obj, x, y);
        evas_object_image_fill_set(obj, 0, 0, x, y);
     }

   _ecore_evas_object_cursor_device_set(ee, NULL, obj, layer, hot_x, hot_y);
}

EAPI void
ecore_evas_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj,
                             int layer, int hot_x, int hot_y)
{
   _ecore_evas_object_cursor_device_set(ee, NULL, obj, layer, hot_x, hot_y);
}

EAPI void
ecore_evas_object_cursor_device_set(Ecore_Evas *ee, Efl_Input_Device *pointer,
                                    Evas_Object *obj, int layer,
                                    int hot_x, int hot_y)
{
   _ecore_evas_object_cursor_device_set(ee, pointer, obj, layer, hot_x, hot_y);
}

EAPI Eina_Bool
ecore_evas_cursor_device_get(const Ecore_Evas *ee, Efl_Input_Device *pointer,
                             Evas_Object **obj, int *layer,
                             int *hot_x, int *hot_y)
{
   Ecore_Evas_Cursor *cursor;

   ECORE_EVAS_CHECK_GOTO(ee, err);

   if (pointer)
     {
        if (efl_input_device_type_get(pointer) != EFL_INPUT_DEVICE_TYPE_SEAT)
          pointer = efl_input_device_seat_get(pointer);
        if (efl_input_device_type_get(pointer) != EFL_INPUT_DEVICE_TYPE_SEAT)
          {
             ERR("Could not find seat");
             return EINA_FALSE;
          }
     }
   else
     pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT);
   if (pointer)
     {
        cursor = eina_hash_find(ee->prop.cursors, &pointer);
        if (cursor)
          {
             if (obj) *obj = cursor->object;
             if (layer) *layer = cursor->layer;
             if (hot_x) *hot_x = cursor->hot.x;
             if (hot_y) *hot_y = cursor->hot.y;
             return EINA_TRUE;
          }
     }

 err:
   if (obj) *obj = NULL;
   if (layer) *layer = 0;
   if (hot_x) *hot_x = 0;
   if (hot_y) *hot_y = 0;
   return EINA_FALSE;
}

EAPI void
ecore_evas_cursor_get(const Ecore_Evas *ee, Evas_Object **obj, int *layer, int *hot_x, int *hot_y)
{
   ecore_evas_cursor_device_get(ee, NULL, obj, layer, hot_x, hot_y);
}

EAPI Evas_Object *
ecore_evas_cursor_device_unset(Ecore_Evas *ee, Efl_Input_Device *pointer)
{
   Ecore_Evas_Cursor *cursor = NULL;
   Evas_Object *obj;

   ECORE_EVAS_CHECK(ee, NULL);

   if (pointer)
     {
        if (efl_input_device_type_get(pointer) != EFL_INPUT_DEVICE_TYPE_SEAT)
          pointer = efl_input_device_seat_get(pointer);
        if (efl_input_device_type_get(pointer) != EFL_INPUT_DEVICE_TYPE_SEAT)
          {
             ERR("Could not find seat");
             return NULL;
          }
     }
   else
     pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT);
   if (pointer)
     cursor = eina_hash_find(ee->prop.cursors, &pointer);
   if (!cursor) return NULL;
   obj = cursor->object;
   if (ee->engine.func->fn_object_cursor_unset)
     ee->engine.func->fn_object_cursor_unset(ee);
   evas_object_hide(obj);
   cursor->object = NULL;
   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_DEL,
                                       _ecore_evas_object_cursor_del,
                                       cursor);
   return obj;
}

EAPI Evas_Object *
ecore_evas_cursor_unset(Ecore_Evas *ee)
{
   return ecore_evas_cursor_device_unset(ee, NULL);
}

EAPI void
ecore_evas_layer_set(Ecore_Evas *ee, int layer)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_layer_set) (ee, layer);
   IFE;
   ee->prop.layer = layer;
}

EAPI int
ecore_evas_layer_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, 0);
   return ee->prop.layer;
}

EAPI Eina_Bool
ecore_evas_focus_device_get(const Ecore_Evas *ee, Efl_Input_Device *seat)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   if (!seat)
     seat = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT);
   return eina_list_data_find(ee->prop.focused_by, seat) ? EINA_TRUE : EINA_FALSE;
}

EAPI void
_ecore_evas_focus_device_set(Ecore_Evas *ee, Efl_Input_Device *seat,
                             Eina_Bool on)
{
   Eina_Bool present;

   if (!seat)
     seat = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT);
   if ((!on) && (!seat)) return;

   if (seat && (efl_input_device_type_get(seat) != EFL_INPUT_DEVICE_TYPE_SEAT))
     {
        ERR("The Input device must be an seat");
        return;
     }
   ee->prop.focused = ee->prop.focused_by || on;
   if (!seat) return;

   present = ecore_evas_focus_device_get(ee, seat);
   if (on)
     {
        if (present) return;
        ee->prop.focused_by = eina_list_append(ee->prop.focused_by, seat);
        efl_event_callback_add(seat, EFL_EVENT_DEL,
                               _ecore_evas_device_del_cb, ee);
        evas_canvas_seat_focus_in(ee->evas, seat);
        if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
        if (ee->func.fn_focus_device_in) ee->func.fn_focus_device_in(ee, seat);
     }
   else
     {
        if (!present) return;
        ee->prop.focused_by = eina_list_remove(ee->prop.focused_by, seat);
        efl_event_callback_del(seat, EFL_EVENT_DEL,
                               _ecore_evas_device_del_cb, ee);
        _ecore_evas_focus_out_dispatch(ee, seat);
     }
}

EAPI void
ecore_evas_focus_device_set(Ecore_Evas *ee, Efl_Input_Device *seat,
                            Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_focus_device_set) (ee, seat, on);
   IFE;
   _ecore_evas_focus_device_set(ee, seat, on);
}

EAPI void
ecore_evas_focus_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_focus_set) (ee, on);
   IFE;
   ecore_evas_focus_device_set(ee, NULL, on);
}

EAPI Eina_Bool
ecore_evas_focus_get(const Ecore_Evas *ee)
{
   return ecore_evas_focus_device_get(ee, NULL);
}

EAPI void
ecore_evas_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_iconified_set) (ee, on);
   IFE;
   ee->prop.iconified = !!on;
}

EAPI Eina_Bool
ecore_evas_iconified_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.iconified ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_borderless_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_borderless_set) (ee, on);
   IFE;
   ee->prop.borderless = !!on;
}

EAPI Eina_Bool
ecore_evas_borderless_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.borderless ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_override_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_override_set) (ee, on);
   IFE;
   ee->prop.override = !!on;
}

EAPI Eina_Bool
ecore_evas_override_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.override ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_maximized_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_maximized_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_maximized_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.maximized ? EINA_TRUE : EINA_FALSE;
}

EAPI Eina_Bool
ecore_evas_window_profile_supported_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->profile_supported ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_window_profile_set(Ecore_Evas *ee, const char *profile)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_profile_set) (ee, profile);
   IFE;
}

EAPI const char *
ecore_evas_window_profile_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, NULL);
   return ee->prop.profile.name;
}

EAPI void
ecore_evas_window_available_profiles_set(Ecore_Evas *ee, const char **profiles, const unsigned int count)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_profiles_set) (ee, profiles, count);
   IFE;
}

EAPI Eina_Bool
ecore_evas_window_available_profiles_get(Ecore_Evas *ee, char ***profiles, unsigned int *count)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);

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
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.wm_rot.supported;
}

EAPI void
ecore_evas_wm_rotation_preferred_rotation_set(Ecore_Evas *ee, int rotation)
{
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee, -1);
   return ee->prop.wm_rot.preferred_rot;
}

EAPI void
ecore_evas_wm_rotation_available_rotations_set(Ecore_Evas *ee, const int *rotations, unsigned int count)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_wm_rot_available_rotations_set) (ee, rotations, count);
   IFE;
}

EAPI Eina_Bool
ecore_evas_wm_rotation_available_rotations_get(const Ecore_Evas *ee, int **rotations, unsigned int *count)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
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
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   if (!ee->prop.wm_rot.app_set)
     {
        return EINA_FALSE;
     }

   return ee->prop.wm_rot.manual_mode.set;
}

EAPI void
ecore_evas_wm_rotation_manual_rotation_done(Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee);
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
   ECORE_EVAS_CHECK(ee, NULL);
   return ee->prop.aux_hint.supported_list;
}

EAPI Eina_List *
ecore_evas_aux_hints_allowed_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, NULL);

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
   ECORE_EVAS_CHECK(ee, -1);

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

                  ee->prop.aux_hint.hints =
                    eina_list_append(ee->prop.aux_hint.hints, aux);

                  if (!strncmp(ee->driver, "wayland", 7))
                    {
                       Ecore_Evas_Interface_Wayland *iface;

                       iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get(ee, "wayland");
                       EINA_SAFETY_ON_NULL_RETURN_VAL(iface, -1);

                       if (iface->aux_hint_add)
                         iface->aux_hint_add(ee, aux->id, hint, val);

                       ee->prop.aux_hint.id++;

                       return aux->id;
                    }
                  else
                    {
                       Eina_Strbuf *buf = _ecore_evas_aux_hints_string_get(ee);
                       if (buf)
                         {
                            if (ee->engine.func->fn_aux_hints_set)
                              ee->engine.func->fn_aux_hints_set(ee, eina_strbuf_string_get(buf));

                            eina_strbuf_free(buf);

                            ee->prop.aux_hint.id++;

                            return aux->id;
                         }
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
   ECORE_EVAS_CHECK(ee, EINA_FALSE);

   Eina_List *ll;
   Ecore_Evas_Aux_Hint *aux;
   EINA_LIST_FOREACH(ee->prop.aux_hint.hints, ll, aux)
     {
        if (id == aux->id)
          {
             ee->prop.aux_hint.hints =
               eina_list_remove(ee->prop.aux_hint.hints, aux);

             eina_stringshare_del(aux->hint);
             eina_stringshare_del(aux->val);
             free(aux);

             if (!strncmp(ee->driver, "wayland", 7))
               {
                  Ecore_Evas_Interface_Wayland *iface;

                  iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get(ee, "wayland");
                  EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);

                  if (iface->aux_hint_del)
                    iface->aux_hint_del(ee, id);

                  return EINA_TRUE;
               }
             else
               {
                  Eina_Strbuf *buf = _ecore_evas_aux_hints_string_get(ee);
                  if (buf)
                    {
                       if (ee->engine.func->fn_aux_hints_set)
                         ee->engine.func->fn_aux_hints_set(ee, eina_strbuf_string_get(buf));

                       eina_strbuf_free(buf);

                       return EINA_TRUE;
                    }
               }

             break;
          }
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_evas_aux_hint_val_set(Ecore_Evas *ee, const int id, const char *val)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);

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

             if (!strncmp(ee->driver, "wayland", 7))
               {
                  Ecore_Evas_Interface_Wayland *iface;

                  iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get(ee, "wayland");
                  EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);

                  if (iface->aux_hint_change)
                    iface->aux_hint_change(ee, id, val);

                  return EINA_TRUE;
               }
             else
               {
                  Eina_Strbuf *buf = _ecore_evas_aux_hints_string_get(ee);
                  if (buf)
                    {
                       if (ee->engine.func->fn_aux_hints_set)
                         ee->engine.func->fn_aux_hints_set(ee, eina_strbuf_string_get(buf));

                       eina_strbuf_free(buf);

                       return EINA_TRUE;
                    }
               }

             break;
          }
     }

   return EINA_FALSE;
}

EAPI const char *
ecore_evas_aux_hint_val_get(const Ecore_Evas *ee, int id)
{
   ECORE_EVAS_CHECK(ee, NULL);

   Eina_List *ll;
   Ecore_Evas_Aux_Hint *aux;
   EINA_LIST_FOREACH(ee->prop.aux_hint.hints, ll, aux)
     {
        if (id == aux->id) return aux->val;
     }

   return NULL;
}

EAPI int
ecore_evas_aux_hint_id_get(const Ecore_Evas *ee, const char *hint)
{
   ECORE_EVAS_CHECK(ee, -1);

   Eina_List *ll;
   Ecore_Evas_Aux_Hint *aux;
   EINA_LIST_FOREACH(ee->prop.aux_hint.hints, ll, aux)
     {
        if (!strcmp(hint,aux->hint)) return aux->id;
     }

   return -1;
}

EAPI void
ecore_evas_fullscreen_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_fullscreen_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_fullscreen_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.fullscreen ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_avoid_damage_set(Ecore_Evas *ee, Ecore_Evas_Avoid_Damage_Type on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_avoid_damage_set) (ee, on);
   IFE;
}

EAPI Ecore_Evas_Avoid_Damage_Type
ecore_evas_avoid_damage_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, ECORE_EVAS_AVOID_DAMAGE_NONE);
   return ee->prop.avoid_damage;
}

EAPI void
ecore_evas_withdrawn_set(Ecore_Evas *ee, Eina_Bool withdrawn)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_withdrawn_set) (ee, withdrawn);
   IFE;
}

EAPI Eina_Bool
ecore_evas_withdrawn_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.withdrawn ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_sticky_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_sticky_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_sticky_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.sticky ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_window_group_set(Ecore_Evas *ee, const Ecore_Evas *ee_group)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_window_group_set) (ee, ee_group);
   IFE;
}

EAPI const Ecore_Evas *
ecore_evas_window_group_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, NULL);
   return ee->prop.group_ee;
}

EAPI void
ecore_evas_aspect_set(Ecore_Evas *ee, double aspect)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_aspect_set) (ee, aspect);
   IFE;
}

EAPI double
ecore_evas_aspect_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, 0.0);
   return ee->prop.aspect;
}

EAPI void
ecore_evas_urgent_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_urgent_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_urgent_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.urgent ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_modal_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_modal_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_modal_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.modal ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_demand_attention_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_demands_attention_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_demand_attention_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.demand_attention ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_focus_skip_set(Ecore_Evas *ee, Eina_Bool on)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_focus_skip_set) (ee, on);
   IFE;
}

EAPI Eina_Bool
ecore_evas_focus_skip_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->prop.focus_skip ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_ignore_events_set(Ecore_Evas *ee, Eina_Bool ignore)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_ignore_events_set) (ee, ignore);
   IFE;
}

EAPI Eina_Bool
ecore_evas_ignore_events_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->ignore_events ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_manual_render_set(Ecore_Evas *ee, Eina_Bool manual_render)
{
   ECORE_EVAS_CHECK(ee);
   ee->manual_render = manual_render;
   if (!ee->animator_count) return;
   if (!ee->engine.func->fn_animator_register) return;
   if (!ee->engine.func->fn_animator_unregister) return;

   if (manual_render)
     ee->engine.func->fn_animator_unregister(ee);
   else
     ee->engine.func->fn_animator_register(ee);
}

EAPI Eina_Bool
ecore_evas_manual_render_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return ee->manual_render ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_manual_render(Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee);

   if (ee->manual_render)
     {
        double t = -1.0;

        if (ee->engine.func->fn_last_tick_get)
          t = ee->engine.func->fn_last_tick_get(ee);
        if (t < 0.0)
          t = ecore_loop_time_get();

        ecore_evas_animator_tick(ee, NULL, t);
     }

   if (ee->engine.func->fn_render)
     {
        ee->engine.func->fn_render(ee);
     }
   else
     {
        ecore_evas_render(ee);
        ecore_evas_render_wait(ee);
     }
}

EAPI void
ecore_evas_msg_parent_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   ECORE_EVAS_CHECK(ee);
   DBG("Msg(to parent): ee=%p msg_domain=%d msg_id=%d size=%d", ee, msg_domain, msg_id, size);
   IFC(ee, fn_msg_parent_send) (ee, msg_domain, msg_id, data, size);
   IFE;
}

EAPI void
ecore_evas_msg_send(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size)
{
   ECORE_EVAS_CHECK(ee);
   DBG("Msg: ee=%p msg_domain=%d msg_id=%d size=%d", ee, msg_domain, msg_id, size);
   IFC(ee, fn_msg_send) (ee, msg_domain, msg_id, data, size);
   IFE;
}

EAPI void
ecore_evas_callback_msg_parent_handle_set(Ecore_Evas *ee, void (*func_parent_handle)(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size))
{
   ECORE_EVAS_CHECK(ee);
   DBG("Msg Parent handle: ee=%p", ee);
   ee->func.fn_msg_parent_handle = func_parent_handle;
}

EAPI void
ecore_evas_callback_msg_handle_set(Ecore_Evas *ee, void (*func_handle)(Ecore_Evas *ee, int msg_domain, int msg_id, void *data, int size))
{
   ECORE_EVAS_CHECK(ee);
   DBG("Msg handle: ee=%p", ee);
   ee->func.fn_msg_handle = func_handle;
}


EAPI void
ecore_evas_comp_sync_set(Ecore_Evas *ee, Eina_Bool do_sync)
{
   ECORE_EVAS_CHECK(ee);
   ee->no_comp_sync = !do_sync;
}

EAPI Eina_Bool
ecore_evas_comp_sync_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   return !ee->no_comp_sync;
}

EAPI Ecore_Window
ecore_evas_window_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, 0);
   return ee->prop.window;
}

EAPI void
ecore_evas_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_screen_geometry_get) (ee, x, y, w, h);
   IFE;
}

EAPI void
ecore_evas_screen_dpi_get(const Ecore_Evas *ee, int *xdpi, int *ydpi)
{
   if (xdpi) *xdpi = 0;
   if (ydpi) *ydpi = 0;
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_screen_dpi_get) (ee, xdpi, ydpi);
   IFE;
}

EAPI void
ecore_evas_draw_frame_set(Ecore_Evas *ee EINA_UNUSED, Eina_Bool draw_frame EINA_UNUSED)
{
   WRN("Calling deprecated function %s (not implemented)", __FUNCTION__);
}

EAPI Eina_Bool
ecore_evas_draw_frame_get(const Ecore_Evas *ee EINA_UNUSED)
{
   WRN("Calling deprecated function %s (not implemented)", __FUNCTION__);
   return EINA_FALSE;
}

EAPI void
ecore_evas_shadow_geometry_set(Ecore_Evas *ee, int l, int r, int t, int b)
{
   ECORE_EVAS_CHECK(ee);
   if ((ee->shadow.l == l) && (ee->shadow.r == r) &&
       (ee->shadow.t == t) && (ee->shadow.b == b)) return;
   ee->shadow.l = l;
   ee->shadow.r = r;
   ee->shadow.t = t;
   ee->shadow.b = b;
   ee->shadow.changed = EINA_TRUE;
}

EAPI void
ecore_evas_shadow_geometry_get(const Ecore_Evas *ee, int *l, int *r, int *t, int *b)
{
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   ECORE_EVAS_CHECK(ee);
   if (l) *l = ee->shadow.l;
   if (r) *r = ee->shadow.r;
   if (t) *t = ee->shadow.t;
   if (b) *b = ee->shadow.b;
}

EAPI void 
ecore_evas_pointer_xy_get(const Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = 0;
   if (y) *y = 0;
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_pointer_xy_get) (ee, x, y);
   IFE;
}

EAPI Eina_Bool 
ecore_evas_pointer_warp(const Ecore_Evas *ee, Evas_Coord x, Evas_Coord y)
{
   ECORE_EVAS_CHECK(ee, EINA_FALSE);
   if (ee->engine.func->fn_pointer_warp)
     return ee->engine.func->fn_pointer_warp(ee, x, y);

   return EINA_FALSE;
}

EAPI void
ecore_evas_pointer_device_xy_get(const Ecore_Evas *ee,
                                 const Efl_Input_Device *pointer,
                                 Evas_Coord *x, Evas_Coord *y)
{
   ECORE_EVAS_CHECK(ee);

   if (pointer)
     {
        if (efl_input_device_type_get(pointer) != EFL_INPUT_DEVICE_TYPE_SEAT)
          pointer = efl_input_device_seat_get(pointer);
        if (efl_input_device_type_get(pointer) != EFL_INPUT_DEVICE_TYPE_SEAT)
          {
             ERR("Could not find seat");
             return;
          }
     }
   if ((!pointer) ||
       (pointer == evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_SEAT)))
     ecore_evas_pointer_xy_get(ee, x, y);
   else
     {
        static Eina_Bool (*pointer_xy_get)(const Evas_Object *, const Efl_Input_Device *, Evas_Coord *, Evas_Coord *y) = NULL;
        Eina_Module *mod;

        if (!pointer_xy_get && ee->vnc_server)
          {
             mod = _ecore_evas_vnc_server_module_load();
             EINA_SAFETY_ON_NULL_RETURN(mod);

             pointer_xy_get = eina_module_symbol_get(mod, "ecore_evas_vnc_server_pointer_xy_get");
             EINA_SAFETY_ON_NULL_RETURN(pointer_xy_get);
          }

        // FIXME: Handle matching of the efl_input_device with proper evas_object

        ecore_evas_pointer_xy_get(ee, x, y);
     }
}

EAPI void *
ecore_evas_pixmap_visual_get(const Ecore_Evas *ee)
{
   ECORE_EVAS_CHECK(ee, NULL);

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
   ECORE_EVAS_CHECK(ee, 0);

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
   ECORE_EVAS_CHECK(ee, 0);

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

   _ecore_evas_fps_debug_init_count++;
   if (_ecore_evas_fps_debug_init_count > 1) return;

   snprintf(buf, sizeof(buf), "%s/.ecore_evas_fps_debug-%i",
            eina_environment_tmp_get(), (int)getpid());
   _ecore_evas_fps_debug_fd = open(buf, O_CREAT | O_BINARY | O_TRUNC | O_RDWR, 0644);
   if (_ecore_evas_fps_debug_fd < 0)
     {
        unlink(buf);
        _ecore_evas_fps_debug_fd = open(buf, O_CREAT | O_BINARY | O_TRUNC | O_RDWR, 0644);
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

        snprintf(buf, sizeof(buf), "%s/.ecore_evas_fps_debug-%i",
                 eina_environment_tmp_get(), (int)getpid());
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
   if (EINA_DBL_EQ(rlapse, 0.0))
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
ecore_evas_animator_tick(Ecore_Evas *ee, Eina_Rectangle *viewport, double loop_time)
{
   Ecore_Evas *subee;
   Eina_List *l;
   Efl_Event_Animator_Tick a = { EINA_RECT_ZERO() };

   if (!viewport)
     {
        evas_output_size_get(ee->evas, &a.update_area.w, &a.update_area.h);
     }
   else
     {
        a.update_area.rect = *viewport;
     }

   ecore_loop_time_set(loop_time);

   ee->animator_ran = EINA_TRUE;
   efl_event_callback_call(ee->evas, EFL_EVENT_ANIMATOR_TICK, &a);

   // FIXME: We do not support partial animator in the subcanvas
   EINA_LIST_FOREACH(ee->sub_ecore_evas, l, subee)
     {
        ecore_evas_animator_tick(subee, NULL, loop_time);
     }

   // We are a source of sync for general animator.
   // Let's only trigger the animator once per rendering loop
   if (!ecore_main_loop_animator_ticked_get())
     {
        // FIXME: We might want to enforce also Ecore_Animatore frametime
        ecore_animator_custom_tick();
     }

   DBG("Animator ticked on %p.", ee->evas);
}

// Per Ecore_Evas ticking
static void
ecore_evas_tick_begin(Ecore_Evas *ee)
{
   if (ee->animator_count++ > 0) return;

   if (ee->manual_render)
     {
       DBG("Attempt to schedule tick for manually rendered canvas.");
       return;
     }
   ee->engine.func->fn_animator_register(ee);
}

static void
ecore_evas_tick_end(Ecore_Evas *ee)
{
   if ((--ee->animator_count) > 0) return;

   if (ee->manual_render) return;

   ee->engine.func->fn_animator_unregister(ee);
}

// Need all possible tick to tick for animator fallback as we don't
// know if a window is the source of animator
static void
_ecore_evas_custom_tick_begin(void *data EINA_UNUSED)
{
   Ecore_Evas *ee;

   EINA_INLIST_FOREACH(ecore_evases, ee)
     if (!ee->deleted &&
         ee->engine.func->fn_animator_register &&
         ee->engine.func->fn_animator_unregister)
       ecore_evas_tick_begin(ee);
}

static void
_ecore_evas_custom_tick_end(void *data EINA_UNUSED)
{
   Ecore_Evas *ee;

   EINA_INLIST_FOREACH(ecore_evases, ee)
     if (!ee->deleted &&
         ee->engine.func->fn_animator_register &&
         ee->engine.func->fn_animator_unregister)
       ecore_evas_tick_end(ee);
}

static void
_ecore_evas_tick_source_find(void)
{
   Ecore_Evas *ee;
   Eina_Bool source = EINA_FALSE;
   Eina_Bool have_x = EINA_FALSE;

   // Check if we do have a potential tick source for legacy
   EINA_INLIST_FOREACH(ecore_evases, ee)
     {
        if (!ee->deleted)
          {
             if ((ee->engine.func->fn_animator_register) &&
                 (ee->engine.func->fn_animator_unregister))
               {
                  source = EINA_TRUE;
               }
             if (ee->driver)
               {
                  if ((!strcmp(ee->driver, "software_x11")) ||
                      (!strcmp(ee->driver, "opengl_x11")))
                    have_x = EINA_TRUE;
               }
          }
     }

   // If just one source require fallback, we can't be sure that
   // we are not running enlightenment and that this source might
   // actually be the true tick source of all other window. In
   // that scenario, we have to forcefully fallback.
   EINA_INLIST_FOREACH(ecore_evases, ee)
     if (!ee->deleted &&
         (!ee->engine.func->fn_animator_register ||
          !ee->engine.func->fn_animator_unregister))
       {
          source = EINA_FALSE;
          break;
       }

   if (!source)
     {
        if (!have_x)
          {
             ecore_animator_custom_source_tick_begin_callback_set(NULL, NULL);
             ecore_animator_custom_source_tick_end_callback_set(NULL, NULL);
             ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_TIMER);
          }
     }
   else
     {
        // Source set will trigger the previous tick end registered and then the new begin.
        // As we don't what was in behind, better first begin and end after source is set.
        ecore_animator_custom_source_tick_begin_callback_set(_ecore_evas_custom_tick_begin, NULL);
        ecore_animator_custom_source_tick_end_callback_set(_ecore_evas_custom_tick_end, NULL);
        ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);
    }
}

static Eina_Bool
_ecore_evas_animator_fallback(void *data)
{
   ecore_evas_animator_tick(data, NULL, ecore_loop_time_get());
   return EINA_TRUE;
}

static void
_check_animator_event_catcher_add(void *data, const Efl_Event *event)
{
   const Efl_Callback_Array_Item_Full *array = event->info;
   Ecore_Evas *ee = data;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_EVENT_ANIMATOR_TICK)
          {
             if (!ee->animator_count)
               INF("Setting up animator for %p from '%s' with title '%s'.", ee->evas, ee->driver, ee->prop.title);

             if (ee->engine.func->fn_animator_register &&
                 ee->engine.func->fn_animator_unregister)
               {
                  // Backend support per window vsync
                  ecore_evas_tick_begin(ee);
               }
             else
               {
                  // Backend doesn't support per window vsync, fallback to generic support
                  if (ee->animator_count++ > 0) return;
                  ee->anim = ecore_animator_add(_ecore_evas_animator_fallback, ee);
               }

             // No need to walk more than once per array as you can not del
             // a partial array
             return;
          }
     }
}

static void
_check_animator_event_catcher_del(void *data, const Efl_Event *event)
{
   const Efl_Callback_Array_Item_Full *array = event->info;
   Ecore_Evas *ee = data;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_EVENT_ANIMATOR_TICK)
          {
             if (ee->animator_count == 1)
               INF("Unsetting up animator for %p from '%s' titled '%s'.", ee->evas, ee->driver, ee->prop.title);

             if (ee->engine.func->fn_animator_register &&
                 ee->engine.func->fn_animator_unregister)
               {
                  // Backend support per window vsync
                  ecore_evas_tick_end(ee);
               }
             else
               {
                  // Backend doesn't support per window vsync, fallback to generic support
                  if (--ee->animator_count > 0) return;
                  ecore_animator_del(ee->anim);
                  ee->anim = NULL;
               }
             return;
          }
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(animator_watch,
                          { EFL_EVENT_CALLBACK_ADD, _check_animator_event_catcher_add },
                          { EFL_EVENT_CALLBACK_DEL, _check_animator_event_catcher_del });

EAPI void
_ecore_evas_register_animators(Ecore_Evas *ee)
{
   efl_event_callback_array_add(ee->evas, animator_watch(), ee);
}

EAPI void
_ecore_evas_register(Ecore_Evas *ee)
{
   ee->registered = 1;
   ecore_evases = (Ecore_Evas *)eina_inlist_prepend
     (EINA_INLIST_GET(ecore_evases), EINA_INLIST_GET(ee));

   _ecore_evas_register_animators(ee);

   _ecore_evas_tick_source_find();
   if (_ecore_evas_render_sync) ee->first_frame = EINA_TRUE;
   if (!ee->engine.func->fn_render)
     evas_event_callback_priority_add(ee->evas, EVAS_CALLBACK_RENDER_POST, EVAS_CALLBACK_PRIORITY_AFTER,
                                      _evas_evas_buffer_rendered, ee);
}

EAPI void
_ecore_evas_subregister(Ecore_Evas *ee_target, Ecore_Evas *ee)
{
   _ecore_evas_register_animators(ee);

   ee_target->sub_ecore_evas = eina_list_append(ee_target->sub_ecore_evas, ee);

   if (!ee->engine.func->fn_render)
     evas_event_callback_priority_add(ee->evas, EVAS_CALLBACK_RENDER_POST, EVAS_CALLBACK_PRIORITY_AFTER,
                                      _evas_evas_buffer_rendered, ee);
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

static Eina_Bool
_ecore_evas_vnc_stop(Ecore_Evas *ee)
{
   Evas_Object *obj;
   Eina_List *l;

   EINA_LIST_FOREACH(ee->vnc_server, l, obj)
     evas_object_del(obj);

   return EINA_TRUE;
}

EAPI void
_ecore_evas_free(Ecore_Evas *ee)
{
   Efl_Input_Device *dev;
   Ecore_Evas_Interface *iface;

   ee->deleted = EINA_TRUE;
   if (ee->refcount > 0) return;

   // Stop all vsync first
   if (ee->animator_count > 0 &&
       ee->engine.func->fn_animator_register &&
       ee->engine.func->fn_animator_unregister)
     {
        // Backend support per window vsync
        ee->engine.func->fn_animator_unregister(ee);
        _ecore_evas_tick_source_find();
     }
   ee->animator_count = 0;

   efl_event_callback_array_del(ee->evas, animator_watch(), ee);
   if (ee->anim)
     ecore_animator_del(ee->anim);
   ee->anim = NULL;

   if (ee->func.fn_pre_free) ee->func.fn_pre_free(ee);
   if (ee->vnc_server) _ecore_evas_vnc_stop(ee);
   while (ee->sub_ecore_evas)
     {
        _ecore_evas_free(ee->sub_ecore_evas->data);
     }
   EINA_LIST_FREE(ee->prop.focused_by, dev)
     {
        efl_event_callback_del(dev, EFL_EVENT_DEL,
                               _ecore_evas_device_del_cb, ee);
     }
   EINA_LIST_FREE(ee->mice_in, dev)
     {
        efl_event_callback_del(dev, EFL_EVENT_DEL,
                               _ecore_evas_mouse_del_cb, ee);
     }
   if (ee->data) eina_hash_free(ee->data);
   ee->data = NULL;
   free(ee->name);
   ee->name = NULL;
   free(ee->prop.title);
   ee->prop.title = NULL;
   free(ee->prop.name);
   ee->prop.name = NULL;
   free(ee->prop.clas);
   ee->prop.clas = NULL;
   _ecore_evas_window_profile_free(ee);
   ee->prop.profile.name = NULL;
   _ecore_evas_window_available_profiles_free(ee);
   ee->prop.profile.available_list = NULL;
   free(ee->prop.wm_rot.available_rots);
   ee->prop.wm_rot.available_rots = NULL;
   if (ee->prop.wm_rot.manual_mode.timer)
     ecore_timer_del(ee->prop.wm_rot.manual_mode.timer);
   _ecore_evas_aux_hint_free(ee);
   ee->prop.wm_rot.manual_mode.timer = NULL;
   efl_event_callback_array_del(ee->evas, _ecore_evas_device_cbs(), ee);
   eina_hash_free(ee->prop.cursors);
   ee->prop.cursors = NULL;
   evas_free(ee->evas);
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

EAPI void
_ecore_evas_idle_timeout_update(Ecore_Evas *ee)
{
   if (ee->engine.idle_flush_timer)
     ecore_timer_del(ee->engine.idle_flush_timer);
   ee->engine.idle_flush_timer = 
     ecore_timer_loop_add(IDLE_FLUSH_TIME, _ecore_evas_cb_idle_flush, ee);
}

static void
_ecore_evas_mouse_move_process_internal(Ecore_Evas *ee,
                                        Efl_Input_Device *pointer,
                                        int x, int y, unsigned int timestamp,
                                        Eina_Bool feed)
{
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Eina_Bool send_event = EINA_TRUE;
   Ecore_Evas_Cursor *cursor;
   Eo *seat;
   int fx, fy, fw, fh, evt_x, evt_y;

   evas_output_framespace_get(ee->evas, &fx, &fy, &fw, &fh);

   if (pointer)
     {
        if (efl_input_device_type_get(pointer) != EFL_INPUT_DEVICE_TYPE_SEAT)
          seat = efl_input_device_seat_get(pointer);
        else seat = pointer;
     }
   else
     {
        pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_MOUSE);
        seat = efl_input_device_seat_get(pointer);
     }
   if (efl_input_device_type_get(seat) != EFL_INPUT_DEVICE_TYPE_SEAT)
     {
        ERR("Could not find seat");
        return;
     }
   cursor = eina_hash_find(ee->prop.cursors, &seat);
   if (cursor)
     {
        cursor->pos_x = x;
        cursor->pos_y = y;
        if (cursor->object)
          {
             evas_object_show(cursor->object);
             if (ee->rotation == 0)
               evas_object_move(cursor->object,
                                x - fx - cursor->hot.x,
                                y - fy - cursor->hot.y);
             else if (ee->rotation == 90)
               evas_object_move(cursor->object,
                                ee->h + fw - y - fx - 1 - cursor->hot.x,
                                x - fy - cursor->hot.y);
             else if (ee->rotation == 180)
               evas_object_move(cursor->object,
                                ee->w + fw - x - fx - 1 - cursor->hot.x,
                                ee->h + fh - y - fy - 1 - cursor->hot.y);
             else if (ee->rotation == 270)
               evas_object_move(cursor->object,
                                y - fx - cursor->hot.x,
                                ee->w + fh - x - fy - 1 - cursor->hot.y);
          }
     }

   if (!feed) return;
   if (ee->rotation == 0)
     {
        evt_x = x - fx;
        evt_y = y - fy;
     }
   else if (ee->rotation == 90)
     {
        evt_x = ee->h + fw - y - 1;
        evt_y = x;
     }
   else if (ee->rotation == 180)
     {
        evt_x = ee->w + fw - x - 1;
        evt_y = ee->h + fh - y - 1;
     }
   else if (ee->rotation == 270)
     {
        evt_x = y;
        evt_y = ee->w + fh - x - 1;
     }
   else
     send_event = EINA_FALSE;

   if (!send_event) return;

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, ee->evas, (void **) &ev);
   if (!evt) return;

   ev->action = EFL_POINTER_ACTION_MOVE;
   ev->device = efl_ref(pointer);
   ev->timestamp = timestamp;
   ev->cur.x = evt_x;
   ev->cur.y = evt_y;
   efl_input_pointer_finalize(evt);

   efl_event_callback_legacy_call(ee->evas,
                                  _event_description_get(ev->action), evt);
   efl_del(evt);
}

EAPI void
_ecore_evas_mouse_move_process(Ecore_Evas *ee, int x, int y, unsigned int timestamp)
{
   _ecore_evas_mouse_move_process_internal(ee, NULL, x, y, timestamp,
                                           EINA_TRUE);
}

EAPI void
_ecore_evas_mouse_device_move_process(Ecore_Evas *ee, Efl_Input_Device *pointer,
                                      int x, int y, unsigned int timestamp)
{
   _ecore_evas_mouse_move_process_internal(ee, pointer, x, y, timestamp,
                                           EINA_TRUE);
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
   ECORE_EVAS_CHECK(ee, NULL);
   return ee->sub_ecore_evas;
}

EAPI void
ecore_evas_input_event_register(Ecore_Evas *ee)
{
   ecore_evas_done(ee, EINA_FALSE);
}

EAPI void
ecore_evas_input_event_unregister(Ecore_Evas *ee)
{
   ecore_event_window_unregister(ee->prop.window);
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

EAPI Ecore_Evas *
ecore_evas_fb_new(const char *disp_name, int rotation, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("fb");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_fb_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, rotation, w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_software_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_software_x11_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, parent, x, y, w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_X_Window
ecore_evas_software_x11_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Software_X11 *iface;
   iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get_internal(ee, "software_x11", 0);
   if (!iface) return 0;

   return iface->window_get(ee);
}

EAPI void
ecore_evas_software_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Interface_Software_X11 *iface;
   iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->resize_set(ee, on);
}

EAPI Eina_Bool
ecore_evas_software_x11_direct_resize_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Software_X11 *iface;
   iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);

   return iface->resize_get(ee);
}

EAPI void
ecore_evas_software_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
   Ecore_Evas_Interface_Software_X11 *iface;
   iface = (Ecore_Evas_Interface_Software_X11 *)_ecore_evas_interface_get(ee, "software_x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->extra_event_window_add(ee, win);
}

EAPI Ecore_Evas *
ecore_evas_software_x11_pixmap_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_software_x11_pixmap_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, parent, x, y, w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;

}

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

EAPI Ecore_Evas *
ecore_evas_gl_x11_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_gl_x11_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, parent, x, y, w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;

}

EAPI Ecore_Evas *
ecore_evas_gl_x11_options_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h, const int *opt)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int, const int*);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_gl_x11_options_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, parent, x, y, w, h, opt);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_gl_x11_pixmap_new(const char *disp_name, Ecore_X_Window parent, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, Ecore_X_Window, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("x");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_gl_x11_pixmap_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, parent, x, y, w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;

}

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

EAPI Ecore_X_Window
ecore_evas_gl_x11_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get_internal(ee, "gl_x11", 0);
   if (!iface) return 0;

   return iface->window_get(ee);
}

EAPI void
ecore_evas_gl_x11_direct_resize_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->resize_set(ee, on);
}

EAPI Eina_Bool
ecore_evas_gl_x11_direct_resize_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
   EINA_SAFETY_ON_NULL_RETURN_VAL(iface, EINA_FALSE);

   return iface->resize_get(ee);
}

EAPI void
ecore_evas_gl_x11_extra_event_window_add(Ecore_Evas *ee, Ecore_X_Window win)
{
   Ecore_Evas_Interface_Gl_X11 *iface;
   iface = (Ecore_Evas_Interface_Gl_X11 *)_ecore_evas_interface_get(ee, "gl_x11");
   EINA_SAFETY_ON_NULL_RETURN(iface);

   iface->extra_event_window_add(ee, win);
}

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

EAPI Evas_Object *
ecore_evas_vnc_start(Ecore_Evas *ee, const char *addr, int port,
                     Ecore_Evas_Vnc_Client_Accept_Cb accept_cb,
                     Ecore_Evas_Vnc_Client_Disconnected_Cb disc_cb,
                     void *data)
{
   static Evas_Object *(*vnc_new)(Ecore_Evas *, int, const char *,
                                  Ecore_Evas_Vnc_Client_Accept_Cb,
                                  Ecore_Evas_Vnc_Client_Disconnected_Cb,
                                  void *) = NULL;
   Evas_Object *r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ee, NULL);

   if (!vnc_new)
     {
        Eina_Module *mod;

        mod = _ecore_evas_vnc_server_module_load();
        EINA_SAFETY_ON_NULL_RETURN_VAL(mod, NULL);

        vnc_new = eina_module_symbol_get(mod, "ecore_evas_vnc_server_new");
        EINA_SAFETY_ON_NULL_RETURN_VAL(vnc_new, NULL);
     }

   r = vnc_new(ee, port, addr, accept_cb, disc_cb, data);
   if (!r) return NULL;

   ee->vnc_server = eina_list_append(ee->vnc_server, r);
   return r;
}

EAPI Ecore_Evas *
ecore_evas_extn_socket_new(int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(int, int);
   Eina_Module *m = _ecore_evas_engine_load("extn");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_extn_socket_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;

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
ecore_evas_extn_socket_events_block_set(Ecore_Evas *ee, Eina_Bool events_block)
{
   void (*set)(Ecore_Evas*, Eina_Bool);
   Eina_Module *m = _ecore_evas_engine_load("extn");
   EINA_SAFETY_ON_NULL_RETURN(m);

   set = eina_module_symbol_get(m,
         "ecore_evas_extn_socket_events_block_set_internal");
   EINA_SAFETY_ON_NULL_RETURN(set);

   set(ee, events_block);
}

EAPI Eina_Bool
ecore_evas_extn_socket_events_block_get(Ecore_Evas *ee)
{
   Eina_Bool (*get)(Ecore_Evas*);
   Eina_Module *m = _ecore_evas_engine_load("extn");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, EINA_FALSE);

   get = eina_module_symbol_get(m,
         "ecore_evas_extn_socket_events_block_get_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(get, EINA_FALSE);

   return get(ee);
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
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, int, int, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("sdl");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_sdl_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(name, w, h, fullscreen, hwsurface, noframe, alpha);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_sdl16_new(const char* name, int w, int h, int fullscreen,
		     int hwsurface, int noframe, int alpha)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, int, int, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("sdl");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_sdl16_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(name, w, h, fullscreen, hwsurface, noframe, alpha);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_gl_sdl_new(const char* name, int w, int h, int fullscreen, int noframe)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("sdl");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_gl_sdl_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(name, w, h, fullscreen, noframe);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_wayland_shm_new(const char *disp_name, unsigned int parent,
			   int x, int y, int w, int h, Eina_Bool frame)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, unsigned int, int, int, int, int, Eina_Bool);
   Eina_Module *m = _ecore_evas_engine_load("wayland");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_wayland_shm_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, parent, x, y, w, h, frame);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_wayland_egl_new(const char *disp_name, unsigned int parent,
			   int x, int y, int w, int h, Eina_Bool frame)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, unsigned int, int, int, int, int, Eina_Bool);
   Eina_Module *m = _ecore_evas_engine_load("wayland");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_wayland_egl_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, parent, x, y, w, h, frame);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
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
   iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get_internal(ee, "wayland", 0);
   if (!iface) return NULL;

   return iface->window_get(ee);
}

EAPI Ecore_Cocoa_Window *
ecore_evas_cocoa_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Cocoa *iface;
   iface = (Ecore_Evas_Interface_Cocoa *)_ecore_evas_interface_get(ee, "opengl_cocoa");
   if (!iface) return NULL;
   return iface->window_get(ee);
}

EAPI Ecore_Wl2_Window *
ecore_evas_wayland2_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Wayland *iface;
   iface = (Ecore_Evas_Interface_Wayland *)_ecore_evas_interface_get_internal(ee, "wayland", 0);
   if (!iface) return NULL;

   return iface->window2_get(ee);
}

EAPI Ecore_Evas *
ecore_evas_drm_new(const char *disp_name, unsigned int parent,
                   int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, unsigned int, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("drm");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_drm_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, parent, x, y, w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_gl_drm_new(const char *disp_name, unsigned int parent,
                          int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char *, unsigned int, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("drm");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_gl_drm_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(disp_name, parent, x, y, w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;

}

EAPI Ecore_Evas *
ecore_evas_software_gdi_new(Ecore_Win32_Window *parent,
			    int                 x,
			    int                 y,
			    int                 width,
			    int                 height)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(Ecore_Win32_Window *, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("win32");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_software_gdi_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(parent, x, y, width, height);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;

}

EAPI Ecore_Evas *
ecore_evas_software_ddraw_new(Ecore_Win32_Window *parent,
			      int                 x,
			      int                 y,
			      int                 width,
			      int                 height)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(Ecore_Win32_Window *, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("win32");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_software_ddraw_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(parent, x, y, width, height);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_Win32_Window *
ecore_evas_win32_window_get(const Ecore_Evas *ee)
{
   Ecore_Evas_Interface_Win32 *iface;
   iface = (Ecore_Evas_Interface_Win32 *)_ecore_evas_interface_get(ee, "win32");
   if (!iface) return NULL;

   return iface->window_get(ee);
}

EAPI Ecore_Evas *
ecore_evas_cocoa_new(Ecore_Cocoa_Window *parent, int x, int y, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(Ecore_Cocoa_Window *, int, int, int, int);
   Eina_Module *m = _ecore_evas_engine_load("cocoa");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_cocoa_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(parent, x, y, w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_psl1ght_new(const char* name, int w, int h)
{
   Ecore_Evas *ee;
   Ecore_Evas *(*new)(const char*, int, int);
   Eina_Module *m = _ecore_evas_engine_load("psl1ght");
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);

   new = eina_module_symbol_get(m, "ecore_evas_psl1ght_new_internal");
   EINA_SAFETY_ON_NULL_RETURN_VAL(new, NULL);

   ee = new(name, w, h);
   if (!_ecore_evas_cursors_init(ee))
     {
        ecore_evas_free(ee);
        return NULL;
     }
   return ee;
}


/* new input model with eo:
 *  1. pass all events from ecore_input_evas through
 *     ecore_evas and send eo events from here
 *  2. those eo events can then be translated to legacy by evas
 *  3. let evas send legacy & eo events to the objects
 */

#define EVENT_XY_SET(EV, MX, MY, FX, FY) do { \
   EV->cur.x = (MX) - (FX); EV->cur.y = (MY) - (FY); \
   } while (0)

static inline void
_pointer_position_set(Efl_Input_Pointer_Data *ev, Ecore_Evas *ee, int x, int y,
                      double mx, double my)
{
   int fx, fy, fw, fh;

   if ((EINA_DBL_EQ(mx, 0.0)) &&
       (EINA_DBL_EQ(my, 0.0)))
     {
        mx = x;
        my = y;
     }

   evas_output_framespace_get(ee->evas, &fx, &fy, &fw, &fh);
   if (ee->rotation == 0)
     EVENT_XY_SET(ev, mx, my, fx, fy);
   else if (ee->rotation == 90)
     EVENT_XY_SET(ev, ee->h + fw - my - 1, mx, fx, fy);
   else if (ee->rotation == 180)
     EVENT_XY_SET(ev, ee->w + fw - mx - 1, ee->h + fh - my - 1, fx, fy);
   else if (ee->rotation == 270)
     EVENT_XY_SET(ev, my, ee->w + fh - mx - 1, fx, fy);
}

static const Efl_Event_Description *
_event_description_get(Efl_Pointer_Action action)
{
   switch (action)
     {
      case EFL_POINTER_ACTION_MOVE:
        return EFL_EVENT_POINTER_MOVE;
      case EFL_POINTER_ACTION_DOWN:
        return EFL_EVENT_POINTER_DOWN;
      case EFL_POINTER_ACTION_UP:
        return EFL_EVENT_POINTER_UP;
      case EFL_POINTER_ACTION_CANCEL:
        return EFL_EVENT_POINTER_CANCEL;
      case EFL_POINTER_ACTION_IN:
        return EFL_EVENT_POINTER_IN;
      case EFL_POINTER_ACTION_OUT:
        return EFL_EVENT_POINTER_OUT;
      case EFL_POINTER_ACTION_WHEEL:
        return EFL_EVENT_POINTER_WHEEL;
      case EFL_POINTER_ACTION_AXIS:
        return EFL_EVENT_POINTER_AXIS;
      default: return NULL;
     }
}

static Eina_Bool
_direct_mouse_updown(Ecore_Evas *ee, const Ecore_Event_Mouse_Button *info, Efl_Pointer_Action action)
{
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Evas *e = ee->evas;
   Eina_Bool processed;

   /* Unused information:
    * same_screen
    * root.{x,y}
    * root_window
    * event_window
    * same_screen
    * modifiers (already passed to evas, no need to do anything)
    */

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, e, (void **) &ev);
   if (!evt) return EINA_FALSE;

   ev->action = action;
   ev->button = info->buttons;
   if (info->double_click) ev->button_flags |= EFL_POINTER_FLAGS_DOUBLE_CLICK;
   if (info->triple_click) ev->button_flags |= EFL_POINTER_FLAGS_TRIPLE_CLICK;
   ev->timestamp = info->timestamp;
   ev->tool = info->multi.device;
   _pointer_position_set(ev, ee, info->x, info->y, info->multi.x, info->multi.y);
   ev->radius = info->multi.radius;
   ev->radius_x = info->multi.radius_x;
   ev->radius_y = info->multi.radius_y;
   ev->pressure = info->multi.pressure;
   ev->angle = info->multi.angle - ee->rotation;
   if (info->dev) ev->device = efl_ref(info->dev);
   else ev->device = efl_ref(evas_default_device_get(e, EFL_INPUT_DEVICE_TYPE_MOUSE));
   efl_input_pointer_finalize(evt);

   efl_event_callback_legacy_call(e, _event_description_get(ev->action), evt);
   processed = ev->evas_done;
   efl_unref(evt);

   return processed;
}

static Eina_Bool
_direct_mouse_down_cb(Ecore_Evas *ee, const Ecore_Event_Mouse_Button *info)
{
   return _direct_mouse_updown(ee, info, EFL_POINTER_ACTION_DOWN);
}

static Eina_Bool
_direct_mouse_up_cb(Ecore_Evas *ee, const Ecore_Event_Mouse_Button *info)
{
   return _direct_mouse_updown(ee, info, EFL_POINTER_ACTION_UP);
}

static Eina_Bool
_direct_mouse_cancel_cb(Ecore_Evas *ee, const Ecore_Event_Mouse_Button *info)
{
   return _direct_mouse_updown(ee, info, EFL_POINTER_ACTION_CANCEL);
}

static Eina_Bool
_direct_mouse_move_cb(Ecore_Evas *ee, const Ecore_Event_Mouse_Move *info)
{
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Evas *e = ee->evas;
   Eina_Bool processed;

   _ecore_evas_mouse_move_process_internal(ee, info->dev, info->x, info->y,
                                           info->timestamp, EINA_FALSE);

   /* Unused information:
    * same_screen
    * root.{x,y}
    * root_window
    * event_window
    * same_screen
    * modifiers (already passed to evas, no need to do anything)
    */

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, e, (void **) &ev);
   if (!evt) return EINA_FALSE;

   ev->action = EFL_POINTER_ACTION_MOVE;
   ev->timestamp = info->timestamp;
   ev->tool = info->multi.device;
   _pointer_position_set(ev, ee, info->x, info->y, info->multi.x, info->multi.y);

   ev->radius = info->multi.radius;
   ev->radius_x = info->multi.radius_x;
   ev->radius_y = info->multi.radius_y;
   ev->pressure = info->multi.pressure;
   ev->angle = info->multi.angle - ee->rotation;
   if (info->dev) ev->device = efl_ref(info->dev);
   else ev->device = efl_ref(evas_default_device_get(e, EFL_INPUT_DEVICE_TYPE_MOUSE));
   efl_input_pointer_finalize(evt);

   efl_event_callback_legacy_call(e, _event_description_get(ev->action), evt);
   processed = ev->evas_done;
   efl_del(evt);

   return processed;
}

static Eina_Bool
_direct_mouse_wheel_cb(Ecore_Evas *ee, const Ecore_Event_Mouse_Wheel *info)
{
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Evas *e = ee->evas;
   Eina_Bool processed;

   /* Unused information:
    * same_screen
    * root.{x,y}
    * root_window
    * event_window
    * modifiers (already passed to evas, no need to do anything)
    */

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, e, (void **) &ev);
   if (!evt) return EINA_FALSE;

   ev->action = EFL_POINTER_ACTION_WHEEL;
   ev->timestamp = info->timestamp;
   _pointer_position_set(ev, ee, info->x, info->y, info->x, info->y);
   ev->wheel.z = info->z;
   ev->wheel.horizontal = !!info->direction;
   if (info->dev) ev->device = efl_ref(info->dev);
   else ev->device = efl_ref(evas_default_device_get(e, EFL_INPUT_DEVICE_TYPE_MOUSE));
   efl_input_pointer_finalize(evt);

   efl_event_callback_legacy_call(e, _event_description_get(ev->action), evt);
   processed = ev->evas_done;
   efl_del(evt);

   return processed;
}

static Eina_Bool
_direct_mouse_inout(Ecore_Evas *ee, const Ecore_Event_Mouse_IO *info, Efl_Pointer_Action action)
{
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Evas *e = ee->evas;
   Eina_Bool processed;

   /* Unused information:
    * event_window
    * modifiers (already passed to evas, no need to do anything)
    */

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, e, (void **) &ev);
   if (!evt) return EINA_FALSE;

   ev->action = action;
   ev->timestamp = info->timestamp;
   _pointer_position_set(ev, ee, info->x, info->y, info->x, info->y);
   if (info->dev) ev->device = efl_ref(info->dev);
   else ev->device = efl_ref(evas_default_device_get(e, EFL_INPUT_DEVICE_TYPE_MOUSE));
   efl_input_pointer_finalize(evt);

   efl_event_callback_legacy_call(e, _event_description_get(ev->action), evt);
   processed = ev->evas_done;
   efl_del(evt);

   return processed;
}

static Eina_Bool
_direct_mouse_in_cb(Ecore_Evas *ee, const Ecore_Event_Mouse_IO *info)
{
   return _direct_mouse_inout(ee, info, EFL_POINTER_ACTION_IN);
}

static Eina_Bool
_direct_mouse_out_cb(Ecore_Evas *ee, const Ecore_Event_Mouse_IO *info)
{
   return _direct_mouse_inout(ee, info, EFL_POINTER_ACTION_OUT);
}

static Eina_Bool
_direct_axis_update_cb(Ecore_Evas *ee, const Ecore_Event_Axis_Update *info)
{
   Eina_Bool haswinx = 0, haswiny = 0;
   Efl_Input_Pointer_Data *ev;
   Efl_Input_Pointer *evt;
   Evas *e = ee->evas;
   Eina_Bool processed;
   double x = 0, y = 0;
   int n;

   /* Unused information:
    * window, root_window, event_window
    */

   evt = efl_input_instance_get(EFL_INPUT_POINTER_CLASS, e, (void **) &ev);
   if (!ev) return EINA_FALSE;

   ev->action = EFL_POINTER_ACTION_AXIS;
   ev->timestamp = info->timestamp;
   ev->tool = info->toolid;

   // see also evas_events.c
   for (n = 0; n < info->naxis; n++)
     {
        const Ecore_Axis *axis = &(info->axis[n]);
        switch (axis->label)
          {
           case ECORE_AXIS_LABEL_WINDOW_X:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_X);
             x = axis->value;
             haswinx = EINA_TRUE;
             break;

           case ECORE_AXIS_LABEL_WINDOW_Y:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_Y);
             y = axis->value;
             haswiny = EINA_TRUE;
             break;

           case ECORE_AXIS_LABEL_X:
             if (!haswinx)
               {
                  _efl_input_value_mark(ev, EFL_INPUT_VALUE_X);
                  x = axis->value;
               }
             ev->raw.x = axis->value;
             ev->has_raw = EINA_TRUE;
             break;

           case ECORE_AXIS_LABEL_Y:
             if (!haswiny)
               {
                  _efl_input_value_mark(ev, EFL_INPUT_VALUE_Y);
                  y = axis->value;
               }
             ev->raw.y = axis->value;
             ev->has_raw = EINA_TRUE;
             break;

           case ECORE_AXIS_LABEL_NORMAL_X:
             ev->norm.x = axis->value;
             ev->has_norm = EINA_TRUE;
             break;

           case ECORE_AXIS_LABEL_NORMAL_Y:
             ev->norm.y = axis->value;
             ev->has_norm = EINA_TRUE;
             break;

           case ECORE_AXIS_LABEL_PRESSURE:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_PRESSURE);
             ev->pressure = axis->value;
             break;

           case ECORE_AXIS_LABEL_DISTANCE:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_DISTANCE);
             ev->distance = axis->value;
             break;

           case ECORE_AXIS_LABEL_AZIMUTH:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_AZIMUTH);
             ev->azimuth = axis->value;
             break;

           case ECORE_AXIS_LABEL_TILT:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_TILT);
             ev->tilt = axis->value;
             break;

           case ECORE_AXIS_LABEL_TWIST:
             _efl_input_value_mark(ev, EFL_INPUT_VALUE_TWIST);
             ev->twist = axis->value;
             break;

           case ECORE_AXIS_LABEL_UNKNOWN:
           case ECORE_AXIS_LABEL_TOUCH_WIDTH_MAJOR:
           case ECORE_AXIS_LABEL_TOUCH_WIDTH_MINOR:
           case ECORE_AXIS_LABEL_TOOL_WIDTH_MAJOR:
           case ECORE_AXIS_LABEL_TOOL_WIDTH_MINOR:
           default:
             DBG("Unsupported axis label %d, value %f (discarded)",
                 axis->label, axis->value);
             break;
          }
     }
   _pointer_position_set(ev, ee, x, y, x, y);
   if (info->dev) ev->device = efl_ref(info->dev);
   else ev->device = efl_ref(evas_default_device_get(e, EFL_INPUT_DEVICE_TYPE_MOUSE));
   efl_input_pointer_finalize(evt);

   efl_event_callback_legacy_call(e, _event_description_get(ev->action), evt);
   processed = ev->evas_done;
   efl_del(evt);

   return processed;
}

static Eina_Bool
_direct_key_updown_cb(Ecore_Evas *ee, const Ecore_Event_Key *info, Eina_Bool down)
{
   Efl_Input_Key_Data *ev;
   Efl_Input_Key *evt;
   Evas *e = ee->evas;
   Eina_Bool processed;

   /* Unused information:
    * window
    * root_window
    * event_window
    * same_screen
    * modifiers (already passed to evas, no need to do anything)
    */

   evt = efl_input_instance_get(EFL_INPUT_KEY_CLASS, e, (void **) &ev);
   if (!evt || !ev) return EINA_FALSE;

   ev->timestamp = info->timestamp;
   ev->pressed = down;
   eina_stringshare_replace(&ev->keyname, info->keyname);
   eina_stringshare_replace(&ev->key, info->key);
   eina_stringshare_replace(&ev->string, info->string);
   eina_stringshare_replace(&ev->compose, info->compose);
   ev->keycode = info->keycode;
   ev->data = info->data;
   ev->event_flags = 0;
   if (info->dev) ev->device = efl_ref(info->dev);
   else ev->device = efl_ref(evas_default_device_get(e, EFL_INPUT_DEVICE_TYPE_KEYBOARD));

   if (down)
     efl_event_callback_legacy_call(e, EFL_EVENT_KEY_DOWN, evt);
   else
     efl_event_callback_legacy_call(e, EFL_EVENT_KEY_UP, evt);

   processed = ev->evas_done;
   efl_del(evt);

   return processed;
}

EAPI Eina_Bool
_ecore_evas_input_direct_cb(void *window, int type, const void *info)
{
   Ecore_Evas *ee = window;

   if (type == ECORE_EVENT_MOUSE_MOVE)
     return _direct_mouse_move_cb(ee, (const Ecore_Event_Mouse_Move *) info);
   else if (type == ECORE_EVENT_MOUSE_BUTTON_DOWN)
     return _direct_mouse_down_cb(ee, (const Ecore_Event_Mouse_Button *) info);
   else if (type == ECORE_EVENT_MOUSE_BUTTON_UP)
     return _direct_mouse_up_cb(ee, (const Ecore_Event_Mouse_Button *) info);
   else if (type == ECORE_EVENT_MOUSE_WHEEL)
     return _direct_mouse_wheel_cb(ee, (const Ecore_Event_Mouse_Wheel *) info);
   else if (type == ECORE_EVENT_MOUSE_IN)
     return _direct_mouse_in_cb(ee, (const Ecore_Event_Mouse_IO *) info);
   else if (type == ECORE_EVENT_MOUSE_OUT)
     return _direct_mouse_out_cb(ee, (const Ecore_Event_Mouse_IO *) info);
   else if (type == ECORE_EVENT_KEY_DOWN)
     return _direct_key_updown_cb(ee, (const Ecore_Event_Key *) info, EINA_TRUE);
   else if (type == ECORE_EVENT_KEY_UP)
     return _direct_key_updown_cb(ee, (const Ecore_Event_Key *) info, EINA_FALSE);
   else if (type == ECORE_EVENT_MOUSE_BUTTON_CANCEL)
     return _direct_mouse_cancel_cb(ee, (const Ecore_Event_Mouse_Button *) info);
   else if (type == ECORE_EVENT_AXIS_UPDATE)
     return _direct_axis_update_cb(ee, (const Ecore_Event_Axis_Update *) info);
   else
     {
        ERR("unhandled input event type %d", type);
        return EINA_FALSE;
     }
}

EAPI void
_ecore_evas_mouse_inout_set(Ecore_Evas *ee, Efl_Input_Device *mouse,
                            Eina_Bool in, Eina_Bool force_out)
{
   Efl_Input_Device *present;

   if (!mouse)
     mouse = evas_default_device_get(ee->evas,
                                     EFL_INPUT_DEVICE_TYPE_MOUSE);;

   EINA_SAFETY_ON_NULL_RETURN(mouse);
   present = eina_list_data_find(ee->mice_in, mouse);

   if (in)
     {
        if (present) return;
        ee->mice_in = eina_list_append(ee->mice_in, mouse);
        efl_event_callback_add(mouse, EFL_EVENT_DEL,
                               _ecore_evas_mouse_del_cb, ee);
        if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
     }
   else
     {
        if (present) ee->mice_in = eina_list_remove(ee->mice_in, mouse);
        else if (!present && !force_out) return;
        efl_event_callback_del(mouse, EFL_EVENT_DEL,
                               _ecore_evas_mouse_del_cb, ee);
        _ecore_evas_mouse_out_dispatch(ee, mouse);
     }
}

EAPI Eina_Bool
_ecore_evas_mouse_in_check(Ecore_Evas *ee, Efl_Input_Device *mouse)
{
   if (!mouse)
     mouse = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_MOUSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(mouse, EINA_FALSE);
   return eina_list_data_find(ee->mice_in, mouse) ? EINA_TRUE : EINA_FALSE;
}

EAPI void
ecore_evas_callback_device_mouse_out_set(Ecore_Evas *ee,
                                         Ecore_Evas_Mouse_IO_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_device_mouse_out_set) (ee, func);
   IFE;
   ee->func.fn_device_mouse_out = func;
}

EAPI void
ecore_evas_callback_device_mouse_in_set(Ecore_Evas *ee,
                                        Ecore_Evas_Mouse_IO_Cb func)
{
   ECORE_EVAS_CHECK(ee);
   IFC(ee, fn_callback_device_mouse_in_set) (ee, func);
   IFE;
   ee->func.fn_device_mouse_in = func;
}

static Evas *(*replacement_new)(int w, int h) = NULL;

EAPI void
ecore_evas_callback_new_set(Evas *(*func)(int w, int h))
{
   replacement_new = func;
}

EAPI Evas *
ecore_evas_evas_new(Ecore_Evas *ee, int w, int h)
{
   Evas *e;

   if (ee->evas) return ee->evas;

   if (replacement_new) e = replacement_new(w, h);
   else e = evas_new();
   if (!e) return NULL;

   ee->evas = e;
   evas_data_attach_set(e, ee);

   if (ECORE_EVAS_PORTRAIT(ee))
     {
        evas_output_size_set(e, w, h);
        evas_output_viewport_set(e, 0, 0, w, h);
     }
   else
     {
        evas_output_size_set(e, h, w);
        evas_output_viewport_set(e, 0, 0, h, w);
     }

   return e;
}

EAPI void
ecore_evas_done(Ecore_Evas *ee, Eina_Bool single_window)
{
   _ecore_evas_register(ee);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
   _ecore_event_window_direct_cb_set(ee->prop.window, _ecore_evas_input_direct_cb);

   if (single_window)
     evas_event_feed_mouse_in(ee->evas, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);
}
