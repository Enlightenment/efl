using System;
using System.Runtime.InteropServices;

public class BoxInherit : evas.Box
{
   System.IntPtr handle;
   public System.IntPtr raw_handle {
      get { return handle; }
   }
   // [System.Runtime.InteropServices.DllImport("eo")] static extern System.IntPtr
   // _efl_add_internal_start([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] System.String file, int line,
   //    System.IntPtr klass, System.IntPtr parent, byte is_ref, byte is_fallback);
   // [System.Runtime.InteropServices.DllImport("eo")] static extern System.IntPtr
   // _efl_add_end(System.IntPtr eo, byte is_ref, byte is_fallback);
   [System.Runtime.InteropServices.DllImport("evas")] static extern System.IntPtr
      evas_box_class_get();
   public delegate byte class_initializer_delegate(IntPtr klass);

   public static byte class_initializer(IntPtr klass)
   {
       System.Console.WriteLine("class_initializer");
       return 0;
   }
   static class_initializer_delegate class_initializer_delegate_inst = new class_initializer_delegate(BoxInherit.class_initializer);
   public BoxInherit(efl.Object parent = null)
   {
      ClassDescription description;
      description.version = 2; // EO_VERSION
      description.name = "Class Name";
      description.class_type = 0; // REGULAR
      description.data_size = UIntPtr.Zero;
      description.class_initializer = Marshal.GetFunctionPointerForDelegate(class_initializer_delegate_inst);
      description.class_constructor = IntPtr.Zero;
      description.class_destructor = IntPtr.Zero;

      IntPtr base_klass = evas_box_class_get();
      Console.WriteLine("Going to register!");
      IntPtr klass = efl.eo.Globals.efl_class_new(ref description, base_klass);
       if(klass == IntPtr.Zero)
           Console.WriteLine("klass was not registed");

      System.IntPtr parent_ptr = System.IntPtr.Zero;
      if(parent != null)
         parent_ptr = parent.raw_handle;

      System.IntPtr eo = efl.eo.Globals._efl_add_internal_start("file", 0, klass, parent_ptr, 0, 0);
      handle = efl.eo.Globals._efl_add_end(eo, 0, 0);
   }
   public BoxInherit(System.IntPtr raw)
   {
      handle = raw;
   }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void evas_obj_box_align_get(System.IntPtr obj,  double horizontal,  double vertical);
   public void align_get(double horizontal, double vertical) { evas_obj_box_align_get(handle, horizontal, vertical); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void evas_obj_box_align_set(System.IntPtr obj,  double horizontal,  double vertical);
   public void align_set(double horizontal, double vertical) { evas_obj_box_align_set(handle, horizontal, vertical); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void evas_obj_box_padding_get(System.IntPtr obj,  evas.Coord horizontal,  evas.Coord vertical);
   public void padding_get(evas.Coord horizontal, evas.Coord vertical) { evas_obj_box_padding_get(handle, horizontal, vertical); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void evas_obj_box_padding_set(System.IntPtr obj,  evas.Coord horizontal,  evas.Coord vertical);
   public void padding_set(evas.Coord horizontal, evas.Coord vertical) { evas_obj_box_padding_set(handle, horizontal, vertical); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void evas_obj_box_layout_set(System.IntPtr obj,  Evas_Object_Box_Layout cb,   System.IntPtr data,  Eina_Free_Cb free_data);
   public void layout_set(Evas_Object_Box_Layout cb,  System.IntPtr data, Eina_Free_Cb free_data) { evas_obj_box_layout_set(handle, cb, data, free_data); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.canvas.Object evas_obj_box_internal_remove(System.IntPtr obj,   efl.canvas.Object child);
   public  efl.canvas.Object internal_remove( efl.canvas.Object child) { return evas_obj_box_internal_remove(handle, child); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool evas_obj_box_remove_all(System.IntPtr obj,  bool clear);
   public bool remove_all(bool clear) { return evas_obj_box_remove_all(handle, clear); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   int evas_obj_box_iterator_new(System.IntPtr obj);
   public  int iterator_new() { return evas_obj_box_iterator_new(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.canvas.Object evas_obj_box_add_to(System.IntPtr obj);
   public  efl.canvas.Object add_to() { return evas_obj_box_add_to(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  Evas_Object_Box_Option evas_obj_box_append(System.IntPtr obj,   efl.canvas.Object child);
   public Evas_Object_Box_Option append( efl.canvas.Object child) { return evas_obj_box_append(handle, child); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  int evas_obj_box_option_property_id_get(System.IntPtr obj,   System.String name);
   public int option_property_id_get( System.String name) { return evas_obj_box_option_property_id_get(handle, name); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   int evas_obj_box_accessor_new(System.IntPtr obj);
   public  int accessor_new() { return evas_obj_box_accessor_new(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.canvas.Object evas_obj_box_internal_remove_at(System.IntPtr obj,   uint pos);
   public  efl.canvas.Object internal_remove_at( uint pos) { return evas_obj_box_internal_remove_at(handle, pos); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool evas_obj_box_remove_at(System.IntPtr obj,   uint pos);
   public bool remove_at( uint pos) { return evas_obj_box_remove_at(handle, pos); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   System.String evas_obj_box_option_property_name_get(System.IntPtr obj,  int property);
   public  System.String option_property_name_get(int property) { return evas_obj_box_option_property_name_get(handle, property); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool evas_obj_box_remove(System.IntPtr obj,   efl.canvas.Object child);
   public bool remove( efl.canvas.Object child) { return evas_obj_box_remove(handle, child); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  int evas_obj_box_count(System.IntPtr obj);
   public int count() { return evas_obj_box_count(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_position_get(System.IntPtr obj,  int x,  int y);
   public void position_get(int x, int y) { efl_gfx_position_get(handle, x, y); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_position_set(System.IntPtr obj,  int x,  int y);
   public void position_set(int x, int y) { efl_gfx_position_set(handle, x, y); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_get(System.IntPtr obj,  int w,  int h);
   public void size_get(int w, int h) { efl_gfx_size_get(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_set(System.IntPtr obj,  int w,  int h);
   public void size_set(int w, int h) { efl_gfx_size_set(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_geometry_get(System.IntPtr obj,  int x,  int y,  int w,  int h);
   public void geometry_get(int x, int y, int w, int h) { efl_gfx_geometry_get(handle, x, y, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_geometry_set(System.IntPtr obj,  int x,  int y,  int w,  int h);
   public void geometry_set(int x, int y, int w, int h) { efl_gfx_geometry_set(handle, x, y, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_color_get(System.IntPtr obj,  int r,  int g,  int b,  int a);
   public void color_get(int r, int g, int b, int a) { efl_gfx_color_get(handle, r, g, b, a); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_color_set(System.IntPtr obj,  int r,  int g,  int b,  int a);
   public void color_set(int r, int g, int b, int a) { efl_gfx_color_set(handle, r, g, b, a); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_color_part_get(System.IntPtr obj,   System.String part,  int r,  int g,  int b,  int a);
   public bool color_part_get( System.String part, int r, int g, int b, int a) { return efl_gfx_color_part_get(handle, part, r, g, b, a); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_color_part_set(System.IntPtr obj,   System.String part,  int r,  int g,  int b,  int a);
   public bool color_part_set( System.String part, int r, int g, int b, int a) { return efl_gfx_color_part_set(handle, part, r, g, b, a); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_visible_get(System.IntPtr obj);
   public bool visible_get() { return efl_gfx_visible_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_visible_set(System.IntPtr obj,  bool v);
   public void visible_set(bool v) { efl_gfx_visible_set(handle, v); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.Object efl_parent_get(System.IntPtr obj);
   public  efl.Object parent_get() { return efl_parent_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_parent_set(System.IntPtr obj,   efl.Object parent);
   public void parent_set( efl.Object parent) { efl_parent_set(handle, parent); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   System.String efl_name_get(System.IntPtr obj);
   public  System.String name_get() { return efl_name_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_name_set(System.IntPtr obj,   System.String name);
   public void name_set( System.String name) { efl_name_set(handle, name); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   System.String efl_comment_get(System.IntPtr obj);
   public  System.String comment_get() { return efl_comment_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_comment_set(System.IntPtr obj,   System.String comment);
   public void comment_set( System.String comment) { efl_comment_set(handle, comment); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  int efl_event_global_freeze_count_get(System.IntPtr obj);
   public int event_global_freeze_count_get() { return efl_event_global_freeze_count_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  int efl_event_freeze_count_get(System.IntPtr obj);
   public int event_freeze_count_get() { return efl_event_freeze_count_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_finalized_get(System.IntPtr obj);
   public bool finalized_get() { return efl_finalized_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   System.IntPtr efl_key_data_get(System.IntPtr obj,   System.String key);
   public  System.IntPtr key_data_get( System.String key) { return efl_key_data_get(handle, key); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_key_data_set(System.IntPtr obj,   System.String key,   System.IntPtr data);
   public void key_data_set( System.String key,  System.IntPtr data) { efl_key_data_set(handle, key, data); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.Object efl_key_ref_get(System.IntPtr obj,   System.String key);
   public  efl.Object key_ref_get( System.String key) { return efl_key_ref_get(handle, key); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_key_ref_set(System.IntPtr obj,   System.String key,   efl.Object objdata);
   public void key_ref_set( System.String key,  efl.Object objdata) { efl_key_ref_set(handle, key, objdata); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.Object efl_key_wref_get(System.IntPtr obj,   System.String key);
   public  efl.Object key_wref_get( System.String key) { return efl_key_wref_get(handle, key); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_key_wref_set(System.IntPtr obj,   System.String key,   efl.Object objdata);
   public void key_wref_set( System.String key,  efl.Object objdata) { efl_key_wref_set(handle, key, objdata); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   int efl_key_value_get(System.IntPtr obj,   System.String key);
   public  int key_value_get( System.String key) { return efl_key_value_get(handle, key); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_key_value_set(System.IntPtr obj,   System.String key,   int value);
   public void key_value_set( System.String key,  int value) { efl_key_value_set(handle, key, value); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_del(System.IntPtr obj);
   public void del() { efl_del(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.Object efl_provider_find(System.IntPtr obj,   efl.Object klass);
   public  efl.Object provider_find( efl.Object klass) { return efl_provider_find(handle, klass); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.Object efl_constructor(System.IntPtr obj);
   public  efl.Object constructor() { return efl_constructor(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_destructor(System.IntPtr obj);
   public void destructor() { efl_destructor(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.Object efl_finalize(System.IntPtr obj);
   public  efl.Object finalize() { return efl_finalize(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.Object efl_name_find(System.IntPtr obj,   System.String search);
   public  efl.Object name_find( System.String search) { return efl_name_find(handle, search); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_wref_add(System.IntPtr obj,   efl.Object wref);
   public void wref_add( efl.Object wref) { efl_wref_add(handle, wref); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_wref_del(System.IntPtr obj,   efl.Object wref);
   public void wref_del( efl.Object wref) { efl_wref_del(handle, wref); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_event_thaw(System.IntPtr obj);
   public void event_thaw() { efl_event_thaw(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_event_freeze(System.IntPtr obj);
   public void event_freeze() { efl_event_freeze(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_event_global_thaw(System.IntPtr obj);
   public void event_global_thaw() { efl_event_global_thaw(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_event_global_freeze(System.IntPtr obj);
   public void event_global_freeze() { efl_event_global_freeze(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_event_callback_priority_add(System.IntPtr obj,  efl.kw_event.Description desc,  efl.Callback_Priority priority,  efl.Event_Cb cb,   System.IntPtr data);
   public bool event_callback_priority_add(efl.kw_event.Description desc, efl.Callback_Priority priority, efl.Event_Cb cb,  System.IntPtr data) { return efl_event_callback_priority_add(handle, desc, priority, cb, data); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_event_callback_del(System.IntPtr obj,  efl.kw_event.Description desc,  efl.Event_Cb func,   System.IntPtr user_data);
   public bool event_callback_del(efl.kw_event.Description desc, efl.Event_Cb func,  System.IntPtr user_data) { return efl_event_callback_del(handle, desc, func, user_data); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_event_callback_array_priority_add(System.IntPtr obj,  efl.Callback_Array_Item array,  efl.Callback_Priority priority,   System.IntPtr data);
   public bool event_callback_array_priority_add(efl.Callback_Array_Item array, efl.Callback_Priority priority,  System.IntPtr data) { return efl_event_callback_array_priority_add(handle, array, priority, data); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_event_callback_array_del(System.IntPtr obj,  efl.Callback_Array_Item array,   System.IntPtr user_data);
   public bool event_callback_array_del(efl.Callback_Array_Item array,  System.IntPtr user_data) { return efl_event_callback_array_del(handle, array, user_data); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_event_callback_call(System.IntPtr obj,  efl.kw_event.Description desc,   System.IntPtr event_info);
   public bool event_callback_call(efl.kw_event.Description desc,  System.IntPtr event_info) { return efl_event_callback_call(handle, desc, event_info); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_event_callback_legacy_call(System.IntPtr obj,  efl.kw_event.Description desc,   System.IntPtr event_info);
   public bool event_callback_legacy_call(efl.kw_event.Description desc,  System.IntPtr event_info) { return efl_event_callback_legacy_call(handle, desc, event_info); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_event_callback_stop(System.IntPtr obj);
   public void event_callback_stop() { efl_event_callback_stop(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_event_callback_forwarder_add(System.IntPtr obj,  efl.kw_event.Description desc,   efl.Object new_obj);
   public void event_callback_forwarder_add(efl.kw_event.Description desc,  efl.Object new_obj) { efl_event_callback_forwarder_add(handle, desc, new_obj); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_event_callback_forwarder_del(System.IntPtr obj,  efl.kw_event.Description desc,   efl.Object new_obj);
   public void event_callback_forwarder_del(efl.kw_event.Description desc,  efl.Object new_obj) { efl_event_callback_forwarder_del(handle, desc, new_obj); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_dbg_info_get(System.IntPtr obj,  efl.Dbg_Info root_node);
   public void dbg_info_get(efl.Dbg_Info root_node) { efl_dbg_info_get(handle, root_node); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   int efl_children_iterator_new(System.IntPtr obj);
   public  int children_iterator_new() { return efl_children_iterator_new(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_composite_attach(System.IntPtr obj,   efl.Object comp_obj);
   public bool composite_attach( efl.Object comp_obj) { return efl_composite_attach(handle, comp_obj); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_composite_detach(System.IntPtr obj,   efl.Object comp_obj);
   public bool composite_detach( efl.Object comp_obj) { return efl_composite_detach(handle, comp_obj); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_composite_part_is(System.IntPtr obj);
   public bool composite_part_is() { return efl_composite_part_is(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_future_link(System.IntPtr obj,   int link);
   public bool future_link( int link) { return efl_future_link(handle, link); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_group_need_recalculate_get(System.IntPtr obj);
   public bool group_need_recalculate_get() { return efl_canvas_group_need_recalculate_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_group_need_recalculate_set(System.IntPtr obj,  bool value);
   public void group_need_recalculate_set(bool value) { efl_canvas_group_need_recalculate_set(handle, value); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_group_change(System.IntPtr obj);
   public void group_change() { efl_canvas_group_change(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_group_calculate(System.IntPtr obj);
   public void group_calculate() { efl_canvas_group_calculate(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   int efl_canvas_group_children_iterate(System.IntPtr obj);
   public  int group_children_iterate() { return efl_canvas_group_children_iterate(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_group_member_add(System.IntPtr obj,   efl.canvas.Object sub_obj);
   public void group_member_add( efl.canvas.Object sub_obj) { efl_canvas_group_member_add(handle, sub_obj); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_group_member_del(System.IntPtr obj,   efl.canvas.Object sub_obj);
   public void group_member_del( efl.canvas.Object sub_obj) { efl_canvas_group_member_del(handle, sub_obj); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_group_add(System.IntPtr obj);
   public void group_add() { efl_canvas_group_add(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_group_del(System.IntPtr obj);
   public void group_del() { efl_canvas_group_del(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_type_set(System.IntPtr obj,   System.String type);
   public void type_set( System.String type) { efl_canvas_object_type_set(handle, type); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  efl.input.Object_Pointer_Mode efl_canvas_object_pointer_mode_get(System.IntPtr obj);
   public efl.input.Object_Pointer_Mode pointer_mode_get() { return efl_canvas_object_pointer_mode_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_pointer_mode_set(System.IntPtr obj,  efl.input.Object_Pointer_Mode pointer_mode);
   public void pointer_mode_set(efl.input.Object_Pointer_Mode pointer_mode) { efl_canvas_object_pointer_mode_set(handle, pointer_mode); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  efl.gfx.Render_Op efl_canvas_object_render_op_get(System.IntPtr obj);
   public efl.gfx.Render_Op render_op_get() { return efl_canvas_object_render_op_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_render_op_set(System.IntPtr obj,  efl.gfx.Render_Op render_op);
   public void render_op_set(efl.gfx.Render_Op render_op) { efl_canvas_object_render_op_set(handle, render_op); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_freeze_events_get(System.IntPtr obj);
   public bool freeze_events_get() { return efl_canvas_object_freeze_events_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_freeze_events_set(System.IntPtr obj,  bool freeze);
   public void freeze_events_set(bool freeze) { efl_canvas_object_freeze_events_set(handle, freeze); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.canvas.Object efl_canvas_object_clip_get(System.IntPtr obj);
   public  efl.canvas.Object clip_get() { return efl_canvas_object_clip_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_clip_set(System.IntPtr obj,   efl.canvas.Object clip);
   public void clip_set( efl.canvas.Object clip) { efl_canvas_object_clip_set(handle, clip); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_repeat_events_get(System.IntPtr obj);
   public bool repeat_events_get() { return efl_canvas_object_repeat_events_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_repeat_events_set(System.IntPtr obj,  bool repeat);
   public void repeat_events_set(bool repeat) { efl_canvas_object_repeat_events_set(handle, repeat); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  double efl_canvas_object_scale_get(System.IntPtr obj);
   public double scale_get() { return efl_canvas_object_scale_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_scale_set(System.IntPtr obj,  double scale);
   public void scale_set(double scale) { efl_canvas_object_scale_set(handle, scale); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_key_focus_get(System.IntPtr obj);
   public bool key_focus_get() { return efl_canvas_object_key_focus_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_key_focus_set(System.IntPtr obj,  bool focus);
   public void key_focus_set(bool focus) { efl_canvas_object_key_focus_set(handle, focus); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_is_frame_object_get(System.IntPtr obj);
   public bool is_frame_object_get() { return efl_canvas_object_is_frame_object_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_is_frame_object_set(System.IntPtr obj,  bool is_frame);
   public void is_frame_object_set(bool is_frame) { efl_canvas_object_is_frame_object_set(handle, is_frame); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_precise_is_inside_get(System.IntPtr obj);
   public bool precise_is_inside_get() { return efl_canvas_object_precise_is_inside_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_precise_is_inside_set(System.IntPtr obj,  bool precise);
   public void precise_is_inside_set(bool precise) { efl_canvas_object_precise_is_inside_set(handle, precise); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_propagate_events_get(System.IntPtr obj);
   public bool propagate_events_get() { return efl_canvas_object_propagate_events_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_propagate_events_set(System.IntPtr obj,  bool propagate);
   public void propagate_events_set(bool propagate) { efl_canvas_object_propagate_events_set(handle, propagate); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_pass_events_get(System.IntPtr obj);
   public bool pass_events_get() { return efl_canvas_object_pass_events_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_pass_events_set(System.IntPtr obj,  bool pass);
   public void pass_events_set(bool pass) { efl_canvas_object_pass_events_set(handle, pass); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_anti_alias_get(System.IntPtr obj);
   public bool anti_alias_get() { return efl_canvas_object_anti_alias_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_anti_alias_set(System.IntPtr obj,  bool anti_alias);
   public void anti_alias_set(bool anti_alias) { efl_canvas_object_anti_alias_set(handle, anti_alias); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   int efl_canvas_object_clipees_get(System.IntPtr obj);
   public  int clipees_get() { return efl_canvas_object_clipees_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.canvas.Object efl_canvas_object_render_parent_get(System.IntPtr obj);
   public  efl.canvas.Object render_parent_get() { return efl_canvas_object_render_parent_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  efl.text.Bidirectional_Type efl_canvas_object_paragraph_direction_get(System.IntPtr obj);
   public efl.text.Bidirectional_Type paragraph_direction_get() { return efl_canvas_object_paragraph_direction_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_paragraph_direction_set(System.IntPtr obj,  efl.text.Bidirectional_Type dir);
   public void paragraph_direction_set(efl.text.Bidirectional_Type dir) { efl_canvas_object_paragraph_direction_set(handle, dir); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_no_render_get(System.IntPtr obj);
   public bool no_render_get() { return efl_canvas_object_no_render_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_no_render_set(System.IntPtr obj,  bool enable);
   public void no_render_set(bool enable) { efl_canvas_object_no_render_set(handle, enable); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_legacy_ctor(System.IntPtr obj);
   public void legacy_ctor() { efl_canvas_object_legacy_ctor(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_clipees_has(System.IntPtr obj);
   public bool clipees_has() { return efl_canvas_object_clipees_has(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_canvas_object_key_grab(System.IntPtr obj,   System.String keyname,  evas.Modifier_Mask modifiers,  evas.Modifier_Mask not_modifiers,  bool exclusive);
   public bool key_grab( System.String keyname, evas.Modifier_Mask modifiers, evas.Modifier_Mask not_modifiers, bool exclusive) { return efl_canvas_object_key_grab(handle, keyname, modifiers, not_modifiers, exclusive); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_canvas_object_key_ungrab(System.IntPtr obj,   System.String keyname,  evas.Modifier_Mask modifiers,  evas.Modifier_Mask not_modifiers);
   public void key_ungrab( System.String keyname, evas.Modifier_Mask modifiers, evas.Modifier_Mask not_modifiers) { efl_canvas_object_key_ungrab(handle, keyname, modifiers, not_modifiers); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_enable_get(System.IntPtr obj);
   public bool map_enable_get() { return efl_gfx_map_enable_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_enable_set(System.IntPtr obj,  bool enabled);
   public void map_enable_set(bool enabled) { efl_gfx_map_enable_set(handle, enabled); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_clockwise_get(System.IntPtr obj);
   public bool map_clockwise_get() { return efl_gfx_map_clockwise_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_smooth_get(System.IntPtr obj);
   public bool map_smooth_get() { return efl_gfx_map_smooth_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_smooth_set(System.IntPtr obj,  bool smooth);
   public void map_smooth_set(bool smooth) { efl_gfx_map_smooth_set(handle, smooth); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_alpha_get(System.IntPtr obj);
   public bool map_alpha_get() { return efl_gfx_map_alpha_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_alpha_set(System.IntPtr obj,  bool alpha);
   public void map_alpha_set(bool alpha) { efl_gfx_map_alpha_set(handle, alpha); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_point_coord_get(System.IntPtr obj,  int idx,  double x,  double y,  double z);
   public void map_point_coord_get(int idx, double x, double y, double z) { efl_gfx_map_point_coord_get(handle, idx, x, y, z); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_point_coord_set(System.IntPtr obj,  int idx,  double x,  double y,  double z);
   public void map_point_coord_set(int idx, double x, double y, double z) { efl_gfx_map_point_coord_set(handle, idx, x, y, z); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_point_image_uv_get(System.IntPtr obj,  int idx,  double u,  double v);
   public void map_point_image_uv_get(int idx, double u, double v) { efl_gfx_map_point_image_uv_get(handle, idx, u, v); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_point_image_uv_set(System.IntPtr obj,  int idx,  double u,  double v);
   public void map_point_image_uv_set(int idx, double u, double v) { efl_gfx_map_point_image_uv_set(handle, idx, u, v); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_color_get(System.IntPtr obj,  int idx,  int r,  int g,  int b,  int a);
   public void map_color_get(int idx, int r, int g, int b, int a) { efl_gfx_map_color_get(handle, idx, r, g, b, a); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_color_set(System.IntPtr obj,  int idx,  int r,  int g,  int b,  int a);
   public void map_color_set(int idx, int r, int g, int b, int a) { efl_gfx_map_color_set(handle, idx, r, g, b, a); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_move_sync_get(System.IntPtr obj);
   public bool map_move_sync_get() { return efl_gfx_map_move_sync_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_map_move_sync_set(System.IntPtr obj,  bool enable);
   public void map_move_sync_set(bool enable) { efl_gfx_map_move_sync_set(handle, enable); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  double efl_gfx_map_point_z_get(System.IntPtr obj,  int idx);
   public double map_point_z_get(int idx) { return efl_gfx_map_point_z_get(handle, idx); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_populate(System.IntPtr obj,  double z);
   public bool map_populate(double z) { return efl_gfx_map_populate(handle, z); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_populate_manual(System.IntPtr obj,  double x,  double y,  double w,  double h,  double z);
   public bool map_populate_manual(double x, double y, double w, double h, double z) { return efl_gfx_map_populate_manual(handle, x, y, w, h, z); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_rotate(System.IntPtr obj,  double degrees,  double cx,  double cy);
   public bool map_rotate(double degrees, double cx, double cy) { return efl_gfx_map_rotate(handle, degrees, cx, cy); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_zoom(System.IntPtr obj,  double zoomx,  double zoomy,  double cx,  double cy);
   public bool map_zoom(double zoomx, double zoomy, double cx, double cy) { return efl_gfx_map_zoom(handle, zoomx, zoomy, cx, cy); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_rotate_3d(System.IntPtr obj,  double dx,  double dy,  double dz,  double cx,  double cy,  double cz);
   public bool map_rotate_3d(double dx, double dy, double dz, double cx, double cy, double cz) { return efl_gfx_map_rotate_3d(handle, dx, dy, dz, cx, cy, cz); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_quat_rotate(System.IntPtr obj,  double qx,  double qy,  double qz,  double qw,  double cx,  double cy,  double cz);
   public bool map_quat_rotate(double qx, double qy, double qz, double qw, double cx, double cy, double cz) { return efl_gfx_map_quat_rotate(handle, qx, qy, qz, qw, cx, cy, cz); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_lightning_3d(System.IntPtr obj,  double lx,  double ly,  double lz,  int lr,  int lg,  int lb,  int ar,  int ag,  int ab);
   public bool map_lightning_3d(double lx, double ly, double lz, int lr, int lg, int lb, int ar, int ag, int ab) { return efl_gfx_map_lightning_3d(handle, lx, ly, lz, lr, lg, lb, ar, ag, ab); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_perspective_3d(System.IntPtr obj,  double px,  double py,  double z0,  double foc);
   public bool map_perspective_3d(double px, double py, double z0, double foc) { return efl_gfx_map_perspective_3d(handle, px, py, z0, foc); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_gfx_map_dup(System.IntPtr obj,   efl.gfx.Map other);
   public bool map_dup( efl.gfx.Map other) { return efl_gfx_map_dup(handle, other); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  short efl_gfx_stack_layer_get(System.IntPtr obj);
   public short layer_get() { return efl_gfx_stack_layer_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_stack_layer_set(System.IntPtr obj,  short l);
   public void layer_set(short l) { efl_gfx_stack_layer_set(handle, l); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.gfx.Stack efl_gfx_stack_below_get(System.IntPtr obj);
   public  efl.gfx.Stack below_get() { return efl_gfx_stack_below_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   efl.gfx.Stack efl_gfx_stack_above_get(System.IntPtr obj);
   public  efl.gfx.Stack above_get() { return efl_gfx_stack_above_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_stack_below(System.IntPtr obj,   efl.gfx.Stack below);
   public void stack_below( efl.gfx.Stack below) { efl_gfx_stack_below(handle, below); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_stack_raise(System.IntPtr obj);
   public void raise() { efl_gfx_stack_raise(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_stack_above(System.IntPtr obj,   efl.gfx.Stack above);
   public void stack_above( efl.gfx.Stack above) { efl_gfx_stack_above(handle, above); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_stack_lower(System.IntPtr obj);
   public void lower() { efl_gfx_stack_lower(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_base_get(System.IntPtr obj,  int w,  int h);
   public void hint_base_get(int w, int h) { efl_gfx_size_hint_base_get(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_base_set(System.IntPtr obj,  int w,  int h);
   public void hint_base_set(int w, int h) { efl_gfx_size_hint_base_set(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_step_get(System.IntPtr obj,  int w,  int h);
   public void hint_step_get(int w, int h) { efl_gfx_size_hint_step_get(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_step_set(System.IntPtr obj,  int w,  int h);
   public void hint_step_set(int w, int h) { efl_gfx_size_hint_step_set(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_aspect_get(System.IntPtr obj,  efl.gfx.size.hint.Aspect mode,  int w,  int h);
   public void hint_aspect_get(efl.gfx.size.hint.Aspect mode, int w, int h) { efl_gfx_size_hint_aspect_get(handle, mode, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_aspect_set(System.IntPtr obj,  efl.gfx.size.hint.Aspect mode,  int w,  int h);
   public void hint_aspect_set(efl.gfx.size.hint.Aspect mode, int w, int h) { efl_gfx_size_hint_aspect_set(handle, mode, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_max_get(System.IntPtr obj,  int w,  int h);
   public void hint_max_get(int w, int h) { efl_gfx_size_hint_max_get(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_max_set(System.IntPtr obj,  int w,  int h);
   public void hint_max_set(int w, int h) { efl_gfx_size_hint_max_set(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_min_get(System.IntPtr obj,  int w,  int h);
   public void hint_min_get(int w, int h) { efl_gfx_size_hint_min_get(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_min_set(System.IntPtr obj,  int w,  int h);
   public void hint_min_set(int w, int h) { efl_gfx_size_hint_min_set(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_request_get(System.IntPtr obj,  int w,  int h);
   public void hint_request_get(int w, int h) { efl_gfx_size_hint_request_get(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_request_set(System.IntPtr obj,  int w,  int h);
   public void hint_request_set(int w, int h) { efl_gfx_size_hint_request_set(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_restricted_min_get(System.IntPtr obj,  int w,  int h);
   public void hint_restricted_min_get(int w, int h) { efl_gfx_size_hint_restricted_min_get(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_restricted_min_set(System.IntPtr obj,  int w,  int h);
   public void hint_restricted_min_set(int w, int h) { efl_gfx_size_hint_restricted_min_set(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_combined_min_get(System.IntPtr obj,  int w,  int h);
   public void hint_combined_min_get(int w, int h) { efl_gfx_size_hint_combined_min_get(handle, w, h); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_margin_get(System.IntPtr obj,  int l,  int r,  int t,  int b);
   public void hint_margin_get(int l, int r, int t, int b) { efl_gfx_size_hint_margin_get(handle, l, r, t, b); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_margin_set(System.IntPtr obj,  int l,  int r,  int t,  int b);
   public void hint_margin_set(int l, int r, int t, int b) { efl_gfx_size_hint_margin_set(handle, l, r, t, b); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_weight_get(System.IntPtr obj,  double x,  double y);
   public void hint_weight_get(double x, double y) { efl_gfx_size_hint_weight_get(handle, x, y); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_weight_set(System.IntPtr obj,  double x,  double y);
   public void hint_weight_set(double x, double y) { efl_gfx_size_hint_weight_set(handle, x, y); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_align_get(System.IntPtr obj,  double x,  double y);
   public void hint_align_get(double x, double y) { efl_gfx_size_hint_align_get(handle, x, y); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_align_set(System.IntPtr obj,  double x,  double y);
   public void hint_align_set(double x, double y) { efl_gfx_size_hint_align_set(handle, x, y); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  efl.gfx.size.hint.Mode efl_gfx_size_hint_display_mode_get(System.IntPtr obj);
   public efl.gfx.size.hint.Mode hint_display_mode_get() { return efl_gfx_size_hint_display_mode_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_gfx_size_hint_display_mode_set(System.IntPtr obj,  efl.gfx.size.hint.Mode dispmode);
   public void hint_display_mode_set(efl.gfx.size.hint.Mode dispmode) { efl_gfx_size_hint_display_mode_set(handle, dispmode); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  void efl_input_pointer_xy_get(System.IntPtr obj,  int x,  int y);
   public void pointer_xy_get(int x, int y) { efl_input_pointer_xy_get(handle, x, y); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern  bool efl_input_pointer_inside_get(System.IntPtr obj);
   public bool pointer_inside_get() { return efl_input_pointer_inside_get(handle); }
   [System.Runtime.InteropServices.DllImport("evas")] static extern   int efl_input_pointer_iterate(System.IntPtr obj,  bool hover);
   public  int pointer_iterate(bool hover) { return efl_input_pointer_iterate(handle, hover); }
}

public class MyBox : BoxInherit
{
    public MyBox(efl.Object parent) : base(parent) {}
}

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();
    [DllImport("evas")] static extern void evas_init();

    static void Main(string[] args)
    {
        efl_object_init();
        ecore_init();
        evas_init();
        
        efl.Loop loop = new efl.LoopConcrete();

        EcoreEvas ecore_evas = new EcoreEvas();
        
        System.Console.WriteLine("Hello World");

        efl.canvas.Object canvas = ecore_evas.canvas;
        canvas.visible_set(true);

        efl.canvas.Rectangle rect = new efl.canvas.RectangleConcrete(canvas);
        rect.color_set(255, 255, 255, 255);
        rect.size_set(640, 480);
        rect.visible_set(true);

        // evas.Box box = new evas.BoxConcrete(canvas);
        evas.Box box = new MyBox(canvas);
        // rect.position_set(160, 120);
        rect.size_set(320, 240);
        box.visible_set(true);
        
        efl.canvas.Image image1 = new efl.canvas.ImageConcrete(canvas);
        image1.file_set("/home/felipe/dev/samsung/upstream/efl/src/examples/elementary/sphere_hunter/score.jpg", "");
        image1.hint_min_set(160, 120);
        image1.visible_set(true);

        efl.canvas.Image image2 = new efl.canvas.ImageConcrete(canvas);
        image2.file_set("/home/felipe/dev/samsung/upstream/efl/src/examples/evas/shooter/assets/images/bricks.jpg", "");
        image2.hint_min_set(160, 120);
        image2.visible_set(true);
        
        box.append(image1);
        box.append(image2);
        
        loop.begin();
    }
}
