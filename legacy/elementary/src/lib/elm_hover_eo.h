#define ELM_OBJ_HOVER_CLASS elm_obj_hover_class_get()

const Eo_Class *elm_obj_hover_class_get(void) EINA_CONST;

 extern EAPI Eo_Op ELM_OBJ_HOVER_BASE_ID;

 enum
   {
      ELM_OBJ_HOVER_SUB_ID_TARGET_SET,
      ELM_OBJ_HOVER_SUB_ID_TARGET_GET,
      ELM_OBJ_HOVER_SUB_ID_BEST_CONTENT_LOCATION_GET,
      ELM_OBJ_HOVER_SUB_ID_DISMISS,
      ELM_OBJ_HOVER_SUB_ID_LAST
   };

#define ELM_OBJ_HOVER_ID(sub_id) (ELM_OBJ_HOVER_BASE_ID + sub_id)


/**
 * @def elm_obj_hover_target_set
 * @since 1.8
 *
 * @brief Sets the target object for the hover.
 *
 * @param[in] target
 *
 * @see elm_hover_target_set
 *
 * @ingroup Hover
 */
#define elm_obj_hover_target_set(target) ELM_OBJ_HOVER_ID(ELM_OBJ_HOVER_SUB_ID_TARGET_SET), EO_TYPECHECK(Evas_Object *, target)

/**
 * @def elm_obj_hover_target_get
 * @since 1.8
 *
 * @brief Get the target object for the hover.
 *
 * @param[out] ret
 *
 * @see elm_hover_target_get
 *
 * @ingroup Hover
 */
#define elm_obj_hover_target_get(ret) ELM_OBJ_HOVER_ID(ELM_OBJ_HOVER_SUB_ID_TARGET_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_hover_best_content_location_get
 * @since 1.8
 *
 * @brief Returns the best swallow location for content in the hover.
 *
 * @param[in] pref_axis
 * @param[out] ret
 *
 * @see elm_hover_best_content_location_get
 *
 * @ingroup Hover
 */
#define elm_obj_hover_best_content_location_get(pref_axis, ret) ELM_OBJ_HOVER_ID(ELM_OBJ_HOVER_SUB_ID_BEST_CONTENT_LOCATION_GET), EO_TYPECHECK(Elm_Hover_Axis, pref_axis), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_hover_dismiss
 * @since 1.8
 *
 * @brief Dismiss a hover object
 *
 *
 * @see elm_hover_dismiss
 */
#define elm_obj_hover_dismiss() ELM_OBJ_HOVER_ID(ELM_OBJ_HOVER_SUB_ID_DISMISS)
