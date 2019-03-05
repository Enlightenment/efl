EWAPI const Efl_Event_Description _ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_ON =
   EFL_EVENT_DESCRIPTION("virtualkeypad,state,on");
EWAPI const Efl_Event_Description _ELM_CONFORMANT_EVENT_VIRTUALKEYPAD_STATE_OFF =
   EFL_EVENT_DESCRIPTION("virtualkeypad,state,off");
EWAPI const Efl_Event_Description _ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_ON =
   EFL_EVENT_DESCRIPTION("clipboard,state,on");
EWAPI const Efl_Event_Description _ELM_CONFORMANT_EVENT_CLIPBOARD_STATE_OFF =
   EFL_EVENT_DESCRIPTION("clipboard,state,off");

Efl_Object *_elm_conformant_efl_object_constructor(Eo *obj, Elm_Conformant_Data *pd);


void _elm_conformant_efl_ui_widget_widget_parent_set(Eo *obj, Elm_Conformant_Data *pd, Efl_Ui_Widget *parent);


Eina_Error _elm_conformant_efl_ui_widget_theme_apply(Eo *obj, Elm_Conformant_Data *pd);


static Eina_Bool
_elm_conformant_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_CONFORMANT_EXTRA_OPS
#define ELM_CONFORMANT_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_conformant_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_parent_set, _elm_conformant_efl_ui_widget_widget_parent_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_conformant_efl_ui_widget_theme_apply),
      ELM_CONFORMANT_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_conformant_class_desc = {
   EO_VERSION,
   "Elm.Conformant",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Conformant_Data),
   _elm_conformant_class_initializer,
   _elm_conformant_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_conformant_class_get, &_elm_conformant_class_desc, EFL_UI_LAYOUT_BASE_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);
