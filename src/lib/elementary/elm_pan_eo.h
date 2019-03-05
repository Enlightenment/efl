#ifndef _ELM_PAN_EO_H_
#define _ELM_PAN_EO_H_

#ifndef _ELM_PAN_EO_CLASS_TYPE
#define _ELM_PAN_EO_CLASS_TYPE

typedef Eo Elm_Pan;

#endif

#ifndef _ELM_PAN_EO_TYPES
#define _ELM_PAN_EO_TYPES


#endif
/** Elementary pan class
 *
 * @ingroup Elm_Pan
 */
#define ELM_PAN_CLASS elm_pan_class_get()

EWAPI const Efl_Class *elm_pan_class_get(void);

/**
 * @brief Position
 *
 * @param[in] obj The object.
 * @param[in] x X coordinate
 * @param[in] y Y coordinate
 *
 * @ingroup Elm_Pan
 */
EOAPI void elm_obj_pan_pos_set(Eo *obj, int x, int y);

/**
 * @brief Position
 *
 * @param[in] obj The object.
 * @param[out] x X coordinate
 * @param[out] y Y coordinate
 *
 * @ingroup Elm_Pan
 */
EOAPI void elm_obj_pan_pos_get(const Eo *obj, int *x, int *y);

/**
 * @brief Content size
 *
 * @param[in] obj The object.
 * @param[out] w Width
 * @param[out] h Height
 *
 * @ingroup Elm_Pan
 */
EOAPI void elm_obj_pan_content_size_get(const Eo *obj, int *w, int *h);

/**
 * @brief Minimal position
 *
 * @param[in] obj The object.
 * @param[out] x X coordinate
 * @param[out] y Y coordinate
 *
 * @ingroup Elm_Pan
 */
EOAPI void elm_obj_pan_pos_min_get(const Eo *obj, int *x, int *y);

/**
 * @brief Maximal position
 *
 * @param[in] obj The object.
 * @param[out] x X coordinate
 * @param[out] y Y coordinate
 *
 * @ingroup Elm_Pan
 */
EOAPI void elm_obj_pan_pos_max_get(const Eo *obj, int *x, int *y);

EWAPI extern const Efl_Event_Description _ELM_PAN_EVENT_CHANGED;

/** Called when pan object changed
 *
 * @ingroup Elm_Pan
 */
#define ELM_PAN_EVENT_CHANGED (&(_ELM_PAN_EVENT_CHANGED))

#endif
