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
#include "elm_photo.eo.legacy.h"
