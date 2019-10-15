
Efl_Object *_efl_ui_layout_legacy_efl_object_constructor(Eo *obj, void *pd);

static Eina_Bool
_efl_ui_layout_legacy_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EFL_UI_LAYOUT_LEGACY_EXTRA_OPS
#define EFL_UI_LAYOUT_LEGACY_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _efl_ui_layout_legacy_efl_object_constructor),
      EFL_UI_LAYOUT_LEGACY_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _efl_ui_layout_legacy_class_desc = {
   EO_VERSION,
   "Efl.Ui.Layout_Legacy",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _efl_ui_layout_legacy_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(efl_ui_layout_legacy_class_get, &_efl_ui_layout_legacy_class_desc, EFL_UI_LAYOUT_CLASS, ELM_LAYOUT_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

static void _elm_layout_sizing_eval(Eo *obj, Elm_Layout_Data *ld);
EAPI EFL_VOID_FUNC_BODY(elm_layout_sizing_eval);

static Eina_Bool
_elm_layout_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_LAYOUT_EXTRA_OPS
#define ELM_LAYOUT_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_layout_sizing_eval, _elm_layout_sizing_eval),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_change, _elm_layout_efl_canvas_group_change),
      EFL_OBJECT_OP_FUNC(efl_gfx_hint_size_restricted_min_set, _elm_layout_efl_gfx_hint_size_restricted_min_set),
      ELM_LAYOUT_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_layout_class_desc = {
   EO_VERSION,
   "Elm.Layout",
   EFL_CLASS_TYPE_MIXIN,
   sizeof(Elm_Layout_Data),
   _elm_layout_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_layout_mixin_get, &_elm_layout_class_desc, NULL, NULL);
