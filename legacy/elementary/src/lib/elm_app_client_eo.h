#define ELM_APP_CLIENT_CLASS elm_app_client_class_get()

const Eo_Class *elm_app_client_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_APP_CLIENT_BASE_ID;

enum
{
   ELM_APP_CLIENT_SUB_ID_CONSTRUCTOR,
   ELM_APP_CLIENT_SUB_ID_VIEW_OPEN,
   ELM_APP_CLIENT_SUB_ID_VIEWS_GET,
   ELM_APP_CLIENT_SUB_ID_VIEWS_ALL_CLOSE,
   ELM_APP_CLIENT_SUB_ID_TERMINATE,
   ELM_APP_CLIENT_SUB_ID_PACKAGE_GET,
   ELM_APP_CLIENT_SUB_ID_VIEW_OPEN_CANCEL,
   ELM_APP_CLIENT_SUB_ID_LAST
};

typedef Eo Elm_App_Client;
typedef Eo Elm_App_Client_View;

typedef Eldbus_Pending Elm_App_Client_Pending;

typedef void (*Elm_App_Client_Open_View_Cb)(void *data, Elm_App_Client_View *view, const char *error, const char *error_message);

#define ELM_APP_CLIENT_ID(sub_id) (ELM_APP_CLIENT_BASE_ID + sub_id)

/**
 * @def elm_app_client_constructor
 * @since 1.8
 *
 * Class constructor of elm_app_client.
 *
 * @param[in] package of application
 */
#define elm_app_client_constructor(package) ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_CONSTRUCTOR), EO_TYPECHECK(const char *, package)


/**
 * @def elm_app_client_view_open
 * @since 1.8
 *
 * Open a application view.
 *
 * @param[in] arg an array of<string,data> containing the arguments of view
 * @param[in] view_open_cb callback to be called when view open
 * @param[in] data calback user data
 * @param[out] pending handler to cancel the view opening if it takes to long
 */
#define elm_app_client_view_open(args, view_open_cb, data, pending) ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_VIEW_OPEN), EO_TYPECHECK(Eina_Value *, args), EO_TYPECHECK(Elm_App_Client_Open_View_Cb, view_open_cb), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_App_Client_Pending **, pending)

/**
 * @def elm_app_client_views_get
 * @since 1.8
 *
 * Return a iterator with all views of application.
 *
 * @param[out] ret the iterator with all views, must be freed after use
 */
#define elm_app_client_views_get(ret) ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_VIEWS_GET), EO_TYPECHECK(Eina_Iterator **, ret)

/**
 * @def elm_app_client_view_all_close
 * @since 1.8
 *
 * Close all views of application.
 */
#define elm_app_client_view_all_close() ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_VIEWS_ALL_CLOSE)

/**
 * @def elm_app_client_terminate
 * @since 1.8
 *
 * Terminate application.
 */
#define elm_app_client_terminate() ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_TERMINATE)

/**
 * @def elm_app_client_package_get
 * @since 1.8
 *
 * Return the application package.
 *
 * @param[out] ret application package
 */
#define elm_app_client_package_get(ret) ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_PACKAGE_GET), EO_TYPECHECK(const char **, ret)

/**
 * @def elm_app_client_view_open_cancel
 * @since 1.8
 *
 * Cancel a pending elm_app_client_view_open().
 *
 * @param[in] pending the view open handler.
 */
#define elm_app_client_view_open_cancel(pending) ELM_APP_CLIENT_ID(ELM_APP_CLIENT_SUB_ID_VIEW_OPEN_CANCEL), EO_TYPECHECK(Elm_App_Client_Pending **, pending)

/**
 * Events
 */
extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_EV_VIEW_CREATED;
#define ELM_APP_CLIENT_EV_VIEW_CREATED (&(_ELM_APP_CLIENT_EV_VIEW_CREATED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_EV_VIEW_DELETED;
#define ELM_APP_CLIENT_EV_VIEW_DELETED (&(_ELM_APP_CLIENT_EV_VIEW_DELETED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_EV_VIEW_LIST_LOADED;
#define ELM_APP_CLIENT_EV_VIEW_LIST_LOADED (&(_ELM_APP_CLIENT_EV_VIEW_LIST_LOADED))

extern EAPI const Eo_Event_Description _ELM_APP_CLIENT_EV_TERMINATED;
#define ELM_APP_CLIENT_EV_TERMINATED (&(_ELM_APP_CLIENT_EV_TERMINATED))
