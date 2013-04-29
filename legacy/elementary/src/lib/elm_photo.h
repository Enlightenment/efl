/**
 * @defgroup Photo Photo
 * @ingroup Elementary
 *
 * @image html photo_inheritance_tree.png
 * @image latex photo_inheritance_tree.eps
 *
 * The Elementary photo widget is intended for displaying a photo, for
 * ex., a person's image (contact). Simple, yet with a very specific
 * purpose. It has a decorative frame around the inner image itself,
 * on the default theme. If and while no photo is set on it, it
 * displays a person icon, indicating it's a photo placeholder.
 *
 * This widget relies on an internal @ref Icon, so that the APIs of
 * these two widgets are similar (drag and drop is also possible here,
 * for example).
 *
 * Signals that you can add callbacks for are:
 * - @c "clicked" - This is called when a user has clicked the photo
 * - @c "drag,start" - One has started dragging the inner image out of the
 *                     photo's frame
 * - @c "drag,end" - One has dropped the dragged image somewhere
 *
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include <elm_photo_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_photo_legacy.h>
#endif

/**
 * @}
 */
