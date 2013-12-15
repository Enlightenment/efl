/**
 * @defgroup Conformant Conformant
 * @ingroup Elementary
 *
 * @image html conformant_inheritance_tree.png
 * @image latex conformant_inheritance_tree.eps
 *
 * @image html img/widget/conformant/preview-00.png
 * @image latex img/widget/conformant/preview-00.eps width=\textwidth
 *
 * @image html img/conformant.png
 * @image latex img/conformant.eps width=\textwidth
 *
 * The aim is to provide a widget that can be used in elementary apps to
 * account for space taken up by the indicator, virtual keypad & softkey
 * windows when running the illume2 module of E17.
 *
 * So conformant content will be sized and positioned considering the
 * space required for such stuff, and when they popup, as a keyboard
 * shows when an entry is selected, conformant content won't change.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for conformant objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li "virtualkeypad,state,on": if virtualkeypad state is switched to "on".
 * (@since 1.8)
 * @li "virtualkeypad,state,off": if virtualkeypad state is switched to "off".
 * (@since 1.8)
 * @li "clipboard,state,on": if clipboard state is switched to "on".
 * (@since 1.8)
 * @li "clipboard,state,off": if clipboard state is switched to "off".
 * (@since 1.8)
 * In all cases, the @c event parameter of the callback will be
 * @c NULL.
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default content parts of the conformant widget that you can use for are:
 * @li "default" - A content of the conformant
 *
 * See how to use this widget in this example:
 * @ref conformant_example
 */

/**
 * @addtogroup Conformant
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_conform_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_conform_legacy.h"
#endif
/**
 * @}
 */
