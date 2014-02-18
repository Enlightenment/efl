#ifndef ELM_WIDGET_CONTAINER_H
#define ELM_WIDGET_CONTAINER_H

#include <elm_widget.h>

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-container-class The Elementary Container Class
 *
 * @image html container_inheritance_tree.png
 * @image latex container_inheritance_tree.eps
 *
 * This class defines a common interface for objects acting like
 * containers, i.e. objects parenting others and displaying their
 * childs "inside" of them somehow.
 *
 * The container must define "parts" (or spots) into which child
 * objects will be placed, inside of it. This is a way of handling
 * more the one content object, by naming content locations
 * properly. This is the role of the @c name argument of the virtual
 * functions in the class.
 *
 * The following object functions are meant to be used with all
 * container objects and derived ones:
 *
 * - elm_object_part_content_set()
 * - elm_object_part_content_get()
 * - elm_object_part_content_unset()
 */

#define ELM_OBJ_CONTAINER_CLASS elm_obj_container_class_get()

const Eo_Class *elm_obj_container_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_CONTAINER_BASE_ID;

enum
{
   ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET,
   ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET,
   ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET,
   ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SWALLOW_LIST_GET,
   ELM_OBJ_CONTAINER_SUB_ID_LAST
};
/**
 * @}
 */

#define ELM_OBJ_CONTAINER_ID(sub_id) (ELM_OBJ_CONTAINER_BASE_ID + sub_id)


/**
 * @def elm_obj_container_content_set
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] name
 * @param[in] content
 * @param[out] ret
 *
 */
#define elm_obj_container_content_set(name, content, ret) ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), EO_TYPECHECK(const char*, name), EO_TYPECHECK(Evas_Object *,content), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_container_content_get
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] name
 * @param[out] ret
 *
 */
#define elm_obj_container_content_get(name, ret) ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), EO_TYPECHECK(const char*, name), EO_TYPECHECK(Evas_Object **,ret)

/**
 * @def elm_obj_container_content_unset
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] name
 * @param[out] ret
 *
 */
#define elm_obj_container_content_unset(name, ret) ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), EO_TYPECHECK(const char*, name), EO_TYPECHECK(Evas_Object **,ret)

/**
 * @def elm_obj_container_content_swallow_list_get
 * @since 1.9
 *
 * No description supplied by the EAPI.
 *
 * @param[out] ret Eina_List **
 *
 */
#define elm_obj_container_content_swallow_list_get(ret) ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SWALLOW_LIST_GET), EO_TYPECHECK(Eina_List **,ret)

#endif
