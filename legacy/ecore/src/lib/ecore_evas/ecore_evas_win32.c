#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* for NULL */

#include <Ecore.h>
#include "ecore_private.h"
#ifdef BUILD_ECORE_EVAS_WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
# include <Ecore_Win32.h>
# include <ecore_win32_private.h>
#endif /* BUILD_ECORE_EVAS_WIN32 */

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_WIN32

#define ECORE_EVAS_EVENT_COUNT 8

static int _ecore_evas_init_count = 0;

static Ecore_Event_Handler *ecore_evas_event_handlers[ECORE_EVAS_EVENT_COUNT];

static Eina_Bool _ecore_evas_win32_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_win32_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_win32_event_window_damage(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_win32_event_window_destroy(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_win32_event_window_show(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_win32_event_window_hide(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_win32_event_window_configure(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_win32_event_window_delete_request(void *data __UNUSED__, int type __UNUSED__, void *event);

/* Private functions */

static int
_ecore_evas_win32_render(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *updates = NULL;
#ifdef BUILD_ECORE_EVAS_SOFTWARE_BUFFER
   Eina_List *ll;
   Ecore_Evas *ee2;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        rend |= _ecore_evas_buffer_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }
#endif
   if (ee->func.fn_pre_render) ee->func.fn_pre_render(ee);
   if (ee->prop.avoid_damage)
     {
        updates = evas_render_updates(ee->evas);
        if (updates) evas_render_updates_free(updates);
     }
   else if ((ee->visible) ||
            ((ee->should_be_visible) && (ee->prop.fullscreen)) ||
            ((ee->should_be_visible) && (ee->prop.override)))
     {
        if (ee->shaped)
          {
             updates = evas_render_updates(ee->evas);
             if (updates) evas_render_updates_free(updates);
          }
        else
          {
             updates = evas_render_updates(ee->evas);
             if (updates) evas_render_updates_free(updates);
          }
     }
   else
     evas_norender(ee->evas);
   if (updates) rend = 1;
   if (ee->func.fn_post_render) ee->func.fn_post_render(ee);
   return rend;
}

static int
_ecore_evas_win32_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1)
     return _ecore_evas_init_count;

   ecore_evas_event_handlers[0]  = ecore_event_handler_add(ECORE_WIN32_EVENT_MOUSE_IN, _ecore_evas_win32_event_mouse_in, NULL);
   ecore_evas_event_handlers[1]  = ecore_event_handler_add(ECORE_WIN32_EVENT_MOUSE_OUT, _ecore_evas_win32_event_mouse_out, NULL);
   ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_DAMAGE, _ecore_evas_win32_event_window_damage, NULL);
   ecore_evas_event_handlers[3]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_DESTROY, _ecore_evas_win32_event_window_destroy, NULL);
   ecore_evas_event_handlers[4]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_SHOW, _ecore_evas_win32_event_window_show, NULL);
   ecore_evas_event_handlers[5]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_HIDE, _ecore_evas_win32_event_window_hide, NULL);
   ecore_evas_event_handlers[6]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_CONFIGURE, _ecore_evas_win32_event_window_configure, NULL);
   ecore_evas_event_handlers[7]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST, _ecore_evas_win32_event_window_delete_request, NULL);

   ecore_event_evas_init();
   return _ecore_evas_init_count;
}

int
_ecore_evas_win32_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
        int i;

        for (i = 0; i < ECORE_EVAS_EVENT_COUNT; i++)
          ecore_event_handler_del(ecore_evas_event_handlers[i]);
        ecore_event_evas_shutdown();
     }

   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;

   return _ecore_evas_init_count;
}

static Eina_Bool
_ecore_evas_win32_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                 *ee;
   Ecore_Win32_Event_Mouse_In *e;

   INF("mouse in");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return 1;

   if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   evas_event_feed_mouse_in(ee->evas, e->time, NULL);
   evas_focus_in(ee->evas);
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);

   return 1;
}

static Eina_Bool
_ecore_evas_win32_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                  *ee;
   Ecore_Win32_Event_Mouse_Out *e;

   INF("mouse out");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return 1;

   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);

   evas_event_feed_mouse_out(ee->evas, e->time, NULL);
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);

   return 1;
}

static Eina_Bool
_ecore_evas_win32_event_window_damage(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                      *ee;
   Ecore_Win32_Event_Window_Damage *e;

   INF("window damage");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return 1;

   if (ee->prop.avoid_damage)
     {
#ifdef _MSC_VER
# pragma message ("[ECORE] [WIN32] No Region code")
#else
# warning [ECORE] [WIN32] No Region code
#endif /* ! _MSC_VER */
     }
   else
     {
        if (ee->rotation == 0)
          evas_damage_rectangle_add(ee->evas,
                                    e->x,
                                    e->y,
                                    e->width,
                                    e->height);
        else if (ee->rotation == 90)
          evas_damage_rectangle_add(ee->evas,
                                    ee->h - e->y - e->height,
                                    e->x,
                                    e->height,
                                    e->width);
        else if (ee->rotation == 180)
          evas_damage_rectangle_add(ee->evas,
                                    ee->w - e->x - e->width,
                                    ee->h - e->y - e->height,
                                    e->width,
                                    e->height);
        else if (ee->rotation == 270)
          evas_damage_rectangle_add(ee->evas,
                                    e->y,
                                    ee->w - e->x - e->width,
                                    e->height,
                                    e->width);
     }

   return 1;
}

static Eina_Bool
_ecore_evas_win32_event_window_destroy(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                       *ee;
   Ecore_Win32_Event_Window_Destroy *e;

   INF("window destroy");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return 1;
   if (ee->func.fn_destroy) ee->func.fn_destroy(ee);
   ecore_evas_free(ee);

   return 1;
}

static Eina_Bool
_ecore_evas_win32_event_window_show(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                    *ee;
   Ecore_Win32_Event_Window_Show *e;

   INF("window show");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return 1;
   if (ee->visible) return 0; /* dont pass it on */
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);

   return 1;
}

static Eina_Bool
_ecore_evas_win32_event_window_hide(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                    *ee;
   Ecore_Win32_Event_Window_Hide *e;

   INF("window hide");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return 1;
   if (!ee->visible) return 0; /* dont pass it on */
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);

   return 1;
}

static Eina_Bool
_ecore_evas_win32_event_window_configure(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                         *ee;
   Ecore_Win32_Event_Window_Configure *e;

   INF("window configure");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return 1;

   if ((ee->x != e->x) || (ee->y != e->y))
     {
        ee->x = e->x;
        ee->y = e->y;
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }

   if ((ee->w != e->width) || (ee->h != e->height))
     {
        ee->w = e->width;
        ee->h = e->height;
        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        if (ee->prop.avoid_damage)
          {
             int pdam;

             pdam = ecore_evas_avoid_damage_get(ee);
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, pdam);
          }
/*         if (ee->shaped) */
/*           _ecore_evas_win32_region_border_resize(ee); */
        if ((ee->expecting_resize.w > 0) &&
            (ee->expecting_resize.h > 0))
          {
             if ((ee->expecting_resize.w == ee->w) &&
                 (ee->expecting_resize.h == ee->h))
               _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                              ecore_win32_current_time_get());
             ee->expecting_resize.w = 0;
             ee->expecting_resize.h = 0;
          }
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }

   return 1;
}

static Eina_Bool
_ecore_evas_win32_event_window_delete_request(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                              *ee;
   Ecore_Win32_Event_Window_Delete_Request *e;

   INF("window delete request");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return 1;
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);

   INF(" * ee event delete\n");
   return 1;
}


/* Ecore_Evas interface */

static void
_ecore_evas_win32_free(Ecore_Evas *ee)
{
   INF("ecore evas free");

   ecore_win32_window_free((struct _Ecore_Win32_Window *)ee->prop.window);
   ecore_event_window_unregister(ee->prop.window);
   _ecore_evas_win32_shutdown();
   ecore_win32_shutdown();
}

static void
_ecore_evas_win32_callback_delete_request_set(Ecore_Evas *ee,
                                              void (*func) (Ecore_Evas *ee))
{
   ee->func.fn_delete_request = func;
}

static void
_ecore_evas_win32_move(Ecore_Evas *ee, int x, int y)
{
  INF("ecore evas move (%dx%d)", x, y);

   if ((x != ee->x) || (y != ee->y))
     {
        ee->x = x;
        ee->y = y;
        ecore_win32_window_move((struct _Ecore_Win32_Window *)ee->prop.window,
                                x, y);
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }
}

static void
_ecore_evas_win32_resize(Ecore_Evas *ee, int width, int height)
{
   INF("ecore evas resize (%dx%d)", width, height);

   if ((ee->w != width) || (ee->h != height))
     {
        ee->w = width;
        ee->h = height;
        ecore_win32_window_resize((struct _Ecore_Win32_Window *)ee->prop.window,
                                  width, height);
        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        if (ee->prop.avoid_damage)
          {
             int pdam;

             pdam = ecore_evas_avoid_damage_get(ee);
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, pdam);
          }
/*         if ((ee->shaped) || (ee->alpha)) */
/*           _ecore_evas_win32_region_border_resize(ee); */
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void
_ecore_evas_win32_move_resize(Ecore_Evas *ee, int x, int y, int width, int height)
{
   INF("ecore evas resize (%dx%d %dx%d)", x, y, width, height);

   if ((ee->w != width) || (ee->h != height) || (x != ee->x) || (y != ee->y))
     {
        int change_size = 0;
        int change_pos = 0;

        if ((ee->w != width) || (ee->h != height)) change_size = 1;
        if ((x != ee->x) || (y != ee->y)) change_pos = 1;

        ee->x = x;
        ee->y = y;
        ee->w = width;
        ee->h = height;
        ecore_win32_window_move_resize((struct _Ecore_Win32_Window *)ee->prop.window,
                                       x, y, width, height);
        if ((ee->rotation == 90) || (ee->rotation == 270))
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        if (ee->prop.avoid_damage)
          {
             int pdam;

             pdam = ecore_evas_avoid_damage_get(ee);
             ecore_evas_avoid_damage_set(ee, 0);
             ecore_evas_avoid_damage_set(ee, pdam);
          }
/*         if ((ee->shaped) || (ee->alpha)) */
/*           _ecore_evas_win32_region_border_resize(ee); */
        if (change_pos)
          {
             if (ee->func.fn_move) ee->func.fn_move(ee);
          }
        if (change_size)
          {
             if (ee->func.fn_resize) ee->func.fn_resize(ee);
          }
     }
}

static void
_ecore_evas_win32_rotation_set_internal(Ecore_Evas *ee, int rotation)
{
   int rot_dif;

   rot_dif = ee->rotation - rotation;
   if (rot_dif < 0) rot_dif = -rot_dif;

   if (rot_dif != 180)
     {
        int minw, minh, maxw, maxh, basew, baseh, stepw, steph;

        if (!ee->prop.fullscreen)
          {
             ecore_win32_window_resize((struct _Ecore_Win32_Window *)ee->prop.window,
                                       ee->h, ee->w);
             ee->expecting_resize.w = ee->h;
             ee->expecting_resize.h = ee->w;
          }
        else
          {
             int w, h;

             ecore_win32_window_size_get((struct _Ecore_Win32_Window *)ee->prop.window,
                                         &w, &h);
             ecore_win32_window_resize((struct _Ecore_Win32_Window *)ee->prop.window,
                                       h, w);
             if ((rotation == 0) || (rotation == 180))
               {
                  evas_output_size_set(ee->evas, ee->w, ee->h);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
               }
             else
               {
                  evas_output_size_set(ee->evas, ee->h, ee->w);
                  evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
               }
             if (ee->func.fn_resize) ee->func.fn_resize(ee);
          }
        ecore_evas_size_min_get(ee, &minw, &minh);
        ecore_evas_size_max_get(ee, &maxw, &maxh);
        ecore_evas_size_base_get(ee, &basew, &baseh);
        ecore_evas_size_step_get(ee, &stepw, &steph);
        ee->rotation = rotation;
        ecore_evas_size_min_set(ee, minh, minw);
        ecore_evas_size_max_set(ee, maxh, maxw);
        ecore_evas_size_base_set(ee, baseh, basew);
        ecore_evas_size_step_set(ee, steph, stepw);
        _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                       ecore_win32_current_time_get());
     }
   else
     {
        ee->rotation = rotation;
        _ecore_evas_mouse_move_process(ee, ee->mouse.x, ee->mouse.y,
                                       ecore_win32_current_time_get());
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }

   if ((ee->rotation == 90) || (ee->rotation == 270))
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
   else
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
}

static void
_ecore_evas_win32_rotation_set(Ecore_Evas *ee, int rotation, int resize)
{
   INF("ecore evas rotation: %s", rotation ? "yes" : "no");

   if (ee->rotation == rotation) return;

#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
   if (!strcmp(ee->driver, "software_gdi"))
     {
        Evas_Engine_Info_Software_Gdi *einfo;

        einfo = (Evas_Engine_Info_Software_Gdi *)evas_engine_info_get(ee->evas);
        if (!einfo) return;
        einfo->info.rotation = rotation;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        _ecore_evas_win32_rotation_set_internal(ee, rotation);
     }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_GDI */

#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
   if (!strcmp(ee->driver, "software_ddraw"))
     {
        Evas_Engine_Info_Software_DDraw *einfo;

        einfo = (Evas_Engine_Info_Software_DDraw *)evas_engine_info_get(ee->evas);
        if (!einfo) return;
        einfo->info.rotation = rotation;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        _ecore_evas_win32_rotation_set_internal(ee, rotation);
     }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_DDRAW */
}

static void
_ecore_evas_win32_shaped_set(Ecore_Evas *ee, int shaped)
{
   if (((ee->shaped) && (shaped)) || ((!ee->shaped) && (!shaped)))
     return;

   if (!strcmp(ee->driver, "software_ddraw")) return;

#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
   if (!strcmp(ee->driver, "software_gdi"))
     {
        Evas_Engine_Info_Software_Gdi *einfo;

        einfo = (Evas_Engine_Info_Software_Gdi *)evas_engine_info_get(ee->evas);
        ee->shaped = shaped;
        if (einfo)
          {
             ee->engine.win32.state.region = ee->shaped;
             einfo->info.region = ee->engine.win32.state.region;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
             if (ee->shaped)
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_GDI */
     }
}

static void
_ecore_evas_win32_show(Ecore_Evas *ee)
{
   INF("ecore evas show");

   ee->should_be_visible = 1;
   if (ee->prop.avoid_damage)
     _ecore_evas_win32_render(ee);
   ecore_win32_window_show((struct _Ecore_Win32_Window *)ee->prop.window);
/*    if (ee->prop.fullscreen) */
/*      ecore_win32_window_focus(ee->prop.window); */
}

static void
_ecore_evas_win32_hide(Ecore_Evas *ee)
{
   INF("ecore evas hide");

   ecore_win32_window_hide((struct _Ecore_Win32_Window *)ee->prop.window);
   ee->should_be_visible = 0;
}

static void
_ecore_evas_win32_raise(Ecore_Evas *ee)
{
   INF("ecore evas raise");

   if (!ee->prop.fullscreen)
     ecore_win32_window_raise((struct _Ecore_Win32_Window *)ee->prop.window);
   else
     ecore_win32_window_raise((struct _Ecore_Win32_Window *)ee->prop.window);
}

static void
_ecore_evas_win32_lower(Ecore_Evas *ee)
{
   INF("ecore evas lower");

   if (!ee->prop.fullscreen)
     ecore_win32_window_lower((struct _Ecore_Win32_Window *)ee->prop.window);
   else
     ecore_win32_window_lower((struct _Ecore_Win32_Window *)ee->prop.window);
}

static void
_ecore_evas_win32_activate(Ecore_Evas *ee)
{
   INF("ecore evas activate");

   ecore_win32_window_focus_set((struct _Ecore_Win32_Window *)ee->prop.window);
}

static void
_ecore_evas_win32_title_set(Ecore_Evas *ee, const char *title)
{
   INF("ecore evas title set");

   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
   ecore_win32_window_title_set((struct _Ecore_Win32_Window *)ee->prop.window,
                                ee->prop.title);
}

static void
_ecore_evas_win32_size_min_set(Ecore_Evas *ee, int width, int height)
{
   if (width < 0) width = 0;
   if (height < 0) height = 0;
   if ((ee->prop.min.w == width) && (ee->prop.min.h == height)) return;
   ee->prop.min.w = width;
   ee->prop.min.h = height;
   ecore_win32_window_size_min_set((struct _Ecore_Win32_Window *)ee->prop.window,
                                   width, height);
}

static void
_ecore_evas_win32_size_max_set(Ecore_Evas *ee, int width, int height)
{
   if (width < 0) width = 0;
   if (height < 0) height = 0;
   if ((ee->prop.max.w == width) && (ee->prop.max.h == height)) return;
   ee->prop.max.w = width;
   ee->prop.max.h = height;
   ecore_win32_window_size_max_set((struct _Ecore_Win32_Window *)ee->prop.window,
                                   width, height);
}

static void
_ecore_evas_win32_size_base_set(Ecore_Evas *ee, int width, int height)
{
   if (width < 0) width = 0;
   if (height < 0) height = 0;
   if ((ee->prop.base.w == width) && (ee->prop.base.h == height)) return;
   ee->prop.base.w = width;
   ee->prop.base.h = height;
   ecore_win32_window_size_base_set((struct _Ecore_Win32_Window *)ee->prop.window,
                                    width, height);
}

static void
_ecore_evas_win32_size_step_set(Ecore_Evas *ee, int width, int height)
{
   if (width < 1) width = 1;
   if (height < 1) height = 1;
   if ((ee->prop.step.w == width) && (ee->prop.step.h == height)) return;
   ee->prop.step.w = width;
   ee->prop.step.h = height;
   ecore_win32_window_size_step_set((struct _Ecore_Win32_Window *)ee->prop.window,
                                    width, height);
}

static void
_ecore_evas_win32_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
{
#if 0
   int x, y;

   if (ee->prop.cursor.object) evas_object_del(ee->prop.cursor.object);

   if (obj == NULL)
     {
        ee->prop.cursor.object = NULL;
        ee->prop.cursor.layer = 0;
        ee->prop.cursor.hot.x = 0;
        ee->prop.cursor.hot.y = 0;
        ecore_win32_window_cursor_show(ee->prop.window, 1);
        return;
     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   ecore_win32_window_cursor_show(ee->prop.window, 0);

   evas_pointer_output_xy_get(ee->evas, &x, &y);
   evas_object_layer_set(ee->prop.cursor.object, ee->prop.cursor.layer);
   evas_object_move(ee->prop.cursor.object,
                    x - ee->prop.cursor.hot.x,
                    y - ee->prop.cursor.hot.y);
   evas_object_pass_events_set(ee->prop.cursor.object, 1);
   if (evas_pointer_inside_get(ee->evas))
     evas_object_show(ee->prop.cursor.object);
#endif
}

static void
_ecore_evas_win32_focus_set(Ecore_Evas *ee, int on __UNUSED__)
{
   ecore_win32_window_focus_set((struct _Ecore_Win32_Window *)ee->prop.window);
}

static void
_ecore_evas_win32_iconified_set(Ecore_Evas *ee, int on)
{
/*    if (((ee->prop.borderless) && (on)) || */
/*        ((!ee->prop.borderless) && (!on))) return; */
   ee->prop.iconified = on;
   ecore_win32_window_iconified_set((struct _Ecore_Win32_Window *)ee->prop.window,
                                    ee->prop.iconified);
}

static void
_ecore_evas_win32_borderless_set(Ecore_Evas *ee, int on)
{
   if (((ee->prop.borderless) && (on)) ||
       ((!ee->prop.borderless) && (!on))) return;
   ee->prop.borderless = on;
   ecore_win32_window_borderless_set((struct _Ecore_Win32_Window *)ee->prop.window,
                                     ee->prop.borderless);

#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
   if (!strcmp(ee->driver, "software_gdi"))
     {
        Evas_Engine_Info_Software_Gdi *einfo;

        einfo = (Evas_Engine_Info_Software_Gdi *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
            einfo->info.borderless = ee->prop.borderless;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
             if (ee->prop.borderless)
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
     }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_GDI */
}

static void
_ecore_evas_win32_fullscreen_set(Ecore_Evas *ee, int on)
{
   struct _Ecore_Win32_Window *window;

   INF("ecore evas fullscreen set");

   if ((ee->engine.win32.state.fullscreen && on) ||
      (!ee->engine.win32.state.fullscreen && !on))
     return;

   ee->engine.win32.state.fullscreen = on;
   ee->prop.fullscreen = on;

   window = (struct _Ecore_Win32_Window *)ee->prop.window;

   if (on != 0)
   {
      ecore_win32_window_shape_set((struct _Ecore_Win32_Window *)ee->prop.window,
                                   0, 0, NULL);
      ecore_win32_window_fullscreen_set((struct _Ecore_Win32_Window *)ee->prop.window,
                                        on);
   }
   else
   {
      ecore_win32_window_fullscreen_set(window, on);
      ecore_win32_window_shape_set(window,
                                   window->shape.width,
                                   window->shape.height,
                                   window->shape.mask);
   }

   /* Nothing to be done for the GDI backend at the evas level */

#ifdef BUILD_ECORE_EVAS_SOFTWRE_DDRAW
   if (strcmp(ee->driver, "software_ddraw") == 0)
     {
        Evas_Engine_Info_Software_DDraw *einfo;

        einfo = (Evas_Engine_Info_Software_DDraw *)evas_engine_info_get(ecore_evas_get(ee));
        if (einfo)
          {
             einfo->info.fullscreen = !!on;
/*           einfo->info.layered = window->shape.layered; */
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
          }
     }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_DDRAW */

#ifdef BUILD_ECORE_EVAS_DIRECT3D
   if (strcmp(ee->driver, "direct3d") == 0)
     {
        Evas_Engine_Info_Direct3D *einfo;

        einfo = (Evas_Engine_Info_Direct3D *)evas_engine_info_get(ecore_evas_get(ee));
        if (einfo)
          {
             einfo->info.fullscreen = !!on;
             einfo->info.layered = window->shape.layered;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
          }
     }
#endif /* BUILD_ECORE_EVAS_DIRECT3D */
}


static Ecore_Evas_Engine_Func _ecore_win32_engine_func =
{
   _ecore_evas_win32_free,
     NULL,
     NULL,
     NULL,
     NULL,
     _ecore_evas_win32_callback_delete_request_set,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     _ecore_evas_win32_move,
     NULL,
     _ecore_evas_win32_resize,
     _ecore_evas_win32_move_resize,
     _ecore_evas_win32_rotation_set,
     _ecore_evas_win32_shaped_set,
     _ecore_evas_win32_show,
     _ecore_evas_win32_hide,
     _ecore_evas_win32_raise,
     _ecore_evas_win32_lower,
     _ecore_evas_win32_activate,
     _ecore_evas_win32_title_set,
     NULL, /* _ecore_evas_x_name_class_set */
     _ecore_evas_win32_size_min_set,
     _ecore_evas_win32_size_max_set,
     _ecore_evas_win32_size_base_set,
     _ecore_evas_win32_size_step_set,
     _ecore_evas_win32_cursor_set,
     NULL, /* _ecore_evas_x_layer_set */
     _ecore_evas_win32_focus_set,
     _ecore_evas_win32_iconified_set,
     _ecore_evas_win32_borderless_set,
     NULL, /* _ecore_evas_x_override_set */
     NULL,
     _ecore_evas_win32_fullscreen_set,
     NULL, /* _ecore_evas_x_avoid_damage_set */
     NULL, /* _ecore_evas_x_withdrawn_set */
     NULL, /* _ecore_evas_x_sticky_set */
     NULL, /* _ecore_evas_x_ignore_events_set */
     NULL,  /* _ecore_evas_x_alpha_set */
     NULL, //transparent

     NULL, // render
     NULL  //screen_geometry_get
};

#endif /* BUILD_ECORE_EVAS_WIN32 */

/* API */

#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
static int
_ecore_evas_engine_software_gdi_init(Ecore_Evas *ee)
{
   Evas_Engine_Info_Software_Gdi *einfo;
   const char                    *driver;
   int                            rmethod;

   driver = "software_gdi";

   rmethod = evas_render_method_lookup(driver);
   if (!rmethod)
     return 0;

   ee->driver = driver;
   evas_output_method_set(ee->evas, rmethod);

   einfo = (Evas_Engine_Info_Software_Gdi *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        /* FIXME: REDRAW_DEBUG missing for now */
        einfo->info.window = ((struct _Ecore_Win32_Window *)ee->prop.window)->window;
        einfo->info.depth = ecore_win32_screen_depth_get();
        einfo->info.rotation = 0;
        einfo->info.borderless = 0;
        einfo->info.fullscreen = 0;
        einfo->info.region = 0;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             return 0;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        return 0;
     }

   return 1;
}
#endif /* BUILD_ECORE_EVAS_SOFTWARE_GDI */

#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
static int
_ecore_evas_engine_software_ddraw_init(Ecore_Evas *ee)
{
   Evas_Engine_Info_Software_DDraw *einfo;
   const char                      *driver;
   int                              rmethod;

   driver = "software_ddraw";

   rmethod = evas_render_method_lookup(driver);
   if (!rmethod)
     return 0;

   ee->driver = driver;
   evas_output_method_set(ee->evas, rmethod);

   einfo = (Evas_Engine_Info_Software_DDraw *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        /* FIXME: REDRAW_DEBUG missing for now */
        einfo->info.window = ((struct _Ecore_Win32_Window *)ee->prop.window)->window;
        einfo->info.depth = ecore_win32_screen_depth_get();
        einfo->info.rotation = 0;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             return 0;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        return 0;
     }

   return 1;
}
#endif /* BUILD_ECORE_EVAS_SOFTWARE_DDRAW */

#ifdef BUILD_ECORE_EVAS_DIRECT3D
static int
_ecore_evas_engine_direct3d_init(Ecore_Evas *ee)
{
   Evas_Engine_Info_Direct3D *einfo;
   const char                *driver;
   int                        rmethod;

   driver = "direct3d";

   rmethod = evas_render_method_lookup(driver);
   if (!rmethod)
     return 0;

   ee->driver = driver;
   evas_output_method_set(ee->evas, rmethod);

   einfo = (Evas_Engine_Info_Direct3D *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        /* FIXME: REDRAW_DEBUG missing for now */
        einfo->info.window = ((struct _Ecore_Win32_Window *)ee->prop.window)->window;
        einfo->info.depth = ecore_win32_screen_depth_get();
        einfo->info.rotation = 0;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             return 0;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        return 0;
     }

   return 1;
}
#endif /* BUILD_ECORE_EVAS_DIRECT3D */

#ifdef BUILD_ECORE_EVAS_OPENGL_GLEW
static int
_ecore_evas_engine_opengl_glew_init(Ecore_Evas *ee)
{
   Evas_Engine_Info_GL_Glew *einfo;
   const char               *driver;
   int                       rmethod;

   driver = "gl_glew";

   rmethod = evas_render_method_lookup(driver);
   if (!rmethod)
     return 0;

   ee->driver = driver;
   evas_output_method_set(ee->evas, rmethod);

   einfo = (Evas_Engine_Info_GL_Glew *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        /* FIXME: REDRAW_DEBUG missing for now */
        einfo->info.window = ((struct _Ecore_Win32_Window *)ee->prop.window)->window;
        einfo->info.depth = ecore_win32_screen_depth_get();
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             return 0;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        return 0;
     }

   return 1;
}
#endif /* BUILD_ECORE_EVAS_OPENGL_GLEW */

#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_DDRAW
static int
_ecore_evas_engine_software_16_ddraw_init(Ecore_Evas *ee)
{
   Evas_Engine_Info_Software_DDraw *einfo;
   const char                      *driver;
   int                              rmethod;

   driver = "software_16_ddraw";

   rmethod = evas_render_method_lookup(driver);
   if (!rmethod)
     return 0;

   ee->driver = driver;
   evas_output_method_set(ee->evas, rmethod);

   if (ecore_win32_screen_depth_get() != 16)
     return 0;

   einfo = (Evas_Engine_Info_Software_16_DDraw *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        /* FIXME: REDRAW_DEBUG missing for now */
        einfo->info.window = ((struct _Ecore_Win32_Window *)ee->prop.window)->window;
        einfo->info.depth = ecore_win32_screen_depth_get();
        einfo->info.rotation = 0;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             return 0;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        return 0;
     }

   return 1;
}
#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_DDRAW */

#ifdef BUILD_ECORE_EVAS_WIN32
static Ecore_Evas *
_ecore_evas_win32_new_internal(int (*_ecore_evas_engine_init)(Ecore_Evas *ee),
                               Ecore_Win32_Window *parent,
                               int                 x,
                               int                 y,
                               int                 width,
                               int                 height)
{
   Ecore_Evas *ee;

   if (!ecore_win32_init())
     return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee)
     return NULL;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_win32_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_win32_engine_func;

   if (width < 1) width = 1;
   if (height < 1) height = 1;
   ee->x = x;
   ee->y = y;
   ee->w = width;
   ee->h = height;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   /* FIXME: sticky to add */
   ee->prop.window = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_size_set(ee->evas, width, height);
   evas_output_viewport_set(ee->evas, 0, 0, width, height);

   ee->engine.win32.parent = parent;
   ee->prop.window = (Ecore_Window)ecore_win32_window_new(parent, x, y, width, height);
   if (!ee->prop.window)
     {
        _ecore_evas_win32_shutdown();
        free(ee);
        return NULL;
     }

   if (!_ecore_evas_engine_init(ee))
     {
        _ecore_evas_win32_shutdown();
        free(ee);
        return NULL;
     }

   ee->engine.func->fn_render = _ecore_evas_win32_render;
   _ecore_evas_register(ee);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);

   return ee;
}

#endif /* BUILD_ECORE_EVAS_WIN32 */

#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI

EAPI Ecore_Evas *
ecore_evas_software_gdi_new(Ecore_Win32_Window *parent,
                            int                 x,
                            int                 y,
                            int                 width,
                            int                 height)
{
   return _ecore_evas_win32_new_internal(_ecore_evas_engine_software_gdi_init,
                                         parent,
                                         x,
                                         y,
                                         width,
                                         height);
}

#else

EAPI Ecore_Evas *
ecore_evas_software_gdi_new(Ecore_Win32_Window *parent __UNUSED__,
                            int                 x __UNUSED__,
                            int                 y __UNUSED__,
                            int                 width __UNUSED__,
                            int                 height __UNUSED__)
{
   return NULL;
}

#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_GDI */

#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW

EAPI Ecore_Evas *
ecore_evas_software_ddraw_new(Ecore_Win32_Window *parent,
                              int                 x,
                              int                 y,
                              int                 width,
                              int                 height)
{
   return _ecore_evas_win32_new_internal(_ecore_evas_engine_software_ddraw_init,
                                         parent,
                                         x,
                                         y,
                                         width,
                                         height);
}

#else

EAPI Ecore_Evas *
ecore_evas_software_ddraw_new(Ecore_Win32_Window *parent __UNUSED__,
                              int                 x __UNUSED__,
                              int                 y __UNUSED__,
                              int                 width __UNUSED__,
                              int                 height __UNUSED__)
{
   return NULL;
}

#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_DDRAW */


#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_DDRAW

EAPI Ecore_Evas *
ecore_evas_software_16_ddraw_new(Ecore_Win32_Window *parent,
                                 int                 x,
                                 int                 y,
                                 int                 width,
                                 int                 height)
{
   return _ecore_evas_win32_new_internal(_ecore_evas_engine_software_16_ddraw_init,
                                         parent,
                                         x,
                                         y,
                                         width,
                                         height);
}

#else

EAPI Ecore_Evas *
ecore_evas_software_16_ddraw_new(Ecore_Win32_Window *parent __UNUSED__,
                                 int                 x __UNUSED__,
                                 int                 y __UNUSED__,
                                 int                 width __UNUSED__,
                                 int                 height __UNUSED__)
{
   return NULL;
}

#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_16_DDRAW */


#ifdef BUILD_ECORE_EVAS_DIRECT3D

EAPI Ecore_Evas *
ecore_evas_direct3d_new(Ecore_Win32_Window *parent,
                        int                 x,
                        int                 y,
                        int                 width,
                        int                 height)
{
   return _ecore_evas_win32_new_internal(_ecore_evas_engine_direct3d_init,
                                         parent,
                                         x,
                                         y,
                                         width,
                                         height);
}

#else

EAPI Ecore_Evas *
ecore_evas_direct3d_new(Ecore_Win32_Window *parent __UNUSED__,
                        int                 x __UNUSED__,
                        int                 y __UNUSED__,
                        int                 width __UNUSED__,
                        int                 height __UNUSED__)
{
   return NULL;
}

#endif /* ! BUILD_ECORE_EVAS_DIRECT3D */


#ifdef BUILD_ECORE_EVAS_OPENGL_GLEW

EAPI Ecore_Evas *
ecore_evas_gl_glew_new(Ecore_Win32_Window *parent,
                       int                 x,
                       int                 y,
                       int                 width,
                       int                 height)
{
   return _ecore_evas_win32_new_internal(_ecore_evas_engine_opengl_glew_init,
                                         parent,
                                         x,
                                         y,
                                         width,
                                         height);
}

#else

EAPI Ecore_Evas *
ecore_evas_gl_glew_new(Ecore_Win32_Window *parent __UNUSED__,
                       int                 x __UNUSED__,
                       int                 y __UNUSED__,
                       int                 width __UNUSED__,
                       int                 height __UNUSED__)
{
   return NULL;
}

#endif /* BUILD_ECORE_EVAS_OPENGL_GLEW */


#ifdef BUILD_ECORE_EVAS_WIN32

EAPI Ecore_Win32_Window *
ecore_evas_win32_window_get(const Ecore_Evas *ee)
{
   return (Ecore_Win32_Window *) ecore_evas_window_get(ee);
}

#else

EAPI Ecore_Win32_Window *
ecore_evas_win32_window_get(const Ecore_Evas *ee __UNUSED__)
{
   return NULL;
}

#endif /* BUILD_ECORE_EVAS_WIN32 */
