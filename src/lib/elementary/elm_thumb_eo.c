EWAPI const Efl_Event_Description _ELM_THUMB_EVENT_GENERATE_ERROR =
   EFL_EVENT_DESCRIPTION("generate,error");
EWAPI const Efl_Event_Description _ELM_THUMB_EVENT_GENERATE_START =
   EFL_EVENT_DESCRIPTION("generate,start");
EWAPI const Efl_Event_Description _ELM_THUMB_EVENT_GENERATE_STOP =
   EFL_EVENT_DESCRIPTION("generate,stop");
EWAPI const Efl_Event_Description _ELM_THUMB_EVENT_LOAD_ERROR =
   EFL_EVENT_DESCRIPTION("load,error");
EWAPI const Efl_Event_Description _ELM_THUMB_EVENT_PRESS =
   EFL_EVENT_DESCRIPTION("press");

Efl_Object *_elm_thumb_efl_object_constructor(Eo *obj, Elm_Thumb_Data *pd);


Efl_Object *_elm_thumb_efl_object_finalize(Eo *obj, Elm_Thumb_Data *pd);


Eina_Error _elm_thumb_efl_file_load(Eo *obj, Elm_Thumb_Data *pd);


Eina_Bool _elm_thumb_efl_file_loaded_get(const Eo *obj, Elm_Thumb_Data *pd);


Eina_Error _elm_thumb_efl_file_file_set(Eo *obj, Elm_Thumb_Data *pd, const char *file);


const char *_elm_thumb_efl_file_file_get(const Eo *obj, Elm_Thumb_Data *pd);


void _elm_thumb_efl_file_key_set(Eo *obj, Elm_Thumb_Data *pd, const char *key);


const char *_elm_thumb_efl_file_key_get(const Eo *obj, Elm_Thumb_Data *pd);


void _elm_thumb_efl_gfx_entity_visible_set(Eo *obj, Elm_Thumb_Data *pd, Eina_Bool v);


void _elm_thumb_efl_ui_draggable_drag_target_set(Eo *obj, Elm_Thumb_Data *pd, Eina_Bool set);


Eina_Bool _elm_thumb_efl_ui_draggable_drag_target_get(const Eo *obj, Elm_Thumb_Data *pd);


static Eina_Bool
_elm_thumb_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_THUMB_EXTRA_OPS
#define ELM_THUMB_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_thumb_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_finalize, _elm_thumb_efl_object_finalize),
      EFL_OBJECT_OP_FUNC(efl_file_load, _elm_thumb_efl_file_load),
      EFL_OBJECT_OP_FUNC(efl_file_loaded_get, _elm_thumb_efl_file_loaded_get),
      EFL_OBJECT_OP_FUNC(efl_file_set, _elm_thumb_efl_file_file_set),
      EFL_OBJECT_OP_FUNC(efl_file_get, _elm_thumb_efl_file_file_get),
      EFL_OBJECT_OP_FUNC(efl_file_key_set, _elm_thumb_efl_file_key_set),
      EFL_OBJECT_OP_FUNC(efl_file_key_get, _elm_thumb_efl_file_key_get),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_visible_set, _elm_thumb_efl_gfx_entity_visible_set),
      EFL_OBJECT_OP_FUNC(efl_ui_draggable_drag_target_set, _elm_thumb_efl_ui_draggable_drag_target_set),
      EFL_OBJECT_OP_FUNC(efl_ui_draggable_drag_target_get, _elm_thumb_efl_ui_draggable_drag_target_get),
      ELM_THUMB_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_thumb_class_desc = {
   EO_VERSION,
   "Elm.Thumb",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Thumb_Data),
   _elm_thumb_class_initializer,
   _elm_thumb_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_thumb_class_get, &_elm_thumb_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_FILE_MIXIN, EFL_UI_CLICKABLE_INTERFACE, EFL_UI_DRAGGABLE_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);
