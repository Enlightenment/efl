
static Eina_Error
__eolian_elm_interface_fileselector_folder_only_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_interface_fileselector_folder_only_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_interface_fileselector_folder_only_set, EFL_FUNC_CALL(only), Eina_Bool only);

static Eina_Value
__eolian_elm_interface_fileselector_folder_only_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_interface_fileselector_folder_only_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_folder_only_get, Eina_Bool, 0);
EOAPI EFL_VOID_FUNC_BODYV(elm_interface_fileselector_thumbnail_size_set, EFL_FUNC_CALL(w, h), int w, int h);
EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_interface_fileselector_thumbnail_size_get, EFL_FUNC_CALL(w, h), int *w, int *h);

static Eina_Error
__eolian_elm_interface_fileselector_hidden_visible_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_interface_fileselector_hidden_visible_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_interface_fileselector_hidden_visible_set, EFL_FUNC_CALL(hidden), Eina_Bool hidden);

static Eina_Value
__eolian_elm_interface_fileselector_hidden_visible_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_interface_fileselector_hidden_visible_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_hidden_visible_get, Eina_Bool, 0);
EOAPI EFL_VOID_FUNC_BODYV(elm_interface_fileselector_sort_method_set, EFL_FUNC_CALL(sort), Elm_Fileselector_Sort sort);
EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_sort_method_get, Elm_Fileselector_Sort, 0);

static Eina_Error
__eolian_elm_interface_fileselector_multi_select_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_interface_fileselector_multi_select_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_interface_fileselector_multi_select_set, EFL_FUNC_CALL(multi), Eina_Bool multi);

static Eina_Value
__eolian_elm_interface_fileselector_multi_select_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_interface_fileselector_multi_select_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_multi_select_get, Eina_Bool, 0);

static Eina_Error
__eolian_elm_interface_fileselector_expandable_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_interface_fileselector_expandable_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_interface_fileselector_expandable_set, EFL_FUNC_CALL(expand), Eina_Bool expand);

static Eina_Value
__eolian_elm_interface_fileselector_expandable_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_interface_fileselector_expandable_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_expandable_get, Eina_Bool, 0);
EOAPI EFL_VOID_FUNC_BODYV(elm_interface_fileselector_mode_set, EFL_FUNC_CALL(mode), Elm_Fileselector_Mode mode);
EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_mode_get, Elm_Fileselector_Mode, 0);

static Eina_Error
__eolian_elm_interface_fileselector_is_save_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_interface_fileselector_is_save_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_interface_fileselector_is_save_set, EFL_FUNC_CALL(is_save), Eina_Bool is_save);

static Eina_Value
__eolian_elm_interface_fileselector_is_save_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_interface_fileselector_is_save_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_is_save_get, Eina_Bool, 0);
EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_selected_models_get, const Eina_List *, NULL);

static Eina_Error
__eolian_elm_interface_fileselector_current_name_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_interface_fileselector_current_name_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_interface_fileselector_current_name_set, EFL_FUNC_CALL(name), const char *name);

static Eina_Value
__eolian_elm_interface_fileselector_current_name_get_reflect(Eo *obj)
{
   const char *val = elm_interface_fileselector_current_name_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_current_name_get, const char *, NULL);
EOAPI EFL_FUNC_BODYV(elm_interface_fileselector_selected_model_set, Eina_Bool, 0, EFL_FUNC_CALL(model), Efl_Io_Model *model);
EOAPI EFL_FUNC_BODY_CONST(elm_interface_fileselector_selected_model_get, Efl_Io_Model *, NULL);
EOAPI EFL_FUNC_BODYV(elm_interface_fileselector_custom_filter_append, Eina_Bool, 0, EFL_FUNC_CALL(func, data, filter_name), Elm_Fileselector_Filter_Func func, void *data, const char *filter_name);
EOAPI EFL_VOID_FUNC_BODY(elm_interface_fileselector_filters_clear);
EOAPI EFL_FUNC_BODYV(elm_interface_fileselector_mime_types_filter_append, Eina_Bool, 0, EFL_FUNC_CALL(mime_types, filter_name), const char *mime_types, const char *filter_name);

static Eina_Bool
_elm_interface_fileselector_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_INTERFACE_FILESELECTOR_EXTRA_OPS
#define ELM_INTERFACE_FILESELECTOR_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_folder_only_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_folder_only_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_thumbnail_size_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_thumbnail_size_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_hidden_visible_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_hidden_visible_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_sort_method_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_sort_method_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_multi_select_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_multi_select_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_expandable_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_expandable_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_mode_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_mode_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_is_save_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_is_save_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_selected_models_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_current_name_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_current_name_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_selected_model_set, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_selected_model_get, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_custom_filter_append, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_filters_clear, NULL),
      EFL_OBJECT_OP_FUNC(elm_interface_fileselector_mime_types_filter_append, NULL),
      ELM_INTERFACE_FILESELECTOR_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"folder_only", __eolian_elm_interface_fileselector_folder_only_set_reflect, __eolian_elm_interface_fileselector_folder_only_get_reflect},
      {"hidden_visible", __eolian_elm_interface_fileselector_hidden_visible_set_reflect, __eolian_elm_interface_fileselector_hidden_visible_get_reflect},
      {"multi_select", __eolian_elm_interface_fileselector_multi_select_set_reflect, __eolian_elm_interface_fileselector_multi_select_get_reflect},
      {"expandable", __eolian_elm_interface_fileselector_expandable_set_reflect, __eolian_elm_interface_fileselector_expandable_get_reflect},
      {"is_save", __eolian_elm_interface_fileselector_is_save_set_reflect, __eolian_elm_interface_fileselector_is_save_get_reflect},
      {"current_name", __eolian_elm_interface_fileselector_current_name_set_reflect, __eolian_elm_interface_fileselector_current_name_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_interface_fileselector_class_desc = {
   EO_VERSION,
   "Elm.Interface.Fileselector",
   EFL_CLASS_TYPE_INTERFACE,
   0,
   _elm_interface_fileselector_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_interface_fileselector_interface_get, &_elm_interface_fileselector_class_desc, NULL, EFL_UI_VIEW_INTERFACE, NULL);
