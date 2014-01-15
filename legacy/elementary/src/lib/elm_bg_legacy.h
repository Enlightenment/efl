/**
 * Add a new background to the parent
 *
 * @param parent The parent object
 * @return The new object or @c NULL if it cannot be created
 *
 * @ingroup Bg
 */
EAPI Evas_Object                 *elm_bg_add(Evas_Object *parent);

/**
 * Set the file (image or edje collection) to give life for the
 * background
 *
 * @param obj The background object handle
 * @param file The file path
 * @param group Optional key (group in Edje) within the file
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 *
 * This sets the image file used in the background object. If the
 * image comes from an Edje group, it will be stretched to completely
 * fill the background object. If it comes from a traditional image file, it
 * will by default be centered in this widget's are (thus retaining
 * its aspect), what could lead to some parts being not visible. You
 * may change the mode of exhibition for a real image file with
 * elm_bg_option_set().
 *
 * @note Once the image of @p obj is set, a previously set one will be
 * deleted, even if @p file is @c NULL.
 *
 * @note This will only affect the contents of one of the background's
 * swallow spots, namely @c "elm.swallow.background". If you want to
 * achieve the @c Layout's file setting behavior, you'll have to call
 * that method on this object.
 *
 * @ingroup Bg
 */
EAPI Eina_Bool                    elm_bg_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 * Get the file (image or edje collection) set on a given background
 * widget
 *
 * @param obj The background object handle
 * @param file Where to store the requested file's path
 * @param group Where to store the optional key within @a file, @b if
 * it's an Edje file
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_file_get(const Evas_Object *obj, const char **file, const char **group);

/**
 * Set the mode of display for a given background widget's image
 *
 * @param obj The background object handle
 * @param option The desired background option (see #Elm_Bg_Option)
 *
 * This sets how the background widget will display its image. This
 * will only work if the elm_bg_file_set() was previously called with
 * an image file on @a obj. The image can be display tiled, scaled,
 * centered or stretched.
 *
 * @see elm_bg_option_get()
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_option_set(Evas_Object *obj, Elm_Bg_Option option);

/**
 * Get the mode of display for a given background widget's image
 *
 * @param obj The background object handle
 * @return The image displaying mode in use for @a obj or #ELM_BG_OPTION_LAST,
 * on errors.
 *
 * @see elm_bg_option_set() for more details
 *
 * @ingroup Bg
 */
EAPI Elm_Bg_Option                elm_bg_option_get(const Evas_Object *obj);

/**
 * Set the color on a given background widget
 *
 * @param obj The background object handle
 * @param r The red color component's value
 * @param g The green color component's value
 * @param b The blue color component's value
 *
 * This sets the color used for the background rectangle, in RGB
 * format. Each color component's range is from 0 to 255.
 *
 * @note You probably only want to use this function if you haven't
 * previously called elm_bg_file_set(), so that you just want a solid
 * color background.
 *
 * @note You can reset the color by setting @p r, @p g, @p b as -1, -1, -1.
 *
 * @see elm_bg_color_get()
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_color_set(Evas_Object *obj, int r, int g, int b);

/**
 * Get the color set on a given background widget
 *
 * @param obj The background object handle
 * @param r Where to store the red color component's value
 * @param g Where to store the green color component's value
 * @param b Where to store the blue color component's value
 *
 * @note Use @c NULL pointers on the file components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_bg_color_get() for more details
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_color_get(const Evas_Object *obj, int *r, int *g, int *b);

/**
 * Set the size of the pixmap representation of the image set on a
 * given background widget.
 *
 * @param obj The background object handle
 * @param w The new width of the image pixmap representation.
 * @param h The new height of the image pixmap representation.
 *
 * @warning This function just makes sense if an image file was set on
 * @p obj, with elm_bg_file_set().
 *
 * This function sets a new size for pixmap representation of the
 * given bg image. It allows for the image to be loaded already in the
 * specified size, reducing the memory usage and load time (for
 * example, when loading a big image file with its load size set to a
 * smaller size)
 *
 * @note This is just a hint for the underlying system. The real size
 * of the pixmap may differ depending on the type of image being
 * loaded, being bigger than requested.
 *
 * @ingroup Bg
 */
EAPI void                         elm_bg_load_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
