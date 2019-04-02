/**
 * Request that your elementary application needs Efreet
 *
 * This initializes the Efreet library when called and if support exists
 * it returns @c EINA_TRUE, otherwise returns @c EINA_FALSE. This must be called
 * before any efreet calls.
 *
 * @return @c EINA_TRUE if support exists and initialization succeeded.
 *
 * @ingroup Efreet
 */
EAPI Eina_Bool elm_need_efreet(void);

/**
 * Request that your elementary application needs Elm_Systray
 *
 * This initializes the Elm_Systray when called and, if support exists,
 * returns @c EINA_TRUE, otherwise returns @c EINA_FALSE. This must be called
 * before any elm_systray calls.
 *
 * @return @c EINA_TRUE if support exists and initialization succeeded.
 *
 * @ingroup Elm_Systray
 * @since 1.8
 */
EAPI Eina_Bool elm_need_systray(void);

/**
 * Request that your elementary application needs Elm_Sys_Notify
 *
 * This initializes the Elm_Sys_Notify when called and if support exists
 * it returns @c EINA_TRUE, otherwise returns @c EINA_FALSE. This must be called
 * before any elm_sys_notify calls.
 *
 * @return @c EINA_TRUE if support exists and initialization succeeded.
 *
 * @ingroup Elm_Sys_Notify
 * @since 1.8
 */
EAPI Eina_Bool elm_need_sys_notify(void);

/**
 * Request that your elementary application needs e_dbus
 *
 * This initializes the E_dbus library when called and if support exists
 * it returns @c EINA_TRUE, otherwise returns @c EINA_FALSE. This must be called
 * before any e_dbus calls.
 *
 * @return @c EINA_TRUE if support exists and initialization succeeded.
 *
 * @deprecated use elm_need_eldbus() for Eldbus (v2) support. Old API is
 * deprecated.
 *
 * @ingroup E_dbus
 */
EAPI Eina_Bool elm_need_e_dbus(void) EINA_DEPRECATED;

/**
 * Request that your elementary application needs eldbus
 *
 * This initializes the eldbus (a.k.a. v2) library when called and if
 * support exists it returns @c EINA_TRUE, otherwise returns
 * @c EINA_FALSE. This must be called before any eldbus calls.
 *
 * @return @c EINA_TRUE if support exists and initialization succeeded.
 *
 * @since 1.8.0
 *
 * @ingroup eldbus
 */
EAPI Eina_Bool elm_need_eldbus(void);

/**
 * Request that your elementary application needs elocation
 *
 * This initializes the elocation library when called and if
 * support exists it returns @c EINA_TRUE, otherwise returns
 * @c EINA_FALSE. This must be called before any elocation usage.
 *
 * @return @c EINA_TRUE if support exists and initialization succeeded.
 *
 * @since 1.8.0
 *
 * @ingroup eldbus
 */
EAPI Eina_Bool elm_need_elocation(void);

/**
 * Request that your elementary application needs ethumb
 *
 * This initializes the Ethumb library when called and if support exists
 * it returns @c EINA_TRUE, otherwise returns @c EINA_FALSE.
 * This must be called before any other function that deals with
 * elm_thumb objects or ethumb_client instances.
 *
 * @ingroup Elm_Thumb
 */
EAPI Eina_Bool elm_need_ethumb(void);

/**
 * Request that your elementary application needs web support
 *
 * This initializes the Ewebkit library when called and if support exists
 * it returns @c EINA_TRUE, otherwise returns @c EINA_FALSE.
 * This must be called before any other function that deals with
 * elm_web objects or ewk_view instances.
 *
 * @ingroup Elm_Web
 */
EAPI Eina_Bool elm_need_web(void);
