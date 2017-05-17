/**
 * @brief Add a new Photocam object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Photocam
 */
EAPI Evas_Object           *elm_photocam_add(Evas_Object *parent);

/**
 * @brief Bring in the viewed portion of the image
 *
 * @param obj The photocam object
 * @param x X-coordinate of region in image original pixels
 * @param y Y-coordinate of region in image original pixels
 * @param w Width of region in image original pixels
 * @param h Height of region in image original pixels
 *
 * This shows the region of the image using animation.
 *
 * @ingroup Elm_Photocam
 */
EAPI void                   elm_photocam_image_region_bring_in(Evas_Object *obj, int x, int y, int w, int h);

/**
 *
 * @brief Set the photo file to be shown
 *
 * @return The return error (see EVAS_LOAD_ERROR_NONE, EVAS_LOAD_ERROR_GENERIC etc.)
 *
 * This sets (and shows) the specified file (with a relative or absolute
 * path) and will return a load error (same error that
 * evas_object_image_load_error_get() will return). The image will change and
 * adjust its size at this point and begin a background load process for this
 * photo that at some time in the future will be displayed at the full
 * quality needed.
 *
 * @ingroup Elm_Photocam
 *
 * @param[in] file The photo file
 */
EAPI Evas_Load_Error elm_photocam_file_set(Evas_Object *obj, const char *file);

/**
 *
 * @brief Returns the path of the current image file
 *
 * @return Returns the path
 *
 * @see elm_photocam_file_set()
 *
 * @ingroup Elm_Photocam
 *
 */
EAPI const char *elm_photocam_file_get(const Evas_Object *obj);

/**
 * @brief Set the photocam image orientation.
 *
 * This function allows to rotate or flip the photocam image.
 *
 * @param[in] orient The photocam image orientation @ref Evas_Image_Orient.
 * Default is #EVAS_IMAGE_ORIENT_NONE.
 *
 * @since 1.14
 *
 * @ingroup Elm_Photocam
 */
EAPI void elm_photocam_image_orient_set(Evas_Object *obj, Evas_Image_Orient orient);

/**
 * @brief Get the photocam image orientation.
 *
 * @return The photocam image orientation @ref Evas_Image_Orient. Default is
 * #EVAS_IMAGE_ORIENT_NONE.
 *
 * @since 1.14
 *
 * @ingroup Elm_Photocam
 */
EAPI Evas_Image_Orient elm_photocam_image_orient_get(const Evas_Object *obj);

/**
 * @brief Get the internal low-res image used for photocam
 *
 * This gets the internal image object inside photocam. Do not modify it. It is
 * for inspection only, and hooking callbacks to. Nothing else. It may be
 * deleted at any time as well.
 *
 * @return The internal image object handle or @c null
 *
 * @ingroup Elm_Photocam
 */
EAPI Evas_Object*      elm_photocam_internal_image_get(const Evas_Object *obj);

/**
 * @brief Set the viewed region of the image
 *
 * This shows the region of the image without using animation.
 *
 * @param[in] x X-coordinate of region in image original pixels
 * @param[in] y Y-coordinate of region in image original pixels
 * @param[in] w Width of region in image original pixels
 * @param[in] h Height of region in image original pixels
 *
 * @ingroup Elm_Photocam
 */
EAPI void              elm_photocam_image_region_show(Evas_Object *obj, int x, int y, int w, int h);

/**
 * @brief Get the current image pixel width and height
 *
 * This gets the current photo pixel width and height (for the original). The
 * size will be returned in the integers @c w and @c h that are pointed to.
 *
 * @param[out] w A pointer to the width return
 * @param[out] h A pointer to the height return
 *
 * @ingroup Elm_Photocam
 */
EAPI void              elm_photocam_image_size_get(const Evas_Object *obj, int *w, int *h);

#include "elm_photocam.eo.legacy.h"
