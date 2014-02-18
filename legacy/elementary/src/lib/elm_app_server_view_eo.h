#define ELM_APP_SERVER_VIEW_CLASS elm_app_server_view_class_get()

const Eo_Class *elm_app_server_view_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_APP_SERVER_VIEW_BASE_ID;

enum
{
   ELM_APP_SERVER_VIEW_SUB_ID_CONSTRUCTOR,
   ELM_APP_SERVER_VIEW_SUB_ID_RESUME,
   ELM_APP_SERVER_VIEW_SUB_ID_PAUSE,
   ELM_APP_SERVER_VIEW_SUB_ID_CLOSE,
   ELM_APP_SERVER_VIEW_SUB_ID_SHALLOW,
   ELM_APP_SERVER_VIEW_SUB_ID_STATE_GET,
   ELM_APP_SERVER_VIEW_SUB_ID_WINDOW_SET,
   ELM_APP_SERVER_VIEW_SUB_ID_TITLE_SET,
   ELM_APP_SERVER_VIEW_SUB_ID_TITLE_GET,
   ELM_APP_SERVER_VIEW_SUB_ID_ICON_NAME_SET,
   ELM_APP_SERVER_VIEW_SUB_ID_ICON_NAME_GET,
   ELM_APP_SERVER_VIEW_SUB_ID_ICON_PIXELS_SET,
   ELM_APP_SERVER_VIEW_SUB_ID_ICON_PIXELS_GET,
   ELM_APP_SERVER_VIEW_SUB_ID_PROGRESS_SET,
   ELM_APP_SERVER_VIEW_SUB_ID_PROGRESS_GET,
   ELM_APP_SERVER_VIEW_SUB_ID_NEW_EVENTS_SET,
   ELM_APP_SERVER_VIEW_SUB_ID_NEW_EVENTS_GET,
   ELM_APP_SERVER_VIEW_SUB_ID_ID_GET,
   ELM_APP_SERVER_VIEW_SUB_ID_PATH_GET,
   ELM_APP_SERVER_VIEW_SUB_ID_LAST
};

#define ELM_APP_SERVER_VIEW_ID(sub_id) (ELM_APP_SERVER_VIEW_BASE_ID + sub_id)

/**
 * @def elm_app_server_view_constructor
 * @since 1.8
 *
 * Class constructor of elm_app_server_view
 *
 * @param[in] id identifier of view
 *
 * @note You must supply the elm_app_server as parent.
 */
#define elm_app_server_view_constructor(id) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(const char *, id)

/**
 * @def elm_app_server_view_resume
 * @since 1.8
 *
 */
#define elm_app_server_view_resume() ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_RESUME)

/**
 * @def elm_app_server_view_pause
 * @since 1.8
 *
 */
#define elm_app_server_view_pause() ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_PAUSE)

/**
 * @def elm_app_server_view_close
 * @since 1.8
 *
 */
#define elm_app_server_view_close() ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_CLOSE)

/**
 * @def elm_app_server_view_shallow
 * @since 1.8
 *
 */
#define elm_app_server_view_shallow() ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_SHALLOW)

/**
 * @def elm_app_server_view_state_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_view_state_get(ret) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_STATE_GET), EO_TYPECHECK(Elm_App_View_State *, ret)

/**
 * @def elm_app_server_view_window_set
 * @since 1.8
 *
 * @param[in] win
 */
#define elm_app_server_view_window_set(win) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_WINDOW_SET), EO_TYPECHECK(Evas_Object *, win)

/**
 * @def elm_app_server_view_title_set
 * @since 1.8
 *
 * @param[in] title
 */
#define elm_app_server_view_title_set(title) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_TITLE_SET), EO_TYPECHECK(const char *, title)

/**
 * @def elm_app_server_view_title_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_view_title_get(ret) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_TITLE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_app_server_view_icon_set
 * @since 1.8
 *
 * @param[in] icon
 */
#define elm_app_server_view_icon_set(icon) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_ICON_NAME_SET), EO_TYPECHECK(const char *, icon)

/**
 * @def elm_app_server_view_icon_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_view_icon_get(ret) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_ICON_NAME_GET), EO_TYPECHECK(const char **, ret)

#define elm_app_server_view_icon_pixels_set(w, h, has_alpha, pixels) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_ICON_PIXELS_SET), EO_TYPECHECK(unsigned int, w), EO_TYPE_CHECK(unsigned int, h), EO_TYPE_CHECK(Eina_Bool, has_alpha), EO_TYPE_CHECK(const unsigned char *, pixels)

#define elm_app_server_view_icon_pixels_get(w, h, has_alpha, pixels) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_ICON_PIXELS_GET), EO_TYPECHECK(unsigned int *, w), EO_TYPE_CHECK(unsigned int *, h), EO_TYPE_CHECK(Eina_Bool *, has_alpha), EO_TYPE_CHECK(const unsigned char **, pixels)

/**
 * @def elm_app_server_view_progress_set
 * @since 1.8
 *
 * @param[in] progress
 */
#define elm_app_server_view_progress_set(progress) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_PROGRESS_SET), EO_TYPECHECK(short, progress)

/**
 * @def elm_app_server_view_progress_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_view_progress_get(ret) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_PROGRESS_GET), EO_TYPECHECK(short *, ret)

/**
 * @def elm_app_server_view_new_events_set
 * @since 1.8
 *
 * @param[in] events
 */
#define elm_app_server_view_new_events_set(events) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_NEW_EVENTS_SET), EO_TYPECHECK(int, events)

/**
 * @def elm_app_server_view_new_events_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_view_new_events_get(ret) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_NEW_EVENTS_GET), EO_TYPECHECK(int *, ret)

/**
 * @def elm_app_server_view_id_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_view_id_get(ret) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_ID_GET), EO_TYPECHECK(Eina_Stringshare **, ret)

/**
 * @def elm_app_server_view_path_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_view_path_get(ret) ELM_APP_SERVER_VIEW_ID(ELM_APP_SERVER_VIEW_SUB_ID_PATH_GET), EO_TYPECHECK(const char **, ret)

/**
 * Events
 */
extern EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_RESUMED;
#define ELM_APP_SERVER_VIEW_EV_RESUMED (&(_ELM_APP_SERVER_VIEW_EV_RESUMED))

extern EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_PAUSED;
#define ELM_APP_SERVER_VIEW_EV_PAUSED (&(_ELM_APP_SERVER_VIEW_EV_PAUSED))

extern EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_CLOSED;
#define ELM_APP_SERVER_VIEW_EV_CLOSED (&(_ELM_APP_SERVER_VIEW_EV_CLOSED))

extern EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_SHALLOW;
#define ELM_APP_SERVER_VIEW_EV_SHALLOW (&(_ELM_APP_SERVER_VIEW_EV_SHALLOW))

extern EAPI const Eo_Event_Description _ELM_APP_SERVER_VIEW_EV_SAVE;
#define ELM_APP_SERVER_VIEW_EV_SAVE (&(_ELM_APP_SERVER_VIEW_EV_SAVE))
