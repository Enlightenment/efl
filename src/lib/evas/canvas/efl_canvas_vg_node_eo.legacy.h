#ifndef _EVAS_VG_NODE_EO_LEGACY_H_
#define _EVAS_VG_NODE_EO_LEGACY_H_

#ifndef _EVAS_VG_NODE_EO_CLASS_TYPE
#define _EVAS_VG_NODE_EO_CLASS_TYPE

typedef Eo Evas_Vg_Node;

#endif

#ifndef _EVAS_VG_NODE_EO_TYPES
#define _EVAS_VG_NODE_EO_TYPES


#endif

/**
 * @defgroup Evas_Vg_Node_Group Vector Graphics Node Functions
 * @ingroup Evas_Object_Vg_Group
 */

/**
 * @brief Sets the transformation matrix to be used for this node object.
 *
 * @note Pass @c null to cancel the applied transformation.
 *
 * @param[in] obj The object.
 * @param[in] m The transformation matrix.
 *
 * @since 1.14
 *
 * @ingroup Evas_Vg_Node_Group
 */
EVAS_API void evas_vg_node_transformation_set(Evas_Vg_Node *obj, const Eina_Matrix3 *m);

/**
 * @brief Gets the transformation matrix used for this node object.
 *
 * @param[in] obj The object.
 * @return The transformation matrix.
 *
 * @since 1.14
 *
 * @ingroup Evas_Vg_Node_Group
 */
EVAS_API const Eina_Matrix3 *evas_vg_node_transformation_get(const Evas_Vg_Node *obj);

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
EVAS_API void evas_vg_node_origin_set(Evas_Vg_Node *obj, double x, double y);

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
EVAS_API void evas_vg_node_origin_get(const Evas_Vg_Node *obj, double *x, double *y);

/**
 * @brief Set Mask Node to this renderer
 *
 * @param[in] obj The object.
 * @param[in] mask Mask object
 * @param[in] op Masking Option. Reserved
 *
 * @since 1.24
 *
 * @ingroup Evas_Vg_Node_Group
 */
EVAS_API void evas_vg_node_mask_set(Evas_Vg_Node *obj, Evas_Vg_Node *mask, int op);

#endif
