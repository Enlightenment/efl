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
 * Set a location in memory to be used as an image object's source
 * bitmap.
 *
 * @param obj The image object
 * @param img The binary data that will be used as image source
 * @param size The size of binary data blob @p img
 * @param format (Optional) expected format of @p img bytes
 * @param key Optional indexing key of @p img to be passed to the
 *            image loader (eg. if @p img is a memory-mapped EET file)
 *
 * This function is handy when the contents of an image file are
 * mapped in memory, for example.
 *
 * The @p format string should be something like @c "png", @c "jpg",
 * @c "tga", @c "tiff", @c "bmp" etc, when provided (@c NULL, on the
 * contrary). This improves the loader performance as it tries the
 * "correct" loader first, before trying a range of other possible
 * loaders until one succeeds.
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @since 1.7
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_memfile_set(Evas_Object *obj, const void *img, size_t size, const char *format, const char *key);

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
 * @param obj The image object
 * @param file The path to file
 * @param group The group that the image belongs in edje file
 *
 * @see elm_image_file_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_file_get(const Evas_Object *obj, const char **file, const char **group);

/**
 * Set the smooth effect for an image.
 *
 * @param obj The image object
 * @param smooth @c EINA_TRUE if smooth scaling should be used, @c EINA_FALSE
 * otherwise. Default is @c EINA_TRUE.
 *
 * Set the scaling algorithm to be used when scaling the image. Smooth
 * scaling provides a better resulting image, but is slower.
 *
 * The smooth scaling should be disabled when making animations that change
 * the image size, since it will be faster. Animations that don't require
 * resizing of the image can keep the smooth scaling enabled (even if the
 * image is already scaled, since the scaled image will be cached).
 *
 * @see elm_image_smooth_get()
 *
 * @ingroup Image
 */
EAPI void             elm_image_smooth_set(Evas_Object *obj, Eina_Bool smooth);

/**
 * Get the smooth effect for an image.
 *
 * @param obj The image object
 * @return @c EINA_TRUE if smooth scaling is enabled, @c EINA_FALSE otherwise.
 *
 * @see elm_image_smooth_set()
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_smooth_get(const Evas_Object *obj);

/**
 * Get the current size of the image.
 *
 * @param obj The image object.
 * @param w Pointer to store width, or NULL.
 * @param h Pointer to store height, or NULL.
 *
 * This is the real size of the image, not the size of the object.
 *
 * @ingroup Image
 */
EAPI void             elm_image_object_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * Disable scaling of this object.
 *
 * @param obj The image object.
 * @param no_scale @c EINA_TRUE if the object is not scalable, @c EINA_FALSE
 * otherwise. Default is @c EINA_FALSE.
 *
 * This function disables scaling of the elm_image widget through the
 * function elm_object_scale_set(). However, this does not affect the widget
 * size/resize in any way. For that effect, take a look at
 * elm_image_resizable_set().
 *
 * @see elm_image_no_scale_get()
 * @see elm_image_resizable_set()
 * @see elm_object_scale_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_no_scale_set(Evas_Object *obj, Eina_Bool no_scale);

/**
 * Get whether scaling is disabled on the object.
 *
 * @param obj The image object
 * @return @c EINA_TRUE if scaling is disabled, @c EINA_FALSE otherwise
 *
 * @see elm_image_no_scale_set()
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_no_scale_get(const Evas_Object *obj);

/**
 * Set if the object is (up/down) resizable.
 *
 * @param obj The image object
 * @param size_up A bool to set if the object is resizable up. Default is
 * @c EINA_TRUE.
 * @param size_down A bool to set if the object is resizable down. Default
 * is @c EINA_TRUE.
 *
 * This function limits the image resize ability. If @p size_up is set to
 * @c EINA_FALSE, the object can't have its height or width resized to a value
 * higher than the original image size. Same is valid for @p size_down.
 *
 * @see elm_image_resizable_get()
 *
 * @ingroup Image
 */
EAPI void             elm_image_resizable_set(Evas_Object *obj, Eina_Bool size_up, Eina_Bool size_down);

/**
 * Get if the object is (up/down) resizable.
 *
 * @param obj The image object
 * @param size_up A bool to set if the object is resizable up
 * @param size_down A bool to set if the object is resizable down
 *
 * @see elm_image_resizable_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_resizable_get(const Evas_Object *obj, Eina_Bool *size_up, Eina_Bool *size_down);

/**
 * Set if the image fills the entire object area, when keeping the aspect ratio.
 *
 * @param obj The image object
 * @param fill_outside @c EINA_TRUE if the object is filled outside,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 *
 * When the image should keep its aspect ratio even if resized to another
 * aspect ratio, there are two possibilities to resize it: keep the entire
 * image inside the limits of height and width of the object (@p fill_outside
 * is @c EINA_FALSE) or let the extra width or height go outside of the object,
 * and the image will fill the entire object (@p fill_outside is @c EINA_TRUE).
 *
 * @note This option will have no effect if
 * elm_image_aspect_fixed_set() is set to @c EINA_FALSE.
 *
 * @see elm_image_fill_outside_get()
 * @see elm_image_aspect_fixed_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside);

/**
 * Get if the object is filled outside
 *
 * @param obj The image object
 * @return @c EINA_TRUE if the object is filled outside, @c EINA_FALSE otherwise.
 *
 * @see elm_image_fill_outside_set()
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_fill_outside_get(const Evas_Object *obj);

/**
 * Enable or disable preloading of the image
 *
 * @param obj The image object
 * @param disabled If EINA_TRUE, preloading will be disabled
 * @ingroup Image
 */
EAPI void             elm_image_preload_disabled_set(Evas_Object *obj, Eina_Bool disabled);

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
 * Set the image orientation.
 *
 * @param obj The image object
 * @param orient The image orientation @ref Elm_Image_Orient
 *  Default is #ELM_IMAGE_ORIENT_NONE.
 *
 * This function allows to rotate or flip the given image.
 *
 * @see elm_image_orient_get()
 * @see @ref Elm_Image_Orient
 *
 * @ingroup Image
 */
EAPI void             elm_image_orient_set(Evas_Object *obj, Elm_Image_Orient orient);

/**
 * Get the image orientation.
 *
 * @param obj The image object
 * @return The image orientation @ref Elm_Image_Orient
 *
 * @see elm_image_orient_set()
 * @see @ref Elm_Image_Orient
 *
 * @ingroup Image
 */
EAPI Elm_Image_Orient elm_image_orient_get(const Evas_Object *obj);

/**
 * Make the image 'editable'.
 *
 * @param obj Image object.
 * @param set Turn on or off editability. Default is @c EINA_FALSE.
 *
 * This means the image is a valid drag target for drag and drop, and can be
 * cut or pasted too.
 *
 * @ingroup Image
 */
EAPI void             elm_image_editable_set(Evas_Object *obj, Eina_Bool set);

/**
 * Check if the image is 'editable'.
 *
 * @param obj Image object.
 * @return Editability.
 *
 * A return value of EINA_TRUE means the image is a valid drag target
 * for drag and drop, and can be cut or pasted too.
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_editable_get(const Evas_Object *obj);

/**
 * Get the inlined image object of the image widget.
 *
 * @param obj The image object to get the inlined image from
 * @return The inlined image object, or NULL if none exists
 *
 * This function allows one to get the underlying @c Evas_Object of type
 * Image from this elementary widget. It can be useful to do things like get
 * the pixel data, save the image to a file, etc.
 *
 * @note Be careful to not manipulate it, as it is under control of
 * elementary.
 *
 * @ingroup Image
 */
EAPI Evas_Object     *elm_image_object_get(const Evas_Object *obj);

/**
 * Set whether the original aspect ratio of the image should be kept on resize.
 *
 * @param obj The image object.
 * @param fixed @c EINA_TRUE if the image should retain the aspect,
 * @c EINA_FALSE otherwise.
 *
 * The original aspect ratio (width / height) of the image is usually
 * distorted to match the object's size. Enabling this option will retain
 * this original aspect, and the way that the image is fit into the object's
 * area depends on the option set by elm_image_fill_outside_set().
 *
 * @see elm_image_aspect_fixed_get()
 * @see elm_image_fill_outside_set()
 *
 * @ingroup Image
 */
EAPI void             elm_image_aspect_fixed_set(Evas_Object *obj, Eina_Bool fixed);

/**
 * Get if the object retains the original aspect ratio.
 *
 * @param obj The image object.
 * @return @c EINA_TRUE if the object keeps the original aspect, @c EINA_FALSE
 * otherwise.
 *
 * @ingroup Image
 */
EAPI Eina_Bool        elm_image_aspect_fixed_get(const Evas_Object *obj);

/**
 * Get whether an image object supports animation or not.
 *
 * @param obj The image object
 * @return @c EINA_TRUE if the image supports animation,
 *         @c EINA_FALSE otherwise.
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
 */
EAPI Eina_Bool        elm_image_animated_available_get(const Evas_Object *obj);

/**
 * Set whether an image object (which supports animation) is to
 * animate itself or not.
 *
 * @param obj The image object

 * @param animated @c EINA_TRUE if the object is to animate itself,
 *                 @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
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
 */
EAPI void             elm_image_animated_set(Evas_Object *obj, Eina_Bool animated);

/**
 * Get whether an image object has animation enabled or not.
 *
 * @param obj The image object
 *
 * @return @c EINA_TRUE if the image has animation enabled,
 *         @c EINA_FALSE otherwise.
 *
 * @see elm_image_animated_set()
 *
 * @ingroup Image
 * @since 1.7
 */
EAPI Eina_Bool        elm_image_animated_get(const Evas_Object *obj);

/**
 * Start or stop an image object's animation.
 *
 * @param obj The image object
 * @param play @c EINA_TRUE to start the animation, @c EINA_FALSE
 *             otherwise. Default is @c EINA_FALSE.
 *
 * To actually start playing any image object's animation, if it
 * supports it, one must do something like:
 *
 * @code
 * if (elm_image_animated_available_get(img))
 *   {
 *      elm_image_animated_set(img, EINA_TRUE);
 *      elm_image_animated_play_set(img, EINA_TRUE);
 *   }
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
 */
EAPI void             elm_image_animated_play_set(Evas_Object *obj, Eina_Bool play);

/**
 * Get whether an image object is under animation or not.
 *
 * @param obj The image object
 * @return @c EINA_TRUE, if the image is being animated, @c EINA_FALSE
 *            otherwise.
 *
 * @see elm_image_animated_play_get()
 *
 * @ingroup Image
 * @since 1.7
 */
EAPI Eina_Bool        elm_image_animated_play_get(const Evas_Object *obj);

