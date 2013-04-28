/**
 * Add a new route object to the parent's canvas
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Route
 */
EAPI Evas_Object *elm_route_add(Evas_Object *parent);

#ifdef ELM_EMAP
EAPI void         elm_route_emap_set(Evas_Object *obj, EMap_Route *emap);
#endif

#ifdef ELM_EMAP
/**
 * emap set
 *
 * @param parent The parent object
 * @param emap The emap
 *
 * @ingroup Route
 */
EAPI void         elm_route_emap_set(Evas_Object *obj, EMap_Route *emap);
#endif

/**
 * Get the minimum and maximum values along the longitude.
 *
 * @param obj The route object.
 * @param min Pointer to store the minimum value.
 * @param max Pointer to store the maximum value.
 *
 * @note If only one value is needed, the other pointer can be passed
 * as @c NULL.
 *
 * @ingroup Route
 */
EAPI void        elm_route_longitude_min_max_get(const Evas_Object *obj, double *min, double *max);

/**
 * Get the minimum and maximum values along the latitude.
 *
 * @param obj The route object.
 * @param min Pointer to store the minimum value.
 * @param max Pointer to store the maximum value.
 *
 * @note If only one value is needed, the other pointer can be passed
 * as @c NULL.
 *
 * @ingroup Route
 */
EAPI void        elm_route_latitude_min_max_get(const Evas_Object *obj, double *min, double *max);
