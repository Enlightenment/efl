/**
 * @defgroup Inwin Inwin
 * @ingroup Win
 *
 * @image html inwin_inheritance_tree.png
 * @image latex inwin_inheritance_tree.eps
 *
 * @image html img/widget/inwin/preview-00.png
 * @image latex img/widget/inwin/preview-00.eps
 * @image html img/widget/inwin/preview-01.png
 * @image latex img/widget/inwin/preview-01.eps
 * @image html img/widget/inwin/preview-02.png
 * @image latex img/widget/inwin/preview-02.eps
 *
 * An inwin is a window inside a window that is useful for a quick popup.
 * It does not hover.
 *
 * It works by creating an object that will occupy the entire window, so it
 * must be created using an @ref Win "elm_win" as parent only. The inwin
 * object can be hidden or restacked below every other object if it's
 * needed to show what's behind it without destroying it. If this is done,
 * the elm_win_inwin_activate() function can be used to bring it back to
 * full visibility again.
 *
 * There are three styles available in the default theme. These are:
 * @li default: The inwin is sized to take over most of the window it's
 * placed in.
 * @li minimal: The size of the inwin will be the minimum necessary to show
 * its contents.
 * @li minimal_vertical: Horizontally, the inwin takes as much space as
 * possible, but it's sized vertically the most it needs to fit its\
 * contents.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for inner windown objects. It also
 * emits the signals inherited from @ref Layout.
 *
 * Some examples of Inwin can be found in the following:
 * @li @ref inwin_example_01
 *
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include "elm_inwin_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_inwin_legacy.h"
#endif
/**
 * @}
 */
