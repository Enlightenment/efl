
Efl_Object *_elm_slider_efl_object_constructor(Eo *obj, Elm_Slider_Data *pd);


void _elm_slider_efl_object_destructor(Eo *obj, Elm_Slider_Data *pd);


void _elm_slider_efl_canvas_group_group_calculate(Eo *obj, Elm_Slider_Data *pd);


Eina_Error _elm_slider_efl_ui_widget_theme_apply(Eo *obj, Elm_Slider_Data *pd);


Eina_Bool _elm_slider_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Slider_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


Eina_Bool _elm_slider_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Slider_Data *pd);


void _elm_slider_efl_text_text_set(Eo *obj, Elm_Slider_Data *pd, const char *text);


const char *_elm_slider_efl_text_text_get(const Eo *obj, Elm_Slider_Data *pd);


void _elm_slider_efl_text_markup_markup_set(Eo *obj, Elm_Slider_Data *pd, const char *markup);


const char *_elm_slider_efl_text_markup_markup_get(const Eo *obj, Elm_Slider_Data *pd);


void _elm_slider_efl_ui_format_format_cb_set(Eo *obj, Elm_Slider_Data *pd, void *func_data, Efl_Ui_Format_Func_Cb func, Eina_Free_Cb func_free_cb);


void _elm_slider_efl_ui_l10n_l10n_text_set(Eo *obj, Elm_Slider_Data *pd, const char *label, const char *domain);


const char *_elm_slider_efl_ui_l10n_l10n_text_get(const Eo *obj, Elm_Slider_Data *pd, const char **domain);


Efl_Object *_elm_slider_efl_part_part_get(const Eo *obj, Elm_Slider_Data *pd, const char *name);


static Eina_Bool
_elm_slider_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SLIDER_EXTRA_OPS
#define ELM_SLIDER_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_slider_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_slider_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_calculate, _elm_slider_efl_canvas_group_group_calculate),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_slider_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_slider_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_slider_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_text_set, _elm_slider_efl_text_text_set),
      EFL_OBJECT_OP_FUNC(efl_text_get, _elm_slider_efl_text_text_get),
      EFL_OBJECT_OP_FUNC(efl_text_markup_set, _elm_slider_efl_text_markup_markup_set),
      EFL_OBJECT_OP_FUNC(efl_text_markup_get, _elm_slider_efl_text_markup_markup_get),
      EFL_OBJECT_OP_FUNC(efl_ui_format_cb_set, _elm_slider_efl_ui_format_format_cb_set),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_text_set, _elm_slider_efl_ui_l10n_l10n_text_set),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_text_get, _elm_slider_efl_ui_l10n_l10n_text_get),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_slider_efl_part_part_get),
      ELM_SLIDER_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_slider_class_desc = {
   EO_VERSION,
   "Elm.Slider",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Slider_Data),
   _elm_slider_class_initializer,
   _elm_slider_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_slider_class_get, &_elm_slider_class_desc, EFL_UI_SLIDER_INTERVAL_CLASS, EFL_UI_LEGACY_INTERFACE, EFL_TEXT_INTERFACE, EFL_TEXT_MARKUP_INTERFACE, EFL_UI_FORMAT_MIXIN, NULL);
