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
