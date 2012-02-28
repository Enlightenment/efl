/**
 * @defgroup Bg Bg
 *
 * @image html img/widget/bg/preview-00.png
 * @image latex img/widget/bg/preview-00.eps
 *
 * @brief Background object, used for setting a solid color, image or Edje
 * group as background to a window or any container object.
 *
 * The bg object is used for setting a solid background to a window or
 * packing into any container object. It works just like an image, but has
 * some properties useful to a background, like setting it to tiled,
 * centered, scaled or stretched.
 *
 * Default content parts of the bg widget that you can use for are:
 * @li "overlay" - overlay of the bg
 *
 * Supported elm_object common APIs.
 * @li elm_object_part_content_set
 * @li elm_object_part_content_get
 * @li elm_object_part_content_unset
 *
 * Here is some sample code using it:
 * @li @ref bg_01_example_page
 * @li @ref bg_02_example_page
 * @li @ref bg_03_example_page
 */

/* bg */
typedef enum
{
   ELM_BG_OPTION_CENTER, /**< center the background */
   ELM_BG_OPTION_SCALE, /**< scale the background retaining aspect ratio */
   ELM_BG_OPTION_STRETCH, /**< stretch the background to fill */
   ELM_BG_OPTION_TILE /**< tile background at its original size */
} Elm_Bg_Option;

/**
 * Add a new background to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Bg
 */
EAPI Evas_Object                 *elm_bg_add(Evas_Object *parent);

/**
 * Set the file (image or edje) used for the background
 *
 * @param obj The bg object
 * @param file The file path
 * @param group Optional key (group in Edje) within the file
 *
 * This sets the image file used in the background object. The image (or edje)
 * will be stretched (retaining aspect if its an image file) to completely fill
 * the bg object. This may mean some parts are not visible.
 *
 * @note  Once the image of @p obj is set, a previously set one will be deleted,
 * even if @p file is NULL.
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 * Get the file (image or edje) used for the background
 *
 * @param obj The bg object
 * @param file The file path
 * @param group Optional key (group in Edje) within the file
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_file_get(const Evas_Object *obj, const char **file, const char **group);

/**
 * Set the option used for the background image
 *
 * @param obj The bg object
 * @param option The desired background option (TILE, SCALE)
 *
 * This sets the option used for manipulating the display of the background
 * image. The image can be tiled or scaled.
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_option_set(Evas_Object *obj, Elm_Bg_Option option);

/**
 * Get the option used for the background image
 *
 * @param obj The bg object
 * @return The desired background option (CENTER, SCALE, STRETCH or TILE)
 *
 * @ingroup Bg
 */
EAPI Elm_Bg_Option                elm_bg_option_get(const Evas_Object *obj);
/**
 * Set the option used for the background color
 *
 * @param obj The bg object
 * @param r
 * @param g
 * @param b
 *
 * This sets the color used for the background rectangle. Its range goes
 * from 0 to 255.
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_color_set(Evas_Object *obj, int r, int g, int b);
/**
 * Get the option used for the background color
 *
 * @param obj The bg object
 * @param r
 * @param g
 * @param b
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_color_get(const Evas_Object *obj, int *r, int *g, int *b);

/**
 * Set the size of the pixmap representation of the image.
 *
 * This option just makes sense if an image is going to be set in the bg.
 *
 * @param obj The bg object
 * @param w The new width of the image pixmap representation.
 * @param h The new height of the image pixmap representation.
 *
 * This function sets a new size for pixmap representation of the given bg
 * image. It allows the image to be loaded already in the specified size,
 * reducing the memory usage and load time when loading a big image with load
 * size set to a smaller size.
 *
 * NOTE: this is just a hint, the real size of the pixmap may differ
 * depending on the type of image being loaded, being bigger than requested.
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_load_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);

/**
 * @}
 */
