EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_PAGE_CHANGED =
   EFL_EVENT_DESCRIPTION("scroll,page,changed");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_HBAR_UNPRESS =
   EFL_EVENT_DESCRIPTION("hbar,unpress");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_HBAR_PRESS =
   EFL_EVENT_DESCRIPTION("hbar,press");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_HBAR_DRAG =
   EFL_EVENT_DESCRIPTION("hbar,drag");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_VBAR_UNPRESS =
   EFL_EVENT_DESCRIPTION("vbar,unpress");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_VBAR_PRESS =
   EFL_EVENT_DESCRIPTION("vbar,press");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_VBAR_DRAG =
   EFL_EVENT_DESCRIPTION("vbar,drag");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_LEFT =
   EFL_EVENT_DESCRIPTION("scroll,left");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_RIGHT =
   EFL_EVENT_DESCRIPTION("scroll,right");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_UP =
   EFL_EVENT_DESCRIPTION("scroll,up");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_SCROLL_DOWN =
   EFL_EVENT_DESCRIPTION("scroll,down");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_EDGE_LEFT =
   EFL_EVENT_DESCRIPTION("edge,left");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_EDGE_RIGHT =
   EFL_EVENT_DESCRIPTION("edge,right");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_EDGE_TOP =
   EFL_EVENT_DESCRIPTION("edge,top");
EWAPI const Efl_Event_Description _ELM_SCROLLER_EVENT_EDGE_BOTTOM =
   EFL_EVENT_DESCRIPTION("edge,bottom");

void _elm_scroller_custom_widget_base_theme_set(Eo *obj, Elm_Scroller_Data *pd, const char *klass, const char *group);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_scroller_custom_widget_base_theme_set, EFL_FUNC_CALL(klass, group), const char *klass, const char *group);

void _elm_scroller_page_scroll_limit_set(const Eo *obj, Elm_Scroller_Data *pd, int page_limit_h, int page_limit_v);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_scroller_page_scroll_limit_set, EFL_FUNC_CALL(page_limit_h, page_limit_v), int page_limit_h, int page_limit_v);

void _elm_scroller_page_scroll_limit_get(const Eo *obj, Elm_Scroller_Data *pd, int *page_limit_h, int *page_limit_v);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_scroller_page_scroll_limit_get, EFL_FUNC_CALL(page_limit_h, page_limit_v), int *page_limit_h, int *page_limit_v);

Efl_Object *_elm_scroller_efl_object_constructor(Eo *obj, Elm_Scroller_Data *pd);


void _elm_scroller_efl_gfx_entity_position_set(Eo *obj, Elm_Scroller_Data *pd, Eina_Position2D pos);


void _elm_scroller_efl_gfx_entity_size_set(Eo *obj, Elm_Scroller_Data *pd, Eina_Size2D size);


void _elm_scroller_efl_canvas_group_group_member_add(Eo *obj, Elm_Scroller_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Bool _elm_scroller_efl_content_content_set(Eo *obj, Elm_Scroller_Data *pd, Efl_Gfx_Entity *content);


Efl_Gfx_Entity *_elm_scroller_efl_content_content_get(const Eo *obj, Elm_Scroller_Data *pd);


Efl_Gfx_Entity *_elm_scroller_efl_content_content_unset(Eo *obj, Elm_Scroller_Data *pd);


Eina_Error _elm_scroller_efl_ui_widget_theme_apply(Eo *obj, Elm_Scroller_Data *pd);


Eina_Bool _elm_scroller_efl_ui_widget_on_access_activate(Eo *obj, Elm_Scroller_Data *pd, Efl_Ui_Activate act);


Eina_Bool _elm_scroller_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Scroller_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Bool _elm_scroller_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Scroller_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


void _elm_scroller_elm_interface_scrollable_page_size_set(Eo *obj, Elm_Scroller_Data *pd, int x, int y);


void _elm_scroller_elm_interface_scrollable_policy_set(Eo *obj, Elm_Scroller_Data *pd, Elm_Scroller_Policy hbar, Elm_Scroller_Policy vbar);


void _elm_scroller_elm_interface_scrollable_single_direction_set(Eo *obj, Elm_Scroller_Data *pd, Elm_Scroller_Single_Direction single_dir);


Elm_Scroller_Single_Direction _elm_scroller_elm_interface_scrollable_single_direction_get(const Eo *obj, Elm_Scroller_Data *pd);


void _elm_scroller_elm_interface_scrollable_content_loop_set(Eo *obj, Elm_Scroller_Data *sd, Eina_Bool loop_h, Eina_Bool loop_v);


void _elm_scroller_elm_interface_scrollable_content_loop_get(Eo *obj EINA_UNUSED, Elm_Scroller_Data *sd, Eina_Bool *loop_h, Eina_Bool *loop_v);


const Efl_Access_Action_Data *_elm_scroller_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Scroller_Data *pd);


Efl_Object *_elm_scroller_efl_part_part_get(const Eo *obj, Elm_Scroller_Data *pd, const char *name);


Eina_Bool _elm_scroller_efl_ui_widget_focus_state_apply(Eo *obj, Elm_Scroller_Data *pd, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect);


static Eina_Bool
_elm_scroller_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_SCROLLER_EXTRA_OPS
#define ELM_SCROLLER_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_scroller_custom_widget_base_theme_set, _elm_scroller_custom_widget_base_theme_set),
      EFL_OBJECT_OP_FUNC(elm_obj_scroller_page_scroll_limit_set, _elm_scroller_page_scroll_limit_set),
      EFL_OBJECT_OP_FUNC(elm_obj_scroller_page_scroll_limit_get, _elm_scroller_page_scroll_limit_get),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_scroller_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_scroller_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_scroller_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_member_add, _elm_scroller_efl_canvas_group_group_member_add),
      EFL_OBJECT_OP_FUNC(efl_content_set, _elm_scroller_efl_content_content_set),
      EFL_OBJECT_OP_FUNC(efl_content_get, _elm_scroller_efl_content_content_get),
      EFL_OBJECT_OP_FUNC(efl_content_unset, _elm_scroller_efl_content_content_unset),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_scroller_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_on_access_activate, _elm_scroller_efl_ui_widget_on_access_activate),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_sub_object_del, _elm_scroller_efl_ui_widget_widget_sub_object_del),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_scroller_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_page_size_set, _elm_scroller_elm_interface_scrollable_page_size_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_policy_set, _elm_scroller_elm_interface_scrollable_policy_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_single_direction_set, _elm_scroller_elm_interface_scrollable_single_direction_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_single_direction_get, _elm_scroller_elm_interface_scrollable_single_direction_get),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_content_loop_set, _elm_scroller_elm_interface_scrollable_content_loop_set),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_content_loop_get, _elm_scroller_elm_interface_scrollable_content_loop_get),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_scroller_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_part_get, _elm_scroller_efl_part_part_get),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_focus_state_apply, _elm_scroller_efl_ui_widget_focus_state_apply),
      ELM_SCROLLER_EXTRA_OPS
   );
   opsp = &ops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_scroller_class_desc = {
   EO_VERSION,
   "Elm.Scroller",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Scroller_Data),
   _elm_scroller_class_initializer,
   _elm_scroller_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_scroller_class_get, &_elm_scroller_class_desc, EFL_UI_LAYOUT_BASE_CLASS, ELM_INTERFACE_SCROLLABLE_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_SCROLLABLE_INTERFACE, EFL_CONTENT_INTERFACE, EFL_UI_LEGACY_INTERFACE, NULL);

#include "elm_scroller_eo.legacy.c"
