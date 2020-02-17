#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_RECOGNIZER_CUSTOM_CLASS

EOLIAN static const Efl_Class *
_efl_canvas_gesture_recognizer_custom_efl_canvas_gesture_recognizer_type_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Custom_Data *pd EINA_UNUSED)
{
   return EFL_CANVAS_GESTURE_CUSTOM_CLASS;
}

EOLIAN static Eo *
_efl_canvas_gesture_recognizer_custom_efl_object_finalize(Eo *obj, Efl_Canvas_Gesture_Recognizer_Custom_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->name, NULL);
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_canvas_gesture_recognizer_custom_efl_object_destructor(Eo *obj, Efl_Canvas_Gesture_Recognizer_Custom_Data *pd)
{
   eina_stringshare_del(pd->name);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_canvas_gesture_recognizer_custom_gesture_name_set(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Custom_Data *pd, const char *name)
{
   eina_stringshare_replace(&pd->name, name);
}

EOLIAN static Eina_Stringshare *
_efl_canvas_gesture_recognizer_custom_gesture_name_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Recognizer_Custom_Data *pd)
{
   return pd->name;
}

#include "efl_canvas_gesture_recognizer_custom.eo.c"
