EWAPI const Efl_Event_Description _ELM_ICON_EVENT_THUMB_DONE =
   EFL_EVENT_DESCRIPTION("thumb,done");
EWAPI const Efl_Event_Description _ELM_ICON_EVENT_THUMB_ERROR =
   EFL_EVENT_DESCRIPTION("thumb,error");

Efl_Object *_elm_icon_efl_object_constructor(Eo *obj, Elm_Icon_Data *pd);


Eina_Error _elm_icon_efl_ui_widget_theme_apply(Eo *obj, Elm_Icon_Data *pd);


Eina_Error _elm_icon_efl_file_load(Eo *obj, Elm_Icon_Data *pd);


static Eina_Bool
_elm_icon_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_ICON_EXTRA_OPS
#define ELM_ICON_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_icon_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_icon_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_file_load, _elm_icon_efl_file_load),
      ELM_ICON_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_icon_class_desc = {
   EO_VERSION,
   "Elm.Icon",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Icon_Data),
   _elm_icon_class_initializer,
   _elm_icon_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_icon_class_get, &_elm_icon_class_desc, EFL_UI_IMAGE_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);
