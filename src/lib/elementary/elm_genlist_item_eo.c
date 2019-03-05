
Elm_Widget_Item *_elm_genlist_item_prev_get(const Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_prev_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_genlist_item_next_get(const Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_next_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_genlist_item_parent_item_get(const Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_parent_item_get, Elm_Widget_Item *, NULL);

const Eina_List *_elm_genlist_item_subitems_get(const Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_subitems_get, const Eina_List *, NULL);

void _elm_genlist_item_selected_set(Eo *obj, Elm_Gen_Item *pd, Eina_Bool selected);


static Eina_Error
__eolian_elm_genlist_item_selected_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_item_selected_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_selected_set, EFL_FUNC_CALL(selected), Eina_Bool selected);

Eina_Bool _elm_genlist_item_selected_get(const Eo *obj, Elm_Gen_Item *pd);


static Eina_Value
__eolian_elm_genlist_item_selected_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_item_selected_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_selected_get, Eina_Bool, 0);

void _elm_genlist_item_expanded_set(Eo *obj, Elm_Gen_Item *pd, Eina_Bool expanded);


static Eina_Error
__eolian_elm_genlist_item_expanded_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_item_expanded_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_expanded_set, EFL_FUNC_CALL(expanded), Eina_Bool expanded);

Eina_Bool _elm_genlist_item_expanded_get(const Eo *obj, Elm_Gen_Item *pd);


static Eina_Value
__eolian_elm_genlist_item_expanded_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_item_expanded_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_expanded_get, Eina_Bool, 0);

int _elm_genlist_item_expanded_depth_get(const Eo *obj, Elm_Gen_Item *pd);


static Eina_Value
__eolian_elm_genlist_item_expanded_depth_get_reflect(Eo *obj)
{
   int val = elm_obj_genlist_item_expanded_depth_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_expanded_depth_get, int, 0);

const Elm_Genlist_Item_Class *_elm_genlist_item_item_class_get(const Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_class_get, const Elm_Genlist_Item_Class *, NULL);

int _elm_genlist_item_index_get(const Eo *obj, Elm_Gen_Item *pd);


static Eina_Value
__eolian_elm_genlist_item_index_get_reflect(Eo *obj)
{
   int val = elm_obj_genlist_item_index_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_index_get, int, -1 /* +1 */);

const char *_elm_genlist_item_decorate_mode_get(const Eo *obj, Elm_Gen_Item *pd);


static Eina_Value
__eolian_elm_genlist_item_decorate_mode_get_reflect(Eo *obj)
{
   const char *val = elm_obj_genlist_item_decorate_mode_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_decorate_mode_get, const char *, NULL);

void _elm_genlist_item_flip_set(Eo *obj, Elm_Gen_Item *pd, Eina_Bool flip);


static Eina_Error
__eolian_elm_genlist_item_flip_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_item_flip_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_flip_set, EFL_FUNC_CALL(flip), Eina_Bool flip);

Eina_Bool _elm_genlist_item_flip_get(const Eo *obj, Elm_Gen_Item *pd);


static Eina_Value
__eolian_elm_genlist_item_flip_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_item_flip_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_flip_get, Eina_Bool, 0);

void _elm_genlist_item_select_mode_set(Eo *obj, Elm_Gen_Item *pd, Elm_Object_Select_Mode mode);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_select_mode_set, EFL_FUNC_CALL(mode), Elm_Object_Select_Mode mode);

Elm_Object_Select_Mode _elm_genlist_item_select_mode_get(const Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_select_mode_get, Elm_Object_Select_Mode, 4 /* Elm.Object.Select_Mode.max */);

Elm_Genlist_Item_Type _elm_genlist_item_type_get(const Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_type_get, Elm_Genlist_Item_Type, 4 /* Elm.Genlist.Item.Type.max */);

void _elm_genlist_item_pin_set(Eo *obj, Elm_Gen_Item *pd, Eina_Bool pin);


static Eina_Error
__eolian_elm_genlist_item_pin_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_genlist_item_pin_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_pin_set, EFL_FUNC_CALL(pin), Eina_Bool pin);

Eina_Bool _elm_genlist_item_pin_get(const Eo *obj, Elm_Gen_Item *pd);


static Eina_Value
__eolian_elm_genlist_item_pin_get_reflect(Eo *obj)
{
   Eina_Bool val = elm_obj_genlist_item_pin_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_genlist_item_pin_get, Eina_Bool, 0);

unsigned int _elm_genlist_item_subitems_count(Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_FUNC_BODY(elm_obj_genlist_item_subitems_count, unsigned int, 0);

void _elm_genlist_item_subitems_clear(Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_genlist_item_subitems_clear);

void _elm_genlist_item_promote(Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_genlist_item_promote);

void _elm_genlist_item_demote(Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_genlist_item_demote);

void _elm_genlist_item_show(Eo *obj, Elm_Gen_Item *pd, Elm_Genlist_Item_Scrollto_Type type);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_show, EFL_FUNC_CALL(type), Elm_Genlist_Item_Scrollto_Type type);

void _elm_genlist_item_bring_in(Eo *obj, Elm_Gen_Item *pd, Elm_Genlist_Item_Scrollto_Type type);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_bring_in, EFL_FUNC_CALL(type), Elm_Genlist_Item_Scrollto_Type type);

void _elm_genlist_item_all_contents_unset(Eo *obj, Elm_Gen_Item *pd, Eina_List **l);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_all_contents_unset, EFL_FUNC_CALL(l), Eina_List **l);

void _elm_genlist_item_update(Eo *obj, Elm_Gen_Item *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_genlist_item_update);

void _elm_genlist_item_fields_update(Eo *obj, Elm_Gen_Item *pd, const char *parts, Elm_Genlist_Item_Field_Type itf);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_fields_update, EFL_FUNC_CALL(parts, itf), const char *parts, Elm_Genlist_Item_Field_Type itf);

void _elm_genlist_item_item_class_update(Eo *obj, Elm_Gen_Item *pd, const Elm_Genlist_Item_Class *itc);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_class_update, EFL_FUNC_CALL(itc), const Elm_Genlist_Item_Class *itc);

void _elm_genlist_item_decorate_mode_set(Eo *obj, Elm_Gen_Item *pd, const char *decorate_it_type, Eina_Bool decorate_it_set);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_genlist_item_decorate_mode_set, EFL_FUNC_CALL(decorate_it_type, decorate_it_set), const char *decorate_it_type, Eina_Bool decorate_it_set);

Efl_Object *_elm_genlist_item_efl_object_constructor(Eo *obj, Elm_Gen_Item *pd);


void _elm_genlist_item_elm_widget_item_del_pre(Eo *obj, Elm_Gen_Item *pd);


void _elm_genlist_item_elm_widget_item_disable(Eo *obj, Elm_Gen_Item *pd);


void _elm_genlist_item_elm_widget_item_signal_emit(Eo *obj, Elm_Gen_Item *pd, const char *emission, const char *source);


void _elm_genlist_item_elm_widget_item_style_set(Eo *obj, Elm_Gen_Item *pd, const char *style);


const char *_elm_genlist_item_elm_widget_item_style_get(const Eo *obj, Elm_Gen_Item *pd);


void _elm_genlist_item_elm_widget_item_item_focus_set(Eo *obj, Elm_Gen_Item *pd, Eina_Bool focused);


Eina_Bool _elm_genlist_item_elm_widget_item_item_focus_get(const Eo *obj, Elm_Gen_Item *pd);


const char *_elm_genlist_item_elm_widget_item_part_text_get(const Eo *obj, Elm_Gen_Item *pd, const char *part);


Efl_Canvas_Object *_elm_genlist_item_elm_widget_item_part_content_get(const Eo *obj, Elm_Gen_Item *pd, const char *part);


void _elm_genlist_item_elm_widget_item_tooltip_text_set(Eo *obj, Elm_Gen_Item *pd, const char *text);


void _elm_genlist_item_elm_widget_item_tooltip_style_set(Eo *obj, Elm_Gen_Item *pd, const char *style);


const char *_elm_genlist_item_elm_widget_item_tooltip_style_get(const Eo *obj, Elm_Gen_Item *pd);


Eina_Bool _elm_genlist_item_elm_widget_item_tooltip_window_mode_set(Eo *obj, Elm_Gen_Item *pd, Eina_Bool disable);


Eina_Bool _elm_genlist_item_elm_widget_item_tooltip_window_mode_get(const Eo *obj, Elm_Gen_Item *pd);


void _elm_genlist_item_elm_widget_item_tooltip_content_cb_set(Eo *obj, Elm_Gen_Item *pd, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);


void _elm_genlist_item_elm_widget_item_tooltip_unset(Eo *obj, Elm_Gen_Item *pd);


void _elm_genlist_item_elm_widget_item_cursor_set(Eo *obj, Elm_Gen_Item *pd, const char *cursor);


void _elm_genlist_item_elm_widget_item_cursor_unset(Eo *obj, Elm_Gen_Item *pd);


const char *_elm_genlist_item_efl_access_object_i18n_name_get(const Eo *obj, Elm_Gen_Item *pd);


Efl_Access_State_Set _elm_genlist_item_efl_access_object_state_set_get(const Eo *obj, Elm_Gen_Item *pd);


void _elm_genlist_item_efl_ui_focus_object_setup_order_non_recursive(Eo *obj, Elm_Gen_Item *pd);


Efl_Ui_Focus_Object *_elm_genlist_item_efl_ui_focus_object_focus_parent_get(const Eo *obj, Elm_Gen_Item *pd);


static Eina_Bool
_elm_genlist_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_GENLIST_ITEM_EXTRA_OPS
#define ELM_GENLIST_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_prev_get, _elm_genlist_item_prev_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_next_get, _elm_genlist_item_next_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_parent_item_get, _elm_genlist_item_parent_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_subitems_get, _elm_genlist_item_subitems_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_selected_set, _elm_genlist_item_selected_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_selected_get, _elm_genlist_item_selected_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_expanded_set, _elm_genlist_item_expanded_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_expanded_get, _elm_genlist_item_expanded_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_expanded_depth_get, _elm_genlist_item_expanded_depth_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_class_get, _elm_genlist_item_item_class_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_index_get, _elm_genlist_item_index_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_decorate_mode_get, _elm_genlist_item_decorate_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_flip_set, _elm_genlist_item_flip_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_flip_get, _elm_genlist_item_flip_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_select_mode_set, _elm_genlist_item_select_mode_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_select_mode_get, _elm_genlist_item_select_mode_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_type_get, _elm_genlist_item_type_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_pin_set, _elm_genlist_item_pin_set),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_pin_get, _elm_genlist_item_pin_get),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_subitems_count, _elm_genlist_item_subitems_count),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_subitems_clear, _elm_genlist_item_subitems_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_promote, _elm_genlist_item_promote),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_demote, _elm_genlist_item_demote),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_show, _elm_genlist_item_show),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_bring_in, _elm_genlist_item_bring_in),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_all_contents_unset, _elm_genlist_item_all_contents_unset),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_update, _elm_genlist_item_update),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_fields_update, _elm_genlist_item_fields_update),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_class_update, _elm_genlist_item_item_class_update),
      EFL_OBJECT_OP_FUNC(elm_obj_genlist_item_decorate_mode_set, _elm_genlist_item_decorate_mode_set),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_genlist_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_del_pre, _elm_genlist_item_elm_widget_item_del_pre),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disable, _elm_genlist_item_elm_widget_item_disable),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_genlist_item_elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_style_set, _elm_genlist_item_elm_widget_item_style_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_style_get, _elm_genlist_item_elm_widget_item_style_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_set, _elm_genlist_item_elm_widget_item_item_focus_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_focus_get, _elm_genlist_item_elm_widget_item_item_focus_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_genlist_item_elm_widget_item_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_get, _elm_genlist_item_elm_widget_item_part_content_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_text_set, _elm_genlist_item_elm_widget_item_tooltip_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_style_set, _elm_genlist_item_elm_widget_item_tooltip_style_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_style_get, _elm_genlist_item_elm_widget_item_tooltip_style_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_window_mode_set, _elm_genlist_item_elm_widget_item_tooltip_window_mode_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_window_mode_get, _elm_genlist_item_elm_widget_item_tooltip_window_mode_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_content_cb_set, _elm_genlist_item_elm_widget_item_tooltip_content_cb_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_tooltip_unset, _elm_genlist_item_elm_widget_item_tooltip_unset),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_cursor_set, _elm_genlist_item_elm_widget_item_cursor_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_cursor_unset, _elm_genlist_item_elm_widget_item_cursor_unset),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_genlist_item_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_genlist_item_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_setup_order_non_recursive, _elm_genlist_item_efl_ui_focus_object_setup_order_non_recursive),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_focus_parent_get, _elm_genlist_item_efl_ui_focus_object_focus_parent_get),
      ELM_GENLIST_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"selected", __eolian_elm_genlist_item_selected_set_reflect, __eolian_elm_genlist_item_selected_get_reflect},
      {"expanded", __eolian_elm_genlist_item_expanded_set_reflect, __eolian_elm_genlist_item_expanded_get_reflect},
      {"expanded_depth", NULL, __eolian_elm_genlist_item_expanded_depth_get_reflect},
      {"index", NULL, __eolian_elm_genlist_item_index_get_reflect},
      {"decorate_mode", NULL, __eolian_elm_genlist_item_decorate_mode_get_reflect},
      {"flip", __eolian_elm_genlist_item_flip_set_reflect, __eolian_elm_genlist_item_flip_get_reflect},
      {"pin", __eolian_elm_genlist_item_pin_set_reflect, __eolian_elm_genlist_item_pin_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_genlist_item_class_desc = {
   EO_VERSION,
   "Elm.Genlist.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Gen_Item),
   _elm_genlist_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_genlist_item_class_get, &_elm_genlist_item_class_desc, ELM_WIDGET_ITEM_STATIC_FOCUS_CLASS, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_genlist_item_eo.legacy.c"
