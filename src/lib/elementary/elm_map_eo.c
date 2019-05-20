EWAPI const Efl_Event_Description _ELM_MAP_EVENT_PRESS =
   EFL_EVENT_DESCRIPTION("press");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_LOADED =
   EFL_EVENT_DESCRIPTION("loaded");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_TILE_LOAD =
   EFL_EVENT_DESCRIPTION("tile,load");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_TILE_LOADED =
   EFL_EVENT_DESCRIPTION("tile,loaded");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_TILE_LOADED_FAIL =
   EFL_EVENT_DESCRIPTION("tile,loaded,fail");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_ROUTE_LOAD =
   EFL_EVENT_DESCRIPTION("route,load");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_ROUTE_LOADED =
   EFL_EVENT_DESCRIPTION("route,loaded");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_ROUTE_LOADED_FAIL =
   EFL_EVENT_DESCRIPTION("route,loaded,fail");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_NAME_LOAD =
   EFL_EVENT_DESCRIPTION("name,load");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_NAME_LOADED =
   EFL_EVENT_DESCRIPTION("name,loaded");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_NAME_LOADED_FAIL =
   EFL_EVENT_DESCRIPTION("name,loaded,fail");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_OVERLAY_CLICKED =
   EFL_EVENT_DESCRIPTION("overlay,clicked");
EWAPI const Efl_Event_Description _ELM_MAP_EVENT_OVERLAY_DEL =
   EFL_EVENT_DESCRIPTION("overlay,del");

void _elm_map_zoom_min_set(Eo *obj, Elm_Map_Data *pd, int zoom);


static Eina_Error
__eolian_elm_map_zoom_min_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_map_zoom_min_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_map_zoom_min_set, EFL_FUNC_CALL(zoom), int zoom);

int _elm_map_zoom_min_get(const Eo *obj, Elm_Map_Data *pd);


static Eina_Value
__eolian_elm_map_zoom_min_get_reflect(const Eo *obj)
{
   int val = elm_obj_map_zoom_min_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_map_zoom_min_get, int, -1 /* +1 */);

void _elm_map_map_rotate_set(Eo *obj, Elm_Map_Data *pd, double degree, int cx, int cy);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_map_rotate_set, EFL_FUNC_CALL(degree, cx, cy), double degree, int cx, int cy);

void _elm_map_map_rotate_get(const Eo *obj, Elm_Map_Data *pd, double *degree, int *cx, int *cy);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_map_rotate_get, EFL_FUNC_CALL(degree, cx, cy), double *degree, int *cx, int *cy);

void _elm_map_user_agent_set(Eo *obj, Elm_Map_Data *pd, const char *user_agent);


static Eina_Error
__eolian_elm_map_user_agent_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   const char *cval;
   if (!eina_value_string_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_map_user_agent_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_map_user_agent_set, EFL_FUNC_CALL(user_agent), const char *user_agent);

const char *_elm_map_user_agent_get(const Eo *obj, Elm_Map_Data *pd);


static Eina_Value
__eolian_elm_map_user_agent_get_reflect(const Eo *obj)
{
   const char *val = elm_obj_map_user_agent_get(obj);
   return eina_value_string_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_map_user_agent_get, const char *, NULL);

void _elm_map_zoom_max_set(Eo *obj, Elm_Map_Data *pd, int zoom);


static Eina_Error
__eolian_elm_map_zoom_max_set_reflect(Eo *obj, Eina_Value val)
{
   Eina_Error r = 0;   int cval;
   if (!eina_value_int_convert(&val, &cval))
      {
         r = EINA_ERROR_VALUE_FAILED;
         goto end;
      }
   elm_obj_map_zoom_max_set(obj, cval);
 end:
   eina_value_flush(&val);
   return r;
}

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_map_zoom_max_set, EFL_FUNC_CALL(zoom), int zoom);

int _elm_map_zoom_max_get(const Eo *obj, Elm_Map_Data *pd);


static Eina_Value
__eolian_elm_map_zoom_max_get_reflect(const Eo *obj)
{
   int val = elm_obj_map_zoom_max_get(obj);
   return eina_value_int_init(val);
}

EOAPI EFL_FUNC_BODY_CONST(elm_obj_map_zoom_max_get, int, -1 /* +1 */);

void _elm_map_region_get(const Eo *obj, Elm_Map_Data *pd, double *lon, double *lat);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_map_region_get, EFL_FUNC_CALL(lon, lat), double *lon, double *lat);

Eina_List *_elm_map_overlays_get(const Eo *obj, Elm_Map_Data *pd);

EOAPI EFL_FUNC_BODY_CONST(elm_obj_map_overlays_get, Eina_List *, NULL);

void _elm_map_tile_load_status_get(const Eo *obj, Elm_Map_Data *pd, int *try_num, int *finish_num);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_map_tile_load_status_get, EFL_FUNC_CALL(try_num, finish_num), int *try_num, int *finish_num);

void _elm_map_source_set(Eo *obj, Elm_Map_Data *pd, Elm_Map_Source_Type type, const char *source_name);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_map_source_set, EFL_FUNC_CALL(type, source_name), Elm_Map_Source_Type type, const char *source_name);

const char *_elm_map_source_get(const Eo *obj, Elm_Map_Data *pd, Elm_Map_Source_Type type);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_map_source_get, const char *, NULL, EFL_FUNC_CALL(type), Elm_Map_Source_Type type);

Elm_Map_Route *_elm_map_route_add(Eo *obj, Elm_Map_Data *pd, Elm_Map_Route_Type type, Elm_Map_Route_Method method, double flon, double flat, double tlon, double tlat, Elm_Map_Route_Cb route_cb, void *data);

EOAPI EFL_FUNC_BODYV(elm_obj_map_route_add, Elm_Map_Route *, NULL, EFL_FUNC_CALL(type, method, flon, flat, tlon, tlat, route_cb, data), Elm_Map_Route_Type type, Elm_Map_Route_Method method, double flon, double flat, double tlon, double tlat, Elm_Map_Route_Cb route_cb, void *data);

Efl_Canvas_Object *_elm_map_track_add(Eo *obj, Elm_Map_Data *pd, void *emap);

EOAPI EFL_FUNC_BODYV(elm_obj_map_track_add, Efl_Canvas_Object *, NULL, EFL_FUNC_CALL(emap), void *emap);

void _elm_map_region_to_canvas_convert(const Eo *obj, Elm_Map_Data *pd, double lon, double lat, int *x, int *y);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_map_region_to_canvas_convert, EFL_FUNC_CALL(lon, lat, x, y), double lon, double lat, int *x, int *y);

Elm_Map_Overlay *_elm_map_overlay_circle_add(Eo *obj, Elm_Map_Data *pd, double lon, double lat, double radius);

EOAPI EFL_FUNC_BODYV(elm_obj_map_overlay_circle_add, Elm_Map_Overlay *, NULL, EFL_FUNC_CALL(lon, lat, radius), double lon, double lat, double radius);

Elm_Map_Overlay *_elm_map_overlay_class_add(Eo *obj, Elm_Map_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_map_overlay_class_add, Elm_Map_Overlay *, NULL);

Elm_Map_Overlay *_elm_map_overlay_bubble_add(Eo *obj, Elm_Map_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_map_overlay_bubble_add, Elm_Map_Overlay *, NULL);

const char **_elm_map_sources_get(const Eo *obj, Elm_Map_Data *pd, Elm_Map_Source_Type type);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_map_sources_get, const char **, NULL, EFL_FUNC_CALL(type), Elm_Map_Source_Type type);

Elm_Map_Overlay *_elm_map_overlay_polygon_add(Eo *obj, Elm_Map_Data *pd);

EOAPI EFL_FUNC_BODY(elm_obj_map_overlay_polygon_add, Elm_Map_Overlay *, NULL);

Elm_Map_Overlay *_elm_map_overlay_line_add(Eo *obj, Elm_Map_Data *pd, double flon, double flat, double tlon, double tlat);

EOAPI EFL_FUNC_BODYV(elm_obj_map_overlay_line_add, Elm_Map_Overlay *, NULL, EFL_FUNC_CALL(flon, flat, tlon, tlat), double flon, double flat, double tlon, double tlat);

void _elm_map_region_show(Eo *obj, Elm_Map_Data *pd, double lon, double lat);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_map_region_show, EFL_FUNC_CALL(lon, lat), double lon, double lat);

Elm_Map_Name *_elm_map_name_add(const Eo *obj, Elm_Map_Data *pd, const char *address, double lon, double lat, Elm_Map_Name_Cb name_cb, void *data);

EOAPI EFL_FUNC_BODYV_CONST(elm_obj_map_name_add, Elm_Map_Name *, NULL, EFL_FUNC_CALL(address, lon, lat, name_cb, data), const char *address, double lon, double lat, Elm_Map_Name_Cb name_cb, void *data);

void _elm_map_name_search(const Eo *obj, Elm_Map_Data *pd, const char *address, Elm_Map_Name_List_Cb name_cb, void *data);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_map_name_search, EFL_FUNC_CALL(address, name_cb, data), const char *address, Elm_Map_Name_List_Cb name_cb, void *data);

void _elm_map_map_region_bring_in(Eo *obj, Elm_Map_Data *pd, double lon, double lat);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_map_region_bring_in, EFL_FUNC_CALL(lon, lat), double lon, double lat);

void _elm_map_region_zoom_bring_in(Eo *obj, Elm_Map_Data *pd, int zoom, double lon, double lat);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_map_region_zoom_bring_in, EFL_FUNC_CALL(zoom, lon, lat), int zoom, double lon, double lat);

void _elm_map_track_remove(Eo *obj, Elm_Map_Data *pd, Efl_Canvas_Object *route);

EOAPI EFL_VOID_FUNC_BODYV(elm_obj_map_track_remove, EFL_FUNC_CALL(route), Efl_Canvas_Object *route);

Elm_Map_Overlay *_elm_map_overlay_route_add(Eo *obj, Elm_Map_Data *pd, const Elm_Map_Route *route);

EOAPI EFL_FUNC_BODYV(elm_obj_map_overlay_route_add, Elm_Map_Overlay *, NULL, EFL_FUNC_CALL(route), const Elm_Map_Route *route);

Elm_Map_Overlay *_elm_map_overlay_scale_add(Eo *obj, Elm_Map_Data *pd, int x, int y);

EOAPI EFL_FUNC_BODYV(elm_obj_map_overlay_scale_add, Elm_Map_Overlay *, NULL, EFL_FUNC_CALL(x, y), int x, int y);

Elm_Map_Overlay *_elm_map_overlay_add(Eo *obj, Elm_Map_Data *pd, double lon, double lat);

EOAPI EFL_FUNC_BODYV(elm_obj_map_overlay_add, Elm_Map_Overlay *, NULL, EFL_FUNC_CALL(lon, lat), double lon, double lat);

void _elm_map_canvas_to_region_convert(const Eo *obj, Elm_Map_Data *pd, int x, int y, double *lon, double *lat);

EOAPI EFL_VOID_FUNC_BODYV_CONST(elm_obj_map_canvas_to_region_convert, EFL_FUNC_CALL(x, y, lon, lat), int x, int y, double *lon, double *lat);

Efl_Object *_elm_map_efl_object_constructor(Eo *obj, Elm_Map_Data *pd);


void _elm_map_efl_object_invalidate(Eo *obj, Elm_Map_Data *pd);


void _elm_map_efl_gfx_entity_position_set(Eo *obj, Elm_Map_Data *pd, Eina_Position2D pos);


void _elm_map_efl_gfx_entity_size_set(Eo *obj, Elm_Map_Data *pd, Eina_Size2D size);


void _elm_map_efl_canvas_group_group_member_add(Eo *obj, Elm_Map_Data *pd, Efl_Canvas_Object *sub_obj);


Eina_Error _elm_map_efl_ui_widget_theme_apply(Eo *obj, Elm_Map_Data *pd);


Eina_Bool _elm_map_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Map_Data *pd);


Eina_Bool _elm_map_efl_ui_widget_widget_input_event_handler(Eo *obj, Elm_Map_Data *pd, const Efl_Event *eo_event, Efl_Canvas_Object *source);


const Efl_Access_Action_Data *_elm_map_efl_access_widget_action_elm_actions_get(const Eo *obj, Elm_Map_Data *pd);


void _elm_map_efl_ui_zoom_zoom_level_set(Eo *obj, Elm_Map_Data *pd, double zoom);


double _elm_map_efl_ui_zoom_zoom_level_get(const Eo *obj, Elm_Map_Data *pd);


void _elm_map_efl_ui_zoom_zoom_mode_set(Eo *obj, Elm_Map_Data *pd, Efl_Ui_Zoom_Mode mode);


Efl_Ui_Zoom_Mode _elm_map_efl_ui_zoom_zoom_mode_get(const Eo *obj, Elm_Map_Data *pd);


void _elm_map_efl_ui_zoom_zoom_animation_set(Eo *obj, Elm_Map_Data *pd, Eina_Bool paused);


Eina_Bool _elm_map_efl_ui_zoom_zoom_animation_get(const Eo *obj, Elm_Map_Data *pd);


void _elm_map_elm_interface_scrollable_wheel_disabled_set(Eo *obj, Elm_Map_Data *pd, Eina_Bool disabled);


static Eina_Bool
_elm_map_class_initializer(Efl_Class *klass)
{
   const Efl_Object_Ops *opsp = NULL;

   const Efl_Object_Property_Reflection_Ops *ropsp = NULL;

#ifndef ELM_MAP_EXTRA_OPS
#define ELM_MAP_EXTRA_OPS
#endif

   EFL_OPS_DEFINE(ops,
      EFL_OBJECT_OP_FUNC(elm_obj_map_zoom_min_set, _elm_map_zoom_min_set),
      EFL_OBJECT_OP_FUNC(elm_obj_map_zoom_min_get, _elm_map_zoom_min_get),
      EFL_OBJECT_OP_FUNC(elm_obj_map_rotate_set, _elm_map_map_rotate_set),
      EFL_OBJECT_OP_FUNC(elm_obj_map_rotate_get, _elm_map_map_rotate_get),
      EFL_OBJECT_OP_FUNC(elm_obj_map_user_agent_set, _elm_map_user_agent_set),
      EFL_OBJECT_OP_FUNC(elm_obj_map_user_agent_get, _elm_map_user_agent_get),
      EFL_OBJECT_OP_FUNC(elm_obj_map_zoom_max_set, _elm_map_zoom_max_set),
      EFL_OBJECT_OP_FUNC(elm_obj_map_zoom_max_get, _elm_map_zoom_max_get),
      EFL_OBJECT_OP_FUNC(elm_obj_map_region_get, _elm_map_region_get),
      EFL_OBJECT_OP_FUNC(elm_obj_map_overlays_get, _elm_map_overlays_get),
      EFL_OBJECT_OP_FUNC(elm_obj_map_tile_load_status_get, _elm_map_tile_load_status_get),
      EFL_OBJECT_OP_FUNC(elm_obj_map_source_set, _elm_map_source_set),
      EFL_OBJECT_OP_FUNC(elm_obj_map_source_get, _elm_map_source_get),
      EFL_OBJECT_OP_FUNC(elm_obj_map_route_add, _elm_map_route_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_track_add, _elm_map_track_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_region_to_canvas_convert, _elm_map_region_to_canvas_convert),
      EFL_OBJECT_OP_FUNC(elm_obj_map_overlay_circle_add, _elm_map_overlay_circle_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_overlay_class_add, _elm_map_overlay_class_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_overlay_bubble_add, _elm_map_overlay_bubble_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_sources_get, _elm_map_sources_get),
      EFL_OBJECT_OP_FUNC(elm_obj_map_overlay_polygon_add, _elm_map_overlay_polygon_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_overlay_line_add, _elm_map_overlay_line_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_region_show, _elm_map_region_show),
      EFL_OBJECT_OP_FUNC(elm_obj_map_name_add, _elm_map_name_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_name_search, _elm_map_name_search),
      EFL_OBJECT_OP_FUNC(elm_obj_map_region_bring_in, _elm_map_map_region_bring_in),
      EFL_OBJECT_OP_FUNC(elm_obj_map_region_zoom_bring_in, _elm_map_region_zoom_bring_in),
      EFL_OBJECT_OP_FUNC(elm_obj_map_track_remove, _elm_map_track_remove),
      EFL_OBJECT_OP_FUNC(elm_obj_map_overlay_route_add, _elm_map_overlay_route_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_overlay_scale_add, _elm_map_overlay_scale_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_overlay_add, _elm_map_overlay_add),
      EFL_OBJECT_OP_FUNC(elm_obj_map_canvas_to_region_convert, _elm_map_canvas_to_region_convert),
      EFL_OBJECT_OP_FUNC(efl_constructor, _elm_map_efl_object_constructor),
      EFL_OBJECT_OP_FUNC(efl_invalidate, _elm_map_efl_object_invalidate),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_position_set, _elm_map_efl_gfx_entity_position_set),
      EFL_OBJECT_OP_FUNC(efl_gfx_entity_size_set, _elm_map_efl_gfx_entity_size_set),
      EFL_OBJECT_OP_FUNC(efl_canvas_group_member_add, _elm_map_efl_canvas_group_group_member_add),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_theme_apply, _elm_map_efl_ui_widget_theme_apply),
      EFL_OBJECT_OP_FUNC(efl_ui_focus_object_on_focus_update, _elm_map_efl_ui_focus_object_on_focus_update),
      EFL_OBJECT_OP_FUNC(efl_ui_widget_input_event_handler, _elm_map_efl_ui_widget_widget_input_event_handler),
      EFL_OBJECT_OP_FUNC(efl_access_widget_action_elm_actions_get, _elm_map_efl_access_widget_action_elm_actions_get),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_level_set, _elm_map_efl_ui_zoom_zoom_level_set),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_level_get, _elm_map_efl_ui_zoom_zoom_level_get),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_mode_set, _elm_map_efl_ui_zoom_zoom_mode_set),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_mode_get, _elm_map_efl_ui_zoom_zoom_mode_get),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_animation_set, _elm_map_efl_ui_zoom_zoom_animation_set),
      EFL_OBJECT_OP_FUNC(efl_ui_zoom_animation_get, _elm_map_efl_ui_zoom_zoom_animation_get),
      EFL_OBJECT_OP_FUNC(elm_interface_scrollable_wheel_disabled_set, _elm_map_elm_interface_scrollable_wheel_disabled_set),
      ELM_MAP_EXTRA_OPS
   );
   opsp = &ops;

   static const Efl_Object_Property_Reflection refl_table[] = {
      {"zoom_min", __eolian_elm_map_zoom_min_set_reflect, __eolian_elm_map_zoom_min_get_reflect},
      {"user_agent", __eolian_elm_map_user_agent_set_reflect, __eolian_elm_map_user_agent_get_reflect},
      {"zoom_max", __eolian_elm_map_zoom_max_set_reflect, __eolian_elm_map_zoom_max_get_reflect},
   };
   static const Efl_Object_Property_Reflection_Ops rops = {
      refl_table, EINA_C_ARRAY_LENGTH(refl_table)
   };
   ropsp = &rops;

   return efl_class_functions_set(klass, opsp, ropsp);
}

static const Efl_Class_Description _elm_map_class_desc = {
   EO_VERSION,
   "Elm.Map",
   EFL_CLASS_TYPE_REGULAR,
   sizeof(Elm_Map_Data),
   _elm_map_class_initializer,
   _elm_map_class_constructor,
   NULL
};

EFL_DEFINE_CLASS(elm_map_class_get, &_elm_map_class_desc, EFL_UI_WIDGET_CLASS, ELM_INTERFACE_SCROLLABLE_MIXIN, EFL_ACCESS_WIDGET_ACTION_MIXIN, EFL_UI_CLICKABLE_MIXIN, EFL_UI_LEGACY_INTERFACE, EFL_UI_ZOOM_INTERFACE, NULL);

#include "elm_map_eo.legacy.c"
