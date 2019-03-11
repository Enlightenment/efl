
Efl_Object *_elm_access_efl_object_constructor(Eo *obj, void *pd);


Eina_Bool _elm_access_efl_ui_widget_on_access_activate(Eo *obj, void *pd, Efl_Ui_Activate act);


Eina_Bool _elm_access_efl_ui_focus_object_on_focus_update(Eo *obj, void *pd);


const Efl_Access_Action_Data *_elm_access_efl_access_widget_action_elm_actions_get(const Eo *obj, void *pd);


Efl_Access_State_Set _elm_access_efl_access_object_state_set_get(const Eo *obj, void *pd);


static Eina_Bool
_elm_access_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_ACCESS_EXTRA_OPS
#define ELM_ACCESS_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_access_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_activate, _elm_access_efl_ui_widget_on_access_activate),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_access_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_access_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_access_efl_access_object_state_set_get),
      ELM_ACCESS_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_access_class_desc = {
   EO_VERSION,
   "Elm.Access",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _elm_access_class_initializer,
   _elm_access_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_access_class_get, &_elm_access_class_desc, EFL_UI_WIDGET_CLASS, EFL_ACCESS_WIDGET_ACTION_MIXIN, NULL);
