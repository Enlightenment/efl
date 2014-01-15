/**
 * @brief Add a new Photocam object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Photocam
 */
EAPI Evas_Object           *elm_photocam_add(Evas_Object *parent);

/**
 * @brief Set the photo file to be shown
 *
 * @param obj The photocam object
 * @param file The photo file
 * @return The return error (see EVAS_LOAD_ERROR_NONE, EVAS_LOAD_ERROR_GENERIC etc.)
 *
 * This sets (and shows) the specified file (with a relative or absolute
 * path) and will return a load error (same error that
 * evas_object_image_load_error_get() will return). The image will change and
 * adjust its size at this point and begin a background load process for this
 * photo that at some time in the future will be displayed at the full
 * quality needed.
 *
 * @ingroup Photocam
 */
EAPI Evas_Load_Error        elm_photocam_file_set(Evas_Object *obj, const char *file);

/**
 * @brief Returns the path of the current image file
 *
 * @param obj The photocam object
 * @return Returns the path
 *
 * @see elm_photocam_file_set()
 *
 * @ingroup Photocam
 */
EAPI const char            *elm_photocam_file_get(const Evas_Object *obj);

/**
 * @brief Set the zoom level of the photo
 *
 * @param obj The photocam object
 * @param zoom The zoom level to set
 *
 * This sets the zoom level. If @p zoom is 1, it means no zoom. If it's smaller
 * than 1, it means zoom in. If it's bigger than 1, it means zoom out. For
 * example, @p zoom 1 will be 1:1 pixel for pixel. @p zoom 2 will be 2:1
 * (that is 2x2 photo pixels will display as 1 on-screen pixel) which is a zoom
 * out. 4:1 will be 4x4 photo pixels as 1 screen pixel, and so on. The @p zoom
 * parameter must be greater than 0. It is suggested to stick to powers of 2.
 * (1, 2, 4, 8, 16, 32, etc.).
 *
 * @ingroup Photocam
 */
EAPI void                   elm_photocam_zoom_set(Evas_Object *obj, double zoom);

/**
 * @brief Get the zoom level of the photo
 *
 * @param obj The photocam object
 * @return The current zoom level
 *
 * This returns the current zoom level of the photocam object. Note that if
 * you set the fill mode to other than ELM_PHOTOCAM_ZOOM_MODE_MANUAL
 * (which is the default), the zoom level may be changed at any time by the
 * photocam object itself to account for photo size and photocam viewport
 * size.
 *
 * @see elm_photocam_zoom_set()
 * @see elm_photocam_zoom_mode_set()
 *
 * @ingroup Photocam
 */
EAPI double                 elm_photocam_zoom_get(const Evas_Object *obj);

/**
 * @brief Set the zoom mode
 *
 * @param obj The photocam object
 * @param mode The desired mode
 *
 * This sets the zoom mode to manual or one of several automatic levels.
 * Manual (ELM_PHOTOCAM_ZOOM_MODE_MANUAL) means that zoom is set manually by
 * elm_photocam_zoom_set() and will stay at that level until changed by code
 * or until zoom mode is changed. This is the default mode. The Automatic
 * modes will allow the photocam object to automatically adjust zoom mode
 * based on properties. ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT) will adjust zoom so
 * the photo fits EXACTLY inside the scroll frame with no pixels outside this
 * region. ELM_PHOTOCAM_ZOOM_MODE_AUTO_FILL will be similar but ensure no
 * pixels within the frame are left unfilled.
 *
 * @ingroup Photocam
 */
EAPI void                   elm_photocam_zoom_mode_set(Evas_Object *obj, Elm_Photocam_Zoom_Mode mode);

/**
 * @brief Get the zoom mode
 *
 * @param obj The photocam object
 * @return The current zoom mode
 *
 * This gets the current zoom mode of the photocam object.
 *
 * @see elm_photocam_zoom_mode_set()
 *
 * @ingroup Photocam
 */
EAPI Elm_Photocam_Zoom_Mode elm_photocam_zoom_mode_get(const Evas_Object *obj);

/**
 * @brief Get the current image pixel width and height
 *
 * @param obj The photocam object
 * @param w A pointer to the width return
 * @param h A pointer to the height return
 *
 * This gets the current photo pixel width and height (for the original).
 * The size will be returned in the integers @p w and @p h that are pointed
 * to.
 *
 * @ingroup Photocam
 */
EAPI void                   elm_photocam_image_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * @brief Get the region of the image that is currently shown
 *
 * @param obj
 * @param x A pointer to the X-coordinate of region
 * @param y A pointer to the Y-coordinate of region
 * @param w A pointer to the width
 * @param h A pointer to the height
 *
 * @see elm_photocam_image_region_show()
 * @see elm_photocam_image_region_bring_in()
 *
 * @ingroup Photocam
 */
EAPI void                   elm_photocam_image_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h);

/**
 * @brief Set the viewed region of the image
 *
 * @param obj The photocam object
 * @param x X-coordinate of region in image original pixels
 * @param y Y-coordinate of region in image original pixels
 * @param w Width of region in image original pixels
 * @param h Height of region in image original pixels
 *
 * This shows the region of the image without using animation.
 *
 * @ingroup Photocam
 */
EAPI void                   elm_photocam_image_region_show(Evas_Object *obj, int x, int y, int w, int h);

/**
 * @brief Bring in the viewed portion of the image
 *
 * @param obj The photocam object
 * @param x X-coordinate of region in image original pixels
 * @param y Y-coordinate of region in image original pixels
 * @param w Width of region in image original pixels
 * @param h Height of region in image original pixels
 *
 * This shows the region of the image using animation.
 *
 * @ingroup Photocam
 */
EAPI void                   elm_photocam_image_region_bring_in(Evas_Object *obj, int x, int y, int w, int h);

/**
 * @brief Set the paused state for photocam
 *
 * @param obj The photocam object
 * @param paused The pause state to set
 *
 * This sets the paused state to on(EINA_TRUE) or off (EINA_FALSE) for
 * photocam. The default is off. This will stop zooming using animation on
 * zoom level changes and change instantly. This will stop any existing
 * animations that are running.
 *
 * @ingroup Photocam
 */
EAPI void                   elm_photocam_paused_set(Evas_Object *obj, Eina_Bool paused);

/**
 * @brief Get the paused state for photocam
 *
 * @param obj The photocam object
 * @return The current paused state
 *
 * This gets the current paused state for the photocam object.
 *
 * @see elm_photocam_paused_set()
 *
 * @ingroup Photocam
 */
EAPI Eina_Bool              elm_photocam_paused_get(const Evas_Object *obj);

/**
 * @brief Get the internal low-res image used for photocam
 *
 * @param obj The photocam object
 * @return The internal image object handle, or NULL if none exists
 *
 * This gets the internal image object inside photocam. Do not modify it. It
 * is for inspection only, and hooking callbacks to. Nothing else. It may be
 * deleted at any time as well.
 *
 * @ingroup Photocam
 */
EAPI Evas_Object           *elm_photocam_internal_image_get(const Evas_Object *obj);

/**
 * @brief Set the gesture state for photocam.
 *
 * @param obj The photocam object
 * @param gesture The gesture state to set
 *
 * This sets the gesture state to on(EINA_TRUE) or off (EINA_FALSE) for
 * photocam. The default is off. This will start multi touch zooming.
 *
 * @ingroup Photocam
 */
EAPI void                  elm_photocam_gesture_enabled_set(Evas_Object *obj, Eina_Bool gesture);

/**
 * @brief Get the gesture state for photocam.
 *
 * @param obj The photocam object
 * @return The current gesture state
 *
 * This gets the current gesture state for the photocam object.
 *
 * @see elm_photocam_gesture_enabled_set()
 *
 * @ingroup Photocam
 */
EAPI Eina_Bool             elm_photocam_gesture_enabled_get(const Evas_Object *obj);
