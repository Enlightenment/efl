#define ELM_OBJ_DATETIME_CLASS elm_obj_datetime_class_get()

const Eo_Class *elm_obj_datetime_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_DATETIME_BASE_ID;

enum
{
   ELM_OBJ_DATETIME_SUB_ID_FORMAT_GET,
   ELM_OBJ_DATETIME_SUB_ID_FORMAT_SET,
   ELM_OBJ_DATETIME_SUB_ID_FIELD_VISIBLE_GET,
   ELM_OBJ_DATETIME_SUB_ID_FIELD_VISIBLE_SET,
   ELM_OBJ_DATETIME_SUB_ID_FIELD_LIMIT_GET,
   ELM_OBJ_DATETIME_SUB_ID_FIELD_LIMIT_SET,
   ELM_OBJ_DATETIME_SUB_ID_VALUE_GET,
   ELM_OBJ_DATETIME_SUB_ID_VALUE_SET,
   ELM_OBJ_DATETIME_SUB_ID_VALUE_MIN_GET,
   ELM_OBJ_DATETIME_SUB_ID_VALUE_MIN_SET,
   ELM_OBJ_DATETIME_SUB_ID_VALUE_MAX_GET,
   ELM_OBJ_DATETIME_SUB_ID_VALUE_MAX_SET,
   ELM_OBJ_DATETIME_SUB_ID_LAST
};

#define ELM_OBJ_DATETIME_ID(sub_id) (ELM_OBJ_DATETIME_BASE_ID + sub_id)


/**
 * @def elm_obj_datetime_format_get
 * @since 1.8
 *
 * Get the datetime format.
 *
 * @param[out] ret
 *
 * @see elm_datetime_format_get
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_format_get(ret) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FORMAT_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_datetime_format_set
 * @since 1.8
 *
 * Set the datetime format.
 *
 * @param[in] fmt
 *
 * @see elm_datetime_format_set
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_format_set(fmt) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FORMAT_SET), EO_TYPECHECK(const char *, fmt)

/**
 * @def elm_obj_datetime_field_visible_get
 * @since 1.8
 *
 * @brief Get whether a field can be visible/not
 *
 * @param[in] fieldtype
 * @param[out] ret
 *
 * @see elm_datetime_field_visible_get
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_field_visible_get(fieldtype, ret) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FIELD_VISIBLE_GET), EO_TYPECHECK(Elm_Datetime_Field_Type, fieldtype), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_datetime_field_visible_set
 * @since 1.8
 *
 * @brief Set a field to be visible or not.
 *
 * @param[in] fieldtype
 * @param[in] visible
 *
 * @see elm_datetime_field_visible_set
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_field_visible_set(fieldtype, visible) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FIELD_VISIBLE_SET), EO_TYPECHECK(Elm_Datetime_Field_Type, fieldtype), EO_TYPECHECK(Eina_Bool, visible)

/**
 * @def elm_obj_datetime_field_limit_get
 * @since 1.8
 *
 * @brief Get the field limits of a field.
 *
 * @param[in] fieldtype
 * @param[out] min
 * @param[out] max
 *
 * @see elm_datetime_field_limit_get
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_field_limit_get(fieldtype, min, max) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FIELD_LIMIT_GET), EO_TYPECHECK(Elm_Datetime_Field_Type, fieldtype), EO_TYPECHECK(int *, min), EO_TYPECHECK(int *, max)

/**
 * @def elm_obj_datetime_field_limit_set
 * @since 1.8
 *
 * @brief Set the field limits of a field.
 *
 * @param[in] fieldtype
 * @param[in] min
 * @param[in] max
 *
 * @see elm_datetime_field_limit_set
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_field_limit_set(fieldtype, min, max) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_FIELD_LIMIT_SET), EO_TYPECHECK(Elm_Datetime_Field_Type, fieldtype), EO_TYPECHECK(int, min), EO_TYPECHECK(int, max)

/**
 * @def elm_obj_datetime_value_get
 * @since 1.8
 *
 * @brief Get the current value of a Datetime object.
 *
 * @param[in,out] currtime
 * @param[out] ret
 *
 * @see elm_datetime_value_get
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_value_get(currtime, ret) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_GET), EO_TYPECHECK(struct tm *, currtime), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_datetime_value_set
 * @since 1.8
 *
 * @brief Set the current value of a Datetime object.
 *
 * @param[in] newtime
 * @param[out] ret
 *
 * @see elm_datetime_value_set
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_value_set(newtime, ret) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_SET), EO_TYPECHECK(const struct tm *, newtime), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_datetime_value_min_get
 * @since 1.8
 *
 * @brief Get the lower boundary of a field.
 *
 * @param[in,out] mintime
 * @param[out] ret
 *
 * @see elm_datetime_value_min_get
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_value_min_get(mintime, ret) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_MIN_GET), EO_TYPECHECK(struct tm *, mintime), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_datetime_value_min_set
 * @since 1.8
 *
 * @brief Set the lower boundary of a field.
 *
 * @param[in] mintime
 * @param[out] ret
 *
 * @see elm_datetime_value_min_set
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_value_min_set(mintime, ret) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_MIN_SET), EO_TYPECHECK(const struct tm *, mintime), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_datetime_value_max_get
 * @since 1.8
 *
 * @brief Get the upper boundary of a field.
 *
 * @param[in,out] maxtime
 * @param[out] ret
 *
 * @see elm_datetime_value_max_get
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_value_max_get(maxtime, ret) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_MAX_GET), EO_TYPECHECK(struct tm *, maxtime), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_datetime_value_max_set
 * @since 1.8
 *
 * @brief Set the upper boundary of a field.
 *
 * @param[in] maxtime
 * @param[out] ret
 *
 * @see elm_datetime_value_max_set
 *
 * @ingroup Datetime
 */
#define elm_obj_datetime_value_max_set(maxtime, ret) ELM_OBJ_DATETIME_ID(ELM_OBJ_DATETIME_SUB_ID_VALUE_MAX_SET), EO_TYPECHECK(const struct tm *, maxtime), EO_TYPECHECK(Eina_Bool *, ret)
