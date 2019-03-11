#ifndef _EFL_CANVAS_VG_NODE_EO_LEGACY_H_
#define _EFL_CANVAS_VG_NODE_EO_LEGACY_H_

#ifndef _EFL_CANVAS_VG_NODE_EO_CLASS_TYPE
#define _EFL_CANVAS_VG_NODE_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Vg_Node;

#endif

#ifndef _EFL_CANVAS_VG_NODE_EO_TYPES
#define _EFL_CANVAS_VG_NODE_EO_TYPES


#endif

/**
 * @brief Sets the transformation matrix to be used for this node object.
 *
 * @note Pass @c null to cancel the applied transformation.
 *
 * @param[in] obj The object.
 * @param[in] m Transformation matrix.
 *
 * @since 1.14
 *
 * @ingroup Evas_Vg_Node_Group
 */
EAPI void evas_vg_node_transformation_set(Efl_Canvas_Vg_Node *obj, const Eina_Matrix3 *m);

/**
 * @brief Gets the transformation matrix used for this node object.
 *
 * @param[in] obj The object.
 *
 * @return Transformation matrix.
 *
 * @since 1.14
 *
 * @ingroup Evas_Vg_Node_Group
 */
EAPI const Eina_Matrix3 *evas_vg_node_transformation_get(const Efl_Canvas_Vg_Node *obj);

/**
 * @brief Sets the origin position of the node object.
 *
 * This origin position affects node transformation.
 *
 * @param[in] obj The object.
 * @param[in] x @c origin x position.
 * @param[in] y @c origin y position.
 *
 * @since 1.14
 *
 * @ingroup Evas_Vg_Node_Group
 */
EAPI void evas_vg_node_origin_set(Efl_Canvas_Vg_Node *obj, double x, double y);

/**
 * @brief Gets the origin position of the node object.
 *
 * @param[in] obj The object.
 * @param[out] x @c origin x position.
 * @param[out] y @c origin y position.
 *
 * @since 1.14
 *
 * @ingroup Evas_Vg_Node_Group
 */
EAPI void evas_vg_node_origin_get(const Efl_Canvas_Vg_Node *obj, double *x, double *y);

/**
 * @brief Set Mask Node to this renderer
 *
 * @param[in] obj The object.
 * @param[in] mask Mask object
 * @param[in] op Masking Option. Reserved
 *
 * @ingroup Evas_Vg_Node_Group
 */
EAPI void evas_vg_node_mask_set(Efl_Canvas_Vg_Node *obj, Efl_Canvas_Vg_Node *mask, int op);

#endif
