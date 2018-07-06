#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <sys/types.h>

#include <Eina.h>
#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>
#include <Ecore_Input_Evas.h>
#include <Ecore_Fb.h>
#include "ecore_fb_private.h"

#include <Ecore_Evas.h>
#include "ecore_evas_private.h"
#include <Evas_Engine_FB.h>

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

static int _ecore_evas_init_count = 0;

static char *ecore_evas_default_display = "0";
static Eina_List *ecore_evas_input_devices = NULL;
static Ecore_Event_Handler *ecore_evas_event_handlers[4] = {NULL, NULL, NULL, NULL};

typedef struct _Ecore_Evas_Engine_FB_Data Ecore_Evas_Engine_FB_Data;

struct _Ecore_Evas_Engine_FB_Data {
   int real_w;
   int real_h;
};

static void
_ecore_evas_mouse_move_process_fb(Ecore_Evas *ee, int x, int y)
{
   const Efl_Input_Device *pointer;
   Ecore_Evas_Cursor *cursor;
   int fbw, fbh;

   ecore_fb_size_get(&fbw, &fbh);

   pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_MOUSE);
   pointer = evas_device_parent_get(pointer);
   cursor = eina_hash_find(ee->prop.cursors, &pointer);
   EINA_SAFETY_ON_NULL_RETURN(cursor);
   cursor->pos_x = x;
   cursor->pos_y = y;
   if (cursor->object)
     {
        evas_object_show(cursor->object);
        if (ee->rotation == 0)
          evas_object_move(cursor->object,
                           x - cursor->hot.x,
                           y - cursor->hot.y);
        else if (ee->rotation == 90)
          evas_object_move(cursor->object,
                           (fbh - ee->h) + ee->h - y - 1 - cursor->hot.x,
                           x - cursor->hot.y);
        else if (ee->rotation == 180)
          evas_object_move(cursor->object,
                           (fbw - ee->w) + ee->w - x - 1 - cursor->hot.x,
                           (fbh - ee->h) + ee->h - y - 1 - cursor->hot.y);
        else if (ee->rotation == 270)
          evas_object_move(cursor->object,
                           y - cursor->hot.x,
                           (fbw - ee->w) + ee->w - x - 1 - cursor->hot.y);
     }
}

static void
_ecore_evas_fb_lose(void *data)
{
   Ecore_Evas *ee = data;
   Eina_List *ll;
   Ecore_Fb_Input_Device *dev;

   if (ee) ee->visible = 0;

   EINA_LIST_FOREACH(ecore_evas_input_devices, ll, dev)
     ecore_fb_input_device_listen(dev, 0);
}

static void
_ecore_evas_fb_gain(void *data)
{
   Ecore_Evas *ee = data;
   Eina_List *ll;
   Ecore_Fb_Input_Device *dev;

   if (ee)
     {
        ee->visible = 1;
        if (ECORE_EVAS_PORTRAIT(ee))
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
        else
          evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
     }

   EINA_LIST_FOREACH(ecore_evas_input_devices, ll, dev)
     ecore_fb_input_device_listen(dev, 1);
}

static Eina_Bool
_ecore_evas_event_mouse_button_down(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee = data;
   Ecore_Event_Mouse_Button *e;

   e = event;
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   _ecore_evas_mouse_move_process_fb(ee, e->x, e->y);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_event_mouse_button_up(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee = data;
   Ecore_Event_Mouse_Button *e;

   e = event;
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   _ecore_evas_mouse_move_process_fb(ee, e->x, e->y);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_event_mouse_move(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee = data;
   Ecore_Event_Mouse_Move *e;

   e = event;
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   _ecore_evas_mouse_move_process_fb(ee, e->x, e->y);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_ecore_evas_event_mouse_wheel(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Evas *ee = data;
   Ecore_Event_Mouse_Wheel *e;

   e = event;
   if (!ee) return ECORE_CALLBACK_PASS_ON;
   _ecore_evas_mouse_move_process_fb(ee, e->x, e->y);
   return ECORE_CALLBACK_PASS_ON;
}

static int
_ecore_evas_fb_init(Ecore_Evas *ee, int w, int h)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *ls;
   Ecore_Fb_Input_Device *device;
   Ecore_Fb_Input_Device_Cap caps;
   int mouse_handled = 0, always_ts;
   const char *s;

   _ecore_evas_init_count++;
   if (_ecore_evas_init_count > 1) return _ecore_evas_init_count;

   ecore_event_evas_init();

   /* register all input devices */
   ls = eina_file_direct_ls("/dev/input/");

   EINA_ITERATOR_FOREACH(ls, info)
     {
        if (strncmp(info->path + info->name_start, "event", 5) != 0)
          continue;

        if (!(device = ecore_fb_input_device_open(info->path)))
          continue;
        ecore_fb_input_device_window_set(device, ee);

        caps = ecore_fb_input_device_cap_get(device);

        /* Mouse */
#ifdef HAVE_TSLIB
        if (caps & ECORE_FB_INPUT_DEVICE_CAP_RELATIVE)
#else
        if ((caps & ECORE_FB_INPUT_DEVICE_CAP_RELATIVE) || (caps & ECORE_FB_INPUT_DEVICE_CAP_ABSOLUTE))
#endif
          {
             ecore_fb_input_device_axis_size_set(device, w, h);
             ecore_fb_input_device_listen(device,1);
             ecore_evas_input_devices = eina_list_append(ecore_evas_input_devices, device);
             if (!mouse_handled)
               {
                  ecore_evas_event_handlers[0]  = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, _ecore_evas_event_mouse_button_down, NULL);
                  ecore_evas_event_handlers[1]  = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, _ecore_evas_event_mouse_button_up, NULL);
                  ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _ecore_evas_event_mouse_move, NULL);
                  ecore_evas_event_handlers[3]  = ecore_event_handler_add(ECORE_EVENT_MOUSE_WHEEL, _ecore_evas_event_mouse_wheel, NULL);
                  mouse_handled = 1;
               }
          }
        /* Keyboard */
        else if ((caps & ECORE_FB_INPUT_DEVICE_CAP_KEYS_OR_BUTTONS) && !(caps & ECORE_FB_INPUT_DEVICE_CAP_ABSOLUTE))
          {
             ecore_fb_input_device_listen(device,1);
             ecore_evas_input_devices = eina_list_append(ecore_evas_input_devices, device);
          }
     }
   eina_iterator_free(ls);

   /* force to check for touchscreen via tslib even if mouse was detected */
   s = getenv("ECORE_EVAS_FB_TS_ALWAYS");
   always_ts = s ? atoi(s) : 0;

   if ((!mouse_handled) || (always_ts))
     {
        if (ecore_fb_ts_init())
          {
             ecore_fb_ts_event_window_set(ee);
             ecore_evas_event_handlers[0]  = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, _ecore_evas_event_mouse_button_down, NULL);
             ecore_evas_event_handlers[1]  = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, _ecore_evas_event_mouse_button_up, NULL);
             ecore_evas_event_handlers[2]  = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _ecore_evas_event_mouse_move, NULL);
             mouse_handled = 1;
          }
     }
   return _ecore_evas_init_count;
}

int
_ecore_evas_fb_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count == 0)
     {
        int i;

        for (i = 0; i < 4; i++)
          {
             if (ecore_evas_event_handlers[i])
               ecore_event_handler_del(ecore_evas_event_handlers[i]);
          }
        ecore_fb_ts_shutdown();
        ecore_event_evas_shutdown();
     }
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static void
_ecore_evas_fb_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_FB_Data *idata = ee->engine.data;

   ecore_evas_input_event_unregister(ee);
   free(idata);
   _ecore_evas_fb_shutdown();
   ecore_fb_shutdown();
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   ee->req.w = w;
   ee->req.h = h;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   if (ECORE_EVAS_PORTRAIT(ee))
     {
       evas_output_size_set(ee->evas, ee->w, ee->h);
       evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
       evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
   else
     {
       evas_output_size_set(ee->evas, ee->h, ee->w);
       evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
       evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
     }
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x EINA_UNUSED, int y EINA_UNUSED, int w, int h)
{
   ee->req.w = w;
   ee->req.h = h;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   if (ECORE_EVAS_PORTRAIT(ee))
     {
       evas_output_size_set(ee->evas, ee->w, ee->h);
       evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
       evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
   else
     {
       evas_output_size_set(ee->evas, ee->h, ee->w);
       evas_output_viewport_set(ee->evas, 0, 0, ee->h, ee->w);
       evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);
     }
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_rotation_set(Ecore_Evas *ee, int rotation, int resize EINA_UNUSED)
{
   const Evas_Device *pointer;
   Ecore_Evas_Cursor *cursor;
   Evas_Engine_Info_FB *einfo;
   int rot_dif;

   pointer = evas_default_device_get(ee->evas, EFL_INPUT_DEVICE_TYPE_MOUSE);
   pointer = evas_device_parent_get(pointer);
   cursor = eina_hash_find(ee->prop.cursors, &pointer);
   EINA_SAFETY_ON_NULL_RETURN(cursor);

   if (ee->rotation == rotation) return;
   einfo = (Evas_Engine_Info_FB *)evas_engine_info_get(ee->evas);
   if (!einfo) return;
   rot_dif = ee->rotation - rotation;
   if (rot_dif < 0) rot_dif = -rot_dif;
   if (rot_dif != 180)
     {

        einfo->info.rotation = rotation;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        if (!ee->prop.fullscreen)
          {
             int tmp;

             tmp = ee->w;
             ee->w = ee->h;
             ee->h = tmp;
             ee->req.w = ee->w;
             ee->req.h = ee->h;
          }
        else
          {
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
          }
        ee->rotation = rotation;
     }
   else
     {
        einfo->info.rotation = rotation;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
        ee->rotation = rotation;
     }
   if (ECORE_EVAS_PORTRAIT(ee))
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
   else
     evas_damage_rectangle_add(ee->evas, 0, 0, ee->h, ee->w);

   _ecore_evas_mouse_move_process_fb(ee, cursor->pos_x, cursor->pos_y);
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_show(Ecore_Evas *ee)
{
   if (ecore_evas_focus_device_get(ee, NULL)) return;
   ee->prop.withdrawn = EINA_FALSE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
   _ecore_evas_focus_device_set(ee, NULL, EINA_TRUE);
}

static void
_ecore_evas_hide(Ecore_Evas *ee)
{
   ee->prop.withdrawn = EINA_TRUE;
   if (ee->func.fn_state_change) ee->func.fn_state_change(ee);
   _ecore_evas_focus_device_set(ee, NULL, EINA_FALSE);
}

static void
_ecore_evas_fullscreen_set(Ecore_Evas *ee, Eina_Bool on)
{
   Eina_List *l;
   Ecore_Fb_Input_Device *dev;
   Ecore_Evas_Engine_FB_Data *idata = ee->engine.data;
   int resized = 0;

   if (((ee->prop.fullscreen) && (on)) ||
       ((!ee->prop.fullscreen) && (!on))) return;
   if (on)
     {
        int w, h;

        idata->real_w = ee->w;
        idata->real_h = ee->h;
        w = ee->w;
        h = ee->h;
        ecore_fb_size_get(&w, &h);
        if ((w == 0) && (h == 0))
          {
             w = ee->w;
             h = ee->h;
          }
        if ((w != ee->w) || (h != ee->h)) resized = 1;
        ee->w = w;
        ee->h = h;
        ee->req.w = ee->w;
        ee->req.h = ee->h;
        evas_output_size_set(ee->evas, ee->w, ee->h);
        evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
   else
     {
        if ((idata->real_w != ee->w) || (idata->real_h != ee->h)) resized = 1;
        ee->w = idata->real_w;
        ee->h = idata->real_h;
        ee->req.w = ee->w;
        ee->req.h = ee->h;
        evas_output_size_set(ee->evas, ee->w, ee->h);
        evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
        evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);
     }
   ee->prop.fullscreen = on;
   EINA_LIST_FOREACH(ecore_evas_input_devices, l, dev)
     ecore_fb_input_device_axis_size_set(dev, ee->w, ee->h);
   /* rescale the input device area */
   if (resized)
     {
        if (ee->func.fn_resize) ee->func.fn_resize(ee);
     }
}

static void
_ecore_evas_screen_geometry_get(const Ecore_Evas *ee EINA_UNUSED, int *x, int *y, int *w, int *h)
{
    int fbw, fbh;

    ecore_fb_size_get(&fbw, &fbh);

    if (x) *x = 0;
    if (y) *y = 0;
    if (w) *w = fbw;
    if (h) *h = fbh;
}

static Ecore_Evas_Engine_Func _ecore_fb_engine_func =
{
   _ecore_evas_fb_free,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     _ecore_evas_resize,
     _ecore_evas_move_resize,
     _ecore_evas_rotation_set,
     NULL,
     _ecore_evas_show,
     _ecore_evas_hide,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     _ecore_evas_fullscreen_set,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
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
     _ecore_evas_screen_geometry_get,
     NULL, // screen_dpi_get
     NULL,
     NULL, // msg_send

     NULL, // pointer_xy_get
     NULL, // pointer_warp

     NULL, // wm_rot_preferred_rotation_set
     NULL, // wm_rot_available_rotations_set
     NULL, // wm_rot_manual_rotation_done_set
     NULL, // wm_rot_manual_rotation_done

     NULL,  // aux_hints_set

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

EAPI Ecore_Evas *
ecore_evas_fb_new_internal(const char *disp_name, int rotation, int w, int h)
{
   Evas_Engine_Info_FB *einfo;
   Ecore_Evas_Engine_FB_Data *idata;
   Ecore_Evas *ee;

   int rmethod;

   if (!disp_name)
   disp_name = ecore_evas_default_display;

   rmethod = evas_render_method_lookup("fb");
   if (!rmethod) return NULL;

   if (!ecore_fb_init(disp_name)) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   idata = calloc(1, sizeof(Ecore_Evas_Engine_FB_Data));

   ee->engine.data = idata;

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_fb_init(ee, w, h);

   ecore_fb_callback_gain_set(_ecore_evas_fb_gain, ee);
   ecore_fb_callback_lose_set(_ecore_evas_fb_lose, ee);

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_fb_engine_func;

   ee->driver = "fb";
   if (disp_name) ee->name = strdup(disp_name);

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->rotation = rotation;
   ee->visible = 1;
   ee->w = w;
   ee->h = h;
   ee->req.w = ee->w;
   ee->req.h = ee->h;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.borderless = EINA_TRUE;
   ee->prop.override = EINA_TRUE;
   ee->prop.maximized = EINA_TRUE;
   ee->prop.fullscreen = EINA_FALSE;
   ee->prop.withdrawn = EINA_TRUE;
   ee->prop.sticky = EINA_FALSE;
   ee->prop.window = 1;

   /* init evas here */
   if (!ecore_evas_evas_new(ee, w, h))
     {
        ERR("Could not create the canvas.");
        ecore_evas_free(ee);
        return NULL;
     }
   evas_output_method_set(ee->evas, rmethod);

   einfo = (Evas_Engine_Info_FB *)evas_engine_info_get(ee->evas);
   if (einfo && disp_name)
     {
        einfo->info.virtual_terminal = 0;
        einfo->info.device_number = strtol(disp_name, NULL, 10);
        einfo->info.refresh = 0;
        einfo->info.rotation = ee->rotation;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }

   ecore_evas_done(ee, EINA_TRUE);

   return ee;
}
