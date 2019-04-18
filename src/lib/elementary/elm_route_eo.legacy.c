
EAPI void
elm_route_emap_set(Elm_Route *obj, void *emap)
{
   elm_obj_route_emap_set(obj, emap);
}

EAPI void
elm_route_longitude_min_max_get(const Elm_Route *obj, double *min, double *max)
{
   elm_obj_route_longitude_min_max_get(obj, min, max);
}

EAPI void
elm_route_latitude_min_max_get(const Elm_Route *obj, double *min, double *max)
{
   elm_obj_route_latitude_min_max_get(obj, min, max);
}
