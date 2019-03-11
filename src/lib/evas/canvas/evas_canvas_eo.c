
void _evas_canvas_image_cache_set(Eo *obj, Evas_Public_Data *pd, int size);


static Eina_Error
__eolian_evas_canvas_image_cache_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   evas_canvas_image_cache_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_image_cache_set, EFL_FUNC_CALL(size), int size);

int _evas_canvas_image_cache_get(const Eo *obj, Evas_Public_Data *pd);


static Eina_Value
__eolian_evas_canvas_image_cache_get_reflect(const Eo *obj)
{
   int val = evas_canvas_image_cache_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_image_cache_get, int, 0);

void _evas_canvas_event_default_flags_set(Eo *obj, Evas_Public_Data *pd, Efl_Input_Flags flags);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_event_default_flags_set, EFL_FUNC_CALL(flags), Efl_Input_Flags flags);

Efl_Input_Flags _evas_canvas_event_default_flags_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_event_default_flags_get, Efl_Input_Flags, 0);

void _evas_canvas_font_cache_set(Eo *obj, Evas_Public_Data *pd, int size);


static Eina_Error
__eolian_evas_canvas_font_cache_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   evas_canvas_font_cache_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_font_cache_set, EFL_FUNC_CALL(size), int size);

int _evas_canvas_font_cache_get(const Eo *obj, Evas_Public_Data *pd);


static Eina_Value
__eolian_evas_canvas_font_cache_get_reflect(const Eo *obj)
{
   int val = evas_canvas_font_cache_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_font_cache_get, int, 0);

void _evas_canvas_data_attach_set(Eo *obj, Evas_Public_Data *pd, void *data);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_data_attach_set, EFL_FUNC_CALL(data), void *data);

void *_evas_canvas_data_attach_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_data_attach_get, void *, NULL);

Efl_Canvas_Object *_evas_canvas_focus_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_focus_get, Efl_Canvas_Object *, NULL);

Efl_Canvas_Object *_evas_canvas_seat_focus_get(const Eo *obj, Evas_Public_Data *pd, Efl_Input_Device *seat);

EOAPI EFL_FUNC_BODYV_CONST(evas_canvas_seat_focus_get, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(seat), Efl_Input_Device *seat);

Efl_Canvas_Object *_evas_canvas_object_top_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_object_top_get, Efl_Canvas_Object *, NULL);

void _evas_canvas_pointer_canvas_xy_by_device_get(const Eo *obj, Evas_Public_Data *pd, Efl_Input_Device *dev, int *x, int *y);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_canvas_pointer_canvas_xy_by_device_get, EFL_FUNC_CALL(dev, x, y), Efl_Input_Device *dev, int *x, int *y);

void _evas_canvas_pointer_canvas_xy_get(const Eo *obj, Evas_Public_Data *pd, int *x, int *y);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_canvas_pointer_canvas_xy_get, EFL_FUNC_CALL(x, y), int *x, int *y);

int _evas_canvas_event_down_count_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_event_down_count_get, int, 0);

int _evas_canvas_smart_objects_calculate_count_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_smart_objects_calculate_count_get, int, 0);

Eina_Bool _evas_canvas_focus_state_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_focus_state_get, Eina_Bool, 0);

Eina_Bool _evas_canvas_seat_focus_state_get(const Eo *obj, Evas_Public_Data *pd, Efl_Input_Device *seat);

EOAPI EFL_FUNC_BODYV_CONST(evas_canvas_seat_focus_state_get, Eina_Bool, 0, EFL_FUNC_CALL(seat), Efl_Input_Device *seat);

Eina_Bool _evas_canvas_changed_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_changed_get, Eina_Bool, 0);

void _evas_canvas_pointer_output_xy_by_device_get(const Eo *obj, Evas_Public_Data *pd, Efl_Input_Device *dev, int *x, int *y);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_canvas_pointer_output_xy_by_device_get, EFL_FUNC_CALL(dev, x, y), Efl_Input_Device *dev, int *x, int *y);

void _evas_canvas_pointer_output_xy_get(const Eo *obj, Evas_Public_Data *pd, int *x, int *y);

EOAPI EFL_VOID_FUNC_BODYV_CONST(evas_canvas_pointer_output_xy_get, EFL_FUNC_CALL(x, y), int *x, int *y);

Efl_Canvas_Object *_evas_canvas_object_bottom_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_object_bottom_get, Efl_Canvas_Object *, NULL);

unsigned int _evas_canvas_pointer_button_down_mask_by_device_get(const Eo *obj, Evas_Public_Data *pd, Efl_Input_Device *dev);

EOAPI EFL_FUNC_BODYV_CONST(evas_canvas_pointer_button_down_mask_by_device_get, unsigned int, 0, EFL_FUNC_CALL(dev), Efl_Input_Device *dev);

unsigned int _evas_canvas_pointer_button_down_mask_get(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_pointer_button_down_mask_get, unsigned int, 0);

Eina_List *_evas_canvas_tree_objects_at_xy_get(Eo *obj, Evas_Public_Data *pd, Efl_Canvas_Object *stop, int x, int y);

EOAPI EFL_FUNC_BODYV(evas_canvas_tree_objects_at_xy_get, Eina_List *, NULL, EFL_FUNC_CALL(stop, x, y), Efl_Canvas_Object *stop, int x, int y);

void _evas_canvas_key_lock_on(Eo *obj, Evas_Public_Data *pd, const char *keyname);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_key_lock_on, EFL_FUNC_CALL(keyname), const char *keyname);

void _evas_canvas_seat_key_lock_on(Eo *obj, Evas_Public_Data *pd, const char *keyname, Efl_Input_Device *seat);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_seat_key_lock_on, EFL_FUNC_CALL(keyname, seat), const char *keyname, Efl_Input_Device *seat);

void _evas_canvas_seat_key_lock_off(Eo *obj, Evas_Public_Data *pd, const char *keyname, Efl_Input_Device *seat);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_seat_key_lock_off, EFL_FUNC_CALL(keyname, seat), const char *keyname, Efl_Input_Device *seat);

void _evas_canvas_key_modifier_add(Eo *obj, Evas_Public_Data *pd, const char *keyname);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_key_modifier_add, EFL_FUNC_CALL(keyname), const char *keyname);

void _evas_canvas_key_modifier_off(Eo *obj, Evas_Public_Data *pd, const char *keyname);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_key_modifier_off, EFL_FUNC_CALL(keyname), const char *keyname);

Eina_Bool _evas_canvas_render_async(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY(evas_canvas_render_async, Eina_Bool, 0);

void _evas_canvas_focus_out(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_focus_out);

void _evas_canvas_norender(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_norender);

void _evas_canvas_nochange_pop(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_nochange_pop);

void _evas_canvas_key_lock_off(Eo *obj, Evas_Public_Data *pd, const char *keyname);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_key_lock_off, EFL_FUNC_CALL(keyname), const char *keyname);

void _evas_canvas_nochange_push(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_nochange_push);

void _evas_canvas_font_cache_flush(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_font_cache_flush);

void _evas_canvas_key_modifier_on(Eo *obj, Evas_Public_Data *pd, const char *keyname);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_key_modifier_on, EFL_FUNC_CALL(keyname), const char *keyname);

void _evas_canvas_seat_key_modifier_on(Eo *obj, Evas_Public_Data *pd, const char *keyname, Efl_Input_Device *seat);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_seat_key_modifier_on, EFL_FUNC_CALL(keyname, seat), const char *keyname, Efl_Input_Device *seat);

void _evas_canvas_seat_key_modifier_off(Eo *obj, Evas_Public_Data *pd, const char *keyname, Efl_Input_Device *seat);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_seat_key_modifier_off, EFL_FUNC_CALL(keyname, seat), const char *keyname, Efl_Input_Device *seat);

Eina_List *_evas_canvas_font_available_list(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_font_available_list, Eina_List *, NULL);

Efl_Canvas_Object *_evas_canvas_object_name_find(const Eo *obj, Evas_Public_Data *pd, const char *name);

EOAPI EFL_FUNC_BODYV_CONST(evas_canvas_object_name_find, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(name), const char *name);

void _evas_canvas_font_path_append(Eo *obj, Evas_Public_Data *pd, const char *path);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_font_path_append, EFL_FUNC_CALL(path), const char *path);

void _evas_canvas_font_path_clear(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_font_path_clear);

void _evas_canvas_touch_point_list_nth_xy_get(Eo *obj, Evas_Public_Data *pd, unsigned int n, double *x, double *y);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_touch_point_list_nth_xy_get, EFL_FUNC_CALL(n, x, y), unsigned int n, double *x, double *y);

void _evas_canvas_key_lock_del(Eo *obj, Evas_Public_Data *pd, const char *keyname);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_key_lock_del, EFL_FUNC_CALL(keyname), const char *keyname);

void _evas_canvas_damage_rectangle_add(Eo *obj, Evas_Public_Data *pd, int x, int y, int w, int h);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_damage_rectangle_add, EFL_FUNC_CALL(x, y, w, h), int x, int y, int w, int h);

void _evas_canvas_sync(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_sync);

const Eina_List *_evas_canvas_font_path_list(const Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(evas_canvas_font_path_list, const Eina_List *, NULL);

void _evas_canvas_image_cache_reload(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_image_cache_reload);

int _evas_canvas_coord_world_x_to_screen(const Eo *obj, Evas_Public_Data *pd, int x);

EOAPI EFL_FUNC_BODYV_CONST(evas_canvas_coord_world_x_to_screen, int, 0, EFL_FUNC_CALL(x), int x);

Eina_List *_evas_canvas_render_updates(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_FUNC_BODY(evas_canvas_render_updates, Eina_List *, NULL);

void _evas_canvas_image_cache_flush(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_image_cache_flush);

int _evas_canvas_coord_screen_y_to_world(const Eo *obj, Evas_Public_Data *pd, int y);

EOAPI EFL_FUNC_BODYV_CONST(evas_canvas_coord_screen_y_to_world, int, 0, EFL_FUNC_CALL(y), int y);

void _evas_canvas_key_modifier_del(Eo *obj, Evas_Public_Data *pd, const char *keyname);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_key_modifier_del, EFL_FUNC_CALL(keyname), const char *keyname);

void _evas_canvas_focus_in(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_focus_in);

void _evas_canvas_seat_focus_in(Eo *obj, Evas_Public_Data *pd, Efl_Input_Device *seat);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_seat_focus_in, EFL_FUNC_CALL(seat), Efl_Input_Device *seat);

void _evas_canvas_seat_focus_out(Eo *obj, Evas_Public_Data *pd, Efl_Input_Device *seat);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_seat_focus_out, EFL_FUNC_CALL(seat), Efl_Input_Device *seat);

void _evas_canvas_obscured_rectangle_add(Eo *obj, Evas_Public_Data *pd, int x, int y, int w, int h);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_obscured_rectangle_add, EFL_FUNC_CALL(x, y, w, h), int x, int y, int w, int h);

void _evas_canvas_render_dump(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_render_dump);

void _evas_canvas_render(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_render);

void _evas_canvas_font_path_prepend(Eo *obj, Evas_Public_Data *pd, const char *path);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_font_path_prepend, EFL_FUNC_CALL(path), const char *path);

void _evas_canvas_obscured_clear(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_obscured_clear);

int _evas_canvas_coord_screen_x_to_world(const Eo *obj, Evas_Public_Data *pd, int x);

EOAPI EFL_FUNC_BODYV_CONST(evas_canvas_coord_screen_x_to_world, int, 0, EFL_FUNC_CALL(x), int x);

void _evas_canvas_key_lock_add(Eo *obj, Evas_Public_Data *pd, const char *keyname);

EOAPI EFL_VOID_FUNC_BODYV(evas_canvas_key_lock_add, EFL_FUNC_CALL(keyname), const char *keyname);

void _evas_canvas_render_idle_flush(Eo *obj, Evas_Public_Data *pd);

EOAPI EFL_VOID_FUNC_BODY(evas_canvas_render_idle_flush);

Efl_Input_Device *_evas_canvas_default_device_get(const Eo *obj, Evas_Public_Data *pd, Efl_Input_Device_Type type);

EOAPI EFL_FUNC_BODYV_CONST(evas_canvas_default_device_get, Efl_Input_Device *, NULL, EFL_FUNC_CALL(type), Efl_Input_Device_Type type);

int _evas_canvas_coord_world_y_to_screen(const Eo *obj, Evas_Public_Data *pd, int y);

EOAPI EFL_FUNC_BODYV_CONST(evas_canvas_coord_world_y_to_screen, int, 0, EFL_FUNC_CALL(y), int y);

Efl_Object *_evas_canvas_efl_object_constructor(Eo *obj, Evas_Public_Data *pd);


void _evas_canvas_efl_object_destructor(Eo *obj, Evas_Public_Data *pd);


void _evas_canvas_efl_object_invalidate(Eo *obj, Evas_Public_Data *pd);


void _evas_canvas_efl_object_event_thaw(Eo *obj, Evas_Public_Data *pd);


void _evas_canvas_efl_object_event_freeze(Eo *obj, Evas_Public_Data *pd);


Efl_Object *_evas_canvas_efl_object_provider_find(const Eo *obj, Evas_Public_Data *pd, const Efl_Class *klass);


Efl_Loop *_evas_canvas_efl_loop_consumer_loop_get(const Eo *obj, Evas_Public_Data *pd);


Eina_Bool _evas_canvas_efl_canvas_pointer_pointer_inside_get(const Eo *obj, Evas_Public_Data *pd, Efl_Input_Device *seat);


Efl_Input_Device *_evas_canvas_efl_canvas_scene_device_get(const Eo *obj, Evas_Public_Data *pd, const char *name);


Efl_Input_Device *_evas_canvas_efl_canvas_scene_seat_get(const Eo *obj, Evas_Public_Data *pd, int id);


Efl_Input_Device *_evas_canvas_efl_canvas_scene_seat_default_get(const Eo *obj, Evas_Public_Data *pd);


Eina_Bool _evas_canvas_efl_canvas_scene_pointer_position_get(const Eo *obj, Evas_Public_Data *pd, Efl_Input_Device *seat, Eina_Position2D *pos);


Eina_Bool _evas_canvas_efl_canvas_scene_image_max_size_get(const Eo *obj, Evas_Public_Data *pd, Eina_Size2D *max);


Eina_Iterator *_evas_canvas_efl_canvas_scene_objects_at_xy_get(Eo *obj, Evas_Public_Data *pd, Eina_Position2D pos, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects);


Efl_Gfx_Entity *_evas_canvas_efl_canvas_scene_object_top_at_xy_get(const Eo *obj, Evas_Public_Data *pd, Eina_Position2D pos, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects);


Eina_Iterator *_evas_canvas_efl_canvas_scene_objects_in_rectangle_get(Eo *obj, Evas_Public_Data *pd, Eina_Rect rect, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects);


Efl_Gfx_Entity *_evas_canvas_efl_canvas_scene_object_top_in_rectangle_get(const Eo *obj, Evas_Public_Data *pd, Eina_Rect rect, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects);


void _evas_canvas_efl_canvas_scene_group_objects_calculate(Eo *obj, Evas_Public_Data *pd);


Eina_Bool _evas_canvas_efl_canvas_scene_group_objects_calculating_get(const Eo *obj, Evas_Public_Data *pd);


static Eina_Bool
_evas_canvas_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef EVAS_CANVAS_EXTRA_OPS
#define EVAS_CANVAS_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(evas_canvas_image_cache_set, _evas_canvas_image_cache_set),
      EFL_OBJECT_OP_FUNC(evas_canvas_image_cache_get, _evas_canvas_image_cache_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_event_default_flags_set, _evas_canvas_event_default_flags_set),
      EFL_OBJECT_OP_FUNC(evas_canvas_event_default_flags_get, _evas_canvas_event_default_flags_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_font_cache_set, _evas_canvas_font_cache_set),
      EFL_OBJECT_OP_FUNC(evas_canvas_font_cache_get, _evas_canvas_font_cache_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_data_attach_set, _evas_canvas_data_attach_set),
      EFL_OBJECT_OP_FUNC(evas_canvas_data_attach_get, _evas_canvas_data_attach_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_focus_get, _evas_canvas_focus_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_seat_focus_get, _evas_canvas_seat_focus_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_object_top_get, _evas_canvas_object_top_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_pointer_canvas_xy_by_device_get, _evas_canvas_pointer_canvas_xy_by_device_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_pointer_canvas_xy_get, _evas_canvas_pointer_canvas_xy_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_event_down_count_get, _evas_canvas_event_down_count_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_smart_objects_calculate_count_get, _evas_canvas_smart_objects_calculate_count_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_focus_state_get, _evas_canvas_focus_state_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_seat_focus_state_get, _evas_canvas_seat_focus_state_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_changed_get, _evas_canvas_changed_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_pointer_output_xy_by_device_get, _evas_canvas_pointer_output_xy_by_device_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_pointer_output_xy_get, _evas_canvas_pointer_output_xy_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_object_bottom_get, _evas_canvas_object_bottom_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_pointer_button_down_mask_by_device_get, _evas_canvas_pointer_button_down_mask_by_device_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_pointer_button_down_mask_get, _evas_canvas_pointer_button_down_mask_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_tree_objects_at_xy_get, _evas_canvas_tree_objects_at_xy_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_key_lock_on, _evas_canvas_key_lock_on),
      EFL_OBJECT_OP_FUNC(evas_canvas_seat_key_lock_on, _evas_canvas_seat_key_lock_on),
      EFL_OBJECT_OP_FUNC(evas_canvas_seat_key_lock_off, _evas_canvas_seat_key_lock_off),
      EFL_OBJECT_OP_FUNC(evas_canvas_key_modifier_add, _evas_canvas_key_modifier_add),
      EFL_OBJECT_OP_FUNC(evas_canvas_key_modifier_off, _evas_canvas_key_modifier_off),
      EFL_OBJECT_OP_FUNC(evas_canvas_render_async, _evas_canvas_render_async),
      EFL_OBJECT_OP_FUNC(evas_canvas_focus_out, _evas_canvas_focus_out),
      EFL_OBJECT_OP_FUNC(evas_canvas_norender, _evas_canvas_norender),
      EFL_OBJECT_OP_FUNC(evas_canvas_nochange_pop, _evas_canvas_nochange_pop),
      EFL_OBJECT_OP_FUNC(evas_canvas_key_lock_off, _evas_canvas_key_lock_off),
      EFL_OBJECT_OP_FUNC(evas_canvas_nochange_push, _evas_canvas_nochange_push),
      EFL_OBJECT_OP_FUNC(evas_canvas_font_cache_flush, _evas_canvas_font_cache_flush),
      EFL_OBJECT_OP_FUNC(evas_canvas_key_modifier_on, _evas_canvas_key_modifier_on),
      EFL_OBJECT_OP_FUNC(evas_canvas_seat_key_modifier_on, _evas_canvas_seat_key_modifier_on),
      EFL_OBJECT_OP_FUNC(evas_canvas_seat_key_modifier_off, _evas_canvas_seat_key_modifier_off),
      EFL_OBJECT_OP_FUNC(evas_canvas_font_available_list, _evas_canvas_font_available_list),
      EFL_OBJECT_OP_FUNC(evas_canvas_object_name_find, _evas_canvas_object_name_find),
      EFL_OBJECT_OP_FUNC(evas_canvas_font_path_append, _evas_canvas_font_path_append),
      EFL_OBJECT_OP_FUNC(evas_canvas_font_path_clear, _evas_canvas_font_path_clear),
      EFL_OBJECT_OP_FUNC(evas_canvas_touch_point_list_nth_xy_get, _evas_canvas_touch_point_list_nth_xy_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_key_lock_del, _evas_canvas_key_lock_del),
      EFL_OBJECT_OP_FUNC(evas_canvas_damage_rectangle_add, _evas_canvas_damage_rectangle_add),
      EFL_OBJECT_OP_FUNC(evas_canvas_sync, _evas_canvas_sync),
      EFL_OBJECT_OP_FUNC(evas_canvas_font_path_list, _evas_canvas_font_path_list),
      EFL_OBJECT_OP_FUNC(evas_canvas_image_cache_reload, _evas_canvas_image_cache_reload),
      EFL_OBJECT_OP_FUNC(evas_canvas_coord_world_x_to_screen, _evas_canvas_coord_world_x_to_screen),
      EFL_OBJECT_OP_FUNC(evas_canvas_render_updates, _evas_canvas_render_updates),
      EFL_OBJECT_OP_FUNC(evas_canvas_image_cache_flush, _evas_canvas_image_cache_flush),
      EFL_OBJECT_OP_FUNC(evas_canvas_coord_screen_y_to_world, _evas_canvas_coord_screen_y_to_world),
      EFL_OBJECT_OP_FUNC(evas_canvas_key_modifier_del, _evas_canvas_key_modifier_del),
      EFL_OBJECT_OP_FUNC(evas_canvas_focus_in, _evas_canvas_focus_in),
      EFL_OBJECT_OP_FUNC(evas_canvas_seat_focus_in, _evas_canvas_seat_focus_in),
      EFL_OBJECT_OP_FUNC(evas_canvas_seat_focus_out, _evas_canvas_seat_focus_out),
      EFL_OBJECT_OP_FUNC(evas_canvas_obscured_rectangle_add, _evas_canvas_obscured_rectangle_add),
      EFL_OBJECT_OP_FUNC(evas_canvas_render_dump, _evas_canvas_render_dump),
      EFL_OBJECT_OP_FUNC(evas_canvas_render, _evas_canvas_render),
      EFL_OBJECT_OP_FUNC(evas_canvas_font_path_prepend, _evas_canvas_font_path_prepend),
      EFL_OBJECT_OP_FUNC(evas_canvas_obscured_clear, _evas_canvas_obscured_clear),
      EFL_OBJECT_OP_FUNC(evas_canvas_coord_screen_x_to_world, _evas_canvas_coord_screen_x_to_world),
      EFL_OBJECT_OP_FUNC(evas_canvas_key_lock_add, _evas_canvas_key_lock_add),
      EFL_OBJECT_OP_FUNC(evas_canvas_render_idle_flush, _evas_canvas_render_idle_flush),
      EFL_OBJECT_OP_FUNC(evas_canvas_default_device_get, _evas_canvas_default_device_get),
      EFL_OBJECT_OP_FUNC(evas_canvas_coord_world_y_to_screen, _evas_canvas_coord_world_y_to_screen),
      EFL_OBJECT_OP_FUNC(efl_constructor, _evas_canvas_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_destructor, _evas_canvas_efl_object_destructor),
      EFL_OBJECT_OP_FUNC(efl_invalidate, _evas_canvas_efl_object_invalidate),
      EFL_OBJECT_OP_FUNC(efl_event_thaw, _evas_canvas_efl_object_event_thaw),
      EFL_OBJECT_OP_FUNC(efl_event_freeze, _evas_canvas_efl_object_event_freeze),
      EFL_OBJECT_OP_FUNC(efl_provider_find, _evas_canvas_efl_object_provider_find),
      EFL_OBJECT_OP_FUNC(efl_loop_get, _evas_canvas_efl_loop_consumer_loop_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_pointer_inside_get, _evas_canvas_efl_canvas_pointer_pointer_inside_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_device_get, _evas_canvas_efl_canvas_scene_device_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_seat_get, _evas_canvas_efl_canvas_scene_seat_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_seat_default_get, _evas_canvas_efl_canvas_scene_seat_default_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_pointer_position_get, _evas_canvas_efl_canvas_scene_pointer_position_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_image_max_size_get, _evas_canvas_efl_canvas_scene_image_max_size_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_objects_at_xy_get, _evas_canvas_efl_canvas_scene_objects_at_xy_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_object_top_at_xy_get, _evas_canvas_efl_canvas_scene_object_top_at_xy_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_objects_in_rectangle_get, _evas_canvas_efl_canvas_scene_objects_in_rectangle_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_object_top_in_rectangle_get, _evas_canvas_efl_canvas_scene_object_top_in_rectangle_get),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_group_objects_calculate, _evas_canvas_efl_canvas_scene_group_objects_calculate),
      EFL_OBJECT_OP_FUNC(efl_canvas_scene_group_objects_calculating_get, _evas_canvas_efl_canvas_scene_group_objects_calculating_get),
      EVAS_CANVAS_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"image_cache", __eolian_evas_canvas_image_cache_set_reflect, __eolian_evas_canvas_image_cache_get_reflect},
      {"font_cache", __eolian_evas_canvas_font_cache_set_reflect, __eolian_evas_canvas_font_cache_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _evas_canvas_class_desc = {
   EO_VERSION,
   "Evas.Canvas",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Evas_Public_Data),
   _evas_canvas_class_initializer,
   NULL,
   NULL
};

EFL_DEFINE_CLASS(evas_canvas_class_get, &_evas_canvas_class_desc, EFL_LOOP_CONSUMER_CLASS, EFL_CANVAS_SCENE_INTERFACE, EFL_INPUT_INTERFACE_INTERFACE, EFL_CANVAS_POINTER_INTERFACE, NULL);

#include "evas_canvas_eo.legacy.c"
