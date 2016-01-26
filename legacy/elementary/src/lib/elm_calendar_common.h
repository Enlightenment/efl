/**
 * @addtogroup Calendar
 *
 * @{
 */

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

/* temporary until better solution is found: is here because of eolian */
typedef struct tm Elm_Calendar_Time;

/**
 * @}
 */
