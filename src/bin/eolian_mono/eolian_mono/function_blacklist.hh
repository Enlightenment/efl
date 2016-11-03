#ifndef EOLIAN_MONO_FUNCTION_BLACKLIST_HH
#define EOLIAN_MONO_FUNCTION_BLACKLIST_HH

namespace eolian_mono {

inline bool is_function_blacklisted(std::string const& c_name)
{
  return c_name == "efl_constructor"
    || c_name == "efl_parent_set"
    || c_name == "efl_parent_get"
    || c_name == "efl_canvas_object_type_set"
    || c_name == "efl_provider_find"
    || c_name == "efl_canvas_group_add"
    || c_name == "efl_canvas_group_member_add"
    || c_name == "efl_event_callback_array_priority_add"
    || c_name == "efl_event_callback_call"
    || c_name == "efl_finalized_get"
    || c_name == "efl_finalize"
    || c_name == "efl_gfx_position_get"
    || c_name == "efl_gfx_geometry_get"
    || c_name == "efl_gfx_size_get"
    || c_name == "efl_event_callback_legacy_call"
    || c_name == "efl_gfx_visible_get"
    || c_name == "efl_canvas_group_change"
    || c_name == "efl_canvas_group_need_recalculate_set"
    || c_name == "efl_gfx_size_hint_restricted_min_set"
    || c_name == "efl_gfx_size_hint_max_get"
    || c_name == "efl_gfx_size_hint_request_get"
    || c_name == "efl_gfx_size_hint_combined_min_get"
    || c_name == "efl_event_callback_priority_add"
    // || c_name == "efl_canvas_group_calculate"

    /*
      // descs[50].api_func = efl.eo.Globals.dlsym(IntPtr.Zero, "efl_event_callback_priority_add");
      // descs[50].func = Marshal.GetFunctionPointerForDelegate(BoxNativeInherit.event_callback_priority_add_static_delegate);
      // descs[51].api_func = efl.eo.Globals.dlsym(IntPtr.Zero, "efl_event_callback_del");
      // descs[51].func = Marshal.GetFunctionPointerForDelegate(BoxNativeInherit.event_callback_del_static_delegate);
      // descs[52].api_func = efl.eo.Globals.dlsym(IntPtr.Zero, "efl_event_callback_array_del");
      // descs[52].func = Marshal.GetFunctionPointerForDelegate(BoxNativeInherit.event_callback_array_del_static_delegate);
      // descs[53].api_func = efl.eo.Globals.dlsym(IntPtr.Zero, "efl_event_callback_stop");
      // descs[53].func = Marshal.GetFunctionPointerForDelegate(BoxNativeInherit.event_callback_stop_static_delegate);
      // descs[54].api_func = efl.eo.Globals.dlsym(IntPtr.Zero, "efl_event_callback_forwarder_add");
      // descs[54].func = Marshal.GetFunctionPointerForDelegate(BoxNativeInherit.event_callback_forwarder_add_static_delegate);
      // descs[55].api_func = efl.eo.Globals.dlsym(IntPtr.Zero, "efl_event_callback_forwarder_del");
      // descs[55].func = Marshal.GetFunctionPointerForDelegate(BoxNativeInherit.event_callback_forwarder_del_static_delegate);
      */    
    ;
}

}

#endif
