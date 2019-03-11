
void _elm_bubble_pos_set(Eo *obj, Elm_Bubble_Data *pd, Elm_Bubble_Pos pos);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_bubble_pos_set, EFL_FUNC_CALL(pos), Elm_Bubble_Pos pos);

Elm_Bubble_Pos _elm_bubble_pos_get(const Eo *obj, Elm_Bubble_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_bubble_pos_get, Elm_Bubble_Pos, 0);

Efl_Object *_elm_bubble_efl_object_constructor(Eo *obj, Elm_Bubble_Data *pd);


void _elm_bubble_efl_ui_widget_on_access_update(Eo *obj, Elm_Bubble_Data *pd, Eina_Bool enable);


Efl_Object *_elm_bubble_efl_part_part_get(const Eo *obj, Elm_Bubble_Data *pd, const char *name);


static Eina_Bool
_elm_bubble_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_BUBBLE_EXTRA_OPS
#define ELM_BUBBLE_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_bubble_pos_set, _elm_bubble_pos_set),
      EFL_OBJECT_OP_FUNC(elm_obj_bubble_pos_get, _elm_bubble_pos_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_bubble_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_update, _elm_bubble_efl_ui_widget_on_access_update),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_bubble_efl_part_part_get),
      ELM_BUBBLE_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_bubble_class_desc = {
   EO_VERSION,
   "Elm.Bubble",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Bubble_Data),
   _elm_bubble_class_initializer,
   _elm_bubble_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_bubble_class_get, &_elm_bubble_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_CLICKABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_bubble_eo.legacy.c"
