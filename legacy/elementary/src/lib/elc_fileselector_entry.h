/**
 * @defgroup File_Selector_Entry File Selector Entry
 * @ingroup Elementary
 *
 * @image html fileselector_entry_inheritance_tree.png
 * @image latex fileselector_entry_inheritance_tree.eps
 *
 * @image html img/widget/fileselector_entry/preview-00.png
 * @image latex img/widget/fileselector_entry/preview-00.eps
 *
 * This is an entry made to be filled with or display a <b>file
 * system path string</b>. Besides the entry itself, the widget has
 * a @ref File_Selector_Button "file selector button" on its side,
 * which will raise an internal @ref Fileselector "file selector widget",
 * when clicked, for path selection aided by file system
 * navigation.
 *
 * This file selector may appear in an Elementary window or in an
 * inner window. When a file is chosen from it, the (inner) window
 * is closed and the selected file's path string is exposed both as
 * a smart event and as the new text on the entry.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for file selector entry objects
 * (@since 1.8).
 *
 * This widget encapsulates operations on its internal file
 * selector on its own API. There is less control over its file
 * selector than that one would have instantiating one directly.
 *
 * Smart callbacks one can register to:
 * - @c "changed" - The text within the entry was changed
 * - @c "activated" - The entry has had editing finished and
 *   changes are to be "committed"
 * - @c "press" - The entry has been clicked
 * - @c "longpressed" - The entry has been clicked (and held) for a
 *   couple seconds
 * - @c "clicked" - The entry has been clicked
 * - @c "clicked,double" - The entry has been double clicked
 * - @c "focused" - The entry has received focus (since 1.8)
 * - @c "unfocused" - The entry has lost focus (since 1.8)
 * - @c "selection,paste" - A paste action has occurred on the
 *   entry
 * - @c "selection,copy" - A copy action has occurred on the entry
 * - @c "selection,cut" - A cut action has occurred on the entry
 * - @c "unpressed" - The file selector entry's button was released
 *   after being pressed.
 * - @c "file,chosen" - The user has selected a path via the file
 *   selector entry's internal file selector, whose string pointer
 *   comes as the @c event_info data (a stringshared string)
 * - @c "language,changed" - the program's language changed
 *
 * Default text parts of the fileselector_button widget that you can use for
 * are:
 * @li "default" - Label of the fileselector_button
 *
 * Default content parts of the fileselector_entry widget that you can use for
 * are:
 * @li "button icon" - Button icon of the fileselector_entry
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
 * @li @ref fileselector_entry_example
 *
 * @see @ref File_Selector_Button for a similar widget.
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elc_fileselector_entry_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elc_fileselector_entry_legacy.h"
#endif
/**
 * @}
 */
