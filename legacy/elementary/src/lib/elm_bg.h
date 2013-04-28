/**
 * @defgroup Bg Background
 * @ingroup Elementary
 *
 * @image html bg_inheritance_tree.png
 * @image latex bg_inheritance_tree.eps
 *
 * @image html img/widget/bg/preview-00.png
 * @image latex img/widget/bg/preview-00.eps
 *
 * @brief Background object, used for setting a solid color, image or
 * Edje group as a background to a window or any container object.
 *
 * The bg (background) widget is used for setting (solid) background
 * decorations to a window (unless it has transparency enabled) or to
 * any container object. It works just like an image, but has some
 * properties useful to a background, like setting it to tiled,
 * centered, scaled or stretched.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for background objects.
 *
 * Default content parts of the bg widget that you can use for are:
 * @li @c "overlay" - overlay of the bg
 *
 * Here is some sample code using it:
 * @li @ref bg_01_example_page
 * @li @ref bg_02_example_page
 * @li @ref bg_03_example_page
 */

#include "elm_bg_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_bg_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_bg_legacy.h"
#endif
/**
 * @}
 */
