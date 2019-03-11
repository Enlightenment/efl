#ifndef _EVAS_LINE_EO_LEGACY_H_
#define _EVAS_LINE_EO_LEGACY_H_

#ifndef _EVAS_LINE_EO_CLASS_TYPE
#define _EVAS_LINE_EO_CLASS_TYPE

typedef Eo Evas_Line;

#endif

#ifndef _EVAS_LINE_EO_TYPES
#define _EVAS_LINE_EO_TYPES


#endif

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
 * @ingroup Evas_Object_Line_Group
 */
EAPI void evas_object_line_xy_set(Evas_Line *obj, int x1, int y1, int x2, int y2);

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
 * @ingroup Evas_Object_Line_Group
 */
EAPI void evas_object_line_xy_get(const Evas_Line *obj, int *x1, int *y1, int *x2, int *y2);

#endif
