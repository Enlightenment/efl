/**
 * @defgroup Thumb Thumbnail
 * @ingroup Elementary
 *
 * @image html thumb_inheritance_tree.png
 * @image latex thumb_inheritance_tree.eps
 *
 * @image html img/widget/thumb/preview-00.png
 * @image latex img/widget/thumb/preview-00.eps
 *
 * A thumbnail object is used for displaying the thumbnail of an image
 * or video. You must have compiled Elementary with @c Ethumb_Client
 * support. Also, Ethumb's DBus service must be present and
 * auto-activated in order to have thumbnails generated. You must also
 * have a @b session bus, not a @b system one.
 *
 * Once the thumbnail object becomes visible, it will check if there
 * is a previously generated thumbnail image for the file set on
 * it. If not, it will start generating this thumbnail.
 *
 * Different configuration settings will cause different thumbnails to
 * be generated even on the same file.
 *
 * Generated thumbnails are stored under @c $HOME/.thumbnails/. Check
 * Ethumb's documentation to change this path, and to see other
 * configuration options.
 *
 * If you set formatting features such as, aspect, size, format,
 * orientation, crop, compression, or quality after the thumbnail
 * has been shown, it needs to be reloaded with elm_thumb_reload.
 *
 * This widget emits the following signals:
 * - @c "clicked" - This is called when a user has clicked the
 *                  thumbnail object without dragging it around.
 * - @c "clicked,double" - This is called when a user has double-clicked
 *                         the thumbnail object.
 * - @c "press" - This is called when a user has pressed down over the
 *                thumbnail object.
 * - @c "generate,start" - The thumbnail generation has started.
 * - @c "generate,stop" - The generation process has stopped.
 * - @c "generate,error" - The thumbnail generation failed.
 * - @c "load,error" - The thumbnail image loading failed.
 *
 * Available styles:
 * - @c "default"
 * - @c "noframe"
 *
 * An example of use of thumbnail:
 *
 * - @ref thumb_example_01
 */

/**
 * @addtogroup Thumb
 * @{
 */

#include "elm_thumb_common.h"
#ifdef EFL_EO_API_SUPPORT
#include "elm_thumb_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_thumb_legacy.h"
#endif
/**
 * @}
 */
