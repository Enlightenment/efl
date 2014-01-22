/**
 * @defgroup Fileselector File Selector
 * @ingroup Elementary
 *
 * @image html fileselector_inheritance_tree.png
 * @image latex fileselector_inheritance_tree.eps
 *
 * @image html img/widget/fileselector/preview-00.png
 * @image latex img/widget/fileselector/preview-00.eps
 *
 * A file selector is a widget that allows a user to navigate
 * through a file system, reporting file selections back via its
 * API.
 *
 * It contains shortcut buttons for home directory (@c ~) and to
 * jump one directory upwards (..), as well as cancel/ok buttons to
 * confirm/cancel a given selection. After either one of those two
 * former actions, the file selector will issue its @c "done" smart
 * callback.
 *
 * There's a text entry on it, too, showing the name of the current
 * selection. There's the possibility of making it editable, so it
 * is useful on file saving dialogs on applications, where one
 * gives a file name to save contents to, in a given directory in
 * the system. This custom file name will be reported on the @c
 * "done" smart callback (explained in sequence).
 *
 * Finally, it has a view to display file system items into in two
 * possible forms:
 * - list
 * - grid
 *
 * If Elementary is built with support of the Ethumb thumbnailing
 * library, the second form of view will display preview thumbnails
 * of files which it supports.
 *
 * This widget inherits from the Layout one, so that all the
 * functions acting on it also work for file selector objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "activated" - the user activated a file. This can happen by
 *      double-clicking or pressing Enter key. (@c event_info is a
 *      pointer to the activated file path)
 * - @c "selected" - the user has clicked on a file (when not in
 *      folders-only mode) or directory (when in folders-only mode)
 * - @c "selected,invalid" - the user has tried to access wrong path
 *      which does not exist.
 * - @c "directory,open" - the list has been populated with new
 *      content (@c event_info is a pointer to the directory's
 *      path, a @b stringshared string)
 * - @c "done" - the user has clicked on the "ok" or "cancel"
 *      buttons (@c event_info is a pointer to the selection's
 *      path, a @b stringshared string)
 * - @c "focused" - When the fileselector has received focus. (since 1.9)
 * - @c "unfocused" - When the fileselector has lost focus. (since 1.9)
 *
 * For text, elm_layout_text_set() will work here on:
 * @li @c "ok" - OK button label if the ok button is set. @since 1.8
 * @li @c "cancel" - Cancel button label if the cancel button is set. @since 1.8
 *
 * Here is an example on its usage:
 * @li @ref fileselector_example
 */

/**
 * @addtogroup Fileselector
 * @{
 */

#include "elc_fileselector_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elc_fileselector_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elc_fileselector_legacy.h"
#endif
/**
 * @}
 */
