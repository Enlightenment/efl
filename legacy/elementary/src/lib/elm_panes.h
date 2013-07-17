/**
 * @defgroup Panes Panes
 * @ingroup Elementary
 *
 * @image html panes_inheritance_tree.png
 * @image latex panes_inheritance_tree.eps
 *
 * @image html img/widget/panes/preview-00.png
 * @image latex img/widget/panes/preview-00.eps width=\textwidth
 *
 * @image html img/panes.png
 * @image latex img/panes.eps width=\textwidth
 *
 * The panes widget adds a draggable bar between two contents. When dragged
 * this bar will resize contents' size.
 *
 * Panes can be split vertically or horizontally, and contents
 * size proportion can be customized (homogeneous by default).
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for panes objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "press" - The panes has been pressed (button wasn't released yet).
 * - @c "unpress" - The panes was released after being pressed.
 * - @c "clicked" - The panes has been clicked>
 * - @c "clicked,double" - The panes has been double clicked
 *
 * Available styles for it:
 * - @c "default"
 *
 * Default content parts of the panes widget that you can use are:
 * @li "left" - A leftside content of the panes
 * @li "right" - A rightside content of the panes
 * @li "top" - A top content of the vertical panes
 * @li "bottom" - A bottom content of the vertical panes
 *
 * If panes are displayed vertically, left content will be displayed on
 * top.
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * Here is an example on its usage:
 * @li @ref panes_example
 */


/**
 * @addtogroup Panes
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_panes_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_panes_legacy.h"
#endif
/**
 * @}
 */
