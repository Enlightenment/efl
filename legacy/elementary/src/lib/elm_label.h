/**
 * @defgroup Label Label
 * @ingroup Elementary
 *
 * @image html label_inheritance_tree.png
 * @image latex label_inheritance_tree.eps
 *
 * @image html img/widget/label/preview-00.png
 * @image latex img/widget/label/preview-00.eps
 *
 * @brief Widget to display text, with simple html-like markup.
 *
 * The Label widget @b doesn't allow text to overflow its boundaries, if the
 * text doesn't fit the geometry of the label it will be ellipsized or be
 * cut. Elementary provides several styles for this widget:
 * @li default - No animation
 * @li marker - Centers the text in the label and makes it bold by default
 * @li slide_long - The entire text appears from the right of the screen and
 * slides until it disappears in the left of the screen(reappearing on the
 * right again).
 * @li slide_short - The text appears in the left of the label and slides to
 * the right to show the overflow. When all of the text has been shown the
 * position is reset.
 * @li slide_bounce - The text appears in the left of the label and slides to
 * the right to show the overflow. When all of the text has been shown the
 * animation reverses, moving the text to the left.
 *
 * Custom themes can of course invent new markup tags and style them any way
 * they like.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for label objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "language,changed": The program's language changed.
 * @li @c "slide,end": The slide is end.
 *
 * See @ref tutorial_label for a demonstration of how to use a label widget.
 * @{
 */
#include "elm_label_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_label_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_label_legacy.h"
#endif
/**
 * @}
 */
