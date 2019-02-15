
void _elm_menu_item_icon_name_set(Eo *obj, Elm_Menu_Item_Data *pd, const char *icon);


static Eina_Error
__eolian_elm_menu_item_icon_name_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_menu_item_icon_name_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_menu_item_icon_name_set, EFL_FUNC_CALL(icon), const char *icon);

const char *_elm_menu_item_icon_name_get(const Eo *obj, Elm_Menu_Item_Data *pd);


static Eina_Value
__eolian_elm_menu_item_icon_name_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_menu_item_icon_name_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_item_icon_name_get, const char *, NULL);

Elm_Widget_Item *_elm_menu_item_prev_get(const Eo *obj, Elm_Menu_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_item_prev_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_menu_item_next_get(const Eo *obj, Elm_Menu_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_item_next_get, Elm_Widget_Item *, NULL);

void _elm_menu_item_selected_set(Eo *obj, Elm_Menu_Item_Data *pd, Eina_Bool selected);


static Eina_Error
__eolian_elm_menu_item_selected_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   Eina_Bool cval;
   if (!eina_value_bool_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_menu_item_selected_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_menu_item_selected_set, EFL_FUNC_CALL(selected), Eina_Bool selected);

Eina_Bool _elm_menu_item_selected_get(const Eo *obj, Elm_Menu_Item_Data *pd);


static Eina_Value
__eolian_elm_menu_item_selected_get_reflect(const Eo *obj)
{
   Eina_Bool val = elm_obj_menu_item_selected_get(obj);
   return eina_value_bool_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_item_selected_get, Eina_Bool, 0);

unsigned int _elm_menu_item_index_get(const Eo *obj, Elm_Menu_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_item_index_get, unsigned int, 0);

void _elm_menu_item_subitems_clear(Eo *obj, Elm_Menu_Item_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_menu_item_subitems_clear);

const Eina_List *_elm_menu_item_subitems_get(const Eo *obj, Elm_Menu_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_item_subitems_get, const Eina_List *, NULL);

Eina_Bool _elm_menu_item_is_separator(const Eo *obj, Elm_Menu_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_item_is_separator, Eina_Bool, 0);

Efl_Canvas_Object *_elm_menu_item_object_get(const Eo *obj, Elm_Menu_Item_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_item_object_get, Efl_Canvas_Object *, NULL);

Efl_Object *_elm_menu_item_efl_object_constructor(Eo *obj, Elm_Menu_Item_Data *pd);


void _elm_menu_item_efl_object_destructor(Eo *obj, Elm_Menu_Item_Data *pd);


Efl_Object *_elm_menu_item_efl_object_parent_get(const Eo *obj, Elm_Menu_Item_Data *pd);


void _elm_menu_item_elm_widget_item_disable(Eo *obj, Elm_Menu_Item_Data *pd);


void _elm_menu_item_elm_widget_item_signal_emit(Eo *obj, Elm_Menu_Item_Data *pd, const char *emission, const char *source);


void _elm_menu_item_elm_widget_item_part_text_set(Eo *obj, Elm_Menu_Item_Data *pd, const char *part, const char *label);


const char *_elm_menu_item_elm_widget_item_part_text_get(const Eo *obj, Elm_Menu_Item_Data *pd, const char *part);


void _elm_menu_item_elm_widget_item_part_content_set(Eo *obj, Elm_Menu_Item_Data *pd, const char *part, Efl_Canvas_Object *content);


Efl_Canvas_Object *_elm_menu_item_elm_widget_item_part_content_get(const Eo *obj, Elm_Menu_Item_Data *pd, const char *part);


Eina_List *_elm_menu_item_efl_access_object_access_children_get(const Eo *obj, Elm_Menu_Item_Data *pd);


Efl_Access_Role _elm_menu_item_efl_access_object_role_get(const Eo *obj, Elm_Menu_Item_Data *pd);


const char *_elm_menu_item_efl_access_object_i18n_name_get(const Eo *obj, Elm_Menu_Item_Data *pd);


Efl_Access_State_Set _elm_menu_item_efl_access_object_state_set_get(const Eo *obj, Elm_Menu_Item_Data *pd);


int _elm_menu_item_efl_access_selection_selected_children_count_get(const Eo *obj, Elm_Menu_Item_Data *pd);


Efl_Object *_elm_menu_item_efl_access_selection_selected_child_get(const Eo *obj, Elm_Menu_Item_Data *pd, int selected_child_index);


static Eina_Bool
_elm_menu_item_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_MENU_ITEM_EXTRA_OPS
#define ELM_MENU_ITEM_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_icon_name_set, _elm_menu_item_icon_name_set),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_icon_name_get, _elm_menu_item_icon_name_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_prev_get, _elm_menu_item_prev_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_next_get, _elm_menu_item_next_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_selected_set, _elm_menu_item_selected_set),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_selected_get, _elm_menu_item_selected_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_index_get, _elm_menu_item_index_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_subitems_clear, _elm_menu_item_subitems_clear),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_subitems_get, _elm_menu_item_subitems_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_is_separator, _elm_menu_item_is_separator),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_object_get, _elm_menu_item_object_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_menu_item_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_menu_item_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_parent_get, _elm_menu_item_efl_object_parent_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_disable, _elm_menu_item_elm_widget_item_disable),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_signal_emit, _elm_menu_item_elm_widget_item_signal_emit),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_set, _elm_menu_item_elm_widget_item_part_text_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_text_get, _elm_menu_item_elm_widget_item_part_text_get),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_set, _elm_menu_item_elm_widget_item_part_content_set),
      EFL_OBJECT_OP_FUNC(elm_wdg_item_part_content_get, _elm_menu_item_elm_widget_item_part_content_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_menu_item_efl_access_object_access_children_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_role_get, _elm_menu_item_efl_access_object_role_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_i18n_name_get, _elm_menu_item_efl_access_object_i18n_name_get),
      EFL_OBJECT_OP_FUNC(efl_access_object_state_set_get, _elm_menu_item_efl_access_object_state_set_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_children_count_get, _elm_menu_item_efl_access_selection_selected_children_count_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_get, _elm_menu_item_efl_access_selection_selected_child_get),
      ELM_MENU_ITEM_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"icon_name", __eolian_elm_menu_item_icon_name_set_reflect, __eolian_elm_menu_item_icon_name_get_reflect},
      {"selected", __eolian_elm_menu_item_selected_set_reflect, __eolian_elm_menu_item_selected_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_menu_item_class_desc = {
   EO_VERSION,
   "Elm.Menu.Item",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Menu_Item_Data),
   _elm_menu_item_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(elm_menu_item_class_get, &_elm_menu_item_class_desc, ELM_WIDGET_ITEM_CLASS, EFL_ACCESS_SELECTION_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_menu_item_eo.legacy.c"
