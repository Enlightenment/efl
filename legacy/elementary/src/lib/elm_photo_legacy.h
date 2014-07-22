/**
 * Add a new photo to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Photo
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
 * @ingroup Photo
 *
 * @param[in] file The path to file that will be used as @a obj's image.
 */
EAPI Eina_Bool elm_photo_file_set(Eo *obj, const char *file);

#include "elm_photo.eo.legacy.h"
