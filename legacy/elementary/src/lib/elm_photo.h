/**
 * @defgroup Photo Photo
 *
 * For displaying the photo of a person (contact). Simple, yet
 * with a very specific purpose.
 *
 * Signals that you can add callbacks for are:
 *
 * "clicked" - This is called when a user has clicked the photo
 * "drag,start" - Someone started dragging the image out of the object
 * "drag,end" - Dragged item was dropped (somewhere)
 *
 * @{
 */

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
 * Set the file that will be used as photo
 *
 * @param obj The photo object
 * @param file The path to file that will be used as photo
 *
 * @return (1 = success, 0 = error)
 *
 * @ingroup Photo
 */
EAPI Eina_Bool elm_photo_file_set(Evas_Object *obj, const char *file);

/**
 * Set the file that will be used as thumbnail in the photo.
 *
 * @param obj The photo object.
 * @param file The path to file that will be used as thumb.
 * @param group The key used in case of an EET file.
 *
 * @ingroup Photo
 */
EAPI void      elm_photo_thumb_set(const Evas_Object *obj, const char *file, const char *group);

/**
 * Set the size that will be used on the photo
 *
 * @param obj The photo object
 * @param size The size that the photo will be
 *
 * @ingroup Photo
 */
EAPI void      elm_photo_size_set(Evas_Object *obj, int size);

/**
 * Set if the photo should be completely visible or not.
 *
 * @param obj The photo object
 * @param fill if true the photo will be completely visible
 *
 * @ingroup Photo
 */
EAPI void      elm_photo_fill_inside_set(Evas_Object *obj, Eina_Bool fill);

/**
 * Set editability of the photo.
 *
 * An editable photo can be dragged to or from, and can be cut or
 * pasted too.  Note that pasting an image or dropping an item on
 * the image will delete the existing content.
 *
 * @param obj The photo object.
 * @param set To set of clear editablity.
 */
EAPI void      elm_photo_editable_set(Evas_Object *obj, Eina_Bool set);

/**
 * @}
 */
