#ifndef _EFL_CANVAS_VG_OBJECT_EO_LEGACY_H_
#define _EFL_CANVAS_VG_OBJECT_EO_LEGACY_H_

#ifndef _EFL_CANVAS_VG_OBJECT_EO_CLASS_TYPE
#define _EFL_CANVAS_VG_OBJECT_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Vg_Object;

#endif

#ifndef _EFL_CANVAS_VG_OBJECT_EO_TYPES
#define _EFL_CANVAS_VG_OBJECT_EO_TYPES

/** Enumeration that defines how viewbox will be filled int the vg canvs's
 * viewport. default Fill_Mode is @c none
 *
 * @ingroup Efl_Canvas_Vg
 */
typedef enum
{
  EFL_CANVAS_VG_FILL_MODE_NONE = 0, /**< Don't scale the viewbox. Placed it
                                     * inside viewport taking align property
                                     * into account */
  EFL_CANVAS_VG_FILL_MODE_STRETCH, /**< Scale the viewbox so that it matches the
                                    * canvas viewport. Aaspect ratio might be
                                    * changed. */
  EFL_CANVAS_VG_FILL_MODE_MEET, /**< Scale the viewbox so that it fits inside
                                 * canvas viewport while maintaining the aspect
                                 * ratio. At least one of the dimensions of the
                                 * viewbox should be equal to the corresponding
                                 * dimension of the viewport. */
  EFL_CANVAS_VG_FILL_MODE_SLICE /**< Scale the viewbox so that it covers the
                                 * entire canvas viewport while maintaining the
                                 * aspect ratio. At least one of the dimensions
                                 * of the viewbox should be equal to the
                                 * corresponding dimension of the viewport. */
} Efl_Canvas_Vg_Fill_Mode;


#endif

/**
 * @brief Control how the viewbox is mapped to the vg canvas's viewport.
 *
 * @param[in] obj The object.
 * @param[in] fill_mode Fill mode type
 *
 * @ingroup Evas_Object_Vg_Group
 */
EAPI void evas_object_vg_fill_mode_set(Efl_Canvas_Vg_Object *obj, Efl_Canvas_Vg_Fill_Mode fill_mode);

/**
 * @brief Control how the viewbox is mapped to the vg canvas's viewport.
 *
 * @param[in] obj The object.
 *
 * @return Fill mode type
 *
 * @ingroup Evas_Object_Vg_Group
 */
EAPI Efl_Canvas_Vg_Fill_Mode evas_object_vg_fill_mode_get(const Efl_Canvas_Vg_Object *obj);

/**
 * @brief Sets the viewbox for the evas vg canvas. viewbox if set should be
 * mapped to the canvas geometry when rendering the vg tree.
 *
 * @param[in] obj The object.
 * @param[in] viewbox viewbox for the vg canvas
 *
 * @ingroup Evas_Object_Vg_Group
 */
EAPI void evas_object_vg_viewbox_set(Efl_Canvas_Vg_Object *obj, Eina_Rect viewbox);

/**
 * @brief Get the current viewbox from the  evas_object_vg
 *
 * @param[in] obj The object.
 *
 * @return viewbox for the vg canvas
 *
 * @ingroup Evas_Object_Vg_Group
 */
EAPI Eina_Rect evas_object_vg_viewbox_get(const Efl_Canvas_Vg_Object *obj);

/**
 * @brief Control how the viewbox is positioned inside the viewport.
 *
 * @param[in] obj The object.
 * @param[in] align_x Alignment in the horizontal axis (0 <= align_x <= 1).
 * @param[in] align_y Alignment in the vertical axis (0 <= align_y <= 1).
 *
 * @ingroup Evas_Object_Vg_Group
 */
EAPI void evas_object_vg_viewbox_align_set(Efl_Canvas_Vg_Object *obj, double align_x, double align_y);

/**
 * @brief Control how the viewbox is positioned inside the viewport.
 *
 * @param[in] obj The object.
 * @param[out] align_x Alignment in the horizontal axis (0 <= align_x <= 1).
 * @param[out] align_y Alignment in the vertical axis (0 <= align_y <= 1).
 *
 * @ingroup Evas_Object_Vg_Group
 */
EAPI void evas_object_vg_viewbox_align_get(const Efl_Canvas_Vg_Object *obj, double *align_x, double *align_y);

/**
 * @brief Set the root node of the evas_object_vg.
 *
 * @note To manually create the shape object and show in the Vg object canvas
 * you must create the hierarchy and set as root node.
 *
 * It takes the ownership of the root node.
 *
 * @param[in] obj The object.
 * @param[in] root Root node of the VG canvas.
 *
 * @ingroup Evas_Object_Vg_Group
 */
EAPI void evas_object_vg_root_node_set(Efl_Canvas_Vg_Object *obj, Efl_Canvas_Vg_Node *root);

/**
 * @brief Get the root node of the evas_object_vg.
 *
 * @param[in] obj The object.
 *
 * @return Root node of the VG canvas.
 *
 * @since 1.14
 *
 * @ingroup Evas_Object_Vg_Group
 */
EAPI Efl_Canvas_Vg_Node *evas_object_vg_root_node_get(const Efl_Canvas_Vg_Object *obj);

#endif
