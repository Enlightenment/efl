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

typedef enum
{
   ELM_CALENDAR_SELECTABLE_NONE = 0,
   ELM_CALENDAR_SELECTABLE_YEAR = (1 << 0),
   ELM_CALENDAR_SELECTABLE_MONTH = (1 << 1),
   ELM_CALENDAR_SELECTABLE_DAY = (1 << 2)
} _Elm_Calendar_Selectable;

/**
 * @enum _Elm_Calendar_Selectable
 * @typedef Elm_Calendar_Selectable
 *
 * A bitmask used to define which fields of a @b tm struct will be taken into
 * account, when elm_calendar_selected_time_set() is invoked.
 *
 * @ingroup Calendar
 * @see elm_calendar_selectable_set()
 * @see elm_calendar_selected_time_set()
 * @since 1.8
 */
typedef _Elm_Calendar_Selectable Elm_Calendar_Selectable;

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

