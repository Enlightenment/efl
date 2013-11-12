#define ELM_APP_CLIENT_VIEW_CLASS elm_app_client_view_class_get()

const Eo_Class *elm_app_client_view_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_APP_CLIENT_VIEW_BASE_ID;

enum
{
   ELM_APP_CLIENT_VIEW_SUB_ID_CONSTRUCTOR,
   ELM_APP_CLIENT_VIEW_SUB_ID_CLOSE,
   ELM_APP_CLIENT_VIEW_SUB_ID_PAUSE,
   ELM_APP_CLIENT_VIEW_SUB_ID_RESUME,
   ELM_APP_CLIENT_VIEW_SUB_ID_TITLE_GET,
   ELM_APP_CLIENT_VIEW_SUB_ID_ICON_GET,
   ELM_APP_CLIENT_VIEW_SUB_ID_ICON_PIXELS_GET,
   ELM_APP_CLIENT_VIEW_SUB_ID_PROGRESS_GET,
   ELM_APP_CLIENT_VIEW_SUB_ID_NEW_EVENTS_GET,
   ELM_APP_CLIENT_VIEW_SUB_ID_STATE_GET,
   ELM_APP_CLIENT_VIEW_SUB_ID_WINDOW_GET,
   ELM_APP_CLIENT_VIEW_SUB_ID_PATH_GET,
   ELM_APP_CLIENT_VIEW_SUB_ID_PACKAGE_GET,
   ELM_APP_CLIENT_VIEW_SUB_ID_LAST
};

typedef void *(*Elm_App_Client_View_Cb)(void *data, Elm_App_Client_View *view, const char *error, const char *error_message);

#define ELM_APP_CLIENT_VIEW_ID(sub_id) (ELM_APP_CLIENT_VIEW_BASE_ID + sub_id)

/**
 * @def elm_app_client_view_constructor
 * @since 1.8
 *
 * Class constructor of elm_app_client_view
 *
 * @param[out] path DBus path of view
 *
 * @note You must supply elm_app_client as parent.
 */
#define elm_app_client_view_constructor(path) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(const char *, path)

/**
 * @def elm_app_client_view_close
 * @since 1.8
 *
 * Close view
 *
 * @param[in] cb callback to be called when view was closed
 * @param[in] data callback user data
 */
#define elm_app_client_view_close(cb, data) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_CLOSE), EO_TYPECHECK(Elm_App_Client_View_Cb, cb), EO_TYPECHECK(const void *, data)

/**
 * @def elm_app_client_view_pause
 * @since 1.8
 *
 * Pause view
 *
 * @param[in] cb callback to be called when view was paused
 * @param[in] data callback user data
 */
#define elm_app_client_view_pause(cb, data) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_PAUSE), EO_TYPECHECK(Elm_App_Client_View_Cb, cb), EO_TYPECHECK(const void *, data)

/**
 * @def elm_app_client_view_resume
 * @since 1.8
 *
 * Resume view
 *
 * @param[in] cb callback to be called when view was resumed
 * @param[in] data callback user data
 */
#define elm_app_client_view_resume(cb, data) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_RESUME), EO_TYPECHECK(Elm_App_Client_View_Cb, cb), EO_TYPECHECK(const void *, data)

/**
 * @def elm_app_client_view_title_get
 * @since 1.8
 *
 * Get title of view
 *
 * @param[out] ret title of view
 */
#define elm_app_client_view_title_get(ret) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_TITLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_app_client_view_icon_get
 * @since 1.8
 *
 * Get icon path of view
 *
 * @param[out] ret icon path of view
 */
#define elm_app_client_view_icon_get(ret) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_ICON_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_app_client_view_icon_pixels_get
 * @since 1.8
 *
 * Get icon pixels of view, view could have a icon in raw format
 * not saved in disk.
 *
 * @param[out] w icon width
 * @param[out] h icon height
 * @param[out] has_alpha if icon have alpha channel
 * @param[out] pixels unsigned char array, with all bytes of icon
 */
#define elm_app_client_view_icon_pixels_get(w, h, has_alpha, pixels) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_ICON_PIXELS_GET), EO_TYPECHECK(unsigned int *, w), EO_TYPECHECK(unsigned int *, h), EO_TYPECHECK(Eina_Bool *, has_alpha), EO_TYPECHECK(const unsigned char **, pixels)

/**
 * @def elm_app_client_view_progress_get
 * @since 1.8
 *
 * Get progress of view, should be -1 if there nothing in progress or something
 * between 0-100
 *
 * @param[out] progress progress of view
 */
#define elm_app_client_view_progress_get(progress) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_PROGRESS_GET), EO_TYPECHECK(unsigned short *, progress)

/**
 * @def elm_app_client_view_new_events_get
 * @since 1.8
 *
 * Get new events of view
 *
 * @param[out] events number of events of view
 */
#define elm_app_client_view_new_events_get(events) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_NEW_EVENTS_GET), EO_TYPECHECK(int *, events)

/**
 * @def elm_app_client_view_state_get
 * @since 1.8
 *
 * Get state of view
 *
 * @param[out] state state of view
 */
#define elm_app_client_view_state_get(state) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_STATE_GET), EO_TYPECHECK(Elm_App_View_State *, state)

/**
 * @def elm_app_client_view_window_get
 * @since 1.8
 *
 * Get window of view
 *
 * @param[out] window window of view
 */
#define elm_app_client_view_window_get(window) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_WINDOW_GET), EO_TYPECHECK(int *, window)

/**
 * @def elm_app_client_view_path_get
 * @since 1.8
 *
 * Get DBus path of view
 *
 * @param[out] ret DBus path of view
 */
#define elm_app_client_view_path_get(ret) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_PATH_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_app_client_view_package_get
 * @since 1.8
 *
 * Get application package
 *
 * @param[out] ret Package of application
 */
#define elm_app_client_view_package_get(ret) ELM_APP_CLIENT_VIEW_ID(ELM_APP_CLIENT_VIEW_SUB_ID_PACKAGE_GET), EO_TYPECHECK(const char **, ret)

/**
 * Events
 */
extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_STATE_CHANGED;
#define ELM_APP_CLIENT_VIEW_EV_STATE_CHANGED (&(_ELM_APP_CLIENT_VIEW_EV_STATE_CHANGED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_TITLE_CHANGED;
#define ELM_APP_CLIENT_VIEW_EV_TITLE_CHANGED (&(_ELM_APP_CLIENT_VIEW_EV_TITLE_CHANGED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_ICON_CHANGED;
#define ELM_APP_CLIENT_VIEW_EV_ICON_CHANGED (&(_ELM_APP_CLIENT_VIEW_EV_ICON_CHANGED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_ICON_PIXELS_CHANGED;
#define ELM_APP_CLIENT_VIEW_EV_ICON_PIXELS_CHANGED (&(_ELM_APP_CLIENT_VIEW_EV_ICON_PIXELS_CHANGED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_NEW_EVENTS_CHANGED;
#define ELM_APP_CLIENT_VIEW_EV_NEW_EVENTS_CHANGED (&(_ELM_APP_CLIENT_VIEW_EV_NEW_EVENTS_CHANGED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_PROGRESS_CHANGED;
#define ELM_APP_CLIENT_VIEW_EV_PROGRESS_CHANGED (&(_ELM_APP_CLIENT_VIEW_EV_PROGRESS_CHANGED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_WINDOW_CHANGED;
#define ELM_APP_CLIENT_VIEW_EV_WINDOW_CHANGED (&(_ELM_APP_CLIENT_VIEW_EV_WINDOW_CHANGED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_VIEW_EV_PROPERTY_CHANGED;
#define ELM_APP_CLIENT_VIEW_EV_PROPERTY_CHANGED (&(_ELM_APP_CLIENT_VIEW_EV_PROPERTY_CHANGED))
