/**
 * @defgroup File_Selector_Button File Selector Button
 * @ingroup Elementary
 *
 * @image html fileselector_button_inheritance_tree.png
 * @image latex fileselector_button_inheritance_tree.eps
 *
 * @image html img/widget/fileselector_button/preview-00.png
 * @image latex img/widget/fileselector_button/preview-00.eps
 * @image html img/widget/fileselector_button/preview-01.png
 * @image latex img/widget/fileselector_button/preview-01.eps
 * @image html img/widget/fileselector_button/preview-02.png
 * @image latex img/widget/fileselector_button/preview-02.eps
 *
 * This is a button that, when clicked, creates an Elementary
 * window (or inner window) <b> with a @ref Fileselector "file
 * selector widget" within</b>. When a file is chosen, the (inner)
 * window is closed and the button emits a signal having the
 * selected file as it's @c event_info.
 *
 * This widget encapsulates operations on its internal file
 * selector on its own API. There is less control over its file
 * selector than that one would have instantiating one directly.
 *
 * The following styles are available for this button:
 * @li @c "default"
 * @li @c "anchor"
 * @li @c "hoversel_vertical"
 * @li @c "hoversel_vertical_entry"
 *
 * This widget inherits from the @ref Button one, so that all the
 * functions acting on it also work for file selector button objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Button:
 * - @c "file,chosen" - the user has selected a path, whose string
 *   pointer comes as the @c event_info data (a stringshared
 *   string)
 * - @c "language,changed" - the program's language changed
 * - @c "focused" - When the fileselector button has received focus. (since 1.8)
 * - @c "unfocused" - When the fileselector button has lost focus. (since 1.8)
 *
 * Default text parts of the fileselector_button widget that you can use for
 * are:
 * @li "default" - Label of the fileselector_button
 *
 * Default content parts of the fileselector_button widget that you can use for
 * are:
 * @li "icon" - Icon of the fileselector_button
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 * @li @ref elm_object_disabled_set
 * @li @ref elm_object_disabled_get
 *
 * Here is an example on its usage:
 * @li @ref fileselector_button_example
 *
 * @see @ref File_Selector_Entry for a similar widget.
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elc_fileselector_button_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elc_fileselector_button_legacy.h"
#endif
/**
 * @}
 */
