#define ELM_OBJ_ICON_CLASS elm_obj_icon_class_get()

const Eo_Class *elm_obj_icon_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_ICON_BASE_ID;

enum
{
   ELM_OBJ_ICON_SUB_ID_THUMB_SET,
   ELM_OBJ_ICON_SUB_ID_STANDARD_SET,
   ELM_OBJ_ICON_SUB_ID_STANDARD_GET,
   ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_SET,
   ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_GET,
   ELM_OBJ_ICON_SUB_ID_LAST
};

#define ELM_OBJ_ICON_ID(sub_id) (ELM_OBJ_ICON_BASE_ID + sub_id)


/**
 * @def elm_obj_icon_thumb_set
 * @since 1.8
 *
 * Set the file that will be used, but use a generated thumbnail.
 *
 * @param[in] file
 * @param[in] group
 *
 * @see elm_icon_thumb_set
 *
 * @ingroup Icon
 */
#define elm_obj_icon_thumb_set(file, group) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_THUMB_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char *, group)

/**
 * @def elm_obj_icon_standard_set
 * @since 1.8
 *
 * Set the icon by icon standards names.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_icon_standard_set
 *
 * @ingroup Icon
 */
#define elm_obj_icon_standard_set(name, ret) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_STANDARD_SET), EO_TYPECHECK(const char *, name), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_icon_standard_get
 * @since 1.8
 *
 * Get the icon name set by icon standard names.
 *
 * @param[out] ret
 *
 * @see elm_icon_standard_get
 *
 * @ingroup Icon
 */
#define elm_obj_icon_standard_get(ret) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_STANDARD_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_icon_order_lookup_set
 * @since 1.8
 *
 * Sets the icon lookup order used by elm_icon_standard_set().
 *
 * @param[in] order
 *
 * @see elm_icon_order_lookup_set
 *
 * @ingroup Icon
 */
#define elm_obj_icon_order_lookup_set(order) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_SET), EO_TYPECHECK(Elm_Icon_Lookup_Order, order)

/**
 * @def elm_obj_icon_order_lookup_get
 * @since 1.8
 *
 * Get the icon lookup order.
 *
 * @param[out] ret
 *
 * @see elm_icon_order_lookup_get
 *
 * @ingroup Icon
 */
#define elm_obj_icon_order_lookup_get(ret) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_GET), EO_TYPECHECK(Elm_Icon_Lookup_Order *, ret)
