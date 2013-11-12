#define ELM_OBJ_DAYSELECTOR_CLASS elm_obj_dayselector_class_get()

const Eo_Class *elm_obj_dayselector_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_DAYSELECTOR_BASE_ID;

enum
{
   ELM_OBJ_DAYSELECTOR_SUB_ID_DAY_SELECTED_SET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_DAY_SELECTED_GET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_WEEK_START_SET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_WEEK_START_GET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_START_SET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_START_GET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_LENGTH_SET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_LENGTH_GET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKDAYS_NAMES_SET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKDAYS_NAMES_GET,
   ELM_OBJ_DAYSELECTOR_SUB_ID_LAST
};

#define ELM_OBJ_DAYSELECTOR_ID(sub_id) (ELM_OBJ_DAYSELECTOR_BASE_ID + sub_id)


/**
 * @def elm_obj_dayselector_day_selected_set
 * @since 1.8
 *
 * Set the state of given Dayselector_Day.
 *
 * @param[in] day
 * @param[in] selected
 *
 * @see elm_dayselector_day_selected_set
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_day_selected_set(day, selected) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_DAY_SELECTED_SET), EO_TYPECHECK(Elm_Dayselector_Day, day), EO_TYPECHECK(Eina_Bool, selected)

/**
 * @def elm_obj_dayselector_day_selected_get
 * @since 1.8
 *
 * Get the state of given Dayselector_Day.
 *
 * @param[in] day
 * @param[out] ret
 *
 * @see elm_dayselector_day_selected_get
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_day_selected_get(day, ret) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_DAY_SELECTED_GET), EO_TYPECHECK(Elm_Dayselector_Day, day), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_dayselector_week_start_set
 * @since 1.8
 *
 * Set the starting day of Dayselector.
 *
 * @param[in] day
 *
 * @see elm_dayselector_week_start_set
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_week_start_set(day) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEK_START_SET), EO_TYPECHECK(Elm_Dayselector_Day, day)

/**
 * @def elm_obj_dayselector_week_start_get
 * @since 1.8
 *
 * Get the starting day of Dayselector.
 *
 * @param[out] ret
 *
 * @see elm_dayselector_week_start_get
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_week_start_get(ret) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEK_START_GET), EO_TYPECHECK(Elm_Dayselector_Day *, ret)

/**
 * @def elm_obj_dayselector_weekend_start_set
 * @since 1.8
 *
 * Set the weekend starting day of Dayselector.
 *
 * @param[in] day
 *
 * @see elm_dayselector_weekend_start_set
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_weekend_start_set(day) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_START_SET), EO_TYPECHECK(Elm_Dayselector_Day, day)

/**
 * @def elm_obj_dayselector_weekend_start_get
 * @since 1.8
 *
 * Get the weekend starting day of Dayselector.
 *
 * @param[out] ret
 *
 * @see elm_dayselector_weekend_start_get
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_weekend_start_get(ret) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_START_GET), EO_TYPECHECK(Elm_Dayselector_Day *, ret)

/**
 * @def elm_obj_dayselector_weekend_length_set
 * @since 1.8
 *
 * Set the weekend length of Dayselector.
 *
 * @param[in] length
 *
 * @see elm_dayselector_weekend_length_set
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_weekend_length_set(length) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_LENGTH_SET), EO_TYPECHECK(unsigned int, length)

/**
 * @def elm_obj_dayselector_weekend_length_get
 * @since 1.8
 *
 * Get the weekend length of Dayselector.
 *
 * @param[out] ret
 *
 * @see elm_dayselector_weekend_length_get
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_weekend_length_get(ret) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_LENGTH_GET), EO_TYPECHECK(unsigned int *, ret)

/**
 * @def elm_obj_dayselector_weekdays_names_set
 * @since 1.8
 *
 * Set the weekdays's names
 *
 * @param[in] weekdays
 *
 * @see elm_dayselector_weekdays_names_set
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_weekdays_names_set(weekdays) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKDAYS_NAMES_SET), EO_TYPECHECK(const char **, weekdays)

/**
 * @def elm_obj_dayselector_weekdays_names_get
 * @since 1.8
 *
 * Get the weekdays' names
 *
 * @param[out] ret
 *
 * @see elm_dayselector_weekdays_names_get
 *
 * @ingroup Dayselector
 */
#define elm_obj_dayselector_weekdays_names_get(ret) ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKDAYS_NAMES_GET), EO_TYPECHECK(Eina_List **, ret)
