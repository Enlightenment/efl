EWAPI const Efl_Event_Description _ELM_FILESELECTOR_BUTTON_EVENT_FILE_CHOSEN =
   EFL_EVENT_DESCRIPTION("file,chosen");

Efl_Object *_elm_fileselector_button_efl_object_constructor(Eo *obj, Elm_Fileselector_Button_Data *pd);


Eina_Error _elm_fileselector_button_efl_ui_widget_theme_apply(Eo *obj, Elm_Fileselector_Button_Data *pd);


Eina_Bool _elm_fileselector_button_efl_ui_autorepeat_autorepeat_supported_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


const Eina_List *_elm_fileselector_button_elm_interface_fileselector_selected_models_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_elm_interface_fileselector_expandable_set(Eo *obj, Elm_Fileselector_Button_Data *pd, Eina_Bool expand);


Eina_Bool _elm_fileselector_button_elm_interface_fileselector_expandable_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_elm_interface_fileselector_thumbnail_size_set(Eo *obj, Elm_Fileselector_Button_Data *pd, int w, int h);


void _elm_fileselector_button_elm_interface_fileselector_thumbnail_size_get(const Eo *obj, Elm_Fileselector_Button_Data *pd, int *w, int *h);


Eina_Bool _elm_fileselector_button_elm_interface_fileselector_selected_model_set(Eo *obj, Elm_Fileselector_Button_Data *pd, Efl_Io_Model *model);


Efl_Io_Model *_elm_fileselector_button_elm_interface_fileselector_selected_model_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_elm_interface_fileselector_hidden_visible_set(Eo *obj, Elm_Fileselector_Button_Data *pd, Eina_Bool hidden);


Eina_Bool _elm_fileselector_button_elm_interface_fileselector_hidden_visible_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_elm_interface_fileselector_is_save_set(Eo *obj, Elm_Fileselector_Button_Data *pd, Eina_Bool is_save);


Eina_Bool _elm_fileselector_button_elm_interface_fileselector_is_save_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_efl_ui_view_model_set(Eo *obj, Elm_Fileselector_Button_Data *pd, Efl_Model *model);


Efl_Model *_elm_fileselector_button_efl_ui_view_model_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_elm_interface_fileselector_sort_method_set(Eo *obj, Elm_Fileselector_Button_Data *pd, Elm_Fileselector_Sort sort);


Elm_Fileselector_Sort _elm_fileselector_button_elm_interface_fileselector_sort_method_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_elm_interface_fileselector_multi_select_set(Eo *obj, Elm_Fileselector_Button_Data *pd, Eina_Bool multi);


Eina_Bool _elm_fileselector_button_elm_interface_fileselector_multi_select_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_elm_interface_fileselector_folder_only_set(Eo *obj, Elm_Fileselector_Button_Data *pd, Eina_Bool only);


Eina_Bool _elm_fileselector_button_elm_interface_fileselector_folder_only_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_elm_interface_fileselector_mode_set(Eo *obj, Elm_Fileselector_Button_Data *pd, Elm_Fileselector_Mode mode);


Elm_Fileselector_Mode _elm_fileselector_button_elm_interface_fileselector_mode_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


void _elm_fileselector_button_elm_interface_fileselector_current_name_set(Eo *obj, Elm_Fileselector_Button_Data *pd, const char *name);


const char *_elm_fileselector_button_elm_interface_fileselector_current_name_get(const Eo *obj, Elm_Fileselector_Button_Data *pd);


Eina_Bool _elm_fileselector_button_elm_interface_fileselector_custom_filter_append(Eo *obj, Elm_Fileselector_Button_Data *pd, Elm_Fileselector_Filter_Func func, void *data, const char *filter_name);


Eina_Bool _elm_fileselector_button_elm_interface_fileselector_mime_types_filter_append(Eo *obj, Elm_Fileselector_Button_Data *pd, const char *mime_types, const char *filter_name);


void _elm_fileselector_button_elm_interface_fileselector_filters_clear(Eo *obj, Elm_Fileselector_Button_Data *pd);


static Eina_Bool
_elm_fileselector_button_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_FILESELECTOR_BUTTON_EXTRA_OPS
#define ELM_FILESELECTOR_BUTTON_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_fileselector_button_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_fileselector_button_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_autorepeat_supported_get, _elm_fileselector_button_efl_ui_autorepeat_autorepeat_supported_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_selected_models_get, _elm_fileselector_button_elm_interface_fileselector_selected_models_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_expandable_set, _elm_fileselector_button_elm_interface_fileselector_expandable_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_expandable_get, _elm_fileselector_button_elm_interface_fileselector_expandable_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_thumbnail_size_set, _elm_fileselector_button_elm_interface_fileselector_thumbnail_size_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_thumbnail_size_get, _elm_fileselector_button_elm_interface_fileselector_thumbnail_size_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_selected_model_set, _elm_fileselector_button_elm_interface_fileselector_selected_model_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_selected_model_get, _elm_fileselector_button_elm_interface_fileselector_selected_model_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_hidden_visible_set, _elm_fileselector_button_elm_interface_fileselector_hidden_visible_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_hidden_visible_get, _elm_fileselector_button_elm_interface_fileselector_hidden_visible_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_is_save_set, _elm_fileselector_button_elm_interface_fileselector_is_save_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_is_save_get, _elm_fileselector_button_elm_interface_fileselector_is_save_get),
      EFL_OBJECT_OP_FUNC(efl_ui_view_model_set, _elm_fileselector_button_efl_ui_view_model_set),
      EFL_OBJECT_OP_FUNC(efl_ui_view_model_get, _elm_fileselector_button_efl_ui_view_model_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_sort_method_set, _elm_fileselector_button_elm_interface_fileselector_sort_method_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_sort_method_get, _elm_fileselector_button_elm_interface_fileselector_sort_method_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_multi_select_set, _elm_fileselector_button_elm_interface_fileselector_multi_select_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_multi_select_get, _elm_fileselector_button_elm_interface_fileselector_multi_select_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_folder_only_set, _elm_fileselector_button_elm_interface_fileselector_folder_only_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_folder_only_get, _elm_fileselector_button_elm_interface_fileselector_folder_only_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_mode_set, _elm_fileselector_button_elm_interface_fileselector_mode_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_mode_get, _elm_fileselector_button_elm_interface_fileselector_mode_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_current_name_set, _elm_fileselector_button_elm_interface_fileselector_current_name_set),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_current_name_get, _elm_fileselector_button_elm_interface_fileselector_current_name_get),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_custom_filter_append, _elm_fileselector_button_elm_interface_fileselector_custom_filter_append),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_mime_types_filter_append, _elm_fileselector_button_elm_interface_fileselector_mime_types_filter_append),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_filters_clear, _elm_fileselector_button_elm_interface_fileselector_filters_clear),
      ELM_FILESELECTOR_BUTTON_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_fileselector_button_class_desc = {
   EO_VERSION,
   "Elm.Fileselector_Button",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Fileselector_Button_Data),
   _elm_fileselector_button_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_fileselector_button_class_get, &_elm_fileselector_button_class_desc, EFL_UI_BUTTON_CLASS, ELM_INTERFACE_FILESELECTOR_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);
