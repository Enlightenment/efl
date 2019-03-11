#ifndef _ELM_PAN_EO_LEGACY_H_
#define _ELM_PAN_EO_LEGACY_H_

#ifndef _ELM_PAN_EO_CLASS_TYPE
#define _ELM_PAN_EO_CLASS_TYPE

typedef Eo Elm_Pan;

#endif

#ifndef _ELM_PAN_EO_TYPES
#define _ELM_PAN_EO_TYPES


#endif

/**
 * @brief Position
 *
 * @param[in] obj The object.
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 *
 * @ingroup Elm_Pan_Group
 */
EAPI void elm_pan_pos_set(Elm_Pan *obj, int x, int y);

/**
 * @brief Position
 *
 * @param[in] obj The object.
 * @param[out] x X coordinate
 * @param[out] y Y coordinate
 *
 * @ingroup Elm_Pan_Group
 */
EAPI void elm_pan_pos_get(const Elm_Pan *obj, int *x, int *y);

/**
 * @brief Content size
 *
 * @param[in] obj The object.
 * @param[out] w Width
 * @param[out] h Height
 *
 * @ingroup Elm_Pan_Group
 */
EAPI void elm_pan_content_size_get(const Elm_Pan *obj, int *w, int *h);

/**
 * @brief Minimal position
 *
 * @param[in] obj The object.
 * @param[out] x X coordinate
 * @param[out] y Y coordinate
 *
 * @ingroup Elm_Pan_Group
 */
EAPI void elm_pan_pos_min_get(const Elm_Pan *obj, int *x, int *y);

/**
 * @brief Maximal position
 *
 * @param[in] obj The object.
 * @param[out] x X coordinate
 * @param[out] y Y coordinate
 *
 * @ingroup Elm_Pan_Group
 */
EAPI void elm_pan_pos_max_get(const Elm_Pan *obj, int *x, int *y);

#endif
