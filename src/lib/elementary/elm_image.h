/**
 * @defgroup Elm_Image Image
 * @ingroup Elementary
 *
 * @image html image_inheritance_tree.png
 * @image latex image_inheritance_tree.eps
 *
 * @image html img/widget/image/preview-00.png
 * @image latex img/widget/image/preview-00.eps
 *
 * An Elementary image object is a direct realization of
 * @ref elm-image-class, and it allows one to load and display an @b image
 * file on it, be it from a disk file or from a memory
 * region. Exceptionally, one may also load an Edje group as the
 * contents of the image. In this case, though, most of the functions
 * of the image API will act as a no-op.
 *
 * One can tune various properties of the image, like:
 * - pre-scaling,
 * - smooth scaling,
 * - orientation,
 * - aspect ratio during resizes, etc.
 *
 * An image object may also be made valid source and destination for
 * drag and drop actions, through the elm_image_editable_set() call.
 *
 * Signals that you can add callbacks for are:
 *
 * @li @c "drop" - This is called when a user has dropped an image
 *                 typed object onto the object in question -- the
 *                 event info argument is the path to that image file
 * @li @c "clicked" - This is called when a user has clicked the image
 * @li @c "download,start" - This is called when the remote image file download
 *                           has started.
 * @li @c "download,progress" - This is continuously called before the remote
 *                              image file download has done. The event info
 *                              data is of type Elm_Image_Progress.
 * @li @c "download,done" - This is called when the download has completed.
 * @li @c "download,error"- This is called when the download has failed.
 *
 * An example of usage for this API follows:
 * @li @ref tutorial_image
 */


/**
 * @addtogroup Elm_Image
 * @{
 */

#ifdef EFL_EO_API_SUPPORT
#include <elm_image_eo.h>
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include <elm_image_legacy.h>
#endif

/**
 * @}
 */
