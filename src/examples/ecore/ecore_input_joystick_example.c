//Compile with:
// gcc -g -Wall -o ecore_input_joystick_example ecore_input_joystick_example.c `pkg-config --cflags --libs ecore ecore-input`

#include <Ecore.h>
#include <Ecore_Input.h>

static Eina_Bool
_joystick_event_handler_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   printf("deadzone: %d\n", ecore_input_joystick_event_axis_deadzone_get());
   Ecore_Event_Joystick *ev = event;
   switch (ev->type)
     {
      case ECORE_EVENT_JOYSTICK_EVENT_TYPE_CONNECTED:
        printf("joystick is connected: %d\n", ev->index);
        break;
      case ECORE_EVENT_JOYSTICK_EVENT_TYPE_DISCONNECTED:
        printf("joystick is disconnected: %d\n", ev->index);
        break;
      case ECORE_EVENT_JOYSTICK_EVENT_TYPE_BUTTON:
        printf("joystick(%d) button index: %d, value: %f, time: %u\n",
                                          ev->index, ev->button.index,
                                     ev->button.value, ev->timestamp);
        break;
      case ECORE_EVENT_JOYSTICK_EVENT_TYPE_AXIS:
        printf("joystick(%d) axis index: %d, value: %f, time: %u\n",
                                          ev->index, ev->axis.index,
                                     ev->axis.value, ev->timestamp);
        break;
      default:
        printf("unhandled event type: %d\n", ev->type);
        break;
     }

   const char *joystick_name;
   joystick_name = ecore_input_joystick_name_get(ev->index);
   printf("joystick name is: %s (index: %d)\n", joystick_name, ev->index);

   if (ev->type == ECORE_EVENT_JOYSTICK_EVENT_TYPE_BUTTON &&
       ev->button.index == ECORE_EVENT_JOYSTICK_BUTTON_START)
     ecore_main_loop_quit();

   return ECORE_CALLBACK_DONE;
}

int
main(void)
{
   if (!ecore_event_init())
     {
        printf("ERROR: Cannot init Ecore!\n");
        return -1;
     }

   ecore_event_handler_add(ECORE_EVENT_JOYSTICK,
                           _joystick_event_handler_cb,
                           NULL);
   ecore_input_joystick_event_axis_deadzone_set(300);

   printf("start the main loop.\n");

   ecore_main_loop_begin();

   ecore_shutdown();

   return 0;
}
