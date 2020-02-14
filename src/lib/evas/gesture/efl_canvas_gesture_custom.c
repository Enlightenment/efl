#define EFL_CANVAS_GESTURE_CUSTOM_PROTECTED
#include "efl_canvas_gesture_private.h"

#define MY_CLASS EFL_CANVAS_GESTURE_CUSTOM_CLASS


EOLIAN static void
_efl_canvas_gesture_custom_efl_object_destructor(Eo *obj, Efl_Canvas_Gesture_Custom_Data *pd)
{
   eina_stringshare_del(pd->gesture_name);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Object *
_efl_canvas_gesture_custom_efl_object_constructor(Eo *obj, Efl_Canvas_Gesture_Custom_Data *pd EINA_UNUSED)
{
   Efl_Canvas_Gesture_Data *gd;

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   gd = efl_data_scope_get(obj, EFL_CANVAS_GESTURE_CLASS);

   return obj;
}

EOLIAN static void
_efl_canvas_gesture_custom_gesture_name_set(Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Custom_Data *pd, const char *name)
{
   eina_stringshare_replace(&pd->gesture_name, name);
}

EOLIAN static Eina_Stringshare *
_efl_canvas_gesture_custom_gesture_name_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Gesture_Custom_Data *pd)
{
   return pd->gesture_name;
}
#include "efl_canvas_gesture_custom.eo.c"
