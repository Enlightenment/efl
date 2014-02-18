#define ELM_APP_SERVER_CLASS elm_app_server_class_get()

const Eo_Class *elm_app_server_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_APP_SERVER_BASE_ID;

#define ELM_APP_CLIENT_VIEW_OPEN_CANCELED "org.enlightenment.Application.ViewOpenCanceled"
#define ELM_APP_CLIENT_VIEW_OPEN_TIMEOUT "org.enlightenment.Application.ViewOpenTimeout"
#define ELM_APP_CLEINT_VIEW_OPEN_ERROR "org.enlightenment.Application.CouldNotCreateView"

enum
{
   ELM_APP_SERVER_SUB_ID_CONSTRUCTOR,
   ELM_APP_SERVER_SUB_ID_SAVE,
   ELM_APP_SERVER_SUB_ID_CLOSE_ALL,
   ELM_APP_SERVER_SUB_ID_VIEW_CHECK,
   ELM_APP_SERVER_SUB_ID_TITLE_SET,
   ELM_APP_SERVER_SUB_ID_TITLE_GET,
   ELM_APP_SERVER_SUB_ID_ICON_SET,
   ELM_APP_SERVER_SUB_ID_ICON_GET,
   ELM_APP_SERVER_SUB_ID_ICON_PIXELS_SET,
   ELM_APP_SERVER_SUB_ID_ICON_PIXELS_GET,
   ELM_APP_SERVER_SUB_ID_PACKAGE_GET,
   ELM_APP_SERVER_SUB_ID_VIEWS_GET,
   ELM_APP_SERVER_SUB_ID_PATH_GET,
   ELM_APP_SERVER_SUB_ID_VIEW_ADD,
   ELM_APP_SERVER_SUB_ID_LAST
};

#define ELM_APP_SERVER_VIEW_CREATE_DUPLICATE "org.enlightenment.Application.ViewDuplicate"

typedef Eo Elm_App_Server;
typedef Eo Elm_App_Server_View;

typedef Elm_App_Server_View *(*Elm_App_Server_Create_View_Cb)(Elm_App_Server *app, const Eina_Value *args, Eina_Stringshare **error_name, Eina_Stringshare **error_message);

#define ELM_APP_SERVER_ID(sub_id) (ELM_APP_SERVER_BASE_ID + sub_id)

/**
 * @def elm_app_server_constructor
 * @since 1.8
 *
 * Class constructor of elm_app_server
 *
 * @param[in] packageid package of application
 * @param[in] create_view_cb callback to be called when user whants to
 * open some application view
 */
#define elm_app_server_constructor(packageid, create_view_cb) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(const char *, packageid), EO_TYPECHECK(Elm_App_Server_Create_View_Cb, create_view_cb)

/**
 * @def elm_app_server_save
 * @since 1.8
 *
 * Save the state of all views
 */
#define elm_app_server_save() ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_SAVE)

/**
 * @def elm_app_server_close_all
 * @since 1.8
 *
 * Close all views of application
 */
#define elm_app_server_close_all() ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_CLOSE_ALL)

/**
 * @def elm_app_server_title_set
 * @since 1.8
 *
 * Set a title to application.
 *
 * @param[in] title title of application
 */
#define elm_app_server_title_set(title) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_TITLE_SET), EO_TYPECHECK(const char *, title)

/**
 * @def elm_app_server_title_get
 * @since 1.8
 *
 * Get title of application
 *
 * @param[out] ret title of application
 */
#define elm_app_server_title_get(ret) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_TITLE_GET), EO_TYPECHECK(Eina_Stringshare **, ret)

/**
 * @def elm_app_server_icon_set
 * @since 1.8
 *
 * @param[in] icon title of icon
 */
#define elm_app_server_icon_set(icon) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_ICON_SET), EO_TYPECHECK(const char*, icon)

/**
 * @def elm_app_server_icon_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_icon_get(ret) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_ICON_GET), EO_TYPECHECK(Eina_Stringshare **, ret)

#define elm_app_server_icon_pixels_set(w, h, has_alpha, pixels) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_ICON_PIXELS_SET), EO_TYPECHECK(unsigned int, w), EO_TYPECHECK(unsigned int, h), EO_TYPECHECK(Eina_Bool, has_alpha), EO_TYPECHECK(const unsigned char *, pixels)

#define elm_app_server_icon_pixels_get(w, h, has_alpha, pixels) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_ICON_PIXELS_GET), EO_TYPECHECK(unsigned int *, w), EO_TYPECHECK(unsigned int *, h), EO_TYPECHECK(Eina_Bool *, has_alpha), EO_TYPECHECK(const unsigned char **, pixels)

/**
 * @def elm_app_server_package_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_package_get(ret) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_PACKAGE_GET), EO_TYPECHECK(Eina_Stringshare **, ret)

/**
 * @def elm_app_server_path_get
 * @since 1.8
 *
 * @param[out] ret
 */
#define elm_app_server_path_get(ret) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_PATH_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_app_server_views_get
 * @since 1.8
 *
 * Return a iterator with all views of application
 *
 * @param[out] ret Iterator with all views of application, you must free
 * iterator after use.
 */
#define elm_app_server_views_get(ret) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_VIEWS_GET), EO_TYPECHECK(Eina_Iterator **, ret)

/**
 * @def elm_app_server_view_check
 * @since 1.8
 *
 * If view id is available and unique, return the full DBus object path of view
 *
 * @param[in] id view identifier
 * @param[out] ret @c EINA_TRUE if id is valid or @c EINA_FALSE if not
 */
#define elm_app_server_view_check(id, ret) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_VIEW_CHECK), EO_TYPECHECK(const char *, id), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_app_server_view_add
 * @since 1.8
 *
 * Add a view to elm_app_server.
 * This should only be used if the application open a view
 * that was not requested by create_view_cb.
 *
 * @param[in] view elm_app_server_view
 */
#define elm_app_server_view_add(view) ELM_APP_SERVER_ID(ELM_APP_SERVER_SUB_ID_VIEW_ADD), EO_TYPECHECK(Elm_App_Server_View *, view)

/**
 * Events
 */
extern EAPI const Eo_Event_Description _ELM_APP_SERVER_EV_TERMINATE;
#define ELM_APP_SERVER_EV_TERMINATE (&(_ELM_APP_SERVER_EV_TERMINATE))
