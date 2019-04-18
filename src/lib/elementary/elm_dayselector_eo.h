#ifndef _ELM_DAYSELECTOR_EO_H_
#define _ELM_DAYSELECTOR_EO_H_

#ifndef _ELM_DAYSELECTOR_EO_CLASS_TYPE
#define _ELM_DAYSELECTOR_EO_CLASS_TYPE

typedef Eo Elm_Dayselector;

#endif

#ifndef _ELM_DAYSELECTOR_EO_TYPES
#define _ELM_DAYSELECTOR_EO_TYPES

/**
 * @brief Identifies the day of the week. API can call the
 * selection/unselection of day with this as a parameter.
 *
 * See also @ref elm_obj_dayselector_day_selected_set,
 * @ref elm_obj_dayselector_day_selected_get.
 *
 * @ingroup Elm_Dayselector
 */
typedef enum
{
  ELM_DAYSELECTOR_SUN = 0, /**< Indicates Sunday. */
  ELM_DAYSELECTOR_MON, /**< Indicates Monday. */
  ELM_DAYSELECTOR_TUE, /**< Indicates Tuesday. */
  ELM_DAYSELECTOR_WED, /**< Indicates Wednesday. */
  ELM_DAYSELECTOR_THU, /**< Indicates Thursday. */
  ELM_DAYSELECTOR_FRI, /**< Indicates Friday. */
  ELM_DAYSELECTOR_SAT, /**< Indicates Saturday. */
  ELM_DAYSELECTOR_MAX /**< Sentinel value to indicate last enum field during
                       * iteration */
} Elm_Dayselector_Day;


#endif
/** Elementary dayselector class
 *
 * @ingroup Elm_Dayselector
 */
#define ELM_DAYSELECTOR_CLASS elm_dayselector_class_get()

EWAPI const Efl_Class *elm_dayselector_class_get(void);

/**
 * @brief Set the starting day of Dayselector.
 *
 * See also @ref Elm_Dayselector_Day, @ref elm_obj_dayselector_week_start_get.
 *
 * @param[in] obj The object.
 * @param[in] day Dayselector_Day the first day that the user wants to display.
 *
 * @ingroup Elm_Dayselector
 */
EOAPI void elm_obj_dayselector_week_start_set(Eo *obj, Elm_Dayselector_Day day);

/**
 * @brief Get the starting day of Dayselector.
 *
 * See also @ref Elm_Dayselector_Day, @ref elm_obj_dayselector_week_start_set.
 *
 * @param[in] obj The object.
 *
 * @return Dayselector_Day the first day that the user wants to display.
 *
 * @ingroup Elm_Dayselector
 */
EOAPI Elm_Dayselector_Day elm_obj_dayselector_week_start_get(const Eo *obj);

/**
 * @brief Set the weekend length of Dayselector.
 *
 * See also @ref elm_obj_dayselector_weekend_length_get.
 *
 * @param[in] obj The object.
 * @param[in] length Weekend length, number of days as an integer.
 *
 * @ingroup Elm_Dayselector
 */
EOAPI void elm_obj_dayselector_weekend_length_set(Eo *obj, unsigned int length);

/**
 * @brief Get the weekend length of Dayselector.
 *
 * See also @ref Elm_Dayselector_Day,
 * @ref elm_obj_dayselector_weekend_length_set.
 *
 * @param[in] obj The object.
 *
 * @return Weekend length, number of days as an integer.
 *
 * @ingroup Elm_Dayselector
 */
EOAPI unsigned int elm_obj_dayselector_weekend_length_get(const Eo *obj);

/**
 * @brief Set the weekend starting day of Dayselector.
 *
 * See also @ref Elm_Dayselector_Day,
 * @ref elm_obj_dayselector_weekend_start_get.
 *
 * @param[in] obj The object.
 * @param[in] day Dayselector_Day the first day from where weekend starts.
 *
 * @ingroup Elm_Dayselector
 */
EOAPI void elm_obj_dayselector_weekend_start_set(Eo *obj, Elm_Dayselector_Day day);

/**
 * @brief Get the weekend starting day of Dayselector.
 *
 * See also @ref Elm_Dayselector_Day,
 * @ref elm_obj_dayselector_weekend_start_set.
 *
 * @param[in] obj The object.
 *
 * @return Dayselector_Day the first day from where weekend starts.
 *
 * @ingroup Elm_Dayselector
 */
EOAPI Elm_Dayselector_Day elm_obj_dayselector_weekend_start_get(const Eo *obj);

/**
 * @brief Set weekdays names to be displayed by the Dayselector.
 *
 * By default or if weekdays is @c null, weekdays abbreviations get from system
 * are displayed: E.g. for an en_US locale: "Sun, Mon, Tue, Wed, Thu, Fri, Sat"
 *
 * The first string should be related to Sunday, the second to Monday...
 *
 * See also @ref elm_obj_dayselector_weekdays_names_get,
 * @ref elm_obj_dayselector_weekend_start_set.
 *
 * @param[in] obj The object.
 * @param[in] weekdays Array of seven strings to be used as weekday names.
 * Warning: It must have 7 elements, or it will access invalid memory. Warning:
 * The strings must be NULL terminated ('@\0').
 *
 * @since 1.8
 *
 * @ingroup Elm_Dayselector
 */
EOAPI void elm_obj_dayselector_weekdays_names_set(Eo *obj, const char **weekdays);

/**
 * @brief Get weekdays names displayed by the calendar.
 *
 * By default, weekdays abbreviations get from system are displayed: E.g. for
 * an en_US locale: "Sun, Mon, Tue, Wed, Thu, Fri, Sat" The first string is
 * related to Sunday, the second to Monday...
 *
 * See also @ref elm_obj_dayselector_weekdays_names_set.
 *
 * @param[in] obj The object.
 *
 * @return A list of seven strings to be used as weekday names.
 *
 * @since 1.8
 *
 * @ingroup Elm_Dayselector
 */
EOAPI Eina_List *elm_obj_dayselector_weekdays_names_get(const Eo *obj) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set the state of given Dayselector_Day.
 *
 * See also @ref Elm_Dayselector_Day,
 * @ref elm_obj_dayselector_day_selected_get.
 *
 * @param[in] obj The object.
 * @param[in] day Dayselector_Day that the user want to set state.
 * @param[in] selected State of the day. @c true is selected.
 *
 * @ingroup Elm_Dayselector
 */
EOAPI void elm_obj_dayselector_day_selected_set(Eo *obj, Elm_Dayselector_Day day, Eina_Bool selected);

/**
 * @brief Get the state of given Dayselector_Day.
 *
 * See also @ref Elm_Dayselector_Day,
 * @ref elm_obj_dayselector_day_selected_set.
 *
 * @param[in] obj The object.
 * @param[in] day Dayselector_Day that the user want to know state.
 *
 * @return @c true on success, @c false on failure
 *
 * @ingroup Elm_Dayselector
 */
EOAPI Eina_Bool elm_obj_dayselector_day_selected_get(const Eo *obj, Elm_Dayselector_Day day);

#endif
