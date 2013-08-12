/**
 * @defgroup Photocam Photocam
 * @ingroup Elementary
 *
 * @image html photocam_inheritance_tree.png
 * @image latex photocam_inheritance_tree.eps
 *
 * @image html img/widget/photocam/preview-00.png
 * @image latex img/widget/photocam/preview-00.eps
 *
 * Photocam is a widget meant specifically for displaying
 * high-resolution digital camera photos, giving speedy feedback (fast
 * load), zooming and panning as well as fitting logic, all with low
 * memory footprint. It is entirely focused on @b jpeg images, and
 * takes advantage of properties of the jpeg format (via Evas loader
 * features in the jpeg loader).
 *
 * Signals that you can add callbacks for are:
 * @li @c "clicked" - This is called when a user has clicked the photo
 *        without dragging around.
 * @li @c "press" - This is called when a user has pressed down on the
 *        photo.
 * @li @c "longpressed" - This is called when a user has pressed down
 *        on the photo for a long time without dragging around.
 * @li @c "clicked,double" - This is called when a user has
 *        double-clicked the photo.
 * @li @c "load" - Photo load begins.
 * @li @c "loaded" - This is called when the image file load is
 *        complete for the first view (low resolution blurry version).
 * @li @c "load,detail" - Photo detailed data load begins.
 * @li @c "loaded,detail" - This is called when the image file load is
 *        complete for the detailed image data (full resolution
 *        needed).
 * @li @c "zoom,start" - Zoom animation started.
 * @li @c "zoom,stop" - Zoom animation stopped.
 * @li @c "zoom,change" - Zoom changed when using an auto zoom mode.
 * @li @c "scroll" - the content has been scrolled (moved)
 * @li @c "scroll,anim,start" - scrolling animation has started
 * @li @c "scroll,anim,stop" - scrolling animation has stopped
 * @li @c "scroll,drag,start" - dragging the contents around has started
 * @li @c "scroll,drag,stop" - dragging the contents around has stopped
 * @li @c "focused" - When the photocam has received focus. (since 1.8)
 * @li @c "unfocused" - When the photocam has lost focus. (since 1.8)
 *
 * This widget implements the @b @ref elm-scrollable-interface
 * interface, so that all (non-deprecated) functions for the base @ref
 * Scroller widget also work for photocam objects.
 *
 * Some calls on the photocam's API are marked as @b deprecated, as
 * they just wrap the scrollable widgets counterpart functions. Use
 * the ones we point you to, for each case of deprecation here,
 * instead -- eventually the deprecated ones will be discarded (next
 * major release).
 *
 * @ref tutorial_photocam shows the API in action.
 * @{
 */

/**
 * @brief Types of zoom available.
 */
typedef enum
{
   ELM_PHOTOCAM_ZOOM_MODE_MANUAL = 0, /**< Zoom controlled normally by elm_photocam_zoom_set */
   ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT, /**< Zoom until photo fits in photocam */
   ELM_PHOTOCAM_ZOOM_MODE_AUTO_FILL, /**< Zoom until photo fills photocam */
   ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT_IN, /**< Zoom in until photo fits in photocam */
   ELM_PHOTOCAM_ZOOM_MODE_LAST
} Elm_Photocam_Zoom_Mode;

/**
 * Structure associated with smart callback 'download,progress'.
 * @since 1.8
 */
typedef struct _Elm_Photocam_Progress Elm_Photocam_Progress;

struct _Elm_Photocam_Progress
{
   double now;
   double total;
};


/**
 * Structre associated with smart callback 'download,error'
 * @since 1.8
 */
typedef struct _Elm_Photocam_Error Elm_Photocam_Error;
struct _Elm_Photocam_Error
{
   int status;

   Eina_Bool open_error;
};

#ifdef EFL_EO_API_SUPPORT
#include "elm_photocam_eo.h"
#endif
#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_photocam_legacy.h"
#endif
/**
 * @}
 */
