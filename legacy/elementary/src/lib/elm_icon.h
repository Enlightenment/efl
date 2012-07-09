/**
 * @defgroup Icon Icon
 * @ingroup Elementary
 *
 * @image html icon_inheritance_tree.png
 * @image latex icon_inheritance_tree.eps
 *
 * @image html img/widget/icon/preview-00.png
 * @image latex img/widget/icon/preview-00.eps
 *
 * An icon object is used to display standard icon images ("delete",
 * "edit", "arrows", etc.) or images coming from a custom file (PNG,
 * JPG, EDJE, etc.), on icon contexts.
 *
 * The icon image requested can be in the Elementary theme in use, or
 * in the @c freedesktop.org theme paths. It's possible to set the
 * order of preference from where an image will be fetched.
 *
 * This widget inherits from the @ref Image one, so that all the
 * functions acting on it also work for icon objects.
 *
 * You should be using an icon, instead of an image, whenever one of
 * the following apply:
 * - you need a @b thumbnail version of an original image
 * - you need freedesktop.org provided icon images
 * - you need theme provided icon images (Edje groups)
 *
 * Various calls on the icon's API are marked as @b deprecated, as
 * they just wrap the image counterpart functions. Use the ones we
 * point you to, for each case of deprecation here, instead --
 * eventually the deprecated ones will be discarded (next major
 * release).
 *
 * Default images provided by Elementary's default theme are described
 * below.
 *
 * These are names for icons that were first intended to be used in
 * toolbars, but can be used in many other places too:
 * @li @c "home"
 * @li @c "close"
 * @li @c "apps"
 * @li @c "arrow_up"
 * @li @c "arrow_down"
 * @li @c "arrow_left"
 * @li @c "arrow_right"
 * @li @c "chat"
 * @li @c "clock"
 * @li @c "delete"
 * @li @c "edit"
 * @li @c "refresh"
 * @li @c "folder"
 * @li @c "file"
 *
 * These are names for icons that were designed to be used in menus
 * (but again, you can use them anywhere else):
 * @li @c "menu/home"
 * @li @c "menu/close"
 * @li @c "menu/apps"
 * @li @c "menu/arrow_up"
 * @li @c "menu/arrow_down"
 * @li @c "menu/arrow_left"
 * @li @c "menu/arrow_right"
 * @li @c "menu/chat"
 * @li @c "menu/clock"
 * @li @c "menu/delete"
 * @li @c "menu/edit"
 * @li @c "menu/refresh"
 * @li @c "menu/folder"
 * @li @c "menu/file"
 *
 * And these are names for some media player specific icons:
 * @li @c "media_player/forward"
 * @li @c "media_player/info"
 * @li @c "media_player/next"
 * @li @c "media_player/pause"
 * @li @c "media_player/play"
 * @li @c "media_player/prev"
 * @li @c "media_player/rewind"
 * @li @c "media_player/stop"
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Image:
 * - @c "thumb,done" - elm_icon_thumb_set() has completed with success
 *                     (@since 1.7)
 * - @c "thumb,error" - elm_icon_thumb_set() has failed (@since 1.7)
 *
 * Elementary icon objects support the following API calls:
 * @li elm_object_signal_emit()
 * @li elm_object_signal_callback_add()
 * @li elm_object_signal_callback_del()
 * for emmiting and listening to signals on the object, when the
 * internal image comes from an Edje object. This behavior was added
 * unintentionally, though, and is @b deprecated. Expect it to be
 * dropped on future releases.
 *
 * An example of usage for this API follows:
 * @li @ref tutorial_icon
 */

/**
 * @addtogroup Icon
 * @{
 */

typedef enum
{
   ELM_ICON_NONE,
   ELM_ICON_FILE,
   ELM_ICON_STANDARD
} Elm_Icon_Type;

/**
 * @enum Elm_Icon_Lookup_Order
 * @typedef Elm_Icon_Lookup_Order
 *
 * Lookup order used by elm_icon_standard_set(). Should look for icons in the
 * theme, FDO paths, or both?
 *
 * @ingroup Icon
 */
typedef enum
{
   ELM_ICON_LOOKUP_FDO_THEME, /**< icon look up order: freedesktop, theme */
   ELM_ICON_LOOKUP_THEME_FDO, /**< icon look up order: theme, freedesktop */
   ELM_ICON_LOOKUP_FDO, /**< icon look up order: freedesktop */
   ELM_ICON_LOOKUP_THEME /**< icon look up order: theme */
} Elm_Icon_Lookup_Order;

/**
 * Add a new icon object to the parent.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @see elm_image_file_set()
 *
 * @ingroup Icon
 */
EAPI Evas_Object          *elm_icon_add(Evas_Object *parent);

/**
 * Set the file that will be used as icon.
 *
 * @param obj The icon object
 * @param file The path to file that will be used as icon image
 * @param group The group that the icon belongs to an edje file
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @note The icon image set by this function can be changed by
 * elm_icon_standard_set().
 *
 * @see elm_icon_file_get()
 *
 * @deprecated Use elm_image_file_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_file_set(Evas_Object *obj, const char *file, const char *group);

/**
 * Set a location in memory to be used as an icon
 *
 * @param obj The icon object
 * @param img The binary data that will be used as an image
 * @param size The size of binary data @p img
 * @param format Optional format of @p img to pass to the image loader
 * @param key Optional key of @p img to pass to the image loader (eg. if @p img is an edje file)
 *
 * The @p format string should be something like "png", "jpg", "tga",
 * "tiff", "bmp" etc. if it is provided (NULL if not). This improves
 * the loader performance as it tries the "correct" loader first before
 * trying a range of other possible loaders until one succeeds.
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * @note The icon image set by this function can be changed by
 * elm_icon_standard_set().
 *
 * @deprecated Use elm_image_memfile_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_memfile_set(Evas_Object *obj, const void *img, size_t size, const char *format, const char *key);

/**
 * Get the file that will be used as icon.
 *
 * @param obj The icon object
 * @param file The path to file that will be used as the icon image
 * @param group The group that the icon belongs to, in edje file
 *
 * @see elm_image_file_set()
 *
 * @deprecated Use elm_image_file_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_file_get(const Evas_Object *obj, const char **file, const char **group);

/**
 * Set the file that will be used, but use a generated thumbnail.
 *
 * @param obj The icon object
 * @param file The path to file that will be used as icon image
 * @param group The group that the icon belongs to an edje file
 *
 * This functions like elm_image_file_set() but requires the Ethumb library
 * support to be enabled successfully with elm_need_ethumb(). When set
 * the file indicated has a thumbnail generated and cached on disk for
 * future use or will directly use an existing cached thumbnail if it
 * is valid.
 *
 * @see elm_image_file_set()
 *
 * @ingroup Icon
 */
EAPI void                  elm_icon_thumb_set(Evas_Object *obj, const char *file, const char *group);

/**
 * Set the icon by icon standards names.
 *
 * @param obj The icon object
 * @param name The icon name
 *
 * @return (@c EINA_TRUE = success, @c EINA_FALSE = error)
 *
 * For example, freedesktop.org defines standard icon names such as "home",
 * "network", etc. There can be different icon sets to match those icon
 * keys. The @p name given as parameter is one of these "keys", and will be
 * used to look in the freedesktop.org paths and elementary theme. One can
 * change the lookup order with elm_icon_order_lookup_set().
 *
 * If name is not found in any of the expected locations and it is the
 * absolute path of an image file, this image will be used.
 *
 * @note The icon image set by this function can be changed by
 * elm_image_file_set().
 *
 * @see elm_icon_standard_get()
 * @see elm_image_file_set()
 *
 * @ingroup Icon
 */
EAPI Eina_Bool             elm_icon_standard_set(Evas_Object *obj, const char *name);

/**
 * Get the icon name set by icon standard names.
 *
 * @param obj The icon object
 * @return The icon name
 *
 * If the icon image was set using elm_image_file_set() instead of
 * elm_icon_standard_set(), then this function will return @c NULL.
 *
 * @see elm_icon_standard_set()
 *
 * @ingroup Icon
 */
EAPI const char           *elm_icon_standard_get(const Evas_Object *obj);

/**
 * Set the smooth scaling for an icon object.
 *
 * @param obj The icon object
 * @param smooth @c EINA_TRUE if smooth scaling should be used, @c EINA_FALSE
 * otherwise. Default is @c EINA_TRUE.
 *
 * Set the scaling algorithm to be used when scaling the icon image. Smooth
 * scaling provides a better resulting image, but is slower.
 *
 * The smooth scaling should be disabled when making animations that change
 * the icon size, since they will be faster. Animations that don't require
 * resizing of the icon can keep the smooth scaling enabled (even if the icon
 * is already scaled, since the scaled icon image will be cached).
 *
 * @see elm_icon_smooth_get()
 *
 * @deprecated Use elm_image_smooth_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_smooth_set(Evas_Object *obj, Eina_Bool smooth);

/**
 * Get whether smooth scaling is enabled for an icon object.
 *
 * @param obj The icon object
 * @return @c EINA_TRUE if smooth scaling is enabled, @c EINA_FALSE otherwise.
 *
 * @see elm_icon_smooth_set()
 *
 * @deprecated Use elm_image_smooth_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_smooth_get(const Evas_Object *obj);

/**
 * Disable scaling of this object.
 *
 * @param obj The icon object.
 * @param no_scale @c EINA_TRUE if the object is not scalable, @c EINA_FALSE
 * otherwise. Default is @c EINA_FALSE.
 *
 * This function disables scaling of the icon object through the function
 * elm_object_scale_set(). However, this does not affect the object
 * size/resize in any way. For that effect, take a look at
 * elm_icon_resizable_set().
 *
 * @see elm_icon_no_scale_get()
 * @see elm_icon_resizable_set()
 * @see elm_object_scale_set()
 *
 * @deprecated Use elm_image_no_scale_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_no_scale_set(Evas_Object *obj, Eina_Bool no_scale);

/**
 * Get whether scaling is disabled on the object.
 *
 * @param obj The icon object
 * @return @c EINA_TRUE if scaling is disabled, @c EINA_FALSE otherwise
 *
 * @see elm_icon_no_scale_set()
 *
 * @deprecated Use elm_image_no_scale_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_no_scale_get(const Evas_Object *obj);

/**
 * Set if the object is (up/down) resizable.
 *
 * @param obj The icon object
 * @param size_up A bool to set if the object is resizable up. Default is
 * @c EINA_TRUE.
 * @param size_down A bool to set if the object is resizable down. Default
 * is @c EINA_TRUE.
 *
 * This function limits the icon object resize ability. If @p size_up is set to
 * @c EINA_FALSE, the object can't have its height or width resized to a value
 * higher than the original icon size. Same is valid for @p size_down.
 *
 * @see elm_icon_resizable_get()
 *
 * @deprecated Use elm_image_resizable_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_resizable_set(Evas_Object *obj, Eina_Bool size_up, Eina_Bool size_down);

/**
 * Get if the object is (up/down) resizable.
 *
 * @param obj The icon object
 * @param size_up A bool to set if the object is resizable up
 * @param size_down A bool to set if the object is resizable down
 *
 * @see elm_icon_resizable_set()
 *
 * @deprecated Use elm_image_resizable_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_resizable_get(const Evas_Object *obj, Eina_Bool *size_up, Eina_Bool *size_down);

/**
 * Get the object's image size
 *
 * @param obj The icon object
 * @param w A pointer to store the width in
 * @param h A pointer to store the height in
 *
 * @deprecated Use elm_image_object_size_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_size_get(const Evas_Object *obj, int *w, int *h);

/**
 * Set if the icon fill the entire object area.
 *
 * @param obj The icon object
 * @param fill_outside @c EINA_TRUE if the object is filled outside,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 *
 * When the icon object is resized to a different aspect ratio from the
 * original icon image, the icon image will still keep its aspect. This flag
 * tells how the image should fill the object's area. They are: keep the
 * entire icon inside the limits of height and width of the object (@p
 * fill_outside is @c EINA_FALSE) or let the extra width or height go outside
 * of the object, and the icon will fill the entire object (@p fill_outside
 * is @c EINA_TRUE).
 *
 * @note Unlike @ref Image, there's no option in icon to set the aspect ratio
 * retain property to false. Thus, the icon image will always keep its
 * original aspect ratio.
 *
 * @see elm_icon_fill_outside_get()
 *
 * @deprecated Use elm_image_fill_outside_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_fill_outside_set(Evas_Object *obj, Eina_Bool fill_outside);

/**
 * Get if the object is filled outside.
 *
 * @param obj The icon object
 * @return @c EINA_TRUE if the object is filled outside, @c EINA_FALSE
 *         otherwise.
 *
 * @see elm_icon_fill_outside_set()
 *
 * @deprecated Use elm_image_fill_outside_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool             elm_icon_fill_outside_get(const Evas_Object *obj);

/**
 * Set the prescale size for the icon.
 *
 * @param obj The icon object
 * @param size The prescale size. This value is used for both width and
 * height.
 *
 * This function sets a new size for pixmap representation of the given
 * icon. It allows the icon to be loaded already in the specified size,
 * reducing the memory usage and load time when loading a big icon with load
 * size set to a smaller size.
 *
 * It's equivalent to the elm_bg_load_size_set() function for bg.
 *
 * @note this is just a hint, the real size of the pixmap may differ
 * depending on the type of icon being loaded, being bigger than requested.
 *
 * @see elm_icon_prescale_get()
 * @see elm_bg_load_size_set()
 *
 * @deprecated Use elm_image_prescale_set() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI void                  elm_icon_prescale_set(Evas_Object *obj, int size);

/**
 * Get the prescale size for the icon.
 *
 * @param obj The icon object
 * @return The prescale size
 *
 * @see elm_icon_prescale_set()
 *
 * @deprecated Use elm_image_prescale_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI int                   elm_icon_prescale_get(const Evas_Object *obj);

/**
 * Gets the image object of the icon. DO NOT MODIFY THIS.
 *
 * @param obj The icon object
 * @return The internal icon object
 *
 * @deprecated Use elm_image_object_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Evas_Object          *elm_icon_object_get(Evas_Object *obj);

/**
 * Sets the icon lookup order used by elm_icon_standard_set().
 *
 * @param obj The icon object
 * @param order The icon lookup order (can be one of
 * ELM_ICON_LOOKUP_FDO_THEME, ELM_ICON_LOOKUP_THEME_FDO, ELM_ICON_LOOKUP_FDO
 * or ELM_ICON_LOOKUP_THEME)
 *
 * @see elm_icon_order_lookup_get()
 * @see Elm_Icon_Lookup_Order
 *
 * @ingroup Icon
 */
EAPI void                  elm_icon_order_lookup_set(Evas_Object *obj, Elm_Icon_Lookup_Order order);

/**
 * Gets the icon lookup order.
 *
 * @param obj The icon object
 * @return The icon lookup order
 *
 * @see elm_icon_order_lookup_set()
 * @see Elm_Icon_Lookup_Order
 *
 * @ingroup Icon
 */
EAPI Elm_Icon_Lookup_Order elm_icon_order_lookup_get(const Evas_Object *obj);

/**
 * Enable or disable preloading of the icon
 *
 * @param obj The icon object
 * @param disabled If EINA_TRUE, preloading will be disabled
 * @ingroup Icon
 *
 * @deprecated Use elm_image_preload_disabled_set() instead.
 *
 */
EINA_DEPRECATED EAPI void  elm_icon_preload_disabled_set(Evas_Object *obj, Eina_Bool disabled);

/**
 * Get if the icon supports animation or not.
 *
 * @param obj The icon object
 * @return @c EINA_TRUE if the icon supports animation,
 *         @c EINA_FALSE otherwise.
 *
 * Return if this elm icon's image can be animated. Currently Evas only
 * supports gif animation. If the return value is EINA_FALSE, other
 * elm_icon_animated_xxx APIs won't work.
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_available_get() instead.
 *
 */
EINA_DEPRECATED EAPI Eina_Bool elm_icon_animated_available_get(const Evas_Object *obj);

/**
 * Set animation mode of the icon.
 *
 * @param obj The icon object
 * @param animated @c EINA_TRUE if the object do animation job,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 *
 * Since the default animation mode is set to EINA_FALSE,
 * the icon is shown without animation. Files like animated GIF files
 * can animate, and this is supported if you enable animated support
 * on the icon.
 * Set it to EINA_TRUE when the icon needs to be animated.
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_set() instead.
 *
 */
EINA_DEPRECATED EAPI void  elm_icon_animated_set(Evas_Object *obj, Eina_Bool animated);

/**
 * Get animation mode of the icon.
 *
 * @param obj The icon object
 * @return The animation mode of the icon object
 * @see elm_icon_animated_set
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_get() instead.
 *
 */
EINA_DEPRECATED EAPI Eina_Bool elm_icon_animated_get(const Evas_Object *obj);

/**
 * Set animation play mode of the icon.
 *
 * @param obj The icon object
 * @param play @c EINA_TRUE the object play animation images,
 * @c EINA_FALSE otherwise. Default is @c EINA_FALSE.
 *
 * To play elm icon's animation, set play to EINA_TRUE.
 * For example, you make gif player using this set/get API and click event.
 * This literally lets you control current play or paused state. To have
 * this work with animated GIF files for example, you first, before
 * setting the file have to use elm_icon_animated_set() to enable animation
 * at all on the icon.
 *
 * 1. Click event occurs
 * 2. Check play flag using elm_icon_animated_play_get
 * 3. If elm icon was playing, set play to EINA_FALSE.
 *    Then animation will be stopped and vice versa
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_play_set() instead.
 *
 */
EINA_DEPRECATED EAPI void  elm_icon_animated_play_set(Evas_Object *obj, Eina_Bool play);

/**
 * Get animation play mode of the icon.
 *
 * @param obj The icon object
 * @return The play mode of the icon object
 *
 * @see elm_icon_animated_play_get
 * @ingroup Icon
 *
 * @deprecated Use elm_image_animated_play_get() instead.
 *
 */
EINA_DEPRECATED EAPI Eina_Bool elm_icon_animated_play_get(const Evas_Object *obj);

/**
 * Set whether the original aspect ratio of the icon should be kept on resize.
 *
 * @param obj The icon object.
 * @param fixed @c EINA_TRUE if the icon should retain the aspect,
 * @c EINA_FALSE otherwise.
 *
 * The original aspect ratio (width / height) of the icon is usually
 * distorted to match the object's size. Enabling this option will retain
 * this original aspect, and the way that the icon is fit into the object's
 * area depends on the option set by elm_icon_fill_outside_set().
 *
 * @see elm_icon_aspect_fixed_get()
 * @see elm_icon_fill_outside_set()
 *
 * @ingroup Icon
 *
 * @deprecated Use elm_image_aspect_fixed_set() instead.
 *
 */
EINA_DEPRECATED EAPI void  elm_icon_aspect_fixed_set(Evas_Object *obj, Eina_Bool fixed);

/**
 * Get if the object retains the original aspect ratio.
 *
 * @param obj The icon object.
 * @return @c EINA_TRUE if the object keeps the original aspect, @c EINA_FALSE
 * otherwise.
 *
 * @deprecated Use elm_image_aspect_fixed_get() instead.
 *
 * @ingroup Icon
 */
EINA_DEPRECATED EAPI Eina_Bool elm_icon_aspect_fixed_get(const Evas_Object *obj);

/**
 * @}
 */
