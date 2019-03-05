#ifndef _EFL_CANVAS_VG_SHAPE_EO_LEGACY_H_
#define _EFL_CANVAS_VG_SHAPE_EO_LEGACY_H_

#ifndef _EFL_CANVAS_VG_SHAPE_EO_CLASS_TYPE
#define _EFL_CANVAS_VG_SHAPE_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Vg_Shape;

#endif

#ifndef _EFL_CANVAS_VG_SHAPE_EO_TYPES
#define _EFL_CANVAS_VG_SHAPE_EO_TYPES


#endif

/**
 * @brief Fill of the shape object
 *
 * @param[in] obj The object.
 * @param[in] f Fill object
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EAPI void evas_vg_shape_fill_set(Efl_Canvas_Vg_Shape *obj, Efl_Canvas_Vg_Node *f);

/**
 * @brief Fill of the shape object
 *
 * @param[in] obj The object.
 *
 * @return Fill object
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EAPI Efl_Canvas_Vg_Node *evas_vg_shape_fill_get(const Efl_Canvas_Vg_Shape *obj);

/**
 * @brief Stroke fill of the shape object
 *
 * @param[in] obj The object.
 * @param[in] f Stroke fill object
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EAPI void evas_vg_shape_stroke_fill_set(Efl_Canvas_Vg_Shape *obj, Efl_Canvas_Vg_Node *f);

/**
 * @brief Stroke fill of the shape object
 *
 * @param[in] obj The object.
 *
 * @return Stroke fill object
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EAPI Efl_Canvas_Vg_Node *evas_vg_shape_stroke_fill_get(const Efl_Canvas_Vg_Shape *obj);

/**
 * @brief Stroke marker of the shape object
 *
 * @param[in] obj The object.
 * @param[in] m Stroke marker object
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EAPI void evas_vg_shape_stroke_marker_set(Efl_Canvas_Vg_Shape *obj, Efl_Canvas_Vg_Node *m);

/**
 * @brief Stroke marker of the shape object
 *
 * @param[in] obj The object.
 *
 * @return Stroke marker object
 *
 * @ingroup Evas_Vg_Shape_Group
 */
EAPI Efl_Canvas_Vg_Node *evas_vg_shape_stroke_marker_get(const Efl_Canvas_Vg_Shape *obj);

#endif
