/**
 * @defgroup Check Check
 * @ingroup Elementary
 *
 * @image html check_inheritance_tree.png
 * @image latex check_inheritance_tree.eps
 *
 * @image html img/widget/check/preview-00.png
 * @image latex img/widget/check/preview-00.eps
 * @image html img/widget/check/preview-01.png
 * @image latex img/widget/check/preview-01.eps
 * @image html img/widget/check/preview-02.png
 * @image latex img/widget/check/preview-02.eps
 *
 * @brief The check widget allows for toggling a value between true
 * and false.
 *
 * Check objects are a lot like radio objects in layout and
 * functionality, except they do not work as a group, but
 * independently, and only toggle the value of a boolean between false
 * and true. elm_check_state_set() sets the boolean state and
 * elm_check_state_get() returns the current state. For convenience,
 * like the radio objects, you can set a pointer to a boolean directly
 * with elm_check_state_pointer_set() for it to modify.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for check objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "changed" - This is called whenever the user changes the state of
 *             the check objects (@c event_info is always @c NULL).
 *
 * Default content parts of the check widget that you can use for are:
 * @li "icon" - An icon of the check
 *
 * Default text parts of the check widget that you can use for are:
 * @li "default" - A label of the check
 * @li "on" - On state label of the check
 * @li "off" - Off state label of the check
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 *
 * @ref tutorial_check should give you a firm grasp of how to use this widget.
 *
 * @{
 */

#define ELM_OBJ_CHECK_CLASS elm_obj_check_class_get()

const Eo_Class *elm_obj_check_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_CHECK_BASE_ID;

enum
{
      ELM_OBJ_CHECK_SUB_ID_STATE_SET,
      ELM_OBJ_CHECK_SUB_ID_STATE_GET,
      ELM_OBJ_CHECK_SUB_ID_STATE_POINTER_SET,
      ELM_OBJ_CHECK_SUB_ID_LAST
};

#define ELM_OBJ_CHECK_ID(sub_id) (ELM_OBJ_CHECK_BASE_ID + sub_id)


/**
 * @def elm_obj_check_state_set
 * @since 1.8
 *
 * @brief Set the on/off state of the check object
 *
 * @param[in] state
 *
 * @see elm_check_state_set
 */
#define elm_obj_check_state_set(state) ELM_OBJ_CHECK_ID(ELM_OBJ_CHECK_SUB_ID_STATE_SET), EO_TYPECHECK(Eina_Bool, state)

/**
 * @def elm_obj_check_state_get
 * @since 1.8
 *
 * @brief Get the state of the check object
 *
 * @param[out] ret
 *
 * @see elm_check_state_get
 */
#define elm_obj_check_state_get(ret) ELM_OBJ_CHECK_ID(ELM_OBJ_CHECK_SUB_ID_STATE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_check_state_pointer_set
 * @since 1.8
 *
 * @brief Set a convenience pointer to a boolean to change
 *
 * @param[in] statep
 *
 * @see elm_check_state_pointer_set
 */
#define elm_obj_check_state_pointer_set(statep) ELM_OBJ_CHECK_ID(ELM_OBJ_CHECK_SUB_ID_STATE_POINTER_SET), EO_TYPECHECK(Eina_Bool *, statep)

/**
 * @brief Add a new Check object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Check
 */
EAPI Evas_Object *                elm_check_add(Evas_Object *parent);

/**
 * @brief Set the on/off state of the check object
 *
 * @param obj The check object
 * @param state The state to use (1 == on, 0 == off)
 *
 * This sets the state of the check. If set with elm_check_state_pointer_set()
 * the state of that variable is also changed. Calling this @b doesn't cause
 * the "changed" signal to be emitted.
 *
 * @ingroup Check
 */
EAPI void                         elm_check_state_set(Evas_Object *obj, Eina_Bool state);

/**
 * @brief Get the state of the check object
 *
 * @param obj The check object
 * @return The boolean state
 *
 * @ingroup Check
 */
EAPI Eina_Bool                    elm_check_state_get(const Evas_Object *obj);

/**
 * @brief Set a convenience pointer to a boolean to change
 *
 * @param obj The check object
 * @param statep Pointer to the boolean to modify
 *
 * This sets a pointer to a boolean, that, in addition to the check objects
 * state will also be modified directly. To stop setting the object pointed
 * to simply use NULL as the @p statep parameter. If @p statep is not NULL,
 * then when this is called, the check objects state will also be modified to
 * reflect the value of the boolean @p statep points to, just like calling
 * elm_check_state_set().
 *
 * @ingroup Check
 */
EAPI void                         elm_check_state_pointer_set(Evas_Object *obj, Eina_Bool *statep);

/**
 * @}
 */
