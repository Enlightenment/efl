/**
 * @defgroup Button Button
 * @ingroup Elementary
 *
 * @image html button_inheritance_tree.png
 * @image latex button_inheritance_tree.eps
 *
 * @image html img/widget/button/preview-00.png
 * @image latex img/widget/button/preview-00.eps
 * @image html img/widget/button/preview-01.png
 * @image latex img/widget/button/preview-01.eps
 * @image html img/widget/button/preview-02.png
 * @image latex img/widget/button/preview-02.eps
 *
 * This is a push-button. Press it and run some function. It can contain
 * a simple label and icon object and it also has an autorepeat feature.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for button objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li "clicked": the user clicked the button (press/release).
 * @li "repeated": the user pressed the button without releasing it.
 * @li "pressed": button was pressed.
 * @li "unpressed": button was released after being pressed.
 * @li @c "focused" : When the button has received focus. (since 1.8)
 * @li @c "unfocused" : When the button has lost focus. (since 1.8)
 * In all cases, the @c event parameter of the callback will be
 * @c NULL.
 *
 * Also, defined in the default theme, the button has the following styles
 * available:
 * @li default: a normal button.
 * @li anchor: Like default, but the button fades away when the mouse is not
 * over it, leaving only the text or icon.
 * @li hoversel_vertical: Internally used by @ref Hoversel to give a
 * continuous look across its options.
 * @li hoversel_vertical_entry: Another internal for @ref Hoversel.
 * @li naviframe: Internally used by @ref Naviframe for its back button.
 * @li colorselector: Internally used by @ref Colorselector
 * for its left and right buttons.
 *
 * Default content parts of the button widget that you can use for are:
 * @li "icon" - An icon of the button
 *
 * Default text parts of the button widget that you can use for are:
 * @li "default" - Label of the button
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 *
 * Here is some sample code using it:
 * @li @ref button_example_00
 * @li @ref button_example_01
 */

/**
 * @addtogroup Button
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_button_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_button_legacy.h"
#endif
/**
 * @}
 */
