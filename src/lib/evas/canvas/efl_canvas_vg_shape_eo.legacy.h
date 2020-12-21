#ifndef _EVAS_VG_SHAPE_EO_LEGACY_H_
#define _EVAS_VG_SHAPE_EO_LEGACY_H_

#ifndef _EVAS_VG_SHAPE_EO_CLASS_TYPE
#define _EVAS_VG_SHAPE_EO_CLASS_TYPE

typedef Eo Evas_Vg_Shape;

#endif

#ifndef _EVAS_VG_SHAPE_EO_TYPES
#define _EVAS_VG_SHAPE_EO_TYPES

#endif

/**
 * @defgroup Evas_Vg_Shape_Group Vector Graphics Shape Functions
 * @ingroup Evas_Object_Vg_Group
 */

/**
 * @brief Fill of the shape object.
 *
 * @param[in] obj The object.
 * @param[in] f The fill object.
 *
 * @since 1.24
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EVAS_API void evas_vg_shape_fill_set(Evas_Vg_Shape *obj, Evas_Vg_Node *f);

/**
 * @brief Fill of the shape object.
 *
 * @param[in] obj The object.
 * @return The fill object.
 *
 * @since 1.24
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EVAS_API Evas_Vg_Node *evas_vg_shape_fill_get(const Evas_Vg_Shape *obj);

/**
 * @brief Stroke fill of the shape object.
 *
 * @param[in] obj The object.
 * @param[in] f The stroke fill object.
 *
 * @since 1.24
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EVAS_API void evas_vg_shape_stroke_fill_set(Evas_Vg_Shape *obj, Evas_Vg_Node *f);

/**
 * @brief Stroke fill of the shape object.
 *
 * @param[in] obj The object.
 * @return The stroke fill object.
 *
 * @since 1.24
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EVAS_API Evas_Vg_Node *evas_vg_shape_stroke_fill_get(const Evas_Vg_Shape *obj);

/**
 * @brief Stroke marker of the shape object
 *
 * @param[in] obj The object.
 * @param[in] m Stroke marker object
 *
 * @since 1.24
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EVAS_API void evas_vg_shape_stroke_marker_set(Evas_Vg_Shape *obj, Evas_Vg_Node *m);

/**
 * @brief Stroke marker of the shape object
 *
 * @param[in] obj The object.
 * @return Stroke marker object
 *
 * @since 1.24
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EVAS_API Evas_Vg_Node *evas_vg_shape_stroke_marker_get(const Evas_Vg_Shape *obj);

#endif
