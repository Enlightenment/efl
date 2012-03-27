#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Input.h"
#include "ecore_wl_private.h"
#include "Ecore_Wayland.h"

/* FIXME: This gives BTN_LEFT/RIGHT/MIDDLE for linux systems ... 
 *        What about other OSs ?? */
#ifdef __linux__
# include <linux/input.h>
#else
# define BTN_LEFT 0x110
# define BTN_RIGHT 0x111
# define BTN_MIDDLE 0x112
# define BTN_SIDE 0x113
# define BTN_EXTRA 0x114
# define BTN_FORWARD 0x115
# define BTN_BACK 0x116
#endif

/* local function prototypes */
static void _ecore_wl_input_cb_motion(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, int sx, int sy);
static void _ecore_wl_input_cb_button(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, unsigned int button, unsigned int state);
static void _ecore_wl_input_cb_axis(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, unsigned int axis, int value);
static void _ecore_wl_input_cb_key(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp __UNUSED__, unsigned int key, unsigned int state);
static void _ecore_wl_input_cb_pointer_enter(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface, int sx, int sy);
static void _ecore_wl_input_cb_pointer_leave(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface __UNUSED__);
static void _ecore_wl_input_cb_keyboard_enter(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface, struct wl_array *keys);
static void _ecore_wl_input_cb_keyboard_leave(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface __UNUSED__);
static void _ecore_wl_input_cb_touch_down(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface __UNUSED__, int id __UNUSED__, int x, int y);
static void _ecore_wl_input_cb_touch_up(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, int id __UNUSED__);
static void _ecore_wl_input_cb_touch_motion(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, int id __UNUSED__, int x, int y);
static void _ecore_wl_input_cb_touch_frame(void *data __UNUSED__, struct wl_input_device *input_device __UNUSED__);
static void _ecore_wl_input_cb_touch_cancel(void *data __UNUSED__, struct wl_input_device *input_device __UNUSED__);
static void _ecore_wl_input_cb_data_offer(void *data, struct wl_data_device *data_device, unsigned int id);
static void _ecore_wl_input_cb_data_enter(void *data, struct wl_data_device *data_device, unsigned int timestamp, struct wl_surface *surface, int x, int y, struct wl_data_offer *offer);
static void _ecore_wl_input_cb_data_leave(void *data, struct wl_data_device *data_device);
static void _ecore_wl_input_cb_data_motion(void *data, struct wl_data_device *data_device, unsigned int timestamp, int x, int y);
static void _ecore_wl_input_cb_data_drop(void *data, struct wl_data_device *data_device);
static void _ecore_wl_input_cb_data_selection(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer);

static void _ecore_wl_input_keyboard_focus_remove(Ecore_Wl_Input *input, unsigned int timestamp);
static void _ecore_wl_input_pointer_focus_remove(Ecore_Wl_Input *input, unsigned int timestamp);
static void _ecore_wl_input_mouse_move_send(Ecore_Wl_Input *input, unsigned int timestamp);
static void _ecore_wl_input_mouse_in_send(Ecore_Wl_Input *input, unsigned int timestamp);
static void _ecore_wl_input_mouse_out_send(Ecore_Wl_Input *input, unsigned int timestamp);
static void _ecore_wl_input_focus_in_send(Ecore_Wl_Input *input, unsigned int timestamp);
static void _ecore_wl_input_focus_out_send(Ecore_Wl_Input *input, unsigned int timestamp);
static void _ecore_wl_input_mouse_down_send(Ecore_Wl_Input *input, unsigned int timestamp);
static void _ecore_wl_input_mouse_up_send(Ecore_Wl_Input *input, unsigned int timestamp);
static void _ecore_wl_input_mouse_wheel_send(Ecore_Wl_Input *input, unsigned int axis, int value, unsigned int timestamp);

/* wayland interfaces */
static const struct wl_input_device_listener _ecore_wl_input_listener = 
{
   _ecore_wl_input_cb_motion,
   _ecore_wl_input_cb_button,
   _ecore_wl_input_cb_axis,
   _ecore_wl_input_cb_key,
   _ecore_wl_input_cb_pointer_enter,
   _ecore_wl_input_cb_pointer_leave,
   _ecore_wl_input_cb_keyboard_enter,
   _ecore_wl_input_cb_keyboard_leave,
   _ecore_wl_input_cb_touch_down,
   _ecore_wl_input_cb_touch_up,
   _ecore_wl_input_cb_touch_motion,
   _ecore_wl_input_cb_touch_frame,
   _ecore_wl_input_cb_touch_cancel
};

static const struct wl_data_device_listener _ecore_wl_data_listener = 
{
   _ecore_wl_input_cb_data_offer,
   _ecore_wl_input_cb_data_enter,
   _ecore_wl_input_cb_data_leave,
   _ecore_wl_input_cb_data_motion,
   _ecore_wl_input_cb_data_drop,
   _ecore_wl_input_cb_data_selection
};

/* local variables */
static int _pointer_x, _pointer_y;

EAPI void 
ecore_wl_input_grab(Ecore_Wl_Input *input, Ecore_Wl_Window *win, unsigned int button)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   input->grab = win;
   input->grab_button = button;
}

EAPI void 
ecore_wl_input_ungrab(Ecore_Wl_Input *input, unsigned int timestamp)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   input->grab = NULL;
   if (input->pointer_focus)
     {
        printf("Ungrab: %d\n", timestamp);
     }
}

void 
_ecore_wl_input_add(Ecore_Wl_Display *ewd, unsigned int id)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = malloc(sizeof(Ecore_Wl_Input)))) return;

   memset(input, 0, sizeof(Ecore_Wl_Input));

   input->display = ewd;
   input->pointer_focus = NULL;
   input->keyboard_focus = NULL;

   input->input_device = 
     wl_display_bind(ewd->wl.display, id, &wl_input_device_interface);
   wl_list_insert(ewd->inputs.prev, &input->link);
   wl_input_device_add_listener(input->input_device, 
                                &_ecore_wl_input_listener, input);
   wl_input_device_set_user_data(input->input_device, input);

   input->data_device = 
     wl_data_device_manager_get_data_device(ewd->wl.data_device_manager, 
                                            input->input_device);
   wl_data_device_add_listener(input->data_device, 
                               &_ecore_wl_data_listener, input);
   ewd->input = input;
}

void 
_ecore_wl_input_del(Ecore_Wl_Input *input)
{
   if (!input) return;

   _ecore_wl_input_keyboard_focus_remove(input, 0);
   _ecore_wl_input_pointer_focus_remove(input, 0);

   if (input->drag_source) _ecore_wl_dnd_del(input->drag_source);
   input->drag_source = NULL;

   if (input->selection_source) _ecore_wl_dnd_del(input->selection_source);
   input->selection_source = NULL;

   if (input->data_device) wl_data_device_destroy(input->data_device);
   if (input->input_device) wl_input_device_destroy(input->input_device);
   wl_list_remove(&input->link);
   free(input);
}

void 
_ecore_wl_input_pointer_xy_get(int *x, int *y)
{
   if (x) *x = _pointer_x;
   if (y) *y = _pointer_y;
}

/* local functions */
static void 
_ecore_wl_input_cb_motion(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, int sx, int sy)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   _pointer_x = sx;
   _pointer_y = sy;

   input->sx = sx;
   input->sy = sy;

   /* TODO: FIXME: NB: Weston window code has set pointer image here also */
   _ecore_wl_input_mouse_move_send(input, timestamp);
}

static void 
_ecore_wl_input_cb_button(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, unsigned int button, unsigned int state)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   input->timestamp = timestamp;

   if ((input->pointer_focus) && (!input->grab) && (state))
     ecore_wl_input_grab(input, input->pointer_focus, button);

//   _ecore_wl_input_mouse_move_send(input, timestamp);

   if ((button >= BTN_SIDE) && (button <= BTN_BACK))
     {
        /* TODO: raise mouse wheel */
        printf("Raise Mouse Wheel Event\n");
     }
   else
     {
        if (state)
          {
             input->button = button;
             _ecore_wl_input_mouse_down_send(input, timestamp);
          }
        else
          {
             _ecore_wl_input_mouse_up_send(input, timestamp);
             input->button = 0;

             if ((input->grab) && (input->grab_button == button))
               ecore_wl_input_ungrab(input, timestamp);
          }
     }
}

static void 
_ecore_wl_input_cb_axis(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, unsigned int axis, int value)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   _ecore_wl_input_mouse_wheel_send(input, axis, value, timestamp);
}

static void 
_ecore_wl_input_cb_key(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp __UNUSED__, unsigned int key, unsigned int state)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win;
   unsigned int keycode = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   win = input->keyboard_focus;
   if ((!win) || (win->keyboard_device != input)) return;

   /* FIXME: NB: I believe this should be min_key_code rather than 8, 
    * but weston code has it like this */
   keycode = key + 8;

   /* if ((input->modifiers & XKB_COMMON_SHIFT_MASK) &&  */
   /*     (XkbKeyGroupWidth(_ecore_wl_disp->xkb, keycode, 0) > 1))  */
   /*   level = 1; */
   /* keysym = XkbKeySymEntry(_ecore_wl_disp->xkb, keycode, level, 0); */

   if (state)
     input->modifiers |= _ecore_wl_disp->xkb->map->modmap[keycode];
   else
     input->modifiers &= ~_ecore_wl_disp->xkb->map->modmap[keycode];
}

static void 
_ecore_wl_input_cb_pointer_enter(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface, int sx, int sy)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win = NULL;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   /* _pointer_x = sx; */
   /* _pointer_y = sy; */

   /* input->sx = sx; */
   /* input->sy = sy; */

//   _ecore_wl_input_mouse_move_send(input, timestamp);

   win = input->pointer_focus;
   if ((win) && (win->surface != surface))
     {
        if (!input->button)
          _ecore_wl_input_pointer_focus_remove(input, timestamp);
     }

   if (surface)
     {
        if ((win = wl_surface_get_user_data(surface)))
          {
             input->pointer_focus = win;
             win->pointer_device = input;
          }
        /* if (input->button) */
        /*   { */
        /*      _ecore_wl_input_mouse_up_send(input, timestamp); */
        /*      input->button = 0; */
        /*   } */
        /* else */
          _ecore_wl_input_mouse_in_send(input, timestamp);
     }
}

static void 
_ecore_wl_input_cb_pointer_leave(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface __UNUSED__)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   _ecore_wl_input_pointer_focus_remove(input, timestamp);
}

static void 
_ecore_wl_input_cb_keyboard_enter(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface, struct wl_array *keys)
{
   Ecore_Wl_Input *input;
   Ecore_Wl_Window *win = NULL;
   unsigned int *k, *end;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   end = keys->data + keys->size;
   input->modifiers = 0;
   for (k = keys->data; k < end; k++)
     input->modifiers |= _ecore_wl_disp->xkb->map->modmap[*k];

   if (surface)
     {
        if ((win = wl_surface_get_user_data(surface)))
          {
             input->keyboard_focus = win;
             win->keyboard_device = input;
          }
        else
          input->keyboard_focus = NULL;
        _ecore_wl_input_focus_in_send(input, timestamp);
     }
}

static void 
_ecore_wl_input_cb_keyboard_leave(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface __UNUSED__)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;
   _ecore_wl_input_keyboard_focus_remove(input, timestamp);
}

static void 
_ecore_wl_input_cb_touch_down(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, struct wl_surface *surface __UNUSED__, int id __UNUSED__, int x, int y)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   /* FIXME: NB: Not sure yet if input->timestamp should be set here. 
    * This needs to be tested with an actual touch device */
   /* input->timestamp = timestamp; */
   input->button = 0;
   input->sx = x;
   input->sy = y;
   _ecore_wl_input_mouse_down_send(input, timestamp);
}

static void 
_ecore_wl_input_cb_touch_up(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, int id __UNUSED__)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   /* FIXME: NB: Not sure yet if input->timestamp should be set here. 
    * This needs to be tested with an actual touch device */
   /* input->timestamp = timestamp; */
   input->button = 0;
   _ecore_wl_input_mouse_up_send(input, timestamp);
}

static void 
_ecore_wl_input_cb_touch_motion(void *data, struct wl_input_device *input_device __UNUSED__, unsigned int timestamp, int id __UNUSED__, int x, int y)
{
   Ecore_Wl_Input *input;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(input = data)) return;

   /* FIXME: NB: Not sure yet if input->timestamp should be set here. 
    * This needs to be tested with an actual touch device */
   /* input->timestamp = timestamp; */
   input->sx = x;
   input->sy = y;

   _ecore_wl_input_mouse_move_send(input, timestamp);
}

static void 
_ecore_wl_input_cb_touch_frame(void *data __UNUSED__, struct wl_input_device *input_device __UNUSED__)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
}

static void 
_ecore_wl_input_cb_touch_cancel(void *data __UNUSED__, struct wl_input_device *input_device __UNUSED__)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
}

static void 
_ecore_wl_input_cb_data_offer(void *data, struct wl_data_device *data_device, unsigned int id)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_add(data, data_device, id);
}

static void 
_ecore_wl_input_cb_data_enter(void *data, struct wl_data_device *data_device, unsigned int timestamp, struct wl_surface *surface, int x, int y, struct wl_data_offer *offer)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_enter(data, data_device, timestamp, surface, x, y, offer);
}

static void 
_ecore_wl_input_cb_data_leave(void *data, struct wl_data_device *data_device)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_leave(data, data_device);
}

static void 
_ecore_wl_input_cb_data_motion(void *data, struct wl_data_device *data_device, unsigned int timestamp, int x, int y)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_motion(data, data_device, timestamp, x, y);
}

static void 
_ecore_wl_input_cb_data_drop(void *data, struct wl_data_device *data_device)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_drop(data, data_device);
}

static void 
_ecore_wl_input_cb_data_selection(void *data, struct wl_data_device *data_device, struct wl_data_offer *offer)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_dnd_selection(data, data_device, offer);
}

static void 
_ecore_wl_input_keyboard_focus_remove(Ecore_Wl_Input *input, unsigned int timestamp)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   _ecore_wl_input_focus_out_send(input, timestamp);
   if ((win = input->keyboard_focus))
     win->keyboard_device = NULL;
   input->keyboard_focus = NULL;
}

static void 
_ecore_wl_input_pointer_focus_remove(Ecore_Wl_Input *input, unsigned int timestamp)
{
   Ecore_Wl_Window *win;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!input->button)
     _ecore_wl_input_mouse_out_send(input, timestamp);

   if ((win = input->pointer_focus))
     win->pointer_device = NULL;

   input->pointer_focus = NULL;
}

static void 
_ecore_wl_input_mouse_move_send(Ecore_Wl_Input *input, unsigned int timestamp)
{
   Ecore_Event_Mouse_Move *ev;
//   Ecore_Event *event;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Move)))) return;

   ev->timestamp = timestamp;
   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;
   ev->multi.device = 0;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->sx;
   ev->multi.y = input->sy;

   if (input->grab)
     {
        ev->window = input->grab->id;
        ev->event_window = input->grab->id;
     }
   else if (input->pointer_focus) 
     {
        ev->window = input->pointer_focus->id;
        ev->event_window = input->pointer_focus->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_MOVE, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_in_send(Ecore_Wl_Input *input, unsigned int timestamp)
{
   Ecore_Wl_Event_Mouse_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Mouse_In)))) return;

   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;
   ev->timestamp = timestamp;

   if (input->pointer_focus)
     {
        ev->win = input->pointer_focus->id;
        ev->event_win = input->pointer_focus->id;
     }

   ecore_event_add(ECORE_WL_EVENT_MOUSE_IN, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_out_send(Ecore_Wl_Input *input, unsigned int timestamp)
{
   Ecore_Wl_Event_Mouse_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Mouse_Out)))) return;

   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;
   ev->timestamp = timestamp;

   if (input->pointer_focus)
     {
        ev->win = input->pointer_focus->id;
        ev->event_win = input->pointer_focus->id;
     }

   ecore_event_add(ECORE_WL_EVENT_MOUSE_OUT, ev, NULL, NULL);
}

static void 
_ecore_wl_input_focus_in_send(Ecore_Wl_Input *input, unsigned int timestamp)
{
   Ecore_Wl_Event_Focus_In *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_In)))) return;
   ev->timestamp = timestamp;
   if (input->keyboard_focus)
     ev->win = input->keyboard_focus->id;
   ecore_event_add(ECORE_WL_EVENT_FOCUS_IN, ev, NULL, NULL);
}

static void 
_ecore_wl_input_focus_out_send(Ecore_Wl_Input *input, unsigned int timestamp)
{
   Ecore_Wl_Event_Focus_Out *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = calloc(1, sizeof(Ecore_Wl_Event_Focus_Out)))) return;
   ev->timestamp = timestamp;
   if (input->keyboard_focus)
     ev->win = input->keyboard_focus->id;
   ecore_event_add(ECORE_WL_EVENT_FOCUS_OUT, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_down_send(Ecore_Wl_Input *input, unsigned int timestamp)
{
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   if (input->button == BTN_LEFT)
     ev->buttons = 1;
   else if (input->button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (input->button == BTN_RIGHT)
     ev->buttons = 3;
   else
     ev->buttons = input->button;

   ev->timestamp = timestamp;
   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;

   /* FIXME: Need to get these from wayland somehow */
   ev->double_click = 0;
   ev->triple_click = 0;

   ev->multi.device = 0;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->sx;
   ev->multi.y = input->sy;

   if (input->pointer_focus)
     {
        ev->window = input->pointer_focus->id;
        ev->event_window = input->pointer_focus->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_up_send(Ecore_Wl_Input *input, unsigned int timestamp)
{
   Ecore_Event_Mouse_Button *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Button)))) return;

   if (input->button == BTN_LEFT)
     ev->buttons = 1;
   else if (input->button == BTN_MIDDLE)
     ev->buttons = 2;
   else if (input->button == BTN_RIGHT)
     ev->buttons = 3;
   else
     ev->buttons = input->button;

   ev->timestamp = timestamp;
   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;
   ev->modifiers = input->modifiers;

   /* FIXME: Need to get these from wayland somehow */
   ev->double_click = 0;
   ev->triple_click = 0;

   ev->multi.device = 0;
   ev->multi.radius = 1;
   ev->multi.radius_x = 1;
   ev->multi.radius_y = 1;
   ev->multi.pressure = 1.0;
   ev->multi.angle = 0.0;
   ev->multi.x = input->sx;
   ev->multi.y = input->sy;

   if (input->pointer_focus)
     {
        ev->window = input->pointer_focus->id;
        ev->event_window = input->pointer_focus->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_BUTTON_UP, ev, NULL, NULL);
}

static void 
_ecore_wl_input_mouse_wheel_send(Ecore_Wl_Input *input, unsigned int axis, int value, unsigned int timestamp)
{
   Ecore_Event_Mouse_Wheel *ev;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(ev = malloc(sizeof(Ecore_Event_Mouse_Wheel)))) return;

   ev->timestamp = timestamp;
   ev->modifiers = input->modifiers;
   ev->x = input->sx;
   ev->y = input->sy;
   ev->root.x = input->sx;
   ev->root.y = input->sy;

   if (axis == WL_INPUT_DEVICE_AXIS_VERTICAL_SCROLL)
     {
        ev->direction = value;
        ev->z = 1;
     }
   else if (axis == WL_INPUT_DEVICE_AXIS_HORIZONTAL_SCROLL)
     {
        /* TODO: handle horizontal scroll */
     }

   if (input->pointer_focus)
     {
        ev->window = input->pointer_focus->id;
        ev->event_window = input->pointer_focus->id;
     }

   ecore_event_add(ECORE_EVENT_MOUSE_WHEEL, ev, NULL, NULL);
}
