/**
 * Add a new image to the parent.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_image_file_set()
 *
 * @ingroup Elm_Image
 */
EAPI Evas_Object     *elm_image_add(Evas_Object *parent);

/** Structure associated with smart callback 'download,progress'.
 *
 * @since 1.8
 *
 * @ingroup Elm_Image
 */
typedef struct _Elm_Image_Progress
{
  double now;
  double total;
} Elm_Image_Progress;

/** Structure associated with smart callback 'download,progress'.
 *
 * @since 1.8
 *
 * @ingroup Elm_Image
 */
typedef struct _Elm_Image_Error
{
  int status;
  Eina_Bool open_error;
} Elm_Image_Error;


typedef Evas_Object Elm_Image;

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
 * @ingroup Elm_Image
 */
EAPI Eina_Bool        elm_image_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 * Get the file that will be used as image.
 *
 * @see elm_image_file_set()
 *
 * @ingroup Elm_Image
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
 * @ingroup Elm_Image
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
 * @ingroup Elm_Image
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
 * @ingroup Elm_Image
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
 * @ingroup Elm_Image
 * @since 1.7
 *
 * @param obj The image object
 * @param[in] play @c EINA_TRUE to start the animation, @c EINA_FALSE
otherwise. Default is @c EINA_FALSE.
 */
EAPI void             elm_image_animated_play_set(Evas_Object *obj, Eina_Bool play);

/**
 * Get whether an image object is under animation or not.
 *
 * @param obj The image object
 * @return @c EINA_TRUE, if the image is being animated, @c EINA_FALSE
 * otherwise.
 *
 * @see elm_image_animated_play_get()
 *
 * @ingroup Elm_Image
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
 * @ingroup Elm_Image
 * @since 1.7
 *
 * @param obj The image object
 * @param[in] anim @c EINA_TRUE if the object is to animate itself,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 */
EAPI void             elm_image_animated_set(Evas_Object *obj, Eina_Bool anim);

/**
 *
 * Get whether an image object has animation enabled or not.
 *
 * @param obj The image object
 * @return @c EINA_TRUE if the image has animation enabled,
 * @c EINA_FALSE otherwise.
 *
 * @see elm_image_animated_set()
 *
 * @ingroup Elm_Image
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
 * @ingroup Elm_Image
 * @since 1.7
 *
 */
EAPI Eina_Bool        elm_image_animated_available_get(const Evas_Object *obj);

/**
 * @brief Contrtol if the image is 'editable'.
 *
 * This means the image is a valid drag target for drag and drop, and can be
 * cut or pasted too.
 *
 * @param[in] set Turn on or off editability. Default is @c false.
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_editable_set(Evas_Object *obj, Eina_Bool set);

/**
 * @brief Contrtol if the image is 'editable'.
 *
 * This means the image is a valid drag target for drag and drop, and can be
 * cut or pasted too.
 *
 * @return Turn on or off editability. Default is @c false.
 *
 * @ingroup Elm_Image
 */
EAPI Eina_Bool elm_image_editable_get(const Evas_Object *obj);

/**
 * @brief Set a location in memory to be used as an image object's source
 * bitmap.
 *
 * This function is handy when the contents of an image file are mapped in
 * memory, for example.
 *
 * The @c format string should be something like $"png", $"jpg", $"tga",
 * $"tiff", $"bmp" etc, when provided ($NULL, on the contrary). This improves
 * the loader performance as it tries the "correct" loader first, before trying
 * a range of other possible loaders until one succeeds.
 *
 * @param[in] img The binary data that will be used as image source
 * @param[in] size The size of binary data blob @c img
 * @param[in] format (Optional) expected format of @c img bytes
 * @param[in] key Optional indexing key of @c img to be passed to the image
 * loader (eg. if @c img is a memory-mapped EET file)
 *
 * @return @c true = success, @c false = error
 *
 * @since 1.7
 *
 * @ingroup Elm_Image
 */
EAPI Eina_Bool elm_image_memfile_set(Evas_Object *obj, const void *img, size_t size, const char *format, const char *key);

/**
 * @brief Control if the image fills the entire object area, when keeping the
 * aspect ratio.
 *
 * When the image should keep its aspect ratio even if resized to another
 * aspect ratio, there are two possibilities to resize it: keep the entire
 * image inside the limits of height and width of the object ($fill_outside is
 * @c false) or let the extra width or height go outside of the object, and the
 * image will fill the entire object ($fill_outside is @c true).
 *
 * @note This option will have no effect if @ref elm_image_aspect_fixed_get is
 * set to @c false.
 *
 * @param[in] fill_outside @c true if the object is filled outside, @c false
 * otherwise. Default is @c false.
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside);

/**
 * @brief Control if the image fills the entire object area, when keeping the
 * aspect ratio.
 *
 * When the image should keep its aspect ratio even if resized to another
 * aspect ratio, there are two possibilities to resize it: keep the entire
 * image inside the limits of height and width of the object ($fill_outside is
 * @c false) or let the extra width or height go outside of the object, and the
 * image will fill the entire object ($fill_outside is @c true).
 *
 * @note This option will have no effect if @ref elm_image_aspect_fixed_get is
 * set to @c false.
 *
 * @return @c true if the object is filled outside, @c false otherwise. Default
 * is @c false.
 *
 * @ingroup Elm_Image
 */
EAPI Eina_Bool elm_image_fill_outside_get(const Evas_Object *obj);

/**
 * @brief Enable or disable preloading of the image
 *
 * @param[in] disabled If true, preloading will be disabled
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_preload_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/** Using Evas_Image_Orient enums.
 *
 * @since 1.14
 *
 * @ingroup Elm_Image
 */
typedef enum _Elm_Image_Orient_Type
{
  ELM_IMAGE_ORIENT_NONE = 0,      /**< no orientation change */
  ELM_IMAGE_ORIENT_0 = 0,         /**< no orientation change */
  ELM_IMAGE_ROTATE_90 = 1,        /**< rotate 90 degrees clockwise */
  ELM_IMAGE_ORIENT_90 = 1,        /**< rotate 90 degrees clockwise */
  ELM_IMAGE_ROTATE_180 = 2,       /**< rotate 180 degrees clockwise */
  ELM_IMAGE_ORIENT_180 = 2,       /**< rotate 180 degrees clockwise */
  ELM_IMAGE_ROTATE_270 = 3,       /**< rotate 90 degrees counter-clockwise (i.e. 270 degrees clockwise) */
  ELM_IMAGE_ORIENT_270 = 3,       /**< rotate 90 degrees counter-clockwise (i.e. 270 degrees clockwise) */
  ELM_IMAGE_FLIP_HORIZONTAL = 4,  /**< flip image horizontally (along the x = width / 2 line) */
  ELM_IMAGE_FLIP_VERTICAL = 5,    /**< flip image vertically (along the y = height / 2 line) */
  ELM_IMAGE_FLIP_TRANSPOSE = 6,   /**< flip image along the y = (width - x) line (bottom-left to top-right) */
  ELM_IMAGE_FLIP_TRANSVERSE = 7   /**< flip image along the y = x line (top-left to bottom-right) */
} Elm_Image_Orient;

/**
 * @brief Contrtol the image orientation.
 *
 * This function allows to rotate or flip the given image.
 *
 * @param[in] orient The image orientation Elm.Image.Orient Default is
 * #ELM_IMAGE_ORIENT_NONE.
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_orient_set(Evas_Object *obj, Elm_Image_Orient orient);

/**
 * @brief Contrtol the image orientation.
 *
 * This function allows to rotate or flip the given image.
 *
 * @return The image orientation Elm.Image.Orient Default is
 * #ELM_IMAGE_ORIENT_NONE.
 *
 * @ingroup Elm_Image
 */
EAPI Elm_Image_Orient elm_image_orient_get(const Evas_Object *obj);

/**
 * @brief Get the inlined image object of the image widget.
 *
 * This function allows one to get the underlying @c Evas_Object of type Image
 * from this elementary widget. It can be useful to do things like get the
 * pixel data, save the image to a file, etc.
 *
 * @note Be careful to not manipulate it, as it is under control of elementary.
 *
 * @return The inlined image object.
 *
 * @ingroup Elm_Image
 */
EAPI Evas_Object *elm_image_object_get(const Evas_Object *obj);

/**
 * @brief Get the current size of the image.
 *
 * This is the real size of the image, not the size of the object.
 *
 * @param[out] w Pointer to store width, or NULL.
 * @param[out] h Pointer to store height, or NULL.
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_object_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * @brief Control if the object is (up/down) resizable.
 *
 * This function limits the image resize ability. If @c size_up is set to
 * @c false, the object can't have its height or width resized to a value
 * higher than the original image size. Same is valid for @c size_down.
 *
 * @param[in] up A bool to set if the object is resizable up. Default is
 * @c true.
 * @param[in] down A bool to set if the object is resizable down. Default is
 * @c true.
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_resizable_set(Evas_Object *obj, Eina_Bool up, Eina_Bool down);

/**
 * @brief Control if the object is (up/down) resizable.
 *
 * This function limits the image resize ability. If @c size_up is set to
 * @c false, the object can't have its height or width resized to a value
 * higher than the original image size. Same is valid for @c size_down.
 *
 * @param[out] up A bool to set if the object is resizable up. Default is
 * @c true.
 * @param[out] down A bool to set if the object is resizable down. Default is
 * @c true.
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_resizable_get(const Evas_Object *obj, Eina_Bool *up, Eina_Bool *down);

/**
 * @brief Control scaling behaviour of this object.
 *
 * This function disables scaling of the elm_image widget through the function
 * elm_object_scale_set(). However, this does not affect the widget size/resize
 * in any way. For that effect, take a look at @ref elm_image_resizable_get and
 * @ref efl_gfx_entity_scale_get
 *
 * @param[in] no_scale @c true if the object is not scalable, @c false
 * otherwise. Default is @c false.
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_no_scale_set(Evas_Object *obj, Eina_Bool no_scale);

/**
 * @brief Control scaling behaviour of this object.
 *
 * This function disables scaling of the elm_image widget through the function
 * elm_object_scale_set(). However, this does not affect the widget size/resize
 * in any way. For that effect, take a look at @ref elm_image_resizable_get and
 * @ref efl_gfx_entity_scale_get
 *
 * @return @c true if the object is not scalable, @c false otherwise. Default
 * is @c false.
 *
 * @ingroup Elm_Image
 */
EAPI Eina_Bool elm_image_no_scale_get(const Evas_Object *obj);

/**
 * @brief Control whether the internal image's aspect ratio
 * is fixed to the original image's aspect ratio
 *
 * @param[in] fixed @ true if the aspect ratio is fixed
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_aspect_fixed_set(Evas_Object *obj, Eina_Bool fixed);

/**
 * @brief Get whether the internal image's aspect ratio
 * is fixed to the original image's
 *
 * @return @ true if the aspect ratio is fixed
 *
 * @ingroup Elm_Image
 */
EAPI Eina_Bool elm_image_aspect_fixed_get(const Evas_Object *obj);

/**
 * @brief Enable asynchronous file I/O for elm_image_file_set.
 *
 * @param obj The image object
 * @param[in] async @ true will make elm_image_file_set() an asynchronous operation
 *
 * If @c true, this will make elm_image_file_set() an asynchronous operation.
 * Use of this function is not recommended and the standard EO-based
 * asynchronous I/O API should be preferred instead.
 *
 * @since 1.19
 *
 * @ingroup Elm_Image
 */
EAPI void elm_image_async_open_set(Evas_Object *obj, Eina_Bool async);

#include "efl_ui_image.eo.legacy.h"
