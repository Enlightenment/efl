/**
 * @defgroup Dayselector Dayselector
 * @ingroup Elementary
 *
 * @image html dayselector_inheritance_tree.png
 * @image latex dayselector_inheritance_tree.eps
 *
 * @image html img/widget/dayselector/preview-00.png
 * @image latex img/widget/dayselector/preview-00.eps
 *
 * @brief <b>Dayselector widget:</b>
 *
 *"elm_dayselector" is a day selection widget. It displays all seven days of
 * the week and allows the user to select multiple days.
 *
 * The selection can be toggle by just clicking on the day.
 *
 * Dayselector also provides the functionality to check whether a day is
 * selected or not.
 *
 * First day of the week is taken from config settings by default. It can be
 * altered by using the API elm_dayselector_week_start_set() API.
 *
 * APIs are provided for setting the duration of weekend
 * elm_dayselector_weekend_start_set() and elm_dayselector_weekend_length_set()
 * does this job.
 *
 * Two styles of weekdays and weekends are supported in Dayselector.
 * Application can emit signals on individual check objects for setting the
 * weekday, weekend styles.
 *
 * Once the weekend start day or weekend length changes, all the weekday &
 * weekend styles will be reset to default style. It's the application's
 * responsibility to set the styles again by sending corresponding signals.
 *
 * Supported elm_object_item common APIs.
 *
 * @li @ref elm_object_part_text_set,
 * @li @ref elm_object_part_text_get,
 * @li @ref elm_object_part_content_set,
 * @li @ref elm_object_part_content_get,
 * @li @ref elm_object_part_content_unset
 *
 * @li "day0" indicates Sunday, "day1" indicates Monday etc. continues and so,
 * "day6" indicates the Saturday part name.
 *
 * Application can change individual day display string by using the API
 * elm_object_part_text_set().
 *
 * elm_object_part_content_set() API sets the individual day object only if
 * the passed one is a Check widget.
 *
 * Check object representing a day can be set/get by the application by using
 * the elm_object_part_content_set/get APIs thus providing a way to handle
 * the different check styles for individual days.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for dayselector objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "dayselector,changed" - when the user changes the state of a day.
 * @li @c "language,changed" - the program's language changed
 *
 * Available styles for dayselector are:
 * @li default
 *
 * This example shows the usage of the widget.
 * @li @ref dayselector_example
 *
 */

/**
 * @addtogroup Dayselector
 * @{
 */

/**
 * Identifies the day of the week.
 * API can call the selection/unselection of day with this as a parameter.
 *
 * @see elm_dayselector_day_selected_set()
 * @see elm_dayselector_day_selected_get()
 */
typedef enum
{
   ELM_DAYSELECTOR_SUN = 0,/**< indicates Sunday */
   ELM_DAYSELECTOR_MON,    /**< indicates Monday */
   ELM_DAYSELECTOR_TUE,    /**< indicates Tuesday */
   ELM_DAYSELECTOR_WED,    /**< indicates Wednesday */
   ELM_DAYSELECTOR_THU,    /**< indicates Thursday */
   ELM_DAYSELECTOR_FRI,    /**< indicates Friday */
   ELM_DAYSELECTOR_SAT,    /**< indicates Saturday */
   ELM_DAYSELECTOR_MAX     /**< Sentinel value, @b don't use */
} Elm_Dayselector_Day;

/**
 * Add the dayselector.
 *
 * @param parent Parent object
 * @return New dayselector object or @c NULL, if it cannot be created
 *
 * @ingroup Dayselector
 */
EAPI Evas_Object *elm_dayselector_add(Evas_Object *parent);

/**
 * Set the state of given Dayselector_Day.
 *
 * @param obj Dayselector object
 * @param day Dayselector_Day that the user want to set state.
 * @param selected state of the day. @c EINA_TRUE is selected.
 *
 * @see Elm_Dayselector_Day
 * @see elm_dayselector_day_selected_get()
 *
 * @ingroup Dayselector
 */
EAPI void   elm_dayselector_day_selected_set(Evas_Object *obj, Elm_Dayselector_Day day, Eina_Bool selected);

/**
 * Get the state of given Dayselector_Day.
 *
 * @param obj Dayselector object
 * @param day Dayselector_Day that the user want to know state.
 * @return @c EINA_TRUE, if the Day is selected
 *
 * @see Elm_Dayselector_Day
 * @see elm_dayselector_day_selected_set()
 *
 * @ingroup Dayselector
 */
EAPI Eina_Bool   elm_dayselector_day_selected_get(const Evas_Object *obj, Elm_Dayselector_Day day);

/**
 * Set the starting day of Dayselector.
 *
 * @param obj Dayselector object
 * @param day Dayselector_Day the first day that the user wants to display.
 *
 * @see Elm_Dayselector_Day
 * @see elm_dayselector_week_start_get()
 *
 * @ingroup Dayselector
 */
EAPI void   elm_dayselector_week_start_set(Evas_Object *obj, Elm_Dayselector_Day day);

/**
 * Get the starting day of Dayselector.
 *
 * @param obj Dayselector object
 * @return Day from where Dayselector displays all the weekdays in order.
 *
 * @see Elm_Dayselector_Day
 * @see elm_dayselector_week_start_set()
 *
 * @ingroup Dayselector
 */
EAPI Elm_Dayselector_Day   elm_dayselector_week_start_get(const Evas_Object *obj);

/**
 * Set the weekend starting day of Dayselector.
 *
 * @param obj Dayselector object
 * @param day Dayselector_Day the first day from where weekend starts.
 *
 * @see Elm_Dayselector_Day
 * @see elm_dayselector_weekend_start_get()
 *
 * @ingroup Dayselector
 */
EAPI void   elm_dayselector_weekend_start_set(Evas_Object *obj, Elm_Dayselector_Day day);

/**
 * Get the weekend starting day of Dayselector.
 *
 * @param obj Dayselector object
 * @return Elm_Dayselector_Day Day from where weekend starts.
 *
 * @see Elm_Dayselector_Day
 * @see elm_dayselector_weekend_start_set()
 *
 * @ingroup Dayselector
 */
EAPI Elm_Dayselector_Day   elm_dayselector_weekend_start_get(const Evas_Object *obj);

/**
 * Set the weekend length of Dayselector.
 *
 * @param obj Dayselector object
 * @param length Weekend length, number of days as an integer.
 *
 * @see elm_dayselector_weekend_length_get()
 *
 * @ingroup Dayselector
 */
EAPI void   elm_dayselector_weekend_length_set(Evas_Object *obj, unsigned int length);

/**
 * Get the weekend length of Dayselector.
 *
 * @param obj Dayselector object
 * @return Number of days marked as a weekend.
 *
 * @see Elm_Dayselector_Day
 * @see elm_dayselector_weekend_length_set()
 *
 * @ingroup Dayselector
 */
EAPI unsigned int   elm_dayselector_weekend_length_get(const Evas_Object *obj);

/**
 * @}
 */
