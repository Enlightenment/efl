/**
 * @defgroup Calendar Calendar
 * @ingroup Elementary
 *
 * @image html calendar_inheritance_tree.png
 * @image latex calendar_inheritance_tree.eps
 *
 * This is a calendar widget. It helps applications to flexibly
 * display a calender with day of the week, date, year and
 * month. Applications are able to set specific dates to be reported
 * back, when selected, in the smart callbacks of the calendar
 * widget. The API of this widget lets the applications perform other
 * functions, like:
 *
 * - placing marks on specific dates
 * - setting the bounds for the calendar (minimum and maximum years)
 * - setting the day names of the week (e.g. "Thu" or "Thursday")
 * - setting the year and month format.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for calendar objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "changed" - emitted when the date in the calendar is changed.
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 *
 * Here is some sample code using it:
 * @li @ref calendar_example_01
 * @li @ref calendar_example_02
 * @li @ref calendar_example_03
 * @li @ref calendar_example_04
 * @li @ref calendar_example_05
 * @li @ref calendar_example_06
 */

/**
 * @addtogroup Calendar
 * @{
 */

typedef enum
{
   ELM_CALENDAR_UNIQUE, /**< Default value. Marks will be displayed only on event day. */
   ELM_CALENDAR_DAILY, /**< Marks will be displayed every day after event day (inclusive). */
   ELM_CALENDAR_WEEKLY, /**< Marks will be displayed every week after event day (inclusive) - i.e. each seven days. */
   ELM_CALENDAR_MONTHLY, /**< Marks will be displayed every month day that coincides to event day. E.g.: if an event is set to 30th Jan, no marks will be displayed on Feb, but will be displayed on 30th Mar*/
   ELM_CALENDAR_ANNUALLY, /**< Marks will be displayed every year that coincides to event day (and month). E.g. an event added to 30th Jan 2012 will be repeated on 30th Jan 2013. */
   ELM_CALENDAR_LAST_DAY_OF_MONTH /**< Marks will be displayed every last day of month after event day (inclusive).  @since 1.7 */
} _Elm_Calendar_Mark_Repeat_Type;

/**
 * @enum _Elm_Calendar_Mark_Repeat_Type
 * @typedef Elm_Calendar_Mark_Repeat_Type
 *
 * Event periodicity, used to define if a mark should be repeated
 * @b beyond event's day. It's set when a mark is added.
 *
 * So, for a mark added to 13th May with periodicity set to WEEKLY,
 * there will be marks every week after this date. Marks will be displayed
 * at 13th, 20th, 27th, 3rd June ...
 *
 * Values don't work as bitmask, only one can be chosen.
 *
 * @see elm_calendar_mark_add()
 *
 * @ingroup Calendar
 */
typedef _Elm_Calendar_Mark_Repeat_Type Elm_Calendar_Mark_Repeat_Type;

typedef enum
{
   ELM_DAY_SUNDAY,
   ELM_DAY_MONDAY,
   ELM_DAY_TUESDAY,
   ELM_DAY_WEDNESDAY,
   ELM_DAY_THURSDAY,
   ELM_DAY_FRIDAY,
   ELM_DAY_SATURDAY,
   ELM_DAY_LAST
} _Elm_Calendar_Weekday;

/**
 * @enum _Elm_Calendar_Weekday
 * @typedef Elm_Calendar_Weekday
 *
 * a weekday
 *
 * @see elm_calendar_first_day_of_week_set()
 *
 * @ingroup Calendar
 */
typedef _Elm_Calendar_Weekday Elm_Calendar_Weekday;


typedef enum
{
   ELM_CALENDAR_SELECT_MODE_DEFAULT = 0, /**< Default value. a day is always selected. */
   ELM_CALENDAR_SELECT_MODE_ALWAYS, /**< a day is always selected. */
   ELM_CALENDAR_SELECT_MODE_NONE, /**< None of the days can be selected. */
   ELM_CALENDAR_SELECT_MODE_ONDEMAND /**< User may have selected a day or not. */
} _Elm_Calendar_Select_Mode;

/**
 * @enum _Elm_Calendar_Select_Mode
 * @typedef Elm_Calendar_Select_Mode
 *
 * the mode, who determine how user could select a day
 *
 * @see elm_calendar_select_mode_set()
 *
 * @ingroup Calendar
 */
typedef _Elm_Calendar_Select_Mode Elm_Calendar_Select_Mode;

typedef struct _Elm_Calendar_Mark Elm_Calendar_Mark;    /**< Item handle for a calendar mark. Created with elm_calendar_mark_add() and deleted with elm_calendar_mark_del(). */

/**
 * @typedef Elm_Calendar_Format_Cb
 *
 * This callback type is used to format the string that will be used
 * to display month and year.
 *
 * @param stime Struct representing time.
 * @return String representing time that will be set to calendar's text.
 *
 * @see elm_calendar_format_function_set()
 *
 * @ingroup Calendar
 */
typedef char * (*Elm_Calendar_Format_Cb)(struct tm *stime);

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
 * Set weekdays names to be displayed by the calendar.
 *
 * @param obj The calendar object.
 * @param weekdays Array of seven strings to be used as weekday names.
 * @warning It must have 7 elements, or it will access invalid memory.
 * @warning The strings must be NULL terminated ('@\0').
 *
 * By default, weekdays abbreviations get from system are displayed:
 * E.g. for an en_US locale: "Sun, Mon, Tue, Wed, Thu, Fri, Sat"
 *
 * The first string should be related to Sunday, the second to Monday...
 *
 * The usage should be like this:
 * @code
 *   const char *weekdays[] =
 *   {
 *      "Sunday", "Monday", "Tuesday", "Wednesday",
 *      "Thursday", "Friday", "Saturday"
 *   };
 *   elm_calendar_weekdays_names_set(calendar, weekdays);
 * @endcode
 *
 * @see elm_calendar_weekdays_name_get()
 *
 * @ref calendar_example_02
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_weekdays_names_set(Evas_Object *obj, const char *weekdays[]);

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
 * @see elm_calendar_min_max_year_get() for more details.
 *
 * @ref calendar_example_05
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_min_max_year_get(const Evas_Object *obj, int *min, int *max);

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
 * current_time = time(NULL) + 5 * 84600;
 * localtime_r(&current_time, &selected_time);
 * elm_calendar_mark_add(cal, "holiday", selected_time,
 *     ELM_CALENDAR_ANNUALLY);
 *
 * current_time = time(NULL) + 1 * 84600;
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
 * Delete mark from the calendar.
 *
 * @param mark The mark to be deleted.
 *
 * If deleting all calendar marks is required, elm_calendar_marks_clear()
 * should be used instead of getting marks list and deleting each one.
 *
 * @see elm_calendar_mark_add()
 *
 * @ref calendar_example_06
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_mark_del(Elm_Calendar_Mark *mark);

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
 * Set the interval on time updates for an user mouse button hold
 * on calendar widgets' month selection.
 *
 * @param obj The calendar object
 * @param interval The (first) interval value in seconds
 *
 * This interval value is @b decreased while the user holds the
 * mouse pointer either selecting next or previous month.
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
 * on calendar widgets' month selection.
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
 * Set the first day of week to use on calendar widgets'.
 *
 * @param obj The calendar object
 * @param day An int which correspond to the first day of the week (Sunday = 0, monday = 1,
 * ..., saturday = 6)
 *
 * @ingroup Calendar
 */
EAPI void                 elm_calendar_first_day_of_week_set(Evas_Object *obj, Elm_Calendar_Weekday day);

/**
 * Get the first day of week, who are used on calendar widgets'.
 *
 * @param obj The calendar object
 * @return An int which correspond to the first day of the week (Sunday = 0, monday = 1,
 * ..., saturday = 6)
 *
 * @see elm_calendar_first_day_of_week_set() for more details
 *
 * @ingroup Calendar
 */
EAPI Elm_Calendar_Weekday elm_calendar_first_day_of_week_get(const Evas_Object *obj);

/**
 * @}
 */
