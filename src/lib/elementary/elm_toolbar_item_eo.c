
Elm_Widget_Item *_elm_toolbar_item_prev_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_prev_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_toolbar_item_next_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_next_get, Elm_Widget_Item *, NULL);

void _elm_toolbar_item_selected_set(Eo *obj, Elm_Toolbar_Item_Data *pd, Eina_Bool selected);


static Eina_Error
__eolian_elm_toolbar_item_selected_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_toolbar_item_selected_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_item_selected_set, EFL_FUNC_CALL(selected), Eina_Bool selected);

Eina_Bool _elm_toolbar_item_selected_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);


static Eina_Value
__eolian_elm_toolbar_item_selected_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_toolbar_item_selected_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_selected_get, Eina_Bool, 0);

void _elm_toolbar_item_priority_set(Eo *obj, Elm_Toolbar_Item_Data *pd, int priority);


static Eina_Error
__eolian_elm_toolbar_item_priority_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_toolbar_item_priority_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_item_priority_set, EFL_FUNC_CALL(priority), int priority);

int _elm_toolbar_item_priority_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);


static Eina_Value
__eolian_elm_toolbar_item_priority_get_reflect(const Eo *obj)
{
   int val = elm_obj_toolbar_item_priority_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_priority_get, int, 0);

void _elm_toolbar_item_icon_set(Eo *obj, Elm_Toolbar_Item_Data *pd, const char *icon);


static Eina_Error
__eolian_elm_toolbar_item_icon_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_toolbar_item_icon_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_item_icon_set, EFL_FUNC_CALL(icon), const char *icon);

const char *_elm_toolbar_item_icon_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);


static Eina_Value
__eolian_elm_toolbar_item_icon_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_toolbar_item_icon_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_icon_get, const char *, NULL);

Efl_Canvas_Object *_elm_toolbar_item_object_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_object_get, Efl_Canvas_Object *, NULL);

Efl_Canvas_Object *_elm_toolbar_item_icon_object_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_icon_object_get, Efl_Canvas_Object *, NULL);

void _elm_toolbar_item_separator_set(Eo *obj, Elm_Toolbar_Item_Data *pd, Eina_Bool separator);


static Eina_Error
__eolian_elm_toolbar_item_separator_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_toolbar_item_separator_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_item_separator_set, EFL_FUNC_CALL(separator), Eina_Bool separator);

Eina_Bool _elm_toolbar_item_separator_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);


static Eina_Value
__eolian_elm_toolbar_item_separator_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_toolbar_item_separator_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_separator_get, Eina_Bool, 0);

Efl_Canvas_Object *_elm_toolbar_item_menu_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_menu_get, Efl_Canvas_Object *, NULL);

Eina_Bool _elm_toolbar_item_state_set(Eo *obj, Elm_Toolbar_Item_Data *pd, Elm_Toolbar_Item_State *state);

EOAPI EFL_FUNC_BODYV(elm_obj_toolbar_item_state_set, Eina_Bool, 0, EFL_FUNC_CALL(state), Elm_Toolbar_Item_State *state);

Elm_Toolbar_Item_State *_elm_toolbar_item_state_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_toolbar_item_state_get, Elm_Toolbar_Item_State *, NULL);

Eina_Bool _elm_toolbar_item_icon_memfile_set(Eo *obj, Elm_Toolbar_Item_Data *pd, const void *img, size_t size, const char *format, const char *key);

EOAPI EFL_FUNC_BODYV(elm_obj_toolbar_item_icon_memfile_set, Eina_Bool, 0, EFL_FUNC_CALL(img, size, format, key), const void *img, size_t size, const char *format, const char *key);

Eina_Bool _elm_toolbar_item_icon_file_set(Eo *obj, Elm_Toolbar_Item_Data *pd, const char *file, const char *key);

EOAPI EFL_FUNC_BODYV(elm_obj_toolbar_item_icon_file_set, Eina_Bool, 0, EFL_FUNC_CALL(file, key), const char *file, const char *key);

Elm_Toolbar_Item_State *_elm_toolbar_item_state_add(Eo *obj, Elm_Toolbar_Item_Data *pd, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_toolbar_item_state_add, Elm_Toolbar_Item_State *, NULL, EFL_FUNC_CALL(icon, label, func, data), const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

Eina_Bool _elm_toolbar_item_state_del(Eo *obj, Elm_Toolbar_Item_Data *pd, Elm_Toolbar_Item_State *state);

EOAPI EFL_FUNC_BODYV(elm_obj_toolbar_item_state_del, Eina_Bool, 0, EFL_FUNC_CALL(state), Elm_Toolbar_Item_State *state);

Elm_Toolbar_Item_State *_elm_toolbar_item_state_next(Eo *obj, Elm_Toolbar_Item_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_toolbar_item_state_next, Elm_Toolbar_Item_State *, NULL);

Elm_Toolbar_Item_State *_elm_toolbar_item_state_prev(Eo *obj, Elm_Toolbar_Item_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_toolbar_item_state_prev, Elm_Toolbar_Item_State *, NULL);

void _elm_toolbar_item_show(Eo *obj, Elm_Toolbar_Item_Data *pd, Elm_Toolbar_Item_Scrollto_Type scrollto);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_item_show, EFL_FUNC_CALL(scrollto), Elm_Toolbar_Item_Scrollto_Type scrollto);

void _elm_toolbar_item_bring_in(Eo *obj, Elm_Toolbar_Item_Data *pd, Elm_Toolbar_Item_Scrollto_Type scrollto);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_item_bring_in, EFL_FUNC_CALL(scrollto), Elm_Toolbar_Item_Scrollto_Type scrollto);

void _elm_toolbar_item_menu_set(Eo *obj, Elm_Toolbar_Item_Data *pd, Eina_Bool menu);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_toolbar_item_menu_set, EFL_FUNC_CALL(menu), Eina_Bool menu);

Efl_Object *_elm_toolbar_item_efl_object_constructor(Eo *obj, Elm_Toolbar_Item_Data *pd);


void _elm_toolbar_item_efl_object_invalidate(Eo *obj, Elm_Toolbar_Item_Data *pd);


void _elm_toolbar_item_elm_widget_item_disable(Eo *obj, Elm_Toolbar_Item_Data *pd);


void _elm_toolbar_item_elm_widget_item_disabled_set(Eo *obj, Elm_Toolbar_Item_Data *pd, Eina_Bool disable);


void _elm_toolbar_item_elm_widget_item_item_focus_set(Eo *obj, Elm_Toolbar_Item_Data *pd, Eina_Bool focused);


Eina_Bool _elm_toolbar_item_elm_widget_item_item_focus_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);


void _elm_toolbar_item_elm_widget_item_signal_emit(Eo *obj, Elm_Toolbar_Item_Data *pd, const char *emission, const char *source);


void _elm_toolbar_item_elm_widget_item_part_text_set(Eo *obj, Elm_Toolbar_Item_Data *pd, const char *part, const char *label);


const char *_elm_toolbar_item_elm_widget_item_part_text_get(const Eo *obj, Elm_Toolbar_Item_Data *pd, const char *part);


void _elm_toolbar_item_elm_widget_item_part_content_set(Eo *obj, Elm_Toolbar_Item_Data *pd, const char *part, Efl_Canvas_Object *content);


Efl_Canvas_Object *_elm_toolbar_item_elm_widget_item_part_content_get(const Eo *obj, Elm_Toolbar_Item_Data *pd, const char *part);


Efl_Canvas_Object *_elm_toolbar_item_elm_widget_item_part_content_unset(Eo *obj, Elm_Toolbar_Item_Data *pd, const char *part);


Eina_Rect _elm_toolbar_item_efl_ui_focus_object_focus_geometry_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);


void _elm_toolbar_item_efl_ui_focus_object_focus_set(Eo *obj, Elm_Toolbar_Item_Data *pd, Eina_Bool focus);


Efl_Ui_Focus_Object *_elm_toolbar_item_efl_ui_focus_object_focus_parent_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);


const char *_elm_toolbar_item_efl_access_object_i18n_name_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);


Efl_Access_State_Set _elm_toolbar_item_efl_access_object_state_set_get(const Eo *obj, Elm_Toolbar_Item_Data *pd);


static Eina_Bool
_elm_toolbar_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_TOOLBAR_ITEM_EXTRA_OPS
#define ELM_TOOLBAR_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_prev_get, _elm_toolbar_item_prev_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_next_get, _elm_toolbar_item_next_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_selected_set, _elm_toolbar_item_selected_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_selected_get, _elm_toolbar_item_selected_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_priority_set, _elm_toolbar_item_priority_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_priority_get, _elm_toolbar_item_priority_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_icon_set, _elm_toolbar_item_icon_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_icon_get, _elm_toolbar_item_icon_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_object_get, _elm_toolbar_item_object_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_icon_object_get, _elm_toolbar_item_icon_object_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_separator_set, _elm_toolbar_item_separator_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_separator_get, _elm_toolbar_item_separator_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_menu_get, _elm_toolbar_item_menu_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_state_set, _elm_toolbar_item_state_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_state_get, _elm_toolbar_item_state_get),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_icon_memfile_set, _elm_toolbar_item_icon_memfile_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_icon_file_set, _elm_toolbar_item_icon_file_set),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_state_add, _elm_toolbar_item_state_add),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_state_del, _elm_toolbar_item_state_del),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_state_next, _elm_toolbar_item_state_next),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_state_prev, _elm_toolbar_item_state_prev),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_show, _elm_toolbar_item_show),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_bring_in, _elm_toolbar_item_bring_in),
      EFL_OBJECT_OP_FUNC(elm_obj_toolbar_item_menu_set, _elm_toolbar_item_menu_set),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_toolbar_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_invalidate, _elm_toolbar_item_efl_object_invalidate),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disable, _elm_toolbar_item_elm_widget_item_disable),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disabled_set, _elm_toolbar_item_elm_widget_item_disabled_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_set, _elm_toolbar_item_elm_widget_item_item_focus_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_get, _elm_toolbar_item_elm_widget_item_item_focus_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_toolbar_item_elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_set, _elm_toolbar_item_elm_widget_item_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_toolbar_item_elm_widget_item_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_set, _elm_toolbar_item_elm_widget_item_part_content_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_get, _elm_toolbar_item_elm_widget_item_part_content_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_unset, _elm_toolbar_item_elm_widget_item_part_content_unset),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_geometry_get, _elm_toolbar_item_efl_ui_focus_object_focus_geometry_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_set, _elm_toolbar_item_efl_ui_focus_object_focus_set),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_parent_get, _elm_toolbar_item_efl_ui_focus_object_focus_parent_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_toolbar_item_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_toolbar_item_efl_access_object_state_set_get),
      ELM_TOOLBAR_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"selected", __eolian_elm_toolbar_item_selected_set_reflect, __eolian_elm_toolbar_item_selected_get_reflect},
      {"priority", __eolian_elm_toolbar_item_priority_set_reflect, __eolian_elm_toolbar_item_priority_get_reflect},
      {"icon", __eolian_elm_toolbar_item_icon_set_reflect, __eolian_elm_toolbar_item_icon_get_reflect},
      {"separator", __eolian_elm_toolbar_item_separator_set_reflect, __eolian_elm_toolbar_item_separator_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_toolbar_item_class_desc = {
   EO_VERSION,
   "Elm.Toolbar_Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Toolbar_Item_Data),
   _elm_toolbar_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_toolbar_item_class_get, &_elm_toolbar_item_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_UI_FOCUS_OBJECT_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_toolbar_item_eo.legacy.c"
