#define ELM_OBJ_CALENDAR_CLASS elm_obj_calendar_class_get()

const Eo_Class *elm_obj_calendar_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_CALENDAR_BASE_ID;

enum
{
   ELM_OBJ_CALENDAR_SUB_ID_WEEKDAYS_NAMES_SET,
   ELM_OBJ_CALENDAR_SUB_ID_WEEKDAYS_NAMES_GET,
   ELM_OBJ_CALENDAR_SUB_ID_INTERVAL_SET,
   ELM_OBJ_CALENDAR_SUB_ID_INTERVAL_GET,
   ELM_OBJ_CALENDAR_SUB_ID_MIN_MAX_YEAR_SET,
   ELM_OBJ_CALENDAR_SUB_ID_MIN_MAX_YEAR_GET,
   ELM_OBJ_CALENDAR_SUB_ID_SELECTED_TIME_SET,
   ELM_OBJ_CALENDAR_SUB_ID_SELECTED_TIME_GET,
   ELM_OBJ_CALENDAR_SUB_ID_FORMAT_FUNCTION_SET,
   ELM_OBJ_CALENDAR_SUB_ID_MARK_ADD,
   ELM_OBJ_CALENDAR_SUB_ID_MARKS_CLEAR,
   ELM_OBJ_CALENDAR_SUB_ID_MARKS_GET,
   ELM_OBJ_CALENDAR_SUB_ID_MARKS_DRAW,
   ELM_OBJ_CALENDAR_SUB_ID_FIRST_DAY_OF_WEEK_SET,
   ELM_OBJ_CALENDAR_SUB_ID_FIRST_DAY_OF_WEEK_GET,
   ELM_OBJ_CALENDAR_SUB_ID_SELECT_MODE_SET,
   ELM_OBJ_CALENDAR_SUB_ID_SELECT_MODE_GET,
   ELM_OBJ_CALENDAR_SUB_ID_SELECTABLE_SET,
   ELM_OBJ_CALENDAR_SUB_ID_SELECTABLE_GET,
   ELM_OBJ_CALENDAR_SUB_ID_DISPLAYED_TIME_GET,
   ELM_OBJ_CALENDAR_SUB_ID_LAST
};

#define ELM_OBJ_CALENDAR_ID(sub_id) (ELM_OBJ_CALENDAR_BASE_ID + sub_id)

/**
 * @def elm_obj_calendar_weekdays_names_set
 * @since 1.8
 *
 * Set weekdays names to be displayed by the calendar.
 *
 * @param[in] weekdays
 *
 * @see elm_calendar_weekdays_names_set
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_weekdays_names_set(weekdays) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_WEEKDAYS_NAMES_SET), EO_TYPECHECK(const char **, weekdays)

/**
 * @def elm_obj_calendar_weekdays_names_get
 * @since 1.8
 *
 * Get weekdays names displayed by the calendar.
 *
 * @param[out] ret
 *
 * @see elm_calendar_weekdays_names_get
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_weekdays_names_get(ret) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_WEEKDAYS_NAMES_GET), EO_TYPECHECK(const char ***, ret)

/**
 * @def elm_obj_calendar_interval_set
 * @since 1.8
 *
 * Set the interval on time updates for an user mouse button hold
 *
 * @param[in] interval
 *
 * @see elm_calendar_interval_set
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_interval_set(interval) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_INTERVAL_SET), EO_TYPECHECK(double, interval)

/**
 * @def elm_obj_calendar_interval_get
 * @since 1.8
 *
 * Get the interval on time updates for an user mouse button hold
 *
 * @param[out] ret
 *
 * @see elm_calendar_interval_get
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_interval_get(ret) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_INTERVAL_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_calendar_min_max_year_set
 * @since 1.8
 *
 * Set the minimum and maximum values for the year
 *
 * @param[in] min
 * @param[in] max
 *
 * @see elm_calendar_min_max_year_set
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_min_max_year_set(min, max) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_MIN_MAX_YEAR_SET), EO_TYPECHECK(int, min), EO_TYPECHECK(int, max)

/**
 * @def elm_obj_calendar_min_max_year_get
 * @since 1.8
 *
 * Get the minimum and maximum values for the year
 *
 * @param[out] min
 * @param[out] max
 *
 * @see elm_calendar_min_max_year_get
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_min_max_year_get(min, max) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_MIN_MAX_YEAR_GET), EO_TYPECHECK(int *, min), EO_TYPECHECK(int *, max)

/**
 * @def elm_obj_calendar_selected_time_set
 * @since 1.8
 *
 * Set selected date to be highlighted on calendar.
 *
 * @param[in] selected_time
 *
 * @see elm_calendar_selected_time_set
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_selected_time_set(selected_time) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_SELECTED_TIME_SET), EO_TYPECHECK(struct tm *, selected_time)

/**
 * @def elm_obj_calendar_selected_time_get
 * @since 1.8
 *
 * Get selected date.
 *
 * @param[in,out] selected_time
 * @param[out] ret
 *
 * @see elm_calendar_selected_time_get
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_selected_time_get(selected_time, ret) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_SELECTED_TIME_GET), EO_TYPECHECK(struct tm *, selected_time), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_calendar_format_function_set
 * @since 1.8
 *
 * Set a function to format the string that will be used to display
 * to display month and year.
 *
 * @param[in] format_function
 *
 * @see elm_calendar_format_function_set
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_format_function_set(format_function) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_FORMAT_FUNCTION_SET), EO_TYPECHECK(Elm_Calendar_Format_Cb, format_function)

/**
 * @def elm_obj_calendar_mark_add
 * @since 1.8
 *
 * Add a new mark to the calendar
 *
 * @param[in] mark_type
 * @param[in] mark_time
 * @param[in] repeat
 * @param[out] ret
 *
 * @see elm_calendar_mark_add
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_mark_add(mark_type, mark_time, repeat, ret) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_MARK_ADD), EO_TYPECHECK(const char *, mark_type), EO_TYPECHECK(struct tm *, mark_time), EO_TYPECHECK(Elm_Calendar_Mark_Repeat_Type, repeat), EO_TYPECHECK(Elm_Calendar_Mark **, ret)

/**
 * @def elm_obj_calendar_marks_clear
 * @since 1.8
 *
 * Remove all calendar's marks
 *
 *
 * @see elm_calendar_marks_clear
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_marks_clear() ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_MARKS_CLEAR)

/**
 * @def elm_obj_calendar_marks_get
 * @since 1.8
 *
 * Get a list of all the calendar marks.
 *
 * @param[out] ret
 *
 * @see elm_calendar_marks_get
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_marks_get(ret) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_MARKS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_calendar_marks_draw
 * @since 1.8
 *
 * Draw calendar marks.
 *
 *
 * @see elm_calendar_marks_draw
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_marks_draw() ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_MARKS_DRAW)

/**
 * @def elm_obj_calendar_first_day_of_week_set
 * @since 1.8
 *
 * Set the first day of week to use on calendar widgets'.
 *
 * @param[in] day
 *
 * @see elm_calendar_first_day_of_week_set
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_first_day_of_week_set(day) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_FIRST_DAY_OF_WEEK_SET), EO_TYPECHECK(Elm_Calendar_Weekday, day)

/**
 * @def elm_obj_calendar_first_day_of_week_get
 * @since 1.8
 *
 * Get the first day of week, who are used on calendar widgets'.
 *
 * @param[out] ret
 *
 * @see elm_calendar_first_day_of_week_get
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_first_day_of_week_get(ret) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_FIRST_DAY_OF_WEEK_GET), EO_TYPECHECK(Elm_Calendar_Weekday *, ret)

/**
 * @def elm_obj_calendar_select_mode_set
 * @since 1.8
 *
 * Set select day mode to use.
 *
 * @param[in] mode
 *
 * @see elm_calendar_select_mode_set
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_select_mode_set(mode) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_SELECT_MODE_SET), EO_TYPECHECK(Elm_Calendar_Select_Mode, mode)

/**
 * @def elm_obj_calendar_select_mode_get
 * @since 1.8
 *
 * Get the select day mode used.
 *
 * @param[out] ret
 *
 * @see elm_calendar_select_mode_get
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_select_mode_get(ret) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_SELECT_MODE_GET), EO_TYPECHECK(Elm_Calendar_Select_Mode *, ret)

/**
 * @def elm_obj_calendar_selectable_set
 * @since 1.8
 *
 * Define which fields of a tm struct will be taken into account, when
 * elm_calendar_selected_time_set() is invoked.
 *
 * @param[in] selectable
 *
 * @see elm_calendar_selectable_set
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_selectable_set(selectable) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_SELECTABLE_SET), EO_TYPECHECK(Elm_Calendar_Selectable, selectable)

/**
 * @def elm_obj_calendar_selectable_get
 * @since 1.8
 *
 * Get how elm_calendar_selected_time_set manage a date
 *
 * @param[out] ret
 *
 * @see elm_calendar_selectable_get
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_selectable_get(ret) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_SELECTABLE_GET), EO_TYPECHECK(Elm_Calendar_Selectable *, ret)

/**
 * @def elm_obj_calendar_displayed_time_get
 * @since 1.8
 *
 * Get the current time displayed in the widget
 *
 * @param[in,out] displayed_time
 * @param[out] ret
 *
 * @see elm_calendar_displayed_time_get
 *
 * @ingroup Calendar
 */
#define elm_obj_calendar_displayed_time_get(displayed_time, ret) ELM_OBJ_CALENDAR_ID(ELM_OBJ_CALENDAR_SUB_ID_DISPLAYED_TIME_GET), EO_TYPECHECK(struct tm *, displayed_time), EO_TYPECHECK(Eina_Bool *, ret)
