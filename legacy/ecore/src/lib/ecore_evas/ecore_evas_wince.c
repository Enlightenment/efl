#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* for NULL */

#include <Ecore.h>
#include "ecore_private.h"
#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
# include <Ecore_WinCE.h>
# include <ecore_wince_private.h>
#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_WINCE */

#include "ecore_evas_private.h"
#include "Ecore_Evas.h"

#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE

#define ECORE_EVAS_EVENT_COUNT 7

static int _ecore_evas_init_count = 0;

static Ecore_Event_Handler *ecore_evas_event_handlers[ECORE_EVAS_EVENT_COUNT];

static Eina_Bool _ecore_evas_wince_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_wince_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_wince_event_window_damage(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_wince_event_window_destroy(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_wince_event_window_show(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_wince_event_window_hide(void *data __UNUSED__, int type __UNUSED__, void *event);

static Eina_Bool _ecore_evas_wince_event_window_delete_request(void *data __UNUSED__, int type __UNUSED__, void *event);

/* Private functions */

static int
_ecore_evas_wince_render(Ecore_Evas *ee)
{
   int rend = 0;
   Eina_List *updates = NULL;
   Eina_List *ll;
   Ecore_Evas *ee2;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
          rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }

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
_ecore_evas_wince_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1)
     return _ecore_evas_init_count;

   ecore_evas_event_handlers[0]  = ecore_event_handler_add(ECORE_WINCE_EVENT_MOUSE_IN, _ecore_evas_wince_event_mouse_in, NULL);
   ecore_evas_event_handlers[1]  = ecore_event_handler_add(ECORE_WINCE_EVENT_MOUSE_OUT, _ecore_evas_wince_event_mouse_out, NULL);
   ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_DAMAGE, _ecore_evas_wince_event_window_damage, NULL);
   ecore_evas_event_handlers[3]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_DESTROY, _ecore_evas_wince_event_window_destroy, NULL);
   ecore_evas_event_handlers[4]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_SHOW, _ecore_evas_wince_event_window_show, NULL);
   ecore_evas_event_handlers[5]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_HIDE, _ecore_evas_wince_event_window_hide, NULL);
   ecore_evas_event_handlers[6]  = ecore_event_handler_add(ECORE_WINCE_EVENT_WINDOW_DELETE_REQUEST, _ecore_evas_wince_event_window_delete_request, NULL);

   ecore_event_evas_init();
   return _ecore_evas_init_count;
}

int
_ecore_evas_wince_shutdown(void)
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
_ecore_evas_wince_event_mouse_in(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                 *ee;
   Ecore_WinCE_Event_Mouse_In *e;

   INF("mouse in");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   if (e->window != (Ecore_WinCE_Window *)ee->prop.window) return 1;

   if (ee->func.fn_mouse_in) ee->func.fn_mouse_in(ee);
   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   evas_event_feed_mouse_in(ee->evas, e->time, NULL);
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);

   return 1;
}

static Eina_Bool
_ecore_evas_wince_event_mouse_out(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                  *ee;
   Ecore_WinCE_Event_Mouse_Out *e;

   INF("mouse out");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if ((!ee) || (ee->ignore_events)) return 1; /* pass on event */
   if (e->window != (Ecore_WinCE_Window *)ee->prop.window) return 1;

   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->time);

   evas_event_feed_mouse_out(ee->evas, e->time, NULL);
   if (ee->func.fn_mouse_out) ee->func.fn_mouse_out(ee);
   if (ee->prop.cursor.object) evas_object_hide(ee->prop.cursor.object);

   return 1;
}

static Eina_Bool
_ecore_evas_wince_event_window_damage(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                      *ee;
   Ecore_WinCE_Event_Window_Damage *e;

   INF("window damage");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != (Ecore_WinCE_Window *)ee->prop.window) return 1;

   if (ee->prop.avoid_damage)
     {
#warning [ECORE] [WINCE] No Region code
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
_ecore_evas_wince_event_window_destroy(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                       *ee;
   Ecore_WinCE_Event_Window_Destroy *e;

   INF("window destroy");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != (Ecore_WinCE_Window *)ee->prop.window) return 1;
   if (ee->func.fn_destroy) ee->func.fn_destroy(ee);
   ecore_evas_free(ee);

   return 1;
}

static Eina_Bool
_ecore_evas_wince_event_window_show(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                    *ee;
   Ecore_WinCE_Event_Window_Show *e;

   INF("window show");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != (Ecore_WinCE_Window *)ee->prop.window) return 1;
   if (ee->visible) return 0; /* dont pass it on */
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);

   return 1;
}

static Eina_Bool
_ecore_evas_wince_event_window_hide(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                    *ee;
   Ecore_WinCE_Event_Window_Hide *e;

   INF("window hide");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != (Ecore_WinCE_Window *)ee->prop.window) return 1;
   if (!ee->visible) return 0; /* dont pass it on */
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);

   return 1;
}

static Eina_Bool
_ecore_evas_wince_event_window_delete_request(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Evas                              *ee;
   Ecore_WinCE_Event_Window_Delete_Request *e;

   INF("window delete request");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return 1; /* pass on event */
   if (e->window != (Ecore_WinCE_Window *)ee->prop.window) return 1;
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);

   return 1;
}


/* Ecore_Evas interface */

static void
_ecore_evas_wince_free(Ecore_Evas *ee)
{
   INF("ecore evas free");

   ecore_wince_window_free((Ecore_WinCE_Window *)ee->prop.window);
   ecore_event_window_unregister(ee->prop.window);
   _ecore_evas_wince_shutdown();
   ecore_wince_shutdown();
}

static void
_ecore_evas_wince_callback_delete_request_set(Ecore_Evas *ee,
                                              void (*func) (Ecore_Evas *ee))
{
   ee->func.fn_delete_request = func;
}

static void
_ecore_evas_wince_move(Ecore_Evas *ee, int x, int y)
{
  INF("ecore evas move (%dx%d)", x, y);
   ee->req.x = x;
   ee->req.y = y;

   if ((x != ee->x) || (y != ee->y))
     {
        ee->x = x;
        ee->y = y;
        ecore_wince_window_move((Ecore_WinCE_Window *)ee->prop.window, x, y);
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }
}

static void
_ecore_evas_wince_resize(Ecore_Evas *ee, int width, int height)
{
   INF("ecore evas resize (%dx%d)", width, height);
   ee->req.w = width;
   ee->req.h = height;

   if ((ee->w != width) || (ee->h != height))
     {
        ee->w = width;
        ee->h = height;
        ecore_wince_window_resize((Ecore_WinCE_Window *)ee->prop.window, width, height);
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
        /* FIXME: damage and shape */

        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void
_ecore_evas_wince_move_resize(Ecore_Evas *ee, int x, int y, int width, int height)
{
   INF("ecore evas resize (%dx%d %dx%d)", x, y, width, height);
   ee->req.x = x;
   ee->req.y = y;
   ee->req.w = width;
   ee->req.h = height;

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
        ecore_wince_window_move_resize((Ecore_WinCE_Window *)ee->prop.window, x, y, width, height);
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
        /* FIXME: damage and shape */
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

/* static void */
/* _ecore_evas_wince_rotation_set(Ecore_Evas *ee, int rotation) */
/* { */
/*    int rot_dif; */

/*    if (ee->rotation == rotation) return; */
/*    rot_dif = ee->rotation - rotation; */
/*    if (rot_dif < 0) rot_dif = -rot_dif; */
/*    if (!strcmp(ee->driver, "software_ddraw")) */
/*      { */
/*        Evas_Engine_Info_Software_16_WinCE *einfo; */

/*        einfo = (Evas_Engine_Info_Software_16_WinCE *)evas_engine_info_get(ee->evas); */
/*        if (!einfo) return; */
/*        if (rot_dif != 180) */
/*          { */
/*             int minw, minh, maxw, maxh, basew, baseh, stepw, steph; */

/*             einfo->info.rotation = rotation; */
/*             evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo); */
/*             if (!ee->prop.fullscreen) */
/*               { */
/*                  ecore_wince_window_resize(ee->prop.window, ee->h, ee->w); */
/*                  ee->expecting_resize.w = ee->h; */
/*                  ee->expecting_resize.h = ee->w; */
/*               } */
/*             else */
/*               { */
/*                  int w, h; */

/*                  ecore_wince_window_size_get(ee->prop.window, &w, &h); */
/*                  ecore_wince_window_resize(ee->prop.window, h, w); */
/*                  if ((rotation == 0) || (rotation == 180)) */
/*                    { */
/*                       evas_output_size_set(ee->evas, ee->w, ee->h); */
/*                       evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h); */
/*                    } */
/*                  else */
/*                    { */
/*                       evas_output_size_set(ee->evas, ee->h, ee->w); */
/*                       evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w); */
/*                    } */
/*                  if (ee->func.fn_resize) ee->func.fn_resize(ee); */
/*               } */
/*             ecore_evas_size_min_get(ee, &minw, &minh); */
/*             ecore_evas_size_max_get(ee, &maxw, &maxh); */
/*             ecore_evas_size_base_get(ee, &basew, &baseh); */
/*             ecore_evas_size_step_get(ee, &stepw, &steph); */
/*             ee->rotation = rotation; */
/*             ecore_evas_size_min_set(ee, minh, minw); */
/*             ecore_evas_size_max_set(ee, maxh, maxw); */
/*             ecore_evas_size_base_set(ee, baseh, basew); */
/*             ecore_evas_size_step_set(ee, steph, stepw); */
/*             _ecore_evas_wince_mouse_move_process(ee, ee->mouse.x, ee->mouse.y, */
/*                                                   ecore_wince_current_time_get()); */
/*          } */
/*        else */
/*          { */
/*             einfo->info.rotation = rotation; */
/*             evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo); */
/*             ee->rotation = rotation; */
/*             _ecore_evas_wince_mouse_move_process(ee, ee->mouse.x, ee->mouse.y, */
/*                                                   ecore_wince_current_time_get()); */
/*             if (ee->func.fn_resize) ee->func.fn_resize(ee); */
/*          } */
/*        if ((ee->rotation == 90) || (ee->rotation == 270)) */
/*          evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w); */
/*        else */
/*          evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h); */
/*      } */
/* } */

static void
_ecore_evas_wince_show(Ecore_Evas *ee)
{
   INF("ecore evas show");

   ee->should_be_visible = 1;
   if (ee->prop.avoid_damage)
     _ecore_evas_wince_render(ee);
   ecore_wince_window_show((Ecore_WinCE_Window *)ee->prop.window);
/*    if (ee->prop.fullscreen) */
/*      ecore_wince_window_focus(ee->prop.window); */
}

static void
_ecore_evas_wince_hide(Ecore_Evas *ee)
{
   INF("ecore evas hide");

   ecore_wince_window_hide((Ecore_WinCE_Window *)ee->prop.window);
   ee->should_be_visible = 0;
}

/* static void */
/* _ecore_evas_wince_raise(Ecore_Evas *ee) */
/* { */
/*    if (!ee->prop.fullscreen) */
/*      ecore_wince_window_raise(ee->prop.window); */
/*    else */
/*      ecore_wince_window_raise(ee->prop.window); */
/* } */

/* static void */
/* _ecore_evas_wince_lower(Ecore_Evas *ee) */
/* { */
/*    if (!ee->prop.fullscreen) */
/*      ecore_wince_window_lower(ee->prop.window); */
/*    else */
/*      ecore_wince_window_lower(ee->prop.window); */
/* } */

static void
_ecore_evas_wince_title_set(Ecore_Evas *ee, const char *title)
{
   INF("ecore evas title set");

   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
   ecore_wince_window_title_set((Ecore_WinCE_Window *)ee->prop.window, ee->prop.title);
}

/* static void */
/* _ecore_evas_wince_size_min_set(Ecore_Evas *ee, int width, int height) */
/* { */
/*    if (width < 0) width = 0; */
/*    if (height < 0) height = 0; */
/*    if ((ee->prop.min.w == width) && (ee->prop.min.h == height)) return; */
/*    ee->prop.min.w = width; */
/*    ee->prop.min.h = height; */
/*    ecore_wince_window_size_min_set(ee->prop.window, width, height); */
/* } */

/* static void */
/* _ecore_evas_wince_size_max_set(Ecore_Evas *ee, int width, int height) */
/* { */
/*    if (width < 0) width = 0; */
/*    if (height < 0) height = 0; */
/*    if ((ee->prop.max.w == width) && (ee->prop.max.h == height)) return; */
/*    ee->prop.max.w = width; */
/*    ee->prop.max.h = height; */
/*    ecore_wince_window_size_max_set(ee->prop.window, width, height); */
/* } */

/* static void */
/* _ecore_evas_wince_size_base_set(Ecore_Evas *ee, int width, int height) */
/* { */
/*    if (width < 0) width = 0; */
/*    if (height < 0) height = 0; */
/*    if ((ee->prop.base.w == width) && (ee->prop.base.h == height)) return; */
/*    ee->prop.base.w = width; */
/*    ee->prop.base.h = height; */
/*    ecore_wince_window_size_base_set(ee->prop.window, width, height); */
/* } */

/* static void */
/* _ecore_evas_wince_size_step_set(Ecore_Evas *ee, int width, int height) */
/* { */
/*    if (width < 1) width = 1; */
/*    if (height < 1) height = 1; */
/*    if ((ee->prop.step.w == width) && (ee->prop.step.h == height)) return; */
/*    ee->prop.step.w = width; */
/*    ee->prop.step.h = height; */
/*    ecore_wince_window_size_step_set(ee->prop.window, width, height); */
/* } */

static void
_ecore_evas_wince_cursor_set(Ecore_Evas *ee, Evas_Object *obj, int layer, int hot_x, int hot_y)
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
        ecore_wince_window_cursor_show(ee->prop.window, 1);
        return;
     }

   ee->prop.cursor.object = obj;
   ee->prop.cursor.layer = layer;
   ee->prop.cursor.hot.x = hot_x;
   ee->prop.cursor.hot.y = hot_y;

   ecore_wince_window_cursor_show(ee->prop.window, 0);

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

/* static void */
/* _ecore_evas_wince_focus_set(Ecore_Evas *ee, int on __UNUSED__) */
/* { */
/*    ecore_wince_window_focus_set(ee->prop.window); */
/* } */

/* static void */
/* _ecore_evas_wince_iconified_set(Ecore_Evas *ee, int on) */
/* { */
/* /\*    if (((ee->prop.borderless) && (on)) || *\/ */
/* /\*        ((!ee->prop.borderless) && (!on))) return; *\/ */
/*    ee->prop.iconified = on; */
/*    ecore_wince_window_iconified_set(ee->prop.window, ee->prop.iconified); */
/* } */

/* static void */
/* _ecore_evas_wince_borderless_set(Ecore_Evas *ee, int on) */
/* { */
/*    if (((ee->prop.borderless) && (on)) || */
/*        ((!ee->prop.borderless) && (!on))) return; */
/*    ee->prop.borderless = on; */
/*    ecore_wince_window_borderless_set(ee->prop.window, ee->prop.borderless); */
/* } */

static void
_ecore_evas_wince_fullscreen_set(Ecore_Evas *ee, int on)
{
   Evas_Engine_Info_Software_16_WinCE *einfo;
   struct _Ecore_WinCE_Window         *window;

   INF("ecore evas fullscreen set");

   if ((ee->engine.wince.state.fullscreen && on) ||
      (!ee->engine.wince.state.fullscreen && !on))
     return;

   ee->engine.wince.state.fullscreen = on;
   ee->prop.fullscreen = on;

   window = (struct _Ecore_WinCE_Window *)ee->prop.window;

   if (on != 0)
   {
/*       ecore_win32_window_shape_set(ee->engine.win32.window, 0, 0, NULL); */
      ecore_wince_window_fullscreen_set((Ecore_WinCE_Window *)ee->prop.window, on);
      ee->w = GetSystemMetrics(SM_CXSCREEN);
      ee->h = GetSystemMetrics(SM_CYSCREEN);
      ee->req.w = ee->w;
      ee->req.h = ee->h;
      evas_output_size_set(ee->evas, ee->w, ee->h);
      evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   }
   else
   {
      int w;
      int h;

      ecore_wince_window_fullscreen_set((Ecore_WinCE_Window *)ee->prop.window, on);
      ecore_wince_window_size_get((Ecore_WinCE_Window *)ee->prop.window, &w, &h);
      ee->w = w;
      ee->h = h;
      ee->req.w = ee->w;
      ee->req.h = ee->h;
      evas_output_size_set(ee->evas, ee->w, ee->h);
      evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
/*       ecore_win32_window_shape_set(window, */
/*                                    window->shape.width, */
/*                                    window->shape.height, */
/*                                    window->shape.mask); */
   }

   einfo = (Evas_Engine_Info_Software_16_WinCE *)evas_engine_info_get(ecore_evas_get(ee));
   if (einfo)
     {
        einfo->info.fullscreen = !!on;
/*         einfo->info.layered = window->shape.layered; */
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
     }
}

static Ecore_Evas_Engine_Func _ecore_wince_engine_func =
{
   _ecore_evas_wince_free,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_wince_callback_delete_request_set,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   NULL,
   _ecore_evas_wince_move,
   NULL,
   _ecore_evas_wince_resize,
   _ecore_evas_wince_move_resize,
   NULL, //_ecore_evas_wince_rotation_set,
   NULL, /* _ecore_evas_x_shaped_set */
   _ecore_evas_wince_show,
   _ecore_evas_wince_hide,
   NULL, //_ecore_evas_wince_raise,
   NULL, //_ecore_evas_wince_lower,
   NULL, //_ecore_evas_wince_activate,
   _ecore_evas_wince_title_set,
   NULL, /* _ecore_evas_x_name_class_set */
   NULL, //_ecore_evas_wince_size_min_set,
   NULL, //_ecore_evas_wince_size_max_set,
   NULL, //_ecore_evas_wince_size_base_set,
   NULL, //_ecore_evas_wince_size_step_set,
   _ecore_evas_wince_cursor_set,
   NULL, /* _ecore_evas_x_layer_set */
   NULL, //_ecore_evas_wince_focus_set,
   NULL, //_ecore_evas_wince_iconified_set,
   NULL, //_ecore_evas_wince_borderless_set,
   NULL, /* _ecore_evas_x_override_set */
   NULL,
   _ecore_evas_wince_fullscreen_set,
   NULL, /* _ecore_evas_x_avoid_damage_set */
   NULL, /* _ecore_evas_x_withdrawn_set */
   NULL, /* _ecore_evas_x_sticky_set */
   NULL, /* _ecore_evas_x_ignore_events_set */
   NULL, /* _ecore_evas_x_alpha_set */
   NULL, //transparent

   NULL, // render
   NULL  // screen_geometry_get
};

/* API */

static Ecore_Evas *
ecore_evas_software_wince_new_internal(int                 backend,
                                       Ecore_WinCE_Window *parent,
                                       int                 x,
                                       int                 y,
                                       int                 width,
                                       int                 height,
                                       int                 fullscreen)
{
   Evas_Engine_Info_Software_16_WinCE *einfo;
   Ecore_Evas                         *ee;
   int                                 rmethod;

   rmethod = evas_render_method_lookup("software_16_wince");
   if (!rmethod)
     return NULL;

   if (!ecore_wince_init())
     return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee)
     {
        ecore_wince_shutdown();
        return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   if (!_ecore_evas_wince_init())
     {
        free(ee);
        ecore_wince_shutdown();
        return NULL;
     }

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_wince_engine_func;

   ee->driver = "software_16_wince";

   if (width < 1) width = 1;
   if (height < 1) height = 1;
   ee->x = x;
   ee->y = y;
   ee->w = width;
   ee->h = height;
   ee->req.x = ee->x;
   ee->req.y = ee->y;
   ee->req.w = ee->w;
   ee->req.h = ee->h;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = 0;
   ee->prop.sticky = 0;
   /* FIXME: sticky to add */

   ee->prop.window = (Ecore_Window)ecore_wince_window_new((Ecore_WinCE_Window *)parent, x, y, width, height);
   if (!ee->prop.window)
     {
        _ecore_evas_wince_shutdown();
        free(ee);
        ecore_wince_shutdown();
        return NULL;
     }

   ecore_wince_window_fullscreen_set((Ecore_WinCE_Window *)ee->prop.window, fullscreen);

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, width, height);
   evas_output_viewport_set(ee->evas, 0, 0, width, height);

   einfo = (Evas_Engine_Info_Software_16_WinCE *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        /* FIXME: REDRAW_DEBUG missing for now */
        einfo->info.window = ((struct _Ecore_WinCE_Window *)ee->prop.window)->window;
        einfo->info.width = width;
        einfo->info.height = height;
        einfo->info.backend = backend;
        einfo->info.rotation = 0;
        einfo->info.fullscreen = fullscreen;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             _ecore_evas_wince_shutdown();
             free(ee);
             ecore_wince_shutdown();
             return NULL;
          }

        ecore_wince_window_backend_set((Ecore_WinCE_Window *)ee->prop.window, backend);
        ecore_wince_window_suspend_cb_set((Ecore_WinCE_Window *)ee->prop.window, einfo->func.suspend);
        ecore_wince_window_resume_cb_set((Ecore_WinCE_Window *)ee->prop.window, einfo->func.resume);
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        _ecore_evas_wince_shutdown();
        free(ee);
        ecore_wince_shutdown();
        return NULL;
     }

   ee->engine.func->fn_render = _ecore_evas_wince_render;
   _ecore_evas_register(ee);
   ecore_event_window_register(ee->prop.window, ee, ee->evas,
                               (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process,
                               (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process,
                               (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process,
                               (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process);
   evas_focus_in(ee->evas);

   return ee;
}

#else

static Ecore_Evas *
ecore_evas_software_wince_new_internal(int                 backend __UNUSED__,
                                       Ecore_WinCE_Window *parent __UNUSED__,
                                       int                 x __UNUSED__,
                                       int                 y __UNUSED__,
                                       int                 width __UNUSED__,
                                       int                 height __UNUSED__,
                                       int                 fullscreen __UNUSED__)
{
   return NULL;
}

#endif /* BUILD_ECORE_EVAS_SOFTWARE_16_WINCE */


EAPI Ecore_Evas *
ecore_evas_software_wince_new(Ecore_WinCE_Window *parent,
                              int                 x,
                              int                 y,
                              int                 width,
                              int                 height)
{
   return ecore_evas_software_wince_new_internal(0, parent, x, y, width, height, 1);
}

EAPI Ecore_Evas *
ecore_evas_software_wince_fb_new(Ecore_WinCE_Window *parent,
                                 int                 x,
                                 int                 y,
                                 int                 width,
                                 int                 height)
{
   return ecore_evas_software_wince_new_internal(1, parent, x, y, width, height, 1);
}

EAPI Ecore_Evas *
ecore_evas_software_wince_gapi_new(Ecore_WinCE_Window *parent,
                                   int                 x,
                                   int                 y,
                                   int                 width,
                                   int                 height)
{
   return ecore_evas_software_wince_new_internal(2, parent, x, y, width, height, 1);
}

EAPI Ecore_Evas *
ecore_evas_software_wince_ddraw_new(Ecore_WinCE_Window *parent,
                                    int                 x,
                                    int                 y,
                                    int                 width,
                                    int                 height)
{
   return ecore_evas_software_wince_new_internal(3, parent, x, y, width, height, 1);
}

EAPI Ecore_Evas *
ecore_evas_software_wince_gdi_new(Ecore_WinCE_Window *parent,
                                  int                 x,
                                  int                 y,
                                  int                 width,
                                  int                 height)
{
   return ecore_evas_software_wince_new_internal(4, parent, x, y, width, height, 0);
}

#ifdef BUILD_ECORE_EVAS_SOFTWARE_16_WINCE

EAPI Ecore_WinCE_Window *
ecore_evas_software_wince_window_get(const Ecore_Evas *ee)
{
   return (Ecore_WinCE_Window *) ecore_evas_window_get(ee);
}

#else

EAPI Ecore_WinCE_Window *
ecore_evas_software_wince_window_get(const Ecore_Evas *ee __UNUSED__)
{
   return NULL;
}

#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_16_WINCE */
