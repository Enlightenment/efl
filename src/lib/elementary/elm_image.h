/**
 * @defgroup Elm_Image Image
 * @ingroup Elementary
 *
 * @image html image_inheritance_tree.png
 * @image latex image_inheritance_tree.eps
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
 * If the image source size is bigger than maximum texture size of the GPU (or also of
 * the software rendering code too), evas can't render it because of such a limitation.
 * If evas just magically always downscales on load if it's too big, then the user has a new bug:
 * "the image is blurry". Potentially any image can cause issue.
 * What if the image is too big to allocate memory for it?
 * A 30000x30000 image will need just a bit under 4GB of RAM to store it.
 * Texture size limitations are something every game developer has to deal with game engines,
 * OpenGL, D3D etc. You can get the maximum image size evas can possibly handle by
 * the calling evas_image_max_size_get() function. If the image size is bigger than this,
 * you can try using load options to pre-scale down on load to lower quality.
 * So use the elm_image_prescale_set() function to scale the image down.
 * Another option is to use the Photocam widget. Photocam solves this issue
 * by loading the prices as needed asynchronously in tiles and automatically using pre-scaling as well
 * So use Photocam if you expect to load very large images.
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
 * @li @c "load,open" - Triggered when the file has been opened, if async
 *                      open is enabled (image size is known). (since 1.19)
 * @li @c "load,ready" - Triggered when the image file is ready for display,
 *                       if preload is enabled. (since 1.19)
 * @li @c "load,error" - Triggered if an async I/O or decoding error occurred,
 *                       if async open or preload is enabled (since 1.19)
 * @li @c "load,cancel" - Triggered whenever async I/O was cancelled. (since 1.19)
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
