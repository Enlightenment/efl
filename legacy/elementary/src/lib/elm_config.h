/**
 * @defgroup Config Elementary Config
 *
 * Elementary configuration is formed by a set options bounded to a
 * given @ref Profile profile, like @ref Theme theme, @ref Fingers
 * "finger size", etc. These are functions with which one synchronizes
 * changes made to those values to the configuration storing files, de
 * facto. You most probably don't want to use the functions in this
 * group unless you're writing an elementary configuration manager.
 *
 * @{
 */

/**
 * Save back Elementary's configuration, so that it will persist on
 * future sessions.
 *
 * @return @c EINA_TRUE, when successful. @c EINA_FALSE, otherwise.
 * @ingroup Config
 *
 * This function will take effect -- thus, do I/O -- immediately. Use
 * it when you want to save all configuration changes at once. The
 * current configuration set will get saved onto the current profile
 * configuration file.
 *
 */
EAPI Eina_Bool elm_config_save(void);

/**
 * Reload Elementary's configuration, bounded to current selected
 * profile.
 *
 * @return @c EINA_TRUE, when successful. @c EINA_FALSE, otherwise.
 * @ingroup Config
 *
 * Useful when you want to force reloading of configuration values for
 * a profile. If one removes user custom configuration directories,
 * for example, it will force a reload with system values instead.
 *
 */
EAPI void      elm_config_reload(void);

/**
 * Flush all config settings then apply those settings to all applications
 * using elementary on the current display.
 * Following functions will update the config settings
 * @li elm_cache_flush_interval_set()
 * @li elm_cache_flush_enabled_set()
 * @li elm_edje_file_cache_set()
 * @li elm_edje_collection_cache_set()
 * @li elm_finger_size_set()
 * @li elm_focus_highlight_enabled_set()
 * @li elm_focus_highlight_animate_set()
 * @li elm_font_overlay_set()
 * @li elm_font_cache_set()
 * @li elm_image_cache_set()
 * @li elm_profile_set()
 * @li elm_scale_set()
 * @li elm_scroll_bounce_enabled_set()
 * @li elm_scroll_bounce_friction_set()
 * @li elm_scroll_page_scroll_friction_set()
 * @li elm_scroll_bring_in_scroll_friction_set()
 * @li elm_scroll_zoom_friction_set()
 * @li elm_scroll_thumbscroll_enabled_set()
 * @li elm_scroll_thumbscroll_threshold_set()
 * @li elm_scroll_thumbscroll_momentum_threshold_set()
 * @li elm_scroll_thumbscroll_friction_set()
 * @li elm_scroll_thumbscroll_border_friction_set()
 * @li elm_scroll_thumbscroll_sensitivity_friction_set()
 * @li elm_theme_set()
 *  
 * @ingroup Config
 *
 */
EAPI void      elm_config_all_flush(void);

/**
 * @}
 */

/**
 * @defgroup Profile Elementary Profile
 *
 * Profiles are pre-set options that affect the whole look-and-feel of
 * Elementary-based applications. There are, for example, profiles
 * aimed at desktop computer applications and others aimed at mobile,
 * touchscreen-based ones. You most probably don't want to use the
 * functions in this group unless you're writing an elementary
 * configuration manager.
 *
 * @{
 */

/**
 * Get Elementary's profile in use.
 *
 * This gets the global profile that is applied to all Elementary
 * applications.
 *
 * @return The profile's name
 * @ingroup Profile
 */
EAPI const char *elm_profile_current_get(void);

/**
 * Get an Elementary's profile directory path in the filesystem. One
 * may want to fetch a system profile's dir or a user one (fetched
 * inside $HOME).
 *
 * @param profile The profile's name
 * @param is_user Whether to lookup for a user profile (@c EINA_TRUE)
 *                or a system one (@c EINA_FALSE)
 * @return The profile's directory path.
 * @ingroup Profile
 *
 * @note You must free it with elm_profile_dir_free().
 */
EAPI const char *elm_profile_dir_get(const char *profile, Eina_Bool is_user);

/**
 * Free an Elementary's profile directory path, as returned by
 * elm_profile_dir_get().
 *
 * @param p_dir The profile's path
 * @ingroup Profile
 *
 */
EAPI void        elm_profile_dir_free(const char *p_dir);

/**
 * Get Elementary's list of available profiles.
 *
 * @return The profiles list. List node data are the profile name
 *         strings.
 * @ingroup Profile
 *
 * @note One must free this list, after usage, with the function
 *       elm_profile_list_free().
 */
EAPI Eina_List  *elm_profile_list_get(void);

/**
 * Free Elementary's list of available profiles.
 *
 * @param l The profiles list, as returned by elm_profile_list_get().
 * @ingroup Profile
 *
 */
EAPI void        elm_profile_list_free(Eina_List *l);

/**
 * Set Elementary's profile.
 *
 * This sets the global profile that is applied to Elementary
 * applications. Just the process the call comes from will be
 * affected.
 *
 * @param profile The profile's name
 * @ingroup Profile
 *
 */
EAPI void        elm_profile_set(const char *profile);

/**
 * @}
 */

/**
 * @defgroup Scrolling Scrolling
 *
 * These are functions setting how scrollable views in Elementary
 * widgets should behave on user interaction.
 *
 * @{
 */

/**
 * Get whether scrollers should bounce when they reach their
 * viewport's edge during a scroll.
 *
 * @return the thumb scroll bouncing state
 *
 * This is the default behavior for touch screens, in general.
 * @ingroup Scrolling
 */
EAPI Eina_Bool    elm_scroll_bounce_enabled_get(void);

/**
 * Set whether scrollers should bounce when they reach their
 * viewport's edge during a scroll.
 *
 * @param enabled the thumb scroll bouncing state
 *
 * @see elm_thumbscroll_bounce_enabled_get()
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_bounce_enabled_set(Eina_Bool enabled);

/**
 * Get the amount of inertia a scroller will impose at bounce
 * animations.
 *
 * @return the thumb scroll bounce friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_scroll_bounce_friction_get(void);

/**
 * Set the amount of inertia a scroller will impose at bounce
 * animations.
 *
 * @param friction the thumb scroll bounce friction
 *
 * @see elm_thumbscroll_bounce_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_bounce_friction_set(double friction);

/**
 * Get the amount of inertia a <b>paged</b> scroller will impose at
 * page fitting animations.
 *
 * @return the page scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_scroll_page_scroll_friction_get(void);

/**
 * Set the amount of inertia a <b>paged</b> scroller will impose at
 * page fitting animations.
 *
 * @param friction the page scroll friction
 *
 * @see elm_thumbscroll_page_scroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_page_scroll_friction_set(double friction);

/**
 * Get the amount of inertia a scroller will impose at region bring
 * animations.
 *
 * @return the bring in scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_scroll_bring_in_scroll_friction_get(void);

/**
 * Set the amount of inertia a scroller will impose at region bring
 * animations.
 *
 * @param friction the bring in scroll friction
 *
 * @see elm_thumbscroll_bring_in_scroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_bring_in_scroll_friction_set(double friction);

/**
 * Get the amount of inertia scrollers will impose at animations
 * triggered by Elementary widgets' zooming API.
 *
 * @return the zoom friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_scroll_zoom_friction_get(void);

/**
 * Set the amount of inertia scrollers will impose at animations
 * triggered by Elementary widgets' zooming API.
 *
 * @param friction the zoom friction
 *
 * @see elm_thumbscroll_zoom_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_zoom_friction_set(double friction);

/**
 * Get whether scrollers should be draggable from any point in their
 * views.
 *
 * @return the thumb scroll state
 *
 * @note This is the default behavior for touch screens, in general.
 * @note All other functions namespaced with "thumbscroll" will only
 *       have effect if this mode is enabled.
 *
 * @ingroup Scrolling
 */
EAPI Eina_Bool    elm_scroll_thumbscroll_enabled_get(void);

/**
 * Set whether scrollers should be draggable from any point in their
 * views.
 *
 * @param enabled the thumb scroll state
 *
 * @see elm_thumbscroll_enabled_get()
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_thumbscroll_enabled_set(Eina_Bool enabled);

/**
 * Get the number of pixels one should travel while dragging a
 * scroller's view to actually trigger scrolling.
 *
 * @return the thumb scroll threshold
 *
 * One would use higher values for touch screens, in general, because
 * of their inherent imprecision.
 * @ingroup Scrolling
 */
EAPI unsigned int elm_scroll_thumbscroll_threshold_get(void);

/**
 * Set the number of pixels one should travel while dragging a
 * scroller's view to actually trigger scrolling.
 *
 * @param threshold the thumb scroll threshold
 *
 * @see elm_thumbscroll_threshold_get()
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_thumbscroll_threshold_set(unsigned int threshold);

/**
 * Get the minimum speed of mouse cursor movement which will trigger
 * list self scrolling animation after a mouse up event
 * (pixels/second).
 *
 * @return the thumb scroll momentum threshold
 *
 * @ingroup Scrolling
 */
EAPI double       elm_scroll_thumbscroll_momentum_threshold_get(void);

/**
 * Set the minimum speed of mouse cursor movement which will trigger
 * list self scrolling animation after a mouse up event
 * (pixels/second).
 *
 * @param threshold the thumb scroll momentum threshold
 *
 * @see elm_thumbscroll_momentum_threshold_get()
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_thumbscroll_momentum_threshold_set(double threshold);

/**
 * Get the amount of inertia a scroller will impose at self scrolling
 * animations.
 *
 * @return the thumb scroll friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_scroll_thumbscroll_friction_get(void);

/**
 * Set the amount of inertia a scroller will impose at self scrolling
 * animations.
 *
 * @param friction the thumb scroll friction
 *
 * @see elm_thumbscroll_friction_get()
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_thumbscroll_friction_set(double friction);

/**
 * Get the amount of lag between your actual mouse cursor dragging
 * movement and a scroller's view movement itself, while pushing it
 * into bounce state manually.
 *
 * @return the thumb scroll border friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_scroll_thumbscroll_border_friction_get(void);

/**
 * Set the amount of lag between your actual mouse cursor dragging
 * movement and a scroller's view movement itself, while pushing it
 * into bounce state manually.
 *
 * @param friction the thumb scroll border friction. @c 0.0 for
 *        perfect synchrony between two movements, @c 1.0 for maximum
 *        lag.
 *
 * @see elm_thumbscroll_border_friction_get()
 * @note parameter value will get bound to 0.0 - 1.0 interval, always
 *
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_thumbscroll_border_friction_set(double friction);

/**
 * Get the sensitivity amount which is be multiplied by the length of
 * mouse dragging.
 *
 * @return the thumb scroll sensitivity friction
 *
 * @ingroup Scrolling
 */
EAPI double       elm_scroll_thumbscroll_sensitivity_friction_get(void);

/**
 * Set the sensitivity amount which is be multiplied by the length of
 * mouse dragging.
 *
 * @param friction the thumb scroll sensitivity friction. @c 0.1 for
 *        minimum sensitivity, @c 1.0 for maximum sensitivity. 0.25
 *        is proper.
 *
 * @see elm_thumbscroll_sensitivity_friction_get()
 * @note parameter value will get bound to 0.1 - 1.0 interval, always
 *
 * @ingroup Scrolling
 */
EAPI void         elm_scroll_thumbscroll_sensitivity_friction_set(double friction);

/**
 * @}
 */

/**
 * Get the duration for occurring long press event.
 *
 * @return Timeout for long press event
 * @ingroup Longpress
 */
EAPI double       elm_longpress_timeout_get(void);

/**
 * Set the duration for occurring long press event.
 *
 * @param lonpress_timeout Timeout for long press event
 * @ingroup Longpress
 */
EAPI void         elm_longpress_timeout_set(double longpress_timeout);

/**
 * Get the duration after which tooltip will be shown.
 *
 * @return Duration after which tooltip will be shown.
 */
EAPI double      elm_config_tooltip_delay_get(void);

/**
 * Set the duration after which tooltip will be shown.
 *
 * @return EINA_TRUE if value is set.  
 */
EAPI void        elm_config_tooltip_delay_set(double delay);

/**
 * Get the configured cursor engine only usage
 *
 * This gets the globally configured exclusive usage of engine cursors.
 *
 * @return 1 if only engine cursors should be used
 * @ingroup Cursors
 */
EAPI Eina_Bool   elm_cursor_engine_only_get(void);

/**
 * Set the configured cursor engine only usage
 *
 * This sets the globally configured exclusive usage of engine cursors.
 * It won't affect cursors set before changing this value.
 *
 * @param engine_only If 1 only engine cursors will be enabled, if 0 will
 * look for them on theme before.
 * @ingroup Cursors
 */
EAPI void        elm_cursor_engine_only_set(Eina_Bool engine_only);

/**
 * Get the global scaling factor
 *
 * This gets the globally configured scaling factor that is applied to all
 * objects.
 *
 * @return The scaling factor
 * @ingroup Scaling
 */
EAPI double elm_scale_get(void);

/**
 * Set the global scaling factor
 *
 * This sets the globally configured scaling factor that is applied to all
 * objects.
 *
 * @param scale The scaling factor to set
 * @ingroup Scaling
 */
EAPI void   elm_scale_set(double scale);

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

