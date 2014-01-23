/**
 * @addtogroup Actionslider Actionslider
 * @ingroup Elementary
 *
 * @image html actionslider_inheritance_tree.png
 * @image latex actionslider_inheritance_tree.eps
 *
 * @image html img/widget/actionslider/preview-00.png
 * @image latex img/widget/actionslider/preview-00.eps
 *
 * An actionslider is a switcher for 2 or 3 labels with customizable magnet
 * properties. The user drags and releases the indicator, to choose a label.
 *
 * Labels occupy the following positions.
 * a. Left
 * b. Right
 * c. Center
 *
 * Positions can be enabled or disabled.
 *
 * Magnets can be set on the above positions.
 *
 * When the indicator is released, it will move to its nearest
 * "enabled and magnetized" position.
 *
 * @note By default all positions are set as enabled.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for actionslider objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "selected" - when user selects an enabled position (the
 *              label is passed as event info).
 * @li @c "pos_changed" - when the indicator reaches any of the
 *                 positions("left", "right" or "center").
 * @li @c "language,changed" - the program's language changed (since 1.9)
 *
 * Default text parts of the actionslider widget that you can use for are:
 * @li "indicator" - An indicator label of the actionslider
 * @li "left" - A left label of the actionslider
 * @li "right" - A right label of the actionslider
 * @li "center" - A center label of the actionslider
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_text_set
 * @li @ref elm_object_part_text_get
 *
 * See an example of actionslider usage @ref actionslider_example_page "here"
 * @{
 */

#include "elm_actionslider_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_actionslider_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_actionslider_legacy.h"
#endif
/**
 * @}
 */
