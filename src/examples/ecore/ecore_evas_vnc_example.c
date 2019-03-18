#define EFL_BETA_API_SUPPORT

#include <Efl.h>
#include <stdio.h>
#include <Ecore.h>
#include <Evas.h>
#include <Eina.h>
#include <Ecore_Evas.h>
#include <Ecore_Input.h>
#include <Ecore_Getopt.h>

static int width = 800;

static void
_resize_cb(Ecore_Evas *ee)
{
   Evas_Object *vnc;
   int w = 0, h = 0;

   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

   fprintf(stderr, "resizing [%i, %i]\n", w, h);
   vnc = ecore_evas_data_get(ee, "vnc");
   evas_object_resize(vnc, w, h);
}

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
        if (x >= width)
          {
             direction = LEFT;
             x = width;
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

static void
_disc_cb(void *data EINA_UNUSED, Ecore_Evas *ee EINA_UNUSED, const char *client_host)
{
   printf("Client %s disconnected\n", client_host);
}

static Eina_Bool
_keyboard_event(void *data EINA_UNUSED, int type, void *event)
{
   Ecore_Event_Key *e = event;
   Efl_Input_Device *seat = NULL;

   if (e->dev)
     seat = efl_input_device_seat_get(e->dev);

   printf("The keyboard on seat '%s' %s the key '%s'\n", seat ?
          efl_name_get(seat) : "default",
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
     seat = efl_input_device_seat_get(e->dev);

   printf("The mouse on seat '%s' is at X: %d Y:%d\n",
          seat ? efl_name_get(seat) : "default", e->x, e->y);
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_mouse_button(void *data EINA_UNUSED, int type, void *event)
{
   Ecore_Event_Mouse_Button *e = event;
   Efl_Input_Device *seat = NULL;

   if (e->dev)
     seat = efl_input_device_seat_get(e->dev);

   printf("The mouse on seat '%s' %s the following button '%d'\n",
          seat ? efl_name_get(seat) : "default",
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
     seat = efl_input_device_seat_get(e->dev);

   printf("The mouse on seat '%s' moved the wheel '%s'\n",
          seat ? efl_name_get(seat) : "default",
          e->z < 0 ? "up" : "down");
   return ECORE_CALLBACK_PASS_ON;
}

static const char *
_device_type_to_string(Efl_Input_Device_Type klass)
{
   switch (klass)
     {
      case EFL_INPUT_DEVICE_TYPE_NONE:
         return "None";
      case EFL_INPUT_DEVICE_TYPE_SEAT:
         return "Seat";
      case EFL_INPUT_DEVICE_TYPE_KEYBOARD:
         return "Keyboard";
      case EFL_INPUT_DEVICE_TYPE_MOUSE:
         return "Mouse";
      case EFL_INPUT_DEVICE_TYPE_TOUCH:
         return "Touch";
      case EFL_INPUT_DEVICE_TYPE_PEN:
         return "Pen";
      case EFL_INPUT_DEVICE_TYPE_WAND:
         return "Wand";
      case EFL_INPUT_DEVICE_TYPE_GAMEPAD:
         return "Gamepad";
      default:
         return "Unknown";
     }
}

static void
_seat_children_print(Efl_Input_Device *seat)
{
   Efl_Input_Device *child;
   Eina_Iterator *it;

   printf("Children of seat: %s (%s, seat id: %d)\n", efl_name_get(seat),
          _device_type_to_string(efl_input_device_type_get(seat)),
          efl_input_device_seat_id_get(seat));

   it = efl_input_device_children_iterate(seat);
   EINA_ITERATOR_FOREACH(it, child)
     {
        printf(" - Sub device: %s (%s, seat id: %d)\n", efl_name_get(child),
               _device_type_to_string(efl_input_device_type_get(child)),
               efl_input_device_seat_id_get(seat));
     }
   eina_iterator_free(it);
}

static void
_dev_added_or_removed(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Device *dev = event->info;

   printf("The device '%s' - class: '%s' - description: '%s' was '%s'\n",
          efl_name_get(dev),
          _device_type_to_string(efl_input_device_type_get(dev)),
          efl_comment_get(dev),
          event->desc == EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED ? "added" : "removed");

   if (efl_input_device_type_get(dev) == EFL_INPUT_DEVICE_TYPE_SEAT)
     _seat_children_print(dev);
}

int
main(int argc, char *argv[])
{
   Ecore_Evas *ee;
   Evas *evas;
   Evas_Object *bg, *rect;
   Ecore_Animator *animator;
   Evas_Object *vnc;
   Ecore_Event_Handler *keydown_handler, *keyup_handler, *mouse_move,
     *mouse_down, *mouse_up, *mouse_wheel;
   char *engine = "software_x11";
   int args, height = 600;
   Eina_Bool quit_option = EINA_FALSE;
   static const Ecore_Getopt options = {
     "ecore_evas_vnc_example",
     NULL,
     "0.1",
     "(C) 2016 Enlightenment Project",
     "BSD 2-Clause",
     "Ecore_Evas VNC example.\n",
     EINA_TRUE,
     {
       ECORE_GETOPT_STORE_DEF_STR('e', "engine", "The engine backend", "software_x11"),
       ECORE_GETOPT_STORE_DEF_INT('w', "width", "The window width", 800),
       ECORE_GETOPT_STORE_DEF_INT('h', "height", "The window height", 600),
       ECORE_GETOPT_VERSION('v', "version"),
       ECORE_GETOPT_COPYRIGHT('c', "copyright"),
       ECORE_GETOPT_LICENSE('k', "license"),
       ECORE_GETOPT_HELP('H', "help"),
       ECORE_GETOPT_SENTINEL
     }
   };
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(engine),
     ECORE_GETOPT_VALUE_INT(width),
     ECORE_GETOPT_VALUE_INT(height),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_NONE
   };

   ecore_evas_init();

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fprintf(stderr, "Could not parse command line options.\n");
        return -1;
     }

   if (quit_option) return 0;

   printf("Using engine '%s'. Width: %d - Height: %d\n", engine, width, height);
   ee = ecore_evas_new(engine, 0, 0, width, height, NULL);

   if (!ee)
     {
        fprintf(stderr, "Could not create the ecore evas\n");
        return -1;
     }

   evas = ecore_evas_get(ee);

   bg = evas_object_rectangle_add(evas);
   evas_object_color_set(bg, 255, 255, 255, 255);
   evas_object_move(bg, 0, 0);
   evas_object_resize(bg, width, height);
   evas_object_show(bg);

   rect = evas_object_rectangle_add(evas);
   evas_object_color_set(rect, 0, 255, 0, 255);
   evas_object_resize(rect, 50, 50);
   evas_object_move(rect, (width - 50) /2, (height - 50)/2);
   evas_object_show(rect);

   animator = ecore_animator_add(_anim, rect);

   ecore_evas_show(ee);

   vnc = ecore_evas_vnc_start(ee, "localhost", -1, _accept_cb, _disc_cb, NULL);
   ecore_evas_data_set(ee, "vnc", vnc);
   ecore_evas_callback_resize_set(ee, _resize_cb);
   evas_object_resize(vnc, width, height);
   evas_object_show(vnc);

   if (!vnc)
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

   _seat_children_print(evas_default_device_get(evas, EFL_INPUT_DEVICE_TYPE_SEAT));
   efl_event_callback_add(evas, EFL_CANVAS_SCENE_EVENT_DEVICE_ADDED,
                          _dev_added_or_removed, NULL);
   efl_event_callback_add(evas, EFL_CANVAS_SCENE_EVENT_DEVICE_REMOVED,
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
