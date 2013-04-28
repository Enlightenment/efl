/**
 * @defgroup Dayselector Dayselector
 * @ingroup Elementary
 *
 * @image html dayselector_inheritance_tree.png
 * @image latex dayselector_inheritance_tree.eps
 *
 * @image html img/widget/dayselector/preview-00.png
 * @image latex img/widget/dayselector/preview-00.eps
 *
 * @brief <b>Dayselector widget:</b>
 *
 *"elm_dayselector" is a day selection widget. It displays all seven days of
 * the week and allows the user to select multiple days.
 *
 * The selection can be toggle by just clicking on the day.
 *
 * Dayselector also provides the functionality to check whether a day is
 * selected or not.
 *
 * First day of the week is taken from config settings by default. It can be
 * altered by using the API elm_dayselector_week_start_set() API.
 *
 * APIs are provided for setting the duration of weekend
 * elm_dayselector_weekend_start_set() and elm_dayselector_weekend_length_set()
 * does this job.
 *
 * Two styles of weekdays and weekends are supported in Dayselector.
 * Application can emit signals on individual check objects for setting the
 * weekday, weekend styles.
 *
 * Once the weekend start day or weekend length changes, all the weekday &
 * weekend styles will be reset to default style. It's the application's
 * responsibility to set the styles again by sending corresponding signals.
 *
 * Supported elm_object_item common APIs.
 *
 * @li @ref elm_object_part_text_set,
 * @li @ref elm_object_part_text_get,
 * @li @ref elm_object_part_content_set,
 * @li @ref elm_object_part_content_get,
 * @li @ref elm_object_part_content_unset
 *
 * @li "day0" indicates Sunday, "day1" indicates Monday etc. continues and so,
 * "day6" indicates the Saturday part name.
 *
 * Application can change individual day display string by using the API
 * elm_object_part_text_set().
 *
 * elm_object_part_content_set() API sets the individual day object only if
 * the passed one is a Check widget.
 *
 * Check object representing a day can be set/get by the application by using
 * the elm_object_part_content_set/get APIs thus providing a way to handle
 * the different check styles for individual days.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for dayselector objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "dayselector,changed" - when the user changes the state of a day.
 * @li @c "language,changed" - the program's language changed
 *
 * Available styles for dayselector are:
 * @li default
 *
 * This example shows the usage of the widget.
 * @li @ref dayselector_example
 *
 */

/**
 * @addtogroup Dayselector
 * @{
 */

#include "elm_dayselector_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_dayselector_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_dayselector_legacy.h"
#endif
/**
 * @}
 */
