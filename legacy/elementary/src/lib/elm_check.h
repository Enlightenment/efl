/**
 * @defgroup Check Check
 * @ingroup Elementary
 *
 * @image html check_inheritance_tree.png
 * @image latex check_inheritance_tree.eps
 *
 * @image html img/widget/check/preview-00.png
 * @image latex img/widget/check/preview-00.eps
 * @image html img/widget/check/preview-01.png
 * @image latex img/widget/check/preview-01.eps
 * @image html img/widget/check/preview-02.png
 * @image latex img/widget/check/preview-02.eps
 *
 * @brief The check widget allows for toggling a value between true
 * and false.
 *
 * Check objects are a lot like radio objects in layout and
 * functionality, except they do not work as a group, but
 * independently, and only toggle the value of a boolean between false
 * and true. elm_check_state_set() sets the boolean state and
 * elm_check_state_get() returns the current state. For convenience,
 * like the radio objects, you can set a pointer to a boolean directly
 * with elm_check_state_pointer_set() for it to modify.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for check objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "changed" - This is called whenever the user changes the state of
 *             the check objects (@c event_info is always @c NULL).
 * - @c "focused" - When the check has received focus. (since 1.8)
 * - @c "unfocused" - When the check has lost focus. (since 1.8)
 * - @c "language,changed" - the program's language changed (since 1.9)
 *
 * Default content parts of the check widget that you can use for are:
 * @li "icon" - An icon of the check
 *
 * Default text parts of the check widget that you can use for are:
 * @li "default" - A label of the check
 * @li "on" - On state label of the check (only valid for "toggle" style.)
 * @li "off" - Off state label of the check (only valid for "toggle" style.)
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 *
 * @ref tutorial_check should give you a firm grasp of how to use this widget.
 *
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_check_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_check_legacy.h"
#endif
/**
 * @}
 */
