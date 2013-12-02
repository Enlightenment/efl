/**
 * @ingroup Popup
 *
 * @{
 */
#define ELM_OBJ_POPUP_CLASS elm_obj_popup_class_get()

const Eo_Class *elm_obj_popup_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_POPUP_BASE_ID;

enum
{
   ELM_OBJ_POPUP_SUB_ID_CONTENT_TEXT_WRAP_TYPE_SET,
   ELM_OBJ_POPUP_SUB_ID_CONTENT_TEXT_WRAP_TYPE_GET,
   ELM_OBJ_POPUP_SUB_ID_ORIENT_SET,
   ELM_OBJ_POPUP_SUB_ID_ORIENT_GET,
   ELM_OBJ_POPUP_SUB_ID_TIMEOUT_SET,
   ELM_OBJ_POPUP_SUB_ID_TIMEOUT_GET,
   ELM_OBJ_POPUP_SUB_ID_ALLOW_EVENTS_SET,
   ELM_OBJ_POPUP_SUB_ID_ALLOW_EVENTS_GET,
   ELM_OBJ_POPUP_SUB_ID_ITEM_APPEND,
   ELM_OBJ_POPUP_SUB_ID_ALIGN_SET,
   ELM_OBJ_POPUP_SUB_ID_ALIGN_GET,
   ELM_OBJ_POPUP_SUB_ID_LAST
};

#define ELM_OBJ_POPUP_ID(sub_id) (ELM_OBJ_POPUP_BASE_ID + sub_id)


/**
 * @def elm_obj_popup_content_text_wrap_type_set
 * @since 1.8
 *
 * @brief Sets the wrapping type of content text packed in content
 *
 * @param[in] wrap
 *
 * @see elm_popup_content_text_wrap_type_set
 */
#define elm_obj_popup_content_text_wrap_type_set(wrap) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_CONTENT_TEXT_WRAP_TYPE_SET), EO_TYPECHECK(Elm_Wrap_Type, wrap)

/**
 * @def elm_obj_popup_content_text_wrap_type_get
 * @since 1.8
 *
 * @brief Returns the wrapping type of content text packed in content area of
 * area of popup object.
 *
 * @param[out] ret
 *
 * @see elm_popup_content_text_wrap_type_get
 */
#define elm_obj_popup_content_text_wrap_type_get(ret) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_CONTENT_TEXT_WRAP_TYPE_GET), EO_TYPECHECK(Elm_Wrap_Type *, ret)

/**
 * @def elm_obj_popup_orient_set
 * @since 1.8
 *
 * @brief Sets the orientation of the popup in the parent region
 *
 * @param[in] orient
 *
 * @see elm_popup_orient_set
 */
#define elm_obj_popup_orient_set(orient) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ORIENT_SET), EO_TYPECHECK(Elm_Popup_Orient, orient)

/**
 * @def elm_obj_popup_orient_get
 * @since 1.8
 *
 * @brief Returns the orientation of Popup
 *
 * @param[out] ret
 *
 * @see elm_popup_orient_get
 */
#define elm_obj_popup_orient_get(ret) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ORIENT_GET), EO_TYPECHECK(Elm_Popup_Orient *, ret)

/**
 * @def elm_obj_popup_timeout_set
 * @since 1.8
 *
 * @brief Sets a timeout to hide popup automatically
 *
 * @param[in] timeout
 *
 * @see elm_popup_timeout_set
 */
#define elm_obj_popup_timeout_set(timeout) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_TIMEOUT_SET), EO_TYPECHECK(double, timeout)

/**
 * @def elm_obj_popup_timeout_get
 * @since 1.8
 *
 * @brief Returns the timeout value set to the popup (in seconds)
 *
 * @param[out] ret
 *
 * @see elm_popup_timeout_get
 */
#define elm_obj_popup_timeout_get(ret) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_TIMEOUT_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_popup_allow_events_set
 * @since 1.8
 *
 * @brief Sets whether events should be passed to by a click outside.
 *
 * @param[in] allow
 *
 * @see elm_popup_allow_events_set
 */
#define elm_obj_popup_allow_events_set(allow) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ALLOW_EVENTS_SET), EO_TYPECHECK(Eina_Bool, allow)

/**
 * @def elm_obj_popup_allow_events_get
 * @since 1.8
 *
 * @brief Returns value indicating whether allow event is enabled or not
 *
 * @param[out] ret
 *
 * @see elm_popup_allow_events_get
 */
#define elm_obj_popup_allow_events_get(ret) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ALLOW_EVENTS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_popup_item_append
 * @since 1.8
 *
 * @brief Add a new item to a Popup object
 *
 * @param[in] label
 * @param[in] icon
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_popup_item_append
 */
#define elm_obj_popup_item_append(label, icon, func, data, ret) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ITEM_APPEND), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Object *, icon), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_popup_align_set
 * @since 1.9
 *
 * @brief Set the alignment of the popup object relative to its parent
 *
 * @param[in] horizontal
 * @param[in] vertical
 *
 * @see elm_popup_align_set
 */
#define elm_obj_popup_align_set(horizontal, vertical) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ALIGN_SET), EO_TYPECHECK(double, horizontal), EO_TYPECHECK(double, vertical)

/**
 * @def elm_obj_popup_align_get
 * @since 1.9
 *
 * @brief Return the alignment of the popup object relative to its parent
 *
 * @param[out] horizontal
 * @param[out] vertical
 *
 * @see elm_popup_align_get
 */
#define elm_obj_popup_align_get(horizontal, vertical) ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ALIGN_GET), EO_TYPECHECK(double *, horizontal), EO_TYPECHECK(double *, vertical)
/**
 * @}
 */
