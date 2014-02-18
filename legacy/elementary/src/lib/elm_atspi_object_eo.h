void _elm_atspi_object_global_callback_add(Eo_Event_Cb cv, void *user_data);
void _elm_atspi_object_global_callback_del(Eo_Event_Cb cv);

#define ELM_ATSPI_CLASS elm_atspi_obj_class_get()
const Eo_Class *elm_atspi_obj_class_get(void) EINA_CONST;

#define ELM_ATSPI_WIDGET_CLASS elm_atspi_widget_obj_class_get()
const Eo_Class *elm_atspi_widget_obj_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_ATSPI_OBJ_BASE_ID;

enum
{
   ELM_ATSPI_OBJ_SUB_ID_NAME_GET, /* virtual */
   ELM_ATSPI_OBJ_SUB_ID_DESCRIPTION_GET, /* virtual */
   ELM_ATSPI_OBJ_SUB_ID_CHILD_AT_INDEX_GET,
   ELM_ATSPI_OBJ_SUB_ID_CHILDREN_GET, /* virtual */
   ELM_ATSPI_OBJ_SUB_ID_PARENT_GET, /* virtual */
   ELM_ATSPI_OBJ_SUB_ID_OBJECT_GET,
   ELM_ATSPI_OBJ_SUB_ID_INDEX_IN_PARENT_GET,
   ELM_ATSPI_OBJ_SUB_ID_RELATION_SET_GET, /* virtual */
   ELM_ATSPI_OBJ_SUB_ID_ROLE_GET, /* virtual */
   ELM_ATSPI_OBJ_SUB_ID_ROLE_NAME_GET,
   ELM_ATSPI_OBJ_SUB_ID_LOCALIZED_ROLE_NAME_GET,
   ELM_ATSPI_OBJ_SUB_ID_STATE_GET, /* virtual */
   ELM_ATSPI_OBJ_SUB_ID_ATTRIBUTES_GET, /* virtual */
   ELM_ATSPI_OBJ_SUB_ID_LAST
};

#define ELM_ATSPI_OBJ_ID(sub_id) (ELM_ATSPI_OBJ_BASE_ID + sub_id)

/* Elm_Atspi_Object events */
extern const Eo_Event_Description _EV_ATSPI_OBJ_NAME_CHANGED;
#define EV_ATSPI_OBJ_NAME_CHANGED (&(_EV_ATSPI_OBJ_NAME_CHANGED))

extern const Eo_Event_Description _EV_ATSPI_OBJ_CHILD_ADD;
#define EV_ATSPI_OBJ_CHILD_ADD (&(_EV_ATSPI_OBJ_CHILD_ADD))

extern const Eo_Event_Description _EV_ATSPI_OBJ_CHILD_DEL;
#define EV_ATSPI_OBJ_CHILD_DEL (&(_EV_ATSPI_OBJ_CHILD_DEL))

extern const Eo_Event_Description _EV_ATSPI_OBJ_STATE_CHANGED;
#define EV_ATSPI_OBJ_STATE_CHANGED (&(_EV_ATSPI_OBJ_STATE_CHANGED))

/* Component Interface */
#define ELM_ATSPI_COMPONENT_INTERFACE elm_atspi_component_interface_get()

const Eo_Class *elm_atspi_component_interface_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_ATSPI_COMPONENT_INTERFACE_BASE_ID;

enum
{
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_CONTAINS,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_ACCESSIBLE_AT_POINT_GET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_EXTENTS_GET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_EXTENTS_SET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_POSITION_GET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_POSITION_SET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_SIZE_GET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_SIZE_SET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_LAYER_GET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_Z_ORDER_GET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_FOCUS_GRAB,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_ALPHA_GET,
   ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_LAST
};

#define ELM_ATSPI_COMPONENT_INTERFACE_ID(sub_id) (ELM_ATSPI_COMPONENT_INTERFACE_BASE_ID + sub_id)
/* Component Interface - END */

/* Window Interface */
#define ELM_ATSPI_WINDOW_INTERFACE elm_atspi_window_interface_get()
const Eo_Class *elm_atspi_window_interface_get(void) EINA_CONST;

extern const Eo_Event_Description _EV_ATSPI_OBJ_WINDOW_ACTIVATED;
#define EV_ATSPI_OBJ_WINDOW_ACTIVATED (&(_EV_ATSPI_OBJ_WINDOW_ACTIVATED))

extern const Eo_Event_Description _EV_ATSPI_OBJ_WINDOW_DEACTIVATED;
#define EV_ATSPI_OBJ_WINDOW_DEACTIVATED (&(_EV_ATSPI_OBJ_WINDOW_DEACTIVATED))
/* Window Interface - END */

/* Action Interface */
#define ELM_ATSPI_ACTION_INTERFACE elm_accessible_action_interface_get()

const Eo_Class *elm_accessible_action_interface_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_ATSPI_ACTION_INTERFACE_BASE_ID;

enum
{
   ELM_ATSPI_ACTION_INTERFACE_SUB_ID_ACTION_COUNT,
   ELM_ATSPI_ACTION_INTERFACE_SUB_ID_ACTION_DO,
   ELM_ATSPI_ACTION_INTERFACE_SUB_ID_DESCRIPTION_GET,
   ELM_ATSPI_ACTION_INTERFACE_SUB_ID_NAME_GET,
   ELM_ATSPI_ACTION_INTERFACE_SUB_ID_LOCALIZED_NAME_GET,
   ELM_ATSPI_ACTION_INTERFACE_SUB_ID_KEY_BINDING_GET,
   ELM_ATSPI_ACTION_INTERFACE_SUB_ID_LAST
};
/* Action Interface - END */

/* Text Interface */
#define ELM_ATSPI_TEXT_INTERFACE elm_accessible_text_interface_get()

const Eo_Class *elm_accessible_text_interface_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_ATSPI_TEXT_INTERFACE_BASE_ID;

enum
{
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_CHARACTER_COUNT,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_CARET_OFFSET_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_CARET_OFFSET_SET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_TEXT_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_TEXT_BEFORE_OFFSET_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_TEXT_AT_OFFSET_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_TEXT_AFTER_OFFSET_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_CHARACTER_AT_OFFSET_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_ATTRIBUTE_VALUE_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_ATTRIBUTES_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_DEFAULT_ATTRIBUTES_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_CHARACTER_EXTENTS_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_OFFSET_AT_POINT_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_SELECTIONS_COUNT,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_SELECTION_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_SELECTION_ADD,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_SELECTION_REMOVE,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_SELECTION_SET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_RANGE_EXTENTS_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_BAOUNDED_RANGES_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_ATTRIBUTE_RUN_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_DEFAULT_ATTRIBUTE_SET_GET,
   ELM_ATSPI_TEXT_INTERFACE_SUB_ID_LAST
};
/* Text Interface - END */


/* EditableText Interface */
#define ELM_ATSPI_EDITABLE_TEXT_INTERFACE elm_accessible_editable_text_interface_get()

const Eo_Class *elm_accessible_editable_text_interface_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_ATSPI_EDITABLE_TEXT_INTERFACE_BASE_ID;

enum
{
   ELM_ATSPI_EDITABLE_TEXT_INTERFACE_SUB_ID_TEXT_CONTENTS_SET,
   ELM_ATSPI_EDITABLE_TEXT_INTERFACE_SUB_ID_TEXT_INSERT,
   ELM_ATSPI_EDITABLE_TEXT_INTERFACE_SUB_ID_TEXT_COPY,
   ELM_ATSPI_EDITABLE_TEXT_INTERFACE_SUB_ID_TEXT_CUT,
   ELM_ATSPI_EDITABLE_TEXT_INTERFACE_SUB_ID_TEXT_DELETE,
   ELM_ATSPI_EDITABLE_TEXT_INTERFACE_SUB_ID_TEXT_PASTE,
   ELM_ATSPI_EDITABLE_TEXT_INTERFACE_SUB_ID_LAST
};
/* EditableText Interface - END */

/* Value Interface */
#define ELM_ATSPI_VALUE_INTERFACE elm_accessible_value_interface_get()

const Eo_Class *elm_accessible_value_interface_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_ATSPI_VALUE_INTERFACE_BASE_ID;

enum
{
   ELM_ATSPI_VALUE_INTERFACE_SUB_ID_MINIMUM_GET,
   ELM_ATSPI_VALUE_INTERFACE_SUB_ID_MAXIMUM_GET,
   ELM_ATSPI_VALUE_INTERFACE_SUB_ID_MINIMUM_INCREMENT_GET,
   ELM_ATSPI_VALUE_INTERFACE_SUB_ID_VALUE_GET,
   ELM_ATSPI_VALUE_INTERFACE_SUB_ID_VALUE_SET,
   ELM_ATSPI_VALUE_INTERFACE_SUB_ID_LAST
};
/* Value Interface - END */


/* Image Interface */
const Eo_Class *elm_accessible_image_interface_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_ATSPI_IMAGE_INTERFACE_BASE_ID;

enum
{
   ELM_ATSPI_IMAGE_INTERFACE_SUB_ID_IMAGE_DESCRIPTION_GET,
   ELM_ATSPI_IMAGE_INTERFACE_SUB_ID_IMAGE_LOCALE_GET,
   ELM_ATSPI_IMAGE_INTERFACE_SUB_ID_IMAGE_EXTENTS_GET,
   ELM_ATSPI_IMAGE_INTERFACE_SUB_ID_IMAGE_POSITION_GET,
   ELM_ATSPI_IMAGE_INTERFACE_SUB_ID_IMAGE_SIZE_GET,
   ELM_ATSPI_IMAGE_INTERFACE_SUB_ID_LAST
};
/* Image Interface - END */

/* Selection Interface */
const Eo_Class *elm_accessible_selection_interface_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_ATSPI_SELECTION_INTERFACE_BASE_ID;

enum
{
   ELM_ATSPI_SELECTION_INTERFACE_SUB_ID_SELECTED_CHILDREN_COUNT,
   ELM_ATSPI_SELECTION_INTERFACE_SUB_ID_SELECTED_CHILD_GET,
   ELM_ATSPI_SELECTION_INTERFACE_SUB_ID_CHILD_SELECT,
   ELM_ATSPI_SELECTION_INTERFACE_SUB_ID_CHILD_DESELECT,
   ELM_ATSPI_SELECTION_INTERFACE_SUB_ID_SELECT_ALL,
   ELM_ATSPI_SELECTION_INTERFACE_SUB_ID_SELECTION_CLEAR,
   ELM_ATSPI_SELECTION_INTERFACE_SUB_ID_LAST
};
/* Selection Interface - END */

/**
 * @def elm_atspi_obj_name_get
 * @since 1.10
 *
 * @param[out] ret obj name
 */
#define elm_atspi_obj_name_get(ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_NAME_GET), \
   EO_TYPECHECK(const char **, ret)

/**
 * @def elm_atspi_obj_role_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_obj_role_get(role)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_ROLE_GET), \
   EO_TYPECHECK(AtspiRole*, role)

/**
 * @def elm_atspi_obj_role_name_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_obj_role_name_get(ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_ROLE_NAME_GET),\
   EO_TYPECHECK(const char **, ret)

/**
 * @def elm_atspi_obj_localized_role_name_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_obj_localized_role_name_get(ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_LOCALIZED_ROLE_NAME_GET),\
   EO_TYPECHECK(const char **, ret)

/**
 * @def elm_atspi_obj_description_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_obj_description_get(ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_DESCRIPTION_GET),\
   EO_TYPECHECK(const char **, ret)

/**
 * @def elm_atspi_obj_child_at_index_get
 * @since 1.10
 *
 * @param[in] idx
 * @param[out] ret
 */
#define elm_atspi_obj_child_at_index_get(idx, ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_CHILD_AT_INDEX_GET), \
   EO_TYPECHECK(int, idx),\
   EO_TYPECHECK(Elm_Atspi_Object**, ret)

/**
 * @def elm_atspi_obj_children_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_obj_children_get(ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_CHILDREN_GET),\
   EO_TYPECHECK(Eina_List**, ret)

/**
 * @def elm_atspi_obj_parent_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_obj_parent_get(ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_PARENT_GET),\
   EO_TYPECHECK(Elm_Atspi_Object**, ret)

/**
 * @def elm_atspi_obj_object_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_obj_object_get(ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_OBJECT_GET),\
   EO_TYPECHECK(Evas_Object**, ret)

/**
 * @def elm_atspi_obj_index_in_parent_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_obj_index_in_parent_get(ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_INDEX_IN_PARENT_GET),\
   EO_TYPECHECK(int*, ret)

/**
 * @def elm_atspi_obj_relation_set_get
 * @since 1.10
 */
#define elm_atspi_obj_relation_set_get()\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_RELATION_SET_GET),\
   EO_TYPECHECK()

/**
 * @def elm_atspi_obj_state_get
 * @since 1.10
 *
 * @param[out] ret state
 */
#define elm_atspi_obj_state_get(ret)\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_STATE_GET),\
   EO_TYPECHECK(Elm_Atspi_State*, ret)

/**
 * @def elm_atspi_obj_attributes_get
 * @since 1.10
 *
 */
#define elm_atspi_obj_attributes_get()\
   ELM_ATSPI_OBJ_ID(ELM_ATSPI_OBJ_SUB_ID_ATTRIBUTES_GET),\
   EO_TYPECHECK()

/**
 * @def elm_atspi_component_interface_contains
 * @since 1.10
 *
 * @param[in] x
 * @param[in] y
 * @param[in] type
 * @param[out] ret
 */
#define elm_atspi_component_interface_contains(x, y, type, ret)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_CONTAINS),\
   EO_TYPECHECK(int, x),\
   EO_TYPECHECK(int, y),\
   EO_TYPECHECK(AtspiCoordType, type),\
   EO_TYPECHECK(Eina_Bool*, ret)

/**
 * @def elm_atspi_component_interface_accessible_at_point_get
 * @since 1.10
 *
 * @param[in] x
 * @param[in] y
 * @param[in] type
 * @param[out] ret
 */
#define elm_atspi_component_interface_accessible_at_point_get(x, y, type, ret)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_ACCESSIBLE_AT_POINT_GET),\
   EO_TYPECHECK(int, x),\
   EO_TYPECHECK(int, y),\
   EO_TYPECHECK(AtspiCoordType, type),\
   EO_TYPECHECK(Elm_Atspi_Object**, ret)

/**
 * @def elm_atspi_component_interface_extents_get
 * @since 1.10
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 * @param[out] type
 */
#define elm_atspi_component_interface_extents_get(x, y, w, h, type)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_EXTENTS_GET),\
   EO_TYPECHECK(int*, x),\
   EO_TYPECHECK(int*, y),\
   EO_TYPECHECK(int*, w),\
   EO_TYPECHECK(int*, h), \
   EO_TYPECHECK(AtspiCoordType, type)

/**
 * @def elm_atspi_component_interface_extents_set
 * @since 1.10
 *
 * @param[in] x
 * @param[in] y
 * @param[in] w
 * @param[in] h
 * @param[in] type
 * @param[out] ret
 */
#define elm_atspi_component_interface_extents_set(x, y, w, h, type, ret)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_EXTENTS_SET),\
   EO_TYPECHECK(int, x),\
   EO_TYPECHECK(int, y),\
   EO_TYPECHECK(int, w),\
   EO_TYPECHECK(int, h),\
   EO_TYPECHECK(AtspiCoordType, type),\
   EO_TYPECHECK(Eina_Bool*, ret)

/**
 * @def elm_atspi_component_interface_position_get
 * @since 1.10
 *
 * @param[out] x
 * @param[out] y
 * @param[in] type
 */
#define elm_atspi_component_interface_position_get(x, y, type)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_POSITION_GET),\
   EO_TYPECHECK(int*, x),\
   EO_TYPECHECK(int*, y), \
   EO_TYPECHECK(AtspiCoordType, type)

/**
 * @def elm_atspi_component_interface_position_set
 * @since 1.10
 *
 * @param[in] x
 * @param[in] y
 * @param[in] type
 * @param[out] ret
 */
#define elm_atspi_component_interface_position_set(x, y, type, ret)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_POSITION_SET),\
   EO_TYPECHECK(int, x),\
   EO_TYPECHECK(int, y),\
   EO_TYPECHECK(AtspiCoordType, type),\
   EO_TYPECHECK(Eina_Bool*, ret)

/**
 * @def elm_atspi_component_interface_size_get
 * @since 1.10
 *
 * @param[out] x
 * @param[out] y
 */
#define elm_atspi_component_interface_size_get(x, y)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_SIZE_GET),\
   EO_TYPECHECK(int*, x),\
   EO_TYPECHECK(int*, y)

/**
 * @def elm_atspi_component_interface_size_set
 * @since 1.10
 *
 * @param[in] x
 * @param[in] y
 * @param[out] ret
 */
#define elm_atspi_component_interface_size_set(x, y, ret)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_SIZE_SET),\
   EO_TYPECHECK(int, x),\
   EO_TYPECHECK(int, y),\
   EO_TYPECHECK(Eina_Bool*, ret)

/**
 * @def elm_atspi_component_interface_layer_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_component_interface_layer_get(ret)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_LAYER_GET),\
   EO_TYPECHECK(int*, ret)

/**
 * @def elm_atspi_component_interface_z_order_get
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_component_interface_z_order_get(ret)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_Z_ORDER_GET),\
   EO_TYPECHECK(int*, ret)

/**
 * @def elm_atspi_component_interface_focus_grab
 * @since 1.10
 *
 * @param[out] ret
 */
#define elm_atspi_component_interface_focus_grab(ret)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_FOCUS_GRAB),\
   EO_TYPECHECK(Eina_Bool*, ret)

/**
 * @def elm_atspi_component_interface_alpha_get
 * @since 1.10
 *
 * @param[out] ret alpha
 */
#define elm_atspi_component_interface_alpha_get(ret)\
   ELM_ATSPI_COMPONENT_INTERFACE_ID(ELM_ATSPI_COMPONENT_INTERFACE_SUB_ID_ALPHA_GET),\
   EO_TYPECHECK(double*, ret)
