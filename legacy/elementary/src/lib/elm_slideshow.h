/**
 * @defgroup Slideshow Slideshow
 * @ingroup Elementary
 *
 * @image html slideshow_inheritance_tree.png
 * @image latex slideshow_inheritance_tree.eps
 *
 * @image html img/widget/slideshow/preview-00.png
 * @image latex img/widget/slideshow/preview-00.eps
 *
 * This widget, as the name indicates, is a pre-made image
 * slideshow panel, with API functions acting on (child) image
 * items presentation. Between those actions, are:
 * - advance to next/previous image
 * - select the style of image transition animation
 * - set the exhibition time for each image
 * - start/stop the slideshow
 *
 * The transition animations are defined in the widget's theme,
 * consequently new animations can be added without having to
 * update the widget's code.
 *
 * @section Slideshow_Items Slideshow items
 *
 * For slideshow items, just like for @ref Genlist "genlist" ones,
 * the user defines a @b classes, specifying functions that will be
 * called on the item's creation and deletion times.
 *
 * The #Elm_Slideshow_Item_Class structure contains the following
 * members:
 *
 * - @c func.get - When an item is displayed, this function is
 *   called, and it's where one should create the item object, de
 *   facto. For example, the object can be a pure Evas image object
 *   or an Elementary @ref Photocam "photocam" widget.
 *   See #SlideshowItemGetFunc.
 * - @c func.del - When an item is no more displayed, this function
 *   is called, where the user must delete any data associated to
 *   the item. See #SlideshowItemDelFunc.
 *
 * @section Slideshow_Caching Slideshow caching
 *
 * The slideshow provides facilities to have items adjacent to the
 * one being displayed <b>already "realized"</b> (i.e. loaded) for
 * you, so that the system does not have to decode image data
 * anymore at the time it has to actually switch images on its
 * viewport. The user is able to set the numbers of items to be
 * cached @b before and @b after the current item, in the widget's
 * item list.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for slideshow objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * - @c "changed" - when the slideshow switches its view to a new
 *   item. event_info parameter in callback contains the current visible item
 * - @c "transition,end" - when a slide transition ends. event_info parameter
 *   in callback contains the current visible item
 * - @c "focused" - When the slideshow has received focus. (since 1.8)
 * - @c "unfocused" - When the slideshow has lost focus. (since 1.8)
 * - @c "language,changed" - the program's language changed (since 1.9)
 *
 * Supported @c elm_object_item common APIs.
 * @li @ref elm_object_item_del
 *
 * List of examples for the slideshow widget:
 * @li @ref slideshow_example
 */

/**
 * @addtogroup Slideshow
 * @{
 */

#include "elm_slideshow_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_slideshow_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_slideshow_legacy.h"
#endif
/**
 * @}
 */
