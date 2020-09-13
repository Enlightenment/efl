#ifndef _EVAS_VG_CONTAINER_EO_LEGACY_H_
#define _EVAS_VG_CONTAINER_EO_LEGACY_H_

#ifndef _EVAS_VG_CONTAINER_EO_CLASS_TYPE
#define _EVAS_VG_CONTAINER_EO_CLASS_TYPE

typedef Eo Evas_Vg_Container;

#endif

#ifndef _EVAS_VG_CONTAINER_EO_TYPES
#define _EVAS_VG_CONTAINER_EO_TYPES


#endif

/**
 * @brief Get child of container.
 *
 * @param[in] obj The object.
 * @param[in] name The Child node name.
 *
 * @return The child object.
 *
 * @since 1.24
 *
 * @ingroup Evas_Vg_Container_Group
 */
EVAS_API Evas_Vg_Node *evas_vg_container_child_get(Evas_Vg_Container *obj, const char *name);

/**
 * @brief Get all children of container.
 *
 * @param[in] obj The object.
 *
 * @return The iterator to children.
 *
 * @since 1.24
 *
 * @ingroup Evas_Vg_Container_Group
 */
EVAS_API Eina_Iterator *evas_vg_container_children_get(Evas_Vg_Container *obj) EINA_WARN_UNUSED_RESULT;

#endif
