/**
 * Add a new calendar widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return a new calendar widget handle or @c NULL, on errors.
 *
 * This function inserts a new calendar widget on the canvas.
 *
 * @ref calendar_example_01
 *
 * @ingroup Calendar
 */
EAPI Evas_Object         *elm_calendar_add(Evas_Object *parent);

/**
 * Get weekdays names displayed by the calendar.
 *
 * @param obj The calendar object.
 * @return Array of seven strings to be used as weekday names.
 *
 * By default, weekdays abbreviations get from system are displayed:
 * E.g. for an en_US locale: "Sun, Mon, Tue, Wed, Thu, Fri, Sat"
 * The first string is related to Sunday, the second to Monday...
 *
 * @see elm_calendar_weekdays_name_set()
 *
 * @ref calendar_example_05
 *
 * @ingroup Calendar
 */
EAPI const char         **elm_calendar_weekdays_names_get(const Evas_Object *obj);

/**
 * Set the interval on time updates for an user mouse button hold
 * on calendar widgets' month/year selection.
 *
 * @param obj The calendar object
 * @param interval The (first) interval value in seconds
 *
 * This interval value is @b decreased while the user holds the
 * mouse pointer either selecting next or previous month/year.
 *
 * This helps the user to get to a given month distant from the
 * current one easier/faster, as it will start to change quicker and
 * quicker on mouse button holds.
 *
 * The calculation for the next change interval value, starting from
 * the one set with this call, is the previous interval divided by
 * 1.05, so it decreases a little bit.
 *
 * The default starting interval value for automatic changes is
 * @b 0.85 seconds.
 *
 * @see elm_calendar_interval_get()
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_interval_set(Evas_Object *obj, double interval);

/**
 * Get the interval on time updates for an user mouse button hold
 * on calendar widgets' month/year selection.
 *
 * @param obj The calendar object
 * @return The (first) interval value, in seconds, set on it
 *
 * @see elm_calendar_interval_set() for more details
 *
 * @ingroup Calendar
 */
EAPI double               elm_calendar_interval_get(const Evas_Object *obj);

/**
 * Set the minimum and maximum values for the year
 *
 * @param obj The calendar object
 * @param min The minimum year, greater than 1901;
 * @param max The maximum year;
 *
 * Maximum must be greater than minimum, except if you don't want to set
 * maximum year.
 * Default values are 1902 and -1.
 *
 * If the maximum year is a negative value, it will be limited depending
 * on the platform architecture (year 2037 for 32 bits);
 *
 * @see elm_calendar_min_max_year_get()
 *
 * @ref calendar_example_03
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_min_max_year_set(Evas_Object *obj, int min, int max);

/**
 * Get the minimum and maximum values for the year
 *
 * @param obj The calendar object.
 * @param min The minimum year.
 * @param max The maximum year.
 *
 * Default values are 1902 and -1.
 *
 * @see elm_calendar_min_max_year_set() for more details.
 *
 * @ref calendar_example_05
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_min_max_year_get(const Evas_Object *obj, int *min, int *max);

/**
 * Set selected date to be highlighted on calendar.
 *
 * @param obj The calendar object.
 * @param selected_time A @b tm struct to represent the selected date.
 *
 * Set the selected date, changing the displayed month if needed.
 * Selected date changes when the user goes to next/previous month or
 * select a day pressing over it on calendar.
 *
 * @see elm_calendar_selected_time_get()
 *
 * @ref calendar_example_04
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_selected_time_set(Evas_Object *obj, struct tm *selected_time);

/**
 * Get selected date.
 *
 * @param obj The calendar object
 * @param selected_time A @b tm struct to point to selected date
 * @return EINA_FALSE means an error occurred and returned time shouldn't
 * be considered.
 *
 * Get date selected by the user or set by function
 * elm_calendar_selected_time_set().
 * Selected date changes when the user goes to next/previous month or
 * select a day pressing over it on calendar.
 *
 * @see elm_calendar_selected_time_get()
 *
 * @ref calendar_example_05
 *
 * @ingroup Calendar
 */
EAPI Eina_Bool            elm_calendar_selected_time_get(const Evas_Object *obj, struct tm *selected_time);

/**
 * Set a function to format the string that will be used to display
 * month and year;
 *
 * @param obj The calendar object
 * @param format_func Function to set the month-year string given
 * the selected date
 *
 * By default it uses strftime with "%B %Y" format string.
 * It should allocate the memory that will be used by the string,
 * that will be freed by the widget after usage.
 * A pointer to the string and a pointer to the time struct will be provided.
 *
 * Example:
 * @code
 * static char *
 * _format_month_year(struct tm *selected_time)
 * {
 *    char buf[32];
 *    if (!strftime(buf, sizeof(buf), "%B %Y", selected_time)) return NULL;
 *    return strdup(buf);
 * }
 *
 * elm_calendar_format_function_set(calendar, _format_month_year);
 * @endcode
 *
 * @ref calendar_example_02
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_format_function_set(Evas_Object *obj, Elm_Calendar_Format_Cb format_func);

/**
 * Add a new mark to the calendar
 *
 * @param obj The calendar object
 * @param mark_type A string used to define the type of mark. It will be
 * emitted to the theme, that should display a related modification on these
 * days representation.
 * @param mark_time A time struct to represent the date of inclusion of the
 * mark. For marks that repeats it will just be displayed after the inclusion
 * date in the calendar.
 * @param repeat Repeat the event following this periodicity. Can be a unique
 * mark (that don't repeat), daily, weekly, monthly or annually.
 * @return The created mark or @p NULL upon failure.
 *
 * Add a mark that will be drawn in the calendar respecting the insertion
 * time and periodicity. It will emit the type as signal to the widget theme.
 * Default theme supports "holiday" and "checked", but it can be extended.
 *
 * It won't immediately update the calendar, drawing the marks.
 * For this, call elm_calendar_marks_draw(). However, when user selects
 * next or previous month calendar forces marks drawn.
 *
 * Marks created with this method can be deleted with
 * elm_calendar_mark_del().
 *
 * Example
 * @code
 * struct tm selected_time;
 * time_t current_time;
 *
 * current_time = time(NULL) + 5 * (24 * 60 * 60);
 * localtime_r(&current_time, &selected_time);
 * elm_calendar_mark_add(cal, "holiday", selected_time,
 *     ELM_CALENDAR_ANNUALLY);
 *
 * current_time = time(NULL) + 1 * (24 * 60 * 60);
 * localtime_r(&current_time, &selected_time);
 * elm_calendar_mark_add(cal, "checked", selected_time, ELM_CALENDAR_UNIQUE);
 *
 * elm_calendar_marks_draw(cal);
 * @endcode
 *
 * @see elm_calendar_marks_draw()
 * @see elm_calendar_mark_del()
 *
 * @ref calendar_example_06
 *
 * @ingroup Calendar
 */
EAPI Elm_Calendar_Mark   *elm_calendar_mark_add(Evas_Object *obj, const char *mark_type, struct tm *mark_time, Elm_Calendar_Mark_Repeat_Type repeat);

/**
 * Remove all calendar's marks
 *
 * @param obj The calendar object.
 *
 * @see elm_calendar_mark_add()
 * @see elm_calendar_mark_del()
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_marks_clear(Evas_Object *obj);

/**
 * Get a list of all the calendar marks.
 *
 * @param obj The calendar object.
 * @return An @c Eina_List of calendar marks objects, or @c NULL on failure.
 *
 * @see elm_calendar_mark_add()
 * @see elm_calendar_mark_del()
 * @see elm_calendar_marks_clear()
 *
 * @ingroup Calendar
 */
EAPI const Eina_List     *elm_calendar_marks_get(const Evas_Object *obj);

/**
 * Draw calendar marks.
 *
 * @param obj The calendar object.
 *
 * Should be used after adding, removing or clearing marks.
 * It will go through the entire marks list updating the calendar.
 * If lots of marks will be added, add all the marks and then call
 * this function.
 *
 * When the month is changed, i.e. user selects next or previous month,
 * marks will be drawn.
 *
 * @see elm_calendar_mark_add()
 * @see elm_calendar_mark_del()
 * @see elm_calendar_marks_clear()
 *
 * @ref calendar_example_06
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_marks_draw(Evas_Object *obj);

/**
 * Set the first day of week to use on calendar widgets'.
 *
 * @param obj The calendar object
 * @param day An int which correspond to the first day of the week (Sunday = 0, Monday = 1,
 * ..., Saturday = 6)
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_first_day_of_week_set(Evas_Object *obj, Elm_Calendar_Weekday day);

/**
 * Get the first day of week, who are used on calendar widgets'.
 *
 * @param obj The calendar object
 * @return An int which correspond to the first day of the week (Sunday = 0, Monday = 1,
 * ..., Saturday = 6)
 *
 * @see elm_calendar_first_day_of_week_set() for more details
 *
 * @ingroup Calendar
 */
EAPI Elm_Calendar_Weekday elm_calendar_first_day_of_week_get(const Evas_Object *obj);

/**
 * Set select day mode to use.
 *
 * @param obj The calendar object.
 * @param mode The select mode to use.
 *
 * Set the day selection mode used.
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_select_mode_set(Evas_Object *obj, Elm_Calendar_Select_Mode mode);

/**
 * Get the select day mode used.
 *
 * @param obj The calendar object.
 *
 * @return the selected mode
 *
 * Get the day selection mode used.
 *
 * @see elm_calendar_select_mode_set() for more details
 *
 * @ingroup Calendar
 */
EAPI Elm_Calendar_Select_Mode elm_calendar_select_mode_get(const Evas_Object *obj);

/**
 * Define which fields of a @b tm struct will be taken into account, when
 * elm_calendar_selected_time_set() is invoked.
 *
 * @param obj The calendar object
 * @param selectable A bitmask of Elm_Calendar_Selectable
 *
 * By Default the bitmask is set to use all fields of a @b tm struct (year,
 * month and day of the month).
 *
 * @ingroup Calendar
 * @see elm_calendar_selected_time_set
 * @since 1.8
 */
EAPI void                 elm_calendar_selectable_set(Evas_Object *obj, Elm_Calendar_Selectable selectable);


/**
 * Get how elm_calendar_selected_time_set manage a date
 *
 * @param obj The calendar object
 * @return The flag used to manage a date with a elm_calendar_selected_time_set
 *
 * @ingroup Calendar
 * @see elm_calendar_selectable_set
 * @see elm_calendar_selected_time_set
 * @since 1.8
 */
EAPI Elm_Calendar_Selectable elm_calendar_selectable_get(const Evas_Object *obj);

/**
 * Get the current time displayed in the widget
 *
 * @param obj The calendar object
 * @param selected_time A @b tm struct to point to displayed date
 * @return EINA_FALSE means an error occurred. If it's an error the returned
 * time is zero filled.
 *
 * @ingroup Calendar
 * @since 1.8
 */
EAPI Eina_Bool                    elm_calendar_displayed_time_get(const Evas_Object *obj, struct tm *displayed_time);
