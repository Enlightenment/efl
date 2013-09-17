/**
 * @defgroup Multibuttonentry Multibuttonentry
 * @ingroup Elementary
 *
 * @image html multibuttonentry_inheritance_tree.png
 * @image latex multibuttonentry_inheritance_tree.eps
 *
 * A multi-button entry is a widget letting an user enter text and
 * each chunk of text managed as a set of buttons. Each text button is
 * inserted by pressing the "return" key. If there is no space in the
 * current row, a new button is added to the next row. When a text
 * button is pressed, it will become focused. Backspace removes the
 * focus. When the multi-button entry loses focus, items longer than
 * one line are shrunk to one line.
 *
 * The typical use case of multi-button entry is composing
 * emails/messages to a group of addresses, each of which is an item
 * that can be clicked for further actions.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for multi-button entry objects
 * (@since 1.8).
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "item,selected" - this is called when an item is selected by
 *       api, user interaction, and etc. this is also called when a
 *       user press back space while cursor is on the first field of
 *       entry.
 * - @c "item,added" - when a new multi-button entry item is added.
 * - @c "item,deleted" - when a multi-button entry item is deleted.
 * - @c "item,clicked" - this is called when an item is clicked by user
 *       interaction. Both "item,selected" and "item,clicked" are needed.
 * - @c "clicked" - when multi-button entry is clicked.
 * - @c "focused" - when multi-button entry is focused.
 * - @c "unfocused" - when multi-button entry is unfocused.
 * - @c "expanded" - when multi-button entry is expanded.
 * - @c "contracted" - when multi-button entry is contracted.
 * - @c "expand,state,changed" - when shrink mode state of
 *       multi-button entry is changed.
 *
 * Default text parts of the multi-button entry widget that you can use are:
 * @li "default" - A label of the multi-button entry
 *
 * Default text parts of multi-button entry @b items that you can use are:
 * @li "default" - A label of the multi-button entry item
 *
 * Supported elm_object_item common APIs.
 * @li @ref elm_object_item_del
 * @li @ref elm_object_item_part_text_set
 * @li @ref elm_object_item_part_text_get
 */


/**
 * @addtogroup Multibuttonentry
 * @{
 */

#include "elc_multibuttonentry_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elc_multibuttonentry_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elc_multibuttonentry_legacy.h"
#endif
/**
 * @}
 */
