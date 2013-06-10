/**
 * @defgroup Scroller Scroller
 * @ingroup Elementary
 *
 * @image html scroller_inheritance_tree.png
 * @image latex scroller_inheritance_tree.eps
 *
 * A scroller holds (and clips) a single object and "scrolls it
 * around". This means that it allows the user to use a scroll bar (or
 * a finger) to drag the viewable region around, moving through a much
 * larger object that is contained in the scroller. The scroller will
 * always have a small minimum size by default as it won't be limited
 * by the contents of the scroller.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for scroller objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @li @c "edge,left" - the left edge of the content has been reached
 * @li @c "edge,right" - the right edge of the content has been reached
 * @li @c "edge,top" - the top edge of the content has been reached
 * @li @c "edge,bottom" - the bottom edge of the content has been reached
 * @li @c "scroll" - the content has been scrolled (moved)
 * @li @c "scroll,left" - the content has been scrolled (moved) leftwards
 * @li @c "scroll,right"  - the content has been scrolled (moved) rightwards
 * @li @c "scroll,up"  - the content has been scrolled (moved) upwards
 * @li @c "scroll,down" - the content has been scrolled (moved) downwards
 * @li @c "scroll,anim,start" - scrolling animation has started
 * @li @c "scroll,anim,stop" - scrolling animation has stopped
 * @li @c "scroll,drag,start" - dragging the contents around has started
 * @li @c "scroll,drag,stop" - dragging the contents around has stopped
 * @li @c "vbar,drag" - the vertical scroll bar has been dragged
 * @li @c "vbar,press" - the vertical scroll bar has been pressed
 * @li @c "vbar,unpress" - the vertical scroll bar has been unpressed
 * @li @c "hbar,drag" - the horizontal scroll bar has been dragged
 * @li @c "hbar,press" - the horizontal scroll bar has been pressed
 * @li @c "hbar,unpress" - the horizontal scroll bar has been unpressed
 * @li @c "scroll,page,changed" - the visible page has changed
 * @li @c "focused" - When the scroller has received focus. (since 1.8)
 * @li @c "unfocused" - When the scroller has lost focus. (since 1.8)
 *
 * This widget implements the @ref elm-scrollable-interface interface.
 * Its (non-deprecated) API functions, except for elm_scroller_add(),
 * which gives basic scroller objects, are meant to be a basis for all
 * other scrollable widgets (i.e. widgets implementing @ref
 * elm-scrollable-interface). So, they will work both on pristine
 * scroller widgets and on other "specialized" scrollable widgets.
 *
 * @note The @c "scroll,anim,*" and @c "scroll,drag,*" signals are
 * only emitted by user intervention.
 *
 * @note When Elementary is under its default profile and theme (meant
 * for touch interfaces), scroll bars will @b not be draggable --
 * their function is merely to indicate how much has been scrolled.
 *
 * @note When Elementary is under its desktop/standard profile and
 * theme, the thumb scroll (a.k.a. finger scroll) won't work.
 *
 * Default content parts of the scroller widget that you can use are:
 * @li @c "default" - Content of the scroller
 *
 * In @ref tutorial_scroller you'll find an example on how to use most
 * of this API.
 * @{
 */

#include <elm_scroller_common.h>
#ifdef EFL_EO_API_SUPPORT
#include <elm_scroller_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_scroller_legacy.h>
#endif

/**
 * @}
 */
