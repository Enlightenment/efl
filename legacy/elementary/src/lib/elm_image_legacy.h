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

/**
 * @brief Control the smooth effect for an image.
 *
 * Set the scaling algorithm to be used when scaling the image. Smooth scaling
 * provides a better resulting image, but is slower.
 *
 * The smooth scaling should be disabled when making animations that change the
 * image size, since it will be faster. Animations that don't require resizing
 * of the image can keep the smooth scaling enabled (even if the image is
 * already scaled, since the scaled image will be cached).
 *
 * @param[in] smooth @c true if smooth scaling should be used, @c false
 * otherwise. Default is @c true.
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_smooth_set(Evas_Object *obj, Eina_Bool smooth);

/**
 * @brief Get the smooth effect for an image.
 *
 * Get the scaling algorithm to be used when scaling the image. Smooth scaling
 * provides a better resulting image, but is slower.
 *
 * The smooth scaling should be disabled when making animations that change the
 * image size, since it will be faster. Animations that don't require resizing
 * of the image can keep the smooth scaling enabled (even if the image is
 * already scaled, since the scaled image will be cached).
 *
 * @return @c true if smooth scaling should be used, @c false otherwise.
 * Default is @c true.
 *
 * @ingroup Elm_Image
 */
EAPI Eina_Bool elm_image_smooth_get(const Evas_Object *obj);


/**
 * Start or stop an image object's animation.
 *
 * To actually start playing any image object's animation, if it
 * supports it, one must do something like:
 *
 * @code
 * if (elm_image_animated_available_get(img))
 * {
 * elm_image_animated_set(img, EINA_TRUE);
 * elm_image_animated_play_set(img, EINA_TRUE);
 * }
 * @endcode
 *
 * elm_image_animated_set() will enable animation on the image, <b>but
 * not start it yet</b>. This is the function one uses to start and
 * stop animations on image objects.
 *
 * @see elm_image_animated_available_get()
 * @see elm_image_animated_set()
 * @see elm_image_animated_play_get()
 *
 * @ingroup Image
 * @since 1.7
 *
 * @param[in] play @c EINA_TRUE to start the animation, @c EINA_FALSE
otherwise. Default is @c EINA_FALSE.
 */
EAPI void             elm_image_animated_play_set(Evas_Object *obj, Eina_Bool play);

/**
 * Get whether an image object is under animation or not.
 *
 * @return @c EINA_TRUE, if the image is being animated, @c EINA_FALSE
 * otherwise.
 *
 * @see elm_image_animated_play_get()
 *
 * @ingroup Image
 * @since 1.7
 */
EAPI Eina_Bool        elm_image_animated_play_get(const Evas_Object *obj);

/**
 *
 * Set whether an image object (which supports animation) is to
 * animate itself or not.
 *
 * An image object, even if it supports animation, will be displayed
 * by default without animation. Call this function with @a animated
 * set to @c EINA_TRUE to enable its animation. To start or stop the
 * animation, actually, use elm_image_animated_play_set().
 *
 * @see elm_image_animated_get()
 * @see elm_image_animated_available_get()
 * @see elm_image_animated_play_set()
 *
 * @ingroup Image
 * @since 1.7
 *
 * @param[in] anim @c EINA_TRUE if the object is to animate itself,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 */
EAPI void             elm_image_animated_set(Evas_Object *obj, Eina_Bool anim);

/**
 *
 * Get whether an image object has animation enabled or not.
 *
 * @return @c EINA_TRUE if the image has animation enabled,
 * @c EINA_FALSE otherwise.
 *
 * @see elm_image_animated_set()
 *
 * @ingroup Image
 * @since 1.7
 *
 */
EAPI Eina_Bool        elm_image_animated_get(const Evas_Object *obj);

/**
 *
 * Get whether an image object supports animation or not.
 *
 * @return @c EINA_TRUE if the image supports animation,
 * @c EINA_FALSE otherwise.
 *
 * This function returns if this Elementary image object's internal
 * image can be animated. Currently Evas only supports GIF
 * animation. If the return value is @b EINA_FALSE, other
 * @c elm_image_animated_xxx API calls won't work.
 *
 * @see elm_image_animated_set()
 *
 * @ingroup Image
 * @since 1.7
 *
 */
EAPI Eina_Bool        elm_image_animated_available_get(const Evas_Object *obj);

#include "elm_image.eo.legacy.h"
