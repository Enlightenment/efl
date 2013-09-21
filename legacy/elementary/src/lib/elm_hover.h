/**
 * @defgroup Hover Hover
 * @ingroup Elementary
 *
 * @image html hover_inheritance_tree.png
 * @image latex hover_inheritance_tree.eps
 *
 * @image html img/widget/hover/preview-00.png
 * @image latex img/widget/hover/preview-00.eps
 *
 * A Hover object will hover over its @p parent object at the @p target
 * location. Anything in the background will be given a darker coloring to
 * indicate that the hover object is on top (at the default theme). When the
 * hover is clicked it is dismissed(hidden), if the contents of the hover are
 * clicked that @b doesn't cause the hover to be dismissed.
 *
 * A Hover object has two parents. One parent that owns it during creation
 * and the other parent being the one over which the hover object spans.
 *
 *
 * @note The hover object will take up the entire space of @p target
 * object.
 *
 * Elementary has the following styles for the hover widget:
 * @li default
 * @li popout
 * @li menu
 * @li hoversel_vertical
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for hover objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "clicked" - the user clicked the empty space in the hover to dismiss
 * @li @c "dismissed" - the user clicked the empty space in the hover to dismiss (since 1.8)
 * @li @c "smart,changed" - a content object placed under the "smart"
 *                   policy was replaced to a new slot direction.
 * @li @c "focused" - When the hover has received focus. (since 1.8)
 * @li @c "unfocused" - When the hover has lost focus. (since 1.8)
 *
 * Default content parts of the hover widget that you can use for are:
 * @li @c "left"
 * @li @c "top-left"
 * @li @c "top"
 * @li @c "top-right"
 * @li @c "right"
 * @li @c "bottom-right"
 * @li @c "bottom"
 * @li @c "bottom-left"
 * @li @c "middle"
 * @li @c "smart"
 *
 * @note These content parts indicates the direction that the content will be
 * displayed
 *
 * All directions may have contents at the same time, except for
 * "smart". This is a special placement hint and its use case
 * depends of the calculations coming from
 * elm_hover_best_content_location_get(). Its use is for cases when
 * one desires only one hover content, but with a dynamic special
 * placement within the hover area. The content's geometry, whenever
 * it changes, will be used to decide on a best location, not
 * extrapolating the hover's parent object view to show it in (still
 * being the hover's target determinant of its medium part -- move and
 * resize it to simulate finger sizes, for example). If one of the
 * directions other than "smart" are used, a previously content set
 * using it will be deleted, and vice-versa.
 *
 * Supported elm_object common APIs.
 * @li @ref elm_object_signal_emit
 * @li @ref elm_object_signal_callback_add
 * @li @ref elm_object_signal_callback_del
 * @li @ref elm_object_part_content_set
 * @li @ref elm_object_part_content_get
 * @li @ref elm_object_part_content_unset
 *
 * See @ref tutorial_hover for more information.
 *
 * @{
 */

#include "elm_hover_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_hover_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_hover_legacy.h"
#endif
/**
 * @}
 */
