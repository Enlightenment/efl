#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include <Efl.h>
#include <stdio.h>
#include <Ecore.h>
#include <Evas.h>
#include <Eina.h>
#include <Ecore_Evas.h>
#include <Ecore_Input.h>

static Eina_Bool
_anim(void *data)
{
   static enum { RIGHT, LEFT } direction = LEFT;
   static const int speed = 20;
   int x, y;
   Evas_Object *rect = data;

   evas_object_geometry_get(rect, &x, &y, NULL, NULL);
   if (direction == LEFT)
     {
        x -= speed;
        if (x <= 0)
          {
             x = 0;
             direction = RIGHT;
          }
     }
   else
     {
        x += speed;
        if (x >= 800)
          {
             direction = LEFT;
             x = 800;
          }
     }

   evas_object_move(rect, x, y);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_accept_cb(void *data EINA_UNUSED, Ecore_Evas *ee EINA_UNUSED, const char *client_host)
{
   printf("Client %s trying to connect\n", client_host);
   return EINA_TRUE;
}

static Efl_Input_Device *
_get_seat(Efl_Input_Device *dev)
{
   if (!dev)
     return NULL;

   while ((dev = efl_input_device_parent_get(dev)))
     {
        if (efl_input_device_type_get(dev) == EFL_INPUT_DEVICE_CLASS_SEAT)
          return dev;
     }
   return NULL;
}

static Eina_Bool
_keyboard_event(void *data EINA_UNUSED, int type, void *event)
{
   Ecore_Event_Key *e = event;
   Efl_Input_Device *seat = NULL;

   if (e->dev)
     seat = _get_seat(e->dev);

   printf("The keyboard on seat '%s' %s the key '%s'\n", seat ?
          efl_input_device_name_get(seat) : "default",
          type == ECORE_EVENT_KEY_DOWN ? "pressed" : "released",
          e->keyname);

   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_mouse_move(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Move *e = event;
   Efl_Input_Device *seat = NULL;

   if (e->dev)
     seat = _get_seat(e->dev);

   printf("The mouse on seat '%s' is at X: %d Y:%d\n",
          seat ? efl_input_device_name_get(seat) : "default", e->x, e->y);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_mouse_button(void *data EINA_UNUSED, int type, void *event)
{
   Ecore_Event_Mouse_Button *e = event;
   Efl_Input_Device *seat = NULL;

   if (e->dev)
     seat = _get_seat(e->dev);

   printf("The mouse on seat '%s' %s the following button '%d'\n",
          seat ? efl_input_device_name_get(seat) : "default",
          type == ECORE_EVENT_MOUSE_BUTTON_DOWN ? "pressed" : "released",
          e->buttons);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_mouse_wheel(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Mouse_Wheel *e = event;
   Efl_Input_Device *seat = NULL;

   if (e->dev)
     seat = _get_seat(e->dev);

   printf("The mouse on seat '%s' moved the wheel '%s'\n",
          seat ? efl_input_device_name_get(seat) : "default",
          e->z < 0 ? "up" : "down");
   return ECORE_CALLBACK_PASS_ON;
}

static const char *
_device_type_to_string(Efl_Input_Device_Class klass)
{
   switch (klass)
     {
      case EFL_INPUT_DEVICE_CLASS_NONE:
         return "None";
      case EFL_INPUT_DEVICE_CLASS_SEAT:
         return "Seat";
      case EFL_INPUT_DEVICE_CLASS_KEYBOARD:
         return "Keyboard";
      case EFL_INPUT_DEVICE_CLASS_MOUSE:
         return "Mouse";
      case EFL_INPUT_DEVICE_CLASS_TOUCH:
         return "Touch";
      case EFL_INPUT_DEVICE_CLASS_PEN:
         return "Pen";
      case EFL_INPUT_DEVICE_CLASS_WAND:
         return "Wand";
      case EFL_INPUT_DEVICE_CLASS_GAMEPAD:
         return "Gamepad";
      default:
         return "Unknown";
     }
}

static void
_dev_added_or_removed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Device *dev = event->info;

   printf("The device '%s' - class: '%s' - description: '%s' was '%s'\n",
          efl_input_device_name_get(dev),
          _device_type_to_string(efl_input_device_type_get(dev)),
          efl_input_device_description_get(dev),
          event->desc == EFL_CANVAS_EVENT_DEVICE_ADDED ? "added" : "removed");
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   Ecore_Evas *ee;
   Evas *evas;
   Evas_Object *bg, *rect;
   Ecore_Animator *animator;
   Eina_Bool r;
   Ecore_Event_Handler *keydown_handler, *keyup_handler, *mouse_move,
     *mouse_down, *mouse_up, *mouse_wheel;

   ecore_evas_init();

   ee = ecore_evas_new(NULL, 0, 0, 800, 600, NULL);

   if (!ee)
     {
        fprintf(stderr, "Could not create the ecore evas\n");
        return -1;
     }

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_move(bg, 0, 0);
   evas_object_resize(bg, 800, 600);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas);
   evas_object_color_set(rect, 0, 255, 0, 255);
   evas_object_resize(rect, 50, 50);
   evas_object_move(rect, (800 - 50) /2, (600 - 50)/2);
   evas_object_show(rect);

   animator = ecore_animator_add(_anim, rect);

   ecore_evas_show(ee);

   r = ecore_evas_vnc_start(ee, "localhost", -1, _accept_cb, NULL);

   if (!r)
     {
        fprintf(stderr, "Could not enable the VNC support!\n");
        goto exit;
     }

   keydown_handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN,
                                             _keyboard_event, NULL);
   keyup_handler = ecore_event_handler_add(ECORE_EVENT_KEY_UP,
                                           _keyboard_event, NULL);
   mouse_move = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, _mouse_move,
                                        NULL);
   mouse_up = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP,
                                      _mouse_button, NULL);
   mouse_down = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN,
                                        _mouse_button, NULL);
   mouse_wheel = ecore_event_handler_add(ECORE_EVENT_MOUSE_WHEEL,
                                        _mouse_wheel, NULL);

   efl_event_callback_add(evas, EFL_CANVAS_EVENT_DEVICE_ADDED,
                          _dev_added_or_removed, NULL);
   efl_event_callback_add(evas, EFL_CANVAS_EVENT_DEVICE_REMOVED,
                          _dev_added_or_removed, NULL);
   ecore_main_loop_begin();

   ecore_event_handler_del(mouse_wheel);
   ecore_event_handler_del(keydown_handler);
   ecore_event_handler_del(keyup_handler);
   ecore_event_handler_del(mouse_move);
   ecore_event_handler_del(mouse_up);
   ecore_event_handler_del(mouse_down);

 exit:
   ecore_evas_free(ee);
   ecore_animator_del(animator);
   ecore_evas_shutdown();
   return 0;
}
