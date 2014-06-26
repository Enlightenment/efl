/**
 * @defgroup Panel Panel
 * @ingroup Elementary
 *
 * @image html panel_inheritance_tree.png
 * @image latex panel_inheritance_tree.eps
 *
 * @image html img/widget/panel/preview-00.png
 * @image latex img/widget/panel/preview-00.eps
 *
 * @brief A panel is an animated side-panel that contains a
 * sub-object. It can be expanded or contracted by clicking the
 * button on its edge.
 *
 * Orientations are as follows:
 * @li @c ELM_PANEL_ORIENT_TOP
 * @li @c ELM_PANEL_ORIENT_LEFT
 * @li @c ELM_PANEL_ORIENT_RIGHT
 * @li @c ELM_PANEL_ORIENT_BOTTOM
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for panel objects (@since 1.8).
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "focused" : When the panel has received focus. (since 1.8)
 * @li @c "unfocused" : When the panel has lost focus. (since 1.8)
 *
 * Default content parts of the panel widget that you can use are:
 * @li @c "default" - A content of the panel
 *
 * @ref tutorial_panel shows one way to use this widget.
 * @{
 */

#include "elm_panel_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_panel_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_panel_legacy.h"
#endif
/**
 * @}
 */
