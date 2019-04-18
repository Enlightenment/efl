#ifndef _ELM_SYSTRAY_EO_H_
#define _ELM_SYSTRAY_EO_H_

#ifndef _ELM_SYSTRAY_EO_CLASS_TYPE
#define _ELM_SYSTRAY_EO_CLASS_TYPE

typedef Eo Elm_Systray;

#endif

#ifndef _ELM_SYSTRAY_EO_TYPES
#define _ELM_SYSTRAY_EO_TYPES

/** Category of the Status Notifier Item.
 *
 * @since 1.8
 *
 * @ingroup Elm_Systray
 */
typedef enum
{
  ELM_SYSTRAY_CATEGORY_APP_STATUS = 0, /**< Indicators of application status */
  ELM_SYSTRAY_CATEGORY_COMMUNICATIONS, /**< Communications apps */
  ELM_SYSTRAY_CATEGORY_SYS_SERVICES, /**< System Service apps */
  ELM_SYSTRAY_CATEGORY_HARDWARE, /**< Hardware indicators */
  ELM_SYSTRAY_CATEGORY_OTHER /**< Undefined category */
} Elm_Systray_Category;

/** Application status information.
 *
 * @since 1.8
 *
 * @ingroup Elm_Systray
 */
typedef enum
{
  ELM_SYSTRAY_STATUS_PASSIVE = 0, /**< Passive (normal) */
  ELM_SYSTRAY_STATUS_ACTIVE, /**< Active */
  ELM_SYSTRAY_STATUS_ATTENTION /**< Needs Attention */
} Elm_Systray_Status;


#endif
/** Elementary systray class
 *
 * @ingroup Elm_Systray
 */
#define ELM_SYSTRAY_CLASS elm_systray_class_get()

EWAPI const Efl_Class *elm_systray_class_get(void);

/**
 * @brief Set the id of the Status Notifier Item.
 *
 * @param[in] obj The object.
 * @param[in] id Status notifier item ID
 *
 * @ingroup Elm_Systray
 */
EOAPI void elm_obj_systray_id_set(Eo *obj, const char *id);

/**
 * @brief Get the id of the Status Notifier Item.
 *
 * @param[in] obj The object.
 *
 * @return Status notifier item ID
 *
 * @ingroup Elm_Systray
 */
EOAPI const char *elm_obj_systray_id_get(const Eo *obj);

/**
 * @brief Set the category of the Status Notifier Item.
 *
 * @param[in] obj The object.
 * @param[in] cat Category
 *
 * @ingroup Elm_Systray
 */
EOAPI void elm_obj_systray_category_set(Eo *obj, Elm_Systray_Category cat);

/**
 * @brief Get the category of the Status Notifier Item.
 *
 * @param[in] obj The object.
 *
 * @return Category
 *
 * @ingroup Elm_Systray
 */
EOAPI Elm_Systray_Category elm_obj_systray_category_get(const Eo *obj);

/**
 * @brief Set the path to the theme where the icons can be found. Set this
 * value to "" to use the default path.
 *
 * @param[in] obj The object.
 * @param[in] icon_theme_path Icon theme path
 *
 * @ingroup Elm_Systray
 */
EOAPI void elm_obj_systray_icon_theme_path_set(Eo *obj, const char *icon_theme_path);

/**
 * @brief Get the path to the icon's theme currently in use.
 *
 * @param[in] obj The object.
 *
 * @return Icon theme path
 *
 * @ingroup Elm_Systray
 */
EOAPI const char *elm_obj_systray_icon_theme_path_get(const Eo *obj);

/**
 * @brief Set the object path of the D-Bus Menu that is to be show when the
 * Status Notifier Item is activated by the user.
 *
 * @param[in] obj The object.
 * @param[in] menu Object path for DBus menu
 *
 * @ingroup Elm_Systray
 */
EOAPI void elm_obj_systray_menu_set(Eo *obj, const Efl_Object *menu);

/**
 * @brief Get the object path of the D-Bus Menu currently in use.
 *
 * @param[in] obj The object.
 *
 * @return Object path for DBus menu
 *
 * @ingroup Elm_Systray
 */
EOAPI const Efl_Object *elm_obj_systray_menu_get(const Eo *obj);

/**
 * @brief Set the name of the attention icon to be used by the Status Notifier
 * Item.
 *
 * @param[in] obj The object.
 * @param[in] att_icon_name Attention icon name
 *
 * @ingroup Elm_Systray
 */
EOAPI void elm_obj_systray_att_icon_name_set(Eo *obj, const char *att_icon_name);

/**
 * @brief Get the name of the attention icon used by the Status Notifier Item.
 *
 * @param[in] obj The object.
 *
 * @return Attention icon name
 *
 * @ingroup Elm_Systray
 */
EOAPI const char *elm_obj_systray_att_icon_name_get(const Eo *obj);

/**
 * @brief Set the status of the Status Notifier Item.
 *
 * @param[in] obj The object.
 * @param[in] st Status
 *
 * @ingroup Elm_Systray
 */
EOAPI void elm_obj_systray_status_set(Eo *obj, Elm_Systray_Status st);

/**
 * @brief Get the status of the Status Notifier Item.
 *
 * @param[in] obj The object.
 *
 * @return Status
 *
 * @ingroup Elm_Systray
 */
EOAPI Elm_Systray_Status elm_obj_systray_status_get(const Eo *obj);

/**
 * @brief Set the name of the icon to be used by the Status Notifier Item.
 *
 * @param[in] obj The object.
 * @param[in] icon_name Status icon name
 *
 * @ingroup Elm_Systray
 */
EOAPI void elm_obj_systray_icon_name_set(Eo *obj, const char *icon_name);

/**
 * @brief Get the name of the icon used by the Status Notifier Item.
 *
 * @param[in] obj The object.
 *
 * @return Status icon name
 *
 * @ingroup Elm_Systray
 */
EOAPI const char *elm_obj_systray_icon_name_get(const Eo *obj);

/**
 * @brief Set the title of the Status Notifier Item.
 *
 * @param[in] obj The object.
 * @param[in] title Title
 *
 * @ingroup Elm_Systray
 */
EOAPI void elm_obj_systray_title_set(Eo *obj, const char *title);

/**
 * @brief Get the title of the Status Notifier Item.
 *
 * @param[in] obj The object.
 *
 * @return Title
 *
 * @ingroup Elm_Systray
 */
EOAPI const char *elm_obj_systray_title_get(const Eo *obj);

/**
 * @brief Register this Status Notifier Item in the System Tray Watcher. This
 * function should only be called after the event #ELM_EVENT_SYSTRAY_READY is
 * emitted.
 *
 * @param[in] obj The object.
 *
 * @return @c true on success, @c false otherwise
 *
 * @ingroup Elm_Systray
 */
EOAPI Eina_Bool elm_obj_systray_register(Eo *obj);

#endif
