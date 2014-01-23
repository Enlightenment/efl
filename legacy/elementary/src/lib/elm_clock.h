/**
 * @defgroup Clock Clock
 * @ingroup Elementary
 *
 * @image html clock_inheritance_tree.png
 * @image latex clock_inheritance_tree.eps
 *
 * @image html img/widget/clock/preview-00.png
 * @image latex img/widget/clock/preview-00.eps
 *
 * This is a @b digital clock widget. In its default theme, it has a
 * vintage "flipping numbers clock" appearance, which will animate
 * sheets of individual algorisms individually as time goes by.
 *
 * A newly created clock will fetch system's time (already
 * considering local time adjustments) to start with, and will tick
 * accordingly. It may or may not show seconds.
 *
 * Clocks have an @b edition  mode. When in it, the sheets will
 * display extra arrow indications on the top and bottom and the
 * user may click on them to raise or lower the time values. After
 * it's told to exit edition mode, it will keep ticking with that
 * new time set (it keeps the difference from local time).
 *
 * Also, when under edition mode, user clicks on the cited arrows
 * which are @b held for some time will make the clock to flip the
 * sheet, thus editing the time, continuously and automatically for
 * the user. The interval between sheet flips will keep reducing in
 * time, so that it helps the user to reach a time which is distant
 * from the one set.
 *
 * The time display is, by default, in military mode (24h), but an
 * am/pm indicator may be optionally shown, too, when it will
 * switch to 12h.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for clock objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "changed" - the clock's user changed the time
 * - @c "focused" - When the clock ehas received focus. (since 1.8)
 * - @c "unfocused" - When the clock has lost focus. (since 1.8)
 * - @c "language,changed" - the program's language changed (since 1.9)
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 *
 * Here is an example on its usage:
 * @li @ref clock_example
 */

/**
 * @addtogroup Clock
 * @{
 */

#include "elm_clock_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_clock_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_clock_legacy.h"
#endif
/**
 * @}
 */
