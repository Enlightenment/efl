#ifndef _EFL_CANVAS_VG_CONTAINER_EO_LEGACY_H_
#define _EFL_CANVAS_VG_CONTAINER_EO_LEGACY_H_

#ifndef _EFL_CANVAS_VG_CONTAINER_EO_CLASS_TYPE
#define _EFL_CANVAS_VG_CONTAINER_EO_CLASS_TYPE

typedef Eo Efl_Canvas_Vg_Container;

#endif

#ifndef _EFL_CANVAS_VG_CONTAINER_EO_TYPES
#define _EFL_CANVAS_VG_CONTAINER_EO_TYPES


#endif

/**
 * @brief Get child of container
 *
 * @param[in] obj The object.
 * @param[in] name Child node name
 *
 * @return Child object
 *
 * @ingroup Evas_Vg_Container_Group
 */
EAPI Efl_Canvas_Vg_Node *evas_vg_container_child_get(Efl_Canvas_Vg_Container *obj, const char *name);

/**
 * @brief Get all children of container
 *
 * @param[in] obj The object.
 *
 * @return Iterator to children
 *
 * @ingroup Evas_Vg_Container_Group
 */
EAPI Eina_Iterator *evas_vg_container_children_get(Efl_Canvas_Vg_Container *obj) EINA_WARN_UNUSED_RESULT;

#endif
