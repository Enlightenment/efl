/**
 * Add a new image to the parent.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_image_file_set()
 *
 * @ingroup Image
 */
EAPI Evas_Object     *elm_image_add(Evas_Object *parent);

/**
 * Set the file that will be used as the image's source.
 *
 * @param obj The image object
 * @param file The path to file that will be used as image source
 * @param group The group that the image belongs to, in case it's an
 *              EET (including Edje case) file. This can be used as a key inside
 *              evas image cache if this is a normal image file not eet file.
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @see elm_image_file_get()
 *
 * @note This function will trigger the Edje file case based on the
 * extension of the @a file string (expects @c ".edj", for this
 * case).
 *
 * @note If you use animated gif image and create multiple image objects with
 * one gif image file, you should set the @p group differently for each object.
 * Or image objects will share one evas image cache entry and you will get
 * unwanted frames.
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 *
 * Get the file that will be used as image.
 *
 * @see elm_image_file_set()
 *
 * @ingroup Image
 *
 * @param[out] file The path to file that will be used as image source
 * @param[out] group The group that the image belongs to, in case it's an
EET (including Edje case) file. This can be used as a key inside
evas image cache if this is a normal image file not eet file.
 */
EAPI void elm_image_file_get(const Eo *obj, const char **file, const char **group);

/**
 * Set the prescale size for the image
 *
 * @param obj The image object
 * @param size The prescale size. This value is used for both width and
 * height.
 *
 * This function sets a new size for pixmap representation of the given
 * image. It allows the image to be loaded already in the specified size,
 * reducing the memory usage and load time when loading a big image with load
 * size set to a smaller size.
 *
 * It's equivalent to the elm_bg_load_size_set() function for bg.
 *
 * @note this is just a hint, the real size of the pixmap may differ
 * depending on the type of image being loaded, being bigger than requested.
 *
 * @see elm_image_prescale_get()
 * @see elm_bg_load_size_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_prescale_set(Evas_Object *obj, int size);

/**
 * Get the prescale size for the image
 *
 * @param obj The image object
 * @return The prescale size
 *
 * @see elm_image_prescale_set()
 *
 * @ingroup Image
 */
EAPI int              elm_image_prescale_get(const Evas_Object *obj);

/**
 * Set the file that will be used as the image's source.
 *
 * @param obj The image object
 * @param file The handler to an Eina_File that will be used as image source
 * @param group The group that the image belongs to, in case it's an
 *              EET (including Edje case) file. This can be used as a key inside
 *              evas image cache if this is a normal image file not eet file.
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @see elm_image_file_set()
 *
 * @note This function will trigger the Edje file case based on the
 * extension of the @a file string use to create the Eina_File (expects
 * @c ".edj", for this case).
 *
 * @note If you use animated gif image and create multiple image objects with
 * one gif image file, you should set the @p group differently for each object.
 * Or image objects will share one evas image cache entry and you will get
 * unwanted frames.
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_mmap_set(Evas_Object *obj, const Eina_File *file, const char *group);

#include "elm_image.eo.legacy.h"
