
void _elm_slider_part_indicator_visible_mode_set(Eo *obj, void *pd, Efl_Ui_Slider_Indicator_Visible_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_slider_part_indicator_visible_mode_set, EFL_FUNC_CALL(mode), Efl_Ui_Slider_Indicator_Visible_Mode mode);

Efl_Ui_Slider_Indicator_Visible_Mode _elm_slider_part_indicator_visible_mode_get(const Eo *obj, void *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_slider_part_indicator_visible_mode_get, Efl_Ui_Slider_Indicator_Visible_Mode, 0);

void _elm_slider_part_indicator_efl_ui_format_format_cb_set(Eo *obj, void *pd, void *func_data, Efl_Ui_Format_Func_Cb func, Eina_Free_Cb func_free_cb);


void _elm_slider_part_indicator_efl_ui_format_format_string_set(Eo *obj, void *pd, const char *units);


const char *_elm_slider_part_indicator_efl_ui_format_format_string_get(const Eo *obj, void *pd);


static Eina_Bool
_elm_slider_part_indicator_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SLIDER_PART_INDICATOR_EXTRA_OPS
#define ELM_SLIDER_PART_INDICATOR_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_slider_part_indicator_visible_mode_set, _elm_slider_part_indicator_visible_mode_set),
      EFL_OBJECT_OP_FUNC(elm_slider_part_indicator_visible_mode_get, _elm_slider_part_indicator_visible_mode_get),
      EFL_OBJECT_OP_FUNC(efl_ui_format_cb_set, _elm_slider_part_indicator_efl_ui_format_format_cb_set),
      EFL_OBJECT_OP_FUNC(efl_ui_format_string_set, _elm_slider_part_indicator_efl_ui_format_format_string_set),
      EFL_OBJECT_OP_FUNC(efl_ui_format_string_get, _elm_slider_part_indicator_efl_ui_format_format_string_get),
      ELM_SLIDER_PART_INDICATOR_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_slider_part_indicator_class_desc = {
   EO_VERSION,
   "Elm.Slider_Part_Indicator",
   EFL_CLASS_TYPE_REGULAR,
   0,
   _elm_slider_part_indicator_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_slider_part_indicator_class_get, &_elm_slider_part_indicator_class_desc, EFL_UI_LAYOUT_PART_CLASS, EFL_UI_FORMAT_MIXIN, NULL);
