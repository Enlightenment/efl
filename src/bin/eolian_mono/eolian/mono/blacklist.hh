#ifndef EOLIAN_MONO_BLACKLIST_HH
#define EOLIAN_MONO_BLACKLIST_HH

#include "grammar/klass_def.hpp"
#include "name_helpers.hh"

namespace eolian_mono {

namespace blacklist {

namespace attributes = efl::eolian::grammar::attributes;

inline bool is_function_blacklisted(std::string const& c_name)
{
  return
    c_name == "efl_event_callback_array_priority_add"
    || c_name == "efl_player_position_get"
    || c_name == "efl_text_font_source_get"
    || c_name == "efl_text_font_source_set"
    || c_name == "efl_ui_focus_manager_focus_get"
    || c_name == "efl_ui_widget_focus_set"
    || c_name == "efl_ui_widget_focus_get"
    || c_name == "efl_ui_text_password_get"
    || c_name == "efl_ui_text_password_set"
    || c_name == "elm_interface_scrollable_repeat_events_get"
    || c_name == "elm_interface_scrollable_repeat_events_set"
    || c_name == "elm_wdg_item_del"
    || c_name == "elm_wdg_item_focus_get"
    || c_name == "elm_wdg_item_focus_set"
    || c_name == "elm_interface_scrollable_mirrored_set"
    || c_name == "evas_obj_table_mirrored_get"
    || c_name == "evas_obj_table_mirrored_set"
    || c_name == "edje_obj_load_error_get"
    || c_name == "efl_ui_focus_user_parent_get"
    || c_name == "efl_canvas_object_scale_get" // duplicated signature
    || c_name == "efl_canvas_object_scale_set" // duplicated signature
    || c_name == "efl_access_parent_get"
    || c_name == "efl_access_name_get"
    || c_name == "efl_access_name_set"
    || c_name == "efl_access_root_get"
    || c_name == "efl_access_type_get"
    || c_name == "efl_access_role_get"
    || c_name == "efl_access_action_description_get"
    || c_name == "efl_access_action_description_set"
    || c_name == "efl_access_image_description_get"
    || c_name == "efl_access_image_description_set"
    || c_name == "efl_access_component_layer_get" // duplicated signature
    || c_name == "efl_access_component_alpha_get"
    || c_name == "efl_access_component_size_get"
    || c_name == "efl_ui_spin_button_loop_get"
    || c_name == "efl_ui_list_model_size_get"
    || c_name == "efl_ui_list_relayout_layout_do"
    ;
}

// Blacklist structs that require some kind of manual binding.
inline bool is_struct_blacklisted(std::string const& full_name)
{
   return full_name == "Efl.Event.Description"
       || full_name == "Eina.Binbuf"
       || full_name == "Eina.Strbuf"
       || full_name == "Eina.Slice"
       || full_name == "Eina.Rw_Slice";
}

inline bool is_struct_blacklisted(attributes::struct_def const& struct_)
{
   return is_struct_blacklisted(name_helpers::struct_full_eolian_name(struct_));
}

inline bool is_struct_blacklisted(attributes::regular_type_def const& struct_)
{
   return is_struct_blacklisted(name_helpers::type_full_eolian_name(struct_));
}

}

}

#endif
