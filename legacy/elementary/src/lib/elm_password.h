/**
 * @defgroup Password_last_show Password show last
 *
 * Show last feature of password mode enables user to view
 * the last input entered for few seconds before masking it.
 * These functions allow to set this feature in password mode
 * of entry widget and also allow to manipulate the duration
 * for which the input has to be visible.
 *
 * @{
 */

/**
 * Get the "show last" setting of password mode.
 *
 * This gets the "show last" setting of password mode which might be
 * enabled or disabled.
 *
 * @return @c EINA_TRUE, if the "show last" setting is enabled, 
 * @c EINA_FALSE  if it's disabled.
 *
 * @ingroup Password_last_show
 */
EAPI Eina_Bool elm_password_show_last_get(void);

/**
 * Set show last setting in password mode.
 *
 * This enables or disables show last setting of password mode.
 *
 * @param password_show_last If EINA_TRUE enables "show last" in password mode.
 * @see elm_password_show_last_timeout_set()
 * @ingroup Password_last_show
 */
EAPI void      elm_password_show_last_set(Eina_Bool password_show_last);

/**
 * Gets the timeout value in "show last" password mode.
 *
 * This gets the time out value for which the last input entered in password
 * mode will be visible.
 *
 * @return The timeout value of "show last" password mode.
 * @ingroup Password_last_show
 */
EAPI double    elm_password_show_last_timeout_get(void);

/**
 * Set's the timeout value in "show last" password mode.
 *
 * This sets the time out value for which the last input entered in password
 * mode will be visible.
 *
 * @param password_show_last_timeout The timeout value.
 * @see elm_password_show_last_set()
 * @ingroup Password_last_show
 */
EAPI void      elm_password_show_last_timeout_set(double password_show_last_timeout);

/**
 * @}
 */
