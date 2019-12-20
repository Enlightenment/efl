#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_INPUT_CLICKABLE_PROTECTED 1

#include <Evas.h>
#include <Ecore.h>
#include "evas_common_private.h"

typedef struct {
   Eina_Bool pressed;
   int pressed_before;
   Efl_Loop_Timer *timer;
   double clicked_last_time;
} Button_State;

typedef struct {
   Button_State state[3];
   Eina_Bool interaction;
} Efl_Input_Clickable_Data;

#define MY_CLASS EFL_INPUT_CLICKABLE_MIXIN

#define DOUBLE_CLICK_TIME ((double)0.25) //in seconds
#define LONGPRESS_TIMEOUT ((double)1.0) //in seconds

static void
_timer_longpress(void *data, const Efl_Event *ev)
{
   Button_State *state;
   Efl_Input_Clickable_Data *pd = efl_data_scope_get(data, MY_CLASS);

   for (int i = 0; i < 3; ++i)
     {
        state = &pd->state[i];
        if (state->timer == ev->object)
          {
             efl_del(state->timer);
             state->timer = NULL;
             efl_event_callback_call(data, EFL_INPUT_EVENT_LONGPRESSED, &i);
          }
     }
}

EOLIAN static void
_efl_input_clickable_press(Eo *obj EINA_UNUSED, Efl_Input_Clickable_Data *pd, unsigned int button)
{
   Button_State *state;
   EINA_SAFETY_ON_FALSE_RETURN(button < 3);

   pd->interaction = EINA_TRUE;
   INF("Widget %s,%p is pressed(%d)", efl_class_name_get(obj), obj, button);

   state = &pd->state[button];
   EINA_SAFETY_ON_NULL_RETURN(state);

   state->pressed = EINA_TRUE;
   if (state->timer) efl_del(state->timer);
   state->timer = efl_add(EFL_LOOP_TIMER_CLASS, obj,
                                     efl_loop_timer_interval_set(efl_added, LONGPRESS_TIMEOUT),
                                     efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _timer_longpress, obj));

   efl_event_callback_call(obj, EFL_INPUT_EVENT_PRESSED, &button);
   pd->interaction = EINA_FALSE;
}

EOLIAN static void
_efl_input_clickable_unpress(Eo *obj EINA_UNUSED, Efl_Input_Clickable_Data *pd, unsigned int button)
{
   Efl_Input_Clickable_Clicked clicked;
   Button_State *state;
   Eina_Bool pressed;
   EINA_SAFETY_ON_FALSE_RETURN(button < 3);

   pd->interaction = EINA_TRUE;

   state = &pd->state[button];
   EINA_SAFETY_ON_NULL_RETURN(state);

   INF("Widget %s,%p is unpressed(%d):%d", efl_class_name_get(obj), obj, button, state->pressed);

   //eval if this is a repeated click
   if (state->clicked_last_time > 0.0 && ecore_time_unix_get() - state->clicked_last_time < DOUBLE_CLICK_TIME)
     state->pressed_before++;
   else
     state->pressed_before = 0;
   //reset state
   state->clicked_last_time = ecore_time_unix_get();
   pressed = state->pressed;
   state->pressed = EINA_FALSE;
   if (state->timer)
     efl_del(state->timer);
   state->timer = NULL;

   //populate state
   efl_event_callback_call(obj, EFL_INPUT_EVENT_UNPRESSED, &button);
   if (pressed)
     {
        INF("Widget %s,%p is clicked(%d)", efl_class_name_get(obj), obj, button);
        clicked.repeated = state->pressed_before;
        clicked.button = button;
        if (button == 1)
          efl_event_callback_call(obj, EFL_INPUT_EVENT_CLICKED, &clicked);
        efl_event_callback_call(obj, EFL_INPUT_EVENT_CLICKED_ANY, &clicked);
     }
   pd->interaction = EINA_FALSE;
}

EOLIAN static void
_efl_input_clickable_button_state_reset(Eo *obj EINA_UNUSED, Efl_Input_Clickable_Data *pd, unsigned int button)
{
   Button_State *state;
   EINA_SAFETY_ON_FALSE_RETURN(button < 3);

   state = &pd->state[button];
   EINA_SAFETY_ON_NULL_RETURN(state);

   INF("Widget %s,%p is press is aborted(%d):%d", efl_class_name_get(obj), obj, button, state->pressed);

   if (state->timer)
     efl_del(state->timer);
   state->timer = NULL;
   state->pressed = EINA_FALSE;
}

EOLIAN static void
_efl_input_clickable_longpress_abort(Eo *obj EINA_UNUSED, Efl_Input_Clickable_Data *pd, unsigned int button)
{
   Button_State *state;
   EINA_SAFETY_ON_FALSE_RETURN(button < 3);

   state = &pd->state[button];
   EINA_SAFETY_ON_NULL_RETURN(state);

   INF("Widget %s,%p - longpress is aborted(%d)", efl_class_name_get(obj), obj, button);

   if (state->timer)
     efl_del(state->timer);
   state->timer = NULL;
}

EOLIAN static Eina_Bool
_efl_input_clickable_interaction_get(const Eo *obj EINA_UNUSED, Efl_Input_Clickable_Data *pd)
{
   return pd->interaction;
}

#include "efl_input_clickable.eo.c"
