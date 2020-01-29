/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef EOLIAN_MONO_BLACKLIST_HH
#define EOLIAN_MONO_BLACKLIST_HH

#include "grammar/klass_def.hpp"
#include "grammar/context.hpp"
#include "name_helpers.hh"
#include "generation_contexts.hh"

namespace eolian_mono {

namespace blacklist {

namespace attributes = efl::eolian::grammar::attributes;

inline bool is_function_blacklisted(std::string const& c_name)
{
  return
    c_name == "efl_event_callback_array_priority_add"
    || c_name == "efl_constructor"
    || c_name == "efl_ui_widget_focus_set"
    || c_name == "efl_ui_widget_focus_get"
    || c_name == "efl_ui_text_password_get"
    || c_name == "efl_ui_text_password_set"
    || c_name == "elm_interface_scrollable_repeat_events_get"
    || c_name == "elm_interface_scrollable_repeat_events_set"
    || c_name == "elm_wdg_item_del"
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
    || c_name == "efl_event_callback_forwarder_priority_add" // Depends on constants support.
    || c_name == "efl_event_callback_forwarder_del"
    || c_name == "efl_ui_text_context_menu_item_add"
    || c_name == "efl_ui_widget_input_event_handler"
    || c_name == "efl_access_object_event_handler_add"
    || c_name == "efl_access_object_event_handler_del"
    || c_name == "efl_access_object_event_emit"
    || c_name == "efl_access_widget_action_elm_actions_get"
    || c_name == "efl_access_action_actions_get"
    ;
}

template<typename Context>
inline bool is_function_blacklisted(attributes::function_def const& func, Context const& context)
{
  auto options = efl::eolian::grammar::context_find_tag<options_context>(context);
  auto c_name = func.c_name;

  if (func.is_beta && !options.want_beta)
    return true;

  return is_function_blacklisted(c_name);
}

inline bool is_non_public_interface_member(attributes::function_def const& func, attributes::klass_def const&current_klass)
{
  if (current_klass.type == attributes::class_type::interface_
      || current_klass.type == attributes::class_type::mixin)
    {
       if (func.scope != attributes::member_scope::scope_public)
         return true;
    }

  return false;
}


// Blacklist structs that require some kind of manual binding.
inline bool is_struct_blacklisted(std::string const& full_name)
{
   // For now, these manual structs are blacklisted regardless of beta status
   return full_name == "Efl.Event_Description"
       || full_name == "Eina.Binbuf"
       || full_name == "Eina.Strbuf"
       || full_name == "Eina.Slice"
       || full_name == "Eina.Rw_Slice"
       || full_name == "Eina.Promise"
       || full_name == "Eina.Value"
       || full_name == "Eina.Value_Type"
       || full_name == "Eina.Future";
}

template <typename Context>
inline bool is_struct_blacklisted(attributes::struct_def const& struct_, Context const& context)
{
   auto options = efl::eolian::grammar::context_find_tag<options_context>(context);
   if (struct_.is_beta && !options.want_beta)
     return true;

   return is_struct_blacklisted(name_helpers::struct_full_eolian_name(struct_));
}

// Struct as type_def is for places where the struct is used as a struct field or parameter/return.
template <typename Context>
inline bool is_struct_blacklisted(attributes::type_def const& struct_, Context const& context)
{
   auto options = efl::eolian::grammar::context_find_tag<options_context>(context);
   if (struct_.is_beta && !options.want_beta)
     return true;

   auto regular = efl::eina::get<attributes::regular_type_def>(struct_.original_type);
   return is_struct_blacklisted(name_helpers::type_full_eolian_name(regular));
}

inline bool is_struct_blacklisted(attributes::regular_type_def const& struct_)
{
   return is_struct_blacklisted(name_helpers::type_full_eolian_name(struct_));
}

template <typename Context>
inline bool is_alias_blacklisted(attributes::alias_def const& alias, Context const& context)
{
   auto options = efl::eolian::grammar::context_find_tag<options_context>(context);
   if (alias.is_beta && !options.want_beta)
     return true;

   return name_helpers::alias_full_eolian_name(alias) == "Eina.Error";
}

inline bool is_property_blacklisted(std::string const&)
{
    return false;
}

template<typename Context>
inline bool is_property_blacklisted(attributes::property_def const& property, Context const& context)
{
    auto name = name_helpers::klass_full_concrete_or_interface_name(property.klass) + "." + name_helpers::property_managed_name(property);

    if (property.getter.is_engaged())
      if (is_function_blacklisted(*property.getter, context))
        return true;
    if (property.setter.is_engaged())
      if (is_function_blacklisted(*property.setter, context))
        return true;
    return is_property_blacklisted(name);
}

template<typename Context>
inline bool is_property_blacklisted(attributes::property_def const& property,
                                    EINA_UNUSED attributes::klass_def const& implementing_class,
                                    Context const& context)
{
   return is_property_blacklisted(property, context);
}

template<typename Context>
inline bool is_class_blacklisted(attributes::klass_def const& cls, Context const& context)
{
   auto options = efl::eolian::grammar::context_find_tag<options_context>(context);

   return cls.is_beta && !options.want_beta;
}

template<typename Context>
inline bool is_class_blacklisted(attributes::klass_name const& cls, Context const& context)
{
   auto options = efl::eolian::grammar::context_find_tag<options_context>(context);

   return cls.is_beta && !options.want_beta;
}


template<typename Context>
inline bool is_event_blacklisted(attributes::event_def const& evt, Context const& context)
{
   auto options = efl::eolian::grammar::context_find_tag<options_context>(context);

   return evt.is_beta && !options.want_beta;
}

}

}

#endif
