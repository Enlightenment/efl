#ifndef _EVAS_LINE_EO_H_
#define _EVAS_LINE_EO_H_

#ifndef _EVAS_LINE_EO_CLASS_TYPE
#define _EVAS_LINE_EO_CLASS_TYPE

typedef Eo Evas_Line;

#endif

#ifndef _EVAS_LINE_EO_TYPES
#define _EVAS_LINE_EO_TYPES


#endif
/** Evas line class
 *
 * @ingroup Evas_Line
 */
#define EVAS_LINE_CLASS evas_line_class_get()

EWAPI const Efl_Class *evas_line_class_get(void);

/**
 * @brief Sets the coordinates of the end points of the given evas line object.
 *
 * @param[in] obj The object.
 * @param[in] x1 The X coordinate of the first point.
 * @param[in] y1 The Y coordinate of the first point.
 * @param[in] x2 The X coordinate of the second point.
 * @param[in] y2 The Y coordinate of the second point.
 *
 * @since 1.8
 *
 * @ingroup Evas_Line
 */
EOAPI void evas_obj_line_xy_set(Eo *obj, int x1, int y1, int x2, int y2);

/**
 * @brief Retrieves the coordinates of the end points of the given evas line
 * object.
 *
 * @param[in] obj The object.
 * @param[out] x1 The X coordinate of the first point.
 * @param[out] y1 The Y coordinate of the first point.
 * @param[out] x2 The X coordinate of the second point.
 * @param[out] y2 The Y coordinate of the second point.
 *
 * @ingroup Evas_Line
 */
EOAPI void evas_obj_line_xy_get(const Eo *obj, int *x1, int *y1, int *x2, int *y2);

#endif
