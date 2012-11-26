/**
 * @defgroup Radio Radio
 * @ingroup Elementary
 *
 * @image html radio_inheritance_tree.png
 * @image latex radio_inheritance_tree.eps
 *
 * @image html img/widget/radio/preview-00.png
 * @image latex img/widget/radio/preview-00.eps
 *
 * @brief Radio is a widget that allows for 1 or more options to be displayed
 * and have the user choose only 1 of them.
 *
 * A radio object contains an indicator, an optional Label and an optional
 * icon object. While it's possible to have a group of only one radio they,
 * are normally used in groups of 2 or more.
 *
 * elm_radio objects are grouped in a slightly different, compared to other
 * UI toolkits. There is no seperate group name/id to remember or manage.
 * The members represent the group, there are the group. To make a group,
 * use elm_radio_group_add() and pass existing radio object and the new radio
 * object.
 *
 * The radio object(s) will select from one of a set
 * of integer values, so any value they are configuring needs to be mapped to
 * a set of integers. To configure what value that radio object represents,
 * use  elm_radio_state_value_set() to set the integer it represents. To set
 * the value the whole group(which one is currently selected) is to indicate
 * use elm_radio_value_set() on any group member, and to get the groups value
 * use elm_radio_value_get(). For convenience the radio objects are also able
 * to directly set an integer(int) to the value that is selected. To specify
 * the pointer to this integer to modify, use elm_radio_value_pointer_set().
 * The radio objects will modify this directly. That implies the pointer must
 * point to valid memory for as long as the radio objects exist.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for radio objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li changed - This is called whenever the user changes the state of one of
 * the radio objects within the group of radio objects that work together.
 *
 * Default text parts of the radio widget that you can use for are:
 * @li "default" - Label of the radio
 *
 * Default content parts of the radio widget that you can use for are:
 * @li "icon" - An icon of the radio
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 *
 * @ref tutorial_radio show most of this API in action.
 * @{
 */

#define ELM_OBJ_RADIO_CLASS elm_obj_radio_class_get()

const Eo_Class *elm_obj_radio_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_RADIO_BASE_ID;

enum
{
   ELM_OBJ_RADIO_SUB_ID_GROUP_ADD,
   ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_SET,
   ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_GET,
   ELM_OBJ_RADIO_SUB_ID_VALUE_SET,
   ELM_OBJ_RADIO_SUB_ID_VALUE_GET,
   ELM_OBJ_RADIO_SUB_ID_VALUE_POINTER_SET,
   ELM_OBJ_RADIO_SUB_ID_SELECTED_OBJECT_GET,
   ELM_OBJ_RADIO_SUB_ID_LAST
};

#define ELM_OBJ_RADIO_ID(sub_id) (ELM_OBJ_RADIO_BASE_ID + sub_id)


/**
 * @def elm_obj_radio_group_add
 * @since 1.8
 *
 * @brief Add this radio to a group of other radio objects
 *
 * @param[in] group
 *
 * @see elm_radio_group_add
 */
#define elm_obj_radio_group_add(group) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_GROUP_ADD), EO_TYPECHECK(Evas_Object *, group)

/**
 * @def elm_obj_radio_state_value_set
 * @since 1.8
 *
 * @brief Set the integer value that this radio object represents
 *
 * @param[in] value
 *
 * @see elm_radio_state_value_set
 */
#define elm_obj_radio_state_value_set(value) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_SET), EO_TYPECHECK(int, value)

/**
 * @def elm_obj_radio_state_value_get
 * @since 1.8
 *
 * @brief Get the integer value that this radio object represents
 *
 * @param[out] ret
 *
 * @see elm_radio_state_value_get
 */
#define elm_obj_radio_state_value_get(ret) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_STATE_VALUE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_radio_value_set
 * @since 1.8
 *
 * @brief Set the value of the radio group.
 *
 * @param[in] value
 *
 * @see elm_radio_value_set
 */
#define elm_obj_radio_value_set(value) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_VALUE_SET), EO_TYPECHECK(int, value)

/**
 * @def elm_obj_radio_value_get
 * @since 1.8
 *
 * @brief Get the value of the radio group
 *
 * @param[out] ret
 *
 * @see elm_radio_value_get
 */
#define elm_obj_radio_value_get(ret) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_VALUE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_radio_value_pointer_set
 * @since 1.8
 *
 * @brief Set a convenience pointer to a integer to change when radio group
 *
 * @param[in] valuep
 *
 * @see elm_radio_value_pointer_set
 */
#define elm_obj_radio_value_pointer_set(valuep) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_VALUE_POINTER_SET), EO_TYPECHECK(int *, valuep)

/**
 * @def elm_obj_radio_selected_object_get
 * @since 1.8
 *
 * @brief Get the selected radio object.
 *
 * @param[out] ret
 *
 * @see elm_radio_selected_object_get
 */
#define elm_obj_radio_selected_object_get(ret) ELM_OBJ_RADIO_ID(ELM_OBJ_RADIO_SUB_ID_SELECTED_OBJECT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @brief Add a new radio to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Radio
 */
EAPI Evas_Object                 *elm_radio_add(Evas_Object *parent);

/**
 * @brief Add this radio to a group of other radio objects
 *
 * @param obj The radio object
 * @param group Any object whose group the @p obj is to join.
 *
 * Radio objects work in groups. Each member should have a different integer
 * value assigned. In order to have them work as a group, they need to know
 * about each other. This adds the given radio object to the group of which
 * the group object indicated is a member.
 *
 * @ingroup Radio
 */
EAPI void                         elm_radio_group_add(Evas_Object *obj, Evas_Object *group);

/**
 * @brief Set the integer value that this radio object represents
 *
 * @param obj The radio object
 * @param value The value to use if this radio object is selected
 *
 * This sets the value of the radio.
 *
 * @ingroup Radio
 */
EAPI void                         elm_radio_state_value_set(Evas_Object *obj, int value);

/**
 * @brief Get the integer value that this radio object represents
 *
 * @param obj The radio object
 * @return The value used if this radio object is selected
 *
 * This gets the value of the radio.
 * @see elm_radio_value_set()
 *
 * @ingroup Radio
 */
EAPI int                          elm_radio_state_value_get(const Evas_Object *obj);

/**
 * @brief Set the value of the radio group.
 *
 * @param obj The radio object (any radio object of the group).
 * @param value The value to use for the group
 *
 * This sets the value of the radio group and will also set the value if
 * pointed to, to the value supplied, but will not call any callbacks.
 *
 * @ingroup Radio
 */
EAPI void                         elm_radio_value_set(Evas_Object *obj, int value);

/**
 * @brief Get the value of the radio group
 *
 * @param obj The radio object (any radio object of the group).
 * @return The integer state
 *
 * @ingroup Radio
 */
EAPI int                          elm_radio_value_get(const Evas_Object *obj);

/**
 * @brief Set a convenience pointer to a integer to change when radio group
 * value changes.
 *
 * @param obj The radio object (any object of a group)
 * @param valuep Pointer to the integer to modify
 *
 * This sets a pointer to a integer, that, in addition to the radio objects
 * state will also be modified directly. To stop setting the object pointed
 * to simply use NULL as the @p valuep argument. If valuep is not NULL, then
 * when this is called, the radio objects state will also be modified to
 * reflect the value of the integer valuep points to, just like calling
 * elm_radio_value_set().
 *
 * @ingroup Radio
 */
EAPI void                         elm_radio_value_pointer_set(Evas_Object *obj, int *valuep);

/**
 * @brief Get the selected radio object.
 *
 * @param obj Any radio object (any object of a group)
 * @return The selected radio object
 *
 * @ingroup Radio
 */
EAPI Evas_Object                 *elm_radio_selected_object_get(Evas_Object *obj);

/**
 * @}
 */
