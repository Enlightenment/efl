
ELM_API void
elm_route_emap_set(Elm_Route *obj, void *emap)
{
   elm_obj_route_emap_set(obj, emap);
}

ELM_API void
elm_route_longitude_min_max_get(const Elm_Route *obj, double *min, double *max)
{
   elm_obj_route_longitude_min_max_get(obj, min, max);
}

ELM_API void
elm_route_latitude_min_max_get(const Elm_Route *obj, double *min, double *max)
{
   elm_obj_route_latitude_min_max_get(obj, min, max);
}
