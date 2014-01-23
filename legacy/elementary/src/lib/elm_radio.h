/**
 * @defgroup Radio Radio
 * @ingroup Elementary
 *
 * @image html radio_inheritance_tree.png
 * @image latex radio_inheritance_tree.eps
 *
 * @image html img/widget/radio/preview-00.png
 * @image latex img/widget/radio/preview-00.eps
 *
 * @brief Radio is a widget that allows for 1 or more options to be displayed
 * and have the user choose only 1 of them.
 *
 * A radio object contains an indicator, an optional Label and an optional
 * icon object. While it's possible to have a group of only one radio they,
 * are normally used in groups of 2 or more.
 *
 * elm_radio objects are grouped in a slightly different, compared to other
 * UI toolkits. There is no seperate group name/id to remember or manage.
 * The members represent the group, there are the group. To make a group,
 * use elm_radio_group_add() and pass existing radio object and the new radio
 * object.
 *
 * The radio object(s) will select from one of a set
 * of integer values, so any value they are configuring needs to be mapped to
 * a set of integers. To configure what value that radio object represents,
 * use  elm_radio_state_value_set() to set the integer it represents. To set
 * the value the whole group(which one is currently selected) is to indicate
 * use elm_radio_value_set() on any group member, and to get the groups value
 * use elm_radio_value_get(). For convenience the radio objects are also able
 * to directly set an integer(int) to the value that is selected. To specify
 * the pointer to this integer to modify, use elm_radio_value_pointer_set().
 * The radio objects will modify this directly. That implies the pointer must
 * point to valid memory for as long as the radio objects exist.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for radio objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li changed - This is called whenever the user changes the state of one of
 * the radio objects within the group of radio objects that work together.
 * @li @c "focused" - When the radio has received focus. (since 1.8)
 * @li @c "unfocused" - When the radio has lost focus. (since 1.8)
 * @li @c "language,changed" - the program's language changed (since 1.9)
 *
 * Default text parts of the radio widget that you can use for are:
 * @li "default" - Label of the radio
 *
 * Default content parts of the radio widget that you can use for are:
 * @li "icon" - An icon of the radio
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
 * @ref tutorial_radio show most of this API in action.
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_radio_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_radio_legacy.h"
#endif
/**
 * @}
 */
