#ifndef ELM_PREFS_H
#define ELM_PREFS_H

/**
 * @addtogroup Prefs
 * @{
 *
 * @section elm-prefs-widget Prefs
 *
 * @image html img/widget/prefs/preview-00.png
 * @image latex img/widget/prefs/preview-00.eps width=\textwidth
 *
 * @image html prefs_inheritance_tree.png
 * @image latex prefs_inheritance_tree.eps
 *
 * Prefs is a special widget inside Elementary. It is mainly targeted
 * to configuration dialogs. Its viewport is blank until you associate
 * a definition file to it. That file (usually with @b .epb
 * extension), is a binary format (Eet) one, coming from a
 * human-readable textual declaration. This textual form, an
 * <b>Elementary prefs collection</b> (usually with @b .epc
 * extension), is translated to the binary form by means of the
 * @b prefs_cc compiler.
 *
 * On it one declares UI elements to populate prefs widgets, which are
 * contained in @b pages (a prefs widget is always bound to one page).
 * Those elements get properties like:
 * - default value,
 * - maximum and minumum ranges,
 * - acceptance and denial regular expressions, for strings,
 * - visibility,
 * - persistence,
 * - editability, etc.
 *
 * Once a prefs widget is created, after elm_prefs_file_set() is
 * issued on it, all of its UI elements will get default values,
 * declared on that file. To fetch an user's own, personal set of
 * those values, one gets to pair an <b>@ref elm-prefs-data</b> handle
 * to the prefs widget.
 *
 * Personal, prefs data values with keys matching the ones present on
 * the file passed on elm_prefs_file_set() to will have their values
 * applied to the respective UI elements of the widget.
 *
 * When a prefs widget dies, the values of the elements declared on
 * its @b .epb file marked as permanent <b>will be written back</b> to
 * the user prefs data handle, if it is writable. One is also able to
 * make this writing event to take place automatically after each UI
 * element modification by using elm_prefs_autosave_set().
 *
 * This widget emits the following signals:
 * - @c "page,saved": a given page has had its values saved back on
 *                    user config file (event info is the page's
 *                    name)
 * - @c "page,reset": a given page has had its values reset (event
 *                    info is the page's name)
 * - @c "page,changed": a given page has been loaded <b>with user
 *                     (@c .cfg) values</b> (event info is the page's name)
 * - @c "page,loaded": a given page has been loaded <b>with system
 *                     (@c .epb) values</b> (event info is the page's name)
 * - @c "item,changed": a given page's item has changed (event info is
 *                      the item's full hierarchical name, in the form
 *                      @c "page:subpage:subsubpage:item")
 * - @c "action": an action item has been triggered (event info is the
 *                item's full hierarchical name, in the form @c
 *                "page:subpage:subsubpage:item")
 *
 * @li @ref epcref
 *
 * The following are examples on how to use prefs:
 * @li @ref prefs_example_01
 * @li @ref prefs_example_02
 * @li @ref prefs_example_03
 */

#include "elm_prefs_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_prefs_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_prefs_legacy.h"
#endif
/**
 * @}
 */
#endif
