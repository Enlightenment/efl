#ifndef EOLIAN_MONO_FUNCTION_BLACKLIST_HH
#define EOLIAN_MONO_FUNCTION_BLACKLIST_HH

namespace eolian_mono {

inline bool is_function_blacklisted(std::string const& c_name)
{
  return
    c_name == "efl_event_callback_array_priority_add"
    || c_name == "efl_event_callback_call"
    || c_name == "efl_event_callback_legacy_call"
    || c_name == "efl_canvas_group_change"
    || c_name == "efl_event_callback_priority_add"
    ;
}

}

#endif
