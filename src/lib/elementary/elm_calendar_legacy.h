#include "elm_calendar_eo.legacy.h"

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
 * @ingroup Elm_Calendar
 */
EAPI Evas_Object         *elm_calendar_add(Evas_Object *parent);

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

/**
 * Set the minimum and maximum values for the year.
 *
 * @param min The minimum year, greater than 1901.
 * @param max The maximum year.
 *
 * Maximum must be greater than minimum, except if you don't want to set maximum year.
 * Default values are 1902 and -1.
 * If the maximum year is a negative value, it will be limited depending on the platform architecture. (year 2037 for 32 bits)
 *
 * @see elm_calendar_min_max_year_get()
 *
 * @ref calendar_example_03
 */
EAPI void                 elm_calendar_min_max_year_set(Elm_Calendar *obj, int min, int max);

/**
 * Get the minimum and maximum values for the year.
 *
 * @param[out] min The minimum year, greater than 1901.
 * @param[out] max The maximum year.
 *
 * Default values are 1902 and -1.
 *
 * @see elm_calendar_min_max_year_set()
 *
 * @ref calendar_example_05
 */
EAPI void                 elm_calendar_min_max_year_get(const Elm_Calendar *obj, int *min, int *max);
