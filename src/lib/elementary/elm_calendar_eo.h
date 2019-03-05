#ifndef _ELM_CALENDAR_EO_H_
#define _ELM_CALENDAR_EO_H_

#ifndef _ELM_CALENDAR_EO_CLASS_TYPE
#define _ELM_CALENDAR_EO_CLASS_TYPE

typedef Eo Elm_Calendar;

#endif

#ifndef _ELM_CALENDAR_EO_TYPES
#define _ELM_CALENDAR_EO_TYPES

/**
 * @brief Event periodicity, used to define if a mark should be repeated beyond
 * event's day.
 *
 * It's set when a mark is added. So, for a mark added to 13th May with
 * periodicity set to WEEKLY, there will be marks every week after this date.
 * Marks will be displayed at 13th, 20th, 27th, 3rd June ...
 *
 * Values don't work as bitmask, only one can be chosen. See also
 * @ref elm_obj_calendar_mark_add.
 *
 * @ingroup Elm_Calendar_Mark_Repeat
 */
typedef enum
{
  ELM_CALENDAR_UNIQUE = 0, /**< Default value. Marks will be displayed only on
                            * event day. */
  ELM_CALENDAR_DAILY, /**< Marks will be displayed every day after event day
                       * (inclusive). */
  ELM_CALENDAR_WEEKLY, /**< Marks will be displayed every week after event day
                        * (inclusive) - i.e. each seven days. */
  ELM_CALENDAR_MONTHLY, /**< Marks will be displayed every month day that
                         * coincides to event day. E.g.: if an event is set to
                         * 30th Jan, no marks will be displayed on Feb, but
                         * will be displayed on 30th Mar. */
  ELM_CALENDAR_ANNUALLY, /**< Marks will be displayed every year that coincides
                          * to event day (and month). E.g. an event added to
                          * 30th Jan 2012 will be repeated on 30th Jan 2013. */
  ELM_CALENDAR_LAST_DAY_OF_MONTH, /**< Marks will be displayed every last day of
                                   * month after event day (inclusive).
                                   *
                                   * @since 1.7 */
  ELM_CALENDAR_REVERSE_DAILY /**< Marks will be displayed every day before event
                              * day.
                              *
                              * @since 1.19 */
} Elm_Calendar_Mark_Repeat_Type;

/**
 * @brief A weekday
 *
 * See also @ref elm_obj_calendar_first_day_of_week_set.
 *
 * @ingroup Elm_Calendar
 */
typedef enum
{
  ELM_DAY_SUNDAY = 0, /**< Sunday weekday */
  ELM_DAY_MONDAY, /**< Monday weekday */
  ELM_DAY_TUESDAY, /**< Tuesday weekday */
  ELM_DAY_WEDNESDAY, /**< Wednesday weekday */
  ELM_DAY_THURSDAY, /**< Thursday weekday */
  ELM_DAY_FRIDAY, /**< Friday weekday */
  ELM_DAY_SATURDAY, /**< Saturday weekday */
  ELM_DAY_LAST /**< Sentinel value to indicate last enum field during iteration
                */
} Elm_Calendar_Weekday;

/**
 * @brief The mode, who determine how user could select a day
 *
 * See also @ref elm_obj_calendar_select_mode_set()
 *
 * @ingroup Elm_Calendar_Select
 */
typedef enum
{
  ELM_CALENDAR_SELECT_MODE_DEFAULT = 0, /**< Default value. A day is always
                                         * selected. */
  ELM_CALENDAR_SELECT_MODE_ALWAYS, /**< A day is always selected. */
  ELM_CALENDAR_SELECT_MODE_NONE, /**< None of the days can be selected. */
  ELM_CALENDAR_SELECT_MODE_ONDEMAND /**< User may have selected a day or not. */
} Elm_Calendar_Select_Mode;

/**
 * @brief A bitmask used to define which fields of a @c tm struct will be taken
 * into account, when elm_calendar_selected_time_set() is invoked.
 *
 * See also @ref elm_obj_calendar_selectable_set,
 * @ref elm_obj_calendar_selected_time_set.
 *
 * @since 1.8
 *
 * @ingroup Elm_Calendar
 */
typedef enum
{
  ELM_CALENDAR_SELECTABLE_NONE = 0, /**< Take no field into account */
  ELM_CALENDAR_SELECTABLE_YEAR = 1 /* 1 >> 0 */, /**< Take year field into
                                                  * account */
  ELM_CALENDAR_SELECTABLE_MONTH = 2 /* 1 >> 1 */, /**< Take month field into
                                                   * account */
  ELM_CALENDAR_SELECTABLE_DAY = 4 /* 1 >> 2 */ /**< Take day field into account
                                                */
} Elm_Calendar_Selectable;

/** Item handle for a calendar mark. Created with
 * @ref elm_obj_calendar_mark_add and deleted with
 * @ref elm_obj_calendar_mark_del.
 *
 * @ingroup Elm_Calendar
 */
typedef struct _Elm_Calendar_Mark Elm_Calendar_Mark;


#endif
/**
 * @brief Calendar widget
 *
 * It helps applications to flexibly display a calendar with day of the week,
 * date, year and month. Applications are able to set specific dates to be
 * reported back, when selected, in the smart callbacks of the calendar widget.
 *
 * @ingroup Elm_Calendar
 */
#define ELM_CALENDAR_CLASS elm_calendar_class_get()

EWAPI const Efl_Class *elm_calendar_class_get(void);

/**
 * @brief The first day of week to use on calendar widgets'.
 *
 * @param[in] obj The object.
 * @param[in] day Weekday enum value, see @ref Elm_Calendar_Weekday
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_first_day_of_week_set(Eo *obj, Elm_Calendar_Weekday day);

/**
 * @brief The first day of week to use on calendar widgets'.
 *
 * @param[in] obj The object.
 *
 * @return Weekday enum value, see @ref Elm_Calendar_Weekday
 *
 * @ingroup Elm_Calendar
 */
EOAPI Elm_Calendar_Weekday elm_obj_calendar_first_day_of_week_get(const Eo *obj);

/**
 * @brief Define which fields of a tm struct will be taken into account, when
 * Elm.Calendar.selected_time.set is invoked.
 *
 * By Default the bitmask is set to use all fields of a tm struct (year, month
 * and day of the month).
 *
 * See also @ref elm_obj_calendar_selected_time_set.
 *
 * @param[in] obj The object.
 * @param[in] selectable A bitmask of Elm_Calendar_Selectable
 *
 * @since 1.8
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_selectable_set(Eo *obj, Elm_Calendar_Selectable selectable);

/**
 * @brief Define which fields of a tm struct will be taken into account, when
 * Elm.Calendar.selected_time.set is invoked.
 *
 * By Default the bitmask is set to use all fields of a tm struct (year, month
 * and day of the month).
 *
 * See also @ref elm_obj_calendar_selected_time_set.
 *
 * @param[in] obj The object.
 *
 * @return A bitmask of Elm_Calendar_Selectable
 *
 * @since 1.8
 *
 * @ingroup Elm_Calendar
 */
EOAPI Elm_Calendar_Selectable elm_obj_calendar_selectable_get(const Eo *obj);

/**
 * @brief The interval on time updates for an user mouse button hold on
 * calendar widgets' month/year selection.
 *
 * This interval value is decreased while the user holds the mouse pointer
 * either selecting next or previous month/year.
 *
 * This helps the user to get to a given month distant from the current one
 * easier/faster, as it will start to change quicker and quicker on mouse
 * button holds.
 *
 * The calculation for the next change interval value, starting from the one
 * set with this call, is the previous interval divided by 1.05, so it
 * decreases a little bit.
 *
 * The default starting interval value for automatic changes is 0.85 seconds.
 *
 * @param[in] obj The object.
 * @param[in] interval The (first) interval value in seconds
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_interval_set(Eo *obj, double interval);

/**
 * @brief The interval on time updates for an user mouse button hold on
 * calendar widgets' month/year selection.
 *
 * This interval value is decreased while the user holds the mouse pointer
 * either selecting next or previous month/year.
 *
 * This helps the user to get to a given month distant from the current one
 * easier/faster, as it will start to change quicker and quicker on mouse
 * button holds.
 *
 * The calculation for the next change interval value, starting from the one
 * set with this call, is the previous interval divided by 1.05, so it
 * decreases a little bit.
 *
 * The default starting interval value for automatic changes is 0.85 seconds.
 *
 * @param[in] obj The object.
 *
 * @return The (first) interval value in seconds
 *
 * @ingroup Elm_Calendar
 */
EOAPI double elm_obj_calendar_interval_get(const Eo *obj);

/**
 * @brief Weekdays names to be displayed by the calendar.
 *
 * By default, weekdays abbreviations get from system are displayed: E.g. for
 * an en_US locale: "Sun, Mon, Tue, Wed, Thu, Fri, Sat"
 *
 * The first string should be related to Sunday, the second to Monday...
 *
 * See also @ref elm_obj_calendar_weekdays_names_get.
 *
 * @ref calendar_example_02. @ref calendar_example_05.
 *
 * @param[in] obj The object.
 * @param[in] weekdays Array of seven strings to be used as weekday names.
 * Warning: It must have 7 elements, or it will access invalid memory. Warning:
 * The strings must be @c null terminated ('@\0').
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_weekdays_names_set(Eo *obj, const char **weekdays);

/**
 * @brief Weekdays names to be displayed by the calendar.
 *
 * By default, weekdays abbreviations get from system are displayed: E.g. for
 * an en_US locale: "Sun, Mon, Tue, Wed, Thu, Fri, Sat"
 *
 * The first string should be related to Sunday, the second to Monday...
 *
 * See also @ref elm_obj_calendar_weekdays_names_get.
 *
 * @ref calendar_example_02. @ref calendar_example_05.
 *
 * @param[in] obj The object.
 *
 * @return Array of seven strings to be used as weekday names. Warning: It must
 * have 7 elements, or it will access invalid memory. Warning: The strings must
 * be @c null terminated ('@\0').
 *
 * @ingroup Elm_Calendar
 */
EOAPI const char **elm_obj_calendar_weekdays_names_get(const Eo *obj);

/**
 * @brief Select day mode to use.
 *
 * The day selection mode used.
 *
 * @param[in] obj The object.
 * @param[in] mode The select mode to use.
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_select_mode_set(Eo *obj, Elm_Calendar_Select_Mode mode);

/**
 * @brief Select day mode to use.
 *
 * The day selection mode used.
 *
 * @param[in] obj The object.
 *
 * @return The select mode to use.
 *
 * @ingroup Elm_Calendar
 */
EOAPI Elm_Calendar_Select_Mode elm_obj_calendar_select_mode_get(const Eo *obj);

/**
 * @brief Set a function to format the string that will be used to display
 * month and year;
 *
 * By default it uses strftime with "%B %Y" format string. It should allocate
 * the memory that will be used by the string, that will be freed by the widget
 * after usage. A pointer to the string and a pointer to the time struct will
 * be provided.
 *
 * @ref calendar_example_02.
 *
 * @param[in] obj The object.
 * @param[in] format_function Function to set the month-year string given the
 * selected date.
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_format_function_set(Eo *obj, Elm_Calendar_Format_Cb format_function);

/**
 * @brief Get a list of all the calendar marks.
 *
 * See also @ref elm_obj_calendar_mark_add, @ref elm_obj_calendar_mark_del(),
 * @ref elm_obj_calendar_marks_clear.
 *
 * @param[in] obj The object.
 *
 * @return List with all calendar marks
 *
 * @ingroup Elm_Calendar
 */
EOAPI const Eina_List *elm_obj_calendar_marks_get(const Eo *obj);

/**
 * @brief Minimum date on calendar.
 *
 * See also @ref elm_obj_calendar_date_max_set,
 * @ref elm_obj_calendar_date_max_get
 *
 * Set minimum date on calendar.
 *
 * Set the minimum date, changing the displayed month or year if needed.
 * Displayed day also to be disabled if it is smaller than minimum date.
 *
 * @param[in] obj The object.
 * @param[in] min A tm struct to point to minimum date.
 *
 * @since 1.19
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_date_min_set(Eo *obj, const Efl_Time *min);

/**
 * @brief Minimum date on calendar.
 *
 * See also @ref elm_obj_calendar_date_max_set,
 * @ref elm_obj_calendar_date_max_get
 *
 * Get minimum date.
 *
 * Default value is 1 JAN,1902.
 *
 * @param[in] obj The object.
 *
 * @return A tm struct to point to minimum date.
 *
 * @since 1.19
 *
 * @ingroup Elm_Calendar
 */
EOAPI const Efl_Time *elm_obj_calendar_date_min_get(const Eo *obj);

/**
 * @brief Maximum date on calendar.
 *
 * See also @ref elm_obj_calendar_date_min_set,
 * @ref elm_obj_calendar_date_min_get
 *
 * Set maximum date on calendar.
 *
 * Set the maximum date, changing the displayed month or year if needed.
 * Displayed day also to be disabled if it is bigger than maximum date.
 *
 * @param[in] obj The object.
 * @param[in] max A tm struct to point to maximum date.
 *
 * @since 1.19
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_date_max_set(Eo *obj, const Efl_Time *max);

/**
 * @brief Maximum date on calendar.
 *
 * See also @ref elm_obj_calendar_date_min_set,
 * @ref elm_obj_calendar_date_min_get
 *
 * Get maximum date.
 *
 * Default maximum year is -1. Default maximum day and month are 31 and DEC.
 *
 * If the maximum year is a negative value, it will be limited depending on the
 * platform architecture (year 2037 for 32 bits);
 *
 * @param[in] obj The object.
 *
 * @return A tm struct to point to maximum date.
 *
 * @since 1.19
 *
 * @ingroup Elm_Calendar
 */
EOAPI const Efl_Time *elm_obj_calendar_date_max_get(const Eo *obj);

/**
 * @brief Set selected date to be highlighted on calendar.
 *
 * Set the selected date, changing the displayed month if needed. Selected date
 * changes when the user goes to next/previous month or select a day pressing
 * over it on calendar.
 *
 * See also @ref elm_obj_calendar_selected_time_get.
 *
 * @ref calendar_example_04
 *
 * @param[in] obj The object.
 * @param[in] selected_time A tm struct to represent the selected date.
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_selected_time_set(Eo *obj, Efl_Time *selected_time);

/**
 * @brief Get selected date.
 *
 * Get date selected by the user or set by function
 * @ref elm_obj_calendar_selected_time_set(). Selected date changes when the
 * user goes to next/previous month or select a day pressing over it on
 * calendar.
 *
 * See also @ref elm_obj_calendar_selected_time_get.
 *
 * @ref calendar_example_05.
 *
 * @param[in] obj The object.
 * @param[in,out] selected_time A tm struct to point to selected date.
 *
 * @return @c true if the method succeeded, @c false otherwise
 *
 * @ingroup Elm_Calendar
 */
EOAPI Eina_Bool elm_obj_calendar_selected_time_get(const Eo *obj, Efl_Time *selected_time);

/**
 * @brief Add a new mark to the calendar
 *
 * Add a mark that will be drawn in the calendar respecting the insertion time
 * and periodicity. It will emit the type as signal to the widget theme.
 * Default theme supports "holiday" and "checked", but it can be extended.
 *
 * It won't immediately update the calendar, drawing the marks. For this,
 * @ref elm_obj_calendar_marks_draw(). However, when user selects next or
 * previous month calendar forces marks drawn.
 *
 * Marks created with this method can be deleted with
 * @ref elm_obj_calendar_mark_del().
 *
 * See also @ref elm_obj_calendar_marks_draw, @ref elm_obj_calendar_mark_del().
 *
 * @ref calendar_example_06
 *
 * @param[in] obj The object.
 * @param[in] mark_type A string used to define the type of mark. It will be
 * emitted to the theme, that should display a related modification on these
 * days representation.
 * @param[in] mark_time A time struct to represent the date of inclusion of the
 * mark. For marks that repeats it will just be displayed after the inclusion
 * date in the calendar.
 * @param[in] repeat Repeat the event following this periodicity. Can be a
 * unique mark (that don't repeat), daily, weekly, monthly or annually.
 *
 * @return The newly added calendar mark
 *
 * @ingroup Elm_Calendar
 */
EOAPI Elm_Calendar_Mark *elm_obj_calendar_mark_add(Eo *obj, const char *mark_type, Efl_Time *mark_time, Elm_Calendar_Mark_Repeat_Type repeat);

/**
 * @brief Delete mark from the calendar.
 *
 * If deleting all calendar marks is required,
 * @ref elm_obj_calendar_marks_clear() should be used instead of getting marks
 * list and deleting each one.
 *
 * See also @ref elm_obj_calendar_mark_add(),
 * @ref elm_obj_calendar_marks_clear().
 *
 * @param[in] obj The object.
 * @param[in] mark The mark to be deleted.
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_mark_del(Eo *obj, Elm_Calendar_Mark *mark);

/**
 * @brief Remove all calendar's marks
 *
 * See also  @ref elm_obj_calendar_mark_add, @ref elm_obj_calendar_mark_del().
 * @param[in] obj The object.
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_marks_clear(Eo *obj);

/**
 * @brief Draw calendar marks.
 *
 * Should be used after adding, removing or clearing marks. It will go through
 * the entire marks list updating the calendar. If lots of marks will be added,
 * add all the marks and then call this function.
 *
 * When the month is changed, i.e. user selects next or previous month, marks
 * will be drawn.
 *
 * See also  @ref elm_obj_calendar_mark_add, @ref elm_obj_calendar_mark_del(),
 * @ref elm_obj_calendar_marks_clear.
 *
 * @ref calendar_example_06
 * @param[in] obj The object.
 *
 * @ingroup Elm_Calendar
 */
EOAPI void elm_obj_calendar_marks_draw(Eo *obj);

/**
 * @brief Get the current time displayed in the widget
 *
 * @param[in] obj The object.
 * @param[in,out] displayed_time A tm struct to point to displayed date.
 *
 * @return @c true if the method succeeded, @c false otherwise
 *
 * @since 1.8
 *
 * @ingroup Elm_Calendar
 */
EOAPI Eina_Bool elm_obj_calendar_displayed_time_get(const Eo *obj, Efl_Time *displayed_time);

EWAPI extern const Efl_Event_Description _ELM_CALENDAR_EVENT_CHANGED;

/** Emitted when the date in the calendar is changed
 *
 * @ingroup Elm_Calendar
 */
#define ELM_CALENDAR_EVENT_CHANGED (&(_ELM_CALENDAR_EVENT_CHANGED))

EWAPI extern const Efl_Event_Description _ELM_CALENDAR_EVENT_DISPLAY_CHANGED;

/** Emitted when the current month displayed in the calendar is changed
 *
 * @ingroup Elm_Calendar
 */
#define ELM_CALENDAR_EVENT_DISPLAY_CHANGED (&(_ELM_CALENDAR_EVENT_DISPLAY_CHANGED))

#endif
