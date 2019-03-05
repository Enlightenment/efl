#ifndef _ELM_ROUTE_EO_H_
#define _ELM_ROUTE_EO_H_

#ifndef _ELM_ROUTE_EO_CLASS_TYPE
#define _ELM_ROUTE_EO_CLASS_TYPE

typedef Eo Elm_Route;

#endif

#ifndef _ELM_ROUTE_EO_TYPES
#define _ELM_ROUTE_EO_TYPES


#endif
/** Elementary route class
 *
 * @ingroup Elm_Route
 */
#define ELM_ROUTE_CLASS elm_route_class_get()

EWAPI const Efl_Class *elm_route_class_get(void);

/**
 * @brief Set map widget for this route
 *
 * @param[in] obj The object.
 * @param[in] emap Elementary map widget
 *
 * @ingroup Elm_Route
 */
EOAPI void elm_obj_route_emap_set(Eo *obj, void *emap);

/**
 * @brief Get the minimum and maximum values along the longitude.
 *
 * @note If only one value is needed, the other pointer can be passed as null.
 *
 * @param[in] obj The object.
 * @param[out] min Pointer to store the minimum value.
 * @param[out] max Pointer to store the maximum value.
 *
 * @ingroup Elm_Route
 */
EOAPI void elm_obj_route_longitude_min_max_get(const Eo *obj, double *min, double *max);

/**
 * @brief Get the minimum and maximum values along the latitude.
 *
 * @note If only one value is needed, the other pointer can be passed as null.
 *
 * @param[in] obj The object.
 * @param[out] min Pointer to store the minimum value.
 * @param[out] max Pointer to store the maximum value.
 *
 * @ingroup Elm_Route
 */
EOAPI void elm_obj_route_latitude_min_max_get(const Eo *obj, double *min, double *max);

#endif
