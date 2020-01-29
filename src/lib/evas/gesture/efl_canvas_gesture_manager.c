#define EFL_CANVAS_GESTURE_PROTECTED
#define EFL_CANVAS_GESTURE_CUSTOM_PROTECTED
#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_MANAGER_CLASS
#define EFL_GESTURE_RECOGNIZER_TYPE_TAP_FINGER_SIZE 10

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
   Eina_Hash *m_config;
   Eina_Bool processing : 1;
} Efl_Canvas_Gesture_Manager_Data;

static void _cleanup_cached_gestures(Efl_Canvas_Gesture_Manager_Data *pd,
                                      Eo *target, const Efl_Event_Description *type, Efl_Canvas_Gesture_Recognizer *recognizer);
static Efl_Canvas_Gesture *
            _get_state(Efl_Canvas_Gesture_Manager_Data *pd, Eo *target,
           Efl_Canvas_Gesture_Recognizer *recognizer, const Efl_Event_Description *type);

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
   Efl_Canvas_Gesture_Recognizer_Long_Tap_Data *ltd;
   Efl_Canvas_Gesture_Recognizer_Flick_Data *fd;
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

   type = EFL_EVENT_GESTURE_LONG_TAP;
   r = eina_hash_find(pd->m_recognizers, &type);
   ltd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_LONG_TAP_CLASS);
   ltd->finger_size = finger_size;

   type = EFL_EVENT_GESTURE_FLICK;
   r = eina_hash_find(pd->m_recognizers, &type);
   fd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_FLICK_CLASS);
   fd->finger_size = finger_size;

   type = EFL_EVENT_GESTURE_ZOOM;
   r = eina_hash_find(pd->m_recognizers, &type);
   zd = efl_data_scope_get(r, EFL_CANVAS_GESTURE_RECOGNIZER_ZOOM_CLASS);
   zd->finger_size = finger_size;
}

EOLIAN static Efl_Object *
_efl_canvas_gesture_manager_efl_object_constructor(Eo *obj, Efl_Canvas_Gesture_Manager_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->m_recognizers = eina_hash_pointer_new(EINA_FREE_CB(_hash_unref_cb));
   pd->m_gesture_contex = eina_hash_pointer_new(EINA_FREE_CB(eina_hash_free));
   pd->m_object_events = eina_hash_pointer_new(EINA_FREE_CB(_hash_unref_cb));
   pd->m_object_gestures = NULL;
   pd->m_gestures_to_delete = eina_array_new(1);;

   pd->m_config = eina_hash_string_superfast_new(EINA_FREE_CB(eina_value_free));
   /* this needs to always be present */
   eina_hash_add(pd->m_config, "glayer_tap_finger_size", eina_value_int_new(EFL_GESTURE_RECOGNIZER_TYPE_TAP_FINGER_SIZE));

   //Register all types of recognizers at very first time.
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_LONG_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_DOUBLE_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_TRIPLE_TAP_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_MOMENTUM_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_FLICK_CLASS, obj));
   efl_gesture_manager_recognizer_register(obj, efl_add(EFL_CANVAS_GESTURE_RECOGNIZER_ZOOM_CLASS, obj));
   _update_finger_sizes(pd, EFL_GESTURE_RECOGNIZER_TYPE_TAP_FINGER_SIZE);

   return obj;
}

EOLIAN static Eina_Value *
_efl_canvas_gesture_manager_config_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Manager_Data *pd, const char *name)
{
   return eina_hash_find(pd->m_config, name);
}

EOLIAN static void
_efl_canvas_gesture_manager_config_set(Eo *obj, Efl_Canvas_Gesture_Manager_Data *pd, const char *name, Eina_Value *value)
{
   Eina_Value *v;
   int finger_size;

   EINA_SAFETY_ON_NULL_RETURN(name);
   v = eina_value_new(eina_value_type_get(value));
   eina_value_copy(value, v);
   eina_hash_add(pd->m_config, name, v);
   efl_event_callback_call(obj, EFL_GESTURE_MANAGER_EVENT_CONFIG_CHANGED, (void*)name);
   if (!eina_streq(name, "glayer_tap_finger_size")) return;
   eina_value_get(value, &finger_size);
   _update_finger_sizes(pd, finger_size);
}

EOLIAN static void
_efl_canvas_gesture_manager_efl_object_destructor(Eo *obj, Efl_Canvas_Gesture_Manager_Data *pd EINA_UNUSED)
{
   Efl_Canvas_Gesture_Recognizer *recognizer;
   void *ptr;

   eina_hash_free(pd->m_config);
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
   Efl_Canvas_Gesture_Recognizer *recognizer;

   if (type == EFL_EVENT_GESTURE_CUSTOM)
     recognizer = eina_list_data_get(pd->custom_recognizers);
   else
     recognizer = eina_hash_find(pd->m_recognizers, &type);
   if (recognizer)
     {
        //Add it to the gesture context.
        Eina_Hash *thisisreallystupid = eina_hash_find(pd->m_gesture_contex, &target);
        int count = 0;
        /* already present */
        if (thisisreallystupid)
          count = (intptr_t)eina_hash_find(thisisreallystupid, &type);
        else
          {
             thisisreallystupid = eina_hash_pointer_new(NULL);
             eina_hash_set(pd->m_gesture_contex, &target, thisisreallystupid);
          }
        count++;
        eina_hash_set(thisisreallystupid, &type, (intptr_t*)(long)count);
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
        int count;

        if (!thisisreallystupid) return;
        count = (intptr_t)eina_hash_find(thisisreallystupid, &type);
        if (!count) return;
        count--;
        eina_hash_set(thisisreallystupid, &type, (intptr_t*)(long)count);
        if (count) return;
        eina_hash_del_by_key(thisisreallystupid, &type);
        if (eina_hash_population(thisisreallystupid)) return;
        eina_hash_del_by_key(pd->m_gesture_contex, &target);
        _cleanup_cached_gestures(pd, target, type, NULL);
     }
}

static void
_gesture_recognizer_process_internal(Efl_Canvas_Gesture_Manager_Data *pd, Efl_Canvas_Gesture_Recognizer *recognizer,
                                     Eo *target, const Efl_Event_Description *gesture_type, void *event)
{
   Efl_Canvas_Gesture_Recognizer_Result recog_result;
   Efl_Canvas_Gesture_Recognizer_Result recog_state;
   Efl_Canvas_Gesture_Touch *touch_event;
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


   /* this is the "default" value for the event, recognizers may modify it if necessary */
   efl_gesture_touch_count_set(gesture, efl_gesture_touch_points_count_get(touch_event));

   //Gesture detecting.
   recog_result = efl_gesture_recognizer_recognize(recognizer, gesture, target, touch_event);
   recog_state = recog_result & EFL_GESTURE_RECOGNIZER_RESULT_RESULT_MASK;

   Efl_Canvas_Gesture_Recognizer_Data *rd =
     efl_data_scope_get(recognizer, EFL_CANVAS_GESTURE_RECOGNIZER_CLASS);

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

   efl_gesture_timestamp_set(gesture, efl_gesture_touch_cur_timestamp_get(touch_event));
   efl_event_callback_call(target, gesture_type, gesture);
post_event:
   //If the current event recognizes the gesture continuously, dont delete gesture.
   if (((recog_state == EFL_GESTURE_RECOGNIZER_RESULT_FINISH) || (recog_state == EFL_GESTURE_RECOGNIZER_RESULT_CANCEL)) &&
       !rd->continues)
     {
        _cleanup_cached_gestures(pd, target, gesture_type, recognizer);
        eina_hash_del(pd->m_object_events, &recognizer, NULL);
        //FIXME: delete it by object not list.
        _cleanup_object(pd->m_gestures_to_delete);
     }
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
   Efl_Canvas_Gesture *dummy = efl_gesture_recognizer_add(recognizer, NULL);

   if (!dummy)
     return;

   const Efl_Event_Description *type = _efl_gesture_type_get(dummy);
   efl_del(dummy);
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
   Efl_Canvas_Gesture *dummy;

   if (!recognizer) return;

   //Find the type of the recognizer
   dummy = efl_gesture_recognizer_add(recognizer, 0);
   if (!dummy) return;

   type = _efl_gesture_type_get(dummy);
   efl_del(dummy);
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
             if (((efl_gesture_state_get(object_gesture->gesture) == EFL_GESTURE_STATE_FINISHED) ||
                  (efl_gesture_state_get(object_gesture->gesture) == EFL_GESTURE_STATE_CANCELED)) &&
                 (!rd->continues))
               {
                  _cleanup_cached_gestures(pd, target, type, recognizer);
                  eina_hash_del(pd->m_object_events, &recognizer, NULL);
                  _cleanup_object(pd->m_gestures_to_delete);
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
   if (type == EFL_EVENT_GESTURE_CUSTOM)
     efl_gesture_custom_gesture_name_set(gesture, efl_gesture_recognizer_custom_gesture_name_get(recognizer));

   pd->m_object_gestures = eina_list_append(pd->m_object_gestures, object_gesture);

   return gesture;
}

static void
_cleanup_cached_gestures(Efl_Canvas_Gesture_Manager_Data *pd,
                         Eo *target, const Efl_Event_Description *type, Efl_Canvas_Gesture_Recognizer *recognizer)
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

void
efl_gesture_manager_gesture_clean_up(Eo *obj, Eo *target, const Efl_Event_Description *type, Efl_Canvas_Gesture_Recognizer *recognizer)
{
   Efl_Canvas_Gesture_Manager_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   EINA_SAFETY_ON_NULL_RETURN(recognizer);
   _cleanup_cached_gestures(pd, target, type, recognizer);
   eina_hash_del(pd->m_object_events, &recognizer, NULL);
   _cleanup_object(pd->m_gestures_to_delete);
}

#include "efl_canvas_gesture_manager.eo.c"
