#define EFL_CANVAS_GESTURE_PROTECTED
#define EFL_CANVAS_GESTURE_CUSTOM_PROTECTED
#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_MANAGER_CLASS

typedef struct _Object_Gesture
{
   Eo                            *object;
   const Efl_Event_Description   *type;
   Efl_Canvas_Gesture            *gesture;
   Efl_Canvas_Gesture_Recognizer *recognizer;
} Object_Gesture;

typedef struct _Efl_Canvas_Gesture_Manager_Data
{
   //Keeps track of all the gesture request for a particular target
   Eina_Hash *m_gesture_contex;     // (*target, Eina_Hash(type, int))
   //Keeps all the event directed to this particular object from touch_begin till touch_end
   Eina_Hash *m_object_events;     // (*recognizer, *efl_gesture_touch)
   //Keeps all the recognizer registered to gesture manager
   Eina_Hash *m_recognizers;    // (*gesture_type, *recognizer)
   Eina_List *custom_recognizers;
   //Keeps track of all current object gestures.
   Eina_List *m_object_gestures;    //(List of *object_gesture)
   //Lazy deletion of gestures
   Eina_Array *m_gestures_to_delete;
   //Kepps config values for gesture recognize
   Eina_Bool processing : 1;
} Efl_Canvas_Gesture_Manager_Data;

static void _cleanup_cached_gestures(Efl_Canvas_Gesture_Manager_Data *pd,
                                      const Eo *target, const Efl_Event_Description *type, const Efl_Canvas_Gesture_Recognizer *recognizer);
static Efl_Canvas_Gesture *
            _get_state(Efl_Canvas_Gesture_Manager_Data *pd, Eo *target,
           Efl_Canvas_Gesture_Recognizer *recognizer, const Efl_Event_Description *type);

static const Efl_Event_Description *
_gesture_recognizer_event_type_get(const Efl_Canvas_Gesture_Recognizer *recognizer)
{
   const Efl_Class *type = efl_gesture_recognizer_type_get(recognizer);

   if (type == EFL_CANVAS_GESTURE_TAP_CLASS)
     return EFL_EVENT_GESTURE_TAP;
   if (type == EFL_CANVAS_GESTURE_DOUBLE_TAP_CLASS)
     return EFL_EVENT_GESTURE_DOUBLE_TAP;
   if (type == EFL_CANVAS_GESTURE_TRIPLE_TAP_CLASS)
     return EFL_EVENT_GESTURE_TRIPLE_TAP;
   if (type == EFL_CANVAS_GESTURE_LONG_PRESS_CLASS)
     return EFL_EVENT_GESTURE_LONG_PRESS;
   if (type == EFL_CANVAS_GESTURE_MOMENTUM_CLASS)
     return EFL_EVENT_GESTURE_MOMENTUM;
   if (type == EFL_CANVAS_GESTURE_FLICK_CLASS)
     return EFL_EVENT_GESTURE_FLICK;
   if (type == EFL_CANVAS_GESTURE_ROTATE_CLASS)
     return EFL_EVENT_GESTURE_ROTATE;
   if (type == EFL_CANVAS_GESTURE_ZOOM_CLASS)
     return EFL_EVENT_GESTURE_ZOOM;
   return EFL_EVENT_GESTURE_CUSTOM;
}

static void
_cleanup_object(Eina_Array *arr)
{
   Eo *obj;

   if (!eina_array_count(arr)) return;

   while ((obj = eina_array_pop(arr)))
     efl_del(obj);
}

static void
_hash_unref_cb(Eo *obj)
{
   efl_unref(obj);
}

/* updates the data for in-tree recognizers without needing to watch events */
static void
_update_finger_sizes(Efl_Canvas_Gesture_Manager_Data *pd, int finger_size)
{
   Efl_Canvas_Gesture_Recognizer *r;
   Efl_Canvas_Gesture_Recognizer_Tap_Data *td;
   Efl_Canvas_Gesture_Recognizer_Double_Tap_Data *dtd;
   Efl_Canvas_Gesture_Recognizer_Triple_Tap_Data *ttd;
   Efl_Canvas_Gesture_Recognizer_Long_Press_Data *ltd;
   Efl_Canvas_Gesture_Recognizer_Flick_Data *fd;
   Efl_Canvas_Gesture_Recognizer_Rotate_Data *rd;
   Efl_Canvas_Gesture_Recognizer_Zoom_Data *zd;
   const Efl_Event_Description *type;

   type = EFL_EVENT_GESTURE_TAP;
   r = eina_hash_find(pd->m_recognizers, &type);
   td = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_TAP_CLASS);
   td->finger_size = finger_size;

   type = EFL_EVENT_GESTURE_DOUBLE_TAP;
   r = eina_hash_find(pd->m_recognizers, &type);
   dtd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_DOUBLE_TAP_CLASS);
   dtd->finger_size = finger_size;

   type = EFL_EVENT_GESTURE_TRIPLE_TAP;
   r = eina_hash_find(pd->m_recognizers, &type);
   ttd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_TRIPLE_TAP_CLASS);
   ttd->finger_size = finger_size;

   type = EFL_EVENT_GESTURE_LONG_PRESS;
   r = eina_hash_find(pd->m_recognizers, &type);
   ltd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_LONG_PRESS_CLASS);
   ltd->finger_size = finger_size;

   type = EFL_EVENT_GESTURE_FLICK;
   r = eina_hash_find(pd->m_recognizers, &type);
   fd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_FLICK_CLASS);
   fd->finger_size = finger_size;

   type = EFL_EVENT_GESTURE_ROTATE;
   r = eina_hash_find(pd->m_recognizers, &type);
   rd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_ROTATE_CLASS);
   rd->finger_size = finger_size;

   type = EFL_EVENT_GESTURE_ZOOM;
   r = eina_hash_find(pd->m_recognizers, &type);
   zd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_ZOOM_CLASS);
   zd->finger_size = finger_size;
}

static void
_gesture_manager_config_changed(void *data, const Efl_Event *ev)
{
   Efl_Canvas_Gesture_Manager_Data *pd = data;

   if ((!ev->info) || eina_streq(ev->info, "glayer_tap_finger_size"))
     _update_finger_sizes(pd, efl_config_int_get(ev->object, "glayer_tap_finger_size"));
}

EOLIAN static Efl_Object *
_efl_canvas_gesture_manager_efl_object_constructor(Eo *obj, Efl_Canvas_Gesture_Manager_Data *pd)
{
   Eo *config;
   int finger_size = 0;
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->m_recognizers = eina_hash_pointer_new(EINA_FREE_CB(_hash_unref_cb));
   pd->m_gesture_contex = eina_hash_pointer_new(EINA_FREE_CB(eina_hash_free));
   pd->m_object_events = eina_hash_pointer_new(EINA_FREE_CB(_hash_unref_cb));
   pd->m_object_gestures = NULL;
   pd->m_gestures_to_delete = eina_array_new(1);;

   /* this needs to always be present */
   config = efl_provider_find(efl_main_loop_get(), EFL_CONFIG_INTERFACE);
   efl_event_callback_add(config, EFL_CONFIG_EVENT_CONFIG_CHANGED, _gesture_manager_config_changed, pd);

   //Register all types of recognizers at very first time.
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_LONG_PRESS_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_DOUBLE_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_TRIPLE_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_MOMENTUM_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_FLICK_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_ROTATE_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_ZOOM_CLASS, obj));
   /* realistically this will never fail, but the whole gesture layer breaks if this is 0 */
   if (!getenv("EFL_RUN_IN_TREE"))
     finger_size = efl_config_int_get(config, "glayer_tap_finger_size");
   if (finger_size < 1) finger_size = 10;
   _update_finger_sizes(pd, finger_size);

   return obj;
}

EOLIAN static void
_efl_canvas_gesture_manager_efl_object_destructor(Eo *obj, Efl_Canvas_Gesture_Manager_Data *pd)
{
   Efl_Canvas_Gesture_Recognizer *recognizer;
   void *ptr;
   Eo *config = efl_provider_find(efl_main_loop_get(), EFL_CONFIG_INTERFACE);
   if (config)
     efl_event_callback_del(config, EFL_CONFIG_EVENT_CONFIG_CHANGED, _gesture_manager_config_changed, pd);

   eina_hash_free(pd->m_recognizers);
   EINA_LIST_FREE(pd->custom_recognizers, recognizer)
     efl_unref(recognizer);
   eina_hash_free(pd->m_gesture_contex);
   eina_hash_free(pd->m_object_events);
   _cleanup_object(pd->m_gestures_to_delete);
   eina_array_free(pd->m_gestures_to_delete);
   EINA_LIST_FREE(pd->m_object_gestures, ptr)
     free(ptr);
   efl_destructor(efl_super(obj, MY_CLASS));
}

void
_efl_canvas_gesture_manager_callback_add_hook(void *data, Eo *target, const Efl_Event_Description *type)
{
   Efl_Canvas_Gesture_Manager_Data *pd = data;
   //If there is a recognizer registered for that event then add it to the gesture context
   Efl_Canvas_Gesture_Recognizer *recognizer;

   if (type == EFL_EVENT_GESTURE_CUSTOM)
     recognizer = eina_list_data_get(pd->custom_recognizers);
   else
     recognizer = eina_hash_find(pd->m_recognizers, &type);
   if (recognizer)
     {
        //Add it to the gesture context.
        Eina_Hash *thisisreallystupid = eina_hash_find(pd->m_gesture_contex, &target);
        intptr_t count = 0;
        /* already present */
        if (thisisreallystupid)
          count = (intptr_t)eina_hash_find(thisisreallystupid, &type);
        else
          {
             thisisreallystupid = eina_hash_pointer_new(NULL);
             eina_hash_set(pd->m_gesture_contex, &target, thisisreallystupid);
          }
        count++;
        eina_hash_set(thisisreallystupid, &type, (intptr_t*)count);
     }
}

void
_efl_canvas_gesture_manager_callback_del_hook(void *data, Eo *target, const Efl_Event_Description *type)
{
   Efl_Canvas_Gesture_Manager_Data *pd = data;
   //If there is a recognizer registered for that event then add it to the gesture context
   Efl_Canvas_Gesture_Recognizer *recognizer;

   if (type == EFL_EVENT_GESTURE_CUSTOM)
     recognizer = eina_list_data_get(pd->custom_recognizers);
   else
     recognizer = eina_hash_find(pd->m_recognizers, &type);
   if (recognizer)
     {
        Eina_Hash *thisisreallystupid = eina_hash_find(pd->m_gesture_contex, &target);
        intptr_t count;

        if (!thisisreallystupid) return;
        count = (intptr_t)eina_hash_find(thisisreallystupid, &type);
        if (!count) return;
        count--;
        eina_hash_set(thisisreallystupid, &type, (intptr_t*)count);
        if (count) return;
        eina_hash_del_by_key(thisisreallystupid, &type);
        if (eina_hash_population(thisisreallystupid)) return;
        eina_hash_del_by_key(pd->m_gesture_contex, &target);
        _cleanup_cached_gestures(pd, target, type, NULL);
     }
}

static void
_recognizer_cleanup_internal(Efl_Canvas_Gesture_Manager_Data *pd, const Efl_Canvas_Gesture_Recognizer *recognizer, const Eo *target, const Efl_Event_Description *type)
{
   _cleanup_cached_gestures(pd, target, type, recognizer);
   eina_hash_del(pd->m_object_events, &recognizer, NULL);
   //FIXME: delete it by object not list.
   _cleanup_object(pd->m_gestures_to_delete);
}

static void
_gesture_recognizer_process_internal(Efl_Canvas_Gesture_Manager_Data *pd, Efl_Canvas_Gesture_Recognizer *recognizer,
                                     Eo *target, const Efl_Event_Description *gesture_type, void *event)
{
   Efl_Canvas_Gesture_Recognizer_Result recog_result;
   Efl_Canvas_Gesture_Recognizer_Result recog_state = EFL_GESTURE_RECOGNIZER_RESULT_FINISH;
   Efl_Canvas_Gesture_Touch *touch_event;
   Efl_Canvas_Gesture_Recognizer_Data *rd = efl_data_scope_get(recognizer, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);
   //If the gesture canceled or already finished by recognizer.
   Efl_Canvas_Gesture *gesture = _get_state(pd, target, recognizer, gesture_type);
   if (!gesture) return;

   //Check there is already created event exist or not.
   touch_event = eina_hash_find(pd->m_object_events, &recognizer);

   if (!touch_event)
     {
        touch_event = efl_add_ref(EFL_CANVAS_GESTURE_TOUCH_CLASS, NULL);
        eina_hash_add(pd->m_object_events, &recognizer, touch_event);
     }

   efl_gesture_touch_point_record(touch_event, event);

   //This is for handling the case that mouse event pairs dont match.
   //Such as the case of canceling gesture recognition after a mouse down.
   if (efl_gesture_touch_state_get(touch_event) == EFL_GESTURE_TOUCH_STATE_UNKNOWN)
     return;
   if ((!rd->continues) && ((efl_gesture_state_get(gesture) == EFL_GESTURE_STATE_CANCELED) ||
       (efl_gesture_state_get(gesture) == EFL_GESTURE_STATE_FINISHED)))
     goto post_event;

   /* this is the "default" value for the event, recognizers may modify it if necessary */
   efl_gesture_touch_count_set(gesture, efl_gesture_touch_points_count_get(touch_event));

   //Gesture detecting.
   recog_result = efl_gesture_recognizer_recognize(recognizer, gesture, target, touch_event);
   recog_state = recog_result & EFL_GESTURE_RECOGNIZER_RESULT_RESULT_MASK;

   if (recog_state == EFL_GESTURE_RECOGNIZER_RESULT_TRIGGER)
     {
        if (efl_gesture_state_get(gesture) == EFL_GESTURE_STATE_NONE)
          efl_gesture_state_set(gesture, EFL_GESTURE_STATE_STARTED);
        else
          efl_gesture_state_set(gesture, EFL_GESTURE_STATE_UPDATED);
     }
   else if (recog_state == EFL_GESTURE_RECOGNIZER_RESULT_FINISH)
     {
        efl_gesture_state_set(gesture, EFL_GESTURE_STATE_FINISHED);
     }
   else if (recog_state == EFL_GESTURE_RECOGNIZER_RESULT_MAYBE)
     {
        return;
     }
   else if (recog_state == EFL_GESTURE_RECOGNIZER_RESULT_CANCEL)
     {
        if (efl_gesture_state_get(gesture) != EFL_GESTURE_STATE_NONE)
          efl_gesture_state_set(gesture, EFL_GESTURE_STATE_CANCELED);
        else
          {
             if (rd->continues)
               return;
             goto post_event;
          }
     }
   else if (recog_state == EFL_GESTURE_RECOGNIZER_RESULT_IGNORE)
     {
        return;
     }

   efl_gesture_timestamp_set(gesture, efl_gesture_touch_current_timestamp_get(touch_event));
   efl_event_callback_call(target, gesture_type, gesture);
post_event:
   /* avoid destroying touch tracking before gesture has ended */
   if ((!rd->continues) &&
        ((efl_gesture_touch_state_get(touch_event) != EFL_GESTURE_TOUCH_STATE_END) || efl_gesture_touch_points_count_get(touch_event)))
     return;
   //If the current event recognizes the gesture continuously, dont delete gesture.
   if (((recog_state == EFL_GESTURE_RECOGNIZER_RESULT_FINISH) || (recog_state == EFL_GESTURE_RECOGNIZER_RESULT_CANCEL)) &&
       !rd->continues)
     _recognizer_cleanup_internal(pd, recognizer, target, gesture_type);
}

void
_efl_canvas_gesture_manager_filter_event(void *data, Eo *target, void *event)
{
   Efl_Canvas_Gesture_Manager_Data *pd = data;
   Eina_Hash *thisisreallystupid;
   Eina_Iterator *it;
   Eina_Hash_Tuple *tup;
   Efl_Canvas_Gesture_Recognizer *recognizer;

   thisisreallystupid = eina_hash_find(pd->m_gesture_contex, &target);
   if (!thisisreallystupid) return;
   pd->processing = EINA_TRUE;
   it = eina_hash_iterator_tuple_new(thisisreallystupid);
   EINA_ITERATOR_FOREACH(it, tup)
     {
        const Efl_Event_Description *gesture_type = *(void**)tup->key;

        if (gesture_type == EFL_EVENT_GESTURE_CUSTOM)
          {
             Eina_List *l;
             EINA_LIST_FOREACH(pd->custom_recognizers, l, recognizer)
               _gesture_recognizer_process_internal(pd, recognizer, target, gesture_type, event);
          }
        else
          {
             recognizer = eina_hash_find(pd->m_recognizers, &gesture_type);
             _gesture_recognizer_process_internal(pd, recognizer, target, gesture_type, event);
          }
     }
   eina_iterator_free(it);
   pd->processing = EINA_FALSE;
}

EOLIAN static void
_efl_canvas_gesture_manager_recognizer_register(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd,
                                                Efl_Canvas_Gesture_Recognizer *recognizer)
{
   const Efl_Event_Description *type = _gesture_recognizer_event_type_get(recognizer);
   //Add the recognizer to the m_recognizers
   if (type == EFL_EVENT_GESTURE_CUSTOM)
     {
        EINA_SAFETY_ON_TRUE_RETURN(pd->custom_recognizers && eina_list_data_find(pd->custom_recognizers, recognizer));
        pd->custom_recognizers = eina_list_append(pd->custom_recognizers, efl_ref(recognizer));
     }
   else
     {
        EINA_SAFETY_ON_TRUE_RETURN(!!eina_hash_find(pd->m_recognizers, &type));
        eina_hash_add(pd->m_recognizers, &type, efl_ref(recognizer));
     }

}

EOLIAN static void
_efl_canvas_gesture_manager_recognizer_unregister(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd,
                                                  Efl_Canvas_Gesture_Recognizer *recognizer)
{
   Eina_List *l, *l_next;
   Object_Gesture *object_gesture;
   const Efl_Event_Description *type;

   if (!recognizer) return;

   //Find the type of the recognizer
   type = _gesture_recognizer_event_type_get(recognizer);
   //Check if its already registered
   if (type == EFL_EVENT_GESTURE_CUSTOM)
     {
        EINA_SAFETY_ON_TRUE_RETURN((!pd->custom_recognizers) || (!eina_list_data_find(pd->custom_recognizers, recognizer)));
        pd->custom_recognizers = eina_list_remove(pd->custom_recognizers, recognizer);
        eina_hash_del_by_key(pd->m_object_events, &recognizer);
        efl_unref(recognizer);
     }
   else
     {
        EINA_SAFETY_ON_TRUE_RETURN(!eina_hash_find(pd->m_recognizers, &type));
        eina_hash_del_by_key(pd->m_object_events, &recognizer);
        eina_hash_del_by_key(pd->m_recognizers, &type);
     }

   //Remove that gesture from the list of object gestures
   EINA_LIST_FOREACH_SAFE(pd->m_object_gestures, l, l_next, object_gesture)
     {
        if ((object_gesture->type == type) && (object_gesture->recognizer == recognizer))
          {
             eina_array_push(pd->m_gestures_to_delete, object_gesture->gesture);
             free(object_gesture);
             pd->m_object_gestures = eina_list_remove_list(pd->m_object_gestures, l);
          }
     }
   if (pd->processing) return;
   _cleanup_object(pd->m_gestures_to_delete);
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
Efl_Canvas_Gesture *
_get_state(Efl_Canvas_Gesture_Manager_Data *pd,
           Eo *target, Efl_Canvas_Gesture_Recognizer *recognizer, const Efl_Event_Description *type)
{
   Eina_List *l;
   Object_Gesture *object_gesture;
   Efl_Canvas_Gesture *gesture;

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
             return object_gesture->gesture;
          }
     }

   gesture = efl_add(efl_gesture_recognizer_type_get(recognizer), recognizer);
   if (!gesture)
     return 0;

   object_gesture = calloc(1, sizeof(Object_Gesture));
   object_gesture->object = target;
   object_gesture->recognizer = recognizer;
   object_gesture->type = type;
   object_gesture->gesture = gesture;
   if (type == EFL_EVENT_GESTURE_CUSTOM)
     efl_gesture_custom_gesture_name_set(gesture, efl_gesture_recognizer_custom_gesture_name_get(recognizer));

   pd->m_object_gestures = eina_list_append(pd->m_object_gestures, object_gesture);

   return gesture;
}

static void
_cleanup_cached_gestures(Efl_Canvas_Gesture_Manager_Data *pd,
                         const Eo *target, const Efl_Event_Description *type, const Efl_Canvas_Gesture_Recognizer *recognizer)
{
   Eina_List *l, *l_next;
   Object_Gesture *object_gesture;

   EINA_LIST_FOREACH_SAFE(pd->m_object_gestures, l, l_next, object_gesture)
     {
        if ((object_gesture->type == type) && (target == object_gesture->object) &&
            ((!recognizer) || (object_gesture->recognizer == recognizer)))
          {
             eina_array_push(pd->m_gestures_to_delete, object_gesture->gesture);
             free(object_gesture);
             pd->m_object_gestures = eina_list_remove_list(pd->m_object_gestures, l);
          }
     }
}

EOLIAN static void
_efl_canvas_gesture_manager_recognizer_cleanup(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd, const Efl_Canvas_Gesture_Recognizer *recognizer, const Eo *target)
{
   const Efl_Event_Description *type;

   EINA_SAFETY_ON_NULL_RETURN(recognizer);

   //Find the type of the recognizer
   type = _gesture_recognizer_event_type_get(recognizer);
   _recognizer_cleanup_internal(pd, recognizer, target, type);
}

#include "efl_canvas_gesture_manager.eo.c"
