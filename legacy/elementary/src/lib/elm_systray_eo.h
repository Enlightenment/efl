/**
 * @ingroup Systray
 *
 * @{
 */
#define ELM_OBJ_SYSTRAY_CLASS elm_obj_systray_class_get()

const Eo_Class *elm_obj_systray_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_SYSTRAY_BASE_ID;

enum
{
   ELM_OBJ_SYSTRAY_SUB_ID_CATEGORY_SET,
   ELM_OBJ_SYSTRAY_SUB_ID_CATEGORY_GET,
   ELM_OBJ_SYSTRAY_SUB_ID_STATUS_SET,
   ELM_OBJ_SYSTRAY_SUB_ID_STATUS_GET,
   ELM_OBJ_SYSTRAY_SUB_ID_ATT_ICON_NAME_SET,
   ELM_OBJ_SYSTRAY_SUB_ID_ATT_ICON_NAME_GET,
   ELM_OBJ_SYSTRAY_SUB_ID_ICON_NAME_SET,
   ELM_OBJ_SYSTRAY_SUB_ID_ICON_NAME_GET,
   ELM_OBJ_SYSTRAY_SUB_ID_ICON_THEME_PATH_SET,
   ELM_OBJ_SYSTRAY_SUB_ID_ICON_THEME_PATH_GET,
   ELM_OBJ_SYSTRAY_SUB_ID_ID_SET,
   ELM_OBJ_SYSTRAY_SUB_ID_ID_GET,
   ELM_OBJ_SYSTRAY_SUB_ID_TITLE_SET,
   ELM_OBJ_SYSTRAY_SUB_ID_TITLE_GET,
   ELM_OBJ_SYSTRAY_SUB_ID_MENU_SET,
   ELM_OBJ_SYSTRAY_SUB_ID_MENU_GET,
   ELM_OBJ_SYSTRAY_SUB_ID_REGISTER,
   ELM_OBJ_SYSTRAY_SUB_ID_LAST
};

#define ELM_OBJ_SYSTRAY_ID(sub_id) (ELM_OBJ_SYSTRAY_BASE_ID + sub_id)

/**
 * @def elm_obj_systray_category_set
 *
 * Set the category of the Status Notifier Item.
 *
 * @param[in] cat Category
 *
 * @see Elm_Systray_Category
 *
 * @since 1.8
 */
#define elm_obj_systray_category_set(cat)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_CATEGORY_SET), \
        EO_TYPECHECK(Elm_Systray_Category, cat)

/**
 * @def elm_obj_systray_category_get
 *
 * Retrieve the category of the Status Notifier Item.
 *
 * @param[out] ret Category
 *
 * @see elm_obj_systray_category_set
 *
 * @since 1.8
 */
#define elm_obj_systray_category_get(ret)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_CATEGORY_GET), \
        EO_TYPECHECK(Elm_Systray_Category *, ret)

/**
 * @def elm_obj_systray_status_set
 *
 * Set the status of the Status Notifier Item.
 *
 * @param[in] st Status
 *
 * @see Elm_Systray_Status
 *
 * @since 1.8
 */
#define elm_obj_systray_status_set(st)                         \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_STATUS_SET), \
        EO_TYPECHECK(Elm_Systray_Status, st)

/**
 * @def elm_obj_systray_status_get
 *
 * Retrieve the status of the Status Notifier Item.
 *
 * @param[out] ret Status
 *
 * @see elm_obj_systray_status_set
 *
 * @since 1.8
 */
#define elm_obj_systray_status_get(ret)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_STATUS_GET), \
        EO_TYPECHECK(Elm_Systray_Status *, ret)

/**
 * @def elm_obj_systray_att_icon_name_set
 *
 * Set the name of the attention icon to be used by the Status Notifier Item.
 *
 * @param[in] att_icon_name
 *
 * @see elm_obj_systray_att_icon_name_get
 *
 * @since 1.8
 */
#define elm_obj_systray_att_icon_name_set(att_icon_name)              \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ATT_ICON_NAME_SET), \
        EO_TYPECHECK(const char *, att_icon_name)

/**
 * @def elm_obj_systray_att_icon_name_get
 *
 * Retrieve the name of the attention icon used by the Status Notifier Item.
 *
 * @param[out] ret
 *
 * @see elm_obj_systray_att_icon_name_set
 *
 * @since 1.8
 */
#define elm_obj_systray_att_icon_name_get(ret)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ATT_ICON_NAME_GET), \
        EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_systray_icon_name_set
 *
 * Set the name of the icon to be used by the Status Notifier Item.
 *
 * @param[in] icon_name
 *
 * @see elm_obj_systray_icon_name_get
 *
 * @since 1.8
 */
#define elm_obj_systray_icon_name_set(icon_name)                  \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ICON_NAME_SET), \
        EO_TYPECHECK(const char *, icon_name)

/**
 * @def elm_obj_systray_icon_name_get
 *
 * Retrieve the name of the icon used by the Status Notifier Item.
 *
 * @param[out] ret
 *
 * @see elm_obj_systray_icon_name_set
 *
 * @since 1.8
 */
#define elm_obj_systray_icon_name_get(ret)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ICON_NAME_GET), \
        EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_systray_icon_theme_path_set
 *
 * Set the path to the theme where the icons can be found.
 * Set this value to "" to use the default path.
 *
 * @param[in] icon_theme_path
 *
 * @see elm_obj_systray_icon_theme_path_get
 *
 * @since 1.8
 */
#define elm_obj_systray_icon_theme_path_set(icon_theme_path)            \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ICON_THEME_PATH_SET), \
        EO_TYPECHECK(const char *, icon_theme_path)

/**
 * @def elm_obj_systray_icon_theme_path_get
 *
 * Retrieve the path to the icon's theme currently in use.
 *
 * @param[out] ret
 *
 * @see elm_obj_systray_icon_theme_path_set
 *
 * @since 1.8
 */
#define elm_obj_systray_icon_theme_path_get(ret)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ICON_THEME_PATH_GET), \
        EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_systray_id_set
 *
 * Set the id of the Status Notifier Item.
 *
 * @param[in] id
 *
 * @see elm_obj_systray_id_get
 *
 * @since 1.8
 */
#define elm_obj_systray_id_set(id)                         \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ID_SET), \
        EO_TYPECHECK(const char *, id)

/**
 * @def elm_obj_systray_id_get
 *
 * Retrieve the id of the Status Notifier Item.
 *
 * @param[out] ret
 *
 * @see elm_obj_systray_id_set
 *
 * @since 1.8
 */
#define elm_obj_systray_id_get(ret)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_ID_GET), \
        EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_systray_title_set
 *
 * Set the title of the Status Notifier Item.
 *
 * @param[in] title
 *
 * @see elm_obj_systray_title_get
 *
 * @since 1.8
 */
#define elm_obj_systray_title_set(title)                      \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_TITLE_SET), \
        EO_TYPECHECK(const char *, title)

/**
 * @def elm_obj_systray_title_get
 *
 * Retrieve the title of the Status Notifier Item.
 *
 * @param[out] ret
 *
 * @see elm_obj_systray_title_set
 *
 * @since 1.8
 */
#define elm_obj_systray_title_get(ret)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_TITLE_GET), \
        EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_systray_menu_set
 *
 * Set the object path of the D-Bus Menu that is to be show when the
 * Status Notifier Item is activated by the user.
 *
 * @param[in] menu
 *
 * @see elm_obj_systray_menu_get
 *
 * @since 1.8
 */
#define elm_obj_systray_menu_set(menu)                       \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_MENU_SET), \
        EO_TYPECHECK(const Eo *, menu)

/**
 * @def elm_obj_systray_menu_get
 *
 * Retrieve the object path of the D-Bus Menu currently in use.
 *
 * @param[out] ret
 *
 * @see elm_obj_systray_menu_set
 *
 * @since 1.8
 */
#define elm_obj_systray_menu_get(ret)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_MENU_GET), \
        EO_TYPECHECK(const Eo **, ret)

/**
 * @def elm_obj_systray_register
 *
 * Register this Status Notifier Item in the System Tray Watcher.
 * This function should only be called after the event
 * ELM_EVENT_SYSTRAY_READY is emitted.
 *
 * @param[out] ret
 *
 * @since 1.8
 */
#define elm_obj_systray_register(ret)                        \
        ELM_OBJ_SYSTRAY_ID(ELM_OBJ_SYSTRAY_SUB_ID_REGISTER), \
        EO_TYPECHECK(Eina_Bool *, ret)
/**
 * @}
 */
