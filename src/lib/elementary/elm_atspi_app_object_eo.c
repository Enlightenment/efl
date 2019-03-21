
void _elm_atspi_app_object_efl_object_destructor(Eo *obj, Elm_Atspi_App_Object_Data *pd);


const char *_elm_atspi_app_object_efl_access_object_i18n_name_get(const Eo *obj, Elm_Atspi_App_Object_Data *pd);


void _elm_atspi_app_object_efl_access_object_description_set(Eo *obj, Elm_Atspi_App_Object_Data *pd, const char *description);


const char *_elm_atspi_app_object_efl_access_object_description_get(const Eo *obj, Elm_Atspi_App_Object_Data *pd);


Efl_Access_Role _elm_atspi_app_object_efl_access_object_role_get(const Eo *obj, Elm_Atspi_App_Object_Data *pd);


Eina_List *_elm_atspi_app_object_efl_access_object_access_children_get(const Eo *obj, Elm_Atspi_App_Object_Data *pd);


static Eina_Bool
_elm_atspi_app_object_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_ATSPI_APP_OBJECT_EXTRA_OPS
#define ELM_ATSPI_APP_OBJECT_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_atspi_app_object_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_atspi_app_object_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_description_set, _elm_atspi_app_object_efl_access_object_description_set),
      EFL_OBJECT_OP_FUNC(efl_access_object_description_get, _elm_atspi_app_object_efl_access_object_description_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_role_get, _elm_atspi_app_object_efl_access_object_role_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_atspi_app_object_efl_access_object_access_children_get),
      ELM_ATSPI_APP_OBJECT_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_atspi_app_object_class_desc = {
   EO_VERSION,
   "Elm.Atspi.App.Object",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Atspi_App_Object_Data),
   _elm_atspi_app_object_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_atspi_app_object_class_get, &_elm_atspi_app_object_class_desc, EFL_OBJECT_CLASS, EFL_ACCESS_OBJECT_MIXIN, NULL);
