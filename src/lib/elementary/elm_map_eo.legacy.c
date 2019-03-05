
EAPI void
elm_map_zoom_min_set(Elm_Map *obj, int zoom)
{
   elm_obj_map_zoom_min_set(obj, zoom);
}

EAPI int
elm_map_zoom_min_get(const Elm_Map *obj)
{
   return elm_obj_map_zoom_min_get(obj);
}

EAPI void
elm_map_rotate_set(Elm_Map *obj, double degree, int cx, int cy)
{
   elm_obj_map_rotate_set(obj, degree, cx, cy);
}

EAPI void
elm_map_rotate_get(const Elm_Map *obj, double *degree, int *cx, int *cy)
{
   elm_obj_map_rotate_get(obj, degree, cx, cy);
}

EAPI void
elm_map_user_agent_set(Elm_Map *obj, const char *user_agent)
{
   elm_obj_map_user_agent_set(obj, user_agent);
}

EAPI const char *
elm_map_user_agent_get(const Elm_Map *obj)
{
   return elm_obj_map_user_agent_get(obj);
}

EAPI void
elm_map_zoom_max_set(Elm_Map *obj, int zoom)
{
   elm_obj_map_zoom_max_set(obj, zoom);
}

EAPI int
elm_map_zoom_max_get(const Elm_Map *obj)
{
   return elm_obj_map_zoom_max_get(obj);
}

EAPI void
elm_map_region_get(const Elm_Map *obj, double *lon, double *lat)
{
   elm_obj_map_region_get(obj, lon, lat);
}

EAPI Eina_List *
elm_map_overlays_get(const Elm_Map *obj)
{
   return elm_obj_map_overlays_get(obj);
}

EAPI void
elm_map_tile_load_status_get(const Elm_Map *obj, int *try_num, int *finish_num)
{
   elm_obj_map_tile_load_status_get(obj, try_num, finish_num);
}

EAPI void
elm_map_source_set(Elm_Map *obj, Elm_Map_Source_Type type, const char *source_name)
{
   elm_obj_map_source_set(obj, type, source_name);
}

EAPI const char *
elm_map_source_get(const Elm_Map *obj, Elm_Map_Source_Type type)
{
   return elm_obj_map_source_get(obj, type);
}

EAPI Elm_Map_Route *
elm_map_route_add(Elm_Map *obj, Elm_Map_Route_Type type, Elm_Map_Route_Method method, double flon, double flat, double tlon, double tlat, Elm_Map_Route_Cb route_cb, void *data)
{
   return elm_obj_map_route_add(obj, type, method, flon, flat, tlon, tlat, route_cb, data);
}

EAPI Efl_Canvas_Object *
elm_map_track_add(Elm_Map *obj, void *emap)
{
   return elm_obj_map_track_add(obj, emap);
}

EAPI void
elm_map_region_to_canvas_convert(const Elm_Map *obj, double lon, double lat, int *x, int *y)
{
   elm_obj_map_region_to_canvas_convert(obj, lon, lat, x, y);
}

EAPI Elm_Map_Overlay *
elm_map_overlay_circle_add(Elm_Map *obj, double lon, double lat, double radius)
{
   return elm_obj_map_overlay_circle_add(obj, lon, lat, radius);
}

EAPI Elm_Map_Overlay *
elm_map_overlay_class_add(Elm_Map *obj)
{
   return elm_obj_map_overlay_class_add(obj);
}

EAPI Elm_Map_Overlay *
elm_map_overlay_bubble_add(Elm_Map *obj)
{
   return elm_obj_map_overlay_bubble_add(obj);
}

EAPI const char **
elm_map_sources_get(const Elm_Map *obj, Elm_Map_Source_Type type)
{
   return elm_obj_map_sources_get(obj, type);
}

EAPI Elm_Map_Overlay *
elm_map_overlay_polygon_add(Elm_Map *obj)
{
   return elm_obj_map_overlay_polygon_add(obj);
}

EAPI Elm_Map_Overlay *
elm_map_overlay_line_add(Elm_Map *obj, double flon, double flat, double tlon, double tlat)
{
   return elm_obj_map_overlay_line_add(obj, flon, flat, tlon, tlat);
}

EAPI void
elm_map_region_show(Elm_Map *obj, double lon, double lat)
{
   elm_obj_map_region_show(obj, lon, lat);
}

EAPI Elm_Map_Name *
elm_map_name_add(const Elm_Map *obj, const char *address, double lon, double lat, Elm_Map_Name_Cb name_cb, void *data)
{
   return elm_obj_map_name_add(obj, address, lon, lat, name_cb, data);
}

EAPI void
elm_map_name_search(const Elm_Map *obj, const char *address, Elm_Map_Name_List_Cb name_cb, void *data)
{
   elm_obj_map_name_search(obj, address, name_cb, data);
}

EAPI void
elm_map_region_bring_in(Elm_Map *obj, double lon, double lat)
{
   elm_obj_map_region_bring_in(obj, lon, lat);
}

EAPI void
elm_map_region_zoom_bring_in(Elm_Map *obj, int zoom, double lon, double lat)
{
   elm_obj_map_region_zoom_bring_in(obj, zoom, lon, lat);
}

EAPI void
elm_map_track_remove(Elm_Map *obj, Efl_Canvas_Object *route)
{
   elm_obj_map_track_remove(obj, route);
}

EAPI Elm_Map_Overlay *
elm_map_overlay_route_add(Elm_Map *obj, const Elm_Map_Route *route)
{
   return elm_obj_map_overlay_route_add(obj, route);
}

EAPI Elm_Map_Overlay *
elm_map_overlay_scale_add(Elm_Map *obj, int x, int y)
{
   return elm_obj_map_overlay_scale_add(obj, x, y);
}

EAPI Elm_Map_Overlay *
elm_map_overlay_add(Elm_Map *obj, double lon, double lat)
{
   return elm_obj_map_overlay_add(obj, lon, lat);
}

EAPI void
elm_map_canvas_to_region_convert(const Elm_Map *obj, int x, int y, double *lon, double *lat)
{
   elm_obj_map_canvas_to_region_convert(obj, x, y, lon, lat);
}
