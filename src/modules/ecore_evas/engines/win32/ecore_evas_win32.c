
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h> /* for NULL */
#include <inttypes.h> /* for UINT_MAX */

#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>
#include <Ecore_Win32.h>
#include "ecore_win32_private.h"

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"
#include "ecore_evas_win32.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
# include <Evas_Engine_Software_Gdi.h>
#endif
#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
# include <Evas_Engine_Software_DDraw.h>
#endif

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif /* ! DLL_EXPORT */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef BUILD_ECORE_EVAS_WIN32

#define ECORE_EVAS_EVENT_COUNT 11

static int _ecore_evas_init_count = 0;

static Ecore_Event_Handler *ecore_evas_event_handlers[ECORE_EVAS_EVENT_COUNT];
static const char *interface_win32_name = "win32";
static const int   interface_win32_version = 1;

typedef struct _Ecore_Evas_Engine_Data_Win32 Ecore_Evas_Engine_Data_Win32;

struct _Ecore_Evas_Engine_Data_Win32 {
   Ecore_Win32_Window *parent;
   struct {
     unsigned char region     : 1;
     unsigned char fullscreen : 1;
   } state;
};

static Ecore_Evas_Interface_Win32 *_ecore_evas_win32_interface_new(void);

static Eina_Bool _ecore_evas_win32_event_mouse_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_mouse_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_window_focus_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_window_focus_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_window_damage(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_window_destroy(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_window_show(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_window_hide(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_window_configure(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_window_delete_request(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

static Eina_Bool _ecore_evas_win32_event_window_property_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event);

/* Private functions */

static int
_ecore_evas_win32_init(void)
{
   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1)
     return _ecore_evas_init_count;

   ecore_evas_event_handlers[0]  = ecore_event_handler_add(ECORE_WIN32_EVENT_MOUSE_IN, _ecore_evas_win32_event_mouse_in, NULL);
   ecore_evas_event_handlers[1]  = ecore_event_handler_add(ECORE_WIN32_EVENT_MOUSE_OUT, _ecore_evas_win32_event_mouse_out, NULL);
   ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_FOCUS_IN, _ecore_evas_win32_event_window_focus_in, NULL);
   ecore_evas_event_handlers[3]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT, _ecore_evas_win32_event_window_focus_out, NULL);
   ecore_evas_event_handlers[4]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_DAMAGE, _ecore_evas_win32_event_window_damage, NULL);
   ecore_evas_event_handlers[5]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_DESTROY, _ecore_evas_win32_event_window_destroy, NULL);
   ecore_evas_event_handlers[6]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_SHOW, _ecore_evas_win32_event_window_show, NULL);
   ecore_evas_event_handlers[7]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_HIDE, _ecore_evas_win32_event_window_hide, NULL);
   ecore_evas_event_handlers[8]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_CONFIGURE, _ecore_evas_win32_event_window_configure, NULL);
   ecore_evas_event_handlers[9]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST, _ecore_evas_win32_event_window_delete_request, NULL);
   ecore_evas_event_handlers[10]  = ecore_event_handler_add(ECORE_WIN32_EVENT_WINDOW_PROPERTY, _ecore_evas_win32_event_window_property_change, NULL);

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
_ecore_evas_win32_event_mouse_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Win32_Event_Mouse_In *e;

   INF("mouse in");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   _ecore_evas_mouse_inout_set(ee, NULL, EINA_TRUE, EINA_FALSE);
   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   evas_event_feed_mouse_in(ee->evas, e->timestamp, NULL);
   evas_focus_in(ee->evas);
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->timestamp);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_mouse_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Win32_Event_Mouse_Out *e;

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   /* FIXME to do */
/*    _ecore_evas_x_modifier_locks_update(ee, e->modifiers); */
   _ecore_evas_mouse_move_process(ee, e->x, e->y, e->timestamp);

   if (_ecore_evas_mouse_in_check(ee, NULL))
     {
        if (evas_event_down_count_get(ee->evas) > 0) return ECORE_CALLBACK_PASS_ON;
        evas_event_feed_mouse_out(ee->evas, e->timestamp, NULL);
        _ecore_evas_mouse_inout_set(ee, NULL, EINA_FALSE, EINA_FALSE);
        _ecore_evas_default_cursor_hide(ee);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_window_focus_in(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Win32_Event_Window_Focus_In *e;

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   _ecore_evas_focus_device_set(ee, NULL, EINA_TRUE);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_window_focus_out(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Win32_Event_Window_Focus_Out *e;

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if ((!ee) || (ee->ignore_events)) return ECORE_CALLBACK_PASS_ON;
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   _ecore_evas_focus_device_set(ee, NULL, EINA_FALSE);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_window_damage(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Win32_Event_Window_Damage *e;

   INF("window damage");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   if (ee->prop.avoid_damage)
     {
#warning [ECORE] [WIN32] No Region code
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

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_window_destroy(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Win32_Event_Window_Destroy *e;

   INF("window destroy");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->func.fn_destroy) ee->func.fn_destroy(ee);
   ecore_evas_free(ee);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_window_show(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Win32_Event_Window_Show *e;

   INF("window show");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   ee->prop.withdrawn = EINA_FALSE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
   if (ee->visible) return ECORE_CALLBACK_PASS_ON;
   /* if (ee->visible) return ECORE_CALLBACK_PASS_DONE; /\* dont pass it on *\/ */
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_window_hide(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Win32_Event_Window_Hide *e;

   INF("window hide");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   ee->prop.withdrawn = EINA_TRUE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
   if (ee->visible) return ECORE_CALLBACK_PASS_ON;
   /* if (ee->visible) return ECORE_CALLBACK_PASS_DONE; /\* dont pass it on *\/ */
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_window_configure(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   const Evas_Device *pointer;
   Ecore_Evas_Cursor *cursor;
   Ecore_Evas *ee;
   Ecore_Win32_Event_Window_Configure *e;


   INF("window configure");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;

   ee->draw_block = EINA_FALSE;
   pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_MOUSE);
   pointer = evas_device_parent_get(pointer);
   cursor = eina_hash_find(ee->prop.cursors, &pointer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cursor, 1);

   if (ee->prop.override)
     {
        if ((ee->x != e->x) || (ee->y != e->y))
          {
             ee->x = e->x;
             ee->y = e->y;
             ee->req.x = ee->x;
             ee->req.y = ee->y;

             if (ee->func.fn_move) ee->func.fn_move(ee);
          }
     }

   if ((ee->w != e->width) || (ee->h != e->height))
     {
        ee->w = e->width;
        ee->h = e->height;
        ee->req.w = ee->w;
        ee->req.h = ee->h;

        if (ECORE_EVAS_PORTRAIT(ee))
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
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
               _ecore_evas_mouse_move_process(ee, cursor->pos_x, cursor->pos_y,
                                              ecore_win32_current_time_get());
             ee->expecting_resize.w = 0;
             ee->expecting_resize.h = 0;
          }
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_window_delete_request(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee;
   Ecore_Win32_Event_Window_Delete_Request *e;

   INF("window delete request");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   if (ee->func.fn_delete_request) ee->func.fn_delete_request(ee);

   INF(" * ee event delete\n");
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_win32_event_window_property_change(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   struct {
      struct {
         unsigned char fullscreen : 1;
      } win32;
      struct {
         Eina_Bool fullscreen : 1;
      } prop;
   } prev;
   Ecore_Evas *ee;
   Ecore_Win32_Event_Window_Property *e;
   Ecore_Evas_Engine_Data_Win32 *wdata;
   Ecore_Win32_Window_State *state;
   unsigned int num;
   unsigned int i;

   INF("window property");

   e = event;
   ee = ecore_event_window_match((Ecore_Window)e->window);
   if (!ee) return ECORE_CALLBACK_PASS_ON; /* pass on event */
   if ((Ecore_Window)e->window != ee->prop.window) return ECORE_CALLBACK_PASS_ON;
   wdata = ee->engine.data;

   prev.win32.fullscreen = wdata->state.fullscreen;

   prev.prop.fullscreen = ee->prop.fullscreen;

   wdata->state.fullscreen = 0;

   ee->prop.fullscreen = EINA_FALSE;

   /* we get the states status */
   ecore_win32_window_state_get(e->window, &state, &num);
   if (state)
     {
        for (i = 0; i < num; i++)
          {
             switch (state[i])
               {
                case ECORE_WIN32_WINDOW_STATE_FULLSCREEN:
                   ee->prop.fullscreen = 1;
                   wdata->state.fullscreen = 1;
                   break;
                default:
                   break;
               }
          }
        free(state);
     }

   if ((prev.win32.fullscreen != wdata->state.fullscreen) ||
       (prev.prop.fullscreen != ee->prop.fullscreen))
     {
        if (ee->func.fn_state_change)
          ee->func.fn_state_change(ee);
     }

   return ECORE_CALLBACK_PASS_ON;
}

/* FIXME, should be in idler */
static void
_ecore_evas_win32_state_update(Ecore_Evas *ee)
{
   Ecore_Win32_Window_State state[10];
   Ecore_Evas_Engine_Data_Win32 *edata = ee->engine.data;
   int num = 0;

   if (ee->prop.modal)
     state[num++] = ECORE_WIN32_WINDOW_STATE_MODAL;
   if (ee->prop.sticky)
     state[num++] = ECORE_WIN32_WINDOW_STATE_STICKY;
   if (ee->prop.maximized)
     state[num++] = ECORE_WIN32_WINDOW_STATE_MAXIMIZED_VERT;
   if (ee->prop.maximized)
     state[num++] = ECORE_WIN32_WINDOW_STATE_MAXIMIZED_HORZ;
//   if (bd->client.netwm.state.shaded)
//     state[num++] = ECORE_WIN32_WINDOW_STATE_SHADED;
   /* if (ee->prop.focus_skip) */
   /*   state[num++] = ECORE_WIN32_WINDOW_STATE_SKIP_TASKBAR; */
   /* if (ee->prop.focus_skip) */
   /*   state[num++] = ECORE_WIN32_WINDOW_STATE_SKIP_PAGER; */
//   if (bd->client.netwm.state.hidden)
//     state[num++] = ECORE_WIN32_WINDOW_STATE_HIDDEN;
   if (edata->state.fullscreen)
     state[num++] = ECORE_WIN32_WINDOW_STATE_FULLSCREEN;
   /* if (edata->state.above) */
   /*   state[num++] = ECORE_WIN32_WINDOW_STATE_ABOVE; */
   /* if (edata->state.below) */
   /*   state[num++] = ECORE_WIN32_WINDOW_STATE_BELOW; */
   /* if (ee->prop.demand_attention) */
   /*   state[num++] = ECORE_WIN32_WINDOW_STATE_DEMANDS_ATTENTION; */

   ecore_win32_window_state_set((Ecore_Win32_Window *)ee->prop.window, state, num);
}


/* Ecore_Evas interface */

static void
_ecore_evas_win32_free(Ecore_Evas *ee)
{
   INF("ecore evas free");

   ecore_win32_window_free((Ecore_Win32_Window *)ee->prop.window);
   ecore_event_window_unregister(ee->prop.window);
   free(ee->engine.data);
   _ecore_evas_win32_shutdown();
   ecore_win32_shutdown();
}

static void
_ecore_evas_win32_callback_delete_request_set(Ecore_Evas *ee,
                                              Ecore_Evas_Event_Cb func)
{
   ee->func.fn_delete_request = func;
}

static void
_ecore_evas_win32_move(Ecore_Evas *ee, int x, int y)
{
  INF("ecore evas move (%dx%d)", x, y);
   ee->req.x = x;
   ee->req.y = y;

   if ((x != ee->x) || (y != ee->y))
     {
        ee->x = x;
        ee->y = y;
        ecore_win32_window_move((Ecore_Win32_Window *)ee->prop.window,
                                x, y);
        if (ee->func.fn_move) ee->func.fn_move(ee);
     }
}

static void
_ecore_evas_win32_resize(Ecore_Evas *ee, int width, int height)
{
   INF("ecore evas resize (%dx%d)", width, height);

   if ((ee->req.w != width) || (ee->req.h != height))
     {
        ee->req.w = width;
        ee->req.h = height;
        ecore_win32_window_resize((Ecore_Win32_Window *)ee->prop.window,
                                  width, height);
     }
}

static void
_ecore_evas_win32_move_resize(Ecore_Evas *ee, int x, int y, int width, int height)
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
        ecore_win32_window_move_resize((Ecore_Win32_Window *)ee->prop.window,
                                       x, y, width, height);
        if (ECORE_EVAS_PORTRAIT(ee))
          {
             evas_output_size_set(ee->evas, ee->w, ee->h);
             evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
          }
        else
          {
             evas_output_size_set(ee->evas, ee->h, ee->w);
             evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
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
   const Evas_Device *pointer;
   Ecore_Evas_Cursor *cursor;
   int rot_dif;

   rot_dif = ee->rotation - rotation;
   if (rot_dif < 0) rot_dif = -rot_dif;

   pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_MOUSE);
   pointer = evas_device_parent_get(pointer);
   cursor = eina_hash_find(ee->prop.cursors, &pointer);
   EINA_SAFETY_ON_NULL_RETURN(cursor);

   if (rot_dif != 180)
     {
        int minw, minh, maxw, maxh, basew, baseh, stepw, steph;

        if (!ee->prop.fullscreen)
          {
             ecore_win32_window_resize((Ecore_Win32_Window *)ee->prop.window,
                                       ee->h, ee->w);
             ee->expecting_resize.w = ee->h;
             ee->expecting_resize.h = ee->w;
          }
        else
          {
             int w, h;

             ecore_win32_window_size_get((Ecore_Win32_Window *)ee->prop.window,
                                         &w, &h);
             ecore_win32_window_resize((Ecore_Win32_Window *)ee->prop.window,
                                       h, w);
             if (ECORE_EVAS_PORTRAIT(ee))
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
        _ecore_evas_mouse_move_process(ee, cursor->pos_x, cursor->pos_y,
                                       ecore_win32_current_time_get());
     }
   else
     {
        ee->rotation = rotation;
        _ecore_evas_mouse_move_process(ee, cursor->pos_x, cursor->pos_y,
                                       ecore_win32_current_time_get());
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }

   if (ECORE_EVAS_PORTRAIT(ee))
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   else
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
}

static void
_ecore_evas_win32_rotation_set(Ecore_Evas *ee, int rotation, int resize EINA_UNUSED)
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
   Ecore_Evas_Engine_Data_Win32 *wdata;
   if (((ee->shaped) && (shaped)) || ((!ee->shaped) && (!shaped)))
     return;

   wdata = ee->engine.data;
   if (!strcmp(ee->driver, "software_ddraw")) return;

#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
   if (!strcmp(ee->driver, "software_gdi"))
     {
        Evas_Engine_Info_Software_Gdi *einfo;

        einfo = (Evas_Engine_Info_Software_Gdi *)evas_engine_info_get(ee->evas);
        ee->shaped = shaped;
        if (einfo)
          {
             wdata->state.region = ee->shaped;
             einfo->info.region = wdata->state.region;
             if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
               {
                  ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
               }
             if (ee->shaped)
               evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
          }
     }
#endif /* BUILD_ECORE_EVAS_SOFTWARE_GDI */
}

static void
_ecore_evas_win32_show(Ecore_Evas *ee)
{
   INF("ecore evas show");

   ee->should_be_visible = 1;
   if (ee->prop.avoid_damage)
     {
        ecore_evas_render(ee);
        ecore_evas_render_wait(ee);
     }
   ecore_win32_window_show((Ecore_Win32_Window *)ee->prop.window);
/*    if (ee->prop.fullscreen) */
/*      ecore_win32_window_focus(ee->prop.window); */
}

static void
_ecore_evas_win32_hide(Ecore_Evas *ee)
{
   INF("ecore evas hide");

   ecore_win32_window_hide((Ecore_Win32_Window *)ee->prop.window);
   ee->should_be_visible = 0;
}

static void
_ecore_evas_win32_raise(Ecore_Evas *ee)
{
   INF("ecore evas raise");

   if (!ee->prop.fullscreen)
     ecore_win32_window_raise((Ecore_Win32_Window *)ee->prop.window);
   else
     ecore_win32_window_raise((Ecore_Win32_Window *)ee->prop.window);
}

static void
_ecore_evas_win32_lower(Ecore_Evas *ee)
{
   INF("ecore evas lower");

   if (!ee->prop.fullscreen)
     ecore_win32_window_lower((Ecore_Win32_Window *)ee->prop.window);
   else
     ecore_win32_window_lower((Ecore_Win32_Window *)ee->prop.window);
}

static void
_ecore_evas_win32_activate(Ecore_Evas *ee)
{
   INF("ecore evas activate");

   ecore_win32_window_focus((Ecore_Win32_Window *)ee->prop.window);
}

static void
_ecore_evas_win32_title_set(Ecore_Evas *ee, const char *title)
{
   INF("ecore evas title set");
   if (eina_streq(ee->prop.title, title)) return;
   if (ee->prop.title) free(ee->prop.title);
   ee->prop.title = NULL;
   if (title) ee->prop.title = strdup(title);
   ecore_win32_window_title_set((Ecore_Win32_Window *)ee->prop.window,
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
   ecore_win32_window_size_min_set((Ecore_Win32_Window *)ee->prop.window,
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
   ecore_win32_window_size_max_set((Ecore_Win32_Window *)ee->prop.window,
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
   ecore_win32_window_size_base_set((Ecore_Win32_Window *)ee->prop.window,
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
   ecore_win32_window_size_step_set((Ecore_Win32_Window *)ee->prop.window,
                                    width, height);
}

static void
_ecore_evas_win32_object_cursor_set(Ecore_Evas *ee, Evas_Object *obj,
                                    int layer EINA_UNUSED,
                                    int hot_x EINA_UNUSED,
                                    int hot_y EINA_UNUSED)
{
   if (obj != _ecore_evas_default_cursor_image_get(ee))
     ecore_win32_cursor_show(EINA_FALSE);
}

static void
_ecore_evas_win32_object_cursor_unset(Ecore_Evas *ee EINA_UNUSED)
{
   ecore_win32_cursor_show(EINA_TRUE);
}

static void
_ecore_evas_win32_focus_set(Ecore_Evas *ee, Eina_Bool on EINA_UNUSED)
{
   ecore_win32_window_focus((Ecore_Win32_Window *)ee->prop.window);
}

static void
_ecore_evas_win32_iconified_set(Ecore_Evas *ee, Eina_Bool on)
{
/*    if (((ee->prop.borderless) && (on)) || */
/*        ((!ee->prop.borderless) && (!on))) return; */
   ee->prop.iconified = on;
   ecore_win32_window_iconified_set((Ecore_Win32_Window *)ee->prop.window,
                                    ee->prop.iconified);
}

static void
_ecore_evas_win32_borderless_set(Ecore_Evas *ee, Eina_Bool on)
{
   if (((ee->prop.borderless) && (on)) ||
       ((!ee->prop.borderless) && (!on))) return;
   ee->prop.borderless = on;
   ecore_win32_window_borderless_set((Ecore_Win32_Window *)ee->prop.window,
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
_ecore_evas_win32_override_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Win32_Window *window;

   INF("ecore evas override set");

   window = (Ecore_Win32_Window *)ee->prop.window;

   if (ee->prop.override == on) return;
   if (ee->should_be_visible) ecore_win32_window_hide(window);
   /* FIXME: use borderless_set for now */
   ecore_win32_window_borderless_set(window, on);
   if (ee->should_be_visible) ecore_win32_window_show(window);
   if (ecore_evas_focus_device_get(ee, NULL)) ecore_win32_window_focus(window);
   ee->prop.override = on;
}

static void
_ecore_evas_win32_fullscreen_set(Ecore_Evas *ee, Eina_Bool on)
{
   Ecore_Evas_Engine_Data_Win32 *wdata = ee->engine.data;

   INF("ecore evas fullscreen set");

   if (ee->prop.fullscreen == !!on) return;

   wdata->state.fullscreen = !!on;
   if (ee->should_be_visible)
     {
        struct _Ecore_Win32_Window *window;

        window = (Ecore_Win32_Window *)ee->prop.window;
        ecore_win32_window_fullscreen_set(window, on);
     }
   else
     _ecore_evas_win32_state_update(ee);

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
}

static void
_ecore_evas_win32_alpha_set(Ecore_Evas *ee, int alpha)
{
#warning "We need to handle window with alpha channel."
   /* Ecore_Evas_Engine_Data_Win32 *wdata = ee->engine.data; */
   alpha = !!alpha;
   if (ee->alpha == alpha) return;

   if (!strcmp(ee->driver, "software_gdi"))
     {
#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
        Evas_Engine_Info_Software_Gdi *einfo;

        einfo = (Evas_Engine_Info_Software_Gdi *)evas_engine_info_get(ee->evas);
        if (!einfo) return;

        ee->shaped = 0;
        ee->alpha = alpha;
        /* ecore_win32_window_free(ee->prop.window); */
        /* ecore_event_window_unregister(ee->prop.window); */
        /* if (ee->alpha) */
        /*   { */
        /*      if (ee->prop.override) */
        /*        ee->prop.window = ecore_x_window_override_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h); */
        /*      else */
        /*        ee->prop.window = ecore_x_window_argb_new(ee->engine.x.win_root, ee->req.x, ee->req.y, ee->req.w, ee->req.h); */
        /*      if (!ee->engine.x.mask) */
        /*        ee->engine.x.mask = ecore_x_pixmap_new(ee->prop.window, ee->req.w, ee->req.h, 1); */
        /*   } */
        /* else */
        /*   { */
        /*      if (ee->prop.override) */
        /*        ee->prop.window = ecore_win32_window_override_new(wdata->win_root, */
        /*                                                          ee->req.x, */
        /*                                                          ee->req.y, */
        /*                                                          ee->req.w, */
        /*                                                          ee->req.h); */
        /*      else */
        /*        ee->prop.window = ecore_win32_window_new(wdata->win_root, */
        /*                                                 ee->req.x, */
        /*                                                 ee->req.y, */
        /*                                                 ee->req.w, */
        /*                                                 ee->req.h); */
        /*      if (wdata->mask) ecore_x_pixmap_free(ee->engine.x.mask); */
        /*      wdata->mask = 0; */
        /*      ecore_win32_window_shape_input_mask_set(ee->prop.window, 0); */
        /*   } */

        /* einfo->info.destination_alpha = alpha; */
        einfo->info.region = alpha;

//        if (ee->engine.x.mask) ecore_x_pixmap_free(ee->engine.x.mask);
//        ee->engine.x.mask = 0;
        /* einfo->info.mask = wdata->mask; */
        /* einfo->info.drawable = ee->prop.window; */
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->req.w, ee->req.h);
        /* ecore_win32_window_shape_mask_set(ee->prop.window, 0); */
        /* ecore_event_window_register(ee->prop.window, ee, ee->evas, */
        /*                             (Ecore_Event_Mouse_Move_Cb)_ecore_evas_mouse_move_process, */
        /*                             (Ecore_Event_Multi_Move_Cb)_ecore_evas_mouse_multi_move_process, */
        /*                             (Ecore_Event_Multi_Down_Cb)_ecore_evas_mouse_multi_down_process, */
        /*                             (Ecore_Event_Multi_Up_Cb)_ecore_evas_mouse_multi_up_process); */
        if (ee->prop.borderless)
          ecore_win32_window_borderless_set((Ecore_Win32_Window *)ee->prop.window, ee->prop.borderless);
        if (ee->visible) ecore_win32_window_show((Ecore_Win32_Window *)ee->prop.window);
        if (ecore_evas_focus_device_get(ee, NULL)) ecore_win32_window_focus((Ecore_Win32_Window *)ee->prop.window);
        if (ee->prop.title)
          {
             ecore_win32_window_title_set((Ecore_Win32_Window *)ee->prop.window, ee->prop.title);
             /* ecore_win32_name_set(ee->prop.window, ee->prop.title); */
          }
        ecore_win32_window_type_set((Ecore_Win32_Window *)ee->prop.window, ECORE_WIN32_WINDOW_TYPE_NORMAL);
#endif /* BUILD_ECORE_EVAS_SOFTWARE_GDI */
     }
}

static void
_ecore_evas_win32_screen_geometry_get(const Ecore_Evas *ee, int *x, int *y, int *w, int *h)
{
   Eina_Iterator *iter;
   Ecore_Win32_Monitor *ewm;
   Ecore_Win32_Monitor *m = NULL;
   unsigned int dist;
   int lx;
   int ly;
   int wx;
   int wy;
   int ww;
   int wh;

   ecore_win32_window_geometry_get((Ecore_Win32_Window *)ee->prop.window,
                                   &wx, &wy, &ww, &wh);
   iter = ecore_win32_monitors_get();
   dist = UINT32_MAX;

   EINA_ITERATOR_FOREACH(iter, ewm)
     {
        unsigned int d;

        lx = ewm->desktop.x - wx + (ewm->desktop.w - ww) / 2;
        ly = ewm->desktop.y - wy + (ewm->desktop.h - wh) / 2;
        d = lx * lx + ly * ly;
        if (d < dist)
          {
             dist = d;
             m = ewm;
          }
     }
   eina_iterator_free(iter);

   if (!m)
     {
        HDC dc;

        if (x) *x = 0;
        if (y) *y = 0;
        dc = GetDC(NULL);
        if (w) *w = GetDeviceCaps(dc, HORZRES);
        if (h) *h = GetDeviceCaps(dc, VERTRES);
        ReleaseDC(NULL, dc);

        return;
     }

   if (x)
     *x = m->desktop.x;
   if (y)
     *y = m->desktop.y;
   if (w)
     *w = m->desktop.w;
   if (h)
     *h = m->desktop.h;
}

static void
_ecore_evas_win32_screen_dpi_get(const Ecore_Evas *ee, int *xdpi, int *ydpi)
{
   Eina_Iterator *iter;
   Ecore_Win32_Monitor *ewm;
   unsigned int dist;
   int x_dpi = -1;
   int y_dpi;
   int lx;
   int ly;
   int x;
   int y;
   int w;
   int h;

   ecore_win32_window_geometry_get((Ecore_Win32_Window *)ee->prop.window,
                                   &x, &y, &w, &h);
   iter = ecore_win32_monitors_get();
   dist = UINT32_MAX;

   EINA_ITERATOR_FOREACH(iter, ewm)
     {
        unsigned int d;

        lx = ewm->desktop.x - x + (ewm->desktop.w - w) / 2;
        ly = ewm->desktop.y - y + (ewm->desktop.h - h) / 2;
        d = lx * lx + ly * ly;
        if (d < dist)
          {
             dist = d;
             x_dpi = ewm->dpi.x;
             y_dpi = ewm->dpi.y;
          }
     }
   eina_iterator_free(iter);

   if (x_dpi == -1)
     {
        HDC dc;

        dc = GetDC(NULL);
        x_dpi = GetDeviceCaps(dc, LOGPIXELSX);
        y_dpi = GetDeviceCaps(dc, LOGPIXELSY);
        ReleaseDC(NULL, dc);
     }

   if (xdpi)
     *xdpi = x_dpi;
   if (ydpi)
     *ydpi = y_dpi;
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
     _ecore_evas_win32_object_cursor_set,
     _ecore_evas_win32_object_cursor_unset,
     NULL, /* _ecore_evas_x_layer_set */
     _ecore_evas_win32_focus_set,
     _ecore_evas_win32_iconified_set,
     _ecore_evas_win32_borderless_set,
     _ecore_evas_win32_override_set,
     NULL,
     _ecore_evas_win32_fullscreen_set,
     NULL, /* _ecore_evas_x_avoid_damage_set */
     NULL, /* _ecore_evas_x_withdrawn_set */
     NULL, /* _ecore_evas_x_sticky_set */
     NULL, /* _ecore_evas_x_ignore_events_set */
     _ecore_evas_win32_alpha_set,
     NULL, //transparent
     NULL, // profiles_set
     NULL, // profile_set

     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,

     NULL, // render
     _ecore_evas_win32_screen_geometry_get,
     _ecore_evas_win32_screen_dpi_get,
     NULL,
     NULL,  // msg_send

     NULL, // pointer_xy_get
     NULL, // pointer_warp

     NULL, // wm_rot_preferred_rotation_set
     NULL, // wm_rot_available_rotations_set
     NULL, // wm_rot_manual_rotation_done_set
     NULL, // wm_rot_manual_rotation_done

     NULL, // aux_hints_set

     NULL, // fn_animator_register
     NULL, // fn_animator_unregister

     NULL, // fn_evas_changed
     NULL, //fn_focus_device_set
     NULL, //fn_callback_focus_device_in_set
     NULL, //fn_callback_focus_device_out_set
     NULL, //fn_callback_device_mouse_in_set
     NULL, //fn_callback_device_mouse_out_set
     NULL, //fn_pointer_device_xy_get
     NULL, //fn_prepare
     NULL, //fn_last_tick_get
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
        einfo->info.window = ((Ecore_Win32_Window *)ee->prop.window)->window;
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
        einfo->info.window = ((Ecore_Win32_Window *)ee->prop.window)->window;
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

static Ecore_Evas *
_ecore_evas_win32_new_internal(int (*_ecore_evas_engine_backend_init)(Ecore_Evas *ee),
                               Ecore_Win32_Window *parent,
                               int                 x,
                               int                 y,
                               int                 width,
                               int                 height)
{
   Ecore_Evas *ee;
   Ecore_Evas_Engine_Data_Win32 *wdata;
   Ecore_Evas_Interface_Win32 *iface;

   if (!ecore_win32_init())
     return NULL;

   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee)
     return NULL;
   wdata = calloc(1, sizeof(Ecore_Evas_Engine_Data_Win32));
   if (!wdata)
     {
	free(ee);
	return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_win32_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_win32_engine_func;
   ee->engine.data = wdata;

   iface = _ecore_evas_win32_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

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
   ee->can_async_render = EINA_FALSE;
   ee->draw_block = EINA_TRUE;

   ee->prop.max.w = 32767;
   ee->prop.max.h = 32767;
   ee->prop.layer = 4;
   ee->prop.request_pos = EINA_FALSE;
   ee->prop.sticky = EINA_FALSE;
   /* FIXME: sticky to add */
   ee->prop.window = 0;
   ee->prop.withdrawn = EINA_TRUE;

   /* init evas here */
   if (!ecore_evas_evas_new(ee, width, height))
     {
        ERR("Can not create Canvas.");
        free(ee);
        return NULL;
     }

   wdata->parent = parent;
   ee->prop.window = (Ecore_Window)ecore_win32_window_new(parent, x, y, width, height);
   if (!ee->prop.window)
     {
        _ecore_evas_win32_shutdown();
        free(ee);
        return NULL;
     }

   if (!_ecore_evas_engine_backend_init(ee))
     {
        _ecore_evas_win32_shutdown();
        free(ee);
        return NULL;
     }

   ecore_evas_done(ee, EINA_FALSE);

   return ee;
}

EAPI Ecore_Evas *
ecore_evas_software_gdi_new_internal(Ecore_Win32_Window *parent,
				     int                 x,
				     int                 y,
				     int                 width,
				     int                 height)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_GDI
   return _ecore_evas_win32_new_internal(_ecore_evas_engine_software_gdi_init,
                                         parent,
                                         x,
                                         y,
                                         width,
                                         height);
#else
   (void) parent;
   (void) x;
   (void) y;
   (void) width;
   (void) height;
   return NULL;
#endif
}

EAPI Ecore_Evas *
ecore_evas_software_ddraw_new_internal(Ecore_Win32_Window *parent,
				       int                 x,
				       int                 y,
				       int                 width,
				       int                 height)
{
#ifdef BUILD_ECORE_EVAS_SOFTWARE_DDRAW
   return _ecore_evas_win32_new_internal(_ecore_evas_engine_software_ddraw_init,
                                         parent,
                                         x,
                                         y,
                                         width,
                                         height);
#else
   (void) parent;
   (void) x;
   (void) y;
   (void) width;
   (void) height;
   return NULL;
#endif /* ! BUILD_ECORE_EVAS_SOFTWARE_DDRAW */
}

static Ecore_Win32_Window *
_ecore_evas_win32_window_get(const Ecore_Evas *ee)
{
   return (Ecore_Win32_Window *) ecore_evas_window_get(ee);
}

static Ecore_Evas_Interface_Win32 *
_ecore_evas_win32_interface_new(void)
{
   Ecore_Evas_Interface_Win32 *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Win32));
   if (!iface) return NULL;

   iface->base.name = interface_win32_name;
   iface->base.version = interface_win32_version;

   iface->window_get = _ecore_evas_win32_window_get;

   return iface;
}
