/**
 * @addtogroup Calendar
 *
 * @{
 */

typedef struct _Elm_Calendar_Mark Elm_Calendar_Mark;    /**< Item handle for a calendar mark. Created with elm_calendar_mark_add() and deleted with elm_calendar_mark_del(). */

/**
 * This callback type is used to format the string that will be used
 * to display month and year.
 *
 * @param stime Struct representing time.
 * @return String representing time that will be set to calendar's text.
 *
 * @see elm_calendar_format_function_set()
 */
typedef char * (*Elm_Calendar_Format_Cb)(struct tm *stime);

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
 */
EAPI void                 elm_calendar_mark_del(Elm_Calendar_Mark *mark);

/* temporary until better solution is found: is here because of eolian */
typedef struct tm Elm_Calendar_Time;

/**
 * @}
 */
