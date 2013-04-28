#define ELM_OBJ_ROUTE_CLASS elm_obj_route_class_get()

const Eo_Class *elm_obj_route_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_ROUTE_BASE_ID;

enum
{
   ELM_OBJ_ROUTE_SUB_ID_EMAP_SET,
   ELM_OBJ_ROUTE_SUB_ID_LONGITUDE_MIN_MAX_GET,
   ELM_OBJ_ROUTE_SUB_ID_LATITUDE_MIN_MAX_GET,
   ELM_OBJ_ROUTE_SUB_ID_LAST
};

#define ELM_OBJ_ROUTE_ID(sub_id) (ELM_OBJ_ROUTE_BASE_ID + sub_id)


/**
 * @def elm_obj_route_emap_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] emap
 *
 * @see elm_route_emap_set
 *
 * @ingroup Route
 */
#define elm_obj_route_emap_set(emap) ELM_OBJ_ROUTE_ID(ELM_OBJ_ROUTE_SUB_ID_EMAP_SET), EO_TYPECHECK(EMap_Route *, emap)

/**
 * @def elm_obj_route_longitude_min_max_get
 * @since 1.8
 *
 * Get the minimum and maximum values along the longitude.
 *
 * @param[out] min
 * @param[out] max
 *
 * @see elm_route_longitude_min_max_get
 *
 * @ingroup Route
 */
#define elm_obj_route_longitude_min_max_get(min, max) ELM_OBJ_ROUTE_ID(ELM_OBJ_ROUTE_SUB_ID_LONGITUDE_MIN_MAX_GET), EO_TYPECHECK(double *, min), EO_TYPECHECK(double *, max)

/**
 * @def elm_obj_route_latitude_min_max_get
 * @since 1.8
 *
 * Get the minimum and maximum values along the latitude.
 *
 * @param[out] min
 * @param[out] max
 *
 * @see elm_route_latitude_min_max_get
 *
 * @ingroup Route
 */
#define elm_obj_route_latitude_min_max_get(min, max) ELM_OBJ_ROUTE_ID(ELM_OBJ_ROUTE_SUB_ID_LATITUDE_MIN_MAX_GET), EO_TYPECHECK(double *, min), EO_TYPECHECK(double *, max)
