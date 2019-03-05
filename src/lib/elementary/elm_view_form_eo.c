
void _elm_view_form_model_set(Eo *obj, Elm_View_Form_Data *pd, Efl_Model *model);

EOAPI EFL_VOID_FUNC_BODYV(elm_view_form_model_set, EFL_FUNC_CALL(model), Efl_Model *model);

void _elm_view_form_widget_add(Eo *obj, Elm_View_Form_Data *pd, const char *propname, Efl_Canvas_Object *evas);

EOAPI EFL_VOID_FUNC_BODYV(elm_view_form_widget_add, EFL_FUNC_CALL(propname, evas), const char *propname, Efl_Canvas_Object *evas);

void _elm_view_form_efl_object_destructor(Eo *obj, Elm_View_Form_Data *pd);


static Eina_Bool
_elm_view_form_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_VIEW_FORM_EXTRA_OPS
#define ELM_VIEW_FORM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_view_form_model_set, _elm_view_form_model_set),
      EFL_OBJECT_OP_FUNC(elm_view_form_widget_add, _elm_view_form_widget_add),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_view_form_efl_object_destructor),
      ELM_VIEW_FORM_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_view_form_class_desc = {
   EO_VERSION,
   "Elm.View.Form",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_View_Form_Data),
   _elm_view_form_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_view_form_class_get, &_elm_view_form_class_desc, EFL_OBJECT_CLASS, NULL);
