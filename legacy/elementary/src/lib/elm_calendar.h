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
 * - @c "display,changed" - emitted when the current month displayed in the
 * calendar is changed.
 * - @c "focused" - When the calendar has received focus. (since 1.8)
 * - @c "unfocused" - When the calendar has lost focus. (since 1.8)
 * - @c "language,changed" - the program's language changed (since 1.9)
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

#include "elm_calendar_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_calendar_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_calendar_legacy.h"
#endif
/**
 * @}
 */
