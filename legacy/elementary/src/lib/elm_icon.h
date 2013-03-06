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

#define ELM_OBJ_ICON_CLASS elm_obj_icon_class_get()

const Eo_Class *elm_obj_icon_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_ICON_BASE_ID;

enum
{
   ELM_OBJ_ICON_SUB_ID_THUMB_SET,
   ELM_OBJ_ICON_SUB_ID_STANDARD_SET,
   ELM_OBJ_ICON_SUB_ID_STANDARD_GET,
   ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_SET,
   ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_GET,
   ELM_OBJ_ICON_SUB_ID_LAST
};

#define ELM_OBJ_ICON_ID(sub_id) (ELM_OBJ_ICON_BASE_ID + sub_id)


/**
 * @def elm_obj_icon_thumb_set
 * @since 1.8
 *
 * Set the file that will be used, but use a generated thumbnail.
 *
 * @param[in] file
 * @param[in] group
 *
 * @see elm_icon_thumb_set
 */
#define elm_obj_icon_thumb_set(file, group) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_THUMB_SET), EO_TYPECHECK(const char *, file), EO_TYPECHECK(const char *, group)

/**
 * @def elm_obj_icon_standard_set
 * @since 1.8
 *
 * Set the icon by icon standards names.
 *
 * @param[in] name
 * @param[out] ret
 *
 * @see elm_icon_standard_set
 */
#define elm_obj_icon_standard_set(name, ret) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_STANDARD_SET), EO_TYPECHECK(const char *, name), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_icon_standard_get
 * @since 1.8
 *
 * Get the icon name set by icon standard names.
 *
 * @param[out] ret
 *
 * @see elm_icon_standard_get
 */
#define elm_obj_icon_standard_get(ret) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_STANDARD_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_icon_order_lookup_set
 * @since 1.8
 *
 * Sets the icon lookup order used by elm_icon_standard_set().
 *
 * @param[in] order
 *
 * @see elm_icon_order_lookup_set
 */
#define elm_obj_icon_order_lookup_set(order) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_SET), EO_TYPECHECK(Elm_Icon_Lookup_Order, order)

/**
 * @def elm_obj_icon_order_lookup_get
 * @since 1.8
 *
 * Gets the icon lookup order.
 *
 * @param[out] ret
 *
 * @see elm_icon_order_lookup_get
 */
#define elm_obj_icon_order_lookup_get(ret) ELM_OBJ_ICON_ID(ELM_OBJ_ICON_SUB_ID_ORDER_LOOKUP_GET), EO_TYPECHECK(Elm_Icon_Lookup_Order *, ret)


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
 * @}
 */
