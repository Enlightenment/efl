#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_MANAGER_CLASS

typedef struct _Object_Gesture
{
   Eo                                    *object;
   const Efl_Event_Description           *type;
   Efl_Canvas_Gesture                    *gesture;
   Efl_Canvas_Gesture_Recognizer         *recognizer;
} Object_Gesture;

typedef struct _Efl_Canvas_Gesture_Manager_Data
{
   //Keeps track of all the gesture request for a particular target
   Eina_Hash    *m_gesture_contex;  // (*target, *event_desc)
   //Keeps all the event directed to this particular object from touch_begin till touch_end
   Eina_Hash    *m_object_events;  // (*target, *efl_gesture_touch)
   //Keeps all the recognizer registered to gesture manager
   Eina_Hash    *m_recognizers; // (*gesture_type, *recognizer)
   //Keeps track of all current object gestures.
   Eina_List    *m_object_gestures; //(List of *object_gesture)
   //Lazy deletion of gestures
   Eina_List    *m_gestures_to_delete;
   //Kepps config values for gesture recognize
   Eina_Hash    *m_config;
} Efl_Canvas_Gesture_Manager_Data;

static void _cleanup_cached_gestures(Efl_Canvas_Gesture_Manager_Data *pd,
                                     Eo *target, const Efl_Event_Description *type);
static Efl_Canvas_Gesture*
_get_state(Efl_Canvas_Gesture_Manager_Data *pd, Eo *target,
                       Efl_Canvas_Gesture_Recognizer *recognizer, const Efl_Event_Description *type);

static void
_cleanup_object(Eina_List *list)
{
   Eina_List *l;
   Eo *obj;

   if (!list) return;

   EINA_LIST_FOREACH(list, l, obj)
     efl_del(obj);

   eina_list_free(list);
}

static void
_hash_unref_cb(Eo *obj)
{
   efl_unref(obj);
}

EOLIAN static Efl_Object *
_efl_canvas_gesture_manager_efl_object_constructor(Eo *obj, Efl_Canvas_Gesture_Manager_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->m_recognizers = eina_hash_pointer_new(EINA_FREE_CB(_hash_unref_cb));
   pd->m_gesture_contex = eina_hash_pointer_new(NULL);
   pd->m_object_events = eina_hash_pointer_new(EINA_FREE_CB(_hash_unref_cb));
   pd->m_object_gestures = NULL;
   pd->m_gestures_to_delete = NULL;

   pd->m_config = eina_hash_string_superfast_new(EINA_FREE_CB(eina_value_free));

   //Register all types of recognizers at very first time.
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_LONG_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_DOUBLE_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_TRIPLE_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_MOMENTUM_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_FLICK_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_ZOOM_CLASS, obj));

   return obj;
}

EOLIAN static Eina_Value *
_efl_canvas_gesture_manager_config_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd, const char *name)
{
   return eina_hash_find(pd->m_config, name);
}

EOLIAN static void
_efl_canvas_gesture_manager_config_set(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd, const char *name, Eina_Value *value)
{
   Eina_Value *v = eina_value_new(eina_value_type_get(value));
   eina_value_copy(value, v);
   eina_hash_add(pd->m_config, name, v);

   //Sets recognizer class property.
   if (!strcmp(name, "glayer_tap_finger_size"))
     {
        int finger_size;
        Efl_Canvas_Gesture_Recognizer *r;
        Efl_Canvas_Gesture_Recognizer_Data *rd;

        eina_value_get(value, &finger_size);

        const Efl_Event_Description *type = EFL_EVENT_GESTURE_TAP;

        r = eina_hash_find(pd->m_recognizers, &type);
        rd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);
        rd->finger_size = finger_size;
     }
}

EOLIAN static void
_efl_canvas_gesture_manager_efl_object_destructor(Eo *obj, Efl_Canvas_Gesture_Manager_Data *pd EINA_UNUSED)
{
   eina_hash_free(pd->m_config);
   eina_hash_free(pd->m_recognizers);
   eina_hash_free(pd->m_gesture_contex);
   eina_hash_free(pd->m_object_events);
   _cleanup_object(pd->m_gestures_to_delete);
   efl_destructor(efl_super(obj, MY_CLASS));
}

void *
_efl_canvas_gesture_manager_private_data_get(Eo *obj)
{
   Efl_Canvas_Gesture_Manager_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   return pd;
}

void
_efl_canvas_gesture_manager_callback_add_hook(void *data, Eo *target, const Efl_Event_Description *type)
{
   Efl_Canvas_Gesture_Manager_Data *pd = data;
   //If there is a recognizer registered for that event then add it to the gesture context
   Efl_Canvas_Gesture_Recognizer *recognizer = eina_hash_find (pd->m_recognizers, &type);
   if (recognizer)
     {
        //Add it to the gesture context.
        eina_hash_list_append(pd->m_gesture_contex, &target, type);
     }
}

void
_efl_canvas_gesture_manager_callback_del_hook(void *data, Eo *target, const Efl_Event_Description *type)
{
   Efl_Canvas_Gesture_Manager_Data *pd = data;
   //If there is a recognizer registered for that event then add it to the gesture context
   Efl_Canvas_Gesture_Recognizer *recognizer = eina_hash_find (pd->m_recognizers, &type);
   if (recognizer)
     {
        eina_hash_list_remove(pd->m_gesture_contex, &target, type);
        _cleanup_cached_gestures(pd, target, type);
     }
}

void
_efl_canvas_gesture_manager_filter_event(Eo *obj, Eo *target, void *event)
{
   Eina_List *l, *gesture_context;
   Efl_Canvas_Gesture_Manager_Data *pd;
   const Efl_Event_Description *gesture_type;
   Efl_Canvas_Gesture_Recognizer *recognizer;
   Efl_Canvas_Gesture *gesture;
   Efl_Canvas_Gesture_Recognizer_Result recog_result;
   Efl_Canvas_Gesture_Recognizer_Result recog_state;
   Efl_Canvas_Gesture_Touch *touch_event;
   Efl_Input_Pointer_Data *pointer_data = efl_data_scope_get(event, EFL_INPUT_POINTER_CLASS);

   pd = efl_data_scope_get(obj, MY_CLASS);
   gesture_context = eina_hash_find(pd->m_gesture_contex, &target);
   if (gesture_context)
     {
        EINA_LIST_FOREACH(gesture_context, l, gesture_type)
          {
             //Check there is already created event exist or not.
             touch_event = eina_hash_find(pd->m_object_events, &gesture_type);

             if (!touch_event)
               {
                  touch_event = efl_add_ref(EFL_CANVAS_GESTURE_TOUCH_CLASS, NULL);
                  eina_hash_add(pd->m_object_events, &gesture_type, touch_event);
               }

             efl_gesture_touch_point_record(touch_event, pointer_data->tool, pointer_data->cur,
                                            pointer_data->timestamp, pointer_data->action);

             //This is for handling the case that mouse event pairs dont match.
			 //Such as the case of canceling gesture recognition after a mouse down.
             if (efl_gesture_touch_state_get(touch_event) == EFL_GESTURE_TOUCH_UNKNOWN)
               continue;

             recognizer = eina_hash_find(pd->m_recognizers, &gesture_type);

			 //If the gesture canceled or already finished by recognizer.
             gesture = _get_state(pd, target, recognizer, gesture_type);
             if (!gesture)
               continue;

			 //Gesture detecting.
             recog_result = efl_gesture_recognizer_recognize(recognizer, gesture, target, touch_event);
             recog_state = recog_result & EFL_GESTURE_RESULT_MASK;

             Efl_Canvas_Gesture_Recognizer_Data *rd =
               efl_data_scope_get(recognizer, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);

             if (recog_state == EFL_GESTURE_TRIGGER)
               {
                  if (efl_gesture_state_get(gesture) == EFL_GESTURE_NONE)
                    efl_gesture_state_set(gesture, EFL_GESTURE_STARTED);
                  else
                    efl_gesture_state_set(gesture, EFL_GESTURE_UPDATED);
               }
             else if (recog_state == EFL_GESTURE_FINISH)
               {
                  efl_gesture_state_set(gesture, EFL_GESTURE_FINISHED);
               }
             else if (recog_state == EFL_GESTURE_MAYBE)
               {
                  continue;
               }
             else if (recog_state == EFL_GESTURE_CANCEL)
               {
                  if (efl_gesture_state_get(gesture) != EFL_GESTURE_NONE)
                    efl_gesture_state_set(gesture, EFL_GESTURE_CANCELED);
                  else
                    {
                       //Need to recognize events that occur consecutively
                       //in a mouse-down state.
                       if (rd->continues)
                         continue;
                    }
               }
             else if (recog_state == EFL_GESTURE_IGNORE)
               {
                  continue;
               }

             efl_gesture_timestamp_set(gesture, efl_gesture_touch_cur_timestamp_get(touch_event));
             efl_event_callback_call(target, gesture_type, gesture);

             //If the current event recognizes the gesture continuously, dont delete gesture.
             if (((recog_state == EFL_GESTURE_FINISH) || (recog_state == EFL_GESTURE_CANCEL)) &&
                 !rd->continues)
               {
                  _cleanup_cached_gestures(pd, target, gesture_type);
                  eina_hash_del(pd->m_object_events, &gesture_type, NULL);
                  //FIXME: delete it by object not list.
                  _cleanup_object(pd->m_gestures_to_delete);
                  pd->m_gestures_to_delete = NULL;

               }
          }
     }
}

EOLIAN static void
_efl_canvas_gesture_manager_recognizer_register(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd,
                                                Efl_Canvas_Gesture_Recognizer *recognizer)
{
   Efl_Canvas_Gesture_Recognizer_Data *rpd;
   Efl_Canvas_Gesture *dummy = efl_gesture_recognizer_add(recognizer, NULL);

   if (!dummy)
     return;

   const Efl_Event_Description *type = efl_gesture_type_get(dummy);

   //Add the recognizer to the m_recognizers
   eina_hash_add(pd->m_recognizers, &type, efl_ref(recognizer));
   //Update the manager
   rpd = efl_data_scope_get(recognizer, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);
   rpd->manager = obj;

   efl_del(dummy);
}

EOLIAN static void
_efl_canvas_gesture_manager_recognizer_unregister(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd,
                                           Efl_Canvas_Gesture_Recognizer *recognizer)
{
   Eina_List *l, *l_next;
   Object_Gesture *object_gesture;
   const Efl_Event_Description *type;
   Efl_Canvas_Gesture *dummy;

   if (!recognizer) return;

   //Find the type of the recognizer
   dummy = efl_gesture_recognizer_add(recognizer, 0);
   if (!dummy)return;

   type = efl_gesture_type_get(dummy);
   efl_del(dummy);

   //Check if its already registered
   recognizer = eina_hash_find(pd->m_recognizers, &type);

   if (!recognizer) return;

   //Remove that gesture from the list of object gestures
   EINA_LIST_FOREACH_SAFE(pd->m_object_gestures, l, l_next, object_gesture)
     {
        if (object_gesture->type == type)
          {
             pd->m_gestures_to_delete = eina_list_append(pd->m_gestures_to_delete, object_gesture->gesture);
             free(object_gesture);
             pd->m_object_gestures = eina_list_remove_list(pd->m_object_gestures, l);
          }
     }

   eina_hash_del(pd->m_recognizers, &type, NULL);
}

static Efl_Canvas_Gesture_Recognizer *
_find_match_recognizer(Efl_Canvas_Gesture_Manager_Data *pd, Efl_Canvas_Gesture_Recognizer_Type type)
{
   const Efl_Event_Description *event_type;

   switch (type)
     {
       case EFL_GESTURE_TAP:
         {
            event_type = EFL_EVENT_GESTURE_TAP;
            break;
         }
       case EFL_GESTURE_DOUBLETAP:
         {
            event_type = EFL_EVENT_GESTURE_DOUBLE_TAP;
            break;
         }
       case EFL_GESTURE_TRIPLETAP:
         {
            event_type = EFL_EVENT_GESTURE_TRIPLE_TAP;
            break;
         }
       case EFL_GESTURE_LONGTAP:
         {
            event_type = EFL_EVENT_GESTURE_LONG_TAP;
            break;
         }
       case EFL_GESTURE_MOMENTUM:
         {
            event_type = EFL_EVENT_GESTURE_MOMENTUM;
            break;
         }
       case EFL_GESTURE_FLICK:
         {
            event_type = EFL_EVENT_GESTURE_FLICK;
            break;
         }
       case EFL_GESTURE_ZOOM:
         {
            event_type = EFL_EVENT_GESTURE_ZOOM;
            break;
         }
       default:
         return NULL;
     }

   return eina_hash_find(pd->m_recognizers, &event_type);
}

EOLIAN static const Efl_Canvas_Gesture_Recognizer *
_efl_canvas_gesture_manager_recognizer_get(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd,
                                           Efl_Canvas_Gesture_Recognizer_Type type)
{
   Efl_Canvas_Gesture_Recognizer *recognizer = _find_match_recognizer(pd, type);

   if (recognizer)
     return recognizer;
   else
     return NULL;
}

// EOLIAN static void
// _efl_canvas_gesture_manager_ungrab_all(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd,
//                                    Eo *target)
// {
//    const Efl_Event_Description *type;
//    Eina_List *l;

//    Eina_List *gesture_type_list = eina_hash_find(pd->m_gesture_contex, &target);

//    if (!gesture_type_list) return;

//    EINA_LIST_FOREACH(gesture_type_list, l, type)
//      {
//         _cleanup_cached_gestures(obj, pd, target, type);
//      }
//    eina_hash_del(pd->m_gesture_contex, &target, NULL);
// }

//Get or create a gesture object that will represent the state for a given object, used by the recognizer
Efl_Canvas_Gesture*
_get_state(Efl_Canvas_Gesture_Manager_Data *pd,
           Eo *target, Efl_Canvas_Gesture_Recognizer *recognizer, const Efl_Event_Description *type)
{
   Eina_List *l;
   Object_Gesture *object_gesture;
   Efl_Canvas_Gesture *gesture;
   Efl_Canvas_Gesture_Recognizer_Data *rd =
     efl_data_scope_get(recognizer, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);

   //If the widget is being deleted we should be careful not to
   //Create a new state.
   if (efl_destructed_is(target))
     return 0;

   //TODO find a optimized way of looking for gesture
   EINA_LIST_FOREACH(pd->m_object_gestures, l, object_gesture)
     {
        if (object_gesture->object == target &&
            object_gesture->recognizer == recognizer &&
            object_gesture->type == type)
         {
            //The gesture is already processed waiting for cleanup
            if (((efl_gesture_state_get(object_gesture->gesture) == EFL_GESTURE_FINISHED) ||
                (efl_gesture_state_get(object_gesture->gesture) == EFL_GESTURE_CANCELED)) &&
                (!rd->continues))
              {
                 _cleanup_cached_gestures(pd, target, type);
                 eina_hash_del(pd->m_object_events, &type, NULL);
                 _cleanup_object(pd->m_gestures_to_delete);
                 pd->m_gestures_to_delete = NULL;
                 return NULL;
              }
            return object_gesture->gesture;
         }
     }

   gesture = efl_gesture_recognizer_add(recognizer, target);
   if (!gesture)
     return 0;

   object_gesture = calloc(1, sizeof(Object_Gesture));
   object_gesture->object = target;
   object_gesture->recognizer = recognizer;
   object_gesture->type = type;
   object_gesture->gesture = gesture;

   pd->m_object_gestures = eina_list_append(pd->m_object_gestures, object_gesture);

   return gesture;
}

static void
_cleanup_cached_gestures(Efl_Canvas_Gesture_Manager_Data *pd,
                         Eo *target, const Efl_Event_Description *type)
{
   Eina_List *l, *l_next;
   Object_Gesture *object_gesture;

   EINA_LIST_FOREACH_SAFE(pd->m_object_gestures, l, l_next, object_gesture)
     {
        if ((object_gesture->type == type) && (target == object_gesture->object))
          {
             pd->m_gestures_to_delete = eina_list_append(pd->m_gestures_to_delete, object_gesture->gesture);
             free(object_gesture);
             pd->m_object_gestures = eina_list_remove_list(pd->m_object_gestures, l);
          }
     }
}

Eina_Bool
_efl_canvas_gesture_manager_watches(const Efl_Event_Description *ev)
{
   /* These are a subset of _elm_win_evas_feed_fake_callbacks
    * in efl_ui_win.c */
   if ((ev == EFL_EVENT_POINTER_MOVE) ||
       (ev == EFL_EVENT_POINTER_DOWN) ||
       (ev == EFL_EVENT_POINTER_UP) ||
       (ev == EFL_EVENT_POINTER_IN) ||
       (ev == EFL_EVENT_POINTER_OUT) ||
       (ev == EFL_EVENT_POINTER_CANCEL) ||
       (ev == EFL_EVENT_POINTER_WHEEL) ||
       (ev == EFL_EVENT_FINGER_MOVE) ||
       (ev == EFL_EVENT_FINGER_DOWN) ||
       (ev == EFL_EVENT_FINGER_UP))
     return EINA_TRUE;
   return EINA_FALSE;
}

void
efl_gesture_manager_gesture_clean_up(Eo *obj, Eo *target, const Efl_Event_Description *type)
{
   Efl_Canvas_Gesture_Manager_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   _cleanup_cached_gestures(pd, target, type);
   eina_hash_del(pd->m_object_events, &type, NULL);
   _cleanup_object(pd->m_gestures_to_delete);
   pd->m_gestures_to_delete = NULL;
}

#include "efl_canvas_gesture_manager.eo.c"
