/**
 * @ingroup Win
 *
 * @{
 */
#define ELM_OBJ_WIN_CLASS elm_obj_win_class_get()

const Eo_Class *elm_obj_win_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_WIN_BASE_ID;

enum
  {
     ELM_OBJ_WIN_SUB_ID_WIN_CONSTRUCTOR,
     ELM_OBJ_WIN_SUB_ID_RESIZE_OBJECT_ADD,
     ELM_OBJ_WIN_SUB_ID_RESIZE_OBJECT_DEL,
     ELM_OBJ_WIN_SUB_ID_TYPE_GET,
     ELM_OBJ_WIN_SUB_ID_TITLE_SET,
     ELM_OBJ_WIN_SUB_ID_TITLE_GET,
     ELM_OBJ_WIN_SUB_ID_ICON_NAME_SET,
     ELM_OBJ_WIN_SUB_ID_ICON_NAME_GET,
     ELM_OBJ_WIN_SUB_ID_ROLE_SET,
     ELM_OBJ_WIN_SUB_ID_ROLE_GET,
     ELM_OBJ_WIN_SUB_ID_ICON_OBJECT_SET,
     ELM_OBJ_WIN_SUB_ID_ICON_OBJECT_GET,
     ELM_OBJ_WIN_SUB_ID_AUTODEL_SET,
     ELM_OBJ_WIN_SUB_ID_AUTODEL_GET,
     ELM_OBJ_WIN_SUB_ID_ACTIVATE,
     ELM_OBJ_WIN_SUB_ID_LOWER,
     ELM_OBJ_WIN_SUB_ID_RAISE,
     ELM_OBJ_WIN_SUB_ID_CENTER,
     ELM_OBJ_WIN_SUB_ID_BORDERLESS_SET,
     ELM_OBJ_WIN_SUB_ID_BORDERLESS_GET,
     ELM_OBJ_WIN_SUB_ID_SHAPED_SET,
     ELM_OBJ_WIN_SUB_ID_SHAPED_GET,
     ELM_OBJ_WIN_SUB_ID_ALPHA_SET,
     ELM_OBJ_WIN_SUB_ID_ALPHA_GET,
     ELM_OBJ_WIN_SUB_ID_OVERRIDE_SET,
     ELM_OBJ_WIN_SUB_ID_OVERRIDE_GET,
     ELM_OBJ_WIN_SUB_ID_FULLSCREEN_SET,
     ELM_OBJ_WIN_SUB_ID_FULLSCREEN_GET,
     ELM_OBJ_WIN_SUB_ID_MAIN_MENU_GET,
     ELM_OBJ_WIN_SUB_ID_MAXIMIZED_SET,
     ELM_OBJ_WIN_SUB_ID_MAXIMIZED_GET,
     ELM_OBJ_WIN_SUB_ID_ICONIFIED_SET,
     ELM_OBJ_WIN_SUB_ID_ICONIFIED_GET,
     ELM_OBJ_WIN_SUB_ID_WITHDRAWN_SET,
     ELM_OBJ_WIN_SUB_ID_WITHDRAWN_GET,
     ELM_OBJ_WIN_SUB_ID_AVAILABLE_PROFILES_SET,
     ELM_OBJ_WIN_SUB_ID_AVAILABLE_PROFILES_GET,
     ELM_OBJ_WIN_SUB_ID_PROFILE_SET,
     ELM_OBJ_WIN_SUB_ID_PROFILE_GET,
     ELM_OBJ_WIN_SUB_ID_URGENT_SET,
     ELM_OBJ_WIN_SUB_ID_URGENT_GET,
     ELM_OBJ_WIN_SUB_ID_DEMAND_ATTENTION_SET,
     ELM_OBJ_WIN_SUB_ID_DEMAND_ATTENTION_GET,
     ELM_OBJ_WIN_SUB_ID_MODAL_SET,
     ELM_OBJ_WIN_SUB_ID_MODAL_GET,
     ELM_OBJ_WIN_SUB_ID_ASPECT_SET,
     ELM_OBJ_WIN_SUB_ID_ASPECT_GET,
     ELM_OBJ_WIN_SUB_ID_SIZE_BASE_SET,
     ELM_OBJ_WIN_SUB_ID_SIZE_BASE_GET,
     ELM_OBJ_WIN_SUB_ID_SIZE_STEP_SET,
     ELM_OBJ_WIN_SUB_ID_SIZE_STEP_GET,
     ELM_OBJ_WIN_SUB_ID_LAYER_SET,
     ELM_OBJ_WIN_SUB_ID_LAYER_GET,
     ELM_OBJ_WIN_SUB_ID_ROTATION_SET,
     ELM_OBJ_WIN_SUB_ID_ROTATION_WITH_RESIZE_SET,
     ELM_OBJ_WIN_SUB_ID_ROTATION_GET,
     ELM_OBJ_WIN_SUB_ID_STICKY_SET,
     ELM_OBJ_WIN_SUB_ID_STICKY_GET,
     ELM_OBJ_WIN_SUB_ID_KEYBOARD_MODE_SET,
     ELM_OBJ_WIN_SUB_ID_KEYBOARD_MODE_GET,
     ELM_OBJ_WIN_SUB_ID_KEYBOARD_WIN_SET,
     ELM_OBJ_WIN_SUB_ID_KEYBOARD_WIN_GET,
     ELM_OBJ_WIN_SUB_ID_INDICATOR_MODE_SET,
     ELM_OBJ_WIN_SUB_ID_INDICATOR_MODE_GET,
     ELM_OBJ_WIN_SUB_ID_INDICATOR_OPACITY_SET,
     ELM_OBJ_WIN_SUB_ID_INDICATOR_OPACITY_GET,
     ELM_OBJ_WIN_SUB_ID_SCREEN_POSITION_GET,
     ELM_OBJ_WIN_SUB_ID_FOCUS_GET,
     ELM_OBJ_WIN_SUB_ID_SCREEN_CONSTRAIN_SET,
     ELM_OBJ_WIN_SUB_ID_SCREEN_CONSTRAIN_GET,
     ELM_OBJ_WIN_SUB_ID_SCREEN_SIZE_GET,
     ELM_OBJ_WIN_SUB_ID_SCREEN_DPI_GET,
     ELM_OBJ_WIN_SUB_ID_CONFORMANT_SET,
     ELM_OBJ_WIN_SUB_ID_CONFORMANT_GET,
     ELM_OBJ_WIN_SUB_ID_QUICKPANEL_SET,
     ELM_OBJ_WIN_SUB_ID_QUICKPANEL_GET,
     ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MAJOR_SET,
     ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MAJOR_GET,
     ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MINOR_SET,
     ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MINOR_GET,
     ELM_OBJ_WIN_SUB_ID_QUICKPANEL_ZONE_SET,
     ELM_OBJ_WIN_SUB_ID_QUICKPANEL_ZONE_GET,
     ELM_OBJ_WIN_SUB_ID_PROP_FOCUS_SKIP_SET,
     ELM_OBJ_WIN_SUB_ID_ILLUME_COMMAND_SEND,
     ELM_OBJ_WIN_SUB_ID_INLINED_IMAGE_OBJECT_GET,
     ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ENABLED_SET,
     ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ENABLED_GET,
     ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_STYLE_SET,
     ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_STYLE_GET,
     ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ANIMATE_SET,
     ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ANIMATE_GET,
     ELM_OBJ_WIN_SUB_ID_SOCKET_LISTEN,
     ELM_OBJ_WIN_SUB_ID_XWINDOW_GET,
     ELM_OBJ_WIN_SUB_ID_WL_WINDOW_GET,
     ELM_OBJ_WIN_SUB_ID_WINDOW_ID_GET,
     ELM_OBJ_WIN_SUB_ID_WM_ROTATION_SUPPORTED_GET,
     ELM_OBJ_WIN_SUB_ID_WM_PREFERRED_ROTATION_SET,
     ELM_OBJ_WIN_SUB_ID_WM_PREFERRED_ROTATION_GET,
     ELM_OBJ_WIN_SUB_ID_WM_AVAILABLE_ROTATIONS_SET,
     ELM_OBJ_WIN_SUB_ID_WM_AVAILABLE_ROTATIONS_GET,
     ELM_OBJ_WIN_SUB_ID_WM_MANUAL_ROTATION_DONE_SET,
     ELM_OBJ_WIN_SUB_ID_WM_MANUAL_ROTATION_DONE_GET,
     ELM_OBJ_WIN_SUB_ID_WM_MANUAL_ROTATION_DONE,
     ELM_OBJ_WIN_SUB_ID_LAST
  };

#define ELM_OBJ_WIN_ID(sub_id) (ELM_OBJ_WIN_BASE_ID + sub_id)


/**
 * @def elm_obj_win_constructor
 * @since 1.8
 *
 * No description supplied by the EAPI.
 *
 * @param[in] name
 * @param[in] type
 *
 */
#define elm_obj_win_constructor(name, type) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WIN_CONSTRUCTOR), EO_TYPECHECK(const char *, name), EO_TYPECHECK(Elm_Win_Type, type)

/**
 * @def elm_obj_win_resize_object_add
 * @since 1.8
 *
 * Add subobj as a resize object of window @p obj.
 *
 * @param[in] subobj
 *
 * @see elm_win_resize_object_add
 */
#define elm_obj_win_resize_object_add(subobj) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_RESIZE_OBJECT_ADD), EO_TYPECHECK(Evas_Object *, subobj)

/**
 * @def elm_obj_win_resize_object_del
 * @since 1.8
 *
 * Delete subobj as a resize object of window @p obj.
 *
 * @param[in] subobj
 *
 * @see elm_win_resize_object_del
 */
#define elm_obj_win_resize_object_del(subobj) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_RESIZE_OBJECT_DEL), EO_TYPECHECK(Evas_Object *, subobj)

/**
 * @def elm_obj_win_type_get
 * @since 1.9
 *
 * Get the type of the window
 *
 * @param[out] ret
 *
 * @see elm_win_type_get
 */
#define elm_obj_win_type_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_TYPE_GET), EO_TYPECHECK(Elm_Win_Type *, ret)

/**
 * @def elm_obj_win_title_set
 * @since 1.8
 *
 * Set the title of the window
 *
 * @param[in] title
 *
 * @see elm_win_title_set
 */
#define elm_obj_win_title_set(title) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_TITLE_SET), EO_TYPECHECK(const char *, title)

/**
 * @def elm_obj_win_title_get
 * @since 1.8
 *
 * Get the title of the window
 *
 * @param[out] ret
 *
 * @see elm_win_title_get
 */
#define elm_obj_win_title_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_TITLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_win_icon_name_set
 * @since 1.8
 *
 * Set the icon name of the window
 *
 * @param[in] icon_name
 *
 * @see elm_win_icon_name_set
 */
#define elm_obj_win_icon_name_set(icon_name) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICON_NAME_SET), EO_TYPECHECK(const char *, icon_name)

/**
 * @def elm_obj_win_icon_name_get
 * @since 1.8
 *
 * Get the icon name of the window
 *
 * @param[out] ret
 *
 * @see elm_win_icon_name_get
 */
#define elm_obj_win_icon_name_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICON_NAME_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_win_role_set
 * @since 1.8
 *
 * Set the role of the window
 *
 * @param[in] role
 *
 * @see elm_win_role_set
 */
#define elm_obj_win_role_set(role) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROLE_SET), EO_TYPECHECK(const char *, role)

/**
 * @def elm_obj_win_role_get
 * @since 1.8
 *
 * Get the role of the window
 *
 * @param[out] ret
 *
 * @see elm_win_role_get
 */
#define elm_obj_win_role_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_win_icon_object_set
 * @since 1.8
 *
 * Set a window object's icon
 *
 * @param[in] icon
 *
 * @see elm_win_icon_object_set
 */
#define elm_obj_win_icon_object_set(icon) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICON_OBJECT_SET), EO_TYPECHECK(Evas_Object *, icon)

/**
 * @def elm_obj_win_icon_object_get
 * @since 1.8
 *
 * Get the icon object used for the window
 *
 * @param[out] ret
 *
 * @see elm_win_icon_object_get
 */
#define elm_obj_win_icon_object_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICON_OBJECT_GET), EO_TYPECHECK(const Evas_Object **, ret)

/**
 * @def elm_obj_win_autodel_set
 * @since 1.8
 *
 * Set the window's autodel state.
 *
 * @param[in] autodel
 *
 * @see elm_win_autodel_set
 */
#define elm_obj_win_autodel_set(autodel) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_AUTODEL_SET), EO_TYPECHECK(Eina_Bool, autodel)

/**
 * @def elm_obj_win_autodel_get
 * @since 1.8
 *
 * Get the window's autodel state.
 *
 * @param[out] ret
 *
 * @see elm_win_autodel_get
 */
#define elm_obj_win_autodel_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_AUTODEL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_activate
 * @since 1.8
 *
 * Activate a window object.
 *
 *
 * @see elm_win_activate
 */
#define elm_obj_win_activate() ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ACTIVATE)

/**
 * @def elm_obj_win_lower
 * @since 1.8
 *
 * Lower a window object.
 *
 *
 * @see elm_win_lower
 */
#define elm_obj_win_lower() ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_LOWER)

/**
 * @def elm_obj_win_raise
 * @since 1.8
 *
 * Raise a window object.
 *
 *
 * @see elm_win_raise
 */
#define elm_obj_win_raise() ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_RAISE)

/**
 * @def elm_obj_win_center
 * @since 1.8
 *
 * Center a window on its screen
 *
 * @param[in] h
 * @param[in] v
 *
 * @see elm_win_center
 */
#define elm_obj_win_center(h, v) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_CENTER), EO_TYPECHECK(Eina_Bool, h), EO_TYPECHECK(Eina_Bool, v)

/**
 * @def elm_obj_win_borderless_set
 * @since 1.8
 *
 * Set the borderless state of a window.
 *
 * @param[in] borderless
 *
 * @see elm_win_borderless_set
 */
#define elm_obj_win_borderless_set(borderless) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_BORDERLESS_SET), EO_TYPECHECK(Eina_Bool, borderless)

/**
 * @def elm_obj_win_borderless_get
 * @since 1.8
 *
 * Get the borderless state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_borderless_get
 */
#define elm_obj_win_borderless_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_BORDERLESS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_shaped_set
 * @since 1.8
 *
 * Set the shaped state of a window.
 *
 * @param[in] shaped
 *
 * @see elm_win_shaped_set
 */
#define elm_obj_win_shaped_set(shaped) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SHAPED_SET), EO_TYPECHECK(Eina_Bool, shaped)

/**
 * @def elm_obj_win_shaped_get
 * @since 1.8
 *
 * Get the shaped state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_shaped_get
 */
#define elm_obj_win_shaped_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SHAPED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_alpha_set
 * @since 1.8
 *
 * Set the alpha channel state of a window.
 *
 * @param[in] alpha
 *
 * @see elm_win_alpha_set
 */
#define elm_obj_win_alpha_set(alpha) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ALPHA_SET), EO_TYPECHECK(Eina_Bool, alpha)

/**
 * @def elm_obj_win_alpha_get
 * @since 1.8
 *
 * Get the alpha channel state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_alpha_get
 */
#define elm_obj_win_alpha_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ALPHA_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_override_set
 * @since 1.8
 *
 * Set the override state of a window.
 *
 * @param[in] override
 *
 * @see elm_win_override_set
 */
#define elm_obj_win_override_set(override) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_OVERRIDE_SET), EO_TYPECHECK(Eina_Bool, override)

/**
 * @def elm_obj_win_override_get
 * @since 1.8
 *
 * Get the override state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_override_get
 */
#define elm_obj_win_override_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_OVERRIDE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_fullscreen_set
 * @since 1.8
 *
 * Set the fullscreen state of a window.
 *
 * @param[in] fullscreen
 *
 * @see elm_win_fullscreen_set
 */
#define elm_obj_win_fullscreen_set(fullscreen) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FULLSCREEN_SET), EO_TYPECHECK(Eina_Bool, fullscreen)

/**
 * @def elm_obj_win_fullscreen_get
 * @since 1.8
 *
 * Get the fullscreen state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_fullscreen_get
 */
#define elm_obj_win_fullscreen_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FULLSCREEN_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_main_menu_get
 * @since 1.8
 *
 * Get the Main Menu of a window.
 *
 * @param[out] ret Main menu.
 *
 * @see elm_win_main_menu_get
 */
#define elm_obj_win_main_menu_get(ret)                    \
        ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_MAIN_MENU_GET), \
        EO_TYPECHECK(Eo **, ret)

/**
 * @def elm_obj_win_maximized_set
 * @since 1.8
 *
 * Set the maximized state of a window.
 *
 * @param[in] maximized
 *
 * @see elm_win_maximized_set
 */
#define elm_obj_win_maximized_set(maximized) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_MAXIMIZED_SET), EO_TYPECHECK(Eina_Bool, maximized)

/**
 * @def elm_obj_win_maximized_get
 * @since 1.8
 *
 * Get the maximized state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_maximized_get
 */
#define elm_obj_win_maximized_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_MAXIMIZED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_iconified_set
 * @since 1.8
 *
 * Set the iconified state of a window.
 *
 * @param[in] iconified
 *
 * @see elm_win_iconified_set
 */
#define elm_obj_win_iconified_set(iconified) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICONIFIED_SET), EO_TYPECHECK(Eina_Bool, iconified)

/**
 * @def elm_obj_win_iconified_get
 * @since 1.8
 *
 * Get the iconified state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_iconified_get
 */
#define elm_obj_win_iconified_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICONIFIED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_withdrawn_set
 * @since 1.8
 *
 * Set the withdrawn state of a window.
 *
 * @param[in] withdrawn
 *
 * @see elm_win_withdrawn_set
 */
#define elm_obj_win_withdrawn_set(withdrawn) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WITHDRAWN_SET), EO_TYPECHECK(Eina_Bool, withdrawn)

/**
 * @def elm_obj_win_withdrawn_get
 * @since 1.8
 *
 * Get the withdrawn state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_withdrawn_get
 */
#define elm_obj_win_withdrawn_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WITHDRAWN_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_available_profiles_set
 * @since 1.8
 *
 * Set the array of available profiles to a window.
 *
 * @param[in] profiles
 * @param[in] count
 *
 * @see elm_win_available_profiles_set
 */
#define elm_obj_win_available_profiles_set(profiles, count) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_AVAILABLE_PROFILES_SET), EO_TYPECHECK(const char **, profiles), EO_TYPECHECK(unsigned int, count)

/**
 * @def elm_obj_win_available_profiles_get
 * @since 1.8
 *
 * Get the array of available profiles of a window.
 *
 * @param[out] profiles
 * @param[out] count
 * @param[out] ret
 *
 * @see elm_win_available_profiles_get
 */
#define elm_obj_win_available_profiles_get(profiles, count, ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_AVAILABLE_PROFILES_GET), EO_TYPECHECK(char ***, profiles), EO_TYPECHECK(unsigned int *, count), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_profile_set
 * @since 1.8
 *
 * Set the profile of a window.
 *
 * @param[in] profile
 *
 * @see elm_win_profile_set
 */
#define elm_obj_win_profile_set(profile) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_PROFILE_SET), EO_TYPECHECK(const char *, profile)

/**
 * @def elm_obj_win_profile_get
 * @since 1.8
 *
 * Get the profile of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_profile_get
 */
#define elm_obj_win_profile_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_PROFILE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_win_urgent_set
 * @since 1.8
 *
 * Set the urgent state of a window.
 *
 * @param[in] urgent
 *
 * @see elm_win_urgent_set
 */
#define elm_obj_win_urgent_set(urgent) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_URGENT_SET), EO_TYPECHECK(Eina_Bool, urgent)

/**
 * @def elm_obj_win_urgent_get
 * @since 1.8
 *
 * Get the urgent state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_urgent_get
 */
#define elm_obj_win_urgent_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_URGENT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_demand_attention_set
 * @since 1.8
 *
 * Set the demand_attention state of a window.
 *
 * @param[in] demand_attention
 *
 * @see elm_win_demand_attention_set
 */
#define elm_obj_win_demand_attention_set(demand_attention) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_DEMAND_ATTENTION_SET), EO_TYPECHECK(Eina_Bool, demand_attention)

/**
 * @def elm_obj_win_demand_attention_get
 * @since 1.8
 *
 * Get the demand_attention state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_demand_attention_get
 */
#define elm_obj_win_demand_attention_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_DEMAND_ATTENTION_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_modal_set
 * @since 1.8
 *
 * Set the modal state of a window.
 *
 * @param[in] modal
 *
 * @see elm_win_modal_set
 */
#define elm_obj_win_modal_set(modal) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_MODAL_SET), EO_TYPECHECK(Eina_Bool, modal)

/**
 * @def elm_obj_win_modal_get
 * @since 1.8
 *
 * Get the modal state of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_modal_get
 */
#define elm_obj_win_modal_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_MODAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_aspect_set
 * @since 1.8
 *
 * Set the aspect ratio of a window.
 *
 * @param[in] aspect
 *
 * @see elm_win_aspect_set
 */
#define elm_obj_win_aspect_set(aspect) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ASPECT_SET), EO_TYPECHECK(double, aspect)

/**
 * @def elm_obj_win_aspect_get
 * @since 1.8
 *
 * Get the aspect ratio of a window.
 *
 * @param[out] ret
 *
 * @see elm_win_aspect_get
 */
#define elm_obj_win_aspect_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ASPECT_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_win_size_base_set
 * @since 1.8
 *
 * Set the base window size used with stepping calculation
 *
 * @param[in] w
 * @param[in] h
 *
 * @see elm_win_size_base_set
 */
#define elm_obj_win_size_base_set(w, h) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SIZE_BASE_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def elm_obj_win_size_base_get
 * @since 1.8
 *
 * Get the base size of a window.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see elm_win_size_base_get
 */
#define elm_obj_win_size_base_get(w, h) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SIZE_BASE_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def elm_obj_win_size_step_set
 * @since 1.8
 *
 * Set the window stepping used with sizing calculation
 *
 * @param[in] w
 * @param[in] h
 *
 * @see elm_win_size_step_set
 */
#define elm_obj_win_size_step_set(w, h) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SIZE_STEP_SET), EO_TYPECHECK(int, w), EO_TYPECHECK(int, h)

/**
 * @def elm_obj_win_size_step_get
 * @since 1.8
 *
 * Get the stepping of a window.
 *
 * @param[out] w
 * @param[out] h
 *
 * @see elm_win_size_step_get
 */
#define elm_obj_win_size_step_get(w, h) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SIZE_STEP_GET), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def elm_obj_win_layer_set
 * @since 1.8
 *
 * Set the layer of the window.
 *
 * @param[in] layer
 *
 * @see elm_win_layer_set
 */
#define elm_obj_win_layer_set(layer) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_LAYER_SET), EO_TYPECHECK(int, layer)

/**
 * @def elm_obj_win_layer_get
 * @since 1.8
 *
 * Get the layer of the window.
 *
 * @param[out] ret
 *
 * @see elm_win_layer_get
 */
#define elm_obj_win_layer_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_LAYER_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_win_rotation_set
 * @since 1.8
 *
 * Set the rotation of the window.
 *
 * @param[in] rotation
 *
 * @see elm_win_rotation_set
 */
#define elm_obj_win_rotation_set(rotation) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROTATION_SET), EO_TYPECHECK(int, rotation)

/**
 * @def elm_obj_win_rotation_with_resize_set
 * @since 1.8
 *
 * Rotates the window and resizes it.
 *
 * @param[in] rotation
 *
 * @see elm_win_rotation_with_resize_set
 */
#define elm_obj_win_rotation_with_resize_set(rotation) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROTATION_WITH_RESIZE_SET), EO_TYPECHECK(int, rotation)

/**
 * @def elm_obj_win_rotation_get
 * @since 1.8
 *
 * Get the rotation of the window.
 *
 * @param[out] ret
 *
 * @see elm_win_rotation_get
 */
#define elm_obj_win_rotation_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROTATION_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_win_sticky_set
 * @since 1.8
 *
 * Set the sticky state of the window.
 *
 * @param[in] sticky
 *
 * @see elm_win_sticky_set
 */
#define elm_obj_win_sticky_set(sticky) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_STICKY_SET), EO_TYPECHECK(Eina_Bool, sticky)

/**
 * @def elm_obj_win_sticky_get
 * @since 1.8
 *
 * Get the sticky state of the window.
 *
 * @param[out] ret
 *
 * @see elm_win_sticky_get
 */
#define elm_obj_win_sticky_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_STICKY_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_keyboard_mode_set
 * @since 1.8
 *
 * Sets the keyboard mode of the window.
 *
 * @param[in] mode
 *
 * @see elm_win_keyboard_mode_set
 */
#define elm_obj_win_keyboard_mode_set(mode) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_KEYBOARD_MODE_SET), EO_TYPECHECK(Elm_Win_Keyboard_Mode, mode)

/**
 * @def elm_obj_win_keyboard_mode_get
 * @since 1.8
 *
 * Get the keyboard mode of the window.
 *
 * @param[out] ret
 *
 * @see elm_win_keyboard_mode_get
 */
#define elm_obj_win_keyboard_mode_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_KEYBOARD_MODE_GET), EO_TYPECHECK(Elm_Win_Keyboard_Mode *, ret)

/**
 * @def elm_obj_win_keyboard_win_set
 * @since 1.8
 *
 * Sets whether the window is a keyboard.
 *
 * @param[in] is_keyboard
 *
 * @see elm_win_keyboard_win_set
 */
#define elm_obj_win_keyboard_win_set(is_keyboard) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_KEYBOARD_WIN_SET), EO_TYPECHECK(Eina_Bool, is_keyboard)

/**
 * @def elm_obj_win_keyboard_win_get
 * @since 1.8
 *
 * Get whether the window is a keyboard.
 *
 * @param[out] ret
 *
 * @see elm_win_keyboard_win_get
 */
#define elm_obj_win_keyboard_win_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_KEYBOARD_WIN_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_indicator_mode_set
 * @since 1.8
 *
 * Sets the indicator mode of the window.
 *
 * @param[in] mode
 *
 * @see elm_win_indicator_mode_set
 */
#define elm_obj_win_indicator_mode_set(mode) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INDICATOR_MODE_SET), EO_TYPECHECK(Elm_Win_Indicator_Mode, mode)

/**
 * @def elm_obj_win_indicator_mode_get
 * @since 1.8
 *
 * Get the indicator mode of the window.
 *
 * @param[out] ret
 *
 * @see elm_win_indicator_mode_get
 */
#define elm_obj_win_indicator_mode_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INDICATOR_MODE_GET), EO_TYPECHECK(Elm_Win_Indicator_Mode *, ret)

/**
 * @def elm_obj_win_indicator_opacity_set
 * @since 1.8
 *
 * Sets the indicator opacity mode of the window.
 *
 * @param[in] mode
 *
 * @see elm_win_indicator_opacity_set
 */
#define elm_obj_win_indicator_opacity_set(mode) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INDICATOR_OPACITY_SET), EO_TYPECHECK(Elm_Win_Indicator_Opacity_Mode, mode)

/**
 * @def elm_obj_win_indicator_opacity_get
 * @since 1.8
 *
 * Get the indicator opacity mode of the window.
 *
 * @param[out] ret
 *
 * @see elm_win_indicator_opacity_get
 */
#define elm_obj_win_indicator_opacity_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INDICATOR_OPACITY_GET), EO_TYPECHECK(Elm_Win_Indicator_Opacity_Mode *, ret)

/**
 * @def elm_obj_win_screen_position_get
 * @since 1.8
 *
 * Get the screen position of a window.
 *
 * @param[out] x
 * @param[out] y
 *
 * @see elm_win_screen_position_get
 */
#define elm_obj_win_screen_position_get(x, y) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_POSITION_GET), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y)

/**
 * @def elm_obj_win_focus_get
 * @since 1.8
 *
 * Determine whether a window has focus
 *
 * @param[out] ret
 *
 * @see elm_win_focus_get
 */
#define elm_obj_win_focus_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_screen_constrain_set
 * @since 1.8
 *
 * Constrain the maximum width and height of a window to the width and height of its screen
 *
 * @param[in] constrain
 *
 * @see elm_win_screen_constrain_set
 */
#define elm_obj_win_screen_constrain_set(constrain) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_CONSTRAIN_SET), EO_TYPECHECK(Eina_Bool, constrain)

/**
 * @def elm_obj_win_screen_constrain_get
 * @since 1.8
 *
 * Retrieve the constraints on the maximum width and height of a window relative to the width and height of its screen
 *
 * @param[out] ret
 *
 * @see elm_win_screen_constrain_get
 */
#define elm_obj_win_screen_constrain_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_CONSTRAIN_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_screen_size_get
 * @since 1.8
 *
 * Get screen geometry details for the screen that a window is on
 *
 * @param[out] x
 * @param[out] y
 * @param[out] w
 * @param[out] h
 *
 * @see elm_win_screen_size_get
 */
#define elm_obj_win_screen_size_get(x, y, w, h) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_SIZE_GET), EO_TYPECHECK(int *, x), EO_TYPECHECK(int *, y), EO_TYPECHECK(int *, w), EO_TYPECHECK(int *, h)

/**
 * @def elm_obj_win_screen_dpi_get
 * @since 1.8
 *
 * Get screen dpi for the screen that a window is on
 *
 * @param[out] xdpi
 * @param[out] ydpi
 *
 * @see elm_win_screen_dpi_get
 */
#define elm_obj_win_screen_dpi_get(xdpi, ydpi) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_DPI_GET), EO_TYPECHECK(int *, xdpi), EO_TYPECHECK(int *, ydpi)

/**
 * @def elm_obj_win_conformant_set
 * @since 1.8
 *
 * Set if this window is an illume conformant window
 *
 * @param[in] conformant
 *
 * @see elm_win_conformant_set
 */
#define elm_obj_win_conformant_set(conformant) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_CONFORMANT_SET), EO_TYPECHECK(Eina_Bool, conformant)

/**
 * @def elm_obj_win_conformant_get
 * @since 1.8
 *
 * Get if this window is an illume conformant window
 *
 * @param[out] ret
 *
 * @see elm_win_conformant_get
 */
#define elm_obj_win_conformant_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_CONFORMANT_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_quickpanel_set
 * @since 1.8
 *
 * Set a window to be an illume quickpanel window
 *
 * @param[in] quickpanel
 *
 * @see elm_win_quickpanel_set
 */
#define elm_obj_win_quickpanel_set(quickpanel) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_SET), EO_TYPECHECK(Eina_Bool, quickpanel)

/**
 * @def elm_obj_win_quickpanel_get
 * @since 1.8
 *
 * Get if this window is a quickpanel or not
 *
 * @param[out] ret
 *
 * @see elm_win_quickpanel_get
 */
#define elm_obj_win_quickpanel_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_quickpanel_priority_major_set
 * @since 1.8
 *
 * Set the major priority of a quickpanel window
 *
 * @param[in] priority
 *
 * @see elm_win_quickpanel_priority_major_set
 */
#define elm_obj_win_quickpanel_priority_major_set(priority) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MAJOR_SET), EO_TYPECHECK(int, priority)

/**
 * @def elm_obj_win_quickpanel_priority_major_get
 * @since 1.8
 *
 * Get the major priority of a quickpanel window
 *
 * @param[out] ret
 *
 * @see elm_win_quickpanel_priority_major_get
 */
#define elm_obj_win_quickpanel_priority_major_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MAJOR_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_win_quickpanel_priority_minor_set
 * @since 1.8
 *
 * Set the minor priority of a quickpanel window
 *
 * @param[in] priority
 *
 * @see elm_win_quickpanel_priority_minor_set
 */
#define elm_obj_win_quickpanel_priority_minor_set(priority) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MINOR_SET), EO_TYPECHECK(int, priority)

/**
 * @def elm_obj_win_quickpanel_priority_minor_get
 * @since 1.8
 *
 * Get the minor priority of a quickpanel window
 *
 * @param[out] ret
 *
 * @see elm_win_quickpanel_priority_minor_get
 */
#define elm_obj_win_quickpanel_priority_minor_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MINOR_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_win_quickpanel_zone_set
 * @since 1.8
 *
 * Set which zone this quickpanel should appear in
 *
 * @param[in] zone
 *
 * @see elm_win_quickpanel_zone_set
 */
#define elm_obj_win_quickpanel_zone_set(zone) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_ZONE_SET), EO_TYPECHECK(int, zone)

/**
 * @def elm_obj_win_quickpanel_zone_get
 * @since 1.8
 *
 * Get which zone this quickpanel should appear in
 *
 * @param[out] ret
 *
 * @see elm_win_quickpanel_zone_get
 */
#define elm_obj_win_quickpanel_zone_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_ZONE_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_win_prop_focus_skip_set
 * @since 1.8
 *
 * Set the window to be skipped by keyboard focus
 *
 * @param[in] skip
 *
 * @see elm_win_prop_focus_skip_set
 */
#define elm_obj_win_prop_focus_skip_set(skip) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_PROP_FOCUS_SKIP_SET), EO_TYPECHECK(Eina_Bool, skip)

/**
 * @def elm_obj_win_illume_command_send
 * @since 1.8
 *
 * Send a command to the windowing environment
 *
 * @param[in] command
 * @param[in] params
 *
 * @see elm_win_illume_command_send
 */
#define elm_obj_win_illume_command_send(command, params) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ILLUME_COMMAND_SEND), EO_TYPECHECK(Elm_Illume_Command, command), EO_TYPECHECK(void *, params)

/**
 * @def elm_obj_win_inlined_image_object_get
 * @since 1.8
 *
 * Get the inlined image object handle
 *
 * @param[out] ret
 *
 * @see elm_win_inlined_image_object_get
 */
#define elm_obj_win_inlined_image_object_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INLINED_IMAGE_OBJECT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_win_focus_highlight_enabled_set
 * @since 1.8
 *
 * Set the enabled status for the focus highlight in a window
 *
 * @param[in] enabled
 *
 * @see elm_win_focus_highlight_enabled_set
 */
#define elm_obj_win_focus_highlight_enabled_set(enabled) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ENABLED_SET), EO_TYPECHECK(Eina_Bool, enabled)

/**
 * @def elm_obj_win_focus_highlight_enabled_get
 * @since 1.8
 *
 * Get the enabled value of the focus highlight for this window
 *
 * @param[out] ret
 *
 * @see elm_win_focus_highlight_enabled_get
 */
#define elm_obj_win_focus_highlight_enabled_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ENABLED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_focus_highlight_style_set
 * @since 1.8
 *
 * Set the style for the focus highlight on this window
 *
 * @param[in] style
 *
 * @see elm_win_focus_highlight_style_set
 */
#define elm_obj_win_focus_highlight_style_set(style) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_STYLE_SET), EO_TYPECHECK(const char *, style)

/**
 * @def elm_obj_win_focus_highlight_style_get
 * @since 1.8
 *
 * Get the style set for the focus highlight object
 *
 * @param[out] ret
 *
 * @see elm_win_focus_highlight_style_get
 */
#define elm_obj_win_focus_highlight_style_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_STYLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_obj_win_focus_highlight_animate_set
 * @since 1.8
 *
 * Set the animate status for the focus highlight for this window.
 *
 * @param[in] animate
 *
 * @see elm_win_focus_highlight_animate_get
 */
#define elm_obj_win_focus_highlight_animate_set(animate) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ANIMATE_SET), EO_TYPECHECK(Eina_Bool, animate)

/**
 * @def elm_obj_win_focus_highlight_animate_get
 * @since 1.8
 *
 * Get the animate status for the focus highlight for this window.
 *
 * @param[out] ret
 *
 * @see elm_win_focus_highlight_animate_get
 */
#define elm_obj_win_focus_highlight_animate_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ANIMATE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_socket_listen
 * @since 1.8
 *
 * Create a socket to provide the service for Plug widget
 *
 * @param[in] svcname
 * @param[in] svcnum
 * @param[in] svcsys
 * @param[out] ret
 *
 * @see elm_win_socket_listen
 */
#define elm_obj_win_socket_listen(svcname, svcnum, svcsys, ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SOCKET_LISTEN), EO_TYPECHECK(const char *, svcname), EO_TYPECHECK(int, svcnum), EO_TYPECHECK(Eina_Bool, svcsys), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_xwindow_get
 * @since 1.8
 *
 * Get the Ecore_X_Window of an Evas_Object
 *
 * @param[out] ret
 *
 * @see elm_win_xwindow_get
 */
#define elm_obj_win_xwindow_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_XWINDOW_GET), EO_TYPECHECK(Ecore_X_Window *, ret)

/**
 * @def elm_obj_win_wl_window_get
 * @since 1.8
 *
 * Get the Ecore_Wl_Window of an Evas_Object
 *
 * @param[out] ret
 *
 * @see elm_win_wl_window_get
 */
#define elm_obj_win_wl_window_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WL_WINDOW_GET), EO_TYPECHECK(Ecore_Wl_Window **, ret)

/**
 * @def elm_obj_win_window_id_get
 * @since 1.8
 *
 * Get the Ecore_Window of an Evas_Object
 *
 * @param[out] ret
 *
 * @see elm_win_window_id_get
 */
#define elm_obj_win_window_id_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WINDOW_ID_GET), EO_TYPECHECK(Ecore_Window *, ret)

/**
 * @def elm_obj_win_wm_rotation_supported_get
 * @since 1.9
 *
 * Query whether window manager supports window rotation or not.
 *
 * @param[out] ret
 *
 * @see elm_win_wm_rotation_supported_get
 */
#define elm_obj_win_wm_rotation_supported_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WM_ROTATION_SUPPORTED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_wm_preferred_rotation_set
 * @since 1.9
 *
 * Set the preferred rotation value.
 *
 * @param[in] rotation
 *
 * @see elm_win_wm_rotation_preferred_rotation_set
 */
#define elm_obj_win_wm_preferred_rotation_set(rotation) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WM_PREFERRED_ROTATION_SET), EO_TYPECHECK(int, rotation)

/**
 * @def elm_obj_win_wm_preferred_rotation_get
 * @since 1.9
 *
 * Get the preferred rotation value.
 *
 * @param[out] ret
 *
 * @see elm_win_wm_rotation_preferred_rotation_get
 */
#define elm_obj_win_wm_preferred_rotation_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WM_PREFERRED_ROTATION_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_obj_win_wm_available_rotations_set
 * @since 1.9
 *
 * Set the array of available rotations.
 *
 * @param[in] rotations
 * @param[in] count
 *
 * @see elm_win_wm_rotation_available_rotations_set
 */
#define elm_obj_win_wm_available_rotations_set(rotation, count) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WM_AVAILABLE_ROTATIONS_SET), EO_TYPECHECK(const int *, rotations), EO_TYPECHECK(unsigned int, count)

/**
 * @def elm_obj_win_wm_available_rotations_get
 * @since 1.9
 *
 * Get the array of available window rotation value.
 *
 * @param[out] rotations
 * @param[out] count
 * @param[out] ret
 *
 * @see elm_win_wm_rotation_available_rotations_get
 */
#define elm_obj_win_wm_available_rotations_get(rotations, count, ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WM_AVAILABLE_ROTATIONS_GET), EO_TYPECHECK(int **, rotations), EO_TYPECHECK(unsigned int *, count), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_wm_manual_rotation_done_set
 * @since 1.9
 *
 * Set manual rotation done mode
 *
 * @param[in] set
 *
 * @see elm_win_wm_rotation_manual_rotation_done_set
 */
#define elm_obj_win_wm_manual_rotation_done_set(set) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WM_MANUAL_ROTATION_DONE_SET), EO_TYPECHECK(Eina_Bool, set)

/**
 * @def elm_obj_win_wm_manual_rotation_done_get
 * @since 1.9
 *
 * Get manual rotation done mode state
 *
 * @param[out] ret
 *
 * @see elm_win_wm_rotation_manual_rotation_done_get
 */
#define elm_obj_win_wm_manual_rotation_done_get(ret) ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WM_MANUAL_ROTATION_DONE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_win_wm_manual_rotation_done
 * @since 1.9
 *
 * Set rotation finish manually
 *
 * @see elm_win_wm_rotation_manual_rotation_done
 */
#define elm_obj_win_wm_manual_rotation_done() ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WM_MANUAL_ROTATION_DONE)
