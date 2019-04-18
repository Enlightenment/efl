EWAPI const Efl_Event_Description _ELM_MENU_EVENT_DISMISSED =
   EFL_EVENT_DESCRIPTION("dismissed");
EWAPI const Efl_Event_Description _ELM_MENU_EVENT_ELM_ACTION_BLOCK_MENU =
   EFL_EVENT_DESCRIPTION("elm,action,block_menu");
EWAPI const Efl_Event_Description _ELM_MENU_EVENT_ELM_ACTION_UNBLOCK_MENU =
   EFL_EVENT_DESCRIPTION("elm,action,unblock_menu");

Elm_Widget_Item *_elm_menu_selected_item_get(const Eo *obj, Elm_Menu_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_selected_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_menu_first_item_get(const Eo *obj, Elm_Menu_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_first_item_get, Elm_Widget_Item *, NULL);

Elm_Widget_Item *_elm_menu_last_item_get(const Eo *obj, Elm_Menu_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_last_item_get, Elm_Widget_Item *, NULL);

const Eina_List *_elm_menu_items_get(const Eo *obj, Elm_Menu_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_menu_items_get, const Eina_List *, NULL);

void _elm_menu_relative_move(Eo *obj, Elm_Menu_Data *pd, int x, int y);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_menu_relative_move, EFL_FUNC_CALL(x, y), int x, int y);

Elm_Widget_Item *_elm_menu_item_add(Eo *obj, Elm_Menu_Data *pd, Elm_Widget_Item *parent, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_menu_item_add, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(parent, icon, label, func, data), Elm_Widget_Item *parent, const char *icon, const char *label, Evas_Smart_Cb func, const void *data);

void _elm_menu_open(Eo *obj, Elm_Menu_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_menu_open);

void _elm_menu_close(Eo *obj, Elm_Menu_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(elm_obj_menu_close);

Elm_Widget_Item *_elm_menu_item_separator_add(Eo *obj, Elm_Menu_Data *pd, Elm_Widget_Item *parent);

EOAPI EFL_FUNC_BODYV(elm_obj_menu_item_separator_add, Elm_Widget_Item *, NULL, EFL_FUNC_CALL(parent), Elm_Widget_Item *parent);

Efl_Object *_elm_menu_efl_object_constructor(Eo *obj, Elm_Menu_Data *pd);


void _elm_menu_efl_object_destructor(Eo *obj, Elm_Menu_Data *pd);


void _elm_menu_efl_gfx_entity_visible_set(Eo *obj, Elm_Menu_Data *pd, Eina_Bool v);


Eina_Error _elm_menu_efl_ui_widget_theme_apply(Eo *obj, Elm_Menu_Data *pd);


void _elm_menu_efl_ui_l10n_translation_update(Eo *obj, Elm_Menu_Data *pd);


Efl_Ui_Focus_Manager *_elm_menu_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj, Elm_Menu_Data *pd, Efl_Ui_Focus_Object *root);


Eina_List *_elm_menu_efl_access_object_access_children_get(const Eo *obj, Elm_Menu_Data *pd);


int _elm_menu_efl_access_selection_selected_children_count_get(const Eo *obj, Elm_Menu_Data *pd);


Efl_Object *_elm_menu_efl_access_selection_selected_child_get(const Eo *obj, Elm_Menu_Data *pd, int selected_child_index);


Efl_Object *_elm_menu_efl_object_provider_find(const Eo *obj, Elm_Menu_Data *pd, const Efl_Class *klass);


static Eina_Bool
_elm_menu_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_MENU_EXTRA_OPS
#define ELM_MENU_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_menu_selected_item_get, _elm_menu_selected_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_first_item_get, _elm_menu_first_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_last_item_get, _elm_menu_last_item_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_items_get, _elm_menu_items_get),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_relative_move, _elm_menu_relative_move),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_add, _elm_menu_item_add),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_open, _elm_menu_open),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_close, _elm_menu_close),
      EFL_OBJECT_OP_FUNC(elm_obj_menu_item_separator_add, _elm_menu_item_separator_add),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_menu_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _elm_menu_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_visible_set, _elm_menu_efl_gfx_entity_visible_set),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_menu_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_l10n_translation_update, _elm_menu_efl_ui_l10n_translation_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_manager_create, _elm_menu_efl_ui_widget_focus_manager_focus_manager_create),
      EFL_OBJECT_OP_FUNC(efl_access_object_access_children_get, _elm_menu_efl_access_object_access_children_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_children_count_get, _elm_menu_efl_access_selection_selected_children_count_get),
      EFL_OBJECT_OP_FUNC(efl_access_selection_selected_child_get, _elm_menu_efl_access_selection_selected_child_get),
      EFL_OBJECT_OP_FUNC(efl_provider_find, _elm_menu_efl_object_provider_find),
      ELM_MENU_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_menu_class_desc = {
   EO_VERSION,
   "Elm.Menu",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Menu_Data),
   _elm_menu_class_initializer,
   _elm_menu_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_menu_class_get, &_elm_menu_class_desc, EFL_UI_WIDGET_CLASS, EFL_UI_CLICKABLE_INTERFACE, EFL_ACCESS_SELECTION_INTERFACE, EFL_UI_WIDGET_FOCUS_MANAGER_MIXIN, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_menu_eo.legacy.c"
