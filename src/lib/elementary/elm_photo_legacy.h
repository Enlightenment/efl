/**
 * Add a new photo to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Photo
 */
EAPI Evas_Object *elm_photo_add(Evas_Object *parent);

/**
 *
 * Set the file that will be used as the photo widget's image.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * @note Use @c NULL on @a file to set the photo widget back to it's
 * initial state, which indicates "no photo".
 *
 * @ingroup Elm_Photo
 *
 * @param[in] file The path to file that will be used as @a obj's image.
 */
EAPI Eina_Bool elm_photo_file_set(Eo *obj, const char *file);

/**
 * Set editability of the photo.
 *
 * An editable photo can be dragged to or from, and can be cut or pasted too.
 * Note that pasting an image or dropping an item on the image will delete the
 * existing content.
 *
 * @param[in] set To set of clear editability.
 *
 * @ingroup Elm_Photo
 */
EAPI void elm_photo_editable_set(Evas_Object *obj, Eina_Bool set);

/**
 * Get editability of the photo.
 *
 * @return To set of clear editability.
 *
 * @ingroup Elm_Photo
 */
EAPI Eina_Bool elm_photo_editable_get(const Evas_Object *obj);

/**
 * @brief Set if the photo should be completely visible or not.
 *
 * @param[in] fill Photo visibility.
 *
 * @ingroup Elm_Photo
 */
EAPI void elm_photo_fill_inside_set(Evas_Object *obj, Eina_Bool fill);

/**
 * @brief Get if the photo should be completely visible or not.
 *
 * @return Photo visibility.
 *
 * @ingroup Elm_Photo
 */
EAPI Eina_Bool elm_photo_fill_inside_get(const Evas_Object *obj);

/**
 * @brief Set whether the original aspect ratio of the photo should be kept on
 * resize.
 *
 * The original aspect ratio (width / height) of the photo is usually distorted
 * to match the object's size. Enabling this option will fix this original
 * aspect, and the way that the photo is fit into the object's area.
 *
 * See also @ref elm_photo_aspect_fixed_get.
 *
 * @param[in] fixed @c true if the photo should fix the aspect, @c false
 * otherwise.
 *
 * @ingroup Elm_Photo
 */
EAPI void elm_photo_aspect_fixed_set(Evas_Object *obj, Eina_Bool fixed);

/**
 * @brief Get if the object keeps the original aspect ratio.
 *
 * @return @c true if the photo should fix the aspect, @c false otherwise.
 *
 * @ingroup Elm_Photo
 */
EAPI Eina_Bool elm_photo_aspect_fixed_get(const Evas_Object *obj);

/**
 * @brief Set the size that will be used on the photo
 *
 * @param[in] size The size of the photo
 *
 * @ingroup Elm_Photo
 */
EAPI void elm_photo_size_set(Evas_Object *obj, int size);

/**
 * @brief Get the size that will be used on the photo
 *
 * @return The size of the photo
 *
 * @ingroup Elm_Photo
 */
EAPI int elm_photo_size_get(const Evas_Object *obj);

/**
 * @brief Set the file that will be used as thumbnail in the photo.
 *
 * @param[in] group The key used in case of an EET file.
 *
 * @ingroup Elm_Photo
 */
EAPI void elm_photo_thumb_set(Evas_Object *obj, const char *file, const char *group);

#include "elm_photo_eo.legacy.h"
