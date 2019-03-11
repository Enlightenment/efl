EWAPI const Efl_Event_Description _ELM_VIEW_LIST_EVENT_MODEL_SELECTED =
   EFL_EVENT_DESCRIPTION("model,selected");

void _elm_view_list_genlist_set(Eo *obj, Elm_View_List_Data *pd, Efl_Canvas_Object *genlist, Elm_Genlist_Item_Type item_type, const char *item_style);

EOAPI EFL_VOID_FUNC_BODYV(elm_view_list_genlist_set, EFL_FUNC_CALL(genlist, item_type, item_style), Efl_Canvas_Object *genlist, Elm_Genlist_Item_Type item_type, const char *item_style);

void _elm_view_list_evas_object_get(Eo *obj, Elm_View_List_Data *pd, Efl_Canvas_Object **widget);

EOAPI EFL_VOID_FUNC_BODYV(elm_view_list_evas_object_get, EFL_FUNC_CALL(widget), Efl_Canvas_Object **widget);

void _elm_view_list_property_connect(Eo *obj, Elm_View_List_Data *pd, const char *property, const char *part);

EOAPI EFL_VOID_FUNC_BODYV(elm_view_list_property_connect, EFL_FUNC_CALL(property, part), const char *property, const char *part);

void _elm_view_list_model_set(Eo *obj, Elm_View_List_Data *pd, Efl_Model *model);

EOAPI EFL_VOID_FUNC_BODYV(elm_view_list_model_set, EFL_FUNC_CALL(model), Efl_Model *model);

Efl_Model *_elm_view_list_model_get(const Eo *obj, Elm_View_List_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_view_list_model_get, Efl_Model *, NULL);

void _elm_view_list_efl_object_destructor(Eo *obj, Elm_View_List_Data *pd);


static Eina_Bool
_elm_view_list_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_VIEW_LIST_EXTRA_OPS
#define ELM_VIEW_LIST_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_view_list_genlist_set, _elm_view_list_genlist_set),
      EFL_OBJECT_OP_FUNC(elm_view_list_evas_object_get, _elm_view_list_evas_object_get),
      EFL_OBJECT_OP_FUNC(elm_view_list_property_connect, _elm_view_list_property_connect),
      EFL_OBJECT_OP_FUNC(elm_view_list_model_set, _elm_view_list_model_set),
      EFL_OBJECT_OP_FUNC(elm_view_list_model_get, _elm_view_list_model_get),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_view_list_efl_object_destructor),
      ELM_VIEW_LIST_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_view_list_class_desc = {
   EO_VERSION,
   "Elm.View.List",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_View_List_Data),
   _elm_view_list_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_view_list_class_get, &_elm_view_list_class_desc, EFL_OBJECT_CLASS, NULL);
